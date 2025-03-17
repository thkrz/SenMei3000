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
    String& identify() override {
      static String s = "13TRUEBNERSMT100038241127";
      return s;
    }
    String& wait() override {
      static String s = "0022";
      return s;
    }
};

#endif /* _SMT100_H */
