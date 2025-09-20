#!/bin/bash -e
FBQN=arduino:samd:mkrnb1500

SKETCH=../arduino/tests/tgsm
arduino-cli compile -b ${FBQN} $SKETCH
PORT=$(arduino-cli board list | awk "/${FBQN}/"'{print $1}')
if [ -n "$PORT" ]; then
	arduino-cli upload -p ${PORT} -b ${FBQN} $SKETCH
	sleep 2
    picocom -b 9600 $PORT
else
	echo "no board found" >&2
fi
