#ifndef _BLOCK_H
#define _BLOCK_H

#include <Arduino.h>

class Block {
  private:
    int8_t _dip;
    int8_t _pin[2];
  protected:
    float _u[2];
  public:
    static String& CONCAT(float*, int len = 2);

    char addr;

    Block(int8_t, int8_t, int8_t);

    bool isConnected();
    void readSample(int num = 1);

    virtual String& data() {};
    virtual const char *identify() {};
    virtual const char *wait() {};
};

#endif /* _BLOCK_H */
