#include "PS2Mouse.h"

void PS2Mouse::setDefaults() {
  streamingMode = true;
  activeCommand = 0;
  dataReporting = false;
  sampleRate = DEFAULT_SAMPLE_RATE;
  resolution = DEFAULT_RESOLUTION;
  scaling2x1 = DEFAULT_SCALING_2X1;
  movementDataChanged = false;
  lastTxTime = 0;
}

void PS2Mouse::reset(bool sendAck) {
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

void PS2Mouse::updateMovementData(MovementData& movementData) {
  uint8_t scaledX = abs(movementData.x);
  uint8_t scaledY = abs(movementData.y);

  // assume input is max.resolution
  uint8_t res = resolution;
  while (res < MAX_RESOLUTION) {
    scaledX = (scaledX >> 1) | ((scaledX & 1) ? 1 : 0);
    scaledY = (scaledY >> 1) | ((scaledY & 1) ? 1 : 0);
    res <<= 1;
  }

  this->movementData.x = (movementData.x < 0) ? -scaledX : scaledX;
  this->movementData.y = (movementData.y < 0) ? -scaledY : scaledY;
  this->movementData.button1 = movementData.button1;
  this->movementData.button2 = movementData.button2;
  this->movementData.button3 = movementData.button3;
  this->movementDataChanged = true;
}

void PS2Mouse::task() {
  // wait until transmission is finished
  if (receiver.isReceiving() || sender.isSending()) {
    return;
  }

  // data received
  if (receiver.hasData()) {
    // stop sending in case we currently are
    sendBufferIdx = sendBufferLen;
    // invoke command handlers
    if (receiver.isDataValid()) {
      if (activeCommand) {
        handleActiveCommand(receiver.popData());
      } else {
        handleNewCommand(receiver.popData());
      }
    }
    return;
  }

  // check if we have anything to send
  if (sendBufferIdx < sendBufferLen) {
    uint8_t dataByte = sendBuffer[sendBufferIdx++];
    sender.beginSend(dataByte);
    return;
  }

  // don't send movement data if
  if (!movementDataChanged || // there is no change
      !streamingMode ||       // or streaming mode is disabled
      !dataReporting ||       // or data reporting is disabled
      (activeCommand != 0))   // or an active command is being processed
  {
    return;
  }

  // wait until sample time has passed
  unsigned long tNow = millis();
  unsigned long elapsed = (tNow < lastTxTime) ? ((UINT32_MAX - lastTxTime) + tNow) : (tNow - lastTxTime);
  if (elapsed < (1000 / sampleRate)) {
    return;
  }

  // send movement data
  uint8_t packet[3];
  buildMovementPacket(scaling2x1 ? true : false, packet);
  sendToHost(packet, 3);
  movementDataChanged = false;
  lastTxTime = tNow;
}

inline void PS2Mouse::sendToHost(const uint8_t * data, uint8_t len) {
  for (uint8_t idx = 0; idx < len; idx++) {
    sendBuffer[idx] = data[idx];
  }
  sendBufferIdx = 0;
  sendBufferLen = len;
}

inline void PS2Mouse::handleActiveCommand(uint8_t dataByte) {
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

inline void PS2Mouse::handleNewCommand(uint8_t dataByte) {
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
      sendBufferIdx = 0;
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

void PS2Mouse::buildStatusPacket(uint8_t * packet) {
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

void PS2Mouse::buildMovementPacket(boolean use2x1Scaling, uint8_t * packet) {
  uint8_t absX = abs(movementData.x);
  uint8_t absY = abs(movementData.y);

  if (use2x1Scaling) {
    absX = apply2x1Scaling(absX);
    absY = apply2x1Scaling(absY);
  }

  packet[0] = 1 << 3; // bit3 is always 1
  packet[1] = (uint8_t)(((movementData.x >= 0) ? (absX) : (~absX + 1)) & 0xFF); // two's complement
  packet[2] = (uint8_t)(((movementData.y >= 0) ? (absY) : (~absY + 1)) & 0xFF); // two's complement

  if (absY > 128) packet[0] |= (1 << 7);
  if (absX > 128) packet[0] |= (1 << 6);
  if (movementData.y < 0) packet[0] |= (1 << 5);
  if (movementData.x < 0) packet[0] |= (1 << 4);
  if (movementData.button3) packet[0] |= (1 << 2);
  if (movementData.button2) packet[0] |= (1 << 1);
  if (movementData.button1) packet[0] |= (1 << 0);
}

int PS2Mouse::apply2x1Scaling(int movement) {
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
