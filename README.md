# HelloDesktop

Draw generated 2D landscape as a Windows desktop background.

Inspired by: https://github.com/juangallostra/Landscape-generator

## Requirements

- Windows
- MSYS2 UCRT64 GCC 15+ (C++23 `import std;` support), `C:\msys64\ucrt64\bin` on `PATH`
- Meson + Ninja

## Build

One-time setup (build output goes in git-ignored `build/`):

```sh
meson setup build/debug --buildtype=debug
meson setup build/release --buildtype=release
```

Compile:

```sh
meson compile -C build/debug
meson compile -C build/release
```

Install (to `bin/<buildtype>/`, i.e. `bin/debug` or `bin/release`):

```sh
meson install -C build/debug
meson install -C build/release
```

If `meson.build` changes the config regenerates automatically; to wipe a configuration, add `--wipe` to the `meson setup` command.

## Project layout

- `src/` - sources
- `resources/` - Windows resources (`HelloDesktop.rc`, `HelloDesktop.exe.manifest`), compiled via `windows.compile_resources` in `meson.build`
- `bin/debug`, `bin/release` - install output

## C++23 modules notes

- Uses `import std;` with GCC `-fmodules`. `meson.build` bootstraps the `std` module (`bits/std.cc`) before compiling sources.
- Include `src/Platform.h` **before** `import std;` in any file that uses Windows headers. MinGW's `winnt.h` includes `<x86intrin.h>` inside `extern "C"`, which conflicts with `std`'s C++-linkage declarations.
- Headers that rely on `std::` must be included after `import std;`.
