#include <MKRNB.h>
#include <RTCZero.h>
#include <SDI12.h>
#include <SHTC3.h>
#include <SPI.h>
#include <SPIMemory.h>
#include <Wire.h>

#include "config.h"
#include "global.h"

#define CAP 1024
#define MSG 12

#define FET 6 /* LED_BUILTIN */
#define MX  1
#define RX  4
#define TX  3
#define MOD 5
#define CS  7

#define BLK (sizeof(uint32_t))
#define LF "\r\n"
#define LEN (addr[0])
#define SIGN(x) ((x)>=0?'+':'\0')

GPRS gprs;
NBClient client;
NB nbAccess;
RTCZero rtc;
SDI12 socket(MX, RX, TX);
SPIFlash flash(CS);
uint32_t addr[CAP];
char sid[63];

float battery() {
  int p = analogRead(A1);
  return (float)p * 0.014956;  // R1 = 1.2M; R2 = 330k
}

void comm() {
  Serial.begin(19200);
  while(!Serial);

  for (;;) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == 'f')
        erase();
      else if (c == 'd') {
        uint32_t len = LEN;
        String s;
        while (load(s)) {
          Serial.print(s);
          discard();
        }
        LEN = len;
        Serial.print('#');
      }
    }
    delay(10);
  }
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

void dir() {
  for (int i = 0; i < CAP; i++)
    addr[i] = flash.readULong(i*BLK);
}

void discard() {
  if (LEN > 0)
    LEN--;
}

void disable() {
  digitalWrite(FET, LOW);
}

bool dump(String &s) {
  if (LEN < CAP) {
    uint32_t a = flash.getAddress(flash.sizeofStr(s));
    if (a == 0)
      return false;
    addr[LEN+1] = a;
    if (flash.writeStr(a, s)) {
      LEN++;
      sync();
      return true;
    }
  }
  return false;
}

void enable() {
  digitalWrite(FET, HIGH);
  delay(600);
}

void erase() {
  flash.eraseChip();
  for (int i = 0; i < CAP; i++) {
    flash.writeULong(i*BLK, 0);
    addr[i] = 0;
  }
}

bool handshake(char i) {
  static char cmd[3] = "a!";

  cmd[0] = i;
  for (int j = 0; j < 3; j++) {
    socket.sendCommand(cmd);
    delay(30);
    if (socket.available()) {
      socket.clearBuffer();
      return true;
    }
  }
  socket.clearBuffer();
  return false;
}

String& ident(char i) {
  static char cmd[4] = "aI!";
  static String s;

  cmd[0] = i;
  socket.sendCommand(cmd);
  delay(30);
  s = socket.readStringUntil('\n');
  return s;
}

void idle() {
  for(;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
}

bool load(String &s) {
  if (LEN < 1)
    return false;
  uint32_t a = addr[LEN];
  return flash.readStr(a, s);
}

String& measure(char i) {
  static char st[4] = "aM!";
  static char rd[5] = "aD0!";
  static String s;

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

char *prnt2(uint8_t n) {
  static char buf[3];

  sprintf(buf, "%02d", n);
  return buf;
}

bool post(String &s) {
  int n = s.length();
  if (n == 0)
    return false;
  bool ok = false;
  if (client.connect(HOST, PORT)) {
    client.println(F("POST "PATH"/"STAT_CTRL_ID" HTTP/1.1"));
    client.println(F("Host: "HOST));
    client.println(F("Connection: close"));
    client.println(F("Content-Type: text/plain"));
    client.print(F("Content-Length: "));
    client.println(n);
    client.println();
    client.print(s);

    char buf[MSG];
    unsigned long t = 0;
    for (int i = 0; i < MSG; i++) {
      while (!client.available() && t < TIMEOUT) {
        delay(10);
        t += 10;
      }
      buf[i] = client.read();
    }
    ok = strncmp("HTTP/1.1 201", buf, MSG) == 0;
  }
  return ok;
}

void pullup() {
  static int8_t pin[8] = {
    A0, A2, A3, A4, A5, A6, 0, 5
  };

  for (int i = 0; i < 8; i++)
    pinMode(pin[i], INPUT_PULLUP);
}

void resend() {
  String s;
  while (load(s)) {
    if (!post(s))
      break;
    discard();
    sync();
  }
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

void schedule() {
  uint8_t m = (rtc.getMinutes() / MIVL + 1) * MIVL;
  if (m == 60)
    m = 0;
  rtc.setAlarmMinutes(m);
}

void sync() {
  while (!flash.eraseSector(0));
  for (int i = 0; i < CAP; i++)
    flash.writeULong(i*BLK, addr[i]);
}

bool update() {
  String s = "UPDATE\r\n";
  for (char *p = sid; *p; p++)
    s += ident(*p);
  return post(s);
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
  pinMode(FET, OUTPUT);
  digitalWrite(FET, LOW);

  pullup();

  flash.begin();
  dir();

  if (digitalRead(MOD) == LOW)
    comm();
    /* not reached */

  socket.begin();
  enable();
  scan();
  disable();

  if (sid[0] == '\0')
    idle();

  Wire.begin();
  SHTC3.begin();

  connect();
  enable();
  while (!update())
    delay(1000);
  disable();

  rtc.begin();
  rtc.setEpoch(nbAccess.getTime());

  rtc.setAlarmSeconds(0);
  schedule();
  rtc.enableAlarm(rtc.MATCH_MMSS);

  flash.powerDown();
  rtc.standbyMode();
}

void loop() {
  String s;
  s.reserve(128);

  s += rtc.getYear() + 2000;
  s += '-';
  s += prnt2(rtc.getMonth());
  s += '-';
  s += prnt2(rtc.getDay());
  s += 'T';
  s += prnt2(rtc.getHours());
  s += ':';
  s += prnt2(rtc.getMinutes());
  s += LF;

  float bat0 = battery();
  s += '%';
  s += SIGN(bat0);
  s += bat0;
  s += LF;


  bool pm = bat0 < BAT_LOW;

  SHTC3.readSample(true, pm);
  float st = SHTC3.getTemperature();
  float rh = SHTC3.getHumidity();
  s += '!';
  s += SIGN(st);
  s += st;
  s += SIGN(rh);
  s += rh;
  s += LF;

  enable();
  for (char *p = sid; *p; p++)
    s += measure(*p);
  disable();

  flash.powerUp();
  if (pm) {
    nbAccess.shutdown();
    dump(s);
  } else {
    verify();
    if (LEN > 0)
      resend();
    if (!post(s))
      dump(s);
  }
  flash.powerDown();

  if (!pm)
    schedule();
  rtc.standbyMode();
}
