#include <SDI12.h>

#include "addr.h"

SDI12 socket(1);
char addr = 'A';

String ack() {
  return "";
}

String measure() {
}

void setup() {
  socket.begin();
}

void loop() {
  if (socket.available());
}
