//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#pragma once 

#include "Types/PlaybackTypes.h"
#include "Types/CommandTypes.h"

#include <memory>
#include <ncurses.h>
#include <string>
#include <vector>

constexpr int MinimumTerminalWidth = 72;
constexpr int MinimumTerminalHeight = 19;
constexpr int MinimumStatusWindowHeight = 8;
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

struct FTUIInputEvent
{
    bool bHasInput = false;
    bool bIsSpecialKey = false;
    bool bIsMouseWheelUp = false;
    bool bIsMouseWheelDown = false;
    int KeyCode = 0;
    wchar_t Character = L'\0';
};

class MConsoleIO 
{
public:
    MConsoleIO();
    
    void InitTUI();
    void ResizeTUI();
    void ShutDownTUI();
    void RenderStatusWindow(const FUISnapshotData& UISnapshot, const FTrackInfo &TrackInfo);
    void RenderInputWindow(const std::wstring& InputBuffer, std::size_t CursorIndex);
    FCommand ParseCommandLine(const std::string &Input);
    FTUIInputEvent ReadInputEvent();
    void ScrollOutputWindowVertical(int DeltaLines);
    
    std::wstring GetNextCommandFromHistory();
    std::wstring GetPreviousCommandFromHistory();
    bool GetTerminalTooSmall() const { return bTerminalTooSmall; }
    std::string ConvertWideToUtf8(const std::wstring& Text) const;
    
    void PrintTrackList(const FUISnapshotData &UISnapshot);
    void PrintTotalTracksNum(const int &InTotalTracks);
    void PrintCommandHelp();
    void PrintCommandHelpArg(ECommandType CommandType);
    void PrintStatus(const FUISnapshotData &UISnapshot, const FTrackInfo &TrackInfo);
    void PrintFindResults(const std::vector<std::pair<int, std::string>> &FoundTracks);
    void PrintOutputMessage(const std::string &Message);
    void PrintOutputLines(const std::vector<std::string> &Lines);
private:
    
    WINDOW* StatusWindow;
    WINDOW* OutputWindow;
    WINDOW* InputWindow;
    
    std::vector<std::string> LastOutputLines;
    std::vector<std::wstring> LastOutputRenderLines;
    std::vector<std::string> CommandHistory;
    
    int StatusWindowHeight;
    int OutputWindowHeight;
    int InputWindowHeight;
    
    int OutputVerticalScrollOffset;
    int CommandHistoryIndex;

    bool bTUIActive;
    bool bTerminalTooSmall;

    void DeleteTUIWindows();
    void CreateTUIWindows(const FTUILayout &Layout);
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
    void WriteCommandToHistory(const std::string &Command);
    std::string FormatTime(float sec);
};
