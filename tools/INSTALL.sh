#!/bin/bash -e
PREFIX="$HOME/.local"
python3 -m zipapp ryujin -o ryujin.pyz -p "/usr/bin/python3"
install -m755 ryujin.pyz "$PREFIX/bin"
install -m644 Ryujin.desktop "$PREFIX/share/applications"
install -m644 Ryujin.png "$PREFIX/share/icons"
if [ -d "$(xdg-user-dir DESKTOP)" ]; then
  install -m644 Ryujin.desktop "$(xdg-user-dir DESKTOP)"
fi
rm -f ryujin.pyz
