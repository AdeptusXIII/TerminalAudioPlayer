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
#include <algorithm>
#include <cwctype>

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
    std::string VersionNumber = "0.17";
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
        ExecuteCommandPrompt(Command);
    }
    StopAudioSyncThread();
}

void MEngine::RunTUILoop()
{
    StartAudioSyncThread();
    ConsoleIO.InitTUI();

    std::wstring InputBuffer;
    std::size_t InputCursorIndex = 0;

    while (!bWantExit)
    {
        SyncAudioState();

        RenderTUIFrame(InputBuffer, InputCursorIndex);

        FTUIInputEvent InputEvent = ConsoleIO.ReadInputEvent();

        if (!InputEvent.bHasInput)
        {
            continue;
        }

        if (InputEvent.bIsSpecialKey)
        {
            if (InputEvent.KeyCode == KEY_ENTER)
            {
                HandleTUIEnter(InputBuffer, InputCursorIndex);
                RenderTUIFrame(InputBuffer, InputCursorIndex);
                continue;
            }
            
            if (HandleTUIControlKey(InputEvent.KeyCode, InputBuffer, InputCursorIndex))
            {
                RenderTUIFrame(InputBuffer, InputCursorIndex);
            }

            continue;
        }

        if (InputEvent.Character == L'\n')
        {
            HandleTUIEnter(InputBuffer, InputCursorIndex);
        }
        else
        {
            HandleTUICharacterInput(InputEvent.Character, InputBuffer, InputCursorIndex);
        }

        RenderTUIFrame(InputBuffer, InputCursorIndex);
    }

    ConsoleIO.ShutDownTUI();
    StopAudioSyncThread();
}

void MEngine::RenderTUIFrame(const std::wstring &InputBuffer, std::size_t InputCursorIndex)
{
    if (ConsoleIO.GetTerminalTooSmall()) return;
    ConsoleIO.RenderStatusWindow(BuildUISnapshotData(), BuildTrackInfoData());
    ConsoleIO.RenderInputWindow(InputBuffer, InputCursorIndex);
}

bool MEngine::HandleTUIControlKey(int KeyCode, std::wstring& InputBuffer, std::size_t& InputCursorIndex)
{
    if (KeyCode == KEY_RESIZE)
    {
        ConsoleIO.ResizeTUI();
        return true;
    }

    if (ConsoleIO.GetTerminalTooSmall())
    {
        return true;
    }

    if (KeyCode == KEY_UP)
    {
        InputBuffer = ConsoleIO.GetPreviousCommandFromHistory();
        InputCursorIndex = InputBuffer.size();
        return true;
    }

    if (KeyCode == KEY_DOWN)
    {
        InputBuffer = ConsoleIO.GetNextCommandFromHistory();
        InputCursorIndex = InputBuffer.size();
        return true;
    }
    
    if (KeyCode == KEY_LEFT)
    {
        if (InputCursorIndex > 0) InputCursorIndex--;
        return true;
    }
    
    if (KeyCode == KEY_RIGHT)
    {
        if (InputCursorIndex < InputBuffer.size()) InputCursorIndex++;
        return true;
    }
    
    if (KeyCode == KEY_BACKSPACE)
    {
        if (InputCursorIndex > 0)
        {
            InputBuffer.erase(InputCursorIndex - 1, 1);
            InputCursorIndex--;
        }
        return true;
    }

    if (KeyCode == KEY_SR)
    {
        ConsoleIO.ScrollOutputWindowVertical(-1);
        return true;
    }

    if (KeyCode == KEY_SF)
    {
        ConsoleIO.ScrollOutputWindowVertical(1);
        return true;
    }

    return false;
}

void MEngine::HandleTUIEnter(std::wstring& InputBuffer, std::size_t& InputCursorIndex)
{
    FCommand Command = ConsoleIO.ParseCommandLine(ConsoleIO.ConvertWideToUtf8(InputBuffer));

    InputBuffer.clear();
    InputCursorIndex = 0;
    
    ExecuteCommandPrompt(Command);
}

