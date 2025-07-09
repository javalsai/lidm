{
  config,
  pkgs,
  lib,
  ...
}:
let
  get-cfg =
    if config.cfg != null then
      import ./get-cfg-file.nix {
        inherit lib;
        inherit (config) cfg src;
      }
    else
      null;
  cfg-file = get-cfg.file;
  maker = get-cfg.maker;
in
pkgs.stdenv.mkDerivation rec {
  pname = "lidm";
  version = config.version;
  src = config.src;

  nativeBuildInputs = with pkgs; [
    gcc
    gnumake
    linux-pam
  ];

  makeFlags =
    [
      "DESTDIR=$(out)"
      "PREFIX="
    ]
    ++ lib.optional (
      config.xsessions != null
    ) "CPPFLAGS+=-DSESSIONS_XSESSIONS=\\\"${config.xsessions}\\\""
    ++ lib.optional (
      config.wayland-sessions != null
    ) "CPPFLAGS+=-DSESSIONS_WAYLAND=\\\"${config.wayland-sessions}\\\""
    ++ lib.optional (cfg-file != null) "CPPFLAGS+=-DLIDM_CONF_PATH=\\\"${cfg-file}\\\"";

  fixupPhase = ''
    rm -rf $out/etc
  '';

  passthru = {
    keysEnum = maker.keys-enum;
  };
}
