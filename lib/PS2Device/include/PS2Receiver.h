#ifndef _PS2_RECEIVER_H_
#define _PS2_RECEIVER_H_

#include "PS2Port.h"

class PS2Receiver {

private:

  PS2Port * const port;
  volatile bool receiving = false;
  volatile bool dataPresent = false;
  volatile bool dataValid = false;
  volatile uint8_t dataByte = 0;
  volatile uint8_t bitIdx = 0;
  volatile uint8_t parity = 0;

public:

  PS2Receiver(PS2Port * port) : port(port) {}

  bool isReceiving();
  bool hasData();
  bool isDataValid();

  uint8_t popData();
  void beginReceive();
  void endReceive();

  void onClock();
  void onInhibit();
};

#endif //_PS2_RECEIVER_H_
