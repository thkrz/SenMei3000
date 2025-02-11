#ifndef _BLOCK_H
#define _BLOCK_H

struct Pin {
  int8_t a;
  bool conn;
};

class Block {
  private:
    char addr;
    float u[2];
    Pin pin[2];
  public:
    String data();
    bool isConnected();
    void readSample();
    void setAddress(char a);
};

#endif /* _BLOCK_H */
