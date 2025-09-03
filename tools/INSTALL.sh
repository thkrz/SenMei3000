#!/bin/bash
PREFIX="$HOME/.local"
python -m zipapp ryujin -o ryujin.pyz -p "/usr/bin/python3"
install -m755 ryujin.pyz "$PREFIX/bin"
install -m644 Ryujin.desktop "$PREFIX/share/applications"
rm -f ryujin.pyz
