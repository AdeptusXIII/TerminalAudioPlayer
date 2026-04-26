cmake_minimum_required(VERSION 3.16)

project(TerminalAudioPlayer VERSION 0.10 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
find_package(SFML 2.6 COMPONENTS audio REQUIRED)

add_executable(terminal_player
    src/main.cpp
        src/Engine/Engine.cpp
        src/Audio/AudioBackend.cpp
        src/Audio/AudioBackend.h
        src/FileManager/FileManager.cpp
        src/FileManager/FileManager.h
        src/TrackLibrary/TrackLibrary.cpp
        src/TrackLibrary/TrackLibrary.h
        src/Audio/AudioFileScanner.cpp
        src/Audio/AudioFileScanner.h
        src/Engine/MenuDefinition.h
        src/Types/PlaybackTypes.h
)

target_include_directories(terminal_player PRIVATE src)
target_link_libraries(terminal_player PRIVATE sfml-audio)
