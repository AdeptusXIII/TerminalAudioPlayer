# Terminal Audio Player

Terminal Audio Player is a small C++ learning project.

The goal is to build a terminal-based audio player while practicing basic software architecture:
input handling, terminal UI, command processing, state management, track library scanning, and audio playback.

Current project version: `0.20.0`

## Status

MVP.

The player is usable, but still experimental. It is mainly built as a learning project, not as a polished production audio player.

## Features

- Terminal UI based on `ncursesw`
- UTF-8 terminal output
- UTF-8 terminal input
- Command history navigation
- Editable input line with cursor movement
- Track scanning from `~/Music/TAP_content`
- Manual directory scanning into a temporary buffer list
- Recursive directory scanning with `scan -r`
- Audio file validation through SFML readability checks
- Persistent player state storage in `state.txt`
- Track lists: `buffer`, `all`, `favorite`, and custom lists
- Persistent `all`, `favorite`, custom lists, active list, volume, playback mode, and paused track position
- Automatic content directory creation on first launch
- Audio file filtering
- Track list
- Track search by name
- Play track by index
- Play, pause, resume, stop
- Next and previous track
- Playback modes:
  - once
  - loop-one
  - loop-all
  - loop-shuffle
- Volume control
- Track status with playback time
- Paused track restoration after restart
- Scrollable command output
- Word-wrapped command output
- Terminal resize handling
- systemd sleep/hibernate handling: playback pauses before sleep and does not auto-resume
- Direct AppImage launch from a file manager, if a supported terminal emulator is installed
- SFML 3 Audio backend using `sf::Music` streaming

## Supported Formats

- `.mp3`
- `.wav`
- `.ogg`
- `.flac`

## Requirements

For the AppImage release:

- Linux x86_64
- UTF-8 terminal
- UTF-8 locale
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
Or launch it from the file manager by running the `.AppImage` file directly.

This launch method needs a terminal emulator. The AppImage launcher currently tries these terminals:

```text
konsole
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

Not every terminal in this list has been tested. The file-manager launch path is confirmed to work with `alacritty`.

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

You can also scan another directory manually:

```text
scan <path>
```

This scans only files directly inside the target directory.

For recursive scanning through all nested directories:

```text
scan -r <path>
```

Scan results are placed into the temporary `buffer` list. To save scanned tracks into the persistent `all` list:

```text
pl all add all from buffer
```

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
next -d
prev
prev -d
play <index>
play -d <index>
find <text>
scan <path>
scan -r <path>
pl list
pl use <buffer|all|favorite|name|index>
pl create <name>
pl delete <name>
pl <list> list
pl all add <index|all> from buffer
pl all remove <index>
pl favorite add <index>
pl favorite add <index> from <buffer|all|name>
pl favorite remove <index>
pl <name> add <index|all>
pl <name> add <index|all> from <buffer|all|favorite|name>
pl <name> remove <index>
mode <once|loop-one|loop-all|loop-shuffle>
volume <0..100>
status
refresh
exit
```

## Track Lists

The player currently has these list types:

```text
buffer     temporary output of the latest scan command
all        persistent list of tracks known by the player
favorite   persistent favorite list
custom     persistent user-created lists
```

`list`, `play`, `next`, and `prev` work with the active list.
Playlist `add` commands without `from <source>` use the active list as the source.

Use:

```text
pl list
```

to show available lists, and:

```text
pl use <buffer|all|favorite|name|index>
```

to switch the active list.

The player stores persistent state in:

```text
~/.local/share/TerminalAudioPlayer/state.txt
```

The state file stores `all`, `favorite`, custom lists, active list, volume, playback mode, and paused track position.
The `buffer` list is temporary and is replaced by the next `scan` command.

If the player exits while a track is playing, it saves the track as paused. On the next launch, `play` resumes from the saved position.

## TUI Controls

Input line:

```text
Enter          run command
Up             previous command from history
Down           next command from history
Left           move input cursor left
Right          move input cursor right
Backspace      delete character before cursor
UTF-8 text     supported in the input line
```

Output window:

```text
Shift+Up       scroll output up, if supported by the terminal
Shift+Down     scroll output down, if supported by the terminal
Mouse Wheel    vertical output scroll, if supported by the terminal
```

Minimum terminal size:

```text
72x19
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
- systemd/libsystemd development headers
- pkgconf

On Arch/CachyOS:

```bash
sudo pacman -S cmake ninja gcc sfml ncurses systemd pkgconf
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
