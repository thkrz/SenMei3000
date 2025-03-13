#ifndef _RAIN_H
#define _RAIN_H

#include "Block.h"

uint32_t mm;

void isr() {
  mm++;
}

class RAIN: public Block {
  public:
    RAIN(int8_t d, int8_t a): Block(d, a, -1) {
      mm = 0;
      attachInterrupt(digitalPinToInterrupt(a), isr, HIGH);
    }

    String& data() override {
      uint32_t tm = mm;
      mm = 0;
      return CONCAT(tm);
    }
    const char *identify() override {
      return "13UNIWUEANRAINE1000000000";
    }
    const char *wait() override {
      return "0001";
    }
};

#endif /* _RAIN_H */
