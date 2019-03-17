#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include <Arduino.h>

template <uint16_t SIZE>
class RingBuf {

private:

  uint8_t buffer[SIZE];
  uint16_t headIdx;
  uint16_t _length;

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
  this->headIdx = 0;
  this->_length = 0;
}

template <uint16_t SIZE>
uint16_t RingBuf<SIZE>::length() {
  return _length;
}

template <uint16_t SIZE>
bool RingBuf<SIZE>::isFilled() {
  return _length >= SIZE;
}

template <uint16_t SIZE>
void RingBuf<SIZE>::put(uint8_t value) {
  buffer[headIdx] = value;
  if (++headIdx > SIZE) {
    headIdx = 0;
  }
  if (_length < SIZE) {
    ++_length;
  }
}

template <uint16_t SIZE>
uint8_t RingBuf<SIZE>::get(uint16_t idx) {
  return buffer[(headIdx + idx) % SIZE];
}

template <uint16_t SIZE>
void RingBuf<SIZE>::clear() {
  headIdx = 0;
  _length = 0;
}

#endif //_RING_BUFFER_H_
