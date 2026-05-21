//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#pragma once

#include <filesystem>
#include <vector>

class MTrackLibraryStorage
{
public:
    bool EnsureStorageFileExists();
    bool EnsureDirectoryExists(const std::filesystem::path &DirectoryPath) const;
    std::vector<std::filesystem::path> LoadTrackPaths();
    bool SaveTrackPaths(const std::vector<std::filesystem::path> &TrackPaths);
    
private:
    std::filesystem::path GetStorageFilePath() const;
    std::filesystem::path GetStorageDirectoryPath() const;
    bool FileExists(const std::filesystem::path &FilePath) const;
    bool CreateEmptyFile(const std::filesystem::path &FilePath) const;
};
