#!/bin/bash -e
PREFIX="$HOME/.local"
python3 -m zipapp ryujin -o ryujin.pyz -p "/usr/bin/python3"
install -m755 ryujin.pyz "$PREFIX/bin"
install -m644 Ryujin.desktop "$PREFIX/share/applications"
if [ -d "$(xdg-user-dir DESKTOP)" ]; then
  install -m644 Ryujin.desktop "$(xdg-usr-dir DESKTOP)"
fi
rm -f ryujin.pyz
