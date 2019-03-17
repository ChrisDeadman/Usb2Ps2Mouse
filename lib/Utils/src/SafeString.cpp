#include "SafeString.h"

const char *SafeString:: get() {
  return buffer;
}

uint16_t SafeString::length() {
  return strlen(buffer);
}

bool SafeString::empty() {
  return length() <= 0;
}

SafeString* SafeString::concat(const char * str) {
  strncat(buffer, str, safeSize(length(), strlen(str)));
  return this;
}

SafeString* SafeString::concat(uint32_t value, const char * format) {
  uint16_t offset = length();
  snprintf(&buffer[offset], maxSize-offset, format, value);
  return this;
}

SafeString* SafeString::clear() {
  buffer[0] = 0;
  return this;
}

inline uint16_t SafeString::safeSize(uint16_t offset, uint16_t size) {
  return (size <= (maxSize - offset)) ? size : (maxSize - offset);
}
