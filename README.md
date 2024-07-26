# LiDM
LiDM is a really light display manager made in C, highly customizable and held together by hopes and prayers 🙏.

![demo image](assets/media/lidm.gif)
> *This is shown as in a terminal emulator, actual linux console doesn't support as much color and decorations.*

> *But all colors and strings are fully customizable.*

# Index
(TODO, VSC(odium) does this automatically, I'm on nvim rn 😎).

## Features
* Builds fast af.
* Works everywhere you can get gcc to compile.
* Fast and possibly efficient.
* Fully customizable, from strings, through action keys, to colors (I hope you know ansi escape codes)
* Automatically detects xorg and wayland sessions, plus allowing to launch the default user shell (If enabled in config)
* Starts with many init systems eg: systemd and dinit.

## WIP
* Desktop's file `TryExec` key.
* Save last selection.
* Show/hide passwd switch.
* Long sessions, strings, usernames, passwords... they will just overflow or fuck your terminal, I know it and I don't know if I'll fix it.

## Forget it
* UTF characters, I'm using `strlen()` and treating characters as per byte basis, UTF-8 chars might work or not actually, might fix it by replacing some `strlen()` with a utflen one.

> [!CAUTION]
> (They should add `> [!DISCLAIMER]` fr) I wrote this readme with the same quality as the code, behing this keyboard there's half a brainrotcell left writing what it remembers of this program, so don't take this to seriously, I'm typing as I think without filter lol, but the program works, or should. Also, about any "TODO" in this readme (Or the code), I didn't forget finishing it, I actually don't care. And, references to ~~code~~ **anything** are likely to be outdated, check the commit history to know what I was refering to exactly if you care.

# Ideology
We all know that the most important thing in a project is the ideology of the author and the movements he wants to support, so [**#stopchatcontrol**](https://stopchatcontrol.eu).
<!-- doubt markdown will allow me to render this properly -->
[ ![stopchatcontrol](https://stopchatcontrol.eu/wp-content/uploads/2023/09/1-1-1024x1024.png) ](https://stopchatcontrol.eu)
> *there's also a [change.org post](https://www.change.org/p/stoppt-die-chatkontrolle-grundrechte-gelten-auch-im-netz).*

# Backstory
I travelling in the summer to visit family with an old laptop that barely supports x86_64, and ly recently added some avx2 instructions I think (I just get invalid op codes), manually building (any previous commit too) didn't work because of something in the `build.zig` file, so out of boredom I decided to craft up my own simple display manager on the only language this thing can handle, ✨C✨ (I hate this and reserve the right for the rust rewrite, actually solid).

I spedrun it, basically did in in 3 days on the same couch on [unhelty back positions (even worse)](https://i.redd.it/4bkje8amisu61.png) while touching *some* grass (:o), and I'm bad af in C, so this is spaghetti code on **another** level. I think it doesn't do almost anything unsafe, I mean, I didn't check allocations and it's capable of reallocating memory until your username uses all memory and crashes the system due to a off by 1 error, but pretty consistent otherwise (Probably).

The name is just ly but changing "y" with "i", that had a reason but forgot it, (maybe the i in *simple*), so I remembered this sh*tty laptop with a lid, this thing is also a display manager (Dm, ly command is also `ly-dm`), so just did lidm due to all that.

![think gif](https://i.giphy.com/media/v1.Y2lkPTc5MGI3NjExcTFzaGVmb3VjN3FnOXV6OG9rMG91a2QwM3c0aDV0NWpoZjFzNDEwayZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/d3mlE7uhX8KFgEmY/giphy.gif) <!--gif's likely broken-->

FYI, this laptop is so bad that I can't even render markdown in reasonable time, I'll just push this and fix render issues live :).

# Requirements
* A computer with unix based system.
* That system should have the resources neccessary for this program to make sense (Sessions, users...).
* A compiler (optional, you can compile by hand, but I doubt you want to see the code).
* Make (Also optional, but does things atomatically, make sure `gcc` and `mkdir -p` work as expected).

# Usage
Regarding arguments: If a single argument is provided (don't even do `--` or standard parsing...), it passes that argument to `chvt` on startup, used (at least) by the dinit service.

On top of pure intuition:
* You can change focus of session/user/passwd with up/down arrows.
* In case arrow keys do nothing on the focused input (Either is empty text or doesn't have more options), it tries to change session and if there's only one session it changes user.
* Typing anything will allow to put a custom user or shell command too, you can use arrow keys to move.
* ESC and then left/right arrows will force to change the option of the focused input, useful if you're editing the current input and arrow keys just move.
* Editing a predefined option on a user or a shell session, will put you in edit mode preserving the original value, other cases start from scratch.

# Building from source
```sh
git clone https://github.com/javalsai/lidm.git
cd lidm
make # 👍
```

# Installation
* You can put the compiled binary anywhere you want tbh, you can even setuid it if you want to run it with any user, but code's not too safe 😬.
* Prepare the [configuration](#configuring).
```sh
# place binary in /usr/bin and copy
# default theme to /etc
make install
```
* Make a service file for this if you want it on launch, just check how ly does it, this works pretty much the same way.
```sh
# automatically detects init system
# and installs service file (for tty7)
make install-service

# There's manual scripts too:
make install-service-systemd # systemd
make install-service-dinit # dinit
```


# Disabling other DM's with systemd
You may want to disable your current Display Manager for instance:
```sh
sudo systemctl disable sddm         # Disables SDDM (KDE's dm)
sudo systemctl disable lightdm      # Disables lightDM (popular lightweight DM)
sudo systemctl disable gdm          # Disables GDM (Gnomes DM)
sudo systemctl disable ly           # Disables LYDM (tui dm)
```

# Configuring
Copy any `.ini` file from [`themes/`](./themes/) (`default.ini` will always be updated) to `/etc/lidm.ini` and/or configure it to your liking. Also, don't place empty lines (for now).

Also configurable colors are just gonna be put inside `\x1b[...m`, ofc you can add an m to break this and this can f* up really bad or even make some nice UI effect possible, but please don't, you should also be able to embed the `\x1b` byte in the config as I won't parse escape codes, I think that the parser is just gonna grab anything in the config file from the space after the `=` (yes, I'ma enforce that space, get good taste if you don't like it) until the newline, you can put any abomination in there.

The default fg style should disable decorators set up in other elements (cursive, underline... it's just adding 20 to the number btw, so if cursive is 4 (iirc), disabling it is 24).

# Contributing
Don't do this to yourself, but you can ofc, you can also fork or whatever (make sure to comply with GNU's GPLv3), but I want to do the rust rewrite 😡 (after it, I'll leave this around in case somebody with ancient hardware needs it, or if somebody wants to port it for a microwave...)

This is also GPLv3 bcs I was too lazy to look for the "do anything I don't care" license, also it's funny legally trapping people into FOSS. **EDIT:** just realized ly uses it and I coudl've just copied it :(, idk how bad GPLv3 respect to changing license and I *could* rebase but that never goes well, so I'll consult my lawyer (chatGPT) later.

# Contributors
Special thanks to:
* Javalsai, Maintainer and everyone's second favourite penguin!!!
* DeaDvey, the most awesomest of all, did some pretty HARDCORE gramer cheking.
* KillerTofus, [made the AUR package](https://github.com/javalsai/lidm/pull/2)! Saved me from reading the Arch Wiki 💀.
* ChatGPT, in times of slow laptops where pages take ages to load, a single tab connected to a bunch of burning cloud GPUs feeding corporate hype is all you need to get quick answers for your questions, as long as you know how to filter AI crap ofc.
* [My lack of gf](https://www.instagram.com/reel/C8sa3Gltmtq/?igsh=NO_ZUCKERBERG__FUCK_OFF_YOUR_FINGERPRINGTING_QUERY__OBVIOUSLY_STANDS_FOR_INSTAGRAM_SHARE_ID_FFS), can't imagine this project being possible if somebody actually cared about me daily.

# Milestones
* If this ~~video~~ **repo** gets to 100 ~~likes~~ **stars**, I'll make an arch aur package and properly document installation on multiple init systems.
* (I might also make a nix flake if yall nix nerds ask me for it).

# Recommendations
Hope you didn't expect actual project recommendations, but these songs are 🔥:
* "Sixpence None the Richer - Kiss Me"
* "Avril Lavigne - Complicated"
* "Shawn Mendes - There's Nothing Holdin' Me Back"
* "Rio Romeo - Nothing's New"
* "ElyOtto - SugarCrash!"
* "The Cranberries - Sunday"
* "Goo Goo Dolls - Iris"
* "Em Beihold - Numb Little Bug"
* "MAGIC! - Rude"
* "The Cranberries - Zombie"
* "Natalie Imbruglia - Torn"
* "Alec Benjamin - I Sent My Therapist To Therapy"
* "The Neighbourhood - Sweater Weather"
* "Cascada - Everytime We Touch" (potentially the sped up versoin if better)
* "Mitski - My Love Mine All Mine"

Oh, an actual recommendation, if you don't like a element you can change the fg color of it to be the same as the bg.

Congrats if you've managed to read through all this.
