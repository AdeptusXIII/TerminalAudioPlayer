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
        case ECommandType::Unknown:
        {
            StringCommand = {};
            break;
        }
    }
    
    return StringCommand;
}

inline std::string PlaybackModeToArg(EPlaybackMode PlaybackMode)
{
    std::string Arg = {};
    
    switch (PlaybackMode)
    {
        case EPlaybackMode::Once:
        {
            Arg = "once";
            break;
        }
        case EPlaybackMode::LoopOne:
        {
            Arg = "loop-one";
            break;
        }
        case EPlaybackMode::LoopAll:
        {
            Arg = "loop-all";
            break;
        }
        case EPlaybackMode::LoopShuffle:
        {
            Arg = "loop-shuffle";
            break;
        }
        case EPlaybackMode::None:
        {
            break;
        }
    }
        
    return Arg;
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
            Arg = "<" + ct::PlaybackModeToArg(EPlaybackMode::Once) + "|" 
                      + ct::PlaybackModeToArg(EPlaybackMode::LoopOne) + "|"
                      + ct::PlaybackModeToArg(EPlaybackMode::LoopAll) + "|"
                      + ct::PlaybackModeToArg(EPlaybackMode::LoopShuffle) + ">";
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
        case ECommandType::Unknown:
        {
            Arg = "";
            break;
        }
    }
    
    return Arg;
}

};