#/usr/bin/env bash
set -e

MYSELF=$(realpath "$0")
MYDIR=$(dirname "$MYSELF")

for pkg in "$MYDIR"/*/; do
    cp "$MYDIR/pkg.gitignore" "$pkg/.gitignore"
done
