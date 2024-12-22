#include <SDI12.h>

SDI12 socket(1);
String c = "";
String r = "";

void setup() {
}

void loop() {
  if (socket.available()) {
    c += (char)socket.read();
  } else if (c.length() > 1) {

  }
}
