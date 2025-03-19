#include "Sensor.h"

#define A (100.0/3.0)
#define B 1.2

static float *smt100data(float*);

Sensor SMT100 = {
  smt100data,
  "13TRUEBNERSMT100038241127",
  "0022"
};

float *smt100data(float *u) {
  static float v[2];

  v[0] = u[0] * A;
  v[1] = (u[1] - B) * A;
  return v;
}
