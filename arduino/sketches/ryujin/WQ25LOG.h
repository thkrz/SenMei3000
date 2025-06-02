#ifndef _WQ25LOG_H
#define _WQ25LOG_H

#include <SPIMemory.h>

#define ADDR(i) ((i)*4)
#define INVALID 0xFFFFFFFF

class WQ25LOG {
  private:
    uint16_t _len;
    SPIFlash _flash;
    uint16_t _index;

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

