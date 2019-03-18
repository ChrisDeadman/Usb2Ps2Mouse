#ifndef _SAFE_STRING_H_
#define _SAFE_STRING_H_

#include <Arduino.h>

class SafeString {

private:

  const int maxSize;
  char * const buffer;

  // forbid copying, stack allocation and destruction
  SafeString() = delete;
  SafeString(const SafeString&) = delete;
  void operator=(const SafeString&) = delete;
  ~SafeString() = delete;

  SafeString(int maxSize) : maxSize(maxSize), buffer(new char[maxSize + 1]) {
    this->buffer[0] = 0;
  }

public:

  static SafeString* alloc(int maxSize) {
    return new SafeString(maxSize);
  }

  const char * get() {
    return buffer;
  }

  int length() {
    return strlen(buffer);
  }

  bool empty() {
    return length() <= 0;
  }

  SafeString* clear() {
    buffer[0] = 0;
    return this;
  }

  SafeString* concat(const char * const str);
  template <typename T> SafeString* concat(const char * const format, T value);

private:

  inline int safeSize(int offset, int size) {
    return (size <= (maxSize - offset)) ? size : (maxSize - offset);
  }
};

SafeString* SafeString::concat(const char * const str) {
  strncat(buffer, str, safeSize(length(), strlen(str)));
  return this;
}

template <typename T> SafeString* SafeString::concat(const char * const format, T value) {
  int offset = length();
  snprintf(&buffer[offset], maxSize-offset, format, value);
  return this;
}

#endif //_SAFE_STRING_H_
