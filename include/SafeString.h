#ifndef _SAFESTRING_H_
#define _SAFESTRING_H_

#include "Platform.h"

class SafeString {
  private:
    char * const buffer;
    const uint16_t maxSize;

    // forbid copying, stack allocation and destruction
  private:

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

    const char * get() {
      return buffer;
    }

    const uint16_t length() {
      return strlen(buffer);
    }

    const bool empty() {
      return length() <= 0;
    }

    SafeString* concat(const char * str) {
      strncat(buffer, str, safeSize(length(), strlen(str)));
      return this;
    }

    SafeString* concat(uint32_t value, const char * format) {
      uint16_t offset = length();
      snprintf(&buffer[offset], maxSize-offset, format, value);
      return this;
    }

    SafeString* clear() {
      buffer[0] = 0;
      return this;
    }

  private:

    inline uint16_t safeSize(uint16_t offset, uint16_t size) {
      return (size <= (maxSize - offset)) ? size : (maxSize - offset);
    }
};

#endif //_SAFESTRING_H_
