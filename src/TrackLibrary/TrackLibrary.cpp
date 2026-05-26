//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#include "TrackLibrary.h"

#include <algorithm>

MTrackLibrary::MTrackLibrary()
{
    TrackLists.push_back({ "buffer", ETrackListKind::Buffer, {}, 0 });
    TrackLists.push_back({ "all", ETrackListKind::All, {}, 0 });
    TrackLists.push_back({ "favorite", ETrackListKind::Favorite, {}, 0 });
    ActiveTrackListIndex = 1;
}

void MTrackLibrary::AddTrackToTrackList(const std::filesystem::path &InPath)
{
    AddTrackIfMissing(GetActiveList(), InPath);
}

void MTrackLibrary::AddTracksToTrackList(const std::vector<std::filesystem::path> &PathsList)
{
    for (const std::filesystem::path& Path : PathsList)
    {
        AddTrackToTrackList(Path);
    }
}

bool MTrackLibrary::SetPrevIndex()
{
    FTrackList& ActiveList = GetActiveList();

    if (ActiveList.CurrentIndex > 0 && ActiveList.CurrentIndex < GetTrackListSize())
    {
        ActiveList.CurrentIndex--;
        return true;
    }

    return false;
}

bool MTrackLibrary::SetNextIndex()
{
    FTrackList& ActiveList = GetActiveList();

    if (ActiveList.CurrentIndex + 1 < GetTrackListSize())
    {
        ActiveList.CurrentIndex++;
        return true;
    }

    return false;
}

