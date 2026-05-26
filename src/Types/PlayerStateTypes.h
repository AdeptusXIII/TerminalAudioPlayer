//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".
#pragma once

#include "PlaybackTypes.h"

#include <filesystem>
#include <string>
#include <vector>

struct FCustomTrackListData
{
    std::string Name;
    std::vector<std::filesystem::path> Tracks;
};

struct FPlayerStateData
{
    std::vector<std::filesystem::path> AllTracks;
    std::vector<std::filesystem::path> FavoriteTracks;
    std::vector<FCustomTrackListData> CustomTrackLists;
    std::string ActiveListName = {};
    EPlaybackMode PlaybackMode = EPlaybackMode::LoopAll;
    EAudioPlayerState AudioPlayerState = EAudioPlayerState::Idle;
    float Volume = 100.0f;
    std::filesystem::path CurrentTrackPath = {};
    float CurrentTrackPositionSec = 0.0f;
};
