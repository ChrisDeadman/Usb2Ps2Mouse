#include "Ps2Sender.h"

bool Ps2Sender::isSending() {
  return sending;
}

void Ps2Sender::beginSend(uint8_t dataByte) {
  this->dataByte = dataByte;
  bitIdx = 0;
  if (!sending) {
    platform_enable_clock();
    sending = true;
  }
}

void Ps2Sender::endSend() {
  if (sending) {
    platform_disable_clock();
    sending = false;
  }
}

void Ps2Sender::onClock() {
  if (!sending) {
    return;
  }

  // all bits sent, stop sending
  if (bitIdx >= 11) {
    endSend();
    return;
  }

  uint8_t bit;
  switch (bitIdx) {
    case 0: // start bit
      bit = 0;
      parity = 1; // ODD parity
      break;
    default: // data bits
      bit = (dataByte >> (bitIdx - 1)) & 1;
      parity ^= bit;
      break;
    case 9: // parity bit
      bit = parity;
      break;
    case 10: // stop bit
      bit = 1;
      break;
  }
  bitIdx++;;

  digitalWrite(DATA_PIN, bit ? HIGH : LOW);
}

void Ps2Sender::onInhibit() {
  if (!sending) {
    return;
  }
  // all bits sent, stop sending
  if (bitIdx >= 11) {
    endSend();
  }
  // resend on next clock if inhibited
  else {
    bitIdx = 0;
  }
}
