#!/bin/bash
FBQN=arduino:samd:mkrnb1500
#TYPE=MI_HOUR
TYPE=MI_MINUTE
MI=15
STAT_CTRL_ID="LAB1"

PORT=$(arduino-cli board list | awk "/$FBQN/"'{print $1}')
arduino-cli compile -b ${FBQN} --build-property="compiler.cpp.extra_flags=-D${TYPE} -DMI=${MI} \"-DSTAT_CTRL_ID=\"${STAT_CTRL_ID}\"\"" .
arduino-cli upload -p ${PORT} -b ${FBQN} .
