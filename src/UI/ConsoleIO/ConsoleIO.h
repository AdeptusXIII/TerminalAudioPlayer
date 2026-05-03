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
    std::vector<std::string> Description;
    std::vector<std::string> Examples;
};

class MConsoleIO 
{
public:
    MConsoleIO();
    
    FCommand ReadCommand();
    
    void PrintTrackList(const FUISnapshotData &UISnapshot);
    void PrintTotalTracksNum(const int &InTotalTracks);
    void PrintCommandHelp();
    void PrintCommandHelpArg(ECommandType CommandType);
    void PrintStatus(const FUISnapshotData &UISnapshot);
    void PrintFindResults(const std::vector<std::pair<int, std::string>> &FindedTracks);
    
private:
    
    int CommandHelpIdentation;
};