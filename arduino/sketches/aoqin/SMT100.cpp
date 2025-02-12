#include "SMT100.h"

#define SLOPE (1/0.046)
#define INTERCEPT 20.1

String SMT100::data() {
  static String s;

  s = "+" + String(V[0] * SLOPE);
  float x = (V[1] - INTERCEPT) * SLOPE;
  if (x >= 0)
    s += "+";
  s += String(x);
  return s;
}
