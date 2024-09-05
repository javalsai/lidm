# Index

* [Index](#index)
* [Installing from Souce](#installing-from-source)
* [AUR](#aur)
* [Nix Flake](#nix-flake)

> \[!CAUTION]
> I encourage you to read the manual installation steps to understand what will get installed in your computer by this package.

# Installing from Source

Firstly, you'll need to build the package, this also includes man pages, default config, themes and other files you might need.

To build it you only need to have some basic packages (should come pre-installed in almost all distros): `make`, `gcc` or another `gcc`ish compiler, and libpam headers. If it builds, it surely works anyways.

```sh
git clone https://github.com/javalsai/lidm.git
cd lidm
make # 👍
```

> \[!NOTE]
> Theres pre-built binaries on the [releases tab](https://github.com/javalsai/lidm/releases) too.

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
make install-service-runit # runit
make install-service-openrc # openrc
make install-service-s6 # s6
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
