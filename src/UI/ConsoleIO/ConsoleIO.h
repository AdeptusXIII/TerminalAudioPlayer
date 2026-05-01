//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#pragma once 

#include "Types/PlaybackTypes.h"
#include "Types/CommandTypes.h"

#include <string>
#include <vector>

struct FUISnapshotData;

struct FHelpEntry
{
    std::string Usage;
    std::string Description;
};

struct FHelpEntryEXT
{
    std::vector<std::string> Usage;
    std::string Description;
    std::vector<std::string> Example;
};

class MConsoleIO 
{
public:
    MConsoleIO();
    
    // MENU MODE FUNCTIONS
    void PrintTrackList(const FUISnapshotData &UISnapshot);
    void PrintTotalTracksNum(const int &InTotalTracks);
    
    // COMMAND LINE MODE FUNCTIONS
    FCommand ReadCommand();
    void PrintHelp();
    void PrintHelpCMD(ECommandType CommandType);
    void PrintStatus(const EAudioPlayerState& AudioPlayerState, EPlaybackMode& PlaybackMode, std::string CurrentTrack, 
        float CurrentVolume);
    
private:
    
    int CommandHelpIdentation;
};