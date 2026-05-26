//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#include "UI/ConsoleIO/ConsoleIO.h"

#include "UI/UISnapshot.h"
#include "Engine/CommandDataTable.h"
#include "Engine/Engine.h"
#include "Engine/StringTemplates.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <locale>
#include <cstdlib>
#include <cwchar>

MConsoleIO::MConsoleIO() 
{
    StatusWindow = nullptr;
    OutputWindow = nullptr;
    InputWindow = nullptr;
    StatusWindowHeight = MinimumStatusWindowHeight;
    OutputWindowHeight = 0;
    InputWindowHeight = MinimumInputWindowHeight;
    OutputVerticalScrollOffset = 0;
    CommandHistoryIndex = 0;
    bTUIActive = false;
    bTerminalTooSmall = false;
}

void MConsoleIO::InitTUI()
{
    if (!IsUTF8Locale())
    {
        std::cout << "TerminalAudioPlayer requires a UTF-8 locale." << std::endl;
        std::cout << "Example: LANG=en_US.UTF-8 ./terminal_player" << std::endl;
        return;
    }
    
    initscr();
    cbreak(); // читать сразу
    noecho(); // символы не печатаются автоматически
    curs_set(1); //курсор виден
    
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, nullptr);

    bTUIActive = true;
    
    int MaxY = 0;
    int MaxX = 0;
    getmaxyx(stdscr, MaxY, MaxX);
    
    if (HandleTerminalTooSmall(MaxX, MaxY)) return;
    
    FTUILayout Layout = BuildTUILayout(MaxX, MaxY);
    CreateTUIWindows(Layout);
    
    PrintOutputMessage("Type <" + ct::CommandTypeToString(ECommandType::Help) + "> to show available commands.");
}

void MConsoleIO::ResizeTUI()
{
    if (!bTUIActive) return;
    
    DeleteTUIWindows();
    
    clear();
    refresh();
    
    int MaxX = 0;
    int MaxY = 0;
    getmaxyx(stdscr, MaxY, MaxX);
    
    if (HandleTerminalTooSmall(MaxX, MaxY)) return;
    
    FTUILayout Layout = BuildTUILayout(MaxX, MaxY);
    CreateTUIWindows(Layout);
    
    RebuildOutputRenderLines();
    RenderOutputWindow();
}

void MConsoleIO::ShutDownTUI()
{
    DeleteTUIWindows();
    bTUIActive = false;
    endwin();
}

void MConsoleIO::RenderStatusWindow(const FUISnapshotData &UISnapshot, const FTrackInfo &TrackInfo)
{
    werase(StatusWindow);
    box(StatusWindow, 0, 0);
    
    std::string ProgressBar = BuildProgressBar(TrackInfo.PositionSec, TrackInfo.DurationSec, 20);
    std::wstring Test = ConvertUtf8ToWide("│ ├── └──");
    
    mvwprintw(StatusWindow, 1, 2, "[State: %s]", ct::AudioPlayerStateToString(UISnapshot.AudioPlayerState).c_str());
    mvwprintw(StatusWindow, 2, 2, "[Mode: %s]", ct::PlaybackModeToString(UISnapshot.PlaybackMode).c_str());
    mvwprintw(StatusWindow, 3, 2, "[Volume: %s/%s]", 
        std::to_string(static_cast<int>(UISnapshot.Volume)).c_str(),
        std::to_string(static_cast<int>(gp::MAX_VOLUME)).c_str());
    
    mvwprintw(StatusWindow, 4, 2, "[Active PlayList: %s", UISnapshot.ActiveTrackListName.c_str());
    
    mvwprintw(StatusWindow, 5, 2, "[%s %s]", 
        FormatTime(TrackInfo.DurationSec).c_str(),
        UISnapshot.CurrentTrackName.c_str());
    
    mvwprintw(StatusWindow, 6, 2, "[%s %s %s]", 
        FormatTime(TrackInfo.PositionSec).c_str(), 
        ProgressBar.c_str(), 
        FormatTime(TrackInfo.RemainingSec).c_str());

    wrefresh(StatusWindow);
}

void MConsoleIO::RenderInputWindow(const std::wstring &InputBuffer, std::size_t CursorIndex)
{
    werase(InputWindow);
    box(InputWindow, 0, 0);
    
    const int PromptStartX = 2;
    const std::string Prompt = "Enter command: ";
    const int TextStartX = PromptStartX + static_cast<int>(Prompt.size());
    const FWindowTextArea InputTextArea = BuildWindowTextArea(InputWindow);
    const int MaxPrintableColumns = InputTextArea.Width - static_cast<int>(Prompt.size());

    mvwprintw(InputWindow, 1, PromptStartX, "%s", Prompt.c_str());
    
    if (MaxPrintableColumns > 0)
    {
        mvwaddnwstr(InputWindow, 1, TextStartX, InputBuffer.c_str(), MaxPrintableColumns);
    }

    wmove(InputWindow, 1, TextStartX + static_cast<int>(CursorIndex));
    wrefresh(InputWindow);
}

