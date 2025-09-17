#ifndef _NMEA_H
#define _NMEA_H

#include <Arduino.h>

#define NMEA_TIMEOUT 3000L
#define GGA_ID "GGA"
#define GGA_LEN 73

class NMEA {
private:
  char _buf[GGA_LEN];
  uint8_t _len;

public:
  void begin();
  char *dataset();
  void end();
  bool poll(uint32_t timeout = NMEA_TIMEOUT);
  bool valid();
};

#endif /* _NMEA_H */
