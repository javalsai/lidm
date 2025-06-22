# Introduction

This file aims to explain the basic project structure and some code conventions
and patterns used for contributors. If you plan on contributing a considerable
amount of code, please read this thoroughly to keep the project behavior
consistent.

Even if your changes are not very related to the code, this guide can help to
understand how they can be related to the rest of the project and what side
changes it can require.

# Structure

The file structure is very simple, you have `.c` files in `src/` with their
header `.h` counterpart in `include/`, they are listed in the `Makefile` and
built all together on the same layer.

Each file contains functions aimed to provide some specialized behavior, they
tend to follow a common prefix (like `vec_` for vector functions) to avoid name
collisions with themselves.

## Important Files

The `main.c` of course provides the entry point of the program, then each file
has special functionality, but a special one is `util.h`, is linked with almost
every file and provides some shared utilities to dealing with UTF-8, dynamic
Vectors and other stuff.

`log.c` is also an important file as it provides logging utilities for
debugging, there's no need to deal with it's initialization, it behaves just
like standard printing utilities but integrated into the configured logfile.

# Debugging

The log module can be easily used by just setting up the `LIDM_LOG`
environmental variable to the logs output path.

# Header Files

But what if you create a new file? It's important that the definitions are only
evaluated once by the compiler, even if they are included by several files. For
this, you can use this simple trick (assume this is for a file named
`mylib.h`):

```h
#ifndef MYLIBH_
#define MYLIBH_

// library contents
// ...

#endif
```

It's also a good idea to include brief comments above functions if it's not
evident what they do and name all parameters.

# Nullability Checks

Nullability checks are not really enforced in the code but it's never a bad
idea to include them, however, `gcc` doesn't support them under certain
conditions.

For this you can use `NULLABLE`, `NNULLABLE` and `UNULLABLE` macros from
`"macros.h"` to conditionally include `_Nullable`, `_Nonnull` and
`_Null-unspecified` respectively.

# Handling & Support

Every function should properly handle allocation failures (avoid them with
stack arrays where possible), support UTF-8 strings, use the `log` module
instead of printing to stdout (messing with the state of the ui) and free all
lost memory (if it can be lost before reaching the final stage where it execs
another program, I'm fine with not freeing memory still reachable at the end).

# Code format, style and linting

Be reasonable and follow `clang-format` and `clang-tidy` rules. `clang-tidy`
can be quite pedantic with the current primitive rules, so feel free to use
`LINTIGNORE` or discuss if said rule should be enforced in first place.

Also avoid grammar typos and for shell scripts use `shellcheck`, you can run
this checks with `make pre-commit` if you have the required tools.

# Documentation

Please, also check if there's documentation related to the feature/changes you
are implementing, could be on markdown or manpages, if there is and you have
some spare time, it's really helpful to update it; although not enforced.
