//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".
#include "Engine.h"

#include <iostream>
#include <limits>
#include <filesystem>
#include <chrono>

MEngine::MEngine()
    
{
    AudioPlayerState = EAudioPlayerState::Idle;
    PlaybackMode = EPlaybackMode::LoopAll;
    CurrentMenuSection = EMenuSection::MainMenu;
    bForcePlayAfterSwitch = true;
    bWantExit = false;
    bWantBackFromLibraryMenu = false;
    bWantBackFromPlaybackMenu = false;
    bPrintDebugInfo = false;
    DefaultContentDir = std::getenv("HOME") + std::string("/Music/TAP_content");
    bAudioSyncThreadRunning = false;
}

void MEngine::Init()
{
    std::string VersionNumber = "0.10";
    std::cout << "TAP - Terminal Audio Player V." << VersionNumber << std::endl;
    
    FileManager.Init(bPrintDebugInfo);   
    AudioBackend.Init(bPrintDebugInfo);
    
    CreateDefaultContentDir();
    WriteTrackListToTrackLibrary(DefaultContentDir);
}

void MEngine::SyncAudioState() 
{
    EPlaybackMode CurrentPlaybackMode = EPlaybackMode::Once;
    bool bShouldReact = false;
    
    {
        std::lock_guard<std::mutex> Lock(EngineMutex);
        
        if (AudioPlayerState == EAudioPlayerState::Playing && AudioBackend.IsStopped()) 
        {
            bShouldReact = true;
            CurrentPlaybackMode = PlaybackMode;
        }
    }
    
    if (!bShouldReact) return;
    
    if (CurrentPlaybackMode == EPlaybackMode::LoopAll) 
    {
        if (TryPlayNextTrackOrFirst()) 
        {
            return;
        }
    }
        
    if (CurrentPlaybackMode == EPlaybackMode::LoopShuffle) 
    {
        if (TryPlayRandomTrack()) 
        {
            return;
        }
    }
    
    std::lock_guard<std::mutex> Lock(EngineMutex);
    SetAudioPlayerState(EAudioPlayerState::Idle);
}

void MEngine::RunMainLoop() 
{
    StartAudioSyncThread();
    while (bWantExit == false)
    {
        EMainMenuOption SelectedOption = EMainMenuOption::None;
        SelectedOption = ShowMainMenuAndGetOption();
        HandleMainMenuOption(SelectedOption);
    }
    StopAudioSyncThread();
}