FCommand MConsoleIO::ParseCommandLine(const std::string &Input)
{
    FCommand Command;

    std::vector<std::string> Tokens;
    std::istringstream Stream(Input);
    std::string Word;

    while (Stream >> Word)
    {
        Tokens.push_back(Word);
    }

    if (Tokens.empty()) return {}; //за нихуя ретурн нихуя =)
    
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Play)) 
    {
        Command.Type = ECommandType::Play;
        for (std::size_t i = 1; i < Tokens.size(); i++)
        {
            Command.Args.emplace_back(Tokens[i]);
        }
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Pause)) 
    {
        Command.Type = ECommandType::Pause;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Stop)) 
    {
        Command.Type = ECommandType::Stop;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Next)) 
    {
        Command.Type = ECommandType::Next;
        for (std::size_t i = 1; i < Tokens.size(); i++)
        {
            Command.Args.emplace_back(Tokens[i]);
        }
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Prev)) 
    {
        Command.Type = ECommandType::Prev;
        for (std::size_t i = 1; i < Tokens.size(); i++)
        {
            Command.Args.emplace_back(Tokens[i]);
        }
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::List)) 
    {
        Command.Type = ECommandType::List;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Refresh)) 
    {
        Command.Type = ECommandType::Refresh;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Exit)) 
    {
        Command.Type = ECommandType::Exit;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Help)) 
    {
        Command.Type = ECommandType::Help;
        if (Tokens.size() >= 2) Command.Args.emplace_back(Tokens[1]);
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Mode)) 
    {
        Command.Type = ECommandType::Mode;
        if (Tokens.size() >= 2) Command.Args.emplace_back(Tokens[1]);
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Volume)) 
    {
        Command.Type = ECommandType::Volume;
        if (Tokens.size() >= 2) Command.Args.emplace_back(Tokens[1]);
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Status)) 
    {
        Command.Type = ECommandType::Status;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Find)) 
    {
        Command.Type = ECommandType::Find;
        if (Tokens.size() >= 2) Command.Args.emplace_back(Tokens[1]);
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Scan)) 
    {
        Command.Type = ECommandType::Scan;
        if (Tokens.size() < 2) 
            return Command;
        
        bool bRecursiveScan = Tokens[1] == ct::CommandFlagToString(ECommandFlag::Recursive);
        
        std::size_t PathStartIndex = bRecursiveScan ? 2 : 1;
        
        if (PathStartIndex >= Tokens.size()) 
            return Command;
        
        std::string PathArg = Tokens[PathStartIndex];
        
        for (std::size_t i = PathStartIndex + 1; i < Tokens.size(); i++)
        {
            PathArg += " " + Tokens[i];
        }
        
        if (bRecursiveScan) 
            Command.Args.emplace_back(ct::CommandFlagToString(ECommandFlag::Recursive));
            
        Command.Args.emplace_back(PathArg);
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Playlist)) 
    {
        Command.Type = ECommandType::Playlist;
        for (std::size_t i = 1; i < Tokens.size(); i++)
        {
            Command.Args.emplace_back(Tokens[i]);
        }
    }
    
    Command.RawInput = Input;
    WriteCommandToHistory(Command.RawInput);
    return Command;
}

FTUIInputEvent MConsoleIO::ReadInputEvent()
{
    FTUIInputEvent Event = {};
    
    WINDOW *SourceWindow = InputWindow;
    if (SourceWindow == nullptr)
    {
        SourceWindow = stdscr;
    }
    
    wint_t Input = 0;
    int Result = wget_wch(SourceWindow, &Input);
    
    if (Result == ERR) return Event;
    
    Event.bHasInput = true;
    
    if (Result == KEY_CODE_YES)
    {
        Event.bIsSpecialKey = true;
        Event.KeyCode = static_cast<int>(Input);
        
        if (Event.KeyCode == KEY_MOUSE)
        {
            MEVENT MouseEvent;
            
            if (getmouse(&MouseEvent) == OK)
            {
                Event.bIsMouseWheelUp = (MouseEvent.bstate & BUTTON4_PRESSED) != 0;
#ifdef BUTTON5_PRESSED
                Event.bIsMouseWheelDown = (MouseEvent.bstate & BUTTON5_PRESSED) != 0;
#endif
            }
        }
    }
    else
    {
        Event.bIsSpecialKey = false;
        Event.Character = static_cast<wchar_t>(Input);
    }
    
    return Event;
}

void MConsoleIO::ScrollOutputWindowVertical(int DeltaLines)
{
    OutputVerticalScrollOffset += DeltaLines;
    
    if (OutputVerticalScrollOffset < 0) 
        OutputVerticalScrollOffset = 0;
    
    const FWindowTextArea OutputTextArea = BuildWindowTextArea(OutputWindow);
    const int MaxPrintableLines = OutputTextArea.Height;
    int MaxScrollOffset = static_cast<int>(LastOutputRenderLines.size()) - MaxPrintableLines;
    
    if (MaxScrollOffset < 0) 
        MaxScrollOffset = 0;
    
    if (OutputVerticalScrollOffset > MaxScrollOffset)
        OutputVerticalScrollOffset = MaxScrollOffset;
    
    RenderOutputWindow();
}

std::wstring MConsoleIO::GetNextCommandFromHistory()
{
    if (CommandHistory.empty()) return L"";
    
    if (CommandHistoryIndex < static_cast<int>(CommandHistory.size()))
    {
        CommandHistoryIndex++;
    }
    
    if (CommandHistoryIndex == static_cast<int>(CommandHistory.size()))
    {
        return L"";
    }
    
    return ConvertUtf8ToWide(CommandHistory[CommandHistoryIndex]);
}

