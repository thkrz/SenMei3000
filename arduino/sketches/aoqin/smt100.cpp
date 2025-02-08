#include "smt100.h"

float smt100moist(float);
float smt100temp(float);

float (*tr[2])(float) = {
  smt100moist,
  smt100temp
};

float smt100moist(float u) {
  return u * 10.0;
}

float smt100temp(float u) {
  return u * 10.0;
}