void MEngine::HandleTUICharacterInput(wchar_t Character, std::wstring& InputBuffer, std::size_t& InputCursorIndex)
{
    if (Character == 127 || Character == L'\b')
    {
        if (InputCursorIndex > 0)
        {
            InputBuffer.erase(InputCursorIndex - 1, 1);
            InputCursorIndex--;
        }
        return;
    }

    if (std::iswprint(Character))
    {
        InputBuffer.insert(InputCursorIndex, 1, Character);
        InputCursorIndex++;
    }
}

void MEngine::ExecuteCommandPrompt(const FCommand &InCommandPrompt)
{
    switch (InCommandPrompt.Type)
    {
        case ECommandType::Play:
        {
            if (!TryPlay())
            {
                ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_WARNING_MSG) + stp::msg::fnc::APP_FNC_TRY_PLAY_MSG);
            }
            break;
        }
        case ECommandType::Pause:
        {
            if (!TryPause())
            {
                ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_WARNING_MSG) + stp::msg::fnc::APP_FNC_TRY_PAUSE_MSG);
            }
            break;
        }
        case ECommandType::Stop:
        {
            if (!TryStop())
            {
                ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_WARNING_MSG) + stp::msg::fnc::APP_FNC_TRY_STOP_MSG);
            }
            break;
        }
        case ECommandType::Next:
        {
            if (!TryNext()) 
            {
                if (!TryPlayNextTrackOrFirst())
                {
                    ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_WARNING_MSG) + stp::msg::fnc::APP_FNC_TRY_NEXT_MSG);
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
                    ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_WARNING_MSG) + stp::msg::fnc::APP_FNC_TRY_PREV_MSG);
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
                ConsoleIO.PrintOutputMessage(stp::msg::APP_SHUTDOWN_MSG);
            }
            else 
            {
                ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_ERR_UNEXP_MSG);
                bWantExit = true;
            }
            break;
        }
        case ECommandType::Help:
        {
            if (InCommandPrompt.Args.empty())
            {
                ConsoleIO.PrintCommandHelp();
                return;
            }
            HandleHelpCommand(InCommandPrompt.Args[0]);
            break;
        }
        case ECommandType::Mode:
        {
            if (InCommandPrompt.Args.empty())
            {
                ConsoleIO.PrintOutputMessage(stp::msg::fnc::APP_FNC_LOW_ARG_MSG);
                return;
            }
            HandleModeCommand(InCommandPrompt.Args[0]);
            break;
        }
        case ECommandType::Select:
        {
            if (InCommandPrompt.Args.empty())
            {
                ConsoleIO.PrintOutputMessage(stp::msg::fnc::APP_FNC_LOW_ARG_MSG);
                return;
            }
            HandleSelectCommand(InCommandPrompt.Args[0]);
            break;
        }
        case ECommandType::Volume:
        {
            if (!InCommandPrompt.Args.empty())
            {
                if (HandleVolumeCommand(InCommandPrompt.Args[0]))
                {
                    ApplyCurrentVolume();
                }
            }
            else
            {
                ConsoleIO.PrintOutputMessage(stp::msg::fnc::APP_FNC_LOW_ARG_MSG);
            }
            break;
        }
        case ECommandType::Status:
        {
            ConsoleIO.PrintStatus(BuildUISnapshotData(), BuildTrackInfoData());
            break;
        }
        case ECommandType::Find:
        {
            if (!InCommandPrompt.Args.empty())
            {
                ConsoleIO.PrintFindResults(FindTracksByName(InCommandPrompt.Args[0]));
            }
            else
            {
                ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_LOW_ARG_MSG);
                break;
            }
            break;
        }
        case ECommandType::Unknown:
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_UNKNOWN_CMD_MSG);
            break;
    }
}

