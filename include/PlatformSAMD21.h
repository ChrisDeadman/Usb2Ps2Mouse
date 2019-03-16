#ifndef _PLATFORM_SAMD21_H_
#define _PLATFORM_SAMD21_H_

#include <Arduino.h>

#define CLOCK_PIN 8
#define DATA_PIN 2
#define LED_PIN LED_BUILTIN

#define Serial Serial1 // use PIN1 + PIN2
#define SERIAL_SPEED 115200

void samd21_setup_clock();
void samd21_enable_clock();
void samd21_disable_clock();
void samd21_enable_clock_irq();
void samd21_disable_clock_irq();
void samd21_stop_clock();
void samd21_setup_watchdog();
void samd21_reset_watchdog();
int samd21_get_free_memory();

#endif // _PLATFORM_SAMD21_H_
