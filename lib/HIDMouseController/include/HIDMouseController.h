#ifndef HID_MOUSE_CONTROLLER_H
#define HID_MOUSE_CONTROLLER_H

#include <hidboot.h>
#include "MouseState.h"

class HIDMouseController : public HIDReportParser
{

private:

  HIDBoot<HID_PROTOCOL_MOUSE> hostMouse;
  MouseState state;

public:

  HIDMouseController(USBHost *usb) : hostMouse(usb) {
    hostMouse.SetReportParser(0, this);
    state.button1 = 0;
    state.button2 = 0;
    state.button3 = 0;
    state.button4 = 0;
    state.button5 = 0;
    state.dX = 0;
    state.dX = 0;
    state.dWheel = 0;
  };

  virtual void Parse(HID *hid, uint32_t is_rpt_id, uint32_t len, uint8_t *buf);
};

#endif // HID_MOUSE_CONTROLLER_H
