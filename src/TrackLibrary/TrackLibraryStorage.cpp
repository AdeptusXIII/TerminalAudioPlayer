//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#include "TrackLibrary/TrackLibraryStorage.h"

#include <cstdlib>
#include <fstream>

bool MTrackLibraryStorage::EnsureStorageFileExists()
{
    const std::filesystem::path StorageFilePath = GetStorageFilePath();
    const std::filesystem::path StorageDirectoryPath = GetStorageDirectoryPath();
    
    if (!EnsureDirectoryExists(StorageDirectoryPath))
    {
        return false;
    }

    if (FileExists(StorageFilePath))
    {
        return true;
    }

    return CreateEmptyFile(StorageFilePath);
}

std::vector<std::filesystem::path> MTrackLibraryStorage::LoadTrackPaths()
{
    std::vector<std::filesystem::path> TrackPaths;
    
    if (!EnsureStorageFileExists()) return TrackPaths;
    
    const std::filesystem::path StorageFilePath = GetStorageFilePath();
    
    std::ifstream File(StorageFilePath);
    
    if (!File.is_open()) return TrackPaths;
    
    std::string Line; 
    
    while (std::getline(File, Line))
    {
        if (Line.empty()) continue;
        
        TrackPaths.emplace_back(Line);
    }
    
    return TrackPaths;
}

bool MTrackLibraryStorage::SaveTrackPaths(const std::vector<std::filesystem::path> &TrackPaths)
{
    if (!EnsureStorageFileExists()) return false;
    
    const std::filesystem::path StorageFilePath = GetStorageFilePath();
    
    std::ofstream File(StorageFilePath);
    
    if (!File.is_open()) return false;
    
    for (const std::filesystem::path &TrackPath : TrackPaths)
    {
        File << TrackPath.string() << "\n";
    }
    
    return true;
}

bool MTrackLibraryStorage::EnsureDirectoryExists(const std::filesystem::path &DirectoryPath) const
{
    std::error_code ErrorCode;
    
    if (std::filesystem::is_directory(DirectoryPath, ErrorCode))
    {
        return true;
    }

    if (ErrorCode)
    {
        return false;
    }
    
    std::filesystem::create_directories(DirectoryPath, ErrorCode);
    
    if (ErrorCode)
    {
        return false;
    }

    return std::filesystem::is_directory(DirectoryPath);
}

std::filesystem::path MTrackLibraryStorage::GetStorageFilePath() const
{
    const char* XdgDataHome = std::getenv("XDG_DATA_HOME");

    if (XdgDataHome != nullptr)
    {
        return std::filesystem::path(XdgDataHome) / "TerminalAudioPlayer/library.txt";
    }

    const char* Home = std::getenv("HOME");

    if (Home != nullptr)
    {
        return std::filesystem::path(Home) / ".local/share/TerminalAudioPlayer/library.txt";
    }

    return std::filesystem::current_path() / "TerminalAudioPlayer/library.txt";
}

std::filesystem::path MTrackLibraryStorage::GetStorageDirectoryPath() const
{
    return GetStorageFilePath().parent_path();
}

bool MTrackLibraryStorage::FileExists(const std::filesystem::path &FilePath) const
{
    std::error_code ErrorCode;
    const bool bExists = std::filesystem::exists(FilePath, ErrorCode);

    if (ErrorCode) return false;

    return bExists;
}

bool MTrackLibraryStorage::CreateEmptyFile(const std::filesystem::path &FilePath) const
{
    std::ofstream File(FilePath);
    return File.is_open();
}
