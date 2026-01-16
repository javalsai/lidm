{
  pkgs,
  config,
  lib,
  ...
}:
let
  inherit (lib)
    types
    mkEnableOption
    mkOption
    mkPackageOption
    ;

  settingsFormat = pkgs.formats.ini { };

  cfg = config.services.displayManager.lidm;
in
{
  options.services.displayManager.lidm = {
    enable = mkEnableOption "lidm";
    package = mkPackageOption pkgs "lidm" { };
    theme = mkOption {
      type = types.either types.str (
        types.submodule {
          freeformType = settingsFormat.type;
        }
      );
      description = ''
        Either a name of a prepackaged theme, see
        [github.com/javalsai/lidm](https://github.com/javalsai/lidm/tree/master/themes)
        or a custom configuration, see the different
        themes for possible configurations.
      '';
      default = "cherry";
    };
    wrapperCommand = mkOption {
      type = types.str;
      default = "";
      example = lib.literalExample "${pkgs.kmscon}/bin/kmscon -l --vt /dev/tty7 --font-name \"Cascadia Code\"";
      description = ''
        A command to wrap lidm, useful because the standard
        tty doesn't support all colors/symbols.
      '';
    };
  };

  config = lib.mkIf cfg.enable {
    assertions = [
      {
        assertion = !config.services.displayManager.autoLogin.enable;
        message = ''
          lidm doesn't support auto login.
        '';
      }
    ];

    security.pam.services.lidm = {
      unixAuth = true;
      startSession = true;
    };

    services = {
      dbus.packages = [ cfg.package ];
      displayManager = {
        enable = true;
        execCmd = "exec ${cfg.wrapperCommand} ${lib.getExe cfg.package}";
      };
      xserver = {
        # To enable user switching, allow ly to allocate displays dynamically.
        display = null;
      };
    };

    environment.systemPackages = [ pkgs.lidm ];

    systemd.services.display-manager = {
      enable = true;
      unitConfig = {
        Wants = [ "systemd-user-sessions.service" ];
        After = [
          "systemd-user-sessions.service"
          "plymouth-quit-wait.service"
        ];
      };
      serviceConfig = {
        Type = "idle";
        StandardInput = "tty";
        TTYPath = "/dev/tty1";
        TTYReset = "yes";
        TTYVHangup = "yes";
        # Clear the console before starting
        TTYVTDisallocate = true;
      };
      environment =
        let
          desktops = config.services.displayManager.sessionData.desktops;
        in
        {
          LIDM_SESSIONS_XSESSIONS = "${desktops}/share/xsessions";
          LIDM_SESSIONS_WAYLAND = "${desktops}/share/wayland-sessions";
          LIDM_PAM_SERVICE = "lidm";
        };
      # Don't kill a user session when using nixos-rebuild
      restartIfChanged = false;
    };

    environment.etc."lidm.ini".source =
      if (lib.typeOf cfg.theme == "string") then
        "${cfg.package}/share/themes/${cfg.theme}.ini"
      else
        settingsFormat.generate "lidm.ini" cfg.theme;
  };
}
