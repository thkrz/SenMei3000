#include "Memory.h"

#define BEG 4

void Flash::begin(int8_t cs) {
  _flash.begin(cs);
  _cap = _flash.getCapacity();
  erase();
}

String Flash::erase() {
  _flash.writeULong(0, BEG)
}

String Flash::pop() {
  uint32_t addr = _flash.readULong(0);
  size_t n = addr - BEG;
  String s = "";
  if (n > 0) {
    char buf[n+1];
    if (_flash.readCharArray(BEG, buf, n)) {
      buf[n] = '\0';
      s = String(buf);
    }
  }
  return s;
}

void Flash::push(String s) {
  int n = s.length();
  char buf[n];
  s.toCharArray(buf, n);
  uint32_t addr = _flash.readULong(0);
  if (addr + n > _cap)
    addr = BEG;
  if (_flash.writeCharArray(addr, buf, n))
    _flash.writeULong(0, addr + n)
}