std::wstring MConsoleIO::GetPreviousCommandFromHistory()
{
    if (CommandHistory.empty()) return L"";
    
    if (CommandHistoryIndex > 0)
    {
        CommandHistoryIndex--;
    }
    
    return ConvertUtf8ToWide(CommandHistory[CommandHistoryIndex]);
}

void MConsoleIO::PrintTrackList(const FUISnapshotData &UISnapshot)
{
    std::vector<std::string> Lines;
    Lines.emplace_back(std::string(stp::msg::APP_LIST) + stp::msg::fnc::APP_FNC_TOTAL_TRACKS + std::to_string(UISnapshot.TrackCount));
    Lines.emplace_back("Active list: " + UISnapshot.ActiveTrackListName);

    for (int i = 0; i < UISnapshot.TrackCount; i++)
    {
        std::string CurrentTrack = (i == UISnapshot.TrackIndex) ? stp::msg::fnc::APP_FNC_CUR_TRACK : "";
        std::string SubCatTorL = (i == UISnapshot.TrackCount - 1) ? stp::sep::SUBCAT_SEP_L : stp::sep::SUBCAT_SEP_T;

        Lines.emplace_back(stp::sep::SUBCAT_SEP_TAB + SubCatTorL
            + "Index(" + std::to_string(i) + ")"
            + "[" + UISnapshot.TrackList[i] + "] "
            + CurrentTrack);
    }

    PrintOutputLines(Lines);
}

void MConsoleIO::PrintTotalTracksNum(const int &InTotalTracks) 
{
    PrintOutputMessage(std::string(stp::msg::APP_LIBRARY) + stp::msg::fnc::APP_FNC_LIB_RESET
        + stp::msg::fnc::APP_FNC_TOTAL_TRACKS + std::to_string(InTotalTracks) + ".");
}

void MConsoleIO::PrintCommandHelp()
{
    std::vector<FHelpEntry> HelpEntries =
    {
        { ct::CommandTypeToString(ECommandType::Play), "Resume, start, or queue playback" },
        { ct::CommandTypeToString(ECommandType::Pause), "Pause current track" },
        { ct::CommandTypeToString(ECommandType::Stop), "Stop current track" },
        { ct::CommandTypeToString(ECommandType::Next), "Play or queue next track" },
        { ct::CommandTypeToString(ECommandType::Prev), "Play or queue previous track" },
        { ct::CommandTypeToString(ECommandType::List), "Show track list" },
        { ct::CommandTypeToString(ECommandType::Refresh), "Rescan track library" },
        { ct::CommandTypeToString(ECommandType::Scan) + " " + ct::RequiredArgDataTypeToString(ECommandType::Scan),
            "Scan directory into buffer list" },
        { ct::CommandTypeToString(ECommandType::Playlist) + " " + ct::RequiredArgDataTypeToString(ECommandType::Playlist),
            "Manage track lists" },
        { ct::CommandTypeToString(ECommandType::Mode) + " " + ct::RequiredArgDataTypeToString(ECommandType::Mode),
            "Set playback mode" },
        { ct::CommandTypeToString(ECommandType::Volume) + " " + ct::RequiredArgDataTypeToString(ECommandType::Volume),
            "Set global player volume" },
        { ct::CommandTypeToString(ECommandType::Status), "Show player status" },
        { ct::CommandTypeToString(ECommandType::Help),"Show this help" },
        { ct::CommandTypeToString(ECommandType::Help) + " " + ct::RequiredArgDataTypeToString(ECommandType::Help),
        "Show detailed description of " + ct::RequiredArgDataTypeToString(ECommandType::Help) },
        { ct::CommandTypeToString(ECommandType::Exit),"Exit player" }
    };
    
    size_t MaxUsageLen = 0;
    
    for (const FHelpEntry& HelpEntry : HelpEntries)
    {
        if (HelpEntry.Usage.size() > MaxUsageLen)
        {
            MaxUsageLen = HelpEntry.Usage.size();
        }
    }
    
    std::vector<std::string> Lines;
    Lines.emplace_back(std::string(stp::msg::APP_HELP) + "Available commands:");
    
    for (const FHelpEntry& HelpEntry : HelpEntries)
    {
        size_t Padding = MaxUsageLen - HelpEntry.Usage.size() + static_cast<int>(std::strlen(stp::sep::SUBCAT_SEP_TAB));
        Lines.emplace_back(HelpEntry.Usage + std::string(Padding, ' ') + HelpEntry.Description);
    }
    
    PrintOutputLines(Lines);
}

