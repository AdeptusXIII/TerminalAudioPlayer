//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#pragma once

#include "Types/PlaybackTypes.h"
#include "Audio/AudioBackend.h"
#include "TrackLibrary/TrackLibrary.h"
#include "TrackLibrary/TrackLibraryStorage.h"
#include "Audio/AudioFileScanner.h"
#include "UI/ConsoleIO/ConsoleIO.h"

#include <string>
#include <thread>
#include <atomic>
#include <mutex>

struct FUISnapshotData;
struct FTrackInfo;

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
    void HandleSelectCommand(const std::string &ArgIndex);
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
    
    void StartAudioSyncThread();
    void StopAudioSyncThread();
    
    FUISnapshotData BuildUISnapshotData();
    FTrackInfo BuildTrackInfoData();

private: // --- Variables ---
    
    EAudioPlayerState AudioPlayerState;
    EPlaybackMode PlaybackMode;
    
    bool bWantExit;
    
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
};
