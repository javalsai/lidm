<!-- Unspecified dates shall be in CET/CEST -->

<!-- Add only very relevant changes, bottom to top -->
<!-- By "very relevant" I mean big features or something manual packagers should know, like leftover files -->
<!-- Once a release would be opened, group the last bunch of dangling changes, add release version as header and its date -->

# 2.0.1

- source and header files can be nested in `src/` and `include/`
- lidm now calls bash for logging in and sourcing profile files, other shells can be configured
- a good PATH default is fetched from `confstr`
- debug logs are now unbuffered

# 2.0.0

- Most stuff (most of `/etc` being a notable exception) now installs to `/usr/local` by default, check [`docs/PACKAGERS.md`](./docs/PACKAGERS.md).
- Added a changelog.
- Finally add proper (experimental) xorg support.
- Systemd service install path changed to `/usr/local/lib/systemd`, there could be a leftover file at `/etc/systemd/system/lidm.service`.
