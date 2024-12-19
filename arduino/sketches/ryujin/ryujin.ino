#include <MKRNB.h>
#include <RTCZero.h>
#include <SDI12.h>
#include <SHTSensor.h>
#include <SPI.h>
#include <Wire.h>

#include "config.h"
#include "global.h"

GPRS gprs;
NBClient client;
NB nbAccess;
RTCZero rtc;
SDI12 socket(1);
SHTSensor sht;
char sid[63];

float battery() {
  int p = analogRead(A1);
  return (float)p * 0.014956;  // R1 = 1.2M; R2 = 330k
}

void blink() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}

void connect() {
  bool connected = false;
  while (!connected) {
    if ((nbAccess.begin() == NB_READY)
        && (gprs.attachGPRS() == GPRS_READY)) {
      connected = true;
    } else {
      delay(1000);
    }
  }
}

void disable() {
  socket.end();
  digitalWrite(0, LOW);
  digitalWrite(LED_BUILTIN, LOW);
}

void enable() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(0, HIGH);
  socket.begin();
  delay(500);
}

bool handshake(char i) {
  static char cmd[3] = "0!";

  cmd[0] = i;
  for (int j = 0; j < 3; j++) {
    socket.sendCommand(cmd);
    socket.clearBuffer();
    delay(30);
    if (socket.available())
      return true;
  }
  socket.clearBuffer();
  return false;
}

String measure(char i) {
  static char st[4] = "aM!";
  static char rd[5] = "aD0!";
  static String s;

  socket.clearBuffer();
  delay(10);
  st[0] = i;
  socket.sendCommand(st);
  delay(30);
  s = socket.readStringUntil('\n');
  uint8_t wait = s.substring(1, 4).toInt();

  for (int j = 0; j < wait; j++) {
    if (socket.available()) {
      socket.clearBuffer();
      break;
    }
    delay(1000);
  }

  rd[0] = i;
  socket.sendCommand(rd);
  delay(30);
  s = socket.readStringUntil('\n');
  return s;
}

void scan() {
  int n = 0;
  for (char c = '0'; c <= '9'; c++) {
    if (handshake(c))
      sid[n++] = c;
  }
  for (char c = 'A'; c <= 'Z'; c++) {
    if (handshake(c))
      sid[n++] = c;
  }
  for (char c = 'a'; c <= 'z'; c++) {
    if (handshake(c))
      sid[n++] = c;
  }
  sid[n] = '\0';
}

bool post(String s) {
  bool success = false;
  if (client.connect(HOST, PORT)) {
    client.println("POST "PATH"/"STAT_CTRL_ID" HTTP/1.1");
    client.println("Host: "HOST);
    client.println("Connection: close");
    client.println("Content-Type: text/plain");
    client.print("Content-Length: ");
    client.println(s.length());
    client.println();
    client.print(s);

    String r = "";
    while (!success && client.available()) {
      r += (char)client.read();
      success = r == "HTTP/1.1 201";
    }
  }
  return success;
}

void schedule() {
  uint8_t m = (rtc.getMinutes() / MIVL + 1) * MIVL;
  rtc.setAlarmMinutes(m);
}

String sprint02d(uint8_t d) {
  static String s;

  s = d < 10 ? "0" : "";
  s += String(d);
  return s;
}

void verify() {
  //if (!client.connected())
  //  client.stop();
  if (!nbAccess.isAccessAlive()) {
    nbAccess.shutdown();
    connect();
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(1, INPUT);
  for (int i = 2; i < 8; i++)
    pinMode(i, INPUT_PULLUP);

  enable();
  scan();
  disable();

  if (sid[0] == '\0')
    for (;;)
      blink();

  Wire.begin();
  sht.init();

  connect();

  rtc.begin();
  rtc.setEpoch(nbAccess.getTime());

  rtc.setAlarmSeconds(0);
  schedule();
  rtc.enableAlarm(rtc.MATCH_MMSS);
  rtc.standbyMode();
}

void loop() {
  String s = String(rtc.getYear() + 2000);
  s += "-";
  s += sprint02d(rtc.getMonth());
  s += "-";
  s += sprint02d(rtc.getDay());
  s += "T";
  s += sprint02d(rtc.getHours());
  s += ":";
  s += sprint02d(rtc.getMinutes());
  s += "\r\n";

  float bat0 = battery();
  s += String(bat0) + "\r\n";

  enable();
  for (char *p = sid; *p; p++)
    s += measure(*p);
  disable();

  verify();
  bool success = post(s);

  schedule();
  rtc.standbyMode();
}
