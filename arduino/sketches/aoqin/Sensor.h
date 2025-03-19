#ifndef _SENSOR_H
#define _SENSOR_H

#include <Arduino.h>

struct Sensor {
  void (*conv)(float*, float**, int*);
  String id;
  String wait;
};

extern Sensor SMT100;

#endif /* _SENSOR_H */
