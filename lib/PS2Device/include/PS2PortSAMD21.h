#ifndef _PS2_PORT_SAMD21_H_
#define _PS2_PORT_SAMD21_H_

#ifdef __SAMD21G18A__

#include <Arduino.h>
#include "PS2Port.h"

#define CLOCK_PIN 8
#define DATA_PIN 2
#define LED_PIN LED_BUILTIN

class PS2PortSAMD21 : public PS2Port {

public:

  void setup(PS2PortObserver * const observer) override;
  void enableClock() override;
  void disableClock() override;
  void write(uint8_t bit) override;
  uint8_t read() override;
  
  void enableClockIrq();
  void disableClockIrq();
};

#endif

#endif // _PS2_PORT_SAMD21_H_
