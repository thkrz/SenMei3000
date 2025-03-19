#ifndef _SMT100_H
#define _SMT100_H

#include "Block.h"

class SMT100: public Block {
  public:
    static const float A = 100.0 / 3.0;
    static const float B = 1.2;

    using Block::Block;

    String& data() override {
      float v[2] = {
        _u[0] * A,
        (_u[1] - B) * A
      };
      return CONCAT(v);
    }
    const char *identify() override {
      return "13TRUEBNERSMT100038241127";
    }
    const char *wait() override {
      return "0022";
    }
};

#endif /* _SMT100_H */
