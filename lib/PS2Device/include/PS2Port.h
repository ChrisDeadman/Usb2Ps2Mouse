#ifndef _PS2_PORT_H_
#define _PS2_PORT_H_

#include <Arduino.h>

class PS2PortObserver {

public:

  virtual void onClock() = 0;
  virtual void onInhibit() = 0;
  virtual void onHostRts() = 0;
};

class PS2Port {

public:

  virtual void setup(PS2PortObserver * const observer) = 0;
  virtual void enableClock() = 0;
  virtual void disableClock() = 0;
  virtual void write(uint8_t bit) = 0;
  virtual uint8_t read() = 0;
};

#endif //_PS2_PORT_H_
