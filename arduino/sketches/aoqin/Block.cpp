#include <Arduino.h>

#include "Block.h"

#define isactive(x) ((x)!=1023)

Block::Block(int8_t a, int8_t b) {
  pin = {{a, false}, {b, false}}
}

bool Block::isConnected(int i) {
  if (i < 0)
    return pin[0].conn || pin[1].conn;
  analogRead(pin[i].a);
  pin[i].conn = isactive(analogRead(pin[i].a));
  return pin[i].conn;
}
