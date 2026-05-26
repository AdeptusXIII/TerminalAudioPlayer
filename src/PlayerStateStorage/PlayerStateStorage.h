//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#pragma once

#include "Types/PlayerStateTypes.h"

#include <filesystem>
#include <vector>

class MPlayerStateStorage
{
public:
    bool EnsureStateFileExists();
    bool Save(const FPlayerStateData& State);
    bool Load(FPlayerStateData& OutState);

    bool EnsureDirectoryExists(const std::filesystem::path &DirectoryPath) const;

private:
    std::filesystem::path GetStateFilePath() const;

    bool FileExists(const std::filesystem::path &FilePath) const;
    bool CreateEmptyFile(const std::filesystem::path &FilePath) const;
};
