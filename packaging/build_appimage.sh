#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build-appimage"
DIST_DIR="$ROOT_DIR/dist"
APPDIR="$BUILD_DIR/TerminalAudioPlayer.AppDir"
APPIMAGETOOL="$BUILD_DIR/appimagetool-x86_64.AppImage"
APPIMAGE_OUT="$DIST_DIR/TerminalAudioPlayer-x86_64.AppImage"

APPIMAGETOOL_URL="https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage"

copy_library() {
    local lib_path="$1"

    if [[ -f "$lib_path" ]]; then
        cp -L "$lib_path" "$APPDIR/usr/lib/"
    fi
}

copy_runtime_libraries() {
    local binary="$1"

    while read -r lib_path; do
        case "$lib_path" in
            /usr/lib/libsfml-*.so*|\
            /usr/lib/libncursesw.so*|\
            /usr/lib/libformw.so*|\
            /usr/lib/libvorbis*.so*|\
            /usr/lib/libFLAC.so*|\
            /usr/lib/libogg.so*)
                copy_library "$lib_path"
                ;;
        esac
    done < <(ldd "$binary" | awk '/=> \// { print $3 }')
}

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" --target terminal_player

rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/lib" "$APPDIR/usr/share/applications" "$APPDIR/usr/share/icons/hicolor/scalable/apps" "$DIST_DIR"

cp "$BUILD_DIR/terminal_player" "$APPDIR/usr/bin/terminal_player"
strip "$APPDIR/usr/bin/terminal_player" || true
copy_runtime_libraries "$APPDIR/usr/bin/terminal_player"

cat > "$APPDIR/AppRun" <<'APPRUN'
#!/usr/bin/env bash
HERE="$(dirname "$(readlink -f "$0")")"
PLAYER="$HERE/usr/bin/terminal_player"

export LD_LIBRARY_PATH="$HERE/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

if [[ -t 0 && -t 1 ]]; then
    exec "$PLAYER" "$@"
fi

if command -v konsole >/dev/null 2>&1; then
    exec konsole -e "$PLAYER" "$@"
elif command -v gnome-terminal >/dev/null 2>&1; then
    exec gnome-terminal -- "$PLAYER" "$@"
elif command -v kgx >/dev/null 2>&1; then
    exec kgx -- "$PLAYER" "$@"
elif command -v xfce4-terminal >/dev/null 2>&1; then
    exec xfce4-terminal -x "$PLAYER" "$@"
elif command -v mate-terminal >/dev/null 2>&1; then
    exec mate-terminal -- "$PLAYER" "$@"
elif command -v lxterminal >/dev/null 2>&1; then
    exec lxterminal -e "$PLAYER" "$@"
elif command -v qterminal >/dev/null 2>&1; then
    exec qterminal -e "$PLAYER" "$@"
elif command -v kitty >/dev/null 2>&1; then
    exec kitty "$PLAYER" "$@"
elif command -v alacritty >/dev/null 2>&1; then
    exec alacritty -e "$PLAYER" "$@"
elif command -v wezterm >/dev/null 2>&1; then
    exec wezterm start -- "$PLAYER" "$@"
elif command -v xterm >/dev/null 2>&1; then
    exec xterm -e "$PLAYER" "$@"
fi

echo "TerminalAudioPlayer requires a terminal emulator to run." >&2
echo "Run this AppImage from a terminal, or install konsole, gnome-terminal, kgx, xfce4-terminal, kitty, alacritty, wezterm, or xterm." >&2
exit 1
APPRUN
chmod +x "$APPDIR/AppRun"

cat > "$APPDIR/TerminalAudioPlayer.desktop" <<'DESKTOP'
[Desktop Entry]
Type=Application
Name=TerminalAudioPlayer
Comment=Terminal-based audio player
Exec=terminal_player
Icon=terminalaudioplayer
Categories=AudioVideo;Audio;Player;ConsoleOnly;
Terminal=true
DESKTOP

cp "$APPDIR/TerminalAudioPlayer.desktop" "$APPDIR/usr/share/applications/TerminalAudioPlayer.desktop"

cat > "$APPDIR/terminalaudioplayer.svg" <<'SVG'
<svg xmlns="http://www.w3.org/2000/svg" width="128" height="128" viewBox="0 0 128 128">
  <rect width="128" height="128" rx="18" fill="#0b0f0c"/>
  <path d="M28 35h72v58H28z" fill="none" stroke="#18ff62" stroke-width="6"/>
  <path d="M40 52l14 12-14 12" fill="none" stroke="#18ff62" stroke-width="7" stroke-linecap="round" stroke-linejoin="round"/>
  <path d="M63 78h28" stroke="#18ff62" stroke-width="7" stroke-linecap="round"/>
</svg>
SVG

cp "$APPDIR/terminalaudioplayer.svg" "$APPDIR/usr/share/icons/hicolor/scalable/apps/terminalaudioplayer.svg"

if [[ ! -x "$APPIMAGETOOL" ]]; then
    curl -L "$APPIMAGETOOL_URL" -o "$APPIMAGETOOL"
    chmod +x "$APPIMAGETOOL"
fi

APPIMAGE_EXTRACT_AND_RUN=1 ARCH=x86_64 "$APPIMAGETOOL" "$APPDIR" "$APPIMAGE_OUT"
chmod +x "$APPIMAGE_OUT"

echo "Created: $APPIMAGE_OUT"
