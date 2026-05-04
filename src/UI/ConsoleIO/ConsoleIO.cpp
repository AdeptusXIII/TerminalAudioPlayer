//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#include "UI/ConsoleIO/ConsoleIO.h"

#include "UI/UISnapshot.h"
#include "Engine/CommandDataTable.h"
#include "Engine/Engine.h"
#include "Engine/StringTemplates.h"

#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

MConsoleIO::MConsoleIO() 
{
    StatusWindow = nullptr;
    OutputWindow = nullptr;
    InputWindow = nullptr;
    StatusWindowHeight = MinimumStatusWindowHeight;
    OutputWindowHeight = 0;
    InputWindowHeight = MinimumInputWindowHeight;
    OutputVerticalScrollOffset = 0;
    OutputHorizontalScrollOffset = 0;
    CommandHelpIdentation = static_cast<int>(std::strlen(stp::sep::SUBCAT_SEP_TAB));
    bTUIActive = false;
}

void MConsoleIO::InitTUI()
{
    initscr();
    cbreak(); // читать сразу
    noecho(); // символы не печатаются автоматически
    curs_set(1); //курсор виден

    int MaxY = 0;
    int MaxX = 0;
    getmaxyx(stdscr, MaxY, MaxX);

    if (StatusWindowHeight + InputWindowHeight >= MaxY)
    {
        StatusWindowHeight = MaxY > 6 ? MaxY - 4 : 3;
    }

    OutputWindowHeight = MaxY - StatusWindowHeight - InputWindowHeight;
    if (OutputWindowHeight < MinimumOutputWindowHeight)
    {
        OutputWindowHeight = MinimumOutputWindowHeight;
        StatusWindowHeight = MaxY - InputWindowHeight - OutputWindowHeight;
    }

    StatusWindow = newwin(StatusWindowHeight, MaxX, 0, 0);
    OutputWindow = newwin(OutputWindowHeight, MaxX, StatusWindowHeight, 0);
    InputWindow = newwin(InputWindowHeight, MaxX, StatusWindowHeight + OutputWindowHeight, 0);

    scrollok(StatusWindow, FALSE);
    scrollok(OutputWindow, FALSE);
    scrollok(InputWindow, FALSE);

    keypad(InputWindow, TRUE);
    nodelay(InputWindow, TRUE);

    bTUIActive = true;
}

void MConsoleIO::ResizeTUI()
{
    if (!bTUIActive) return;
    
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
    
    clear();
    refresh();
    
    int MaxX = 0;
    int MaxY = 0;
    getmaxyx(stdscr, MaxY, MaxX);
    
    StatusWindowHeight = MinimumStatusWindowHeight;
    InputWindowHeight = MinimumInputWindowHeight;
    
    if (StatusWindowHeight + InputWindowHeight >= MaxY)
    {
        StatusWindowHeight = MaxY > 6 ? MaxY - 4 : 3;
    }
    
    OutputWindowHeight = MaxY - StatusWindowHeight - InputWindowHeight;
    if (OutputWindowHeight < MinimumOutputWindowHeight)
    {
        OutputWindowHeight = MinimumOutputWindowHeight;
        StatusWindowHeight = MaxY - InputWindowHeight - OutputWindowHeight;
    }
    
    StatusWindow = newwin(StatusWindowHeight, MaxX, 0, 0);
    OutputWindow = newwin(OutputWindowHeight, MaxX, StatusWindowHeight, 0);
    InputWindow = newwin(InputWindowHeight, MaxX, StatusWindowHeight + OutputWindowHeight, 0);

    
    scrollok(StatusWindow, FALSE);
    scrollok(OutputWindow, FALSE);
    scrollok(InputWindow, FALSE);
    
    keypad(InputWindow, TRUE);
    nodelay(InputWindow, TRUE);
    
    RenderOutputWindow();
}

void MConsoleIO::ShutDownTUI()
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

    bTUIActive = false;
    endwin();
}

void MConsoleIO::RenderStatusWindow(const FUISnapshotData &UISnapshot, const FTrackInfo &TrackInfo)
{
    werase(StatusWindow);
    box(StatusWindow, 0, 0);

    mvwprintw(StatusWindow, 1, 2, stp::msg::APP_STATUS_MSG);
    mvwprintw(StatusWindow, 2, 2, "[State: %s]", ct::AudioPlayerStateToString(UISnapshot.AudioPlayerState).c_str());
    mvwprintw(StatusWindow, 3, 2, "[Playback Mode: %s]", ct::PlaybackModeToString(UISnapshot.PlaybackMode).c_str());
    mvwprintw(StatusWindow, 4, 2, "[Current Track: %s]", UISnapshot.CurrentTrackName.c_str());
    mvwprintw(StatusWindow, 5, 2, "[Duration: %s]", FormatTime(TrackInfo.DurationSec).c_str());
    mvwprintw(StatusWindow, 6, 2, "[Position: %s]", FormatTime(TrackInfo.PositionSec).c_str());
    mvwprintw(StatusWindow, 7, 2, "[Remaining: %s]", FormatTime(TrackInfo.RemainingSec).c_str());
    mvwprintw(StatusWindow, 8, 2, "[Volume: %s/%s]", std::to_string(static_cast<int>(gp::MIN_VOLUME)).c_str(),
        std::to_string(static_cast<int>(UISnapshot.Volume)).c_str());

    wrefresh(StatusWindow);
}

