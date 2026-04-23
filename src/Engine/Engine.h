//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".
#pragma once

#include "Engine/MenuDefinition.h"
#include "Audio/AudioBackend.h"
#include "FileManager/FileManager.h"
#include "TrackLibrary/TrackLibrary.h"
#include "Audio/AudioFileScanner.h"

#include <string>

enum class EAudioPlayerState
{
    Idle,
    Playing,
    Paused,
};

class MEngine 
{
public:
    MEngine();
    
    void Init();
    void SyncAudioState();
    void RunMainLoop();

private:
    void PrintMainMenuAndState();
    void PrintLibraryMenuAndState();
    void PrintAudioPlayerState();
    void PrintTrackList();
    void PrintCurrentTrack();
    void PrintMenuSection();
    
    EMainMenuOption ShowMainMenuAndGetOption();
    void SelectTrackByIndex();
    int ReadIntInRange(int Min, int Max, const std::string& Prompt);
    ELibraryMenuOption ShowLibraryMenuAndGetOption();
    
    void SetAudioPlayerState(EAudioPlayerState TargetAudioPlayerState);
    void HandleMainMenuOption(EMainMenuOption InOption);
    void HandleLibraryMenuOption(ELibraryMenuOption InOption);
    
    bool TryExitOption();
    bool TryPrevOption();
    bool TryPlayOption();
    bool TryPauseOption();
    bool TryStopOption();
    bool TryNextOption();
    
    void OpenLibrary();
    
    bool CreateDefaultContentDir();
    void WriteTrackListToTrackLibrary(const std::filesystem::path &InPath);
    void RefreshTrackLibrary(const std::filesystem::path &InPath);
    
    EAudioPlayerState AudioPlayerState;
    EMenuSection CurrentMenuSection;
    bool bForcePlayAfterSwitch;
    bool bWantExit;
    bool bWantBackFromLibrary;
    bool bPrintDebugInfo;
    std::string DefaultContentDir;
    MAudioBackend AudioBackend;
    MFileManager FileManager;
    MTrackLibrary TrackLibrary;
    MAudioFileScanner AudioFileScanner;
};