void MConsoleIO::PrintCommandHelpArg(ECommandType CommandType)
{
    FHelpEntryEXT HelpEntryEXT = {};
    
    switch (CommandType)
    {
        case ECommandType::Play:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Play));
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Play) + " <index>");
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Play) + " " +
                ct::CommandFlagToString(ECommandFlag::Deferred) + " <index>");
            
            HelpEntryEXT.Description.emplace_back("Resumes playback if the current track is paused.");
            HelpEntryEXT.Description.emplace_back("Starts the currently selected track if the player is idle.");
            HelpEntryEXT.Description.emplace_back("play <index> immediately starts a track by index from the active list.");
            HelpEntryEXT.Description.emplace_back("play -d <index> defers the indexed track until the current track finishes.");
            HelpEntryEXT.Description.emplace_back("If nothing is playing, deferred play starts immediately.");
            
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Play));
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Play) + " 4");
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Play) + " " +
                ct::CommandFlagToString(ECommandFlag::Deferred) + " 4");
            break;
        }
        case ECommandType::Pause:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Pause));
            
            HelpEntryEXT.Description.emplace_back("Pauses the currently active track.");
            
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Pause));
            break;
        }
        case ECommandType::Stop:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Stop));
            
            HelpEntryEXT.Description.emplace_back("Stops the currently active track.");
            
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Stop));
            break;
        }
        case ECommandType::Next:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Next));
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Next) + " " +
                ct::CommandFlagToString(ECommandFlag::Deferred));
            
            HelpEntryEXT.Description .emplace_back("Initiates playback of the next track in the list. If there is no "
                "next track, it initiates playback of the very first track in the list.");
            HelpEntryEXT.Description.emplace_back("next -d defers the next track until the current track finishes.");
            HelpEntryEXT.Description.emplace_back("If nothing is playing, deferred next starts immediately.");
            
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Next));
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Next) + " " +
                ct::CommandFlagToString(ECommandFlag::Deferred));
            break;
        }
        case ECommandType::Prev:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Prev));
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Prev) + " " +
                ct::CommandFlagToString(ECommandFlag::Deferred));
            
            HelpEntryEXT.Description.emplace_back("Initiates playback of the previous track in the list. "
                "If there is no previous track, it initiates playback of the very last track in the list.");
            HelpEntryEXT.Description.emplace_back("prev -d defers the previous track until the current track finishes.");
            HelpEntryEXT.Description.emplace_back("If nothing is playing, deferred prev starts immediately.");
            
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Prev));
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Prev) + " " +
                ct::CommandFlagToString(ECommandFlag::Deferred));
            break;
        }
        case ECommandType::List:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::List));
            
            HelpEntryEXT.Description.emplace_back("Shows a complete list of known and valid tracks supported by the"
                " player.");
            
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::List));
            break;
        }
        case ECommandType::Refresh:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Refresh));
            
            HelpEntryEXT.Description.emplace_back("Refreshes the track list. Use this if you've added a new track to "
                "the default directory.");
            
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Refresh));
            break;
        }
        case ECommandType::Exit:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Exit));
            
            HelpEntryEXT.Description.emplace_back("Immediately terminates the program.");
            
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Exit));
            break;
        }
        case ECommandType::Help:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Help));
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Help) + " " + 
                ct::RequiredArgDataTypeToString(ECommandType::Help));
            
            HelpEntryEXT.Description.emplace_back("Show general help or help for one specific command.");
            
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Help));
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Help) + " " + 
                ct::CommandTypeToString(ECommandType::Volume));
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Help) + " " + 
                ct::CommandTypeToString(ECommandType::Mode));
            break;
        }
        case ECommandType::Mode:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Mode) + " " + 
                ct::RequiredArgDataTypeToString(ECommandType::Mode));
            
            {
                HelpEntryEXT.Description.emplace_back("Set playback mode for the player. The <" + 
                    ct::PlaybackModeToString(EPlaybackMode::Once) + "> argument switches the player to single-play mode."
                    " This means the currently active track will play exactly once.");
                HelpEntryEXT.Description.emplace_back("");
            }
            
            {
                HelpEntryEXT.Description.emplace_back("The <" + ct::PlaybackModeToString(EPlaybackMode::LoopOne) + "> "
                    "argument puts the player into loop mode for the currently active track. This is almost the same as "
                    "<" + ct::PlaybackModeToString(EPlaybackMode::Once) + ">, except it plays one specific track repeatedly.");
                HelpEntryEXT.Description.emplace_back("");
            }
                
            {
                HelpEntryEXT.Description.emplace_back("The <" + ct::PlaybackModeToString(EPlaybackMode::LoopAll) + "> "
                    "argument puts the player into a looped playback mode, playing the track list from start to finish,"
                    " then restarting from the beginning after the last track in the list has played. This creates an "
                    "endless loop.");
                HelpEntryEXT.Description.emplace_back("");
            }
            
            {
                HelpEntryEXT.Description.emplace_back("The <" + ct::PlaybackModeToString(EPlaybackMode::LoopShuffle) + "> "
                    "argument puts the player into a looped playback mode of the list of all tracks, but each subsequent"
                    " track is selected randomly.");
            }
            
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Mode) + " " + 
                ct::PlaybackModeToString(EPlaybackMode::Once));
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Mode) + " " + 
                ct::PlaybackModeToString(EPlaybackMode::LoopOne));
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Mode) + " " + 
                ct::PlaybackModeToString(EPlaybackMode::LoopAll));
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Mode) + " " + 
                ct::PlaybackModeToString(EPlaybackMode::LoopShuffle));
            break;
        }
        case ECommandType::Volume:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Volume) +" " + 
                ct::RequiredArgDataTypeToString(ECommandType::Volume));
            
            HelpEntryEXT.Description.emplace_back("Sets the global volume modifier as a percentage from " +
                std::to_string(static_cast<int>(gp::MIN_VOLUME)) + " to " + 
                std::to_string(static_cast<int>(gp::MAX_VOLUME)) + ".");
            
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Volume) + " 100");
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Volume) + " 50");
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Volume) + " 23.99");
            break;
        }
        case ECommandType::Status:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Status));
            
            HelpEntryEXT.Description.emplace_back("Displays the current state of the player:");
            HelpEntryEXT.Description.emplace_back("-playback mode");
            HelpEntryEXT.Description.emplace_back("-status");
            HelpEntryEXT.Description.emplace_back("-current track");
            HelpEntryEXT.Description.emplace_back("-global volume modifier");
            HelpEntryEXT.Description.emplace_back("-total duration in sec");
            HelpEntryEXT.Description.emplace_back("-current position in sec");
            HelpEntryEXT.Description.emplace_back("-remaining playback time of the current track.");
      
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Status));
            break;
        }
        case ECommandType::Find:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Find) + " " + 
                ct::RequiredArgDataTypeToString(ECommandType::Find));
            HelpEntryEXT.Description.emplace_back("Finds tracks whose names contain the " + 
                ct::RequiredArgDataTypeToString(ECommandType::Find) + " argument and displays a list.");
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Find) + " raskol");
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Find) + " honey");
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Find) + " abb");
            break;
        }
        case ECommandType::Scan:
        {
            HelpEntryEXT.Usage.emplace_back("scan <path>");
            HelpEntryEXT.Usage.emplace_back("scan " + ct::CommandFlagToString(ECommandFlag::Recursive) + " <path>");
            
            HelpEntryEXT.Description.emplace_back("Scans a directory for supported audio files and stores the result in the buffer list.");
            HelpEntryEXT.Description.emplace_back("scan <path> checks only files directly inside the target directory.");
            HelpEntryEXT.Description.emplace_back("scan " + ct::CommandFlagToString(ECommandFlag::Recursive) + " <path> checks the target directory and all nested directories.");
            HelpEntryEXT.Description.emplace_back("The buffer is temporary and is replaced by the next scan command.");
            HelpEntryEXT.Description.emplace_back("After scanning, the active list is switched to buffer so list/play can inspect it.");
            HelpEntryEXT.Description.emplace_back("Use pl all add <index|all> from buffer to save scanned tracks into the all list.");
            HelpEntryEXT.Description.emplace_back("Paths starting with ~ are expanded to the current user's home directory.");
            
            HelpEntryEXT.Examples.emplace_back("scan ~/Music");
            HelpEntryEXT.Examples.emplace_back("scan " + ct::CommandFlagToString(ECommandFlag::Recursive) + " ~/Music");
            HelpEntryEXT.Examples.emplace_back("scan ~/Downloads/Music");
            HelpEntryEXT.Examples.emplace_back("scan /mnt/storage/audio");
            break;
        }
        case ECommandType::Playlist:
        {
            HelpEntryEXT.Usage.emplace_back("pl list");
            HelpEntryEXT.Usage.emplace_back("pl current");
            HelpEntryEXT.Usage.emplace_back("pl use <buffer|all|favorite|name|index>");
            HelpEntryEXT.Usage.emplace_back("pl create <name>");
            HelpEntryEXT.Usage.emplace_back("pl delete <name>");
            HelpEntryEXT.Usage.emplace_back("pl <list> list");
            HelpEntryEXT.Usage.emplace_back("pl all add <index|all> from buffer");
            HelpEntryEXT.Usage.emplace_back("pl all remove <index>");
            HelpEntryEXT.Usage.emplace_back("pl favorite add <index>");
            HelpEntryEXT.Usage.emplace_back("pl favorite add <index> from <buffer|all|name>");
            HelpEntryEXT.Usage.emplace_back("pl favorite remove <index>");
            HelpEntryEXT.Usage.emplace_back("pl <name> add <index|all>");
            HelpEntryEXT.Usage.emplace_back("pl <name> add <index|all> from <buffer|all|favorite|name>");
            HelpEntryEXT.Usage.emplace_back("pl <name> remove <index>");
            
            HelpEntryEXT.Description.emplace_back("Manages track lists.");
            HelpEntryEXT.Description.emplace_back("Built-in lists are: 0 buffer, 1 all, 2 favorite.");
            HelpEntryEXT.Description.emplace_back("Custom playlists start at index 3.");
            HelpEntryEXT.Description.emplace_back("buffer is temporary scan output.");
            HelpEntryEXT.Description.emplace_back("all is the persistent list of tracks known by the player.");
            HelpEntryEXT.Description.emplace_back("favorite contains favorite tracks and every favorite track is also added to all.");
            HelpEntryEXT.Description.emplace_back("pl use changes the active list used by list/play/next/prev.");
            HelpEntryEXT.Description.emplace_back("add without from uses the current active list as the source.");
            HelpEntryEXT.Description.emplace_back("all, favorite, custom lists, active list, volume, playback mode, and paused track position are saved to state.txt.");
            
            HelpEntryEXT.Examples.emplace_back("pl list");
            HelpEntryEXT.Examples.emplace_back("pl use buffer");
            HelpEntryEXT.Examples.emplace_back("pl all add all from buffer");
            HelpEntryEXT.Examples.emplace_back("pl favorite add 3");
            HelpEntryEXT.Examples.emplace_back("pl favorite add 3 from buffer");
            HelpEntryEXT.Examples.emplace_back("pl create raskol");
            HelpEntryEXT.Examples.emplace_back("pl raskol add 5");
            HelpEntryEXT.Examples.emplace_back("pl raskol add 5 from all");
            HelpEntryEXT.Examples.emplace_back("pl use raskol");
            break;
        }
    }
    
    std::vector<std::string> Lines;
    Lines.emplace_back(stp::msg::APP_HELP + ct::CommandTypeToString(CommandType));
    Lines.emplace_back(" Usage:");
    for (const auto &Usage : HelpEntryEXT.Usage)
    {
        Lines.emplace_back("  " + Usage);
    }
    
    Lines.emplace_back("");
    Lines.emplace_back(" Description:");
    for (const auto &Description : HelpEntryEXT.Description)
    {
        Lines.emplace_back("  " + Description);
    }
    
    Lines.emplace_back("");
    Lines.emplace_back(" Examples:");
    for (const auto &Example : HelpEntryEXT.Examples)
    {
        Lines.emplace_back("  " + Example);
    }

    PrintOutputLines(Lines);
}