void MConsoleIO::RenderInputWindow(const std::string &InputBuffer)
{
    werase(InputWindow);
    box(InputWindow, 0, 0);

    mvwprintw(InputWindow, 1, 2, "Enter command: %s", InputBuffer.c_str());

    wmove(InputWindow, 1, 17 + static_cast<int>(InputBuffer.size()));
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
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Prev)) 
    {
        Command.Type = ECommandType::Prev;
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
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Select)) 
    {
        Command.Type = ECommandType::Select;
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

    Command.RawInput = Input;
    return Command;
}

int MConsoleIO::ReadInputKey()
{
    return wgetch(InputWindow);
}

void MConsoleIO::ScrollOutputWindowVertical(int DeltaLines)
{
    OutputVerticalScrollOffset += DeltaLines;
    
    if (OutputVerticalScrollOffset < 0) 
        OutputVerticalScrollOffset = 0;
    
    const int MaxPrintableLines = OutputWindowHeight - 2;
    int MaxScrollOffset = static_cast<int>(LastOutputLines.size()) - MaxPrintableLines;
    
    if (MaxScrollOffset < 0) 
        MaxScrollOffset = 0;
    
    if (OutputVerticalScrollOffset > MaxScrollOffset)
        OutputVerticalScrollOffset = MaxScrollOffset;
    
    RenderOutputWindow();
}

void MConsoleIO::ScrollOutputWindowHorizontal(int DeltaColumns)
{
    if (OutputWindow == nullptr) return;
    OutputHorizontalScrollOffset += DeltaColumns;
    
    if (OutputHorizontalScrollOffset < 0)
        OutputHorizontalScrollOffset = 0;
    
    int WindowHeight = 0;
    int WindowWidth = 0;
    getmaxyx(OutputWindow, WindowHeight, WindowWidth);
    
    const int TextStartX = 2;
    const int RightPadding = 1;
    const int MaxPrintableColumns = WindowWidth - TextStartX - RightPadding - 1;
    
    int MaxLineLength = 0;

    for (const std::string& Line : LastOutputLines)
    {
        if (static_cast<int>(Line.size()) > MaxLineLength)
        {
            MaxLineLength = static_cast<int>(Line.size());
        }
    }

    int MaxScrollOffset = MaxLineLength - MaxPrintableColumns;

    if (MaxScrollOffset < 0)
        MaxScrollOffset = 0;

    if (OutputHorizontalScrollOffset > MaxScrollOffset)
        OutputHorizontalScrollOffset = MaxScrollOffset;

    RenderOutputWindow();
}

FCommand MConsoleIO::ReadCommand()
{
    std::string Input;
    std::cout << "Enter command: ";
    std::getline(std::cin, Input);
    
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
        Command.RawInput = Input;
        return Command;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Pause)) 
    {
        Command.Type = ECommandType::Pause;
        Command.RawInput = Input;
        return Command;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Stop)) 
    {
        Command.Type = ECommandType::Stop;
        Command.RawInput = Input;
        return Command;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Next)) 
    {
        Command.Type = ECommandType::Next;
        Command.RawInput = Input;
        return Command;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Prev)) 
    {
        Command.Type = ECommandType::Prev;
        Command.RawInput = Input;
        return Command;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::List)) 
    {
        Command.Type = ECommandType::List;
        Command.RawInput = Input;
        return Command;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Refresh)) 
    {
        Command.Type = ECommandType::Refresh;
        Command.RawInput = Input;
        return Command;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Exit)) 
    {
        Command.Type = ECommandType::Exit;
        Command.RawInput = Input;
        return Command;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Help)) 
    {
        Command.Type = ECommandType::Help;
        if (Tokens.size() >= 2) Command.Args.emplace_back(Tokens[1]);
        Command.RawInput = Input;
        return Command;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Mode)) 
    {
        Command.Type = ECommandType::Mode;
        if (Tokens.size() >= 2) Command.Args.emplace_back(Tokens[1]);
        Command.RawInput = Input;
        return Command;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Select)) 
    {
        Command.Type = ECommandType::Select;
        if (Tokens.size() >= 2) Command.Args.emplace_back(Tokens[1]);
        Command.RawInput = Input;
        return Command;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Volume)) 
    {
        Command.Type = ECommandType::Volume;
        if (Tokens.size() >= 2) Command.Args.emplace_back(Tokens[1]);
        Command.RawInput = Input;
        return Command;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Status)) 
    {
        Command.Type = ECommandType::Status;
        Command.RawInput = Input;
        return Command;
    }
    if (Tokens[0] == ct::CommandTypeToString(ECommandType::Find)) 
    {
        Command.Type = ECommandType::Find;
        if (Tokens.size() >= 2) Command.Args.emplace_back(Tokens[1]);
        Command.RawInput = Input;
        return Command;
    }
    
    
    return {};
}

