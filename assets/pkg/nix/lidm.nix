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
  cfg-file = if get-cfg != null then get-cfg.file else null;
  maker = if get-cfg != null then get-cfg.maker else null;
in
pkgs.stdenv.mkDerivation rec {
  pname = "lidm";
  version = config.version;
  src = config.src;
  meta = {
    description = "A ✨fully✨ colorful customizable TUI display manager made in C for simplicity.";
    mainProgram = "lidm";
    platforms = lib.platforms.unix;
  };

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
    ) "CPPFLAGS+=-DSESSIONS_WAYLAND=\\\"${config.wayland-sessions}\\\"";

  fixupPhase = ''
    rm -rf $out/etc
  '';

  passthru = {
    cfgFile = cfg-file;
    keysEnum =
      if maker != null then
	maker.keys-enum
      else
	{ };
  };
}
