#ifndef _LOGGING_H_
#define _LOGGING_H_

#include "SafeString.h"

#define Serial Serial1 // use PIN1 + PIN2
#define SERIAL_SPEED 115200

SafeString * logBuffer = SafeString::alloc(1000);

volatile unsigned long timeLastReceived;
volatile unsigned long timeLastSent;

extern "C" char* sbrk(int incr);
int getFreeMemory() {
  uint8_t stack_top;
  return &stack_top - reinterpret_cast<uint8_t*>(sbrk(0));
}

void ps2DataReceived(uint8_t dataByte, bool dataValid) {
  timeLastReceived = millis();
  logBuffer->concat("<= ")->concat("%02X", dataByte)->concat(dataValid ? "" : "!!")->concat("\r\n");
}

void ps2DataSent(uint8_t dataByte) {
  timeLastSent = millis();
  logBuffer->concat("=> ")->concat("%02X", dataByte)->concat("\r\n");
}

void logStatus() {
  logBuffer->concat("----------------------")->concat("\r\n");
  logBuffer->concat("USB => PS/2 Mouse V0.2")->concat("\r\n");
  logBuffer->concat("----------------------")->concat("\r\n");
  logBuffer->concat("device_id=")->concat("%d", ps2Mouse.getDeviceId())->concat("\r\n");
  logBuffer->concat("----------------------")->concat("\r\n");
  logBuffer->concat("free_memory=")->concat("%d", getFreeMemory())->concat("\r\n");
  logBuffer->concat("time_now=")->concat("%lu", millis())->concat("\r\n");
  logBuffer->concat("time_last_received=")->concat("%lu", timeLastReceived)->concat("\r\n");
  logBuffer->concat("time_last_sent=")->concat("%lu", timeLastSent)->concat("\r\n");
  logBuffer->concat("time_last_inhibit=")->concat("%lu", ps2Mouse.getTimeLastInhibit())->concat("\r\n");
  logBuffer->concat("time_last_host_rts=")->concat("%lu", ps2Mouse.getTimeLastHostRts())->concat("\r\n");
  logBuffer->concat("----------------------")->concat("\r\n");
  logBuffer->concat("clock=")->concat("%u", digitalRead(CLOCK_PIN))->concat("\r\n");
  logBuffer->concat("data=")->concat("%u", digitalRead(DATA_PIN))->concat("\r\n");
  logBuffer->concat("led=")->concat("%u", digitalRead(LED_PIN))->concat("\r\n");
  logBuffer->concat("----------------------")->concat("\r\n");
}

void printLog() {
  if (!logBuffer->empty()) {
    Serial.println(logBuffer->get());
    logBuffer->clear();
  }
}

#endif //_LOGGING_H_