void MConsoleIO::PrintStatus(const FUISnapshotData &UISnapshot, const FTrackInfo &TrackInfo)
{
    std::string State;
    std::string Mode;
    std::string VolumeStr;
    
    switch (UISnapshot.AudioPlayerState)
    {
        case EAudioPlayerState::Idle:
        {
            State = "Idle";
            break;
        }
        case EAudioPlayerState::Playing:
        {
            State = "Playing";
            break;
        }
        case EAudioPlayerState::Paused:
        {
            State = "Paused";
            break;
        }
        case EAudioPlayerState::None:
        {
            State = "None";
            break;
        }
    }
    
    switch (UISnapshot.PlaybackMode)
    {
        case EPlaybackMode::Once:
        {
            Mode = "Once";
            break;
        }
        case EPlaybackMode::LoopOne:
        {
            Mode = "Loop One";
            break;
        }
        case EPlaybackMode::LoopAll:
        {
            Mode = "Loop All";
            break;
        }
        case EPlaybackMode::LoopShuffle:
        {
            Mode = "Loop Shuffle";
            break;
        }
        case EPlaybackMode::None:
        {
            Mode = "None";
            break;
        }
    }

    if (std::floor(UISnapshot.Volume) == UISnapshot.Volume)
    {
        VolumeStr = std::to_string(static_cast<int>(UISnapshot.Volume));
    }
    else
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << UISnapshot.Volume;
        VolumeStr = oss.str();
    }
    
    float DurationSec = TrackInfo.DurationSec;
    float CurrentOffsetSec = TrackInfo.PositionSec;
    float RemainingSec = TrackInfo.RemainingSec;
    
    std::vector<std::string> Lines;
    Lines.emplace_back(stp::msg::APP_STATUS);
    Lines.emplace_back("  [State: " + State + "]");
    Lines.emplace_back("  [Playback Mode: " + Mode + "]");
    Lines.emplace_back("  [Current Track: " + UISnapshot.CurrentTrackName + "]");
    Lines.emplace_back("  [Duration: " + FormatTime(DurationSec) + "]");
    Lines.emplace_back("  [Position: " + FormatTime(CurrentOffsetSec) + "]");
    Lines.emplace_back("  [Remaining: " + FormatTime(RemainingSec) + "]");
    Lines.emplace_back("  [Volume: " + VolumeStr + "/" + std::to_string(static_cast<int>(gp::MAX_VOLUME)) + "]");
    PrintOutputLines(Lines);
}

