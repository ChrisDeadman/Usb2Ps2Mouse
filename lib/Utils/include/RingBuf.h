#ifndef _RING_BUF_H_
#define _RING_BUF_H_

#include <Arduino.h>

template <typename T, int SIZE>
class RingBuf {

private:

  T buffer[SIZE];
  int headIdx; // prev. put idx
  int tailIdx; // next get idx

public:

  RingBuf();

  int length();
  bool isFilled();

  void put(T value);
  T get(int idx);
  void clear();
};

template <typename T, int SIZE>
RingBuf<T,SIZE>::RingBuf() {
  clear();
}

template <typename T, int SIZE>
int RingBuf<T,SIZE>::length() {
  return (tailIdx > 0) ? SIZE : (headIdx + 1);
}

template <typename T, int SIZE>
bool RingBuf<T,SIZE>::isFilled() {
  return length() >= SIZE;
}

template <typename T, int SIZE>
void RingBuf<T,SIZE>::put(T value) {
  if (++headIdx >= SIZE) {
    headIdx = 0;
    tailIdx = 1;
  } else if (tailIdx > 0) {
    tailIdx = (tailIdx + 1) % SIZE;
  }
  buffer[headIdx] = value;
}

template <typename T, int SIZE>
T RingBuf<T,SIZE>::get(int idx) {
  return buffer[(tailIdx + idx) % SIZE];
}

template <typename T, int SIZE>
void RingBuf<T,SIZE>::clear() {
  headIdx = -1;
  tailIdx = 0;
}

#endif //_RING_BUF_H_
