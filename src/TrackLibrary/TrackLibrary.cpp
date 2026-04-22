//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#include "TrackLibrary.h"

MTrackLibrary::MTrackLibrary()
{
    TrackList = std::vector<std::filesystem::path>();
    CurrentIndex = 0;
}

void MTrackLibrary::AddTrackToTrackList(const std::filesystem::path &InPath) 
{
    if (!InPath.empty()) 
    {
        TrackList.emplace_back(InPath);
    }
}

void MTrackLibrary::AddTracksToTrackList(const std::vector<std::filesystem::path> &PathsList) 
{
    if (!PathsList.empty()) 
    {
        for (const auto& Path : PathsList) 
        {
            AddTrackToTrackList(Path);
        }
    }
}

bool MTrackLibrary::SetPrevIndex() 
{
    if (CurrentIndex > 0 && CurrentIndex < GetTrackListSize()) 
    {
        CurrentIndex--;
        return true;
    }
   
    return false;
}

bool MTrackLibrary::SetNextIndex() 
{
    if (CurrentIndex + 1 < GetTrackListSize())
    {
        CurrentIndex++;
        return true;
    }
    
    return false;
}

bool MTrackLibrary::SetCurrentIndex(int Index) 
{
    if (Index >= 0 && Index < GetTrackListSize()) 
    {
        CurrentIndex = Index;
        return true;
    }
    
    return false;
}

bool MTrackLibrary::IsEmpty() const
{
    return GetTrackListSize() == 0;
}

std::string MTrackLibrary::GetTrackNameByIndex(int Index) const
{
    if (Index >= TrackList.size())
        return "";
    
    return TrackList[Index].filename();
}

std::string MTrackLibrary::GetCurrentTrackName() const  
{
    if (CurrentIndex < GetTrackListSize()) 
    {
        return TrackList[CurrentIndex].filename();    
    }
    
    return  "";
}

int MTrackLibrary::GetTrackListSize() const
{
    return static_cast<int>(TrackList.size());
}

int MTrackLibrary::GetCurrentIndex() const 
{
    return CurrentIndex;
}

std::filesystem::path MTrackLibrary::GetCurrentTrackPath() const
{
    if (CurrentIndex >= TrackList.size())
        return {};
        
    return TrackList[CurrentIndex];
}

std::filesystem::path MTrackLibrary::GetPrevTrackPath() const 
{
    if (CurrentIndex > 0 && CurrentIndex < GetTrackListSize()) 
    {
        return TrackList[CurrentIndex - 1];
    }
    
    return {};
}

std::filesystem::path MTrackLibrary::GetNextTrackPath() const 
{
    if (CurrentIndex + 1 < GetTrackListSize())
    {
        return TrackList[CurrentIndex + 1];
    }
    
    return {};
}

std::filesystem::path MTrackLibrary::GetTrackPathByIndex(int Index) const 
{
    if (Index < 0 || Index >= GetTrackListSize()) return {};
    
    return TrackList[Index];
}

void MTrackLibrary::Clear() 
{
    CurrentIndex = 0;
    TrackList.clear();
}
