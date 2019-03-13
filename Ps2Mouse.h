#ifndef _PS2_MOUSE_H_
#define _PS2_MOUSE_H_

#include "Platform.h"

#define DEFAULT_SAMPLE_RATE 100
#define DEFAULT_RESOLUTION 4
#define MAX_RESOLUTION 8
#define DEFAULT_SCALING_2X1 false

struct MovementData {
  volatile int x = 0;
  volatile int y = 0;
  volatile bool button1 = 0;
  volatile bool button2 = 0;
  volatile bool button3 = 0;
  volatile bool changed = false;
};

class Ps2Mouse {
  private:
    const uint8_t ACK_CODE = 0xFA;
    const uint8_t BAT_OK = 0xAA;
    const uint8_t DEVICE_ID = 0x00;

    uint8_t sendBuffer[4];
    uint8_t sendBufferIdx = 0;
    uint8_t sendBufferLen = 0;

    volatile uint8_t activeCommand = 0;
    volatile bool streamingMode = true;
    volatile bool dataReporting = false;
    volatile uint8_t sampleRate = DEFAULT_SAMPLE_RATE;
    volatile uint8_t resolution = DEFAULT_RESOLUTION;
    volatile bool scaling2x1 = DEFAULT_SCALING_2X1;

    MovementData movementData;
    volatile unsigned long lastTxTime;

  public:
    Ps2Mouse();
    bool hasData();
    uint8_t popData();

  public:
    void reset(bool sendAck = false);
    void setDefaults();
    void task(unsigned long tNow);
    void updateMovementData(MovementData& movementData);
    void receiveFromHost(uint8_t dataByte, bool valid);
    void resend();

  private:
    void sendToHost(const uint8_t * data, uint8_t len);
    void handleActiveCommand(uint8_t dataByte);
    void handleNewCommand(uint8_t dataByte);
    void buildStatusPacket(uint8_t * packet);
    void buildMovementPacket(boolean use2x1Scaling, uint8_t * packet);
    int apply2x1Scaling(int movement);
};

#endif //_PS2_MOUSE_H_
