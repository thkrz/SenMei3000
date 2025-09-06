#ifndef _W25QLOG_H
#define _W25QLOG_H

#include <SPIMemory.h>

class W25QLOG {
private:
  SPIFlash _flash;
  uint32_t _cap;
  uint32_t _rp;
  uint32_t _wp;

public:
  W25QLOG(int cs);

  bool begin();

  bool append(const String &);
  bool format();
  bool read(String &);
  void seek(uint32_t);
  void sleep(bool);
  bool unlink();
};

#endif /* _W25QLOG_H */
