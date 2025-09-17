#include "NMEA.h"

void NMEA::begin() {
  Serial.begin(4800);
}

char *NMEA::dataset() {
  return _buf;
}

void NMEA::end() {
  Serial.end();
}

bool NMEA::poll(uint32_t timeout) {
  bool sent = false;
  uint32_t t0 = millis();
  while (millis() - t0 < timeout)
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '$') {
        _len = 0;
        sent = true;
        t0 = millis();
      }
      if (sent) {
        if (_len < GGA_LEN - 1)
          _buf[_len++] = c;
        if (c == '\n') {
          _buf[_len] = '\0';
          return true;
        }
      }
    }
  return false;
}


bool NMEA::valid() {
  uint8_t pp = 0;
  char *p = &_buf[1];
  while (*p != '*') {
    pp ^= (uint8_t)*p;
    p++;
  }
  if (pp != strtol(++p, nullptr, 16))
    return false;

  if (strncmp(&_buf[3], GGA_ID, 3) != 0)
    return false;

  if (_buf[38] == '0')
    return false;

  if (atoi(&_buf[40]) < 4)
    return false;

  return true;
}
