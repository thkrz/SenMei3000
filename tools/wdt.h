Below is a short, self-contained example that illustrates how you can add a watchdog to an
MKR NB 1500 (SAMD21) sketch without pulling-in big dependencies.
The same technique can be dropped into your data-logger:

1.  enable the WDT when the CPU starts a “work phase”,
2.  kick it (“reset it”) from every long-running loop,
3.  disable it (or pause it) before you put the MCU in standby-sleep so that it does **not**
    expire while the processor is deliberately off.

The code uses the device registers directly; no external library is required.

--------------------------------------------------------------------
Minimal helper
--------------------------------------------------------------------
```cpp
/* ---------- watchdog_samd21.h ---------- */
#pragma once
#include <sam.h>

namespace WDTutil {

  // ------------- time-out table -------------
  // period code | cycles | 1.024 kHz clock ≈   |
  //     0       |   8    |   7.8 ms            |
  //     1       |  16    |  15.6 ms            |
  // ...
  //     9       | 8192   |   8   s  (largest)  |
  constexpr uint8_t PERIOD_8S = 9;   // pick the largest slot

  inline void enable(uint8_t periodCode = PERIOD_8S, bool runInStandby = false)
  {
    // Feed once in case the WDT was already running
    WDT->CLEAR.reg = WDT_CLEAR_KEY;
    while (WDT->STATUS.bit.SYNCBUSY);

    // Generic clock: connect OSCULP32K (32.768 kHz /32 = 1.024 kHz)
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(WDT_GCLK_ID) |
                        GCLK_CLKCTRL_CLKEN           |
                        GCLK_CLKCTRL_GEN_GCLK1;      // GCLK1 = OSCULP32K/32
    while (GCLK->STATUS.bit.SYNCBUSY);

    // Configure timeout and behaviour in standby
    WDT->CONFIG.reg = periodCode;                    // see table above
    WDT->CTRL.reg   = (runInStandby ? WDT_CTRL_WEN : 0) | WDT_CTRL_ENABLE;
    while (WDT->STATUS.bit.SYNCBUSY);

    // First clear starts the counter
    WDT->CLEAR.reg = WDT_CLEAR_KEY;
  }

  inline void reset()
  {
    WDT->CLEAR.reg = WDT_CLEAR_KEY;            // “kick the dog”
    while (WDT->STATUS.bit.SYNCBUSY);
  }

  inline void disable()
  {
    /* Disable the peripheral completely        */
    /* (must write CTRL.ENABLE=0 while CLEAR=key) */
    WDT->CLEAR.reg = WDT_CLEAR_KEY;
    while (WDT->STATUS.bit.SYNCBUSY);
    WDT->CTRL.reg  = 0;
    while (WDT->STATUS.bit.SYNCBUSY);
  }

  inline bool lastResetWasWatchdog()
  {
    return (RSTC->RCAUSE.reg & RSTC_RCAUSE_WDT);
  }
}
```

--------------------------------------------------------------------
How you would use it in your logger
--------------------------------------------------------------------
```cpp
#include "watchdog_samd21.h"

/* ----------------------------------------------------------------
   setup()
------------------------------------------------------------------*/
void setup()
{
  /*  optional: discover whether we rebooted because of the WDT  */
  if (WDTutil::lastResetWasWatchdog()) {
      Serial.begin(115200);
      while (!Serial);
      Serial.println(F("Resumed after watchdog reset!"));
  }

  /* ... your existing setup code ... */
}

/* ----------------------------------------------------------------
   loop()
------------------------------------------------------------------*/
void loop()
{
  /* ---------------- “work phase” ---------------- */
  WDTutil::enable();             // start 8-second watchdog

  q = "";                        // build payload, take ADC, etc.

  /* Long loops: remember to kick the dog           */
  enable();
  for (char *p = sid; *p; ++p) {
      q += rc(measure, *p);
      WDTutil::reset();          // we could be inside a 2-3-second exchange
  }
  disable();

  /* More potentially long blocks */
  if (!reconnect() || !resend() || !post(q)) {
      w25q.append(q);
  }
  WDTutil::reset();              // AT / TCP may have taken time

  /* ------------- finished with active work ------------- */
  WDTutil::disable();            // stop WDT before going to deep sleep

  /* programme next RTC alarm and sleep exactly like today */
  schedule();
  rtc.standbyMode();             // CPU off, GSM possibly off as well
}
```

Important details

•  PERIOD 9 (= 8 s) is the largest interval the SAMD21 offers; if some of your modem
   exchanges last longer, call `WDTutil::reset()` inside those loops (as in the example).

•  We *disable* the watchdog before `rtc.standbyMode()` because the low-power
   sleep will last minutes or hours and would otherwise guarantee a reset.

•  If you would rather keep the dog running during sleep (to recover from a
   frozen RTC, for instance), pass `runInStandby = true` to `enable()` **and**
   choose a period that exceeds your worst-case sleep time; this inevitably means
   you lose sub-minute granularity.

•  The helper also provides `lastResetWasWatchdog()` so you can log or treat a WDT
   reboot differently from a POR/BOD reset.

•  For ultra-compact code you can replace `watchdog_samd21.h` with a
   three-line `#include <Adafruit_SleepyDog.h>` solution; the principle
   (enable → reset in long loops → disable before standby) is exactly the same.
