#!/usr/bin/env bash
set -e

MYSELF=$(realpath "$0")
MYDIR=$(dirname "$MYSELF")

for pkg in "$MYDIR"/*/; do
    printf "\x1b[1mEntering '%s'\x1b[0m\n" "$pkg"
    cd "$pkg"

    # shellcheck disable=SC1091
    source "PKGBUILD"
    for source in "${source[@]}"; do
        awk -F'::' '{print $1}' <<<"$source" | xargs rm -rf
    done

    rm -rf ./*.{gz,zst} src pkg
    makepkg -f .

    echo
done
