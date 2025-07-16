[![GitHub license](https://badgen.net/github/license/javalsai/lidm)](https://github.com/javalsai/lidm/blob/master/LICENSE)
[![GitHub branches](https://badgen.net/github/branches/javalsai/lidm)](https://github.com/javalsai/lidm)
[![Latest Release](https://badgen.net/github/release/javalsai/lidm)](https://github.com/javalsai/lidm/releases)

# LiDM

LiDM is a really light unix [login manager](https://en.wikipedia.org/wiki/Login_manager) made in C, highly customizable and held together by hopes and prayers 🙏.

LiDM is like any [Display Manager](https://en.wikipedia.org/wiki/X_display_manager) you have seen such as SDDM or GDM but without using any X.org graphics at all. Instead being a purely [text based interface](https://en.wikipedia.org/wiki/Text-based_user_interface) inside your terminal/TTY.

![demo gif](assets/media/lidm.gif)

> _shown as in a featured terminal emulator, actual linux console doesn't support as much color and decorations_

> _however, all colors and strings are fully customizable_

## Features

- Simple as C, you only need a C compiler and standard unix libraries to build this.
- Fully customizable, from strings, including action keys, to colors (I hope you know ansi escape codes)
- Automatically detects xorg and wayland sessions, plus allowing to launch the default user shell (if enabled in config)
- Starts with many init systems (systemd, dinit, runit, openrc and s6).

# Table of Contents

- [Ideology](#ideology)
- [Usage](#usage)
  - [Arguments](#arguments)
  - [Program](#program)
- [Requirements](#requirements)
- [Installation](#installation)
- [Configuring](#configuring)
- [PAM](#pam)
- [Contributing](#contributing)
- [Inspiration](#inspiration)
- [Contributors](#contributors)

# Ideology

We all know that the most important thing in a project is the ideology of the author and the movements he wants to support, so [**#stopchatcontrol**](https://stopchatcontrol.eu).

[ ![stopchatcontrol](https://stopchatcontrol.eu/wp-content/uploads/2023/09/1-1-1024x1024.png) ](https://stopchatcontrol.eu)

> _there's also a [change.org post](https://www.change.org/p/stoppt-die-chatkontrolle-grundrechte-gelten-auch-im-netz)._

# Usage

### Arguments

If a single argument is provided (don't even do `--` or standard unix parsing...), it switches to said tty number at startup. Used (at least) by most service files.

### Program

Base (mostly intuitive):

- Use arrow keys to navigate the inputs and type over any of them to override the default value.
- Enter will just attempt to login.
- If you are focused on an edited input, horizontal arrow keys will attempt to move across the text just as expected.

On top of that:

- Using the horizontal arrow keys if the focused input is not in text mode or the movement would overflow the input. It will try to change in such direction the option of session or the user.
- Pressing <kbd>ESC</kbd> and then horizontal arrows will force to change the option of the focused input even if it's in edit mode.
- Editing an option on a user or a shell session will put you in edit mode appending after the original value.

# Requirements

- Make (Also optional, but does things automatically, make sure `gcc` and `mkdir -p` work as expected).
- A compiler like `cc`, `gcc` or `clang`. Make sure to use the desired `CC=<compiler>` on the `make` command.
- PAM library, used for user authentication, just what `login` or `su` use internally. Don't worry, it's surely pre-installed.

# Installation

Check the [installation guide](./docs/INSTALL.md) to use your preferred installation source.

<details>

<summary>Packagers read here!!</summary>

If you are a package maintainer or are willing to become one, please read [the packagers guide](./docs/PACKAGERS.md).

</details>

# Configuring

Copy any `.ini` file from [`themes/`](./themes/) (`default.ini` will always be updated) to `/etc/lidm.ini` and/or configure it to your liking. You can also set `LIDM_CONF` environment variable to specify a config path.

The format attempts to mimic the TOML format. Escape sequences like `\x1b` are allowed as well as comments and empty lines.

Colors are gonna be put inside `\x1b[...m`, if you don't know what this is check [an ANSI table](https://gist.github.com/JBlond/2fea43a3049b38287e5e9cefc87b2124). Mind that `\x1b` is the same as `\e`, `\033` and several other representations.

> [!NOTE]
> The default `fg` style should disable decorators set up in other elements (cursive, underline...). It's just adding 20 to the number, so if an underline is 4, disabling it is done with 24.

> [!TIP]
> If you don't like seeing an element, you can change the fg color of it to be the same as the bg, making it invisible.

# PAM

If your distribution does not use the standard PAM service name `login` (`/etc/pam.d/login`) for its PAM services or if you want to use another PAM file, simply set the `LIDM_PAM_SERVICE` env variable to your PAM service name.

When the env variable is empty it defaults to the `login` PAM service or whatever fallback your distribution packager has defined during compilation.

# Contributing

If you want to contribute check the [contribution guide](docs/CONTRIBUTING.md).

# Inspiration

This project was started after facing some issues building [ly](https://github.com/fairyglade/ly) on an ancient laptop, the UI is heavily inspired by it.

For this reason the project's philosophy is to be simple and minimal, such that even prehistoric hardware is capable of running it.

I forgot what exactly the name came from, but it surely was a mix of a few things so:

- Obviously it's inspired by `ly`. `ly-dm` leads to "lydm".
- Wow make "lydm" simple with a "y" → "i" transformation.
- Associate it with the "i" in s**i**mple and other display managers like **Li**ghtDM.
- And the **la**ptop this project started in has a **lid**.

# Contributors

[![GitHub Contributors](https://contrib.rocks/image?repo=javalsai/lidm&max=20)](https://github.com/javalsai/lidm/graphs/contributors)

[killertofus](https://github.com/killertofus), [deadvey](https://github.com/deadvey), [grialion](https://github.com/grialion/), cerealexperiments\_, [antiz96](https://github.com/Antiz96), [rmntgx](https://github.com/rmntgx) and [more...](https://github.com/javalsai/lidm/graphs/contributors)

With their big or small contributions, every commit has helped in its own way and encouraged me to keep putting my soul into this.

---

🌟 Finally, consider starring this repo [or...](https://www.reddit.com/r/github/comments/1l2mchg/is_this_allowed) 🔪

![star-history](https://api.star-history.com/svg?repos=javalsai/lidm&type=Date)
