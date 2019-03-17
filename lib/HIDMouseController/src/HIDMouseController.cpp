#include "HIDMouseController.h"

// callbacks
extern "C" {
void __hidMouseStateChangeEmptyCallback(MouseState*) { }
}
// implement in your code
void hidMouseStateChange(MouseState*) __attribute__ ((weak, alias("__hidMouseStateChangeEmptyCallback")));

void HIDMouseController::Parse(HID * /* hid */, uint32_t /* is_rpt_id */, uint32_t /* len */, uint8_t *buf) {
  bool button1 = (buf[0] & 0x01) > 0;
  bool button2 = (buf[0] & 0x02) > 0;
  bool button3 = (buf[0] & 0x04) > 0;
  bool button4 = (buf[0] & 0x08) > 0;
  bool button5 = (buf[0] & 0x10) > 0;
  int8_t dX = buf[1];
  int8_t dY = buf[2];
  int8_t dWheel = buf[3];

  // no changes
  if ((state.button1 == button1) &&
      (state.button2 == button2) &&
      (state.button3 == button3) &&
      (state.button4 == button4) &&
      (state.button5 == button5) &&
      (dX == 0) &&
      (dY == 0) &&
      (dWheel == 0)) {
        return;
      }

  state.button1 = button1;
  state.button2 = button2;
  state.button3 = button3;
  state.button4 = button4;
  state.button5 = button5;
  state.dX = dX;
  state.dY = dY;
  state.dWheel = dWheel;
  hidMouseStateChange(&state);
};