void MConsoleIO::PrintFindResults(const std::vector<std::pair<int, std::string>> &FoundTracks)
{
    if (FoundTracks.empty())
    {
        PrintOutputMessage(std::string(stp::msg::APP_FIND) + "No matches found.");
        return;
    }

    std::vector<std::string> Lines;
    Lines.emplace_back(std::string(stp::msg::APP_FIND) + "Results: " + std::to_string(FoundTracks.size()));

    for (size_t i = 0; i < FoundTracks.size(); i++)
    {
        const std::string SubCatTorL = (i == FoundTracks.size() - 1) ? stp::sep::SUBCAT_SEP_L : stp::sep::SUBCAT_SEP_T;

        Lines.emplace_back(stp::sep::SUBCAT_SEP_TAB + SubCatTorL
            + "Index(" + std::to_string(FoundTracks[i].first) + ")"
            + "[" + FoundTracks[i].second + "]");
    }

    PrintOutputLines(Lines);
}

void MConsoleIO::PrintOutputMessage(const std::string &Message)
{
    PrintOutputLines({ Message });
}

void MConsoleIO::DeleteTUIWindows()
{
    if (StatusWindow != nullptr)
    {
        delwin(StatusWindow);
        StatusWindow = nullptr;
    }
    if (OutputWindow != nullptr)
    {
        delwin(OutputWindow);
        OutputWindow = nullptr;
    }
    if (InputWindow != nullptr)
    {
        delwin(InputWindow);
        InputWindow = nullptr;
    }
}

void MConsoleIO::CreateTUIWindows(const FTUILayout &Layout)
{
    StatusWindowHeight = Layout.StatusHeight;
    OutputWindowHeight = Layout.OutputHeight;
    InputWindowHeight = Layout.InputHeight;
    
    StatusWindow = newwin(StatusWindowHeight, Layout.Width, 0, 0);
    OutputWindow = newwin(OutputWindowHeight, Layout.Width, StatusWindowHeight, 0);
    InputWindow = newwin(InputWindowHeight, Layout.Width, StatusWindowHeight + OutputWindowHeight, 0);

    leaveok(StatusWindow, TRUE);
    leaveok(OutputWindow, TRUE);
    leaveok(InputWindow, FALSE);
    
    scrollok(StatusWindow, FALSE);
    scrollok(OutputWindow, FALSE);
    scrollok(InputWindow, FALSE);
    
    keypad(InputWindow, TRUE);
    nodelay(InputWindow, TRUE);
    
    RenderOutputWindow();
}

