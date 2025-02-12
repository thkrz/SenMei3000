#include "Block.h"

#define VOLT(x) ((x)*5.0/1023.0)

Block::Block(int8_t d, int8_t a, int8_t b) {
  pinMode(d, INPUT_PULLUP);
  dip = d;
  pin[0] = a;
  pin[1] = b;
}

bool Block::isConnected() {
  return digitalRead(dip) == LOW;
}

void Block::readSample(int num) {
  for (int i = 0; i < 2; i++) {
    V[i] = 0;
    analogRead(pin[i]);
    for (int n = 0; n < num; n++)
      V[i] += VOLT(analogRead(pin[i]));
    V[i] /= num;
  }
}
