//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#pragma once 

#include <vector>
#include <string>

enum class ECommandType
{
    Unknown,
    Play,
    Pause,
    Stop,
    Next,
    Prev,
    List,
    Refresh,
    Exit,
    Help,
    Mode,
    Select,
    Volume,
    Status,
    Find
};

struct FCommand
{
    ECommandType Type = ECommandType::Unknown;
    std::vector<std::string> Args;
    std::string RawInput;
};