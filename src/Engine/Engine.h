//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".
#pragma once

#include "Engine/MenuDefinition.h"
#include "Audio/AudioBackend.h"
#include "FileManager/FileManager.h"
#include "TrackLibrary/TrackLibrary.h"
#include "Audio/AudioFileScanner.h"

#include <string>
#include <thread>
#include <atomic>
#include <mutex>

enum class EAudioPlayerState
{
    Idle,
    Playing,
    Paused,
    
    None
};

enum class EPlaybackMode 
{
    Once,
    LoopOne,
    LoopAll,
    LoopShuffle,
    
    None
};

class MEngine 
{
public:
    MEngine();
    
    void Init();
    void SyncAudioState();
    void RunMainLoop();

private: // --- Functions ---
    
    void PrintMainMenuAndState();
    void PrintLibraryMenuAndState();
    void PrintPlaybackModeMenuAndState();
    void PrintAudioPlayerState();
    void PrintPlaybackMode();
    void PrintTrackList();
    void PrintCurrentTrack();
    void PrintMenuSection();
    
    EMainMenuOption ShowMainMenuAndGetOption();
    ELibraryMenuOption ShowLibraryMenuAndGetOption();
    EPlaybackMenuOption ShowPlaybackModeMenuAndGetOption();
    
    void OpenMenuLibrary();
    void OpenMenuPlaybackMode();
    
    void SetAudioPlayerState(EAudioPlayerState TargetAudioPlayerState);
    void HandleMainMenuOption(EMainMenuOption InOption);
    void HandleLibraryMenuOption(ELibraryMenuOption InOption);
    void HandlePlaybackModeMenuOption(EPlaybackMenuOption InOption);
    
    bool TryExitOption();
    bool TryPrevOption();
    bool TryPlayOption();
    bool TryPauseOption();
    bool TryStopOption();
    bool TryNextOption();
    bool TryPlayNextTrackOrFirst();
    bool TryPlayRandomTrack();
    
    bool CreateDefaultContentDir();
    void WriteTrackListToTrackLibrary(const std::filesystem::path &InPath);
    void RefreshTrackLibrary(const std::filesystem::path &InPath);
    void SelectTrackByIndex();
    int ReadIntInRange(int Min, int Max, const std::string& Prompt);
    
    void StartAudioSyncThread();
    void StopAudioSyncThread();
    
private: // --- Variables ---
    
    EAudioPlayerState AudioPlayerState;
    EPlaybackMode PlaybackMode;
    EMenuSection CurrentMenuSection;
    
    bool bForcePlayAfterSwitch;
    bool bWantExit;
    bool bWantBackFromLibraryMenu;
    bool bWantBackFromPlaybackMenu;
    bool bPrintDebugInfo;
    
    std::string DefaultContentDir;
    std::thread AudioSyncThread;
    std::atomic<bool> bAudioSyncThreadRunning;
    std::mutex EngineMutex;
    
    MAudioBackend AudioBackend;
    MFileManager FileManager;
    MTrackLibrary TrackLibrary;
    MAudioFileScanner AudioFileScanner;
};