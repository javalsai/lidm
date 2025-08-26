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

        # run with: nix run .#nixosConfigurations.x86_64-linux.vm.config.system.build.vm
        # you can ssh into the machine with: ssh root@localhost -p 2000
        nixosConfigurations.vm = nixpkgs.lib.nixosSystem {
          inherit system;
          modules = [
            self.nixosModules.default
            {
              services.displayManager.lidm = {
                enable = true;
                theme = "default";
              };

              programs.hyprland.enable = true;
              programs.hyprland.withUWSM = true;
              services.desktopManager.plasma6.enable = true;

              services.xserver.enable = true;
              services.xserver.windowManager.i3.enable = true;

              services.xserver.displayManager.lightdm.enable = false;

              users.users.test = {
                password = "test";
                group = "users";
                extraGroups = [ "wheel" ];
                description = "Max Mustermann";
                isNormalUser = true;
                createHome = true;
                shell = pkgs.bashInteractive;
              };

              users.extraUsers.root.password = "";
              users.mutableUsers = false;

              nixpkgs.overlays = [ self.overlays.default ];

              virtualisation.vmVariant.virtualisation = {
                diskImage = null;
                forwardPorts = [
                  {
                    from = "host";
                    host.port = 2000;
                    guest.port = 22;
                  }
                ];
              };

              services.openssh.enable = true;
              services.openssh.settings = {
                PermitRootLogin = "yes";
                PermitEmptyPasswords = "yes";
              };

              security.pam.services.sshd.allowNullPassword = true;

              services.qemuGuest.enable = true;
              networking.hostName = "lidm-test-vm";

              system.stateVersion = "25.05";
            }
          ];
        };
      }
    )
    // flake-utils.lib.eachDefaultSystemPassThrough (system: {
      nixosModules.default = assets/pkg/nix/module.nix;
      overlays.default = final: _: { lidm = final.callPackage assets/pkg/nix/lidm.nix { }; };
    });
}
