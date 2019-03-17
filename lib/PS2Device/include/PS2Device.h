#ifndef _PS2_DEVICE_H_
#define _PS2_DEVICE_H_

#include "PS2Port.h"
#include "PS2Receiver.h"
#include "PS2Sender.h"

class PS2Device : PS2PortObserver {

private:

  unsigned long timeLastInhibit;
  unsigned long timeLastHostRts;

protected:

  PS2Sender sender;
  PS2Receiver receiver;

public:

  PS2Device(PS2Port * port) : sender(port), receiver(port) {
    port->setup(this);
  }

  bool isBusy();
  unsigned long getTimeLastInhibit();
  unsigned long getTimeLastHostRts();

  void onClock() override;
  void onInhibit() override;
  void onHostRts() override;

  virtual void task() = 0;
};

#endif //_PS2_DEVICE_H_