bool MTrackLibrary::SetCurrentIndex(int Index)
{
    FTrackList& ActiveList = GetActiveList();

    if (Index >= 0 && Index < GetTrackListSize())
    {
        ActiveList.CurrentIndex = Index;
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
    const FTrackList& ActiveList = GetActiveList();

    if (Index < 0 || Index >= static_cast<int>(ActiveList.Tracks.size()))
    {
        return "";
    }

    return ActiveList.Tracks[Index].filename().string();
}

std::string MTrackLibrary::GetCurrentTrackName() const
{
    return GetTrackNameByIndex(GetCurrentIndex());
}

int MTrackLibrary::GetTrackListSize() const
{
    return static_cast<int>(GetActiveList().Tracks.size());
}

int MTrackLibrary::GetCurrentIndex() const
{
    return GetActiveList().CurrentIndex;
}

std::filesystem::path MTrackLibrary::GetCurrentTrackPath() const
{
    return GetTrackPathByIndex(GetCurrentIndex());
}

std::filesystem::path MTrackLibrary::GetPrevTrackPath() const
{
    const int PrevIndex = GetCurrentIndex() - 1;
    return GetTrackPathByIndex(PrevIndex);
}

std::filesystem::path MTrackLibrary::GetNextTrackPath() const
{
    const int NextIndex = GetCurrentIndex() + 1;
    return GetTrackPathByIndex(NextIndex);
}

std::filesystem::path MTrackLibrary::GetTrackPathByIndex(int Index) const
{
    const FTrackList& ActiveList = GetActiveList();

    if (Index < 0 || Index >= static_cast<int>(ActiveList.Tracks.size()))
    {
        return {};
    }

    return ActiveList.Tracks[Index];
}

std::vector<std::filesystem::path> MTrackLibrary::GetAllTrackPaths() const
{
    return GetAllList().Tracks;
}

std::vector<std::filesystem::path> MTrackLibrary::GetFavoriteTrackPaths() const
{
    const FTrackList* FavoriteList = FindTrackListByName("favorite");

    if (FavoriteList == nullptr)
    {
        return {};
    }

    return FavoriteList->Tracks;
}

std::vector<FCustomTrackListData> MTrackLibrary::GetCustomTrackLists() const
{
    std::vector<FCustomTrackListData> Result;

    for (const FTrackList& TrackList : TrackLists)
    {
        if (TrackList.Kind != ETrackListKind::Custom) continue;

        FCustomTrackListData CustomList;
        CustomList.Name = TrackList.Name;
        CustomList.Tracks = TrackList.Tracks;

        Result.emplace_back(CustomList);
    }

    return Result;
}

std::string MTrackLibrary::GetActiveTrackListName() const
{
    return GetActiveList().Name;
}

int MTrackLibrary::GetActiveTrackListIndex() const
{
    return ActiveTrackListIndex;
}

int MTrackLibrary::GetTrackListCount() const
{
    return static_cast<int>(TrackLists.size());
}

std::vector<std::string> MTrackLibrary::GetTrackListSummaries() const
{
    std::vector<std::string> Summaries;

    for (std::size_t i = 0; i < TrackLists.size(); i++)
    {
        std::string Summary = "[" + std::to_string(i) + "] " + TrackLists[i].Name
            + "  " + std::to_string(TrackLists[i].Tracks.size()) + " tracks";

        if (static_cast<int>(i) == ActiveTrackListIndex)
        {
            Summary += " <- active";
        }

        Summaries.emplace_back(Summary);
    }

    return Summaries;
}

bool MTrackLibrary::SetActiveTrackListByName(const std::string &Name)
{
    const std::string NormalizedName = NormalizeListName(Name);

    for (std::size_t i = 0; i < TrackLists.size(); i++)
    {
        if (TrackLists[i].Name == NormalizedName)
        {
            ActiveTrackListIndex = static_cast<int>(i);
            return true;
        }
    }

    return false;
}

bool MTrackLibrary::SetActiveTrackListByIndex(int Index)
{
    if (Index < 0 || Index >= static_cast<int>(TrackLists.size()))
    {
        return false;
    }

    ActiveTrackListIndex = Index;
    return true;
}

bool MTrackLibrary::SetBufferTracks(const std::vector<std::filesystem::path> &Paths)
{
    FTrackList& BufferList = GetBufferList();
    BufferList.Tracks.clear();
    BufferList.CurrentIndex = 0;

    for (const std::filesystem::path& Path : Paths)
    {
        AddTrackIfMissing(BufferList, Path);
    }

    return true;
}

bool MTrackLibrary::SetAllTracks(const std::vector<std::filesystem::path> &Paths)
{
    FTrackList& AllList = GetAllList();
    AllList.Tracks.clear();
    AllList.CurrentIndex = 0;

    for (const std::filesystem::path& Path : Paths)
    {
        AddTrackIfMissing(AllList, Path);
    }

    return true;
}

bool MTrackLibrary::SetFavoriteTracks(const std::vector<std::filesystem::path> &TrackPaths)
{
    FTrackList* FavoriteList = FindTrackListByName("favorite");

    if (FavoriteList == nullptr) return false;

    FavoriteList->Tracks = TrackPaths;
    FavoriteList->CurrentIndex = 0;
    return true;
}

bool MTrackLibrary::SetCustomTrackLists(const std::vector<FCustomTrackListData> &CustomTrackLists)
{
    TrackLists.erase(
        std::remove_if(
            TrackLists.begin(),
            TrackLists.end(),
            [](const FTrackList& TrackList)
            {
                return TrackList.Kind == ETrackListKind::Custom;
            }),
            TrackLists.end());

    for (const FCustomTrackListData& CustomTrackList : CustomTrackLists)
    {
        const std::string NormalizedName = NormalizeListName(CustomTrackList.Name);

        if (NormalizedName.empty() || IsProtectedListName(NormalizedName)) continue;

        FTrackList NewList;
        NewList.Name = NormalizedName;
        NewList.Kind = ETrackListKind::Custom;
        NewList.Tracks = CustomTrackList.Tracks;
        NewList.CurrentIndex = 0;

        TrackLists.emplace_back(NewList);
    }

    if (ActiveTrackListIndex >= static_cast<int>(TrackLists.size()))
    {
        ActiveTrackListIndex = 1;
    }

    return true;
}

bool MTrackLibrary::ImportFromBuffer(bool bImportAll, int BufferIndex)
{
    FTrackList& BufferList = GetBufferList();
    FTrackList& AllList = GetAllList();

    if (bImportAll)
    {
        bool bChanged = false;

        for (const std::filesystem::path& TrackPath : BufferList.Tracks)
        {
            bChanged = AddTrackIfMissing(AllList, TrackPath) || bChanged;
        }

        return bChanged;
    }

    if (BufferIndex < 0 || BufferIndex >= static_cast<int>(BufferList.Tracks.size()))
    {
        return false;
    }

    return AddTrackIfMissing(AllList, BufferList.Tracks[BufferIndex]);
}

bool MTrackLibrary::CreateCustomTrackList(const std::string &Name)
{
    const std::string NormalizedName = NormalizeListName(Name);

    if (NormalizedName.empty() || IsProtectedListName(NormalizedName) || FindTrackListByName(NormalizedName) != nullptr)
    {
        return false;
    }

    TrackLists.push_back({ NormalizedName, ETrackListKind::Custom, {}, 0 });
    return true;
}

bool MTrackLibrary::DeleteCustomTrackList(const std::string &Name)
{
    const std::string NormalizedName = NormalizeListName(Name);

    for (std::size_t i = 3; i < TrackLists.size(); i++)
    {
        if (TrackLists[i].Name == NormalizedName)
        {
            TrackLists.erase(TrackLists.begin() + static_cast<long>(i));
            ActiveTrackListIndex = std::min(ActiveTrackListIndex, static_cast<int>(TrackLists.size()) - 1);
            return true;
        }
    }

    return false;
}

bool MTrackLibrary::AddToFavoritesFromList(const std::string &SourceListName, int SourceTrackIndex)
{
    FTrackList* SourceList = FindTrackListByName(SourceListName);

    if (SourceList == nullptr || SourceTrackIndex < 0 || SourceTrackIndex >= static_cast<int>(SourceList->Tracks.size()))
    {
        return false;
    }

    const std::filesystem::path TrackPath = SourceList->Tracks[SourceTrackIndex];

    AddTrackIfMissing(GetAllList(), TrackPath);
    return AddTrackIfMissing(GetFavoriteList(), TrackPath);
}

bool MTrackLibrary::AddToTrackListFromList(const std::string &TargetListName, const std::string &SourceListName, bool bAddAll, int SourceTrackIndex)
{
    FTrackList* TargetList = FindTrackListByName(TargetListName);
    FTrackList* SourceList = FindTrackListByName(SourceListName);

    if (TargetList == nullptr || SourceList == nullptr || TargetList->Kind == ETrackListKind::Buffer)
    {
        return false;
    }

    if (bAddAll)
    {
        bool bChanged = false;

        for (const std::filesystem::path& TrackPath : SourceList->Tracks)
        {
            AddTrackIfMissing(GetAllList(), TrackPath);
            bChanged = AddTrackIfMissing(*TargetList, TrackPath) || bChanged;
        }

        return bChanged;
    }

    if (SourceTrackIndex < 0 || SourceTrackIndex >= static_cast<int>(SourceList->Tracks.size()))
    {
        return false;
    }

    const std::filesystem::path TrackPath = SourceList->Tracks[SourceTrackIndex];
    AddTrackIfMissing(GetAllList(), TrackPath);
    return AddTrackIfMissing(*TargetList, TrackPath);
}

bool MTrackLibrary::RemoveFromTrackList(const std::string &ListName, int TrackIndex)
{
    FTrackList* List = FindTrackListByName(ListName);

    if (List == nullptr || List->Kind == ETrackListKind::Buffer)
    {
        return false;
    }

    if (TrackIndex < 0 || TrackIndex >= static_cast<int>(List->Tracks.size()))
    {
        return false;
    }

    List->Tracks.erase(List->Tracks.begin() + TrackIndex);

    if (List->CurrentIndex >= static_cast<int>(List->Tracks.size()))
    {
        List->CurrentIndex = 0;
    }

    return true;
}

void MTrackLibrary::Clear()
{
    GetActiveList().Tracks.clear();
    GetActiveList().CurrentIndex = 0;
}

FTrackList& MTrackLibrary::GetActiveList()
{
    return TrackLists[ActiveTrackListIndex];
}

const FTrackList& MTrackLibrary::GetActiveList() const
{
    return TrackLists[ActiveTrackListIndex];
}

FTrackList* MTrackLibrary::FindTrackListByName(const std::string &Name)
{
    const std::string NormalizedName = NormalizeListName(Name);

    for (FTrackList& List : TrackLists)
    {
        if (List.Name == NormalizedName)
        {
            return &List;
        }
    }

    return nullptr;
}

const FTrackList* MTrackLibrary::FindTrackListByName(const std::string &Name) const
{
    const std::string NormalizedName = NormalizeListName(Name);

    for (const FTrackList& List : TrackLists)
    {
        if (List.Name == NormalizedName)
        {
            return &List;
        }
    }

    return nullptr;
}

FTrackList& MTrackLibrary::GetBufferList()
{
    return TrackLists[0];
}

FTrackList& MTrackLibrary::GetAllList()
{
    return TrackLists[1];
}

FTrackList& MTrackLibrary::GetFavoriteList()
{
    return TrackLists[2];
}

const FTrackList& MTrackLibrary::GetAllList() const
{
    return TrackLists[1];
}

bool MTrackLibrary::ContainsTrack(const FTrackList &List, const std::filesystem::path &TrackPath) const
{
    return std::find(List.Tracks.begin(), List.Tracks.end(), TrackPath) != List.Tracks.end();
}

bool MTrackLibrary::AddTrackIfMissing(FTrackList &List, const std::filesystem::path &TrackPath)
{
    if (TrackPath.empty() || ContainsTrack(List, TrackPath))
    {
        return false;
    }

    List.Tracks.push_back(TrackPath);
    return true;
}

bool MTrackLibrary::IsProtectedListName(const std::string &Name) const
{
    const std::string NormalizedName = NormalizeListName(Name);
    return NormalizedName == "buffer" || NormalizedName == "all" || NormalizedName == "favorite";
}

std::string MTrackLibrary::NormalizeListName(const std::string &Name) const
{
    if (Name == "fav")
    {
        return "favorite";
    }

    if (Name == "general")
    {
        return "all";
    }

    return Name;
}
