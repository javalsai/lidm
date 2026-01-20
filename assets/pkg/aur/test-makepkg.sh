#!/usr/bin/env bash
set -e

MYSELF=$(realpath "$0")
MYDIR=$(dirname "$MYSELF")

cd "$MYDIR"
typeset -a pkgs=(lidm{,-git,-bin,-systemd,-dinit})

for pkg in "${pkgs[@]}"; do
    printf "\x1b[mEntering '%s'\x1b[0m\n" "$pkg"
    cd "$pkg"
    makepkg -Cf
    cd ..
    echo
done

if [[ -n "${PRINT_TREE:-}" ]]; then
    for pkg in "${pkgs[@]}"; do
        eza --tree "$pkg/pkg/"*
    done
fi
