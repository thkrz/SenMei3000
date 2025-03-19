#include "Sensor.h"

#define A (100.0/3.0)
#define B 1.2

/* Truebner SMT100
 * https://www.truebner.de/de/smt100.php
 */
Sensor SMT100 = {
  [](float *u, float **w, int *len) {
    static float v[5];

    *len = 5;
    v[0] = u[0];
    v[1] = 0;
    v[2] = u[0] * A;
    v[3] = (u[1] - B) * A;
    v[4] = 0;
    *w = v;
  },
  String("13TRUEBNERSMT100038241127"),
  String("0022")
};
