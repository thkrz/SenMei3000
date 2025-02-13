#ifndef _SMT100_H
#define _SMT100_H

#include "Block.h"

class SMT100: public Block {
  public:
    static const float A = 100.0 / 3.0;
    static const float B = 1.2;

    using Block::Block;

    String data() override {
      float a[2] = {
        u[0] * A,
        (u[1] - B) * A
      };
      return CONCAT(a);
    }
    String identify() override {
      return "13TRUEBNERSMT100038241127102256";
    }
    String wait() override {
      return "0022";
    }
};

#endif /* _SMT100_H */
