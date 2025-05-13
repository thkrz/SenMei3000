#!/bin/bash -e
FBQN=arduino:samd:mkrnb1500

DEF=
for arg in "$@"; do
  DEF="$DEF -D$arg"
done

arduino-cli compile -b ${FBQN} --build-property="compiler.cpp.extra_flags=$DEF" .
PORT=$(arduino-cli board list | awk "/${FBQN}/"'{print $1}')
if [ -n "$PORT" ]; then
  arduino-cli upload -p ${PORT} -b ${FBQN} .
fi
