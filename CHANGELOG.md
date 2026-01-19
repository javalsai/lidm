<!-- Unsepcified dates shall be in CET/CEST -->

<!-- Add only very relevant changes, bottom to top -->
<!-- By "very relevant" I mean big features or something manual packagers should know, like leftover files -->
<!-- Once a release would be opened, group the last bunch of dangling changes, add release version as header and its date -->

- Added a changelog.
- Finally add proper (experimental) xorg support.
- Systemd service install path changed to `/usr/local/lib/systemd`, there could be a leftover file at `/etc/systemd/system/lidm.service`.
