#include "Sensor.h"

/* Truebner SMT100
 * https://www.truebner.de/de/smt100.php
 */
Sensor SMT100 = {
  [](float *u, float **w, int *len) {
    static float v[5];

    *len = 5;
    v[2] = u[0] * 100.0 / 3.0;
    v[3] = (u[1] - 1.2) * 100.0 / 3.0;
    *w = v;
  },
  String("13TRUEBNERSMT100000000000"),
  String("0022")
};

/* Truebner SMT50
 * https://www.truebner.de/de/smt50.php
 */
Sensor SMT50 = {
  [](float *u, float **w, int *len) {
    static float v[5];

    *len = 5;
    v[2] = u[0] * 50.0 / 3.0;
    v[3] = (u[1] - 0.5) * 100.0;
    *w = v;
  },
  String("13TRUEBNERSMT050000000000"),
  String("0022")
};
