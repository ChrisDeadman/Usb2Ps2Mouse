#include "Ps2Mouse.h"
#include "StopWatch.h"

Ps2Mouse::Ps2Mouse() {
  reset();
}

bool Ps2Mouse::hasData() {
  return sendBufferIdx < sendBufferLen;
}

uint8_t Ps2Mouse::popData() {
  if (!hasData()) {
    return 0xFF;
  }
  uint8_t value = sendBuffer[sendBufferIdx];
  sendBufferIdx++;
  return value;
}

void Ps2Mouse::reset(bool sendAck) {
  setDefaults();
  uint8_t packet[3];
  uint8_t idx = 0;
  if (sendAck) {
    packet[idx++] = ACK_CODE;
  }
  packet[idx++] = BAT_OK;
  packet[idx++] = DEVICE_ID;
  sendToHost(packet, idx);
}

void Ps2Mouse::setDefaults() {
  streamingMode = true;
  activeCommand = 0;
  dataReporting = false;
  sampleRate = DEFAULT_SAMPLE_RATE;
  resolution = DEFAULT_RESOLUTION;
  scaling2x1 = DEFAULT_SCALING_2X1;
  lastTxTime = 0;
}

void Ps2Mouse::updateMovementData(MovementData& movementData) {
  unsigned int scaledX = abs(movementData.x);
  unsigned int scaledY = abs(movementData.y);

  // assume input is max.resolution
  uint8_t res = resolution;
  while (res < MAX_RESOLUTION) {
    scaledX = (scaledX >> 1) | ((scaledX & 1) ? 1 : 0);
    scaledY = (scaledY >> 1) | ((scaledY & 1) ? 1 : 0);
    res <<= 1;
  }
  if (movementData.x < 0) scaledX = -scaledX;
  if (movementData.y < 0) scaledY = -scaledY;

  this->movementData.x = scaledX;
  this->movementData.y = scaledY;
  this->movementData.button1 = movementData.button1;
  this->movementData.button2 = movementData.button2;
  this->movementData.button3 = movementData.button3;
  this->movementData.changed = movementData.changed;
}

void Ps2Mouse::task(unsigned long tNow) {
  // don't send movement data if
  if (!movementData.changed ||// there is no change
      !streamingMode ||       // or streaming mode is disabled
      !dataReporting ||       // or data reporting is disabled
      (activeCommand != 0) || // or an active command is being processed
      hasData())              // or we're currently sending
  {
    return;
  }

  // wait until sample time has passed
  if (StopWatch::elapsed(lastTxTime, tNow) < (1000 / sampleRate)) {
    return;
  }

  // send movement data
  uint8_t packet[3];
  buildMovementPacket(scaling2x1 ? true : false, packet);
  movementData.changed = false;
  sendToHost(packet, 3);
  lastTxTime = tNow;
}

void Ps2Mouse::receiveFromHost(uint8_t dataByte, bool valid) {
  // stop sending immediately
  sendBufferIdx = sendBufferLen;

  if (valid) {
    if (activeCommand) {
      handleActiveCommand(dataByte);
    } else {
      handleNewCommand(dataByte);
    }
  }
}

void Ps2Mouse::resend() {
  sendBufferIdx = 0;
}

inline void Ps2Mouse::sendToHost(const uint8_t * data, uint8_t len) {
  for (uint8_t idx = 0; idx < len; idx++) {
    sendBuffer[idx] = data[idx];
  }
  sendBufferIdx = 0;
  sendBufferLen = len;
}

inline void Ps2Mouse::handleActiveCommand(uint8_t dataByte) {
  switch (activeCommand) {
    // Wrap Mode
    case (0xEE):
      switch (dataByte) {
        // Reset
        case 0xFF:
          reset(true);
          activeCommand = 0;
          break;
        // Reset Wrap Mode
        case 0xEC:
          sendToHost(&ACK_CODE, 1);
          activeCommand = 0;
          break;
        // echo every other byte in wrap mode
        default:
          sendToHost(&dataByte, 1);
          break;
      }
      break;
    // Set resolution
    case (0xE8):
      switch (dataByte) {
        // Reset
        case 0xFF:
          reset(true);
          activeCommand = 0;
          break;
        default:
          resolution = 1 << dataByte;
          if (resolution > MAX_RESOLUTION) resolution = MAX_RESOLUTION;
          sendToHost(&ACK_CODE, 1);
          activeCommand = 0;
          break;
      }
      break;
    // Set sample rate
    case (0xF3):
      switch (dataByte) {
        // Reset
        case 0xFF:
          reset(true);
          activeCommand = 0;
          break;
        default:
          sampleRate = dataByte;
          sendToHost(&ACK_CODE, 1);
          activeCommand = 0;
          break;
      }
      break;
    // Unknown active command
    default:
      activeCommand = 0;
      break;
  }
}

