#include "WQ25LOG.h"

#define ADDR(i) (_start+(i)*4)
#define INVALID 0xFFFFFFFF
#define TOGGLE (_start=_start>0?0:_cap)

WQ25LOG::WQ25LOG(int cs, uint16_t index)
  : _flash(cs) {
  _index = index;
}

void WQ25LOG::begin() {
  _flash.begin();
  _cap = _flash.getCapacity() >> 1;

  _start = 0;
  if (_flash.readULong(0) == INVALID && _flash.readULong(_cap) != INVALID)
    _start = _cap;

  _len = _index;
  while (_len > 0 && _flash.readULong(ADDR(_len-1)) == INVALID)
    _len--;
}

uint32_t WQ25LOG::_getReadAddress() {
  for (int i = _len; i > 0; i--) {
    uint32_t a = _flash.readULong(ADDR(i-1));
    if (_flash.readByte(a) == 0xFF)
      return a;
  }
  return INVALID;
}

uint32_t WQ25LOG::_getWriteAddress() {
  if (_len == 0)
    return ADDR(_index);
  uint32_t a = _flash.readULong(ADDR(_len-1));
  uint16_t len = _flash.readWord(a+1);
  return a + 3 + len;
}

bool WQ25LOG::append(String &s) {
  if (_len == _index)
    return false;

  uint32_t a = _getWriteAddress();

  char *buf = (char*)s.c_str();
  uint16_t len = s.length() + 1;

  if (a + 3 + len > _cap)
    return false;

  _flash.writeWord(a + 1, len);
  _flash.writeCharArray(a + 3, buf, len);
  _flash.writeULong(ADDR(_len), a);

  _len++;
  return true;
}

void WQ25LOG::compact() {
  uint16_t len = 0;
  for (int i = _len; i > 0; i--) {
    uint32_t a = _flash.readULong(ADDR(i-1));
    if (_flash.readByte(a) != 0xFF)
      continue;
  }

  for (uint32_t a = 0; i < _cap; a += 0x10000)
    _flash.eraseBlock64K(_start + a);
  TOGGLE;
}

bool WQ25LOG::format() {
  return _flash.eraseChip();
}

bool WQ25LOG::readLast(String &s) {
  uint32_t a = _getReadAddress();
  if (a == INVALID)
    return false;
  s = "";
  uint16_t len = _flash.readWord(a + 1);
  for (int j = 0; j < len - 1; j++)
    s += (char)_flash.readChar(a + 3 + j);
  return true;
}

void WQ25LOG::removeLast() {
  uint32_t a = _getReadAddress();
  if (a != INVALID)
    _flash.writeByte(a, 0x00);
}

void WQ25LOG::sleep(bool state) {
  if (state)
    _flash.powerDown();
  else
    _flash.powerUp();
}

