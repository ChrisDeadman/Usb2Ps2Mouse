#ifndef _PS2_SENDER_H_
#define _PS2_SENDER_H_

#include "PS2Port.h"

class PS2Sender {

private:

  PS2Port * const port;
  volatile bool sending = false;
  volatile uint8_t dataByte = 0;
  volatile uint8_t bitIdx = 0;
  volatile uint8_t parity = 0;

public:

  PS2Sender(PS2Port * port) : port(port) {}

  bool isSending();

  void beginSend(uint8_t dataByte);
  void endSend();

  void onClock();
  void onInhibit();
};

#endif //_PS2_SENDER_H_
