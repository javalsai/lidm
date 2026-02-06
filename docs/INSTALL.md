# Table of Contents

- [Packages](#packages)
  - [Void Linux](#void-linux)
  - [Fedora](#fedora)
  - [AUR](#aur)
  - [Nix Flake](#nix-flake)
  - [Nix Module](#nix-module)
- [Installing from Source](#installing-from-source)

# Packages

Lidm relies on its community to package lidm for distributions and make the installation easier for everyone. Please raise any issues on the packages themselves and not the source repository.

If you packaged lidm for any distribution not listed here or keep your own version of an inactive package, feel free to contact me to be added here. (could be a GitHub discussion or an email if you don't use GitHub)

These packages just install lidm, once installed you have to enable it. It depends on your init system, but if you're using systemd (most likely), first find your currently active display manager (if any, examples are `sddm`, `gdm`, `ly`) and disable it:

```sh
systemctl disable <display manager name here>
```

And then enable lidm:

```sh
systemctl enable lidm
```

Lidm now should show on the next boot!

If you are on a TTY and don't wish to reboot, you should be able to use `--now` with both commands. It's likely to kill your current desktop environment if it was launched from it.

## Void Linux

Lidm can be installed from the official Void Linux packages:

```sh
xbps-install lidm
```

To enable the service that starts on boot:

```sh
ln -s /etc/sv/lidm /var/service/
```

It is recommended to disable any other display managers that are still enabled:

```sh
touch /etc/sv/lightdm/down # replace lightdm with your previous display manager (e.g., sddm, gdm)
```

## Fedora

Thanks to @KernelFreeze there's a COPR repo available at <https://copr.fedorainfracloud.org/coprs/celestelove/lidm/> to install lidm. (<https://github.com/javalsai/lidm/discussions/92>)

## AUR

There are several [AUR packages](https://aur.archlinux.org/packages?K=lidm&SeB=n) for most things. You only need to install one of the following:

- `lidm`
- `lidm-bin`
- `lidm-git`

Each of those depend on `lidm-service` which is provided by `lidm-systemd` and `lidm-dinit` (you'll have to pick one). I maintain both but I'm open to anyone that actually uses systemd to take the first one.

There's packagers needed for openrc, s6 and runit. If you know how to make AUR packages, feel free to take any of those if you use each init system.

## Nix Flake

You can install by doing

```sh
nix profile install github:javalsai/lidm
```

or try it out without installing by:

```sh
nix run github:javalsai/lidm
```

> [!CAUTION]
> This doesn't include [service files](../assets/pkg/aur#services)

## Nix Module

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
> [service files](../assets/pkg/aur#services) **are** included and enabled

# Installing from Source

Firstly, you'll need to build the package, this also includes man pages, default config, themes and other files you might need.

To build it you only need to have some basic packages (should come pre-installed in almost all distros): `make`, `gcc` or another `gcc`ish compiler, and libpam headers. If it builds, it surely works anyways.

```sh
git clone https://github.com/javalsai/lidm.git
cd lidm
make # 👍
```

> [!NOTE]
> There are pre-built binaries on the [releases tab](https://github.com/javalsai/lidm/releases) too.

Then you can install the files onto your filesystem with:

```sh
make install
```

This will install mostly into `/usr/local` by default, this path is mostly unused nowadays (systemd can load services from it but man usually doesn't find manual pages in there) and could feel like lidm didn't install at all. You can add `PREFIX=/usr` to install along the rest of your system packages if you wish.

And additionally, to install service files (start-up behavior). <sup>[more docs](../assets/services/README.md)</sup>

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
