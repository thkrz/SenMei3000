#!/bin/bash -e
FIRMWARE="\"$(cat fw.txt)\""
FBQN=arduino:samd:mkrnb1500

if [ -z "$1" ]; then
  exit 1
fi

STAT_CTRL_ID=$(kdialog --inputbox "STAT_CTRL_ID:" --title "BURN ID")
if [ -z "$STAT_CTRL_ID" ]; then
  exit 1
fi

DEF="-DFIRMWARE=$FIRMWARE -DAPN=\"iot.1nce.net\" -DMI_MINUTE=15 -DLEGACY_BUILT=1"
arduino-cli compile -b ${FBQN} --build-property="compiler.cpp.extra_flags=$DEF" .
PORT=$(arduino-cli board list | awk "/${FBQN}/"'{print $1}')
if [ -n "$PORT" ]; then
  arduino-cli upload -p ${PORT} -b ${FBQN} .
  sleep 5
  echo "\$${STAT_CTRL_ID}\#" | picocom -q -b 57600 -f n -x 1000 $PORT >/dev/null
else
  echo "no board found" >&2
fi
