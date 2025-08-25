#ifndef _W25QLOG_H
#define _W25QLOG_H

#include <SPIMemory.h>

class W25QLOG {
private:
  SPIFlash _flash;
  uint32_t _rp;
  uint32_t _wp;

public:
  W25QLOG(int cs);

  void begin();

  bool append(String &);
  bool format();
  bool read(String &);
  void seek(uint32_t);
  void sleep(bool);
  void unlink();
};

#endif /* _W25QLOG_H */
