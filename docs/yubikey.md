# Yubikeys

Quick reference explaining how yubikeys work for now.

# Enable

Yubikeys are disabled by default, to enable them activate a keybinding for it (`[functions] fido`) in the config file.

Note that pressing this configured keybinding has no difference from trying to log in with an empty password, there's virtually no difference.

`pam_u2f` must be configured with a registered key (`pamu2fcfg`).

# Extra

All my yubikey knowledge comes from the [pr that implemented this](https://github.com/javalsai/lidm/pull/89), please refer to it for extra details. Contributions to this documentation are welcome (explaining more in detail, potential issues... really anything that improves this).

Allegedly this pam module configuration should work:

```pam
#%PAM-1.0

auth sufficient pam_u2f.so cue
auth       requisite    pam_nologin.so
auth       include      system-local-login
account    include      system-local-login
session    include      system-local-login
password   include      system-local-login
```

Also, I recommend giving the [arch wiki](https://wiki.archlinux.org/title/YubiKey) a read anyways.
