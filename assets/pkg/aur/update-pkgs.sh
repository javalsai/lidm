#!/usr/bin/env bash
set -e

MYSELF=$(realpath "$0")
MYDIR=$(dirname "$MYSELF")

if [ -z "$1" ]; then
    printf "\x1b[1;31mERR: No version to update provided\x1b[0m\n" >&2
    exit 1;
fi
version="$1"
printf "\x1b[34mINF: Using '%s' version\x1b[0m\n" "$version"

for pkg in "$MYDIR"/lidm{,-bin}/; do
    cd "$pkg"
    printf "\x1b[1mEntering '%s'\x1b[0m\n" "$pkg"
    sed -i "s/pkgver=.*/pkgver=$1/" PKGBUILD
    sed -i "s/pkgrel=.*/pkgrel=1/" PKGBUILD
    updpkgsums
    makepkg --printsrcinfo | tee .SRCINFO
    echo
done
