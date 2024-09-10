# Service Files

This folder contains the files necessary to set up lidm on start up for the supported init systems, all of them are configured for tty7.

If you don't know what a init system is, you're certainly using `systemd`.

There's make scripts to automatically copy the service files to the proper locations, you just have to run `make install-service-$INIT`. `make install-service` will attempt to detect the init system in use and install for it.

The manual steps for installation are:

## Systemd

* Copy `systemd.service` to `/etc/systemd/system/lidm.service`
* To enable it you can run `systemctl enable lidm`

## Dinit

* Copy `dinit` to `/etc/dinit.d/lidm`
* To enable it, run `dinitctl enable lidm`

## Runit

* Copy `runit/` to `/etc/runit/sv/lidm/`
* Add the service with `ln -s /etc/runit/sv/lidm /run/runit/service`
* And to enable it `sv enable lidm`

## OpenRC

* Copy `openrc` to `/etc/init.d/lidm`
* Enable the service with `rc-update add lidm`

## S6

* Copy `s6/` to `/etc/s6/sv/lidm/`
* Add the service with `s6-service add default lidm`
* Reload the database with `s6-db-reload` (you might have to run this every time the service file changes)

> \[!WARNING]
> Make sure to disable any other service that might run on tty7, such us lightdm or most display managers out there.
