#include "Sensor.h"

#define A (100.0/3.0)
#define B 1.2

static void smt100data(float*, float**, int*);

Sensor SMT100 = {
  smt100data,
  String("13TRUEBNERSMT100038241127"),
  String("0022")
};

void smt100data(float *u, float **w, int *len) {
  static float v[5];

  *len = 5;
  v[0] = u[0];
  v[1] = 0;
  v[2] = u[0] * A;
  v[3] = (u[1] - B) * A;
  v[4] = 0;
  *w = v;
}
