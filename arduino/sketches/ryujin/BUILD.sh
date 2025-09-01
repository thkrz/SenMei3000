#!/bin/bash -e
FIRMWARE="\"1.000.001\""
FBQN=arduino:samd:mkrnb1500

DEF=-DFIRMWARE=$FIRMWARE
for arg in "$@"; do
  DEF="$DEF -D$arg"
done

arduino-cli compile -e -b ${FBQN} --build-property="compiler.cpp.extra_flags=$DEF" .
install -m644 build/arduino.samd.mkrnb1500/ryujin.ino.bin fw/${FIRMWARE//\"/}.bin && rm -r build/
PORT=$(arduino-cli board list | awk "/${FBQN}/"'{print $1}')
if [ -n "$PORT" ]; then
  arduino-cli upload -p ${PORT} -b ${FBQN} .
else
  echo "no board found" >&2
fi
