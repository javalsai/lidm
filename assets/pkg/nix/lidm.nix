{ config, pkgs, ...}: pkgs.stdenv.mkDerivation rec {
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
    ];

    installPhase = ''
        make install DESTDIR=$out PREFIX=
        mkdir -p $out/etc/systemd/system/
        make install-service-systemd DESTDIR=$out PREFIX=
    '';

    fixupPhase = ''
        rm -rf $out/etc
    '';
}
