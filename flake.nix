{
  description = "A ✨fully✨ colorful customizable TUI display manager made in C for simplicity.";

  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs =
    {
      self,
      flake-utils,
      nixpkgs,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
        lidm = pkgs.callPackage assets/pkg/nix/lidm.nix { };
      in
      {
        packages = {
          inherit lidm;
          default = lidm;
        };
        devShell = pkgs.mkShell {
          buildInputs = lidm.nativeBuildInputs ++ [ pkgs.clang-tools ];
        };
        formatter = pkgs.nixfmt-tree;
      }
    )
    // flake-utils.lib.eachDefaultSystemPassThrough (system: {
      nixosModules.default = assets/pkg/nix/module.nix;
      overlays.default = final: _: { lidm = final.callPackage assets/pkg/nix/lidm.nix { }; };
    });
}
