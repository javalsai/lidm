#!/usr/bin/env bash
set -e

MYSELF=$(realpath "$0")
MYDIR=$(dirname "$MYSELF")

cd "$MYDIR"
typeset -a pkgs=(lidm{,-git,-bin,-systemd,-dinit})

for pkg in "${pkgs[@]}"; do
    printf "\x1b[mEntering '%s'\x1b[0m\n" "$pkg"
    cd "$pkg"
    # shellcheck disable=SC1091
    source PKGBUILD
    # shellcheck disable=SC2154
    for f in "${source[@]}"; do
        echo "$f"
        awk -F:: '{print $1}' <<<"$f" | xargs rm -rf
    done
    cd ..
    echo
done
