#!/bin/bash -e
FBQN=arduino:samd:mkrnb1500

if [ -n "$1" ]; then
	STAT_CTRL_ID="$1"
else
	STAT_CTRL_ID=$(kdialog --inputbox "STAT_CTRL_ID:" --title "BURN ID")
	if [ -z "$STAT_CTRL_ID" ]; then
		exit 1
	fi
fi

SKETCH=../arduino/sketches/ryujin
FIRMWARE="\"$(cat $SKETCH/fw.txt)\""
DEF="-DFIRMWARE=$FIRMWARE -DAPN=\"iot.1nce.net\" -DMI_MINUTE=15 -DLEGACY_BUILT=1"
arduino-cli compile -b ${FBQN} --build-property="compiler.cpp.extra_flags=$DEF" $SKETCH
PORT=$(arduino-cli board list | awk "/${FBQN}/"'{print $1}')
if [ -n "$PORT" ]; then
	arduino-cli upload -p ${PORT} -b ${FBQN} $SKETCH
	sleep 5
	echo -n "FORMAT..."
	r=$(echo f | python3 ryujin-ctrl.py)
	echo $r
	if [ "$r" == "OK" ]; then
		echo -n "BURN ${STAT_CTRL_ID}..."
		r=$(echo "\$${STAT_CTRL_ID}\#" | python3 ryujin-ctrl.py)
		echo $r
	fi
else
	echo "no board found" >&2
fi
