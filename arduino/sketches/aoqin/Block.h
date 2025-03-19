#ifndef _BLOCK_H
#define _BLOCK_H

#include <Arduino.h>

#include "Sensor.h"

class Block {
  private:
    Sensor *_sen;
    int8_t _dip;
    int8_t _pin[2];
    float _u[2];

  public:
    char addr = -1;

    Block(Sensor*, int8_t, int8_t, int8_t);

    String& data();
    String& identify();
    bool isConnected();
    void readSample(int num = 1);
    String& wait();
};

#endif /* _BLOCK_H */
