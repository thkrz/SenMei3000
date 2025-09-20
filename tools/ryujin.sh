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
    def=-DFIRMWARE=$1

    read -p "APN [io1.1nce.net]: " apn
    def="-DAPN=${apn:-iot.1nce.net} $def"

    read -p "MI_MINUTE [15]: " min
    def="-DMI_MINUTE=${min:-15} $def"

    read -p "LEGACY_BUILT [1]: " lgcy
    def="-DLEGACY_BUILT=${lgcy:-1} $def"
    
    echo $def
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

cmd=$1
if [ -z "$cmd" ]; then
    echo "1) init"
    echo "2) gsm"
    echo "3) status"
    echo "4) update"
    read -p "Select command [status]: " cmd
    cmd=${cmd:-4}
fi

case "$cmd" in
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
        read -p "STAT_CTRL_ID []: " sid
        r=$(send "\$$sid#")
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