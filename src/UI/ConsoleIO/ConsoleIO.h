//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#pragma once 

#include "Types/PlaybackTypes.h"
#include "UI/ConsoleIO/MenuDefinition.h"
#include "Types/CommandTypes.h"

#include <string>
#include <vector>

/** SubCategory Separator Type T: "├──" */
const std::string SUBCAT_SEP_T = "├──";

/** SubCategory Separator Type L: "└──" */
const std::string SUBCAT_SEP_L = "└──";

/** SubCategory Separator Type I: "│" */
const std::string SUBCAT_SEP_I = "│";

/** SubCategory Separator Type TAB: "  " */
const std::string SUBCAT_SEP_TAB = "  ";

struct FUISnapshotData;

class MConsoleIO 
{
public:
    MConsoleIO();
    
    // MENU MODE FUNCTIONS
    void PrintMainMenuAndState(const FUISnapshotData &UISnapshot);
    void PrintLibraryMenuAndState(const FUISnapshotData &UISnapshot);
    void PrintPlaybackModeMenuAndState(const FUISnapshotData &UISnapshot);
    void PrintTrackList(const FUISnapshotData &UISnapshot);
    void PrintTotalTracksNum(const int &InTotalTracks);
    
    int ReadIntInRange(int Min, int Max, const std::string& Prompt);
    
    // COMMAND LINE MODE FUNCTIONS
    FCommand ReadCommand();
    
private:
    
    void PrintAudioPlayerState(const EAudioPlayerState &InAudioPlayerState);
    void PrintPlaybackMode(const EPlaybackMode &InPlaybackMode);
    void PrintCurrentTrack(const std::string &InTrackName);
    void PrintMenuSection(const EMenuSection &InMenuSection);
};