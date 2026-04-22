//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".
#include "Engine.h"

#include <iostream>
#include <limits>
#include <filesystem>

MEngine::MEngine()
    
{
    AudioPlayerState = EAudioPlayerState::Idle;
    CurrentMenuSection = EMenuSection::MainMenu;
    bForceStartAfterSwitch = true;
    bWantExit = false;
    bWantBackFromLibrary = false;
    bPrintDebugInfo = false;
    DefaultContentDir = std::getenv("HOME") + std::string("/Music/TAP_content");
}

void MEngine::Init()
{
    std::string VersionNumber = "0.9";
    std::cout << "TAP - Terminal Audio Player V." << VersionNumber << std::endl;
    
    FileManager.Init(bPrintDebugInfo);   
    AudioBackend.Init(bPrintDebugInfo);
    
    CreateDefaultContentDir();
    WriteTrackListToTrackLibrary(DefaultContentDir);
}

void MEngine::SyncAudioState() 
{
    if (AudioPlayerState == EAudioPlayerState::Playing && AudioBackend.IsStopped()) 
    {
        SetAudioPlayerState(EAudioPlayerState::Idle);
    }
}

void MEngine::RunMainLoop() 
{
    while (bWantExit == false)
    {
        SyncAudioState();
        
        EMainMenuOption SelectedOption = EMainMenuOption::None;
        
        SelectedOption = ShowMainMenuAndGetOption();

        HandleMainMenuOption(SelectedOption);
    }
}

void MEngine::PrintMenuOptAndState()
{
    PrintAudioPlayerState();
    PrintCurrentTrack();
    PrintMenuSection();
    
    std::cout << "Choose option: " << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "[Opt: 1] - Prev" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "[Opt: 2] - Play" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "[Opt: 3] - Pause" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "[Opt: 4] - Stop" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "[Opt: 5] - Next" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "[Opt: 6] - Library" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_L << "[Opt: 0] - Exit" << std::endl;
}

void MEngine::PrintLibraryOptAndState() 
{
    PrintAudioPlayerState();
    PrintCurrentTrack();
    PrintMenuSection();
    
    std::cout << "Choose option: " << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "[Opt: 1] - List" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "[Opt: 2] - Select track by index" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "[Opt: 3] - Refresh" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_L << "[Opt: 0] - Back" << std::endl;
}

void MEngine::PrintAudioPlayerState()
{
    switch (AudioPlayerState)
    {
        case EAudioPlayerState::Idle:
            if (bPrintDebugInfo) 
            {
                std::cout << std::endl << "[AudioPlayerState]::Idle" << std::endl;
            }
            else 
            {
                std::cout << std::endl << "[TAP::STATE::IDLE]" << std::endl;
            }
            break;

        case EAudioPlayerState::Playing:
            if (bPrintDebugInfo) 
            {
                std::cout << std::endl << "[AudioPlayerState]::Playing" << std::endl;
            }
            else 
            {
                std::cout << std::endl <<  "[TAP::STATE::PLAYING]" << std::endl;
            }
            break;

        case EAudioPlayerState::Paused:
            if (bPrintDebugInfo) 
            {
                std::cout << std::endl << "[AudioPlayerState]::Paused" << std::endl;
            }
            else 
            {
                std::cout << std::endl << "[TAP::STATE::PAUSED]" << std::endl;
            }
            break;
    }
}

void MEngine::PrintTrackList() 
{
    PrintMenuSection();
    std::cout << "Total tracks found: " << TrackLibrary.GetTrackListSize() << std::endl;
    std::string CurrentTrack = {};
    std::string SubCatTorL = {};
    
    if (!TrackLibrary.IsEmpty())
    {
        for (int i = 0; i < TrackLibrary.GetTrackListSize(); i++)
        {
            if (i == TrackLibrary.GetCurrentIndex())
                CurrentTrack = " <- current/selected";
            else
                CurrentTrack = {};
            if ( i == TrackLibrary.GetTrackListSize() - 1)
                SubCatTorL = SUBCAT_SEP_L;
            else
                SubCatTorL = SUBCAT_SEP_T;
            std::cout << SUBCAT_SEP_TAB + SubCatTorL << "Index(" << i << ")" << "[" << TrackLibrary.GetTrackNameByIndex(i) << "] " << CurrentTrack 
                << std::endl;
        }
    }
}

void MEngine::PrintCurrentTrack() 
{
    std::string CurrentTrackName = TrackLibrary.GetCurrentTrackName();
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
    }
    
    std::cout << "[TAP::" << MenuSection << "] ";
}

EMainMenuOption MEngine::ShowMainMenuAndGetOption()
{
    PrintMenuOptAndState();
    
    int Option = ReadIntInRange(0, MAX_MAIN_MENU_OPTION_COUNT, "[TAP::MENU] Enter opt: ");
    
    return static_cast<EMainMenuOption>(Option);
}

