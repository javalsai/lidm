# AUR Packages
These files are just for reference, I'll manually edit and publish them, at least until I automate it with github actions (like updating version automatically on a release).

There are three packages that follow standard conventions:
* `lidm`: Builds latest release (manually updated per release basis)
* `lidm-bin`: Fetches latest release binary (compiled by GitHub Actions, also updated per release)
* `lidm-git`: Fetches latest commit and builds it (should be updated automatically)

> [!IMPORTANT]
> None of those packages include the service files, I'm considering automatically detecting it on the package function or providing it as separate services (standard practice again).
>
> Depending on how good these packages go (my first packages :P) I'll make these ones too.

## Services
Summary of what to do to install service files anyways:
* systemd: Copy `assets/services/systemd.service` to `/etc/systemd/system/lidm.service` and enable the service (`systemctl enable lidm`)
* dinit: Copy `assets/services/dinit` to `/etc/dinit.d/lidm` and enable the service `dinitctl enable lidm`
