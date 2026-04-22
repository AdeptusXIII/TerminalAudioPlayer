//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".
#pragma once

#include <filesystem>
#include <vector>

class MTrackLibrary 
{
public:
    MTrackLibrary();
    
    void AddTrackToTrackList(const std::filesystem::path &InPath);
    void AddTracksToTrackList(const std::vector<std::filesystem::path> &PathsList);
    bool SetPrevIndex();
    bool SetNextIndex();
    bool SetCurrentIndex(int Index);
    bool IsEmpty() const;
    
    std::string GetTrackNameByIndex(int Index) const;
    std::string GetCurrentTrackName() const;
    int GetTrackListSize() const;
    int GetCurrentIndex() const;
    std::filesystem::path GetCurrentTrackPath() const;
    std::filesystem::path GetPrevTrackPath() const;
    std::filesystem::path GetNextTrackPath() const;
    std::filesystem::path GetTrackPathByIndex(int Index) const;
    
    void Clear();

private:
    std::vector<std::filesystem::path> TrackList;
    int CurrentIndex;
};

