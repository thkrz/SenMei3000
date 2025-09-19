#include "W25QLOG.h"

#define COMMIT 0xFE
#define DELETE 0xFC

#define SECTOR1 0x00001000
#define MAX_LEN 256
#define SHIFT(a, n) ((a) += 3 + (n))

W25QLOG::W25QLOG(int cs)
  : _flash(cs) {};

bool W25QLOG::begin() {
  if (!_flash.begin())
    return false;

  _cap = _flash.getCapacity();
  _rp = SECTOR1;
  _wp = SECTOR1;
  while (_wp + 3 < _cap) {
    uint16_t len = _flash.readWord(_wp + 1);
    if (len == 0xFFFF)
      break;
    SHIFT(_wp, len);
  }
  return true;
}

bool W25QLOG::append(const String &s) {
  uint16_t len = s.length() + 1;
  if (len > MAX_LEN - 1 || _wp + 3 + len > _cap)
    return false;

  if (!_flash.writeWord(_wp + 1, len) || !_flash.writeCharArray(_wp + 3, (char *)s.c_str(), len) || !_flash.writeByte(_wp, COMMIT))
    return false;
  SHIFT(_wp, len);
  return true;
}

bool W25QLOG::format() {
  _rp = SECTOR1;
  _wp = SECTOR1;
  return _flash.eraseSection(SECTOR1, _cap - SECTOR1);
}

String &W25QLOG::get() {
  static String s = "";

  if (!_flash.readStr(0, s))
    s = "";
  return s;
}

bool W25QLOG::put(String &s) {
  if (_flash.sizeofStr(s) > SECTOR1 - 3)
    return false;
  if (_flash.eraseSector(0))
    return _flash.writeStr(0, s);
  return false;
}

bool W25QLOG::read(String &s, bool advance) {
  char buf[MAX_LEN];

  while (_rp < _wp) {
    uint16_t len = _flash.readWord(_rp + 1);
    if (len == 0 || len > MAX_LEN - 1)
      return false;
    switch (_flash.readByte(_rp)) {
      case COMMIT:
        if (!_flash.readCharArray(_rp + 3, buf, len))
          return false;
        buf[len] = '\0';
        s = "";
        s += buf;
        if (advance)
          SHIFT(_rp, len);
        return true;
      case DELETE:
        break;
      default:
        return false;
    }
    SHIFT(_rp, len);
  }
  return false;
}

void W25QLOG::seek(uint32_t a) {
  _rp = a + SECTOR1;
}

void W25QLOG::sleep(bool state) {
  if (state)
    _flash.powerDown();
  else
    _flash.powerUp();
}

bool W25QLOG::unlink() {
  return _flash.writeByte(_rp, DELETE);
}
