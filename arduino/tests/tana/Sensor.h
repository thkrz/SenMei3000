#ifndef _Sensor_H
#define _Sensor_H

#include <Arduino.h>

struct Sensor {
  float *(*conv)(float*);
  String id;
  String wait;
};

extern Sensor SMT100;

#endif /* _Sensor_H */
