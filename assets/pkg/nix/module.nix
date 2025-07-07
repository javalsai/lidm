{
  config,
  lib,
  pkgs,
  ...
}:

let
  cfg = config.services.lidm;

  dmcfg = config.services.displayManager;
  desktops = dmcfg.sessionData.desktops;

  version = "1.2.1";
  lidmPkg = pkgs.callPackage ./lidm.nix {
    inherit pkgs;
    config = {
      inherit version lib;
      cfg = cfg.config;
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
  options.services.lidm.config = lib.mkOption {
    type =
      with lib.types;
      oneOf [
        str
        attrs
      ];
    default = { };
    description = "Config options for lidm | Either attr tree or name of bundled themes";
  };
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
