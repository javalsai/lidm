#!/usr/bin/env bash
set -e

MYSELF=$(realpath "$0")
MYDIR=$(dirname "$MYSELF")

LIDM_PATH=${LIDM_PATH:-$(command which lidm)}
echo "Using '$LIDM_PATH'"
[[ -e "$LIDM_PATH" ]] || { echo "'$LIDM_PATH' is not executable" >&2; exit 1; }

echo "Press \`Ctrl + C\` once you are done viewing the theme"
sleep 3

for theme in "$MYDIR/"*.ini; do
    LIDM_CONF="$theme" "$LIDM_PATH" || :
    echo "That was '$(basename "$theme")'"
    sleep 2 || :
done
