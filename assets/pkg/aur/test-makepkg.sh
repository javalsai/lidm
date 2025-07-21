#!/usr/bin/env bash
set -e

MYSELF=$(realpath "$0")
MYDIR=$(dirname "$MYSELF")

for pkg in "$MYDIR"/*/; do
    printf "\x1b[1mEntering '%s'\x1b[0m\n" "$pkg"
    cd "$pkg"
    makepkg -f .
    echo
done
