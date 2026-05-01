//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".
#include "Engine.h"

#include "StringTemplates.h"
#include "Engine/CommandDataTable.h"
#include "Engine/GlobalParameters.h"
#include "UI//UISnapshot.h"

#include <iostream>
#include <filesystem>
#include <chrono>
#include <cmath>

MEngine::MEngine()
    
{
    AudioPlayerState = EAudioPlayerState::Idle;
    PlaybackMode = EPlaybackMode::LoopAll;
    bWantExit = false;
    CurrentVolume = 100;
    DefaultContentDir = std::getenv("HOME") + std::string("/Music/TAP_content");
    bAudioSyncThreadRunning = false;
}

void MEngine::Init()
{
    std::string VersionNumber = "0.11";
    std::cout << stp::msg::ENGINE_INIT_MSG << VersionNumber << std::endl;
    
    FileManager.Init();   
    AudioBackend.Init();
    
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

void MEngine::RunCommandLineLoop()
{
    StartAudioSyncThread();
    while (!bWantExit)
    {
        FCommand Command = ConsoleIO.ReadCommand();
        HandleCommandPromt(Command);
    }
    StopAudioSyncThread();
}

void MEngine::HandleCommandPromt(const FCommand &InCommandPrompt)
{
    switch (InCommandPrompt.Type)
    {
        case ECommandType::Play:
        {
            if (!TryPlay())
            {
                std::cout << "\n" << stp::msg::APP_WARNING_MSG << stp::msg::fnc::APP_FNC_TRY_PLAY_MSG <<"\n\n";
            }
            break;
        }
        case ECommandType::Pause:
        {
            if (!TryPause())
            {
                std::cout << "\n" << stp::msg::APP_WARNING_MSG << stp::msg::fnc::APP_FNC_TRY_PAUSE_MSG << "\n\n";
            }
            break;
        }
        case ECommandType::Stop:
        {
            if (!TryStop())
            {
                std::cout << "\n" << stp::msg::APP_WARNING_MSG << stp::msg::fnc::APP_FNC_TRY_STOP_MSG << "\n\n";
            }
            break;
        }
        case ECommandType::Next:
        {
            if (!TryNext()) 
            {
                if (!TryPlayNextTrackOrFirst())
                {
                    std::cout << "\n" << stp::msg::APP_WARNING_MSG << stp::msg::fnc::APP_FNC_TRY_NEXT_MSG << "\n\n";
                }
            }
            break;
        }
        case ECommandType::Prev:
        {
            if (!TryPrev()) 
            {
                if (!TryPlayPrevTrackOrLast())
                {
                    std::cout << "\n" << stp::msg::APP_WARNING_MSG << stp::msg::fnc::APP_FNC_TRY_PREV_MSG << "\n\n";
                }
            }
            break;
        }
        case ECommandType::List:
        {
            ConsoleIO.PrintTrackList(BuildUISnapshotData());
            break;
        }
        case ECommandType::Refresh:
        {
            RefreshTrackLibrary(DefaultContentDir);
            int TrackCount = 0;
            {
                std::lock_guard<std::mutex> Lock(EngineMutex);
                TrackCount = TrackLibrary.GetTrackListSize();
            }
            ConsoleIO.PrintTotalTracksNum(TrackCount);
            break;
        }
        case ECommandType::Exit:
        {
            if (TryExit()) 
            {
                std::cout << "\n" << stp::msg::APP_SHUTDOWN_MSG << "\n\n";
            }
            else 
            {
                std::cerr << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_ERR_UNEXP_MSG <<"\n\n";
                bWantExit = true;
            }
            break;
        }
        case ECommandType::Help:
        {
            if (InCommandPrompt.Args.empty())
            {
                ConsoleIO.PrintHelp();
                return;
            }
            HandleCommandArg_Help(InCommandPrompt.Args[0]);
            break;
        }
        case ECommandType::Mode:
        {
            if (InCommandPrompt.Args.empty())
            {
                std::cout << "\n" << stp::msg::fnc::APP_FNC_LOW_ARG_MSG << "\n\n" << std::endl;
                return;
            }
            HandleCommandArg_Mode(InCommandPrompt.Args[0]);
            break;
        }
        case ECommandType::Select:
        {
            if (InCommandPrompt.Args.empty())
            {
                std::cout << "\n" << stp::msg::fnc::APP_FNC_LOW_ARG_MSG << "\n\n" << std::endl;
                return;
            }
            CommandSelectTrackByIndex(InCommandPrompt.Args[0]);
            break;
        }
        case ECommandType::Volume:
        {
            if (!InCommandPrompt.Args.empty())
            {
                if (CommandParseVolume(InCommandPrompt.Args[0]))
                {
                    ApplyCurrentVolume();
                }
            }
            else
            {
                std::cout << "\n" << stp::msg::fnc::APP_FNC_LOW_ARG_MSG << "\n\n" << std::endl;
            }
            break;
        }
        case ECommandType::Status:
        {
            ConsoleIO.PrintStatus(AudioPlayerState, PlaybackMode, TrackLibrary.GetCurrentTrackName(), CurrentVolume);
            break;
        }
        case ECommandType::Unknown:
            std::cout << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_UNKNOWN_CMD_MSG << "\n\n";
            break;
    }
}

void MEngine::HandleCommandArg_Mode(const std::string &Arg)
{
    if (Arg.empty()) return;
    
    std::lock_guard<std::mutex> Lock(EngineMutex);
    {
        if (Arg == ct::PlaybackModeToArg(EPlaybackMode::Once))
        {
            AudioBackend.SetLoop(false);
            PlaybackMode = EPlaybackMode::Once;
        }
        else if (Arg == ct::PlaybackModeToArg(EPlaybackMode::LoopOne))
        {
            AudioBackend.SetLoop(true);
            PlaybackMode = EPlaybackMode::LoopOne;
        }
        else if (Arg == ct::PlaybackModeToArg(EPlaybackMode::LoopAll))
        {
            AudioBackend.SetLoop(false);
            PlaybackMode = EPlaybackMode::LoopAll;
        }
        else if (Arg == ct::PlaybackModeToArg(EPlaybackMode::LoopShuffle))
        {
            AudioBackend.SetLoop(false);
            PlaybackMode = EPlaybackMode::LoopShuffle;
        }
        else
        {
            std::cout << "\n" << stp::msg::APP_ERROR_MSG << "\n\n";
        }
    }
}

void MEngine::HandleCommandArg_Help(const std::string &Arg)
{
    if (Arg.empty()) return;
    
    if (Arg == ct::CommandTypeToString(ECommandType::Play))
    {
        ConsoleIO.PrintHelpCMD(ECommandType::Play);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Pause))
    {
        ConsoleIO.PrintHelpCMD(ECommandType::Pause);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Stop))
    {
        ConsoleIO.PrintHelpCMD(ECommandType::Stop);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Next))
    {
        ConsoleIO.PrintHelpCMD(ECommandType::Next);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Prev))
    {
        ConsoleIO.PrintHelpCMD(ECommandType::Prev);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::List))
    {
        ConsoleIO.PrintHelpCMD(ECommandType::List);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Refresh))
    {
        ConsoleIO.PrintHelpCMD(ECommandType::Refresh);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Exit))
    {
        ConsoleIO.PrintHelpCMD(ECommandType::Exit);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Help))
    {
        ConsoleIO.PrintHelpCMD(ECommandType::Help);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Mode))
    {
        ConsoleIO.PrintHelpCMD(ECommandType::Mode);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Select))
    {
        ConsoleIO.PrintHelpCMD(ECommandType::Select);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Volume))
    {
        ConsoleIO.PrintHelpCMD(ECommandType::Volume);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Status))
    {
        ConsoleIO.PrintHelpCMD(ECommandType::Status);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Unknown))
    {
        return;
    }
    else
    {
        std::cout << "\n" << stp::msg::APP_HELP_MSG << stp::msg::fnc::APP_FNC_UNKNOWN_HELP_MSG << "\n\n";
    }
}

