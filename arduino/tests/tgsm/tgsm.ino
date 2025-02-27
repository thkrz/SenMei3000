#include <MKRNB.h>
#include <RTCZero.h>

#include "config.h"
#include "global.h"

#define LF "\r\n"
#define SIGN(x) (((x)>=0?'+':'\0')+String(x))
#define PRNT2(x) (((x)<10?'0':'\0')+String(x))

GPRS gprs;
NBClient client;
NB nbAccess;
RTCZero rtc;

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

bool post(String s) {
  int n = s.length();
  if (n == 0)
    return false;
  bool ok = false;
  if (client.connect(HOST, PORT)) {
    client.println("POST "PATH"/"STAT_CTRL_ID" HTTP/1.1");
    client.println("Host: "HOST);
    client.println("Connection: close");
    client.println("Content-Type: text/plain");
    client.print("Content-Length: ");
    client.println(n);
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

void verify() {
  //if (!client.connected())
  //  client.stop();
  if (!nbAccess.isAccessAlive()) {
    nbAccess.shutdown();
    connect();
  }
}

void setup() {
  //pinMode(FET, OUTPUT);
  //digitalWrite(FET, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  delay(3000);

  connect();

  rtc.begin();
  rtc.setEpoch(nbAccess.getTime());

  rtc.setAlarmSeconds(0);
  rtc.enableAlarm(rtc.MATCH_SS);
  rtc.standbyMode();
}

void loop() {
  String s = String(rtc.getYear() + 2000);
  s += "-";
  s += PRNT2(rtc.getMonth());
  s += "-";
  s += PRNT2(rtc.getDay());
  s += "T";
  s += PRNT2(rtc.getHours());
  s += ":";
  s += PRNT2(rtc.getMinutes());
  s += LF;

  verify();
  post(s);
  rtc.standbyMode();
}
