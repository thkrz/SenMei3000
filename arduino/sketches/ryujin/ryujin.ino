#include <MKRNB.h>
#include <RTCZero.h>
#include <SDI12.h>
#include <SHTC3.h>
#include <SPI.h>
#include <SPIMemory.h>
#include <Wire.h>

#include "config.h"
#include "global.h"

#define CS_PIN  7
#define MOD_PIN 5
#define FET_PIN 0
#define MUX_PIN 1
#define SDI_R_PIN 2
#define SDI_W_PIN 3

#define LF "\r\n"

GPRS gprs;
NBClient client;
NB nbAccess;
RTCZero rtc;
SDI12 socket(MUX_PIN, SDI_R_PIN, SDI_W_PIN);
SPIFlash flash;
char sid[63];
uint32_t addr = 0;
uint32_t paddr = 0;

float battery() {
  int p = analogRead(A1);
  return (float)p * 0.014956;  // R1 = 1.2M; R2 = 330k
}

bool calibrate() {
  String s = "CALIBRATE\r\n";
  enable();
  for (char *p = sid; *p; p++)
    s += info(*p);
  disable();
  return post(s);
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
  digitalWrite(FET_PIN, LOW);
  digitalWrite(LED_BUILTIN, LOW);
}

void dump(String s) {
  flash.writeStr(addr, s);
  addr += s.length();
}

void enable() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(FET_PIN, HIGH);
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

String ident(char i) {
  static char cmd[4] = "aI!";
  static String s;

  cmd[0] = i;
  socket.sendCommand(cmd);
  delay(300);
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

String load() {
  static String s;

  if (flash.readStr(paddr, s))
    return s;
  return "";
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

void pullup() {
  int8_t pin[12] = {
    A0, A2, A3, A4, A5, A6,
    3, 4, 5, 6, 13, 14
  };

  for (int i = 0; i < 12; i++)
    pinMode(pin[i], INPUT_PULLUP);
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
  bool ok = false;
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
    while (!ok && client.available()) {
      r += (char)client.read();
      ok = r == "HTTP/1.1 201";
    }
  }
  return ok;
}

void schedule() {
  uint8_t m = (rtc.getMinutes() / MIVL + 1) * MIVL;
  if (m == 60)
    m = 0;
  rtc.setAlarmMinutes(m);
}

String sprint02d(uint8_t d) {
  static String s;

  s = d < 10 ? "0" : "";
  s += String(d);
  return s;
}

void switchmode() {
  Serial.begin(9600);
  while (!Serial);

  for (;;) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '!') {
        String s = load();
        Serial.print(s + "#");
      }
    }
    delay(100);
  }
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
  //pinMode(DBG_PIN, INPUT_PULLUP);
  //if (digitalRead(DBG_PIN) == HIGH)
  //  debug();
  //  /* not reached */
  //pullup();
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FET_PIN, OUTPUT);
  pinMode(MUX_PIN, OUTPUT);

  enable();
  scan();
  disable();

  if (sid[0] == '\0')
    idle();

  flash.begin(CS_PIN);

  Wire.begin();
  SHTC3.begin();

  connect();
  calibrate();

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
  s += LF;

  float bat0 = battery();
  s += String(bat0) + LF;

  bool pm = bat0 < BAT_LOW;

  SHTC3.readSample(low_power=pm);
  s += String(SHTC3.getTemperature()) + LF;
  s += String(SHTC3.getHumidity()) + LF;

  enable();
  for (char *p = sid; *p; p++)
    s += measure(*p);
  disable();

  if (pm) {
    nbAccess.shutdown();
    dump(s);
  } else {
    if (paddr < addr)
      s += load();
    verify();
    if (!post(s)) {
      addr = paddr;
      dump(s);
    } else {
      paddr = addr;
    }
  }

  if (!pm)
    schedule();
  rtc.standbyMode();
}
