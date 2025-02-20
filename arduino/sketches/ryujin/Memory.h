#ifndef _MEMORY_H
#define _MEMORY_H

#include <inttypes.h>
#include <SPIMemory.h>

class Flash {
  private:
    SPIFlash _flash();
    uint32_t _cap;
  public:
    Flash() {};

    void begin(int8_t cs);
    void erase();
    String pop();
    void push(String s);
};

#endif // _MEMORY_H
