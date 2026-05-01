//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#include "UI/ConsoleIO/ConsoleIO.h"

#include "UI/UISnapshot.h"
#include "Engine/CommandDataTable.h"
#include "Engine/StringTemplates.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

MConsoleIO::MConsoleIO() 
{
    CommandHelpIdentation = 2;
}

void MConsoleIO::PrintTrackList(const FUISnapshotData &UISnapshot)
{
    std::cout << std::endl;
    std::cout << stp::msg::fnc::APP_FNC_TOTAL_TRACKS_MSG << UISnapshot.TrackCount << std::endl;

    for (int i = 0; i < UISnapshot.TrackCount; i++)
    {
        std::string CurrentTrack = (i == UISnapshot.CurrentTrackIndex) ? stp::msg::fnc::APP_FNC_CUR_TRACK_MSG : "";
        std::string SubCatTorL = (i == UISnapshot.TrackCount - 1) ? stp::sep::SUBCAT_SEP_L : stp::sep::SUBCAT_SEP_T;

        std::cout << stp::sep::SUBCAT_SEP_TAB + SubCatTorL 
                  << "Index(" << i << ")" 
                  << "[" << UISnapshot.TrackList[i] 
                  << "] "
                  << CurrentTrack
                  << std::endl;
    }
}

void MConsoleIO::PrintTotalTracksNum(const int &InTotalTracks) 
{
    std::cout << stp::msg::APP_LIBRARY_MSG << stp::msg::fnc::APP_FNC_LIB_RESET_MSG 
              << stp::msg::fnc::APP_FNC_TOTAL_TRACKS_MSG << InTotalTracks << "." << std::endl;
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
    
    return {};
}

void MConsoleIO::PrintHelp()
{
    std::vector<FHelpEntry> HelpEntries =
    {
        { ct::CommandTypeToString(ECommandType::Play) + ct::RequiredArgDataTypeToString(ECommandType::Play),
            "Resume or start playback" },
        { ct::CommandTypeToString(ECommandType::Pause) + ct::RequiredArgDataTypeToString(ECommandType::Pause),
            "Pause current track" },
        { ct::CommandTypeToString(ECommandType::Stop) + ct::RequiredArgDataTypeToString(ECommandType::Stop),
            "Stop current track" },
        { ct::CommandTypeToString(ECommandType::Next) + ct::RequiredArgDataTypeToString(ECommandType::Next),
            "Play next track" },
        { ct::CommandTypeToString(ECommandType::Prev) + ct::RequiredArgDataTypeToString(ECommandType::Prev),
            "Play previous track" },
        { ct::CommandTypeToString(ECommandType::List) + ct::RequiredArgDataTypeToString(ECommandType::List),
            "Show track list" },
        { ct::CommandTypeToString(ECommandType::Refresh) + ct::RequiredArgDataTypeToString(ECommandType::Refresh),
            "Rescan track library" },
        { ct::CommandTypeToString(ECommandType::Mode) + " " + ct::RequiredArgDataTypeToString(ECommandType::Mode),
            "Set playback mode" },
        { ct::CommandTypeToString(ECommandType::Select) + " " + ct::RequiredArgDataTypeToString(ECommandType::Select),
            "Play track by index" },
        { ct::CommandTypeToString(ECommandType::Volume) + " " + ct::RequiredArgDataTypeToString(ECommandType::Volume),
            "Set global player volume" },
        { ct::CommandTypeToString(ECommandType::Status) + ct::RequiredArgDataTypeToString(ECommandType::Status),
            "Show player status" },
        { ct::CommandTypeToString(ECommandType::Help),"Show this help" },
        { ct::CommandTypeToString(ECommandType::Help) + " " + ct::RequiredArgDataTypeToString(ECommandType::Help),
        "Show detailed description of <command>" },
        { ct::CommandTypeToString(ECommandType::Exit) + ct::RequiredArgDataTypeToString(ECommandType::Exit),
            "Exit player" }
    };
    
    size_t MaxUsageLen = 0;
    
    for (const FHelpEntry& HelpEntry : HelpEntries)
    {
        if (HelpEntry.Usage.size() > MaxUsageLen)
        {
            MaxUsageLen = HelpEntry.Usage.size();
        }
    }
    
    std::cout << std::endl << stp::msg::APP_HELP_MSG << " Available commands:\n\n";
    
    for (const FHelpEntry& HelpEntry : HelpEntries)
    {
        std::cout << HelpEntry.Usage;
        
        size_t Padding = MaxUsageLen - HelpEntry.Usage.size() + CommandHelpIdentation;
        
        std::cout << std::string(Padding, ' ');
        
        std::cout << HelpEntry.Description << std::endl;
    }
}

