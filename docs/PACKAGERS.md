This is a guide listing all possible options packagers have to tweak behavior of lidm and the extra stuff to package.

# Components

If you want to package lidm for a distribution you have to package the binary and:

- Man pages ([`../assets/man/`](../assets/man/))
- Service files ([`../assets/services/`](../assets/services/))
- PAM (see [#preprocessor-defines](#preprocessor-defines))
- And optionally you can include some default themes in `/usr` ([`../themes/`](../themes/))

# Preprocessor Defines

Most of the behavior that can be tweaked using preprocessor `#define`s, to include some simply add `CPPFLAGS+='-D{{name}}={{value}}'` to your `make` command. e.g:

```sh
make \
    CPPFLAGS+='-DSESSIONS_XSESSIONS=\"/var/empty\"' \
    CPPFLAGS+='-DSESSIONS_WAYLAND=\"/var/empty\"'
```

The list of possible `#define`s is:

| Name                       | Default                               | Description                                                                | Env Override?            |
| -------------------------- | ------------------------------------- | -------------------------------------------------------------------------- | ------------------------ |
| `PAM_SERVICE_FALLBACK`     | `"login"`                             | Name of the default PAM module to use. Defaults to the distro's `"login"`. | Yes (`LIDM_PAM_SERVICE`) |
| `SESSIONS_XSESSIONS`       | `"/usr/share/xsessions"`              |                                                                            | No                       |
| `SESSIONS_XSESSIONS_LOCAL` | `"/usr/local/share/xsessions"`        |                                                                            | No                       |
| `SESSIONS_WAYLAND`         | `"/usr/share/wayland-sessions"`       |                                                                            | No                       |
| `SESSIONS_WAYLAND_LOCAL`   | `"/usr/local/share/wayland-sessions"` |                                                                            | No                       |
| `LIDM_CONF_PATH`           | `"/etc/lidm.ini"`                     | Path of the default configuration.                                         | Yes (`LIDM_CONF`)        |

# Other Build Settings

## Compiler

Lidm attempts to support being built by `gcc` and `clang` with preference over the first. However, if you wish to take advantage of LLVM's optimizations over GNU's you can change the compiler with `make CC=clang` or try any other compiler.

## Compiler Flags

Compiler flags should be passed with `CFLAGS`, its `-O3 -Wall` by default so adding anything will overwrite this.

## Build Metadata

`lidm -v` outputs some information about the build version, this can be weaked with `INFO_GIT_REV` and `INFO_BUILD_TS`, by default they are:

```make
INFO_GIT_REV?=$$(git describe --long --tags --always || echo '?')
INFO_BUILD_TS?=$$(date +%s)
```

But this can be changed by just passing those env variables, for example, in the case git is not applicable in the build environment of the package.

## Target Directory

`DESTDIR` can be used to for installation recipes to install on alternative root directories. Along with `PREFIX` (defaults to `/usr/local`, packaging this you'll probably want to make it `/usr`) for systems which don't use the common `/usr` structure. e.g. `make install DESTDIR=$out PREFIX=`

`/etc` is sometimes in that `PREFIX` and sometimes not, be careful.

```txt
$ fd -t f . --base-directory $out
bin/lidm
etc/lidm.ini
share/man/man1/lidm.1
share/man/man5/lidm-config.5
```

# Build Recipes for Packaging

To ease most of the installation process there's several `make` recipes to install man pages and service files, I encpurage you to check their source yourself, but:

- `make` / `make lidm`: Builds the app binary.
- `make install`: Attempts to install the binary, place a default config file in `/etc/lidm.ini` and install the man pages.
- `make install-service-(systemd|dinit|runit|openrc|s6)(|-etc)`: Just check the source, service files for different init systems and `-etc` variants for alternative paths. You might need `FORCE=1` in the environment if you are packaging for other init system or scripting.
- `make print-version`: Outputs the current version in the `Makefile` for scripts that might want to extract that info.

You can choose to use these packages or create your own service files / etc. There's are merely suggestions on what to use.