void MEngine::SelectTrackByIndex() 
{
    if (TrackLibrary.IsEmpty()) return;
    
    int TrackLibrarySize = TrackLibrary.GetTrackListSize() - 1;
    int TrackIndex = ReadIntInRange(0, TrackLibrarySize, "Enter index: ");
    
    if (bForceStartAfterSwitch) 
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

ELibraryMenuOption MEngine::ShowLibraryMenuAndGetOption() 
{
    PrintLibraryOptAndState();
    
    int Option = ReadIntInRange(0, MAX_LIBRARY_MENU_OPTION_COUNT, "[TAP::LIBRARY] Enter opt: ");
    
    return static_cast<ELibraryMenuOption>(Option);
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
            if (TryPrevOption()) 
            {
                if (bPrintDebugInfo) 
                {
                    std::cout << std::endl << "[ENGINE] Previous Track Selected: " << TrackLibrary.GetCurrentTrackName() 
                        << std::endl;
                }
            }
            else 
            {
                std::cout << std::endl << "[TAP::WARNING] No previous track." << std::endl;
            }
            break;

        case EMainMenuOption::Play:
            if (TryPlayOption())
            {
                if (bPrintDebugInfo) 
                {
                    std::cout << std::endl << "[ENGINE] Track Playing: " << TrackLibrary.GetCurrentTrackName() 
                        << std::endl;
                }
            }
            else
            {
                std::cout << std::endl << "[TAP::WARNING] Track is already playing or not selected." << std::endl;
            }
            break;

        case EMainMenuOption::Pause:
            if (TryPauseOption())
            {
                if (bPrintDebugInfo) 
                {
                    std::cout << std::endl << "[ENGINE] Track Paused" << std::endl;
                }
            }
            else
            {
                std::cout << std::endl << "[TAP::WARNING] Track is already paused or not playing." << std::endl;
            }
            break;

        case EMainMenuOption::Stop:
            if (TryStopOption())
            {
                if (bPrintDebugInfo) 
                {
                    std::cout << std::endl << "[ENGINE] Track Stopped." << std::endl;
                }
            }
            else
            {
                std::cout << std::endl << "[TAP::WARNING] Track is already stopped." << std::endl;
            }
            break;

        case EMainMenuOption::Next:
            if (TryNextOption()) 
            {
                if (bPrintDebugInfo) 
                {
                    std::cout << std::endl << "[ENGINE] Next Track Selected: " << TrackLibrary.GetCurrentTrackName() 
                        << std::endl;
                }
            }
            else 
            {
                std::cout << std::endl << "[TAP::WARNING] No next track." << std::endl;
            }
            break;
        
        case EMainMenuOption::Library:
            OpenLibrary();
            break;
            
        case EMainMenuOption::Exit:
            if (TryExitOption()) 
            {
                std::cout << std::endl << "[TAP::SHUTDOWN] Exit." << std::endl;
                SetAudioPlayerState(EAudioPlayerState::Idle);
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
    switch (InOption) 
    {
        case ELibraryMenuOption::List:
            PrintTrackList();
            break;
            
        case ELibraryMenuOption::SelectByIndex:
            SelectTrackByIndex();
            break;
            
        case ELibraryMenuOption::Refresh:
            RefreshTrackLibrary(DefaultContentDir);
            break;
            
        case ELibraryMenuOption::Back:
            bWantBackFromLibrary = true;
            break;
    }
}

bool MEngine::TryExitOption() 
{
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
    std::filesystem::path Path = TrackLibrary.GetPrevTrackPath();
    if (bForceStartAfterSwitch && !TrackLibrary.IsEmpty() && !Path.empty())
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
    
    if (!bForceStartAfterSwitch && !TrackLibrary.IsEmpty() && !Path.empty()) 
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
    std::filesystem::path Path = TrackLibrary.GetNextTrackPath();
    if (bForceStartAfterSwitch && !TrackLibrary.IsEmpty() && !Path.empty())
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
    
    if (!bForceStartAfterSwitch && !TrackLibrary.IsEmpty() && !Path.empty()) 
    {
        SetAudioPlayerState(EAudioPlayerState::Idle);
        AudioBackend.StopTrack();
        TrackLibrary.SetNextIndex();
        return true;
    }
    
    return false;
}

void MEngine::OpenLibrary() 
{
    CurrentMenuSection = EMenuSection::LibraryMenu;
    while (!bWantBackFromLibrary) 
    {
        ELibraryMenuOption Option = ELibraryMenuOption::None;
    
        Option = ShowLibraryMenuAndGetOption();
    
        HandleLibraryMenuOption(Option);
    }
    bWantBackFromLibrary = false;
    CurrentMenuSection = EMenuSection::MainMenu;
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
    if (AudioPlayerState == EAudioPlayerState::Playing || AudioPlayerState == EAudioPlayerState::Paused) 
    {
        AudioBackend.StopTrack();
        SetAudioPlayerState(EAudioPlayerState::Idle);
    }
    
    TrackLibrary.Clear();
    WriteTrackListToTrackLibrary(InPath);
}
