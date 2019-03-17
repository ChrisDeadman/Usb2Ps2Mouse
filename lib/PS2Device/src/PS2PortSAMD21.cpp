#ifdef __SAMD21G18A__
#include "PS2PortSAMD21.h"

// use TIMER/COUNTER#5
TcCount16 * const TC = (TcCount16*) TC5;
const IRQn TCIRQn = TC5_IRQn;

PS2PortObserver * observer = NULL;
volatile int8_t platform_sub_clock = 0;
volatile bool platform_clock_enabled = false;
volatile bool platform_clock_inhibited = false;

void TC5_Handler() {
  // wait until host releases the clock
  if (platform_clock_inhibited) {
    if (digitalRead(CLOCK_PIN) == HIGH) {
      platform_clock_inhibited = false;
      digitalWrite(LED_PIN, LOW); // status LED
      // Host requests to send
      if (digitalRead(DATA_PIN) == LOW) {
        observer->onHostRts();
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
      observer->onInhibit();
      digitalWrite(CLOCK_PIN, HIGH); // release clock pin
      digitalWrite(DATA_PIN, HIGH); // release data pin
      digitalWrite(LED_PIN, HIGH); // status LED
    }
    // generate clock
    else if (platform_clock_enabled) {
      if (platform_sub_clock == 1) {
        observer->onClock();
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

void PS2PortSAMD21::setup(PS2PortObserver * const _observer) {
  observer = _observer;
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
  enableClockIrq();
  while (TC->STATUS.bit.SYNCBUSY); // wait for synchronization
}

void PS2PortSAMD21::enableClock() {
  platform_sub_clock = 0;
  if (!platform_clock_enabled) {
    platform_clock_enabled = true;
    digitalWrite(LED_PIN, HIGH); // status LED
  }
}

void PS2PortSAMD21::disableClock() {
  if (platform_clock_enabled) {
    platform_clock_enabled = false;
    platform_sub_clock = 0;
    digitalWrite(CLOCK_PIN, HIGH); // release clock pin
    digitalWrite(DATA_PIN, HIGH); // release data pin
    digitalWrite(LED_PIN, LOW); // status LED
  }
}

uint8_t PS2PortSAMD21::read() {
  return digitalRead(DATA_PIN);
}

void PS2PortSAMD21::write(uint8_t bit) {
  return digitalWrite(DATA_PIN, bit ? HIGH : LOW);
}

void PS2PortSAMD21::enableClockIrq() {
  TC->INTENSET.bit.MC0 = 1;
  NVIC_EnableIRQ(TCIRQn);
}

void PS2PortSAMD21::disableClockIrq() {
  NVIC_DisableIRQ(TCIRQn); // disable interrupt request
  TC->INTENSET.bit.MC0 = 0;
}

#endif
