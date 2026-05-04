//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#pragma once

#include "Types/PlaybackTypes.h"
#include "Audio/AudioBackend.h"
#include "FileManager/FileManager.h"
#include "TrackLibrary/TrackLibrary.h"
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
    
    void ExecuteCommandPrompt(const FCommand &InCommandPrompt);
    
    void HandleModeCommand(const std::string &Arg);
    void HandleHelpCommand(const std::string &Arg);
    void HandleSelectCommand(const std::string &ArgIndex);
    bool HandleVolumeCommand(const std::string &Arg);
    
    bool CommandArgIsInt(const std::string &Arg);
    
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
    
    std::string DefaultContentDir;
    std::thread AudioSyncThread;
    std::atomic<bool> bAudioSyncThreadRunning;
    std::mutex EngineMutex;
    
    MAudioBackend AudioBackend;
    MFileManager FileManager;
    MTrackLibrary TrackLibrary;
    MAudioFileScanner AudioFileScanner;
    MConsoleIO ConsoleIO;
};