void MConsoleIO::PrintTrackList(const FUISnapshotData &UISnapshot)
{
    std::vector<std::string> Lines;
    Lines.emplace_back(stp::msg::fnc::APP_FNC_TOTAL_TRACKS_MSG + std::to_string(UISnapshot.TrackCount));

    for (int i = 0; i < UISnapshot.TrackCount; i++)
    {
        std::string CurrentTrack = (i == UISnapshot.TrackIndex) ? stp::msg::fnc::APP_FNC_CUR_TRACK_MSG : "";
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
    PrintOutputMessage(std::string(stp::msg::APP_LIBRARY_MSG) + stp::msg::fnc::APP_FNC_LIB_RESET_MSG
        + stp::msg::fnc::APP_FNC_TOTAL_TRACKS_MSG + std::to_string(InTotalTracks) + ".");
}

void MConsoleIO::PrintCommandHelp()
{
    std::vector<FHelpEntry> HelpEntries =
    {
        { ct::CommandTypeToString(ECommandType::Play), "Resume or start playback" },
        { ct::CommandTypeToString(ECommandType::Pause), "Pause current track" },
        { ct::CommandTypeToString(ECommandType::Stop), "Stop current track" },
        { ct::CommandTypeToString(ECommandType::Next), "Play next track" },
        { ct::CommandTypeToString(ECommandType::Prev), "Play previous track" },
        { ct::CommandTypeToString(ECommandType::List), "Show track list" },
        { ct::CommandTypeToString(ECommandType::Refresh), "Rescan track library" },
        { ct::CommandTypeToString(ECommandType::Mode) + " " + ct::RequiredArgDataTypeToString(ECommandType::Mode),
            "Set playback mode" },
        { ct::CommandTypeToString(ECommandType::Select) + " " + ct::RequiredArgDataTypeToString(ECommandType::Select),
            "Play track by index" },
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
    Lines.emplace_back(std::string(stp::msg::APP_HELP_MSG) + " Available commands:");
    
    for (const FHelpEntry& HelpEntry : HelpEntries)
    {
        size_t Padding = MaxUsageLen - HelpEntry.Usage.size() + CommandHelpIdentation;
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
            
            HelpEntryEXT.Description.emplace_back("Resumes or initiates playback of a track.");
            
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Play));
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
            
            HelpEntryEXT.Description .emplace_back("Initiates playback of the next track in the list.");
            HelpEntryEXT.Description .emplace_back("If there is no next track, it initiates playback of the very "
                "first track in the list.");
            
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Next));
            break;
        }
        case ECommandType::Prev:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Prev));
            
            HelpEntryEXT.Description.emplace_back("Initiates playback of the previous track in the list. "
                "If there is no previous track, it initiates playback of the very last track in the list.");
            
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Prev));
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
            
            HelpEntryEXT.Description.emplace_back("Refreshes the track list.");
            HelpEntryEXT.Description.emplace_back("Use this if you've added a new track to the default directory.");
            
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
            
            HelpEntryEXT.Description.emplace_back("Set playback mode for the player.");
            HelpEntryEXT.Description.emplace_back("The <" + ct::PlaybackModeToString(EPlaybackMode::Once) + "> argument "
                "switches the player to single-play mode. This means the currently active track will play exactly once.");
            HelpEntryEXT.Description.emplace_back("The <" + ct::PlaybackModeToString(EPlaybackMode::LoopOne) + "> "
                "argument puts the player into loop mode for the currently active track. This is almost the same as <" + 
            ct::PlaybackModeToString(EPlaybackMode::Once) + ">, except it plays one specific track repeatedly.");
            HelpEntryEXT.Description.emplace_back("The <" + ct::PlaybackModeToString(EPlaybackMode::LoopAll) + "> "
                "argument puts the player into a looped playback mode, playing the track list from start to finish, then "
                "restarting from the beginning after the last track in the list has played. This creates an endless loop.");
            HelpEntryEXT.Description.emplace_back("The <" + ct::PlaybackModeToString(EPlaybackMode::LoopShuffle) + "> "
                "argument puts the player into a looped playback mode of the list of all tracks, but each subsequent "
                "track is selected randomly.");
            
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
        case ECommandType::Select:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Select) + " " + 
                ct::RequiredArgDataTypeToString(ECommandType::Select));
            
            HelpEntryEXT.Description.emplace_back("Initiates playback of a track by index.");
            HelpEntryEXT.Description.emplace_back("Use the list command to get the track index.");
            
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Select) + " 5");
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Select) + " 0");
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::Select) + " 2");
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
    }
    
    std::vector<std::string> Lines;
    Lines.emplace_back(stp::msg::APP_HELP_MSG + ct::CommandTypeToString(CommandType));
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
    Lines.emplace_back(stp::msg::APP_STATUS_MSG);
    Lines.emplace_back("  [State: " + State + "]");
    Lines.emplace_back("  [Playback Mode: " + Mode + "]");
    Lines.emplace_back("  [Current Track: " + UISnapshot.CurrentTrackName + "]");
    Lines.emplace_back("  [Duration: " + FormatTime(DurationSec) + "]");
    Lines.emplace_back("  [Position: " + FormatTime(CurrentOffsetSec) + "]");
    Lines.emplace_back("  [Remaining: " + FormatTime(RemainingSec) + "]");
    Lines.emplace_back("  [Volume: " + std::to_string(static_cast<int>(gp::MIN_VOLUME)) + "/" + VolumeStr + "]");
    PrintOutputLines(Lines);
}