void MEngine::PrintMainMenuAndState()
{
    PrintAudioPlayerState();
    PrintPlaybackMode();
    PrintCurrentTrack();
    PrintMenuSection();
    
    std::cout << "Choose option: " << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(1) - [ Prev ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(2) - [ Play ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(3) - [ Pause ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(4) - [ Stop ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(5) - [ Next ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(6) - [ Library ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(7) - [ Playback-Mode ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_L << "(0) - [ Exit ]" << std::endl;
}

void MEngine::PrintLibraryMenuAndState() 
{
    PrintAudioPlayerState();
    PrintPlaybackMode();
    PrintCurrentTrack();
    PrintMenuSection();
    
    std::cout << "Choose option: " << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(1) - [ List ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(2) - [ Select track by index ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(3) - [ Refresh ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_L << "(0) - [ Back ]" << std::endl;
}

void MEngine::PrintPlaybackModeMenuAndState() 
{
    PrintAudioPlayerState();
    PrintPlaybackMode();
    PrintCurrentTrack();
    PrintMenuSection();
    
    std::cout << "Choose option: " << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(1) - [ Once ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(2) - [ Loop One ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(3) - [ Loop All ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(4) - [ Loop Shuffle ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_L << "(0) - [ Back ]" << std::endl;
}

void MEngine::PrintAudioPlayerState()
{
    EAudioPlayerState CurrentAudioPlayerState = EAudioPlayerState::None;
    std::lock_guard<std::mutex> Lock(EngineMutex);
    {
        CurrentAudioPlayerState = AudioPlayerState;
    }
    
    switch (CurrentAudioPlayerState)
    {
        case EAudioPlayerState::Idle:
            std::cout << std::endl << "[TAP::STATE::IDLE]" << std::endl;
            break;

        case EAudioPlayerState::Playing:
            std::cout << std::endl <<  "[TAP::STATE::PLAYING]" << std::endl;
            break;

        case EAudioPlayerState::Paused:
            if (bPrintDebugInfo) 
                std::cout << std::endl << "[TAP::STATE::PAUSED]" << std::endl;
            break;
    }
}

void MEngine::PrintPlaybackMode() 
{
    EPlaybackMode CurrentPlaybackMode = EPlaybackMode::None;
    std::lock_guard<std::mutex> Lock(EngineMutex);
    {
        CurrentPlaybackMode = PlaybackMode;
    }
    
    switch (CurrentPlaybackMode) 
    {
        case EPlaybackMode::Once:
            if (bPrintDebugInfo) 
            {
                std::cout << "[PlaybackMode]::Once" << std::endl;
            }
            else 
            {
                std::cout << "[TAP::PLAYBACK-MODE::ONCE]" << std::endl;
            }
            break;
            
        case EPlaybackMode::LoopOne:
            if (bPrintDebugInfo) 
            {
                std::cout << "[PlaybackMode]::LoopOne" << std::endl;
            }
            else 
            {
                std::cout << "[TAP::PLAYBACK-MODE::LOOP-ONE]" << std::endl;
            }
            break;
            
        case EPlaybackMode::LoopAll:
            if (bPrintDebugInfo) 
            {
                std::cout << "[PlaybackMode]::LoopAll" << std::endl;
            }
            else 
            {
                std::cout<< "[TAP::PLAYBACK-MODE::LOOP-ALL]" << std::endl;
            }
            break;   
            
        case EPlaybackMode::LoopShuffle:
            if (bPrintDebugInfo) 
            {
                std::cout << "[PlaybackMode]::LoopShuffle" << std::endl;
            }
            else 
            {
                std::cout << "[TAP::PLAYBACK-MODE::LOOP-SHUFFLE]" << std::endl;
            }
            break;        
    }
}

void MEngine::PrintTrackList()
{
    int TrackCount = 0;
    int CurrentIndex = 0;
    std::vector<std::string> TrackNames;

    {
        std::lock_guard<std::mutex> Lock(EngineMutex);

        TrackCount = TrackLibrary.GetTrackListSize();
        CurrentIndex = TrackLibrary.GetCurrentIndex();

        for (int i = 0; i < TrackCount; i++)
        {
            TrackNames.push_back(TrackLibrary.GetTrackNameByIndex(i));
        }
    }

    std::cout << std::endl;
    PrintMenuSection();
    std::cout << "Total tracks found: " << TrackCount << std::endl;

    for (int i = 0; i < TrackCount; i++)
    {
        std::string CurrentTrack = (i == CurrentIndex) ? " <- current/selected" : "";
        std::string SubCatTorL = (i == TrackCount - 1) ? SUBCAT_SEP_L : SUBCAT_SEP_T;

        std::cout << SUBCAT_SEP_TAB + SubCatTorL 
                  << "Index(" << i << ")" 
                  << "[" << TrackNames[i] 
                  << "] "
                  << CurrentTrack
                  << std::endl;
    }
}

void MEngine::PrintCurrentTrack() 
{
    std::string CurrentTrackName = "None";
    std::lock_guard<std::mutex> Lock(EngineMutex);
    {
        CurrentTrackName = TrackLibrary.GetCurrentTrackName();
    }
    
    std::cout << "[TAP::SELECTED TRACK:: " << CurrentTrackName << "]" << std::endl;
}

void MEngine::PrintMenuSection() 
{
    std::string MenuSection = {};
    switch (CurrentMenuSection) 
    {
        case EMenuSection::MainMenu:
            MenuSection = "MENU";
            break;
            
        case EMenuSection::LibraryMenu:
            MenuSection = "LIBRARY";
            break;
            
        case EMenuSection::PlaybackMenu:
            MenuSection = "PLAYBACK-MODE";
            break;    
    }
    
    std::cout << "[TAP::" << MenuSection << "] ";
}

EMainMenuOption MEngine::ShowMainMenuAndGetOption()
{
    PrintMainMenuAndState();
    
    int Option = ReadIntInRange(0, MAX_MAIN_MENU_OPTION_COUNT, "[TAP::MENU] Enter opt: ");
    
    return static_cast<EMainMenuOption>(Option);
}

ELibraryMenuOption MEngine::ShowLibraryMenuAndGetOption() 
{
    PrintLibraryMenuAndState();
    
    int Option = ReadIntInRange(0, MAX_LIBRARY_MENU_OPTION_COUNT, "[TAP::LIBRARY] Enter opt: ");
    
    return static_cast<ELibraryMenuOption>(Option);
}

EPlaybackMenuOption MEngine::ShowPlaybackModeMenuAndGetOption() 
{
    PrintPlaybackModeMenuAndState();
    
    int Option = ReadIntInRange(0, MAX_PLAYBACK_MENU_OPTION_COUNT, "[TAP::PLAYBACK-MODE] Enter opt: ");
    
    return static_cast<EPlaybackMenuOption>(Option);
}

void MEngine::OpenMenuLibrary() 
{
    CurrentMenuSection = EMenuSection::LibraryMenu;
    while (!bWantBackFromLibraryMenu) 
    {
        ELibraryMenuOption Option = ELibraryMenuOption::None;
        Option = ShowLibraryMenuAndGetOption();
        HandleLibraryMenuOption(Option);
    }
    bWantBackFromLibraryMenu = false;
    CurrentMenuSection = EMenuSection::MainMenu;
}

void MEngine::OpenMenuPlaybackMode() 
{
    CurrentMenuSection = EMenuSection::PlaybackMenu;
    while (!bWantBackFromPlaybackMenu) 
    {
        EPlaybackMenuOption Option = EPlaybackMenuOption::None;
        Option = ShowPlaybackModeMenuAndGetOption();
        HandlePlaybackModeMenuOption(Option);
    }
    bWantBackFromPlaybackMenu = false;
    CurrentMenuSection = EMenuSection::MainMenu;
}

void MEngine::SetAudioPlayerState(EAudioPlayerState TargetAudioPlayerState)
{
    AudioPlayerState = TargetAudioPlayerState;
}

void MEngine::HandleMainMenuOption(EMainMenuOption InOption)
{
    switch (InOption)
    {
        case EMainMenuOption::Prev:
            if (!TryPrevOption()) 
            {
                std::cout << std::endl << "[TAP::WARNING] No previous track." << std::endl;
            }
            break;

        case EMainMenuOption::Play:
            if (!TryPlayOption())
            {
                std::cout << std::endl << "[TAP::WARNING] Track is already playing or not selected." << std::endl;
            }
            break;

        case EMainMenuOption::Pause:
            if (!TryPauseOption())
            {
                std::cout << std::endl << "[TAP::WARNING] Track is already paused or not playing." << std::endl;
            }
            break;

        case EMainMenuOption::Stop:
            if (!TryStopOption())
            {
                std::cout << std::endl << "[TAP::WARNING] Track is already stopped." << std::endl;
            }
            break;

        case EMainMenuOption::Next:
            if (!TryNextOption()) 
            {
                std::cout << std::endl << "[TAP::WARNING] No next track." << std::endl;
            }
            break;
        
        case EMainMenuOption::Library:
            OpenMenuLibrary();
            break;
            
        case EMainMenuOption::PlaybackMode:
            OpenMenuPlaybackMode();
            break;    
            
        case EMainMenuOption::Exit:
            if (TryExitOption()) 
            {
                std::cout << std::endl << "[TAP::SHUTDOWN] Exit." << std::endl;
            }
            else 
            {
                std::cerr << std::endl << "[TAP::ERROR] Unexpected Error. Force exit." << std::endl;
                bWantExit = true;
            }
            break;
    }
}

void MEngine::HandleLibraryMenuOption(ELibraryMenuOption InOption)
{
    switch (InOption) {
        case ELibraryMenuOption::List:
            PrintTrackList();
            break;
            
        case ELibraryMenuOption::SelectByIndex:
            SelectTrackByIndex();
            break;
            
        case ELibraryMenuOption::Refresh: 
            {
                RefreshTrackLibrary(DefaultContentDir);
                std::lock_guard<std::mutex> Lock(EngineMutex);
                std::cout << "[TAP::LIBRARY] Library refreshed. Total tracks found: "
                          << TrackLibrary.GetTrackListSize() << "." << std::endl;
                break;
            }
            
        case ELibraryMenuOption::Back:
            bWantBackFromLibraryMenu = true;
            break;
    }
}

void MEngine::HandlePlaybackModeMenuOption(EPlaybackMenuOption InOption) 
{
    std::lock_guard<std::mutex> Lock(EngineMutex);

    switch (InOption) 
    {
        case EPlaybackMenuOption::Once:
            AudioBackend.SetLoop(false);
            PlaybackMode = EPlaybackMode::Once;
            break;
            
        case EPlaybackMenuOption::LoopOne:
            AudioBackend.SetLoop(true);
            PlaybackMode = EPlaybackMode::LoopOne;
            break;
            
        case EPlaybackMenuOption::LoopAll:
            AudioBackend.SetLoop(false);
            PlaybackMode = EPlaybackMode::LoopAll;
            break;
            
        case EPlaybackMenuOption::LoopShuffle:
            AudioBackend.SetLoop(false);
            PlaybackMode = EPlaybackMode::LoopShuffle;
            break;
            
        case EPlaybackMenuOption::Back:
            bWantBackFromPlaybackMenu = true;
            break;    
    }
}

bool MEngine::TryExitOption() 
{
    std::lock_guard<std::mutex> Lock(EngineMutex);

    if (AudioBackend.ShutDown()) 
    {
        SetAudioPlayerState(EAudioPlayerState::Idle);
        bWantExit = true;
        return true;
    }
    
    return false;
}

bool MEngine::TryPrevOption() 
{   
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    std::filesystem::path Path = TrackLibrary.GetPrevTrackPath();
    if (bForcePlayAfterSwitch && !TrackLibrary.IsEmpty() && !Path.empty())
    {
        if (AudioBackend.PlayTrack(Path)) 
        {
            SetAudioPlayerState(EAudioPlayerState::Playing);
            TrackLibrary.SetPrevIndex();
            return true;
        }
        else 
        {
            if (bPrintDebugInfo)
            {
                std::cerr << "[ENGINE] Error while trying to play file: " << Path.string() << std::endl ;
            }
            return false;
        }
    }
    
    if (!bForcePlayAfterSwitch && !TrackLibrary.IsEmpty() && !Path.empty()) 
    {
        SetAudioPlayerState(EAudioPlayerState::Idle);
        AudioBackend.StopTrack();
        TrackLibrary.SetPrevIndex();
        return true;
    }
    
    return false;
}

bool MEngine::TryPlayOption() 
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    std::filesystem::path Path = TrackLibrary.GetCurrentTrackPath();
   
    if (AudioPlayerState == EAudioPlayerState::Paused ) 
    {
        if (AudioBackend.ResumeTrack()) 
        {
            SetAudioPlayerState(EAudioPlayerState::Playing);
            return true;
        }
    }
    if (AudioPlayerState == EAudioPlayerState::Idle)
    {
        if (AudioBackend.PlayTrack(Path)) 
        {
            SetAudioPlayerState(EAudioPlayerState::Playing);
            return true;
        }
        else 
        {
            if (bPrintDebugInfo)
            {
                std::cerr << "[ENGINE] Error while trying to play file: " << Path.string() << std::endl ;
            }
            return false;
        }
    }
    return false;
}

bool MEngine::TryPauseOption() 
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    bool bContinue = false;
    
    if (AudioPlayerState == EAudioPlayerState::Playing)
    {
        if (AudioBackend.PauseTrack()) 
        {
            SetAudioPlayerState(EAudioPlayerState::Paused);
            bContinue = true;
        }
    }
    
    return bContinue;
}

bool MEngine::TryStopOption() 
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    bool bContinue = false;
    
    if (AudioPlayerState == EAudioPlayerState::Playing || AudioPlayerState == EAudioPlayerState::Paused)
    {
        if (AudioBackend.StopTrack()) 
        {
            SetAudioPlayerState(EAudioPlayerState::Idle);
            bContinue = true;
        }
    }
    
    return bContinue;
}

bool MEngine::TryNextOption() 
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    std::filesystem::path Path = TrackLibrary.GetNextTrackPath();
    if (bForcePlayAfterSwitch && !TrackLibrary.IsEmpty() && !Path.empty())
    {
        if (AudioBackend.PlayTrack(Path)) 
        {
            SetAudioPlayerState(EAudioPlayerState::Playing);
            TrackLibrary.SetNextIndex();
            return true;
        }
        else 
        {
            if (bPrintDebugInfo)
            {
                std::cerr << "[ENGINE] Error while trying to play file: " << Path.string() << std::endl ;
            }
            return false;
        }
    }
    
    if (!bForcePlayAfterSwitch && !TrackLibrary.IsEmpty() && !Path.empty()) 
    {
        SetAudioPlayerState(EAudioPlayerState::Idle);
        AudioBackend.StopTrack();
        TrackLibrary.SetNextIndex();
        return true;
    }
    
    return false;
}

bool MEngine::TryPlayNextTrackOrFirst() 
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    std::filesystem::path Path = TrackLibrary.GetNextTrackPath();
    if (Path.empty() && !TrackLibrary.IsEmpty()) 
    {
        std::filesystem::path FirstTrackPath = TrackLibrary.GetTrackPathByIndex(0);
        if (!FirstTrackPath.empty() && AudioBackend.PlayTrack(FirstTrackPath))
        {
            TrackLibrary.SetCurrentIndex(0);
            SetAudioPlayerState(EAudioPlayerState::Playing);
            return true;
        }
        
        std::cout << "[TAP::ERROR] Failed to play first track." << std::endl;
        return false;
    }
    
    if (AudioBackend.PlayTrack(Path)) 
    {
        TrackLibrary.SetNextIndex();
        SetAudioPlayerState(EAudioPlayerState::Playing);
        return true;
    }
    
    std::cout << "[TAP::ERROR] Error while trying to play: " << TrackLibrary.GetCurrentTrackName() << std::endl;
    return false;
}

bool MEngine::TryPlayRandomTrack() 
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    if (TrackLibrary.IsEmpty()) 
    {
        std::cout << "[TAP::ERROR] Track library is empty." << std::endl;
        return false;
    }
    
    int TrackCount = TrackLibrary.GetTrackListSize();
    
    if (TrackCount == 1) 
    {
        int OnlyOneIndex = 0;
        std::filesystem::path Path = TrackLibrary.GetTrackPathByIndex(OnlyOneIndex);
        
        if (!Path.empty() && AudioBackend.PlayTrack(Path)) {
            TrackLibrary.SetCurrentIndex(OnlyOneIndex);
            SetAudioPlayerState(EAudioPlayerState::Playing);
            return true;
        }
        
        std::cout << "[TAP::ERROR] Failed play." << std::endl;
        return false;
    }
    
    
    int NextIndex = rand() % TrackCount;
    while (TrackLibrary.GetCurrentIndex() == NextIndex) 
    {
        NextIndex = rand() % TrackCount;
    }
    
    std::filesystem::path Path = TrackLibrary.GetTrackPathByIndex(NextIndex);
    
    if (AudioBackend.PlayTrack(Path)) 
    {
        TrackLibrary.SetCurrentIndex(NextIndex);
        SetAudioPlayerState(EAudioPlayerState::Playing);
        return true;
    }
    
    std::cout << "[TAP::ERROR] Error while trying to play: " << TrackLibrary.GetCurrentTrackName() << std::endl;
    return false;
}

bool MEngine::CreateDefaultContentDir() 
{
    bool bCreated = false;
    
    if (FileManager.CreateDir(DefaultContentDir))
    {
        bCreated = true;
        if (bPrintDebugInfo) 
        {
            std::cout << "[ENGINE] Created default content directory " << DefaultContentDir << std::endl;
        }
    }
    
    return bCreated;
}

void MEngine::WriteTrackListToTrackLibrary(const std::filesystem::path &InPath) 
{
    TrackLibrary.AddTracksToTrackList(AudioFileScanner.ScanPath(InPath));
}

void MEngine::RefreshTrackLibrary(const std::filesystem::path &InPath)
{
    std::vector<std::filesystem::path> ScannedPath = AudioFileScanner.ScanPath(InPath);
    
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    if (AudioPlayerState == EAudioPlayerState::Playing || AudioPlayerState == EAudioPlayerState::Paused) 
    {
        AudioBackend.StopTrack();
        SetAudioPlayerState(EAudioPlayerState::Idle);
    }
    
    TrackLibrary.Clear();
    TrackLibrary.AddTracksToTrackList(ScannedPath);
}

void MEngine::SelectTrackByIndex() 
{
    int TrackLibrarySize = 0;
    {
        std::lock_guard<std::mutex> Lock(EngineMutex);
        
        if (TrackLibrary.IsEmpty()) return;
        
        TrackLibrarySize = TrackLibrary.GetTrackListSize() - 1;
    }
    
    int TrackIndex = ReadIntInRange(0, TrackLibrarySize, "Enter index: ");
    
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    if (TrackIndex >= TrackLibrary.GetTrackListSize())
    {
        std::cout << "[TAP::ERROR] Track index is no longer valid." << std::endl;
        return;
    }
    
    if (bForcePlayAfterSwitch) 
    {
        std::filesystem::path Path = TrackLibrary.GetTrackPathByIndex(TrackIndex);
        if (Path.empty())
        {
            std::cout << "[TAP::ERROR] Track path is empty." << std::endl;
            return;
        }

        if (AudioBackend.PlayTrack(Path))
        {
            SetAudioPlayerState(EAudioPlayerState::Playing);
            if (!TrackLibrary.SetCurrentIndex(TrackIndex))
            {
                std::cout << "[TAP::ERROR] Failed to set current track index." << std::endl;
                return;
            }
            return;
        }

        std::cout << "[TAP::ERROR] Error while trying to play file: " << Path.string() <<
        std::endl;
    }
    else 
    {
        SetAudioPlayerState(EAudioPlayerState::Idle);
        AudioBackend.StopTrack();
        
        if (!TrackLibrary.SetCurrentIndex(TrackIndex)) 
        {
            std::cout << "[TAP::ERROR] Failed to set current track index." << std::endl;
        }
    }
}

int MEngine::ReadIntInRange(int Min, int Max, const std::string& Prompt)
{
    int Value = -1;
    
    while (true)
    {
        std::cout << Prompt;
        std::cin >> Value;
        
        bool bInputValid = !std::cin.fail();
        bool bInputInRange = bInputValid && !(Value < Min || Value > Max);
        
        if (bInputInRange) 
        {
            return Value;
        }
        
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << std::endl << "[TAP::ERROR] Input must be an integer between " << Min << " and " << Max << "." 
            << std::endl;
    }
}

void MEngine::StartAudioSyncThread() 
{
    if (bAudioSyncThreadRunning) return;
    
    bAudioSyncThreadRunning = true;
    
    AudioSyncThread = std::thread([this]()
    {
        while (bAudioSyncThreadRunning) 
        {
            SyncAudioState();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

void MEngine::StopAudioSyncThread() 
{
    bAudioSyncThreadRunning = false;
    if (AudioSyncThread.joinable()) 
    {
        AudioSyncThread.join();
    }
}
