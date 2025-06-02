#include "W25QLOG.h"

#define SHIFT(a,n) ((a)+=3+(n))

W25QLOG::W25QLOG(int cs)
  : _flash(cs) {
}

void W25QLOG::begin() {
  _flash.begin();

  _rp = 0;
  _wp = 0;
  while (_wp < _flash.getCapacity()) {
    uint16_t len = _flash.readWord(_wp + 1);
    if (len == 0xFFFF)
      break;
    SHIFT(_wp, len);
  }
}

bool W25QLOG::append(String &s) {
  uint16_t len = s.length() + 1;

  if (_wp + 3 + len > _flash.getCapacity())
    return false;

  _flash.writeWord(_wp + 1, len);
  _flash.writeCharArray(_wp + 3, (char*)s.c_str(), len);

  SHIFT(_wp, len);
  return true;
}

bool W25QLOG::format() {
  return _flash.eraseChip();
}

bool W25QLOG::read(String &s) {
  while (_rp < _wp) {
    uint16_t len = _flash.readWord(_rp + 1);
    if (_flash.readByte(_rp) == 0xFF) {
      char buf[len];
      _flash.readCharArray(_rp + 3, buf, len);
      s = "";
      s += buf;
      return true;
    }
    SHIFT(_rp, len);
  }
  return false;
}

void W25QLOG::seek(uint32_t a) {
  _rp = a;
}

void W25QLOG::sleep(bool state) {
  if (state)
    _flash.powerDown();
  else
    _flash.powerUp();
}

void W25QLOG::unlink() {
  _flash.writeByte(_rp, 0x00);
}

