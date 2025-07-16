# AUR Packages

These files are just for reference, I'll manually edit and publish them, at least until I automate it with github actions (like updating version automatically on a release).

There are three packages that follow standard conventions:

- [`lidm`](https://aur.archlinux.org/packages/lidm): Builds latest release (manually updated per release basis)
- [`lidm-bin`](https://aur.archlinux.org/packages/lidm-bin): Fetches latest release binary (compiled by GitHub Actions, also updated per release)
- [`lidm-git`](https://aur.archlinux.org/packages/lidm-git): Fetches latest commit and builds it (should be updated automatically)

> \[!IMPORTANT]
> None of those packages include the service files. [You have to do this yourself](../../services/README.md).
