//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#include "PlayerStateStorage.h"

#include "Engine/StringTemplates.h"

#include <cstdlib>
#include <fstream>

namespace
{
    std::string PlaybackModeToStorageString(EPlaybackMode PlaybackMode)
    {
        switch (PlaybackMode)
        {
            case EPlaybackMode::Once:
                return "once";
            case EPlaybackMode::LoopOne:
                return "loop-one";
            case EPlaybackMode::LoopAll:
                return "loop-all";
            case EPlaybackMode::LoopShuffle:
                return "loop-shuffle";
            case EPlaybackMode::None:
                return "none";
        }

        return "loop-all";
    }

    EPlaybackMode PlaybackModeFromStorageString(const std::string& Value)
    {
        if (Value == "once") return EPlaybackMode::Once;
        if (Value == "loop-one") return EPlaybackMode::LoopOne;
        if (Value == "loop-all") return EPlaybackMode::LoopAll;
        if (Value == "loop-shuffle") return EPlaybackMode::LoopShuffle;

        return EPlaybackMode::LoopAll;
    }

    std::string AudioPlayerStateToStorageString(EAudioPlayerState AudioPlayerState)
    {
        switch (AudioPlayerState)
        {
            case EAudioPlayerState::Idle:
                return "idle";
            case EAudioPlayerState::Playing:
                return "playing";
            case EAudioPlayerState::Paused:
                return "paused";
            case EAudioPlayerState::None:
                return "none";
        }

        return "idle";
    }

    EAudioPlayerState AudioPlayerStateFromStorageString(const std::string& Value)
    {
        if (Value == "playing") return EAudioPlayerState::Playing;
        if (Value == "paused") return EAudioPlayerState::Paused;
        if (Value == "idle") return EAudioPlayerState::Idle;

        return EAudioPlayerState::Idle;
    }
}

bool MPlayerStateStorage::EnsureStateFileExists()
{
    const std::filesystem::path StateFilePath = GetStateFilePath();
    const std::filesystem::path StateDirectoryPath = StateFilePath.parent_path();

    if (!EnsureDirectoryExists(StateDirectoryPath))
    {
        return false;
    }

    if (FileExists(StateFilePath))
    {
        return true;
    }

    return CreateEmptyFile(StateFilePath);
}

bool MPlayerStateStorage::Save(const FPlayerStateData &State)
{
    if (!EnsureStateFileExists()) return false;

    const std::filesystem::path StateFilePath = GetStateFilePath();
    const std::filesystem::path TempFilePath = StateFilePath.string() + ".tmp";

    std::ofstream File(TempFilePath);
    if (!File.is_open()) return false;

    // 1. all
    File << "[" + static_cast<std::string>(stp::svst::S_ALL) + "]" + "\n";
    for (const std::filesystem::path &Path : State.AllTracks)
    {
        File << Path.string() << "\n";
    }

    // 2. favorite
    File << "\n";
    File << "[" + static_cast<std::string>(stp::svst::S_FAV) + "]" + "\n";
    for (const std::filesystem::path &Path : State.FavoriteTracks)
    {
        File << Path.string() << "\n";
    }

    // 3. custom
    for (const FCustomTrackListData& CustomList : State.CustomTrackLists)
    {
        File << "\n";
        File << "[" + static_cast<std::string>(stp::svst::S_CUSTOM) + ":" << CustomList.Name << "]\n";

        for (const std::filesystem::path& TrackPath : CustomList.Tracks)
        {
            File << TrackPath.string() << "\n";
        }
    }

    // 4. active-list
    File << "\n";
    File << "[" + static_cast<std::string>(stp::svst::S_ACTIVE_LIST) + "]" + "\n";
    File << State.ActiveListName << "\n";

    // 5. playback mode
    File << "\n";
    File << "[" + static_cast<std::string>(stp::svst::S_PLAYBACK_MODE) + "]" + "\n";
    File << PlaybackModeToStorageString(State.PlaybackMode) << "\n";

    // 6. audio player state
    File << "\n";
    File << "[" + static_cast<std::string>(stp::svst::S_PLAYER_AUDIO_STATE) + "]" + "\n";
    File << AudioPlayerStateToStorageString(State.AudioPlayerState) << "\n";

    // 7. volume
    File << "\n";
    File << "[" + static_cast<std::string>(stp::svst::S_VOLUME) + "]" + "\n";
    File << State.Volume << "\n";

    // 8. current track
    File << "\n";
    File << "[" + static_cast<std::string>(stp::svst::S_CURRENT_TRACK) + "]" + "\n";
    File << State.CurrentTrackPath.string() << "\n";

    // 9. current position
    File << "\n";
    File << "[" + static_cast<std::string>(stp::svst::S_CURRENT_POSITION) + "]" + "\n";
    File << State.CurrentTrackPositionSec << "\n";

    File.close();

    if (!File)
    {
        std::error_code RemoveErrorCode;
        std::filesystem::remove(TempFilePath, RemoveErrorCode);
        return false;
    }

    std::error_code RenameErrorCode;
    std::filesystem::rename(TempFilePath, StateFilePath, RenameErrorCode);

    if (RenameErrorCode)
    {
        std::error_code RemoveErrorCode;
        std::filesystem::remove(TempFilePath, RemoveErrorCode);
        return false;
    }

    return true;
}

