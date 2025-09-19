#ifndef _CTRL_H
#define _CTRL_H

#define ERR "ERROR"
#define OK "OK"

class Ctrl {
private:
  W25QLOG _w25q;

  inline void _burn() {
    String s = "";

    while (Serial.available()) {
      char c = Serial.read();
      if (c == '\n')
        break;
      if (isPrintable(c))
        s += c;
    }
    Serial.print(_w25q.put(s) ? OK : ERR);
  }

  inline void _dump() {
    _w25q.seek(0);
    while (_w25q.read(s, true))
      Serial.print(s);
  }

  inline void _format() {
    Serial.print(_w25q.format() ? OK : ERR);
  }

  inline void _info() {
    Serial.print(F("FIRMWARE=" FIRMWARE "\r\n"));
    Serial.print(F("STAT_CTRL_ID="));
    Serial.print(_w25q.get())
      Serial.print(F("\r\n"));
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
  }

public:
  Ctrl(W25QLOG &w25q) {
    _w25q = w25q;
    Serial.begin(115200);
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
          case 'd':
            _dump();
            break;
          case 'f':
            _format();
            break;
          case 'i':
            _info();
            break;
        }
        Serial.print(F("#"));
      }
    }
  }
};

#endif /* _CTRL_H */
