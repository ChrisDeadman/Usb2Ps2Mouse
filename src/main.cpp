//
// PS/2 mouse controller
//
#include "PS2PortSAMD21.h"
#include "PS2Mouse.h"
PS2PortSAMD21 ps2Port;
PS2Mouse ps2Mouse(&ps2Port);

//
// USB mouse Controller
//
#include <usbhub.h>
#include "HIDMouseController.h"
USBHost usb;
USBHub usbHub(&usb); // so we can support controllers connected via hubs
HIDMouseController usbMouse(&usb);

void hidMouseStateChange(MouseState * state) {
  ps2Mouse.updateState(state);
}

//
// Helpers
//
#include "Logging.h"

//
// Initial setup
//
void setupWatchdog() {
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

void setup() {
  setupWatchdog();
  Serial.begin(SERIAL_SPEED);
  ps2Mouse.reset();
  logStatus(); printLog(); // print status log on bootup
}

//
// Main loop
//
void loop() {
  // reset watchdog
  if (!WDT->STATUS.bit.SYNCBUSY) {
    WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
  }

  // print log if someone writes to our serial input
  if (Serial.available() && Serial.read()) {
    printLog();
    logStatus(); printLog();
  }

  // Poll USB device if PS/2 part is not busy
  // NOTE1: There seems to be some interrupt nesting issues between timer interrupts and USB (freeze causes watchdog reset), so disable IRQs during USB task
  // NOTE2: lower USB_XFER_TIMEOUT (e.g. to 50) in UsbCore.h, otherwise this may take longer than the watchdog's timeout
  if (!ps2Mouse.isBusy()) {
    ps2Port.disableClockIrq();
    usb.Task();
    ps2Port.enableClockIrq();
  }

  ps2Mouse.task();
}
