{
  stdenv,
  lib,
  gcc,
  gnumake,
  linux-pam,
  xsessions ? null,
  wayland-sessions ? null,
  ...
}:
stdenv.mkDerivation {
  pname = "lidm";
  version = builtins.elemAt (builtins.match "VERSION[[:blank:]]*=[[:space:]]*([^\n]*)\n.*" (builtins.readFile ../../../Makefile)) 0;
  src = ../../..;

  nativeBuildInputs = [
    gcc
    gnumake
    linux-pam
  ];

  makeFlags = [
    "DESTDIR=$(out)"
    "PREFIX="
  ]
  ++ lib.optional (xsessions != null) "CPPFLAGS+=-DSESSIONS_XSESSIONS=\\\"${xsessions}\\\""
  ++ lib.optional (
    wayland-sessions != null
  ) "CPPFLAGS+=-DSESSIONS_WAYLAND=\\\"${wayland-sessions}\\\"";

  postInstall = ''
    mkdir -p "$out/share/themes"
    cp $src/themes/*.ini $out/share/themes

  '';

  postFixup = ''
    rm -rf $out/etc
  '';

  meta = {
    description = "A fully colorful customizable TUI display manager made in C for simplicity";
    homepage = "https://github.com/javalsai/lidm";
    license = with lib.licenses; [ gpl3 ];
    mainProgram = "lidm";
  };
}
