{ cfg, src, lib, ... }:
let
    maker = import ./make-cfg.nix {
        inherit lib;
        keys-h-file = builtins.readFile "${src}/include/keys.h";
    };
in builtins.toFile "lidm.conf" (
    if builtins.isString cfg
        then builtins.readFile "${src}/themes/${cfg}.ini"
    else if builtins.isAttrs cfg
        then maker.make cfg
    else builtins.throw "invalid cfg type, expected str or attrs"
)
