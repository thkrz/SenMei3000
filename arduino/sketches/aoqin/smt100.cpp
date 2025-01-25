#include "smt100.h"

float (*tr[2])(float) = {
  smt100moist,
  smt100temp
};

float smt100moist(float u) {
  return u;
}

float smt100temp(float u) {
  return u;
}