bool MPlayerStateStorage::Load(FPlayerStateData &OutState)
{
    if (!EnsureStateFileExists()) return false;

    std::ifstream File(GetStateFilePath());
    if (!File.is_open()) return false;

    OutState = {};

    std::string CurrentSection;
    std::string Line;

    while (std::getline(File, Line))
    {
        if (Line.empty()) continue;

        // 1. check line [all]
        if (Line == "[" + static_cast<std::string>(stp::svst::S_ALL) + "]")
        {
            CurrentSection = stp::svst::S_ALL;
            continue;
        }

        // 2. check line [favorite]
        if (Line == "[" + static_cast<std::string>(stp::svst::S_FAV) + "]")
        {
            CurrentSection = stp::svst::S_FAV;
            continue;
        }

        // 3. check line [custom:name]
        if (Line.rfind("[" + static_cast<std::string>(stp::svst::S_CUSTOM) + ":", 0) == 0 && Line.back() == ']')
        {
            CurrentSection = stp::svst::S_CUSTOM;

            const std::size_t NameStart = std::string("[" + static_cast<std::string>(stp::svst::S_CUSTOM) + ":").size();
            const std::size_t NameLength = Line.size() - NameStart - 1;

            OutState.CustomTrackLists.push_back({});
            OutState.CustomTrackLists.back().Name = Line.substr(NameStart, NameLength);

            continue;
        }

        // 4. check line [active-list]
        if (Line == "[" + static_cast<std::string>(stp::svst::S_ACTIVE_LIST) + "]")
        {
            CurrentSection = stp::svst::S_ACTIVE_LIST;
            continue;
        }

        // 5. check line [playback-mode]
        if (Line == "[" + static_cast<std::string>(stp::svst::S_PLAYBACK_MODE) + "]")
        {
            CurrentSection = stp::svst::S_PLAYBACK_MODE;
            continue;
        }

        // 6. check line [audio-state]
        if (Line == "[" + static_cast<std::string>(stp::svst::S_PLAYER_AUDIO_STATE) + "]")
        {
            CurrentSection = stp::svst::S_PLAYER_AUDIO_STATE;
            continue;
        }

        // 7. check line [volume]
        if (Line == "[" + static_cast<std::string>(stp::svst::S_VOLUME) + "]")
        {
            CurrentSection = stp::svst::S_VOLUME;
            continue;
        }

        // 8. check line [current-track]
        if (Line == "[" + static_cast<std::string>(stp::svst::S_CURRENT_TRACK) + "]")
        {
            CurrentSection = stp::svst::S_CURRENT_TRACK;
            continue;
        }

        // 9. check line [current-position]
        if (Line == "[" + static_cast<std::string>(stp::svst::S_CURRENT_POSITION) + "]")
        {
            CurrentSection = stp::svst::S_CURRENT_POSITION;
            continue;
        }

        // 1. check section [all]
        if (CurrentSection == stp::svst::S_ALL)
        {
            OutState.AllTracks.emplace_back(Line);
            continue;
        }

        // 2. check section [favorite]
        if (CurrentSection == stp::svst::S_FAV)
        {
            OutState.FavoriteTracks.emplace_back(Line);
            continue;
        }

        // 3. check section [custom:name]
        if (CurrentSection == stp::svst::S_CUSTOM)
        {
            if (!OutState.CustomTrackLists.empty())
            {
                OutState.CustomTrackLists.back().Tracks.emplace_back(Line);
            }

            continue;
        }

        // 4. check section [active-list]
        if (CurrentSection == stp::svst::S_ACTIVE_LIST)
        {
            OutState.ActiveListName = Line;
            continue;
        }

        // 5. check section [playback-mode]
        if (CurrentSection == stp::svst::S_PLAYBACK_MODE)
        {
            OutState.PlaybackMode = PlaybackModeFromStorageString(Line);
            continue;
        }

        // 6. check section [audio-state]
        if (CurrentSection == stp::svst::S_PLAYER_AUDIO_STATE)
        {
            OutState.AudioPlayerState = AudioPlayerStateFromStorageString(Line);
            continue;
        }

        // 7. check section [volume]
        if (CurrentSection == stp::svst::S_VOLUME)
        {
            try
            {
                OutState.Volume = std::stof(Line);
            }
            catch (...)
            {
                OutState.Volume = 100.0f;
            }

            continue;
        }

        // 8. check section [current-track]
        if (CurrentSection == stp::svst::S_CURRENT_TRACK)
        {
            OutState.CurrentTrackPath = Line;
            continue;
        }

        // 9. check section [current-position]
        if (CurrentSection == stp::svst::S_CURRENT_POSITION)
        {
            try
            {
                OutState.CurrentTrackPositionSec = std::stof(Line);
            }
            catch (...)
            {
                OutState.CurrentTrackPositionSec = 0.0f;
            }

            continue;
        }
    }

    return true;
}

