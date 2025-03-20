#include "Block.h"

#define VOLT(x) ((x)*5.0/1023.0)

static String& CONCAT(float*, int);

String& CONCAT(float *x, int len) {
  static String s;

  s = "";
  for (int i = 0; i < len; i++) {
    if (x[i] >= 0)
      s += '+';
    s += x[i];
  }
  return s;
}

Block::Block(Sensor *sen, int8_t d, int8_t a, int8_t b) {
  pinMode(d, INPUT_PULLUP);
  _dip = d;
  _pin[0] = a;
  _pin[1] = b;
  _sen = sen;
}

String& Block::identify() {
  return _sen->id;
}

bool Block::isConnected() {
  return digitalRead(_dip) == LOW;
}

String& Block::measurement() {
  int len;
  float *w;
  _sen->conv(_u, &w, &len);
  return CONCAT(w, len);
}

void Block::readSample() {
  for (int i = 0; i < 2; i++) {
    analogRead(_pin[i]);
    _u[i] = VOLT(analogRead(_pin[i]));
  }
}

String& Block::wait() {
  return _sen->wait;
}
