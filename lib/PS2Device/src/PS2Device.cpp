#include "PS2Device.h"

bool PS2Device::isBusy()  {
  return receiver.isReceiving() || sender.isSending();
}

unsigned long PS2Device::getTimeLastInhibit() {
  return timeLastInhibit;
}

unsigned long PS2Device::getTimeLastHostRts() {
  return timeLastHostRts;
}

void PS2Device::onClock() {
  receiver.onClock();
  sender.onClock();
}

void PS2Device::onInhibit() {
  receiver.onInhibit();
  sender.onInhibit();
  if (sender.isSending()) timeLastInhibit = millis();
}

void PS2Device::onHostRts() {
  sender.endSend();
  receiver.endReceive();
  receiver.beginReceive();
  timeLastHostRts = millis();
}
