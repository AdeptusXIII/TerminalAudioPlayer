//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".
#pragma once

#include <filesystem>
#include <unordered_set>
#include <vector>

enum class EScanResultStatus
{
    Success,
    EmptyPath,
    PathDoesNotExist,
    PathIsNotDirectory,
    UnexpectedError
};

struct FScanResult
{
    EScanResultStatus Status = EScanResultStatus::Success;
    std::vector<std::filesystem::path> Tracks;
};

class MAudioFileScanner 
{
    static const std::unordered_set<std::string> AllowedExtensions;
public:
    MAudioFileScanner();
    
    FScanResult ScanPath(const std::filesystem::path &InPath);
    FScanResult ScanPathRecursive(const std::filesystem::path &InPath);

private:
    bool IsAudioFile(const std::filesystem::path& InPath) const;
    int GetCharPriority(char C) const;
    /** А приоритетнее Б? */
    bool IsCharHigherPriority(char A, char B) const;
    /** Строка А приоритетнее Б? */
    bool IsStringHigherPriority(const std::string& A, const std::string& B) const;
};