void MConsoleIO::PrintFindResults(const std::vector<std::pair<int, std::string>> &FindedTracks)
{
    if (FindedTracks.empty())
    {
        PrintOutputMessage(std::string(stp::msg::APP_FIND_MSG) + "No matches found.");
        return;
    }

    std::vector<std::string> Lines;
    Lines.emplace_back(std::string(stp::msg::APP_FIND_MSG) + "Results: " + std::to_string(FindedTracks.size()));

    for (size_t i = 0; i < FindedTracks.size(); i++)
    {
        const std::string SubCatTorL = (i == FindedTracks.size() - 1) ? stp::sep::SUBCAT_SEP_L : stp::sep::SUBCAT_SEP_T;

        Lines.emplace_back(stp::sep::SUBCAT_SEP_TAB + SubCatTorL
            + "Index(" + std::to_string(FindedTracks[i].first) + ")"
            + "[" + FindedTracks[i].second + "]");
    }

    PrintOutputLines(Lines);
}

void MConsoleIO::PrintOutputMessage(const std::string &Message)
{
    PrintOutputLines({ Message });
}

void MConsoleIO::PrintOutputLines(const std::vector<std::string> &Lines)
{
    LastOutputLines = Lines;
    OutputVerticalScrollOffset = 0;
    OutputHorizontalScrollOffset = 0;
    
    if (!bTUIActive || OutputWindow == nullptr)
    {
        for (const std::string &Line : Lines)
        {
            std::cout << Line << std::endl;
        }
        return;
    }

    RenderOutputWindow();
}

void MConsoleIO::RenderOutputWindow()
{
    if (!bTUIActive || OutputWindow == nullptr) return;
    
    werase(OutputWindow);
    box(OutputWindow, 0, 0);
    
    int WindowHeight = 0;
    int WindowWidth = 0;
    getmaxyx(OutputWindow, WindowHeight, WindowWidth);
    
    const int TextStartX = 2;
    const int RightPadding = 1;
    
    const int MaxPrintableLines = WindowHeight - 2;
    const int MaxPrintableColumns = WindowWidth - TextStartX - RightPadding - 1;
    
    
    if (MaxPrintableLines <= 0 || MaxPrintableColumns <= 0)
    {
        wrefresh(OutputWindow);
        return;
    }
    
    for (int i = 0; i < MaxPrintableLines; i++)
    {
        const int SourceIndex = OutputVerticalScrollOffset + i;
        if (SourceIndex >=static_cast<int>(LastOutputLines.size())) break;
        
        const std::string& Line = LastOutputLines[SourceIndex];
        if (OutputHorizontalScrollOffset >= static_cast<int>(Line.size()))  continue;
        
        mvwaddnstr(
            OutputWindow,
            i + 1, 
            TextStartX, 
            Line.c_str() + OutputHorizontalScrollOffset,
            MaxPrintableColumns);
    }
    
    wrefresh(OutputWindow);
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
