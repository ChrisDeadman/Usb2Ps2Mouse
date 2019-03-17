#ifndef MOUSE_STATE_H
#define MOUSE_STATE_H

struct MouseState {
  bool button1 = 0;
  bool button2 = 0;
  bool button3 = 0;
  bool button4 = 0;
  bool button5 = 0;
  int8_t dX = 0;
  int8_t dY = 0;
  int8_t dWheel = 0;
};

#endif // MOUSE_STATE_H