void MEngine::HandleModeCommand(const std::string &Arg)
{
    if (Arg.empty()) return;
    
    std::lock_guard<std::mutex> Lock(EngineMutex);
    {
        if (Arg == ct::PlaybackModeToString(EPlaybackMode::Once))
        {
            AudioBackend.SetLoop(false);
            PlaybackMode = EPlaybackMode::Once;
        }
        else if (Arg == ct::PlaybackModeToString(EPlaybackMode::LoopOne))
        {
            AudioBackend.SetLoop(true);
            PlaybackMode = EPlaybackMode::LoopOne;
        }
        else if (Arg == ct::PlaybackModeToString(EPlaybackMode::LoopAll))
        {
            AudioBackend.SetLoop(false);
            PlaybackMode = EPlaybackMode::LoopAll;
        }
        else if (Arg == ct::PlaybackModeToString(EPlaybackMode::LoopShuffle))
        {
            AudioBackend.SetLoop(false);
            PlaybackMode = EPlaybackMode::LoopShuffle;
        }
        else
        {
            ConsoleIO.PrintOutputMessage(stp::msg::APP_ERROR_MSG);
            ConsoleIO.PrintOutputMessage(stp::msg::fnc::APP_FNC_INVALID_ARG_MSG);
        }
    }
}

void MEngine::HandleHelpCommand(const std::string &Arg)
{
    if (Arg.empty()) return;
    
    if (Arg == ct::CommandTypeToString(ECommandType::Play))
    {
        ConsoleIO.PrintCommandHelpArg(ECommandType::Play);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Pause))
    {
        ConsoleIO.PrintCommandHelpArg(ECommandType::Pause);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Stop))
    {
        ConsoleIO.PrintCommandHelpArg(ECommandType::Stop);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Next))
    {
        ConsoleIO.PrintCommandHelpArg(ECommandType::Next);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Prev))
    {
        ConsoleIO.PrintCommandHelpArg(ECommandType::Prev);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::List))
    {
        ConsoleIO.PrintCommandHelpArg(ECommandType::List);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Refresh))
    {
        ConsoleIO.PrintCommandHelpArg(ECommandType::Refresh);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Exit))
    {
        ConsoleIO.PrintCommandHelpArg(ECommandType::Exit);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Help))
    {
        ConsoleIO.PrintCommandHelpArg(ECommandType::Help);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Mode))
    {
        ConsoleIO.PrintCommandHelpArg(ECommandType::Mode);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Select))
    {
        ConsoleIO.PrintCommandHelpArg(ECommandType::Select);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Volume))
    {
        ConsoleIO.PrintCommandHelpArg(ECommandType::Volume);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Status))
    {
        ConsoleIO.PrintCommandHelpArg(ECommandType::Status);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Find))
    {
        ConsoleIO.PrintCommandHelpArg(ECommandType::Find);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Unknown))
    {
        return;
    }
    else
    {
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_HELP_MSG) + stp::msg::fnc::APP_FNC_UNKNOWN_HELP_MSG);
    }
}

void MEngine::HandleSelectCommand(const std::string &ArgIndex)
{
    if (!CommandArgIsInt(ArgIndex))
    {
         ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_INVALID_ARG_MSG);
        return;
    }
    
    int TrackIndex = std::stoi(ArgIndex);
    
    std::lock_guard<std::mutex> Lock(EngineMutex);
    {
        bool bIndexInRange = (TrackIndex >= 0 && TrackIndex < TrackLibrary.GetTrackListSize());
            
        if (!bIndexInRange)
        {
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_INVALID_INDEX_MSG);
            return;
        }
            
        if (gp::bForcePlayAfterSwitch) 
        {
            std::filesystem::path Path = TrackLibrary.GetTrackPathByIndex(TrackIndex);
            if (Path.empty())
            {
                ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_PATH_EMPTY_MSG);
                return;
            }

            if (AudioBackend.PlayTrack(Path))
            {
                SetAudioPlayerState(EAudioPlayerState::Playing);
                if (!TrackLibrary.SetCurrentIndex(TrackIndex))
                {
                    ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_FAIL_SET_INDEX_MSG);
                    return;
                }
                return;
            }

            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_ERR_TRY_PLAY_FILE_MSG + Path.string());
        }
        else 
        {
            SetAudioPlayerState(EAudioPlayerState::Idle);
            AudioBackend.StopTrack();
        
            if (!TrackLibrary.SetCurrentIndex(TrackIndex)) 
            {
                ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_FAIL_SET_INDEX_MSG);
            }
        }
    }
}