bool MPlayerStateStorage::EnsureDirectoryExists(const std::filesystem::path &DirectoryPath) const
{
    std::error_code ErrorCode;

    if (std::filesystem::is_directory(DirectoryPath, ErrorCode))
    {
        return true;
    }

    if (ErrorCode)
    {
        return false;
    }

    std::filesystem::create_directories(DirectoryPath, ErrorCode);

    if (ErrorCode)
    {
        return false;
    }

    return std::filesystem::is_directory(DirectoryPath);
}

std::filesystem::path MPlayerStateStorage::GetStateFilePath() const
{
    const char* XdgDataHome = std::getenv("XDG_DATA_HOME");

    if (XdgDataHome != nullptr)
    {
        return std::filesystem::path(XdgDataHome) / "TerminalAudioPlayer/state.txt";
    }

    const char* Home = std::getenv("HOME");

    if (Home != nullptr)
    {
        return std::filesystem::path(Home) / ".local/share/TerminalAudioPlayer/state.txt";
    }

    return std::filesystem::current_path() / "TerminalAudioPlayer/state.txt";
}

bool MPlayerStateStorage::FileExists(const std::filesystem::path &FilePath) const
{
    std::error_code ErrorCode;
    const bool bExists = std::filesystem::exists(FilePath, ErrorCode);

    if (ErrorCode) return false;

    return bExists;
}

bool MPlayerStateStorage::CreateEmptyFile(const std::filesystem::path &FilePath) const
{
    std::ofstream File(FilePath);
    return File.is_open();
}
