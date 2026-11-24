{
  config,
  lib,
  pkgs,
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
  cfg-file = get-cfg.file or null;
  maker = get-cfg.maker or null;
in
pkgs.stdenv.mkDerivation {
  pname = "lidm";
  inherit (config) src version;

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
    keysEnum = if maker != null then maker.keys-enum else { };
  };

  meta = {
    description = "A ✨fully✨ colorful customizable TUI display manager made in C for simplicity.";
    license = lib.licenses.gpl3Only;
    mainProgram = "lidm";
    platforms = lib.platforms.linux;
  };
}
