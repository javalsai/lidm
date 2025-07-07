{ lib, keys-h-file }:

let
  double-match-to-nameval = dmatch: {
    name = builtins.elemAt dmatch 0;
    value = builtins.elemAt dmatch 1;
  };

  keys-enum =
    let
      key-names = builtins.replaceStrings [ "\n" " " ] [ "" "" ] (
        builtins.elemAt (builtins.match ".*KEY_NAMES\\[][[:blank:]]*=[[:blank:]]*\\{([^}]*)}.*" keys-h-file) 0
      );

      keys-2d-list = builtins.map (builtins.match "\\[(.*)]=\"(.*)\"") (
        builtins.filter (s: builtins.isString s && s != "") (builtins.split "," key-names)
      );

    in
    builtins.listToAttrs (
      builtins.map (
        k:
        k
        // {
          value = {
            __enum_key = k.value;
          };
        }
      ) (builtins.map double-match-to-nameval keys-2d-list)
    );
in
{
  inherit keys-enum;
  make =
    let
      name-val-to-attrs = (name: value: { inherit name value; });
      map-attrs = attrset: fn: lib.map fn (lib.attrsets.mapAttrsToList name-val-to-attrs attrset);

      try-foldl' =
        op: nul: list:
        if (builtins.length list) == 0 then "" else builtins.foldl' op nul list;
      concat-with = sepr: list: try-foldl' (x: y: if x == null then y else x + sepr + y) null list;

      ser-bool = bool: if bool then "true" else "false";
      ser-str = str: "\"${builtins.replaceStrings [ "\n" "\"" ] [ "\\n" "\\\"" ] str}\"";
      ser-kvs =
        { name, value }:
        if builtins.isList value then
          concat-with "\n" (builtins.map (value: ser-kvs { inherit name value; }) value)
        else
          "${name} = ${
            if builtins.isString value then
              ser-str value
            else if builtins.isInt value then
              builtins.toString value
            else if builtins.isBool value then
              ser-bool value
            else if builtins.isAttrs value then
              value.__enum_key
            else
              builtins.throw "type not supported"
          }";

      ser-table =
        table':
        let
          tname = table'.name;
          table = table'.value;
        in
        "[${tname}]\n" + (concat-with "\n" (map-attrs table ser-kvs));
    in
    cfg: concat-with "\n\n" (map-attrs cfg ser-table);
}
