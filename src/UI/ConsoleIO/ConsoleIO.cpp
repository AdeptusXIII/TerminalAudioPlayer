//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#include "UI/ConsoleIO/ConsoleIO.h"

#include "UI/UISnapshot.h"
#include "Engine/CommandDataTable.h"
#include "Engine/StringTemplates.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

MConsoleIO::MConsoleIO() 
{
    CommandHelpIdentation = 2;
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
    
    std::cout << std::endl << stp::msg::APP_HELP_MSG << " Available commands:\n\n";
    
    for (const FHelpEntry& HelpEntry : HelpEntries)
    {
        std::cout << HelpEntry.Usage;
        
        size_t Padding = MaxUsageLen - HelpEntry.Usage.size() + CommandHelpIdentation;
        
        std::cout << std::string(Padding, ' ');
        
        std::cout << HelpEntry.Description << std::endl;
    }
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
            
            HelpEntryEXT.Description .emplace_back("Initiates playback of the next track in the list. "
                "If there is no next track, it initiates playback of the very first track in the list.");
            
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
            
            HelpEntryEXT.Description.emplace_back("Shows a complete list of known and valid tracks supported by the player.");
            
            HelpEntryEXT.Examples.emplace_back(ct::CommandTypeToString(ECommandType::List));
            break;
        }
        case ECommandType::Refresh:
        {
            HelpEntryEXT.Usage.emplace_back(ct::CommandTypeToString(ECommandType::Refresh));
            
            HelpEntryEXT.Description.emplace_back("Refreshes the track list. Use this if you've added a new track to"
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
            
            HelpEntryEXT.Description.emplace_back("Set playback mode for the player.");
            HelpEntryEXT.Description.emplace_back("\nThe <" + ct::PlaybackModeToString(EPlaybackMode::Once) + "> argument "
                "switches the player to single-play mode. This means the currently active track will play exactly once.");
            HelpEntryEXT.Description.emplace_back("\nThe <" + ct::PlaybackModeToString(EPlaybackMode::LoopOne) + "> "
                "argument puts the player into loop mode for the currently active track. This is almost the same as <" + 
            ct::PlaybackModeToString(EPlaybackMode::Once) + ">, except it plays one specific track repeatedly.");
            HelpEntryEXT.Description.emplace_back("\nThe <" + ct::PlaybackModeToString(EPlaybackMode::LoopAll) + "> "
                "argument puts the player into a looped playback mode, playing the track list from start to finish, then "
                "restarting from the beginning after the last track in the list has played. This creates an endless loop.");
            HelpEntryEXT.Description.emplace_back("\nThe <" + ct::PlaybackModeToString(EPlaybackMode::LoopShuffle) + "> "
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
            
            HelpEntryEXT.Description.emplace_back("Initiates playback of a track by index. Use the list command to "
                "get the track index.");
            
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
            HelpEntryEXT.Description.emplace_back("Shows the current state of the player: playback mode, state, "
                "current track,global volume modifier.");
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
    
    std::cout << std::endl << stp::msg::APP_HELP_MSG << ct::CommandTypeToString(CommandType) << std::endl;
    
    std::cout << std::endl << " Usage: " << std::endl;
    for (const auto & Example : HelpEntryEXT.Usage)
    {
        std::cout << "  " << Example << std::endl;
    }
    
    std::cout << std::endl << " Description: " << std::endl; 
    for (const auto & Description : HelpEntryEXT.Description)
    {
        std::cout << "  " << Description << std::endl;
    }
    
    std::cout << std::endl << " Examples: "<< std::endl;
    for (const auto & Example : HelpEntryEXT.Examples)
    {
        std::cout << "  " << Example << std::endl;
    }
    
    std::cout << std::endl;
}

void MConsoleIO::PrintStatus(const FUISnapshotData &UISnapshot)
{
    std::string State;
    std::string Mode;
    std::string VolumeStr;
    
    std::cout << stp::msg::APP_STATUS_MSG << std::endl;
    
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

    if (std::floor(UISnapshot.CurrentVolume) == UISnapshot.CurrentVolume)
    {
        VolumeStr = std::to_string(static_cast<int>(UISnapshot.CurrentVolume));
    }
    else
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << UISnapshot.CurrentVolume;
        VolumeStr = oss.str();
    }
    
    std::cout << "  State: " << State << std::endl;
    std::cout << "  Playback Mode: " << Mode << std::endl;
    std::cout << "  Current Track: " << UISnapshot.CurrentTrackName << std::endl;
    std::cout << "  Volume: " << VolumeStr << std::endl;
}

void MConsoleIO::PrintFindResults(const std::vector<std::pair<int, std::string>> &FindedTracks)
{
    if (FindedTracks.empty())
    {
        std::cout << "\n" << stp::msg::APP_FIND_MSG << "No matches found.\n\n";
        return;
    }

    std::cout << "\n" << stp::msg::APP_FIND_MSG <<  "Results: " << FindedTracks.size() << "\n";

    for (size_t i = 0; i < FindedTracks.size(); i++)
    {
        const std::string SubCatTorL = (i == FindedTracks.size() - 1) ? stp::sep::SUBCAT_SEP_L : stp::sep::SUBCAT_SEP_T;

        std::cout << stp::sep::SUBCAT_SEP_TAB << SubCatTorL
                  << "Index(" << FindedTracks[i].first << ")"
                  << "[" << FindedTracks[i].second << "]"
                  << std::endl;
    }

    std::cout << std::endl;
}