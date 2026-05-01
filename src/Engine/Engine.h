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

class MEngine 
{
public:
    MEngine();
    
    void Init();
    void SyncAudioState();
    void RunCommandLineLoop();

private: // --- Functions ---
    
    // COMMAND LINE MODE FUNCTIONS
    void HandleCommandPromt(const FCommand &InCommandPrompt);
    void HandleCommandArg_Mode(const std::string &Arg);
    void HandleCommandArg_Help(const std::string &Arg);
    
    void CommandSelectTrackByIndex(const std::string &ArgIndex);
    bool CommandArgIsInt(const std::string &Arg);
    bool CommandParseVolume(const std::string &Arg);
    
    // UNIVERSAL FUNCTIONS
    void SetAudioPlayerState(EAudioPlayerState TargetAudioPlayerState);
    bool CreateDefaultContentDir();
    void WriteTrackListToTrackLibrary(const std::filesystem::path &InPath);
    void RefreshTrackLibrary(const std::filesystem::path &InPath);
    void ApplyCurrentVolume();
    
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