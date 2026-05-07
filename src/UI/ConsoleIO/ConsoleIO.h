//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#pragma once 

#include "Types/PlaybackTypes.h"
#include "Types/CommandTypes.h"

#include <memory>
#include <ncurses.h>
#include <string>
#include <vector>

constexpr int MinimumTerminalWidth = 72;
constexpr int MinimumTerminalHeight = 18;
constexpr int MinimumStatusWindowHeight = 7;
constexpr int MinimumOutputWindowHeight = 8;
constexpr int MinimumInputWindowHeight = 3;

struct FUISnapshotData;
struct FTrackInfo;

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
    
    /** NCURSES ConsoleApp-style */
    //------------------------------------------------------------------------------------------------------------------
    void InitTUI();
    void ResizeTUI();
    void ShutDownTUI();
    void RenderStatusWindow(const FUISnapshotData& UISnapshot, const FTrackInfo &TrackInfo);
    void RenderInputWindow(const std::string& InputBuffer);
    FCommand ParseCommandLine(const std::string &Input);
    int ReadInputKey();
    void ScrollOutputWindowVertical(int DeltaLines);
    void ScrollOutputWindowHorizontal(int DeltaColumns);
    bool GetTerminalTooSmall() const { return bTerminalTooSmall; }

    //------------------------------------------------------------------------------------------------------------------
    
    
    /** Standard COUT-CommandLine-style */
    //------------------------------------------------------------------------------------------------------------------
    FCommand ReadCommand();
    void PrintTrackList(const FUISnapshotData &UISnapshot);
    void PrintTotalTracksNum(const int &InTotalTracks);
    void PrintCommandHelp();
    void PrintCommandHelpArg(ECommandType CommandType);
    void PrintStatus(const FUISnapshotData &UISnapshot, const FTrackInfo &TrackInfo);
    void PrintFindResults(const std::vector<std::pair<int, std::string>> &FindedTracks);
    void PrintOutputMessage(const std::string &Message);
    //------------------------------------------------------------------------------------------------------------------
    
private:
    
    /** NCURSES ConsoleApp-style */
    //------------------------------------------------------------------------------------------------------------------
    WINDOW* StatusWindow;
    WINDOW* OutputWindow;
    WINDOW* InputWindow;
    
    std::vector<std::string> LastOutputLines;
    
    int StatusWindowHeight;
    int OutputWindowHeight;
    int InputWindowHeight;
    
    int OutputVerticalScrollOffset;
    int OutputHorizontalScrollOffset;

    bool bTUIActive;
    bool bTerminalTooSmall;
    //------------------------------------------------------------------------------------------------------------------
    
    
    /** Standard COUT-CommandLine-style */
    //------------------------------------------------------------------------------------------------------------------
    int CommandHelpIdentation;
    //------------------------------------------------------------------------------------------------------------------
    
    
    /** NCURSES ConsoleApp-style helpers */
    //------------------------------------------------------------------------------------------------------------------
    void DeleteTUIWindows();
    void PrintOutputLines(const std::vector<std::string> &Lines);
    void RenderOutputWindow();
    bool IsTerminalTooSmall(int Width, int Height);
    std::string BuildProgressBar(float PositionSec, float DuractionSec, int BarWidth);
    //------------------------------------------------------------------------------------------------------------------
    
    
    /** Standard COUT-CommandLine-style helpers */
    //------------------------------------------------------------------------------------------------------------------
    std::string FormatTime(float sec);
    //------------------------------------------------------------------------------------------------------------------
};