void MEngine::CommandSelectTrackByIndex(const std::string &ArgIndex)
{
    if (!CommandArgIsInt(ArgIndex))
    {
         std::cout << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_INVALID_ARG_MSG << "\n\n";
        return;
    }
    
    int TrackIndex = std::stoi(ArgIndex);
    
    std::lock_guard<std::mutex> Lock(EngineMutex);
    {
        bool bIndexInRange = (TrackIndex >= 0 && TrackIndex < TrackLibrary.GetTrackListSize());
            
        if (!bIndexInRange)
        {
            std::cout << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_INVALID_INDEX_MSG << "\n\n";
            return;
        }
            
        if (gp::bForcePlayAfterSwitch) 
        {
            std::filesystem::path Path = TrackLibrary.GetTrackPathByIndex(TrackIndex);
            if (Path.empty())
            {
                std::cout << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_PATH_EMPTY_MSG <<"\n\n";
                return;
            }

            if (AudioBackend.PlayTrack(Path))
            {
                SetAudioPlayerState(EAudioPlayerState::Playing);
                if (!TrackLibrary.SetCurrentIndex(TrackIndex))
                {
                    std::cout << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_FAIL_SET_INDEX_MSG << "\n\n";
                    return;
                }
                return;
            }

            std::cout << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_ERR_TRY_PLAY_FILE_MSG << Path.string() <<
            std::endl << std::endl;
        }
        else 
        {
            SetAudioPlayerState(EAudioPlayerState::Idle);
            AudioBackend.StopTrack();
        
            if (!TrackLibrary.SetCurrentIndex(TrackIndex)) 
            {
                std::cout << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_FAIL_SET_INDEX_MSG << "\n\n";
            }
        }
    }
}

