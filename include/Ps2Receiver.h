#ifndef _PS2_RECEIVER_H_
#define _PS2_RECEIVER_H_

#include "Platform.h"

class Ps2Receiver {
  private:
    volatile bool receiving = false;
    volatile bool dataPresent = false;
    volatile bool dataValid = false;
    volatile uint8_t dataByte = 0;
    volatile uint8_t bitIdx = 0;
    volatile uint8_t parity = 0;

  public:
    bool isReceiving();
    bool hasData();
    bool isDataValid();

  public:
    uint8_t popData();
    void beginReceive();
    void endReceive();
    void onClock();
    void onInhibit();
};

#endif //_PS2_RECEIVER_H_
