#include <Arduino.h>
#include <cstdarg>

#include "Sensor.h"

Sensor::Sensor(int pin1, int pin2 = -1) {
  n = pin2 < 0 ? 1 : 2;
  pin[0] = pin1;
  pin[1] = pin2;
}

void Sensor::measure() {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < 3; j++)
      v[i] += analogRead(pin[i]);
    v[i] /= 3.0;
  }
}
