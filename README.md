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

## Usage

Run `HelloDesktop.exe`. A notification-area (tray) icon provides a right-click menu:

- **Start renderer / Stop renderer** - creates/destroys the desktop background window (double-click toggles too)
- **Exit**

The app is a GUI-subsystem exe (no console window). When launched from a console it attaches to it, and logs diagnostics (desktop window lookup, Z-order) to stderr (if available) and `OutputDebugString`.

## Project layout

- `src/` - sources
  - `main.cpp`, `App.h`, `HelloDesktop.cpp` - entry point and main app loop (monitor selection, tray wiring)
  - `BackgroundWindowManager` - desktop-hosted background window
  - `Renderer`, `Drawing.h`, `LandscapeGenerator.h` - landscape generation and drawing
  - `TrayIcon` - notification-area icon and menu
  - `Log.h` - logging and parent console attach
  - `Platform.h` - Windows headers (see C++23 modules notes)
- `resources/` - Windows resources (`HelloDesktop.rc`, `HelloDesktop.exe.manifest`), compiled via `windows.compile_resources` in `meson.build`
- `bin/debug`, `bin/release` - install output

## Rendering approach

`BackgroundWindowManager` creates its own child window inside the desktop host and draws into it with GDI:

- Windows 11 24H2+ (verified on 25H2): child of the `WorkerW` that is itself a child of `Progman`. The window must be `WS_EX_LAYERED` or DWM does not composite it. Parenting directly to `Progman` does not work.
- Older: child of the `WorkerW` that follows the window holding `SHELLDLL_DefView`.

Drawing happens in the window's `WM_PAINT` (via a paint handler); the render loop just invalidates the window. Pixel buffers are BGRA (GDI order).

## Known limitations

- Only the primary monitor is rendered; the window covers just that monitor.
- The legacy (pre-24H2) `WorkerW` path is untested.
- The tray icon uses the stock application icon.

## C++23 modules notes

- Uses `import std;` with GCC `-fmodules`. `meson.build` bootstraps the `std` module (`bits/std.cc`) before compiling sources.
- Include `src/Platform.h` **before** `import std;` in any file that uses Windows headers. MinGW's `winnt.h` includes `<x86intrin.h>` inside `extern "C"`, which conflicts with `std`'s C++-linkage declarations.
- Headers that rely on `std::` must be included after `import std;`.
