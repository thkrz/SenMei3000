#include "gps.h"
#include <TinyGPSPlus.h>

bool GPS::_parse() {
  return true;
}

bool GPS::begin() { Serial.begin(4800); }

void GPS::close() { Serial.close(); }

bool GPS::wait() {
  bool sentence = false;
  uint32_t t0 = millis();
  while (millis() - t0 < GPS_TIMEOUT)
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '$') {
        _len = 0;
        sentence = true;
      }
      if (sentence) {
        if (_len < MAX_NMEA)
          _buf[_len++] = c;
        if (c == '\n')
          return _parse();
      }
    }
  return false;
}
