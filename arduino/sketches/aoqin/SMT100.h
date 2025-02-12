#ifndef _SMT100_H
#define _SMT100_H

#include "Block.h"

class SMT100: public Block {
  public:
    using Block::Block;

    String data();
    String identify() {
      return "13TRUEBNERSMT100038241127102256";
    }
    String wait() {
      return "0022";
    }
};

#endif /* _SMT100_H */
