//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#pragma once

#include "Types/PlaybackTypes.h"
#include "Audio/AudioBackend.h"
#include "TrackLibrary/TrackLibrary.h"
#include "TrackLibrary/TrackLibraryStorage.h"
#include "Audio/AudioFileScanner.h"
#include "UI/ConsoleIO/ConsoleIO.h"
#include "System/PowerEventWatcher.h"

#include <string>
#include <thread>
#include <atomic>
#include <mutex>

struct FUISnapshotData;
struct FTrackInfo;

struct FNowPlayingInfo
{
    std::filesystem::path Path;
    std::string TrackName = "None";
    std::string SourceListName = "None";
    int SourceIndex = -1;
    bool bValid = false;
};

struct FPendingPlaybackInfo
{
    std::filesystem::path Path;
    std::string SourceListName = "None";
    int SourceIndex = -1;
    bool bValid = false;
};

class MEngine 
{
public:
    MEngine();
    
    void Init();
    void SyncAudioState();
    void RunCommandLineLoop();
    void RunTUILoop();

private: // --- Functions ---
    
    void RenderTUIFrame(const std::wstring& InputBuffer, std::size_t InputCursorIndex);
    bool HandleTUIControlKey(const FTUIInputEvent& InputEvent, std::wstring& InputBuffer, std::size_t& InputCursorIndex);
    void HandleTUIEnter(std::wstring& InputBuffer, std::size_t& InputCursorIndex);
    void HandleTUICharacterInput(wchar_t Character, std::wstring& InputBuffer, std::size_t& InputCursorIndex);
    
    void ExecuteCommandPrompt(const FCommand &InCommandPrompt);
    
    void HandleModeCommand(const std::string &Arg);
    void HandleHelpCommand(const std::string &Arg);
    void HandlePlayCommand(const std::vector<std::string>& Args);
    void HandleNextCommand(const std::vector<std::string>& Args);
    void HandlePrevCommand(const std::vector<std::string>& Args);
    bool HandleVolumeCommand(const std::string &Arg);
    void HandleScanCommand(const std::vector<std::string> &Args);
    void HandlePlaylistCommand(const std::vector<std::string>& Args);
    
    bool CommandArgIsInt(const std::string &Arg);
    bool SaveAllTrackList();
    void PrintTrackListSummaries();
    std::filesystem::path ExpandUserPath(const std::string& Path) const;
    
    // UNIVERSAL FUNCTIONS
    void SetAudioPlayerState(EAudioPlayerState TargetAudioPlayerState);
    bool CreateDefaultContentDir();
    void WriteTrackListToTrackLibrary(const std::filesystem::path &InPath);
    void RefreshTrackLibrary(const std::filesystem::path &InPath);
    void ApplyCurrentVolume();
    std::vector<std::pair<int, std::string>> FindTracksByName(const std::string &Arg);
    
    bool TryExit();
    bool TryPrev();
    bool TryPlay();
    bool TryPause();
    bool TryStop();
    bool TryNext();
    
    bool TryPlayNextTrackOrFirst();
    bool TryPlayPrevTrackOrLast();
    bool TryPlayRandomTrack();
    bool TryPlayCurrentTrack();
    bool TryPlayTrackByIndex(int TrackIndex);
    bool DeferOrPlayTrackByIndex(int TrackIndex);
    bool DeferOrPlayNextTrack();
    bool DeferOrPlayPrevTrack();
    bool TryPlayPendingTrack();
    
    void StartAudioSyncThread();
    void StopAudioSyncThread();
    void StartPowerEventWatcher();
    void StopPowerEventWatcher();
    void HandleSystemSleepStart();
    void HandleSystemResume();
    
    FUISnapshotData BuildUISnapshotData();
    FTrackInfo BuildTrackInfoData();
    void SetNowPlaying(const std::filesystem::path& Path);
    void ClearNowPlaying();
    void SetPendingPlayback(const std::filesystem::path& Path, int SourceIndex);
    void ClearPendingPlayback();

private: // --- Variables ---
    
    EAudioPlayerState AudioPlayerState;
    EPlaybackMode PlaybackMode;
    
    FNowPlayingInfo NowPlayingInfo;
    FPendingPlaybackInfo PendingPlaybackInfo;
    
    bool bWantExit;
    bool bPausedBySystemSleep;
    
    float CurrentVolume;
    
    std::filesystem::path DefaultContentDir;
    std::thread AudioSyncThread;
    std::atomic<bool> bAudioSyncThreadRunning;
    std::mutex EngineMutex;
    
    MAudioBackend AudioBackend;
    MTrackLibrary TrackLibrary;
    MTrackLibraryStorage TrackLibraryStorage;
    MAudioFileScanner AudioFileScanner;
    MConsoleIO ConsoleIO;
    MPowerEventWatcher PowerEventWatcher;
};
