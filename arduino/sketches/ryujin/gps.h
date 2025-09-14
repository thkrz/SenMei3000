#ifndef _GPS_H
#define _GPS_H

#define GPS_TIMEOUT 3000L
#define MAX_NMEA 85

class GPS {
private:
  char _buf[MAX_NMEA + 1];
  char *_part[MAX_TOK];
  uint8_t _len;
  uint8_t _idx;

  bool _cksum();

public:
  bool begin();
  bool wait();
};

#endif /* _GPS_H */
