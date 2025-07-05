{
  description = "A ✨fully✨ colorful customizable TUI display manager made in C for simplicity.";

  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs =
    { flake-utils, nixpkgs, ... }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };

        name = "lidm";
        version = builtins.elemAt (
          builtins.match "VERSION[[:blank:]]*=[[:space:]]*([^\n]*)\n.*" (builtins.readFile ./Makefile)
        ) 0;

        lidm = (
          pkgs.stdenv.mkDerivation {
            pname = name;
            version = version;

            src = ./.;

            nativeBuildInputs = with pkgs; [
              gcc
              gnumake
              linux-pam
            ];

            makeFlags = [
              "DESTDIR=$(out)"
              "PREFIX="
            ];

            fixupPhase = ''
              rm -rf $out/etc
            '';
          }
        );
      in
      rec {
        defaultApp = flake-utils.lib.mkApp { drv = defaultPackage; };
        defaultPackage = lidm;
        devShell = pkgs.mkShell { buildInputs = lidm.nativeBuildInputs ++ [ pkgs.clang-tools ]; };
      }
    );
}
