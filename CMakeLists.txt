cmake_minimum_required(VERSION 3.16)

project(TerminalAudioPlayer VERSION 0.20.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CURSES_NEED_WIDE TRUE)
find_package(SFML 3 COMPONENTS Audio REQUIRED)
find_package(Curses REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD REQUIRED IMPORTED_TARGET libsystemd)

add_executable(terminal_player
    src/main.cpp
        src/Engine/Engine.cpp
        src/Audio/AudioBackend.cpp
        src/Audio/AudioBackend.h
        src/TrackLibrary/TrackLibrary.cpp
        src/TrackLibrary/TrackLibrary.h
        src/Audio/AudioFileScanner.cpp
        src/Audio/AudioFileScanner.h
        src/Types/PlaybackTypes.h
        src/Types/CommandTypes.h
        src/UI/ConsoleIO/ConsoleIO.cpp
        src/UI/ConsoleIO/ConsoleIO.h
        src/UI/UISnapshot.h
        src/Engine/CommandDataTable.h
        src/Engine/GlobalParameters.h
        src/Engine/GlobalParameters.cpp
        src/Engine/StringTemplates.h
        src/PlayerStateStorage/PlayerStateStorage.cpp
        src/PlayerStateStorage/PlayerStateStorage.h
        src/System/PowerEventWatcher.cpp
        src/System/PowerEventWatcher.h
        src/Types/PlayerStateTypes.h
)

target_include_directories(terminal_player PRIVATE src)
target_include_directories(terminal_player PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(terminal_player PRIVATE SFML::Audio)
target_link_libraries(terminal_player PRIVATE ${CURSES_LIBRARIES})
target_link_libraries(terminal_player PRIVATE PkgConfig::SYSTEMD)
