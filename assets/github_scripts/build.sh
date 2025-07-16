#!/usr/bin/env bash
set -euo pipefail

if [ -z "$ARCH" ]; then
    echo "\$ARCH not present" >&2
    exit 1
fi

ERR=0
# shellcheck disable=SC2034
make -j"$(nproc)" "$@" 2> /tmp/stderr || ERR=$?

BSIZE=$(stat --printf="%s" lidm)
HSIZE=$(numfmt --to=iec-i<<<"$BSIZE")B
WARNS=$(
  sed -E \
      's/^([^ ]+\.[ch]):([0-9]+):([0-9]+): ([a-z]+): (.*)$/::\4 file=\1,line=\2,col=\3::\5/' \
      /tmp/stderr
)
WARNS_NUM=$({ [[ "$WARNS" == "" ]] && echo 0; } || wc -l <<<"$WARNS")

echo "$WARNS"

{
    echo "# Build for $ARCH"
    echo ""
    if [ -s "/tmp/stderr" ]; then
        echo "<details><summary><code>stderr</code></summary>"
        echo ""
        echo "\`\`\`"
        cat "/tmp/stderr"
        echo "\`\`\`"
        echo ""
        echo "</details>"
    else
        echo "*no \`stderr\` to show*"
    fi
    echo ""
    echo "## Stats"
    echo "* **Filesize:** $HSIZE ($BSIZE B)"
    echo "* **Warnings & Errors:** $WARNS_NUM"
} >> "$GITHUB_STEP_SUMMARY"

if [ "$ERR" -ne 0 ]; then exit "$ERR"; fi

mv lidm lidm-"$ARCH"

echo "DESCR='$HSIZE, $WARNS_NUM warnings'" >> "$GITHUB_OUTPUT"
