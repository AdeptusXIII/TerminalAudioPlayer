//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#pragma once

#include "Types/CommandTypes.h"
#include "Types/PlaybackTypes.h"
#include "Engine/GlobalParameters.h"

#include <string>

namespace ct
{

constexpr int MODE_ARG_COUNT = 4;

inline std::string CommandTypeToString(ECommandType CommandType)
{
    std::string StringCommand = {};
    
    switch (CommandType)
    {
        case ECommandType::Play:
        {
            StringCommand = "play";
            break;
        }
        case ECommandType::Pause:
        {
            StringCommand = "pause";
            break;
        }
        case ECommandType::Stop:
        {
            StringCommand = "stop";
            break;
        }
        case ECommandType::Next:
        {
            StringCommand = "next";
            break;
        }
        case ECommandType::Prev:
        {
            StringCommand = "prev";
            break;
        }
        case ECommandType::List:
        {
            StringCommand = "list";
            break;
        }
        case ECommandType::Refresh:
        {
            StringCommand = "refresh";
            break;
        }
        case ECommandType::Exit:
        {
            StringCommand = "exit";
            break;
        }
        case ECommandType::Help:
        {
            StringCommand = "help";
            break;
        }
        case ECommandType::Mode:
        {
            StringCommand = "mode";
            break;
        }
        case ECommandType::Select:
        {
            StringCommand = "select";
            break;
        }
        case ECommandType::Volume:
        {
            StringCommand = "volume";
            break;
        }
        case ECommandType::Status:
        {
            StringCommand = "status";
            break;
        }
        case ECommandType::Find:
        {
            StringCommand = "find";
            break;
        }
        case ECommandType::Scan:
        {
            StringCommand = "scan";
            break;
        }
        case ECommandType::Playlist:
        {
            StringCommand = "pl";
            break;
        }
        case ECommandType::Unknown:
        {
            StringCommand = {};
            break;
        }
    }
    
    return StringCommand;
}

inline std::string AudioPlayerStateToString(EAudioPlayerState AudioPlayerState)
{
    std::string AudioPlayerStateStr = {};
    
    switch (AudioPlayerState)
    {
        case EAudioPlayerState::Idle:
        {
            AudioPlayerStateStr = "Idle";
            break;
        }
        case EAudioPlayerState::Playing:
        {
            AudioPlayerStateStr = "Playing";
            break;
        }
        case EAudioPlayerState::Paused:
        {
            AudioPlayerStateStr = "Paused";
            break;
        }
        case EAudioPlayerState::None:
        {
            AudioPlayerStateStr = "None";
            break;
        }
    }
    
    return AudioPlayerStateStr;
}

inline std::string PlaybackModeToString(EPlaybackMode PlaybackMode)
{
    std::string PlaybackModeStr = {};
    
    switch (PlaybackMode)
    {
        case EPlaybackMode::Once:
        {
            PlaybackModeStr = "once";
            break;
        }
        case EPlaybackMode::LoopOne:
        {
            PlaybackModeStr = "loop-one";
            break;
        }
        case EPlaybackMode::LoopAll:
        {
            PlaybackModeStr = "loop-all";
            break;
        }
        case EPlaybackMode::LoopShuffle:
        {
            PlaybackModeStr = "loop-shuffle";
            break;
        }
        case EPlaybackMode::None:
        {
            break;
        }
    }
        
    return PlaybackModeStr;
};

inline std::string RequiredArgDataTypeToString(ECommandType CommandType)
{
    std::string Arg = {};
    
    switch (CommandType)
    {
        case ECommandType::Play:
        {
            Arg = "";
            break;
        }
        case ECommandType::Pause:
        {
            Arg = "";
            break;
        }
        case ECommandType::Stop:
        {
            Arg = "";
            break;
        }
        case ECommandType::Next:
        {
            Arg = "";
            break;
        }
        case ECommandType::Prev:
        {
            Arg = "";
            break;
        }
        case ECommandType::List:
        {
            Arg = "";
            break;
        }
        case ECommandType::Refresh:
        {
            Arg = "";
            break;
        }
        case ECommandType::Exit:
        {
            Arg = "";
            break;
        }
        case ECommandType::Help:
        {
            Arg = "<command>";
            break;
        }
        case ECommandType::Mode:
        {
            Arg = "<" + ct::PlaybackModeToString(EPlaybackMode::Once) + "|" 
                      + ct::PlaybackModeToString(EPlaybackMode::LoopOne) + "|"
                      + ct::PlaybackModeToString(EPlaybackMode::LoopAll) + "|"
                      + ct::PlaybackModeToString(EPlaybackMode::LoopShuffle) + ">";
            break;
        }
        case ECommandType::Select:
        {
            Arg = "<integer index>";
            break;
        }
        case ECommandType::Volume:
        {
            Arg = "<" + std::to_string(static_cast<int>(gp::MIN_VOLUME)) + ".." 
                      + std::to_string(static_cast<int>(gp::MAX_VOLUME)) + ">";
            break;
        }
        case ECommandType::Status:
        {
            Arg = "";
            break;
        }
        case ECommandType::Find:
        {
            Arg = "<name>";
            break;
        }
        case ECommandType::Scan:
        {
            Arg = "<path>";
            break;
        }
        case ECommandType::Playlist:
        {
            Arg = "<list|use|create|delete|...>";
            break;
        }
        case ECommandType::Unknown:
        {
            Arg = "";
            break;
        }
    }
    
    return Arg;
}

inline std::string CommandFlagToString(ECommandFlag CommandFlag)
{
    std::string CommandFlagStr = {};
    switch (CommandFlag)
    {
        case ECommandFlag::Recursive:
        {
            CommandFlagStr = "-r";
        }
    }
    
    return CommandFlagStr;
}

};
