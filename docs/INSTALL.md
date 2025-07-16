# Table of Contents

- [Installing from Source](#installing-from-source)
- [AUR](#aur)
- [Nix Flake](#nix-flake)
- [Nix Module](#nix-module)

> [!CAUTION]
> I encourage you to read the manual installation steps to understand what will get installed in your computer by this package.

# Installing from Source

Firstly, you'll need to build the package, this also includes man pages, default config, themes and other files you might need.

To build it you only need to have some basic packages (should come pre-installed in almost all distros): `make`, `gcc` or another `gcc`ish compiler, and libpam headers. If it builds, it surely works anyways.

```sh
git clone https://github.com/javalsai/lidm.git
cd lidm
make # 👍
```

> [!NOTE]
> There's pre-built binaries on the [releases tab](https://github.com/javalsai/lidm/releases) too.

Then you can install the files onto your filesystem with:

```sh
make install
```

And additionally, to install service files (start-up behavior). <sup>[more docs](./assets/services/README.md)</sup>

```sh
# automatically detects your init system
# and install service file (for tty7)
make install-service

# or if you don't like autodetection
make install-service-systemd # systemd
make install-service-dinit # dinit
make install-service-runit # runit (/etc/sv)
make install-service-runit-etc # runit (/etc/runit/sv)
make install-service-openrc # openrc
make install-service-s6 # s6 (/etc/sv)
make install-service-s6-etc # s6 (/etc/s6/sv)

#  For runit and s6, some distros (e.g. Artix) like to put it in /etc/<init>/sv
# to better isolate their packages while other distros (e.g. Void) just put it
# in /etc/sv
```

# AUR

[AUR packages](https://aur.archlinux.org/packages?K=lidm&SeB=n) will automatically install most files.

> [!CAUTION]
> [service files](./assets/pkg/aur#services) have to be manually installed by now.

# Nix Flake

You can install by doing

```sh
nix profile install github:javalsai/lidm
```

or try it out without installing by:

```sh
nix run github:javalsai/lidm
```

> [!CAUTION]
> This doesn't include [service files](./assets/pkg/aur#services) neither

# Nix Module

<details>
<summary>Sidenote</summary>

The nix module lacks on several aspects, if you know much about nix and know
how to improve this package, feel free to open an issue or a PR to help. The
nix package maintainer position is open too.

</details>

Lidm includes a nix module in `assets/pkg/nix/module.nix` that you can add
(along the included nix files) and import in your `configuration.nix`.

Once imported you'll need to add:

```nix
services.displayManager.enable = true;
systemd.services.lidm.enable = true;
```

Optionally, you can configure it setting `services.lidm.config`. You can either
pass:

- A string to copy the config from a theme in `themes/` with said name
  (**name**, e.g `"cherry"`).
- An attribute tree with the same names as the config file, e.g:

```nix
with config.lidm.keysEnum; {
    strings = {
        f_poweroff = "custom_poweroff";
    };

    behavior = {
        include_defshell = true;
        source = [
            "path1"
            "path2"
        ];
    };

    functions = { poweroff = F1; };

    # etc...
};
```

> _it's not necessary to cover all keys and anything can be put there, even if it's not valid config_

> [!NOTE]
> [service files](./assets/pkg/aur#services) **are** included and enabled
