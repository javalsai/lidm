# lidm
Lidm is a really light display manager made in C, highly customizable and held together by hopes and prayers 🙏

![demo image](assets/lidm.png)
> this is shown as in a terminal emulator, actual linux console doesn't support as much color and decorations

## Features
* Builds fast af
* Works everywhere you can get gcc to compile
* Fast and possibly efficient
* Fully customizable, from strings to colors (I hope you know ansi escape codes), to action buttons
* Automatically detects xorg and wayland sessions, plus allowing to launch the default user shell (if enabled in config)

## WIP
* Save last selection
* Config parsing, it's fully customizable, but everything hardcoded for now :)
* Long sessions, strings, usernames, passwords... they will just overflow or f*ck your terminal, I know it and I don't know if I'll fix it.

## Forget it
* Any kind of arguments
* UTF characters, I'm using `strlen()` and treating characters as per byte basis, UTF-8 chars might work or not

> [!CAUTION]
> (they should add `> [!DISCLAIMER]` fr) I wrote this readme with the same quality as the code, behing this keyboard there's half a brainrotcell left writing what it remembers of this program, so don't take this to seriously, I'm typing as I think without filter lol, but the program works, or should. Also, about any "TODO" in this readme (or the code), I didn't forget finishing it, I actually don't care

# Backstory
I went into summer travel to visit family with an old laptop that barely supports x86_64, and ly recently added some avx2 instructions I think (I just get invalid op codes), manually building (any previous commit too) didn't work because of something in the `build.zig` file, so out of boredom I decided to craft up my own simple display manager on the only language this thing can handle, ✨C✨ (I hate this and reserve the right for the rust rewrite, actually solid). I spedrun it, basically did in in 3 days while touching *some* grass (:o), and I'm bad af in C, so this is spaghetti code on another level. I think it doesn't do almost nothing unsafe, I mean, I didn't check allocations and it's capable of reallocating memory until your username uses all memory and crashes the system due to a off by 1 error, but pretty consistent otherwise (probably).

The name is just ly byt changing "y" with "i", that had a reason but forgot it, (maybe the i in *simple*), so I remembered this sh*tty laptop with a lid, this thing is also a display manager (dm, ly command is also `ly-dm`), so just did lidm due to all that.
![think gif](https://i.giphy.com/media/v1.Y2lkPTc5MGI3NjExcTFzaGVmb3VjN3FnOXV6OG9rMG91a2QwM3c0aDV0NWpoZjFzNDEwayZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/d3mlE7uhX8KFgEmY/giphy.gif) <!--gif's likely broken-->

Btw, this laptop is so bad that I can't even render markdown in reasonable time, I'll just push this and fix render issues live :)

# Index
(TODO, VSC(odium) does this automatically, I'm on nvim rn 😎)

# Requirements
* A computer with unix based system.
* That system should have the resources neccessary for this program to make sense (sessions, users...)
* A compiler (optional, you can compile by hand, but I doubt you want to see the code)
* Make (also optional, but does things atomatically, make sure `gcc` and `mkdir -p` work as expected)

# Compiling
```sh
make # 👍
```

# Configuring
Ugh, config will be a straigh copy of config defaults to `/etc` I think, there's no config yet :P and you need to enable the service, just do what ly does (I'm doing this on dinit, all init systems should be supported).

# Contributing
Don't do this to yourself, but you can ofc, you can also fork or whatever (make sure to comply with GNU's GPLv3), but I want to do the rust rewrite 😡

# Recommendations
Hope you didn't expect actual project recommendations, but these songs are 🔥
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

Oh, an actual recommendation, if you don't like a element you can change the fg color of it to be the same as the bg.

Also (this isn't quite a recommendation lol), the default fg style shoulddisable decorators set up in other elements (cursivem underline... it's just adding 20 to the number btw, so if cursive is 4 (iirc), disabling it is 24).

Congrats if you've managed to read through all this, wrote all this in exactly 30min.