bool MEngine::CommandArgIsInt(const std::string &Arg)
{
    if (Arg.empty()) return false;

    try 
    {
        size_t Pos;
        std::stoul(Arg, &Pos);
        return Pos == Arg.size();
    }
    catch (...) 
    {
        return false;
    }
}

bool MEngine::CommandParseVolume(const std::string& Arg)
{
    if (Arg.empty()) return false;
    
    try
    {
        size_t Pos;
        float Volume = std::stof(Arg, &Pos);
        
        if (Pos != Arg.size())
        {
            std::cout << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_INVALID_ARG_MSG << "\n\n";
            return false;
        }

        if (!std::isfinite(Volume))
        {
            std::cout << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_INVALID_ARG_MSG << "\n\n";
            return false;
        }

        if (Volume < gp::MIN_VOLUME || Volume > gp::MAX_VOLUME)
        {
            return false;
        }

        CurrentVolume = Volume;
        return true;
    }
    catch (...)
    {
        std::cout << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_INVALID_ARG_MSG << "\n\n";
    }
    
    return false;
}

void MEngine::SetAudioPlayerState(EAudioPlayerState TargetAudioPlayerState)
{
    AudioPlayerState = TargetAudioPlayerState;
}

bool MEngine::CreateDefaultContentDir() 
{
    bool bCreated = false;
    
    if (FileManager.CreateDir(DefaultContentDir))
    {
        bCreated = true;
        if (gp::bPrintDebugInfo) 
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

void MEngine::ApplyCurrentVolume()
{
    AudioBackend.SetVolume(CurrentVolume);
}

bool MEngine::TryExit() 
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

bool MEngine::TryPrev() 
{   
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    std::filesystem::path Path = TrackLibrary.GetPrevTrackPath();
    if (gp::bForcePlayAfterSwitch && !TrackLibrary.IsEmpty() && !Path.empty())
    {
        if (AudioBackend.PlayTrack(Path)) 
        {
            SetAudioPlayerState(EAudioPlayerState::Playing);
            TrackLibrary.SetPrevIndex();
            return true;
        }
        else 
        {
            if (gp::bPrintDebugInfo)
            {
                std::cerr << "[ENGINE] Error while trying to play file: " << Path.string() << std::endl ;
            }
            return false;
        }
    }
    
    if (!gp::bForcePlayAfterSwitch && !TrackLibrary.IsEmpty() && !Path.empty()) 
    {
        SetAudioPlayerState(EAudioPlayerState::Idle);
        AudioBackend.StopTrack();
        TrackLibrary.SetPrevIndex();
        return true;
    }
    
    return false;
}

bool MEngine::TryPlay() 
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
            if (gp::bPrintDebugInfo)
            {
                std::cerr << "[ENGINE] Error while trying to play file: " << Path.string() << std::endl ;
            }
            return false;
        }
    }
    return false;
}

