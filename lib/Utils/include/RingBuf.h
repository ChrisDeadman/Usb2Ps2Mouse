#ifndef _RING_BUF_H_
#define _RING_BUF_H_

#include <Arduino.h>

template <uint16_t SIZE>
class RingBuf {

private:

  uint8_t buffer[SIZE];
  uint16_t headIdx; // prev. put idx
  uint16_t tailIdx; // next get idx

public:

  RingBuf();

  uint16_t length();
  bool isFilled();

  void put(uint8_t value);
  uint8_t get(uint16_t idx);
  void clear();
};

template <uint16_t SIZE>
RingBuf<SIZE>::RingBuf() {
  clear();
}

template <uint16_t SIZE>
uint16_t RingBuf<SIZE>::length() {
  return (tailIdx > 0) ? SIZE : (headIdx + 1);
}

template <uint16_t SIZE>
bool RingBuf<SIZE>::isFilled() {
  return length() >= SIZE;
}

template <uint16_t SIZE>
void RingBuf<SIZE>::put(uint8_t value) {
  if (++headIdx >= SIZE) {
    headIdx = 0;
    tailIdx = 1;
  } else if (tailIdx > 0) {
    tailIdx = (tailIdx + 1) % SIZE;
  }
  buffer[headIdx] = value;
}

template <uint16_t SIZE>
uint8_t RingBuf<SIZE>::get(uint16_t idx) {
  return buffer[(tailIdx + idx) % SIZE];
}

template <uint16_t SIZE>
void RingBuf<SIZE>::clear() {
  headIdx = -1;
  tailIdx = 0;
}

#endif //_RING_BUF_H_
