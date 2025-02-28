#ifndef _BLOCK_H
#define _BLOCK_H

#include <Arduino.h>

class Block {
  private:
    int8_t dip;
    int8_t pin[2];
  protected:
    float u[2];
  public:
    static String& CONCAT(float*, int len = 2);

    char addr;

    Block(int8_t, int8_t, int8_t);

    bool isConnected();
    void readSample(int num = 1);

    virtual String data() {};
    virtual String identify() {};
    virtual String wait() {};
};

#endif /* _BLOCK_H */