void MConsoleIO::PrintHelpCMD(ECommandType CommandType)
{
    FHelpEntryEXT HelpEntryEXT = {};
    
    switch (CommandType)
    {
        case ECommandType::Play:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Play));
            HelpEntryEXT.Description = "Resumes or initiates playback of a track.";
            HelpEntryEXT.Example.emplace_back("play");
            break;
        }
        case ECommandType::Pause:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Pause));
            HelpEntryEXT.Description = "Pauses the currently active track.";
            HelpEntryEXT.Example.emplace_back("pause");
            break;
        }
        case ECommandType::Stop:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Stop));
            HelpEntryEXT.Description = "Stops the currently active track.";
            HelpEntryEXT.Example.emplace_back("stop");
            break;
        }
        case ECommandType::Next:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Next));
            HelpEntryEXT.Description = "Initiates playback of the next track in the list. If there is no next track,"
                                       " it initiates playback of the very first track in the list.";
            HelpEntryEXT.Example.emplace_back("next");
            break;
        }
        case ECommandType::Prev:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Prev));
            HelpEntryEXT.Description = "Initiates playback of the previous track in the list. If there is no previous track,"
                                       " it initiates playback of the very last track in the list.";
            HelpEntryEXT.Example.emplace_back("prev");
            break;
        }
        case ECommandType::List:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::List));
            HelpEntryEXT.Description = "Shows a complete list of known and valid tracks supported by the player.";
            HelpEntryEXT.Example.emplace_back("list");
            break;
        }
        case ECommandType::Refresh:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Refresh));
            HelpEntryEXT.Description = "Refreshes the track list. Use this if you've added a new track to the default directory.";
            HelpEntryEXT.Example.emplace_back("refresh");
            break;
        }
        case ECommandType::Exit:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Exit));
            HelpEntryEXT.Description = "Immediately terminates the program.";
            HelpEntryEXT.Example.emplace_back("exit");
            break;
        }
        case ECommandType::Help:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Help));
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Help) + " " + 
                ct::RequiredArgDataTypeToString(ECommandType::Help));
            HelpEntryEXT.Description = "Show general help or help for one specific command.";
            HelpEntryEXT.Example.emplace_back("help");
            HelpEntryEXT.Example.emplace_back("help volume");
            HelpEntryEXT.Example.emplace_back("help mode");
            break;
        }
        case ECommandType::Mode:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Mode) + " " + 
                ct::RequiredArgDataTypeToString(ECommandType::Mode));
            HelpEntryEXT.Description = "Set playback mode for the player.";
            HelpEntryEXT.Example.emplace_back("mode once");
            HelpEntryEXT.Example.emplace_back("mode loop-one");
            HelpEntryEXT.Example.emplace_back("mode loop-all");
            HelpEntryEXT.Example.emplace_back("mode loop-shuffle");
            break;
        }
        case ECommandType::Select:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Select) + " " + 
                ct::RequiredArgDataTypeToString(ECommandType::Select));
            HelpEntryEXT.Description = "Initiates playback of a track by index. Use the list command to get the track index.";
            HelpEntryEXT.Example.emplace_back("select 5");
            HelpEntryEXT.Example.emplace_back("select 0");
            HelpEntryEXT.Example.emplace_back("select 2");
            break;
        }
        case ECommandType::Volume:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Volume) +" " + 
                ct::RequiredArgDataTypeToString(ECommandType::Volume));
            HelpEntryEXT.Description = "Sets the global volume modifier as a percentage from "
            + std::to_string(static_cast<int>(gp::MIN_VOLUME)) + " to " + std::to_string(static_cast<int>(gp::MAX_VOLUME)) + ".";
            HelpEntryEXT.Example.emplace_back("volume 100");
            HelpEntryEXT.Example.emplace_back("volume 50");
            HelpEntryEXT.Example.emplace_back("volume 23.99");
            break;
        }
        case ECommandType::Status:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Status));
            HelpEntryEXT.Description = "Shows the current state of the player: playback mode, state, current track, "
                                       "global volume modifier.";
            HelpEntryEXT.Example.emplace_back("status");
            break;
        }
    }
    
    std::cout << std::endl << stp::msg::APP_HELP_MSG << ct::CommandTypeToString(CommandType) << std::endl;
    
    std::cout << std::endl << " Usage: " << std::endl;
    
    for (const auto & Example : HelpEntryEXT.Usage)
    {
        std::cout << "  " << Example << std::endl;
    }
    std::cout << std::endl << " Description: " << std::endl << "  " << HelpEntryEXT.Description << std::endl << std::endl;
    std::cout << std::endl << " Examples: "<< std::endl;
    
    for (const auto & Example : HelpEntryEXT.Example)
    {
        std::cout << "  " << Example << std::endl;
    }
    
    std::cout << std::endl;
}

void MConsoleIO::PrintStatus(const EAudioPlayerState &AudioPlayerState, EPlaybackMode &PlaybackMode, 
    std::string CurrentTrack, float CurrentVolume)
{
    std::string State;
    std::string Mode;
    std::string VolumeStr;
    
    std::cout << stp::msg::APP_STATUS_MSG << std::endl;
    
    switch (AudioPlayerState)
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
    
    switch (PlaybackMode)
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

    if (std::floor(CurrentVolume) == CurrentVolume)
    {
        VolumeStr = std::to_string(static_cast<int>(CurrentVolume));
    }
    else
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << CurrentVolume;
        VolumeStr = oss.str();
    }
    
    std::cout << "  State: " << State << std::endl;
    std::cout << "  Playback Mode: " << Mode << std::endl;
    std::cout << "  Current Track: " << CurrentTrack << std::endl;
    std::cout << "  Volume: " << VolumeStr << std::endl;
}