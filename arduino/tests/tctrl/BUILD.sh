#!/bin/bash -e
FIRMWARE="\"$(cat fw.txt)\""
#FBQN=arduino:samd:mkrnb1500
FBQN=arduino:samd:mkrzero

DEF=-DFIRMWARE=$FIRMWARE
cflag=0
for arg in "$@"; do
  if [ "$arg" = "-c" ]; then
    cflag=1
  else
    DEF="$DEF -D$arg"
  fi
done

arduino-cli compile -b ${FBQN} --build-property="compiler.cpp.extra_flags=$DEF" .
if [ "$cflag" -ne 0 ]; then
  exit 0
fi
PORT=$(arduino-cli board list | awk "/${FBQN}/"'{print $1}')
if [ -n "$PORT" ]; then
  arduino-cli upload -p ${PORT} -b ${FBQN} .
else
  echo "no board found" >&2
fi