bool MEngine::TryPause() 
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

bool MEngine::TryStop() 
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

bool MEngine::TryNext() 
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    std::filesystem::path Path = TrackLibrary.GetNextTrackPath();
    if (gp::bForcePlayAfterSwitch && !TrackLibrary.IsEmpty() && !Path.empty())
    {
        if (AudioBackend.PlayTrack(Path)) 
        {
            SetAudioPlayerState(EAudioPlayerState::Playing);
            TrackLibrary.SetNextIndex();
            return true;
        }
        else 
        {
            if (gp::bPrintDebugInfo)
            {
                std::cerr << "[ENGINE] Error while trying to play file: " << Path.string() << std::endl ;
            }
            return false;
        }
    }
    
    if (!gp::bForcePlayAfterSwitch && !TrackLibrary.IsEmpty() && !Path.empty()) 
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
        
        std::cout << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_FAIL_PLAY_FIRST_MSG << std::endl;
        return false;
    }
    
    if (AudioBackend.PlayTrack(Path)) 
    {
        TrackLibrary.SetNextIndex();
        SetAudioPlayerState(EAudioPlayerState::Playing);
        return true;
    }
    
    std::cout << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_ERR_TRY_PLAY_FILE_MSG << TrackLibrary.GetCurrentTrackName() << std::endl;
    return false;
}

bool MEngine::TryPlayPrevTrackOrLast()
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    std::filesystem::path Path = TrackLibrary.GetPrevTrackPath();
    if (Path.empty() && !TrackLibrary.IsEmpty()) 
    {
        std::filesystem::path LastTrackPath = TrackLibrary.GetTrackPathByIndex(TrackLibrary.GetTrackListSize() - 1);
        if (!LastTrackPath.empty() && AudioBackend.PlayTrack(LastTrackPath))
        {
            TrackLibrary.SetCurrentIndex(TrackLibrary.GetTrackListSize() - 1);
            SetAudioPlayerState(EAudioPlayerState::Playing);
            return true;
        }
        
        std::cout << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_FAIL_PLAY_LAST_MSG << std::endl;
        return false;
    }
    
    if (AudioBackend.PlayTrack(Path)) 
    {
        TrackLibrary.SetNextIndex();
        SetAudioPlayerState(EAudioPlayerState::Playing);
        return true;
    }
    
    std::cout << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_ERR_TRY_PLAY_FILE_MSG << TrackLibrary.GetCurrentTrackName() << std::endl;
    return false;
}

bool MEngine::TryPlayRandomTrack() 
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    if (TrackLibrary.IsEmpty()) 
    {
        std::cout << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_ERR_LIBRARY_EMPTY_MSG << std::endl;
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
        
        std::cout << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_FAIL_PLAY_MSG << std::endl;
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
    
    std::cout << "\n" << stp::msg::APP_ERROR_MSG << stp::msg::fnc::APP_FNC_ERR_TRY_PLAY_FILE_MSG << TrackLibrary.GetCurrentTrackName() << std::endl;
    return false;
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

FUISnapshotData MEngine::BuildUISnapshotData() 
{
    FUISnapshotData SnapshotData;
    {
        std::lock_guard<std::mutex> Lock(EngineMutex);
        
        SnapshotData.AudioPlayerState = AudioPlayerState;
        SnapshotData.PlaybackMode = PlaybackMode;
        SnapshotData.TrackCount = TrackLibrary.GetTrackListSize();
        SnapshotData.CurrentTrackIndex = TrackLibrary.GetCurrentIndex();
        SnapshotData.CurrentTrackName = TrackLibrary.GetCurrentTrackName();
        
        std::vector<std::string> TrackNames;
        
        for (int i = 0; i < SnapshotData.TrackCount; i++)
        {
            TrackNames.push_back(TrackLibrary.GetTrackNameByIndex(i));
        }
        
        SnapshotData.TrackList = TrackNames;
    }
    
    return SnapshotData;
}
