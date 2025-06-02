#ifndef _WQ25LOG_H
#define _WQ25LOG_H

#include <SPIMemory.h>

class WQ25LOG {
  private:
    uint32_t _cap;
    SPIFlash _flash;
    uint16_t _index;
    uint16_t _len;
    uint32_t _start;

    uint32_t _getReadAddress();
    uint32_t _getWriteAddress();

  public:
    WQ25LOG(int cs, uint16_t index = 4096);

    void begin();

    bool append(String&);
    void compact();
    bool format();
    bool readLast(String&);
    void removeLast();
    void sleep(bool);
};

#endif /* _WQ25LOG_H */

