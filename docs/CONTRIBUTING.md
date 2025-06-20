# Contributing

Contributions are welcome! Here's how you can help:

- [Contributing code](#code)
- [Reporting issues](#issues)

## Code

For small fixes or incremental improvements simply fork the repo and follow the process below. For larger changes submit an [RFC:](RFC.md)
1. [Fork](https://help.github.com/articles/fork-a-repo/) the repository and [clone](https://help.github.com/articles/cloning-a-repository/) your fork.

2. Start coding!
    - Configure clangd LSP by generating `compile_commands.json` (e.g. `bear -- make` or `compiledb make`)
    - Implement your feature.
    - Check your code works as expected.
    - Run the code formatter: `clang-format -i $(git ls-files "*.c" "*.h")`

3. Commit your changes to a new branch (not `master`, one change per branch) and push it:
    - Commit messages should:
        - Header line: explain the commit in one line (use the imperative)
        - Be descriptive.
        - Have a first line with less than *80 characters* and have a second line that is *empty* if you want to add a description.

4. Once you are happy with your changes, submit a pull request.
     - Open the pull request.
     - Add a short description explaining briefly what you've done (or if it's a work-in-progress - what you need to do)

## Issues

1. Do a quick search on GitHub to check if the issue has already been reported.
2. [Open an issue](https://github.com//javalsai/lidm/issues/new) and describe the issue you are having - you could include:
     - Screenshots.
     - Ways to reproduce the issue.
     - Your lidm version.
     - Your platform (e.g. arch linux or Ubuntu 15.04 x64)

After reporting you should aim to answer questions or clarifications as this helps pinpoint the cause of the issue.

