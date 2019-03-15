#include "Ps2Receiver.h"

bool Ps2Receiver::isReceiving() {
  return receiving;
}

bool Ps2Receiver::hasData() {
  return dataPresent;
}

bool Ps2Receiver::isDataValid() {
  return dataValid;
}

uint8_t Ps2Receiver::popData() {
  dataPresent = false;
  return dataByte;
}

void Ps2Receiver::beginReceive() {
  dataByte = 0;
  bitIdx = 0;
  dataValid = true;
  dataPresent = false;
  if (!receiving) {
    platform_enable_clock();
    receiving = true;
  }
}

void Ps2Receiver::endReceive() {
  if (receiving) {
    platform_disable_clock();
    receiving = false;
  }
}

void Ps2Receiver::onClock() {
  if (!receiving) {
    return;
  }

  // ACK sent, stop receiving
  if (bitIdx >= 11) {
    endReceive();
    return;
  }

  uint8_t bit = digitalRead(DATA_PIN) == HIGH ? 1 : 0;

  switch (bitIdx) {
    case 0: // start bit
      if (bit != 0) {
        dataValid = false;
      }
      parity = 1; // ODD parity
      break;
    default: // data bits
      dataByte |= (bit << (bitIdx - 1));
      parity ^= bit;
      break;
    case 9: // parity bit
      if (bit != parity) {
        dataValid = false;
      }
      break;
    case 10: // stop bit
      if (bit != 1) {
        dataValid = false;
      }
      dataPresent = true;
      digitalWrite(DATA_PIN, dataValid ? LOW : HIGH); // ACK / NAK
      break;
  }
  bitIdx++;
}

void Ps2Receiver::onInhibit() {
  if (!receiving) {
    return;
  }
  endReceive();
}
