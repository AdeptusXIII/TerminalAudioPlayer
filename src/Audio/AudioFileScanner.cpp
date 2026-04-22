//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#include "AudioFileScanner.h"

#include <algorithm>
#include <iostream>
#include <unordered_set>

const std::unordered_set<std::string> MAudioFileScanner::AllowedExtensions =
{
    ".mp3", ".wav", ".ogg", ".flac"
};

MAudioFileScanner::MAudioFileScanner() 
{
    
}

std::vector<std::filesystem::path> MAudioFileScanner::ScanPath(const std::filesystem::path &InPath) 
{
    if (InPath.empty()) 
    {
        std::cerr << "[AudioFileScanner] Path is " << InPath << " empty." << std::endl;
        return {};
    }
    std::error_code EC;
    const bool bExists = std::filesystem::exists(InPath, EC);
    if (EC) 
    {
        std::cerr << "[AudioFileScanner] Error checking path: " << EC.message() << std::endl;
        return {};
    }
    if (!bExists)
    {
        std::cerr << "[AudioFileScanner] Path does not exist: " << InPath << std::endl;
        return {};
    }

    if (!std::filesystem::is_directory(InPath))
    {
        std::cerr << "[AudioFileScanner] Path is not a directory: " << InPath << std::endl;
        return {};
    }

    
    std::vector<std::filesystem::path> FilePathsList;
    
    for (const auto& Entry : std::filesystem::directory_iterator(InPath)) 
    {
        if (IsAudioFile(Entry.path())) 
        {
            FilePathsList.push_back(Entry.path());
        }
    }
    
    std::sort(FilePathsList.begin(), FilePathsList.end(),
        [&](const std::filesystem::path& A, const std::filesystem::path& B) 
        {
            return IsStringHigherPriority(A.stem().string(), B.stem().string());
        });
    
    return FilePathsList;
}

bool MAudioFileScanner::IsAudioFile(const std::filesystem::path &InPath) const
{
    if (!std::filesystem::exists(InPath) || !std::filesystem::is_regular_file(InPath))
        return false;
    
    std::string extension = InPath.extension().string();
    
    if (extension.empty())
        return false;
    
    std::transform(extension.begin(), extension.end(), extension.begin(), 
        [](unsigned char c) { return std::tolower(c); });
    
    return AllowedExtensions.find(extension) != AllowedExtensions.end();
}

int MAudioFileScanner::GetCharPriority(char C) const
{
    // 0 -> A..Z
    // 1 -> a..z
    // 2 -> 0..9
    // 3 -> всё остальное
    
    if (std::isupper(static_cast<unsigned char>(C))) return 0;
    if (std::islower(static_cast<unsigned char>(C))) return 1;
    if (std::isdigit(static_cast<unsigned char>(C))) return 2;
    
    return 3;
}

bool MAudioFileScanner::IsCharHigherPriority(char A, char B) const
{
    int A_Priority = GetCharPriority(A);
    int B_Priority = GetCharPriority(B);
    
    if (A_Priority < B_Priority)
        return true; // 'A' < 'b'
    else if (A_Priority > B_Priority)
        return false; // 'a' > 'B'
    else // if ( A_Priority == B_Priority)
        return A < B; // 'A' < 'B'
}

bool MAudioFileScanner::IsStringHigherPriority(const std::string& A, const std::string& B) const
{
    size_t MinLen = std::min(A.length(), B.length());

    for (size_t i = 0; i < MinLen; i++)
    {
        // если A[i] раньше B[i] -> вся строка A раньше
        if (IsCharHigherPriority(A[i], B[i]))
            return true;

        // если B[i] раньше A[i] -> вся строка A позже
        if (IsCharHigherPriority(B[i], A[i]))
            return false;
    }

    // иначе 'Aaa' == 'Aaa' -> значит решаем по длине
    return A.length() < B.length();
}