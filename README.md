
Usb => Ps2Mouse v0.1
=====================

Author: ChrisDeadman

Convert a USB mouse into a PS/2 mouse with the power of arduino :-)

## Notes
* Watchdog timer is enabled and configured to 4sec
* You need to lower USB_XFER_TIMEOUT (e.g. to 50) in UsbCore.h (part of the used platform library)
  Usb.Task() may otherwise take longer than the watchdog's timeout and the chip will just reset over and over
* Diagnostic information is available via Serial1 - of course not USB since where else would you plug your mouse :D
  Sent/Received data + Status is returned by sending any character to Serial1 @ 115200 8N1
* Use an USB OTG cable to connect your USB mouse to the board
* CLOCK(pin8) and DATA(pin2) pins can be changed in PlatformSAMD21.h, check [avrfreaks](https://www.avrfreaks.net/sites/default/files/PS2%20Keyboard.pdf) for instruction on how to hook up a PS/2 connector to it
* Don't forget that SAMD21 has 3.3V rated pins, so you should really use level-converters to avoid any damage

## (Known) Limitations
* Mouse is limited to 3 buttons without scrollwheel because USBHost support is only supported via the HidBoot protocol
* It is not a keyboard (altho the Platform/Ps2Sender/Ps2Receiver classes can be used to build one)

## Supported Arduino Boards
* ATSAMD21G18 (developed on SparkFun SAMD21 Mini Breakout)

Low-level documentation about the PS/2 protocol can be found at [avrfreaks](https://www.avrfreaks.net/sites/default/files/PS2%20Keyboard.pdf)

Release notes
=======================

### Usb => Ps2Mouse v0.1
* Initial version
