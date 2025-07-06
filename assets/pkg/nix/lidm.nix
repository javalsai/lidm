{ config, pkgs, lib, ...}: pkgs.stdenv.mkDerivation rec {
    pname = "lidm";
    version = config.version;
    src = config.src;

    nativeBuildInputs = with pkgs; [
        gcc
        gnumake
        linux-pam
    ];

    makeFlags = [
        "DESTDIR=$(out)"
        "PREFIX="
    ]
    ++ lib.optional (config.xsessions != null)
        "CPPFLAGS+=-DSESSIONS_XSESSIONS=\\\"${config.xsessions}\\\""
    ++ lib.optional (config.wayland-sessions != null)
        "CPPFLAGS+=-DSESSIONS_WAYLAND=\\\"${config.wayland-sessions}\\\"";

    fixupPhase = ''
        rm -rf $out/etc
    '';
}
