# Service Files

This folder contains the files necessary to set up lidm on start up for the supported init systems, all of them are configured for tty7.

If you're using lidm from a packaged source, this should be included with the package. Only read this for manual installations, if you are packaging or if your package exceptionally doesn't include them.

If you don't know what a init system is, you're certainly using `systemd`.

There's make scripts to automatically copy the service files to the proper locations, you just have to run `make install-service-$INIT` (or `make install-service-$INIT-etc`). `make install-service` will attempt to detect the init system in use and install for it.

The manual steps for installation are:

## Systemd

- Copy `systemd.service` to `/usr/local/lib/systemd/system/lidm.service` (if the directory doesn't exist, create it first) or `/usr/lib/systemd/system/lidm.service` (if you wish to install along your system files).
- To enable it you can run `systemctl enable lidm`

## Dinit

- Copy `dinit` to `/etc/dinit.d/lidm`
- To enable it, run `dinitctl enable lidm`

## Runit

Your runit service path can be either `/etc/runit/sv` or `/etc/sv`.

- Copy `runit/` to `runit-path/lidm/`
- Add the service with `ln -s runit-path/lidm /run/runit/service`
- And to enable it `sv enable lidm`

## OpenRC

- Copy `openrc` to `/etc/init.d/lidm`
- Enable the service with `rc-update add lidm`

## S6

Your S6 service path can be either `/etc/s6/sv` or `/etc/sv`.

- Copy `s6/` to `s6-path/lidm/`
- Add the service with `s6-service add default lidm`
- Reload the database with `s6-db-reload` (you might have to run this every time the service file changes)

> [!WARNING]
> Make sure to disable any other service that might run on tty7, such us lightdm or most display managers out there.
