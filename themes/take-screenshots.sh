#!/usr/bin/env bash
set -e

MYSELF=$(realpath "$0")
MYDIR=$(dirname "$MYSELF")

COLS=60
ROWS=35

if [[ -z "$IM_FLOATING" ]]; then
    # 12 pts ≈ 16 px
    exec hyprctl dispatch exec \
        "[float; size $((COLS*16)) $((ROWS*16))]" \
        "kitty --override font_size=12.0 --override background_opacity=1 --override cursor_trail=0 --override cursor_shape=beam --override cursor_blink_interval=0 bash -c 'cd \"$PWD\" && IM_FLOATING=1 LIDM_PATH=\"$LIDM_PATH\" \"$MYSELF\"'"
fi

LIDM_PATH=${LIDM_PATH:-$(command which lidm)}
echo "Using '$LIDM_PATH'"
[[ -e "$LIDM_PATH" ]] || {
    echo "'$LIDM_PATH' is not executable" >&2;
    sleep 2;
    exit 1;
}
echo "Press enter when the window is clearly visible and nothing in the way"
echo "ONLY use this script if you are on hyprland and a \"normal\" kitty config"
echo "AND make sure you have rg, jq, grim, gifski and maybe a few more"
read

BG=$(rg '^background ' ~/.config/kitty/kitty.conf | cut -d'#' -f2)
printf '\033]4;0;rgb:%s\007' "${BG:0:2}/${BG:2:2}/${BG:4:2}"

PRAD=$(hyprctl getoption decoration:rounding | rg int | cut -d' ' -f2)
hyprctl keyword decoration:rounding 0

for theme in "$MYDIR"/*.ini; do
    tty=$(tty)
    (LIDM_CONF="$theme" "$LIDM_PATH" <"$tty" || :)&
    LIDM_PID=$!

    sleep .2
    GEOMETRY=$(
        hyprctl -j activewindow | \
            jq -r '(.at[0]|tostring) + "," + (.at[1]|tostring) + " " + (.size[0]|tostring) + "x" + (.size[1]|tostring)'
    )
    grim -g "$GEOMETRY" - > "$MYDIR/screenshots/$(basename "$theme" | cut -d. -f1).png"
    kill $LIDM_PID
    sleep .1
done

gifski \
    -Q 100 --fps 2 \
    -W $((COLS*16)) \
    -o "$MYDIR/../assets/media/lidm.gif" \
    "$MYDIR"/screenshots/*.png

printf '\033]104\007'
hyprctl keyword decoration:rounding "$PRAD"
