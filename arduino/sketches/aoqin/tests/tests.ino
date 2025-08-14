#include <SDI12.h>

#define CMD_LEN 4
#define EE_ADDR 0
#define NUM_CON 6
#define BUS_PIN 4

static void rc();

SDI12 socket(BUS_PIN);
char cmd[CMD_LEN];
int len = 0;

void rc() {
  Serial.println(cmd);
  char addr = '0';

  String s;
  s += addr;
  s += "\r\n";
  socket.sendResponse(s);
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  socket.begin();
  socket.forceListen();
}

void loop() {
  if (socket.available()) {
    char c = socket.read();
    if (c == '!') {
      socket.clearBuffer();
      socket.forceHold();
      if (len > 0) {
        rc();
        len = 0;
      }
      socket.forceListen();
    } else if (c > 0 && len < CMD_LEN)
      cmd[len++] = c;
  }
}
