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
#include "Watchdog.h"
Watchdog watchdog;

//
// Initial setup
//
void setup() {
  watchdog.setup();
  Serial.begin(SERIAL_SPEED);
  ps2Mouse.reset();
  logStatus(); printLog(); // print status log on bootup
}

//
// Main loop
//
void loop() {
  // reset watchdog
  watchdog.reset();

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
