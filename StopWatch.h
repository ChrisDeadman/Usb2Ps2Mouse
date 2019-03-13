#ifndef _STOPWATCH_H_
#define _STOPWATCH_H_

#include "Platform.h"

class StopWatch {
  private:
    volatile unsigned long startTime = 0;
    volatile unsigned long stopTime = 0;
    volatile bool stopped = true;

  public:

    void start() {
      startTime = millis();
      stopped = false;
    }

    void stop() {
      stopTime = millis();
      stopped = true;
    }

    unsigned long elapsed() {
      unsigned long endTime = stopped ? stopTime : millis();
      return elapsed(startTime, endTime);
    }

    static unsigned long elapsed(unsigned long startTime, unsigned long endTime) {
      if (endTime < startTime) {
        return (UINT32_MAX - startTime) + endTime;
      } else {
        return endTime - startTime;
      }
    }
};

#endif //_STOPWATCH_H_
