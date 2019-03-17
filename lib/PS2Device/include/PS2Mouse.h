#ifndef _PS2_MOUSE_H_
#define _PS2_MOUSE_H_

#include "PS2Device.h"

#define DEFAULT_SAMPLE_RATE 100
#define DEFAULT_RESOLUTION 4
#define MAX_RESOLUTION 8
#define DEFAULT_SCALING_2X1 false

struct MovementData {
  int8_t x = 0;
  int8_t y = 0;
  bool button1 = 0;
  bool button2 = 0;
  bool button3 = 0;
};

class PS2Mouse : public PS2Device {

private:

  const uint8_t ACK_CODE = 0xFA;
  const uint8_t BAT_OK = 0xAA;
  const uint8_t DEVICE_ID = 0x00;

  uint8_t sendBuffer[4];
  uint8_t sendBufferIdx = 0;
  uint8_t sendBufferLen = 0;

  uint8_t activeCommand;
  bool streamingMode;
  bool dataReporting;
  uint8_t sampleRate;
  uint8_t resolution;
  bool scaling2x1;

  MovementData movementData;
  bool movementDataChanged;
  unsigned long lastTxTime;

public:

  PS2Mouse(PS2Port * port) : PS2Device(port) {
    setDefaults();
  }

  void setDefaults();
  void reset(bool sendAck = false);
  void updateMovementData(MovementData& movementData);
  void task() override;

private:

  void sendToHost(const uint8_t * data, uint8_t len);
  void handleActiveCommand(uint8_t dataByte);
  void handleNewCommand(uint8_t dataByte);
  void buildStatusPacket(uint8_t * packet);
  void buildMovementPacket(boolean use2x1Scaling, uint8_t * packet);
  int apply2x1Scaling(int movement);
};

#endif //_PS2_MOUSE_H_