void MConsoleIO::PrintOutputLines(const std::vector<std::string> &Lines)
{
    SetOutputLines(Lines);
    
    if (!bTUIActive || OutputWindow == nullptr)
    {
        return;
    }

    RenderOutputWindow();
}

void MConsoleIO::SetOutputLines(const std::vector<std::string> &Lines)
{
    LastOutputLines = Lines;
    OutputVerticalScrollOffset = 0;
    RebuildOutputRenderLines();
}

void MConsoleIO::RebuildOutputRenderLines()
{
    LastOutputRenderLines.clear();
    
    const FWindowTextArea OutputTextArea = BuildWindowTextArea(OutputWindow);
    const int MaxPrintableColumns = OutputTextArea.Width;
    
    for (const std::string &Line : LastOutputLines)
    {
        std::wstring WideLine = ConvertUtf8ToWide(Line);
        AppendWrappedOutputRenderLines(WideLine, MaxPrintableColumns);
    }
}

void MConsoleIO::AppendWrappedOutputRenderLines(const std::wstring &Line, int MaxPrintableColumns)
{
    if (Line.empty())
    {
        LastOutputRenderLines.emplace_back(L"");
        return;
    }
    
    if (MaxPrintableColumns <= 0)
    {
        LastOutputRenderLines.push_back(Line);
        return;
    }
    
    std::size_t IndentLength = 0; 
    while (IndentLength < Line.size() && Line[IndentLength] == L' ')
    {
        IndentLength++;
    }
    std::wstring Indent = Line.substr(0, IndentLength);
    bool bFirstVisualLine = true;
    
    auto AppendPiece = [&](const std::wstring &Piece)
    {
        if (bFirstVisualLine)
        {
            LastOutputRenderLines.push_back(Piece);
            bFirstVisualLine = false;
        }
        else
        {
            LastOutputRenderLines.push_back(Indent + Piece);
        }
    };
    
    std::size_t Start = 0;
    while (Start < Line.size())
    {
        int CurrentMaxColumns = MaxPrintableColumns;

        if (!bFirstVisualLine)
        {
            CurrentMaxColumns -= static_cast<int>(Indent.size());
        }

        if (CurrentMaxColumns <= 0)
        {
            CurrentMaxColumns = MaxPrintableColumns;
        }

        const std::size_t Remaining = Line.size() - Start;

        if (Remaining <= static_cast<std::size_t>(CurrentMaxColumns))
        {
            AppendPiece(Line.substr(Start));
            break;
        }

        const std::size_t WrapEnd = Start + static_cast<std::size_t>(CurrentMaxColumns);

        std::size_t SpacePos = Line.rfind(L' ', WrapEnd);

        if (SpacePos != std::wstring::npos && SpacePos > Start)
        {
            AppendPiece(Line.substr(Start, SpacePos - Start));
            Start = SpacePos + 1;
        }
        else
        {
            AppendPiece(Line.substr(Start, static_cast<std::size_t>(CurrentMaxColumns)));
            Start += static_cast<std::size_t>(CurrentMaxColumns);
        }
    }
}

void MConsoleIO::RenderOutputWindow()
{
    if (!bTUIActive || OutputWindow == nullptr) return;
    
    werase(OutputWindow);
    box(OutputWindow, 0, 0);
    
    const FWindowTextArea OutputTextArea = BuildWindowTextArea(OutputWindow);
    
    const int MaxPrintableColumns = OutputTextArea.Width;
    const int MaxPrintableLines = OutputTextArea.Height;
    
    if (MaxPrintableLines <= 0 || MaxPrintableColumns <= 0)
    {
        wrefresh(OutputWindow);
        return;
    }
    
    for (int i = 0; i < MaxPrintableLines; i++)
    {
        const int SourceIndex = OutputVerticalScrollOffset + i;
        
        
        if (SourceIndex >= static_cast<int>(LastOutputRenderLines.size()))
        {
            break;
        }

        const std::wstring& Line = LastOutputRenderLines[SourceIndex];
        
        mvwaddnwstr(
            OutputWindow,
            OutputTextArea.StartY + i,
            OutputTextArea.StartX,
            Line.c_str(),
            MaxPrintableColumns);
    }
    
    wrefresh(OutputWindow);
}

bool MConsoleIO::HandleTerminalTooSmall(int TerminalWidth, int TerminalHeight)
{
    bTerminalTooSmall = 
        TerminalWidth < MinimumTerminalWidth ||
        TerminalHeight < MinimumTerminalHeight;
    
    if (bTerminalTooSmall)
    {
        RenderTerminalTooSmall(TerminalWidth, TerminalHeight);
        return true;
    }
    
    return false;
}

void MConsoleIO::RenderTerminalTooSmall(int TerminalWidth, int TerminalHeight)
{
    clear();
    mvprintw(1, 2, "TERMINAL TOO SMALL");
    mvprintw(2, 2, "MINIMUM SIZE: %dx%d", MinimumTerminalWidth, MinimumTerminalHeight);
    mvprintw(3, 2, "CURRENT SIZE: %dx%d", TerminalWidth, TerminalHeight);
    refresh();
}

