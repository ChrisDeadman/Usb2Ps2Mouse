#include <usbhub.h>

#include "Platform.h"
#include "SafeString.h"
#include "Ps2Sender.h"
#include "Ps2Receiver.h"
#include "Ps2Mouse.h"

//
// PS/2 mouse controller
//
Ps2Sender sender;
Ps2Receiver receiver;
Ps2Mouse ps2Mouse;

MovementData movementData;
volatile unsigned long timeLastReceived;
volatile unsigned long timeLastSent;
volatile unsigned long timeLastInhibit;
volatile unsigned long timeLastHostRts;

void ps2Clock() {
  sender.onClock();
  receiver.onClock();
}

void ps2ClockInhibit() {
  sender.onInhibit();
  receiver.onInhibit();
  if (sender.isSending()) timeLastInhibit = millis();
}

void ps2ClockHostRts() {
  timeLastHostRts = millis();
  sender.endSend();
  receiver.endReceive();
  receiver.beginReceive();
}

//
// USB mouse Controller
//
#include <MouseController.h>
USBHost usb;
USBHub usbHub(&usb); // so we can support controllers connected via hubs
MouseController usbMouse(usb);

inline void usbMouseDataChanged() {
  movementData.x = usbMouse.getXChange();
  movementData.y = -usbMouse.getYChange(); // y is inverted
  movementData.button1 = usbMouse.getButton(LEFT_BUTTON);
  movementData.button2 = usbMouse.getButton(RIGHT_BUTTON);
  movementData.button3 = usbMouse.getButton(MIDDLE_BUTTON);
  movementData.changed = true;
}

void mouseMoved() {
  usbMouseDataChanged();
}
void mouseDragged() {
  usbMouseDataChanged();
}
void mousePressed() {
  usbMouseDataChanged();
}
void mouseReleased() {
  usbMouseDataChanged();
}

//
// Serial logging
//
SafeString * logBuffer = SafeString::alloc(1000);

inline void logDataReceived(uint8_t dataByte, bool dataValid) {
  timeLastReceived = millis();
  logBuffer->concat("<= ")->concat(dataByte, "%02X")->concat(dataValid ? "" : "!!")->concat("\r\n");
}

inline void logDataSent(uint8_t dataByte) {
  timeLastSent = millis();
  logBuffer->concat("=> ")->concat(dataByte, "%02X")->concat("\r\n");
}

void logStatus() {
  logBuffer->concat("----------------------")->concat("\r\n");
  logBuffer->concat("USB => PS/2 Mouse V0.1")->concat("\r\n");
  logBuffer->concat("----------------------")->concat("\r\n");
  logBuffer->concat("free_memory=")->concat(platform_get_free_memory(), "%d")->concat("\r\n");
  logBuffer->concat("time_now=")->concat(millis(), "%lu")->concat("\r\n");
  logBuffer->concat("time_last_received=")->concat(timeLastReceived, "%lu")->concat("\r\n");
  logBuffer->concat("time_last_sent=")->concat(timeLastSent, "%lu")->concat("\r\n");
  logBuffer->concat("time_last_inhibit=")->concat(timeLastInhibit, "%lu")->concat("\r\n");
  logBuffer->concat("time_last_host_rts=")->concat(timeLastHostRts, "%lu")->concat("\r\n");
  logBuffer->concat("----------------------")->concat("\r\n");
  logBuffer->concat("clock=")->concat(digitalRead(CLOCK_PIN), "%u")->concat("\r\n");
  logBuffer->concat("data=")->concat(digitalRead(DATA_PIN), "%u")->concat("\r\n");
  logBuffer->concat("led=")->concat(digitalRead(LED_PIN), "%u")->concat("\r\n");
  logBuffer->concat("----------------------")->concat("\r\n");
  logBuffer->concat("sending=")->concat(sender.isSending() ? "true" : "false")->concat("\r\n");
  logBuffer->concat("receiving=")->concat(receiver.isReceiving() ? "true" : "false")->concat("\r\n");
  logBuffer->concat("----------------------")->concat("\r\n");
}

void printLog() {
  if (!logBuffer->empty()) {
    Serial.println(logBuffer->get());
    logBuffer->clear();
  }
}

//
// Initial setup
//
void setup() {
  platform_setup_clock();
  platform_setup_watchdog();
  usb.Init();
  Serial.begin(SERIAL_SPEED);
  ps2Mouse.reset();
  logStatus(); printLog(); // print status log on bootup
}

//
// Main loop
//
void loop() {
  platform_reset_watchdog();

  // print log if someone writes to our serial input
  if (Serial.available() && Serial.read()) {
    printLog();
    logStatus(); printLog();
  }

  // transmission in progress
  if (sender.isSending() || receiver.isReceiving()) {
    return;
  }

  // data received
  if (receiver.hasData()) {
    uint8_t dataByte = receiver.popData();
    bool dataValid = receiver.isDataValid();
    ps2Mouse.receiveFromHost(dataByte, dataValid);
    logDataReceived(dataByte, dataValid);
  }

  // send if we have something to send
  if (ps2Mouse.hasData()) {
    uint8_t dataByte = ps2Mouse.popData();
    sender.beginSend(dataByte);
    logDataSent(dataByte);
  }
  // give time to PS/2 / USB handlers
  else {
    // NOTE1: There seems to be some interrupt nesting issues between timer interrupts and USB (freeze causes watchdog reset), so disable IRQs during USB task
    // NOTE2: lower USB_XFER_TIMEOUT (e.g. to 50) in UsbCore.h, otherwise this may take longer than the watchdog's timeout
    platform_disable_clock_irq();
    usb.Task();
    platform_enable_clock_irq();

    ps2Mouse.updateMovementData(movementData);
    movementData.changed = false;
    ps2Mouse.task(millis());
  }
}
