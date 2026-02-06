#!/usr/bin/env bash

set -e

MYSELF=$(realpath "$0")
MYDIR=$(dirname "$MYSELF")

COLS=60
ROWS=35

if [[ -z "$IM_FLOATING" ]]; then
    if [[ -z "$LIDM_SESSIONS_N" ]]; then
        echo "Please set LIDM_SESSIONS_N" >&2;
        exit 1;
    fi

    # 12 pts ≈ 16 px
    exec hyprctl dispatch exec \
        "[float; size $((COLS*16)) $((ROWS*16))]" \
        "kitty --override font_size=12.0 --override background_opacity=1 --override cursor_trail=0 --override cursor_shape=beam --override cursor_blink_interval=0 bash -c 'cd \"$PWD\" && IM_FLOATING=1 LIDM_PATH=\"$LIDM_PATH\" LIDM_SCR_TTY=\"$(tty)\" LIDM_SESSIONS_N=\"$LIDM_SESSIONS_N\" bash -i \"$MYSELF\"'"
fi

LIDM_PATH=${LIDM_PATH:-$(command which lidm)}
echo "Using '$LIDM_PATH'"
[[ -e "$LIDM_PATH" ]] || {
    echo "'$LIDM_PATH' is not executable" >&2;
    sleep 2;
    exit 1;
}

notify-send "also make sure this is not in the way either"
echo "Press enter when the window is clearly visible and nothing in the way"
echo "ONLY use this script if you are on hyprland and a \"normal\" kitty config"
echo "AND make sure you have rg, jq, grim, gifski and maybe a few more"

exec 2>"$LIDM_SCR_TTY"
echo "This should show on the original terminal" > "$LIDM_SCR_TTY"
echo "And this too" >&2

read -r

BG=$(rg '^background ' ~/.config/kitty/kitty.conf | cut -d'#' -f2)
printf '\033]4;0;rgb:%s\007' "${BG:0:2}/${BG:2:2}/${BG:4:2}"

PRAD=$(hyprctl getoption decoration:rounding | rg int | cut -d' ' -f2)
hyprctl keyword decoration:rounding 0

for theme in "$MYDIR"/*.ini; do
    LIDM_CONF="$theme" "$LIDM_PATH" &
    LIDM_PID=$!

    (
        sleep .2
        for n in $(seq "$LIDM_SESSIONS_N"); do
            GEOMETRY=$(
                hyprctl -j activewindow | \
                    jq -r '(.at[0]|tostring) + "," + (.at[1]|tostring) + " " + (.size[0]|tostring) + "x" + (.size[1]|tostring)'
            )
            grim -g "$GEOMETRY" - > "$MYDIR/screenshots/$(basename "$theme" | cut -d. -f1)-$n.png"
            notify-send "$n taken"
            wtype -k right
            sleep .5
        done
        kill -15 $LIDM_PID
    ) &

    fg %-
    sleep .5
done

gifski \
    -Q 95 --fps 4 \
    -W $((COLS*32)) \
    -o "$MYDIR/../assets/media/lidm.gif" \
    "$MYDIR"/screenshots/*-?.png &> "$LIDM_SCR_TTY"

for screenshot in "$MYDIR"/screenshots/*"-2.png"; do
    # shellcheck disable=SC2001
    mv "$screenshot" "$(sed 's/-[0-9]\.png/.png/' <<<"$screenshot")"

done
rm "$MYDIR"/screenshots/*"-"[0-9].png

printf '\033]104\007'
hyprctl keyword decoration:rounding "$PRAD"