std::string MConsoleIO::BuildProgressBar(float PositionSec, float DurationSec, int BarWidth)
{
    float Progress = 0.f;
    
    if (DurationSec > 0.f)
    {
        Progress = PositionSec / DurationSec;
    }
    
    Progress = std::clamp(Progress, 0.f, 1.f);
    
    int FilledBarWidth = static_cast<int>(Progress * BarWidth);
    int EmptyBarWidth = BarWidth - FilledBarWidth;
    
    return "[" + std::string(FilledBarWidth, '#') + std::string(EmptyBarWidth, '-') + "]";
}

bool MConsoleIO::IsUTF8Locale()
{
    setlocale(LC_ALL, "");

    const char* Locale = setlocale(LC_CTYPE, nullptr);

    if (Locale == nullptr) return false;

    std::string LocaleName = Locale;

    return LocaleName.find("UTF-8") != std::string::npos
        || LocaleName.find("utf8") != std::string::npos
        || LocaleName.find("UTF8") != std::string::npos;
}

std::wstring MConsoleIO::ConvertUtf8ToWide(const std::string &Text) const
{
    if (Text.empty()) return L"";
    
    std::mbstate_t State{};
    const char* Source = Text.c_str();
    
    const std::size_t RequiredSize = std::mbsrtowcs(nullptr, &Source, 0, &State);
    
    if (RequiredSize == static_cast<std::size_t>(-1))
    {
        return L"";
    }
    
    std::wstring Result(RequiredSize, L'\0');

    State = std::mbstate_t{};
    Source = Text.c_str();

    std::mbsrtowcs(Result.data(), &Source, Result.size(), &State);

    return Result;
}

std::string MConsoleIO::ConvertWideToUtf8(const std::wstring &Text) const
{
    if (Text.empty()) return "";
    
    std::mbstate_t State{};
    const wchar_t* Source = Text.c_str();
    
    const std::size_t RequiredSize = std::wcsrtombs(nullptr, &Source, 0, &State);
    
    if (RequiredSize == static_cast<std::size_t>(-1))
    {
        return "";
    }
    
    std::string Result(RequiredSize, '\0');
    
    State = std::mbstate_t{};
    Source = Text.c_str();
    
    std::wcsrtombs(Result.data(), &Source, Result.size(), &State);
    
    return Result;
}

FTUILayout MConsoleIO::BuildTUILayout(int TerminalWidth, int TerminalHeight)
{
    FTUILayout Layout;
    
    Layout.Width = TerminalWidth;
    Layout.StatusHeight = MinimumStatusWindowHeight;
    Layout.InputHeight = MinimumInputWindowHeight;
    Layout.OutputHeight = TerminalHeight - Layout.StatusHeight - Layout.InputHeight;
    
    if (Layout.OutputHeight < MinimumOutputWindowHeight)
    {
        Layout.OutputHeight = MinimumOutputWindowHeight;
        Layout.StatusHeight = TerminalHeight - Layout.InputHeight - Layout.OutputHeight;
    }
    
    return Layout;
}

FWindowTextArea MConsoleIO::BuildWindowTextArea(WINDOW *Window)
{
    FWindowTextArea TextArea = {};
    
    if (Window == nullptr) return TextArea;
    
    int WindowHeight = 0;
    int WindowWidth = 0;
    getmaxyx(Window, WindowHeight, WindowWidth);
    
    TextArea.StartX = 2;
    TextArea.StartY = 1;
    TextArea.Width = WindowWidth - TextArea.StartX*2; // лево и право рамка + отступ
    TextArea.Height = WindowHeight - TextArea.StartY*2; // верх и низ рамка
    
    if (TextArea.Height < 0) TextArea.Height = 0;
    if (TextArea.Width < 0)  TextArea.Width = 0;
    
    return TextArea;
}

void MConsoleIO::WriteCommandToHistory(const std::string &Command)
{
    if (Command.empty())
    {
        CommandHistoryIndex = static_cast<int>(CommandHistory.size());
        return;
    }
    
    bool bOnlySpaces = true;
    
    for (char C : Command)
    {
        if (!std::isspace(static_cast<unsigned char>(C)))
        {
            bOnlySpaces = false;
            break;
        }
    }
    
    if (bOnlySpaces)
    {
        CommandHistoryIndex = static_cast<int>(CommandHistory.size());
        return;
    }
    
    if (!CommandHistory.empty() && CommandHistory.back() == Command)
    {
        CommandHistoryIndex = static_cast<int>(CommandHistory.size());\
        return;
    }
        
    CommandHistory.emplace_back(Command);
    
    constexpr std::size_t MaxCommandHistorySize = 100;
    
    if (CommandHistory.size() > MaxCommandHistorySize)
    {
        CommandHistory.erase(CommandHistory.begin());
    }
    
    CommandHistoryIndex = static_cast<int>(CommandHistory.size());
}

std::string MConsoleIO::FormatTime(float sec)
{
    int Total = static_cast<int>(sec);

    int Hours = Total / 3600;
    int Minutes = (Total % 3600) / 60;
    int Seconds = Total % 60;

    std::ostringstream oss;
    
    oss << std::setw(2) << std::setfill('0') << Hours << ":";
    oss << std::setw(2) << std::setfill('0') << Minutes << ":"
        << std::setw(2) << std::setfill('0') << Seconds;

    return oss.str();
}
