#!/bin/bash -e
FIRMWARE="\"$(cat fw.txt)\""
FBQN=arduino:samd:mkrnb1500

DEF="-DFIRMWARE=$FIRMWARE -DAPN=\"iot.1nce.net\" -DMI_MINUTE=2 -DLEGACY_BUILT=1"
arduino-cli compile -b ${FBQN} --build-property="compiler.cpp.extra_flags=$DEF" .
