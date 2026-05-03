//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#pragma once

#include "Types/PlaybackTypes.h"

#include <vector>
#include <string>

struct FUISnapshotData 
{
    EAudioPlayerState AudioPlayerState = EAudioPlayerState::None;
    EPlaybackMode PlaybackMode = EPlaybackMode::None;
    
    int TrackCount = -1;
    int CurrentTrackIndex = -1;
    float CurrentVolume = -1.f;
    
    std::string CurrentTrackName = "None";
    std::vector<std::string> TrackList = {"None"};
};
