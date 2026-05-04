# Terminal Audio Player

Terminal Audio Player is a small C++ learning project.

The goal is to build a terminal-based audio player while practicing basic software architecture:
input handling, UI, command processing, state management, track library scanning, and audio playback.

## Current Features

- Terminal UI (ncurses)
- Track scanning from `~/Music/TAP_content`
- Audio file filtering
- Track list
- Select track by index
- Play, pause, resume, stop
- Next and previous track
- Track status
- Refresh track library
- SFML Audio backend

## Supported Formats

- `.mp3`
- `.wav`
- `.ogg`
- `.flac`

## Tech

- C++17
- CMake
- SFML Audio
- ncurses

## Status

MVP

## How To Run

Download the latest Linux binary from the [Releases](https://github.com/AdeptusXIII/TerminalAudioPlayer/releases)
page.

  ```bash
  cd ~/Downloads
  chmod +x terminal_player
  ./terminal_player
  ```

When you first launch the player, it automatically creates a directory with content and scans it:

  ```bash
  ~/Music/TAP_content
  ```

Put your audio files there before running the player.

## How To Use?

- You must place audio files inside the working folder(~/Music/TAP_content).
- Just type help. The program supports scrolling using the arrow keys. 


## Install Locally

Download the latest Linux binary from the [Releases](https://github.com/AdeptusXIII/TerminalAudioPlayer/releases)
page.

  ```bash
  mkdir -p ~/.local/bin
  mv ~/Downloads/terminal_player ~/.local/bin/terminal_player
  chmod +x ~/.local/bin/terminal_player
  terminal_player
  ```

## License

This project is licensed under the GNU General Public License v3.0.

See [LICENSE](LICENSE) for the full license text.