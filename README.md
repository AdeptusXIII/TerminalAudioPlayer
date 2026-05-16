# Terminal Audio Player

Terminal Audio Player is a small C++ learning project.

The goal is to build a terminal-based audio player while practicing basic software architecture:
input handling, terminal UI, command processing, state management, track library scanning, and audio playback.

Current project version: `0.16`

## Status

MVP.

The player is usable, but still experimental. It is mainly built as a learning project, not as a polished production audio player.

## Features

- Terminal UI based on `ncursesw`
- UTF-8 terminal output
- Track scanning from `~/Music/TAP_content`
- Automatic content directory creation on first launch
- Audio file filtering
- Track list
- Track search by name
- Select track by index
- Play, pause, resume, stop
- Next and previous track
- Playback modes:
  - once
  - loop-one
  - loop-all
  - loop-shuffle
- Volume control
- Track status with playback time
- Scrollable command output
- Terminal resize handling
- SFML 3 Audio backend

## Supported Formats

- `.mp3`
- `.wav`
- `.ogg`
- `.flac`

## Requirements

For the AppImage release:

- Linux x86_64
- UTF-8 terminal
- FUSE support for normal AppImage execution

The AppImage bundles the required runtime libraries used by the player, including SFML Audio and ncursesw.

If your system cannot mount AppImages through FUSE, use the fallback command shown below.

## Download

Download the latest AppImage from the [Releases](https://github.com/AdeptusXIII/TerminalAudioPlayer/releases) page.

The release file is named:

```text
TerminalAudioPlayer-x86_64.AppImage
```

## How To Run

From the directory where you downloaded the AppImage:

```bash
chmod +x TerminalAudioPlayer-x86_64.AppImage
./TerminalAudioPlayer-x86_64.AppImage
```
Or launch it from the file manager by running the .AppImage file directly.

Theoretically, this launch method is supported by the following terminals:

```
console
gnome-terminal
kgx
xfce4-terminal
mate-terminal
lxterminal
qterminal
kitty
alacritty
wezterm
xterm
```

I don't have the opportunity to test everything. Everything works fine on my alacritty.

If AppImage mounting through FUSE is not available on your system:

```bash
APPIMAGE_EXTRACT_AND_RUN=1 ./TerminalAudioPlayer-x86_64.AppImage
```

## Music Directory

On first launch, the player creates and scans this directory:

```bash
~/Music/TAP_content
```

Put your audio files there.

If the player is already running after you added new files, type:

```text
refresh
```

or restart the program.

Do not run the player with `sudo`. The content directory should belong to your normal user.

## Basic Usage

Type commands into the input line and press Enter.

Start with:

```text
help
```

Common commands:

```text
list
play
pause
stop
next
prev
select <index>
find <text>
mode <once|loop-one|loop-all|loop-shuffle>
volume <0..100>
status
refresh
exit
```

The output window supports scrolling:

```text
Up / Down / Mouse Wheel    vertical output scroll
```

Minimum terminal size:

```text
72x18
```

If the terminal is smaller, the player shows a "terminal too small" message instead of drawing the full UI.

## Install Locally

You can place the AppImage in `~/.local/bin` and run it like a normal command:

```bash
mkdir -p ~/.local/bin
mv ~/Downloads/TerminalAudioPlayer-x86_64.AppImage ~/.local/bin/terminal-audio-player
chmod +x ~/.local/bin/terminal-audio-player
terminal-audio-player
```

If `terminal-audio-player` is not found, make sure `~/.local/bin` is in your `PATH`.

## Build From Source

Build requirements:

- C++17 compiler
- CMake
- Ninja
- SFML 3 Audio
- ncursesw

On Arch/CachyOS:

```bash
sudo pacman -S cmake ninja gcc sfml ncurses pkgconf
```

Configure and build:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target terminal_player
```

Run the local build:

```bash
./build/terminal_player
```

## Build AppImage

The repository contains a packaging script:

```bash
./packaging/build_appimage.sh
```

It creates:

```text
dist/TerminalAudioPlayer-x86_64.AppImage
```

The script builds a Release binary, creates an AppDir, copies the required runtime libraries, downloads `appimagetool` if needed, and packages the final AppImage.

Generated packaging folders are ignored by git:

```text
build-appimage/
dist/
```

## Tech

- C++17
- CMake
- SFML 3 Audio
- ncursesw
- AppImage packaging

## License

This project is licensed under the GNU General Public License v3.0.

See [LICENSE](LICENSE) for the full license text.
