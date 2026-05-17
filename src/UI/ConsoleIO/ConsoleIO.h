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

struct FTUILayout
{
    int StatusHeight = 0;
    int OutputHeight = 0;
    int InputHeight = 0;
    int Width = 0;
};

struct FWindowTextArea
{
    int StartX = 0;
    int StartY = 0;
    int Width = 0;
    int Height = 0;
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
    void PrintFindResults(const std::vector<std::pair<int, std::string>> &FoundTracks);
    void PrintOutputMessage(const std::string &Message);
    //------------------------------------------------------------------------------------------------------------------
    
private:
    
    /** NCURSES ConsoleApp-style */
    //------------------------------------------------------------------------------------------------------------------
    WINDOW* StatusWindow;
    WINDOW* OutputWindow;
    WINDOW* InputWindow;
    
    std::vector<std::string> LastOutputLines;
    std::vector<std::wstring> LastOutputRenderLines;
    
    int StatusWindowHeight;
    int OutputWindowHeight;
    int InputWindowHeight;
    
    int OutputVerticalScrollOffset;

    bool bTUIActive;
    bool bTerminalTooSmall;
    //------------------------------------------------------------------------------------------------------------------
    
    
    /** Standard COUT-CommandLine-style */
    //------------------------------------------------------------------------------------------------------------------
    int CommandHelpIndentation;
    //------------------------------------------------------------------------------------------------------------------
    
    
    /** NCURSES ConsoleApp-style helpers */
    //------------------------------------------------------------------------------------------------------------------
    void DeleteTUIWindows();
    void CreateTUIWindows(const FTUILayout &Layout);
    void PrintOutputLines(const std::vector<std::string> &Lines);
    void SetOutputLines(const std::vector<std::string> &Lines);
    void RebuildOutputRenderLines();
    void AppendWrappedOutputRenderLines(const std::wstring& Line, int MaxPrintableColumns);
    void RenderOutputWindow();
    bool HandleTerminalTooSmall(int TerminalWidth, int TerminalHeight);
    void RenderTerminalTooSmall(int TerminalWidth, int TerminalHeight);
    std::string BuildProgressBar(float PositionSec, float DurationSec, int BarWidth);
    bool IsUTF8Locale();
    std::wstring ConvertUtf8ToWide(const std::string& Text) const;
    FTUILayout BuildTUILayout(int TerminalWidth, int TerminalHeight);
    FWindowTextArea BuildWindowTextArea(WINDOW* Window);
    //------------------------------------------------------------------------------------------------------------------
    
    
    /** Standard COUT-CommandLine-style helpers */
    //------------------------------------------------------------------------------------------------------------------
    std::string FormatTime(float sec);
    //------------------------------------------------------------------------------------------------------------------
};
