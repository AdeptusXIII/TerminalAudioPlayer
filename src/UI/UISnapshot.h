//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#pragma once

#include "Types/PlaybackTypes.h"

#include <vector>
#include <string>

struct FTrackInfo
{
    std::string TrackName = "None";
    float DurationSec = -1.f;
    float PositionSec = -1.f;
    float RemainingSec = -1.f;
};

struct FUISnapshotData 
{
    EAudioPlayerState AudioPlayerState = EAudioPlayerState::None;
    EPlaybackMode PlaybackMode = EPlaybackMode::None;
    
    int TrackCount = -1;
    int TrackIndex = -1;
    float Volume = -1.f;
    
    std::string CurrentTrackName = "None";
    std::vector<std::string> TrackList = {"None"};
};
