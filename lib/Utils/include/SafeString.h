#ifndef _SAFE_STRING_H_
#define _SAFE_STRING_H_

#include <Arduino.h>

class SafeString {

private:

  const uint16_t maxSize;
  char * const buffer;

  // forbid copying, stack allocation and destruction
  SafeString() = delete;
  SafeString(const SafeString&) = delete;
  void operator=(const SafeString&) = delete;
  ~SafeString() = delete;

  SafeString(uint16_t maxSize) : maxSize(maxSize), buffer(new char[maxSize + 1]) {
    this->buffer[0] = 0;
  }

public:

  static SafeString* alloc(uint16_t maxSize) {
    return new SafeString(maxSize);
  }

  const char * get();
  uint16_t length();
  bool empty();
  SafeString* concat(const char * str);
  SafeString* concat(uint32_t value, const char * format);
  SafeString* clear();

private:

  uint16_t safeSize(uint16_t offset, uint16_t size);
};

#endif //_SAFE_STRING_H_
