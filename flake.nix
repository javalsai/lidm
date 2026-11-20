{
  description = "A ✨fully✨ colorful customizable TUI display manager made in C for simplicity.";

  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs =
    {
      flake-utils,
      nixpkgs,
      self,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };

        name = "lidm";
        version = import assets/pkg/nix/version.nix;

        lidm = pkgs.callPackage assets/pkg/nix/lidm.nix {
          inherit pkgs;
          lib = pkgs.lib;
          config = {
            inherit version;
            src = ./.;
            xsessions = null;
            wayland-sessions = null;
            cfg = null;
            # cfg = "cherry";
          };
        };
      in
      {
        apps.default = (flake-utils.lib.mkApp { drv = lidm; }) // { meta = lidm.meta; };
        packages.default = lidm;
        devShells.default = pkgs.mkShell { buildInputs = lidm.nativeBuildInputs ++ [ pkgs.clang-tools ]; };
        formatter = pkgs.nixfmt-tree;
      }
    )
    // {
      nixosModules.lidm = assets/pkg/nix/module.nix;
    };
}
