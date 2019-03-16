#ifndef _PS2_SENDER_H_
#define _PS2_SENDER_H_

#include "Platform.h"

class Ps2Sender {
  private:
    volatile bool sending = false;
    volatile uint8_t dataByte = 0;
    volatile uint8_t bitIdx = 0;
    volatile uint8_t parity = 0;

  public:
    bool isSending();

  public:
    void beginSend(uint8_t dataByte);
    void endSend();
    void onClock();
    void onInhibit();
};

#endif //_PS2_SENDER_H_
