{ config, lib, pkgs, ... }:

let
    dmcfg = config.services.displayManager;
    desktops = dmcfg.sessionData.desktops;

    version = "1.2.1";
    lidmPkg = pkgs.callPackage ./lidm.nix {
        inherit pkgs;
        config = {
            inherit version lib;
            src = pkgs.fetchFromGitHub {
              owner = "javalsai";
              repo = "lidm";
              rev = "v${version}";
              sha256 = "sha256-3CgUI8PUs4c1bfBrykPw87SSa4lzrh4E4Hug7cGRKFk=";
            };

            xsessions = "${desktops}/share/xsessions";
            wayland-sessions = "${desktops}/share/wayland-sessions";
        };
    };
in
{
    config = {
        services.displayManager.defaultSession = "lidm";

        systemd.services.lidm = {
            description = "TUI display manager";
            aliases = [ "display-manager.service" ];
            after = [
                "systemd-user-sessions.service"
                "plymouth-quit-wait.service"
            ];
            serviceConfig = {
                Type = "idle";
                ExecStart = "${lidmPkg}/bin/lidm 7";
                StandardInput = "tty";
                StandardOutput = "tty";
                StandardError = "tty";
                TTYPath = "/dev/tty7";
                TTYReset = "yes";
                TTYVHangup = "yes";
            };
        };
    };
}
