#ifndef _CTRL_H
#define _CTRL_H

#define ERR "ERROR"
#define OK "OK"

class Ctrl {
private:
  W25QLOG _w25q;

  void _burn() {
    String s = "";

    while (Serial.available()) {
      char c = Serial.read();
      if (c == '#')
        break;
      if (isPrintable(c))
        s += c;
    }
    Serial.print(_w25q.put(s) ? OK : ERR);
  }

  void _clear() {
    Serial.print(_w25q.format(false) ? OK : ERR);
  }

  void _dump() {
    String s = "";

    _w25q.seek(0);
    while (_w25q.read(s, true))
      Serial.print(s);
  }

  void _format() {
    Serial.print(_w25q.format(true) ? OK : ERR);
  }

  void _info() {
    Serial.print(F("FIRMWARE=" FIRMWARE "\r\n"));
    Serial.print(F("APN=" APN "\r\n"));
#if defined(MI_MINUTE)
    Serial.print(F("MI_MINUTE="));
    Serial.print(MI_MINUTE);
#elif defined(MI_HOUR)
    Serial.print(F("MI_HOUR="));
    Serial.print(MI_HOUR);
#endif
    Serial.print(F("\r\n"));
#if defined(LEGACY_BUILT) && LEGACY_BUILT == 1
    Serial.print(F("LEGACY_BUILT=1"));
#else
    Serial.print(F("LEGACY_BUILT=0"));
#endif
    Serial.print(F("\r\n"));
  }

  void _query() {
    Serial.print(_w25q.get());
  }

public:
  Ctrl(W25QLOG& w25q)
    : _w25q(w25q) {
    Serial.begin(57600);
  }

  ~Ctrl() {
    Serial.end();
  }

  void exec() {
    while (!Serial);

    for (;;) {
      if (Serial.available()) {
        char c = Serial.read();
        switch (c) {
          case '$':
            _burn();
            break;
          case 'c':
            _clear();
            break;
          case 'd':
            _dump();
            break;
          case 'f':
            _format();
            break;
          case 'i':
            _info();
            break;
          case 'q':
            _query();
            break;
        }
        Serial.print(F("#"));
      }
    }
  }
};

#endif /* _CTRL_H */
