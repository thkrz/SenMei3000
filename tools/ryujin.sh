#!/bin/bash
FBQN=arduino:samd:mkrnb1500
BAUD=9600
PORT=

compile() {
    SKETCH=$1
    shift 1
    DEF=
    for flag in "$@"; do
        DEF="-D$flag $DEF"
    done
    arduino-cli compile -b ${FBQN} --build-property="compiler.cpp.extra_flags=$DEF" $SKETCH
}

flags() {
    DEF=-DFIRMWARE=$1
    read -p "APN [io1.1nce.net]: " apn
    DEF="-DAPN=${apn:-iot.1nce.net} $DEF"

    read -p "MI_MINUTE [15]: " min
    DEF="-DMI_MINUTE=${min:-15} $DEF"

    read -p "LEGACY_BUILT [1]: " lgcy
    DEF="-DLEGACY_BUILT=${lgcy:-1} $DEF"
    
    echo $DEF
}

port() {
    arduino-cli board list | awk "/${FBQN}/"'{print $1}'
}

send() {
    echo "$@" | picocom -b $BAUD $PORT
}

upload() {
    arduino-cli upload -p $PORT -b ${FBQN} $1
}

usage() {
    echo "ryujin [COMMAND]" >&2
    exit 1
}

case "$1" in
init)
    PORT=$(port)
    sketch=../arduino/sketches/ryujin
    echo CC $sketch
    comnpile $sketch $(flags $(cat $sketch/fw.txt)) >/dev/null
    echo UP $sketch
    upload $sketch >/dev/null
    sleep 2
    echo -n "Format..."
    r=$(send f)
    echo $r
    if [ "$r" = "OK" ]; then
        echo -n "Burn..."
        r=$(send "\$MAINPRO_01#")
        echo $r
    fi
    ;;
gsm)
    echo "gsm"
    ;;
status)
    ;;
?)
    usage
    ;;
esac