bool MEngine::HandleVolumeCommand(const std::string& Arg)
{
    if (Arg.empty()) return false;
    
    try
    {
        size_t Pos;
        float Volume = std::stof(Arg, &Pos);
        
        if (Pos != Arg.size())
        {
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_INVALID_ARG_MSG);
            return false;
        }

        if (!std::isfinite(Volume))
        {
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_INVALID_ARG_MSG);
            return false;
        }

        if (Volume < gp::MIN_VOLUME || Volume > gp::MAX_VOLUME)
        {
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_INVALID_ARG_MSG);
            return false;
        }

        CurrentVolume = Volume;
        return true;
    }
    catch (...)
    {
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_INVALID_ARG_MSG);
    }
    
    return false;
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

std::vector<std::pair<int, std::string>> MEngine::FindTracksByName(const std::string &Arg)
{
    if (Arg.empty()) return {};

    std::vector<std::pair<int, std::string>> FoundTracks = {};
    std::string Query = Arg;
    std::transform(Query.begin(), Query.end(), Query.begin(), 
        [](unsigned char c) { return std::tolower(c); });
    
    for (int i = 0; i < TrackLibrary.GetTrackListSize(); i++)
    {
        std::string TrackName = TrackLibrary.GetTrackNameByIndex(i);
        std::string FormattedTrackName = TrackName;
        std::transform(FormattedTrackName.begin(), FormattedTrackName.end(), FormattedTrackName.begin(), 
            [](unsigned char c) { return std::tolower(c); });

        if (FormattedTrackName.find(Query) != std::string::npos)
        {
            FoundTracks.emplace_back(i, TrackName);
        }
    }
    
    return FoundTracks;
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
        
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_FAIL_PLAY_FIRST_MSG);
        return false;
    }
    
    if (AudioBackend.PlayTrack(Path)) 
    {
        TrackLibrary.SetNextIndex();
        SetAudioPlayerState(EAudioPlayerState::Playing);
        return true;
    }
    
    ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_ERR_TRY_PLAY_FILE_MSG + TrackLibrary.GetCurrentTrackName());
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
        
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_FAIL_PLAY_LAST_MSG);
        return false;
    }
    
    if (AudioBackend.PlayTrack(Path)) 
    {
        TrackLibrary.SetNextIndex();
        SetAudioPlayerState(EAudioPlayerState::Playing);
        return true;
    }
    
    ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_ERR_TRY_PLAY_FILE_MSG + TrackLibrary.GetCurrentTrackName());
    return false;
}

bool MEngine::TryPlayRandomTrack() 
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    if (TrackLibrary.IsEmpty()) 
    {
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_ERR_LIBRARY_EMPTY_MSG);
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
        
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_FAIL_PLAY_MSG);
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
    
    ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR_MSG) + stp::msg::fnc::APP_FNC_ERR_TRY_PLAY_FILE_MSG + TrackLibrary.GetCurrentTrackName());
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
        SnapshotData.Volume = CurrentVolume;
        SnapshotData.TrackIndex = TrackLibrary.GetCurrentIndex();
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

FTrackInfo MEngine::BuildTrackInfoData()
{
    FTrackInfo TrackInfo = {};
    
    TrackInfo.DurationSec = AudioBackend.GetTrackDurationSec();
    TrackInfo.PositionSec = AudioBackend.GetTrackCurrentOffsetSec();
    TrackInfo.RemainingSec = AudioBackend.GetTrackRemainingSec();
    
    return TrackInfo;
}
