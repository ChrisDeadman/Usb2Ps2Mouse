#ifdef __SAMD21G18A__
#include "PlatformSAMD21.h"

volatile int8_t platform_sub_clock = 0;
volatile bool platform_clock_enabled = false;
volatile bool platform_clock_inhibited = false;

// external callbacks
extern "C" {
  void __ps2EmptyCallback() {}
}
// implement those in your code
void ps2Clock() __attribute__ ((weak, alias("__ps2EmptyCallback")));
void ps2ClockInhibit()__attribute__ ((weak, alias("__ps2EmptyCallback")));
void ps2ClockHostRts()__attribute__ ((weak, alias("__ps2EmptyCallback")));

// use TIMER/COUNTER#5
TcCount16 * const TC = (TcCount16*) TC5;
const IRQn TCIRQn = TC5_IRQn;

void TC5_Handler() {
  // wait until host releases the clock
  if (platform_clock_inhibited) {
    if (digitalRead(CLOCK_PIN) == HIGH) {
      platform_clock_inhibited = false;
      digitalWrite(LED_PIN, LOW); // status LED
      // Host requests to send
      if (digitalRead(DATA_PIN) == LOW) {
        ps2ClockHostRts();
      }
      // According to http://www.burtonsys.com/ps2_chapweske.htm:
      // "The Clock line must be continuously high for at least 50 microseconds before the device can begin to transmit its data."
      //
      // One sub-clock is 40us, so should be close enough with the additional cycle we need for the else branch
      // If not enough for your platform, set platform_sub_clock to -1 or -2 for additional wait cycles
      platform_sub_clock = 0;
    }
  } else {
    // Sub-Clock:
    // 0: HIGH
    // 1: HIGH -> ps2Clock() is called here
    // 2: LOW
    // 3: LOW
    uint8_t clk = (platform_sub_clock < 2) ? HIGH : LOW;
    digitalWrite(CLOCK_PIN, clk);

    // check if host inhibits clock
    if ((clk == HIGH) && (digitalRead(CLOCK_PIN) == LOW)) {
      platform_clock_inhibited = true;
      platform_sub_clock = 0;
      ps2ClockInhibit();
      digitalWrite(CLOCK_PIN, HIGH); // release clock pin
      digitalWrite(DATA_PIN, HIGH); // release data pin
      digitalWrite(LED_PIN, HIGH); // status LED
    }
    // generate clock
    else if (platform_clock_enabled) {
      if (platform_sub_clock == 1) {
        ps2Clock();
      }
      if (platform_sub_clock < 3) {
        ++platform_sub_clock;
      } else {
        platform_sub_clock = 0;
      }
    }
  }

  // set interrupt flag
  TC->INTFLAG.bit.MC0 = 1;
}

void samd21_setup_clock() {
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(CLOCK_PIN, HIGH);
  digitalWrite(DATA_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH); // status LED

  // clock the TC with the core cpu clock (48MHz)
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5);
  while (GCLK->STATUS.bit.SYNCBUSY); // wait for synchronization

  // reset TC
  TC->CTRLA.reg = TC_CTRLA_SWRST;
  while (TC->CTRLA.bit.SWRST); // wait for synchronization

  TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16; // set TC Mode to 16 bits
  TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ; // set TC mode as match frequency
  TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1; // set prescaler to 1
  TC->CTRLA.reg |= TC_CTRLA_ENABLE; // enable TC
  TC->CC[0].reg = (uint16_t) ((SystemCoreClock / 10000 / 4) - 1); // set counter to 10kHz*4 (4 sub-clocks needed)
  while (TC->STATUS.bit.SYNCBUSY); // wait for synchronization

  // enable TC interrupt request
  NVIC_ClearPendingIRQ(TCIRQn);
  NVIC_SetPriority(TCIRQn, 0);
  samd21_enable_clock_irq();
  while (TC->STATUS.bit.SYNCBUSY); // wait for synchronization
}

void samd21_enable_clock() {
  platform_sub_clock = 0;
  if (!platform_clock_enabled) {
    platform_clock_enabled = true;
    digitalWrite(LED_PIN, HIGH); // status LED
  }
}

void samd21_disable_clock() {
  if (platform_clock_enabled) {
    platform_clock_enabled = false;
    platform_sub_clock = 0;
    digitalWrite(CLOCK_PIN, HIGH); // release clock pin
    digitalWrite(DATA_PIN, HIGH); // release data pin
    digitalWrite(LED_PIN, LOW); // status LED
  }
}

void samd21_enable_clock_irq() {
  TC->INTENSET.bit.MC0 = 1;
  NVIC_EnableIRQ(TCIRQn);
}

void samd21_disable_clock_irq() {
  NVIC_DisableIRQ(TCIRQn); // disable interrupt request
  TC->INTENSET.bit.MC0 = 0;
}

void samd21_stop_clock() {
  samd21_disable_clock();
  samd21_disable_clock_irq();
  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE; // disable timer
  while (TC->STATUS.bit.SYNCBUSY); // wait for synchronization
}

void samd21_setup_watchdog() {
  // use low-power 32.768kHz oscillator as clock source @ 1024Hz.
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(GCM_FDPLL96M_32K) | GCLK_GENDIV_DIV(4); // use divisor of 32=2^(x+1)|x=4
  while (GCLK->STATUS.bit.SYNCBUSY); // wait for synchronization
  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(GCM_FDPLL96M_32K) | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_DIVSEL | GCLK_GENCTRL_IDC;
  while (GCLK->STATUS.bit.SYNCBUSY); // wait for synchronization
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_WDT | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2; // feed GCLK2 to WDT
  while (GCLK->STATUS.bit.SYNCBUSY); // wait for synchronization

  // disable early-warning interrupt
  NVIC_DisableIRQ(WDT_IRQn);
  NVIC_ClearPendingIRQ(WDT_IRQn);
  WDT->INTENSET.bit.EW = 0;

  WDT->CONFIG.bit.PER = 9; // Set period to 4096=2^(x+3)|x=9|00<=x<=0B clock cycles (4s)
  WDT->CTRL.bit.WEN = 0; // disable windowed mode (also resets on underflow)
  WDT->CTRL.bit.ENABLE = 1; // start WDT
  while (WDT->STATUS.bit.SYNCBUSY); // wait for synchronization
}

void samd21_reset_watchdog() {
  if (!WDT->STATUS.bit.SYNCBUSY) {
    WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
  }
}

extern "C" char* sbrk(int incr);
int samd21_get_free_memory() {
  uint8_t stack_top;
  return &stack_top - reinterpret_cast<uint8_t*>(sbrk(0));
}

#endif
