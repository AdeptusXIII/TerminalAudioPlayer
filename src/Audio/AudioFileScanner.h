//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".
#pragma once

#include <filesystem>
#include <unordered_set>
#include <vector>

class MAudioFileScanner 
{
    static const std::unordered_set<std::string> AllowedExtensions;
public:
    MAudioFileScanner();
    
    std::vector<std::filesystem::path> ScanPath(const std::filesystem::path &InPath);

private:
    bool IsAudioFile(const std::filesystem::path& InPath) const;
    int GetCharPriority(char C) const;
    /** А приоритетнее Б? */
    bool IsCharHigherPriority(char A, char B) const;
    /** Строка А приоритетнее Б? */
    bool IsStringHigherPriority(const std::string& A, const std::string& B) const;
};

