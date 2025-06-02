#ifndef _WQ25LOG_H
#define _WQ25LOG_H

#include <SPIMemory.h>

class WQ25LOG {
  private:
    SPIFlash _flash;
    uint32_t _rp;
    uint32_t _wp;

  public:
    WQ25LOG(int cs);

    void begin();

    bool append(String&);
    void delete();
    bool format();
    bool read(String&);
    void seek(uint32_t);
    void sleep(bool);
};

#endif /* _WQ25LOG_H */

