#!/bin/bash -e
PREFIX="$HOME/.local"
python3 -m zipapp ryujin -o ryujin.pyz -p "/usr/bin/python3"
install -m755 ryujin.pyz "$PREFIX/bin"
desktop-file-install Ryujin.desktop
rm -f ryujin.pyz