inline void Ps2Mouse::handleNewCommand(uint8_t dataByte) {
  uint8_t packet[4]; // prepare packet buffer

  switch (dataByte) {
    // Reset
    case 0xFF:
      reset(true);
      break;
    // Set defaults
    case 0xF6:
      setDefaults();
      sendToHost(&ACK_CODE, 1);
      break;
    // Resend
    case 0xFE:
      resend();
      break;
    // Get Device ID
    case 0xF2:
      packet[0] = ACK_CODE;
      packet[1] = DEVICE_ID;
      sendToHost(packet, 2);
      break;
    // Set Stream Mode
    case 0xEA:
      streamingMode = true;
      sendToHost(&ACK_CODE, 1);
      break;
    // Set Remote Mode
    case 0xF0:
      streamingMode = false;
      sendToHost(&ACK_CODE, 1);
      break;
    // Set Wrap Mode
    case 0xEE:
      activeCommand = dataByte;
      sendToHost(&ACK_CODE, 1);
      break;
    // Set scaling 1:1
    case 0xE6:
      scaling2x1 = false;
      sendToHost(&ACK_CODE, 1);
      break;
    // Set scaling 2:1
    case 0xE7:
      scaling2x1 = true;
      sendToHost(&ACK_CODE, 1);
      break;
    // Set resolution
    case 0xE8:
      activeCommand = dataByte;
      sendToHost(&ACK_CODE, 1);
      break;
    // Set sample rate
    case 0xF3:
      activeCommand = dataByte;
      sendToHost(&ACK_CODE, 1);
      break;
    // Enable Data Reporting
    case 0xF4:
      dataReporting = true;
      sendToHost(&ACK_CODE, 1);
      break;
    // Disable Data Reporting
    case 0xF5:
      dataReporting = false;
      sendToHost(&ACK_CODE, 1);
      break;
    // Status request
    case 0xE9:
      packet[0] = ACK_CODE;
      buildStatusPacket(&packet[1]);
      sendToHost(packet, 4);
      break;
    // Read Data
    case 0xEB:
      packet[0] = ACK_CODE;
      buildMovementPacket(false, &packet[1]);
      sendToHost(packet, 4);
      break;
    // Unknown command -> just ACK
    default:
      sendToHost(&ACK_CODE, 1);
      break;
  }
}

void Ps2Mouse::buildStatusPacket(uint8_t * packet) {
  packet[0] = 0;
  packet[1] = resolution;
  packet[2] = sampleRate;

  if (!streamingMode) packet[0] |= (1 << 6);
  if (dataReporting) packet[0] |= (1 << 5);
  if (scaling2x1) packet[0] |= (1 << 4);
  if (movementData.button1) packet[0] |= (1 << 2);
  if (movementData.button3) packet[0] |= (1 << 1);
  if (movementData.button2) packet[0] |= (1 << 0);
}

void Ps2Mouse::buildMovementPacket(boolean use2x1Scaling, uint8_t * packet) {
  int absX = abs(movementData.x);
  int absY = abs(movementData.y);

  if (use2x1Scaling) {
    absX = apply2x1Scaling(absX);
    absY = apply2x1Scaling(absY);
  }

  packet[0] = 1 << 3; // bit3 is always 1
  packet[1] = (uint8_t)(((movementData.x >= 0) ? (absX) : (~absX + 1)) & 0xFF); // two's complement
  packet[2] = (uint8_t)(((movementData.y >= 0) ? (absY) : (~absY + 1)) & 0xFF); // two's complement

  if (absY > 255) packet[0] |= (1 << 7);
  if (absX > 255) packet[0] |= (1 << 6);
  if (movementData.y < 0) packet[0] |= (1 << 5);
  if (movementData.x < 0) packet[0] |= (1 << 4);
  if (movementData.button3) packet[0] |= (1 << 2);
  if (movementData.button2) packet[0] |= (1 << 1);
  if (movementData.button1) packet[0] |= (1 << 0);
}

int Ps2Mouse::apply2x1Scaling(int movement) {
  // scaling=2:1 table
  switch (movement) {
    case 0:
      return 0;
    case 1:
    case 2:
      return 1;
    case 3:
      return 3;
    case 4:
      return 6;
    case 5:
      return 9;
    default:
      return movement << 1;
  }
}
