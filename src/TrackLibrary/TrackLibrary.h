//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".
#pragma once

#include "Types/PlayerStateTypes.h"

#include <filesystem>
#include <string>
#include <vector>

enum class ETrackListKind
{
    Buffer,
    All,
    Favorite,
    Custom
};

struct FTrackList
{
    std::string Name;
    ETrackListKind Kind = ETrackListKind::Custom;
    std::vector<std::filesystem::path> Tracks;
    int CurrentIndex = 0;
};

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
    std::vector<std::filesystem::path> GetAllTrackPaths() const;
    std::vector<std::filesystem::path> GetFavoriteTrackPaths() const;
    std::vector<FCustomTrackListData> GetCustomTrackLists() const;
    std::string GetActiveTrackListName() const;
    int GetActiveTrackListIndex() const;
    int GetTrackListCount() const;
    std::vector<std::string> GetTrackListSummaries() const;
    
    bool SetActiveTrackListByName(const std::string& Name);
    bool SetActiveTrackListByIndex(int Index);
    bool SetBufferTracks(const std::vector<std::filesystem::path>& Paths);
    bool SetAllTracks(const std::vector<std::filesystem::path>& Paths);
    bool SetFavoriteTracks(const std::vector<std::filesystem::path>& TrackPaths);
    bool SetCustomTrackLists(const std::vector<FCustomTrackListData>& CustomTrackLists);

    bool ImportFromBuffer(bool bImportAll, int BufferIndex);
    bool CreateCustomTrackList(const std::string& Name);
    bool DeleteCustomTrackList(const std::string& Name);
    bool AddToFavoritesFromList(const std::string& SourceListName, int SourceTrackIndex);
    bool AddToTrackListFromList(const std::string& TargetListName, const std::string& SourceListName, bool bAddAll, int SourceTrackIndex);
    bool RemoveFromTrackList(const std::string& ListName, int TrackIndex);
    
    void Clear();

private:
    std::vector<FTrackList> TrackLists;
    int ActiveTrackListIndex;
    
    FTrackList& GetActiveList();
    const FTrackList& GetActiveList() const;
    FTrackList* FindTrackListByName(const std::string& Name);
    const FTrackList* FindTrackListByName(const std::string& Name) const;
    FTrackList& GetBufferList();
    FTrackList& GetAllList();
    FTrackList& GetFavoriteList();
    const FTrackList& GetAllList() const;
    
    bool ContainsTrack(const FTrackList& List, const std::filesystem::path& TrackPath) const;
    bool AddTrackIfMissing(FTrackList& List, const std::filesystem::path& TrackPath);
    bool IsProtectedListName(const std::string& Name) const;
    std::string NormalizeListName(const std::string& Name) const;
};
