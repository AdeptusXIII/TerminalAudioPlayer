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

namespace
{
    std::string ScanResultStatusToMessage(EScanResultStatus Status)
    {
        switch (Status)
        {
            case EScanResultStatus::EmptyPath:
                return stp::msg::fnc::APP_FNC_PATH_EMPTY;
            case EScanResultStatus::PathDoesNotExist:
                return stp::msg::fnc::APP_FNC_PATH_NOT_EXISTS;
            case EScanResultStatus::PathIsNotDirectory:
                return stp::msg::fnc::APP_FNC_PATH_NOT_DIR;
            case EScanResultStatus::UnexpectedError:
                return stp::msg::fnc::APP_FNC_ERR_UNEXP;
            case EScanResultStatus::Success:
                return {};
        }

        return stp::msg::fnc::APP_FNC_ERR_UNEXP;
    }
}

MEngine::MEngine()
    
{
    AudioPlayerState = EAudioPlayerState::Idle;
    PlaybackMode = EPlaybackMode::LoopAll;
    bWantExit = false;
    bPausedBySystemSleep = false;
    CurrentVolume = 100;
    
    const char* HomeEnv = std::getenv("HOME");
    if (HomeEnv != nullptr)
    {
        DefaultContentDir = std::filesystem::path(HomeEnv) / "Music/TAP_content";
    }
    else
    {
        DefaultContentDir = std::filesystem::current_path() / "TAP_content";
    }

    bAudioSyncThreadRunning = false;
    PowerEventWatcher.SetPrepareForSleepCallback([this](bool bPreparingForSleep)
    {
        if (bPreparingForSleep)
        {
            HandleSystemSleepStart();
        }
        else
        {
            HandleSystemResume();
        }
    });
}

void MEngine::Init()
{
    std::string VersionNumber = "0.18.1";
    std::cout << stp::msg::ENGINE_INIT << VersionNumber << std::endl;
    
    TrackLibraryStorage.EnsureStorageFileExists();
    TrackLibrary.SetAllTracks(TrackLibraryStorage.LoadTrackPaths());
    
    CreateDefaultContentDir();
    
    if (TrackLibrary.GetTrackListSize() == 0)
    {
        WriteTrackListToTrackLibrary(DefaultContentDir);
        SaveAllTrackList();
    }
}

void MEngine::SyncAudioState() 
{
    EPlaybackMode CurrentPlaybackMode = EPlaybackMode::Once;
    bool bShouldReact = false;
    bool bNowPlayingFromActiveList = true;
    
    {
        std::lock_guard<std::mutex> Lock(EngineMutex);
        
        if (AudioPlayerState == EAudioPlayerState::Playing && AudioBackend.IsStopped()) 
        {
            bShouldReact = true;
            CurrentPlaybackMode = PlaybackMode;
            bNowPlayingFromActiveList = NowPlayingInfo.bValid
                && NowPlayingInfo.SourceListName == TrackLibrary.GetActiveTrackListName();
        }
    }
    
    if (!bShouldReact) return;
    
    if (TryPlayPendingTrack())
    {
        return;
    }
    
    if (CurrentPlaybackMode == EPlaybackMode::LoopAll) 
    {
        if (bNowPlayingFromActiveList && TryPlayNextTrackOrFirst()) 
        {
            return;
        }
        
        if (!bNowPlayingFromActiveList && TryPlayCurrentTrack())
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
    ClearNowPlaying();
}

void MEngine::RunCommandLineLoop()
{
    StartAudioSyncThread();
    StartPowerEventWatcher();
    while (!bWantExit)
    {
        FCommand Command = ConsoleIO.ReadCommand();
        ExecuteCommandPrompt(Command);
    }
    StopPowerEventWatcher();
    StopAudioSyncThread();
}

void MEngine::RunTUILoop()
{
    StartAudioSyncThread();
    StartPowerEventWatcher();
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
            
            if (HandleTUIControlKey(InputEvent, InputBuffer, InputCursorIndex))
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
    StopPowerEventWatcher();
    StopAudioSyncThread();
}

void MEngine::RenderTUIFrame(const std::wstring &InputBuffer, std::size_t InputCursorIndex)
{
    if (ConsoleIO.GetTerminalTooSmall()) return;
    ConsoleIO.RenderStatusWindow(BuildUISnapshotData(), BuildTrackInfoData());
    ConsoleIO.RenderInputWindow(InputBuffer, InputCursorIndex);
}

bool MEngine::HandleTUIControlKey(const FTUIInputEvent& InputEvent, std::wstring& InputBuffer, std::size_t& InputCursorIndex)
{
    const int KeyCode = InputEvent.KeyCode;
    
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
    
    if (KeyCode == KEY_MOUSE)
    {
        if (InputEvent.bIsMouseWheelUp)
        {
            ConsoleIO.ScrollOutputWindowVertical(-1);
            return true;
        }
        
        if (InputEvent.bIsMouseWheelDown)
        {
            ConsoleIO.ScrollOutputWindowVertical(1);
            return true;
        }
        
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
            HandlePlayCommand(InCommandPrompt.Args);
            break;
        }
        case ECommandType::Pause:
        {
            if (!TryPause())
            {
                ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_WARNING) + stp::msg::fnc::APP_FNC_TRY_PAUSE);
            }
            break;
        }
        case ECommandType::Stop:
        {
            if (!TryStop())
            {
                ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_WARNING) + stp::msg::fnc::APP_FNC_TRY_STOP);
            }
            break;
        }
        case ECommandType::Next:
        {
            HandleNextCommand(InCommandPrompt.Args);
            break;
        }
        case ECommandType::Prev:
        {
            HandlePrevCommand(InCommandPrompt.Args);
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
            SaveAllTrackList();
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
                ConsoleIO.PrintOutputMessage(stp::msg::APP_SHUTDOWN);
            }
            else 
            {
                ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR) + stp::msg::fnc::APP_FNC_ERR_UNEXP);
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
                ConsoleIO.PrintOutputMessage(stp::msg::fnc::APP_FNC_LOW_ARG);
                return;
            }
            HandleModeCommand(InCommandPrompt.Args[0]);
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
                ConsoleIO.PrintOutputMessage(stp::msg::fnc::APP_FNC_LOW_ARG);
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
                ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR) + stp::msg::fnc::APP_FNC_LOW_ARG);
                break;
            }
            break;
        }
        case ECommandType::Scan:
        {
            if (InCommandPrompt.Args.empty())
            {
                ConsoleIO.PrintOutputMessage(stp::msg::fnc::APP_FNC_LOW_ARG);
                return;
            }
            
            HandleScanCommand(InCommandPrompt.Args);
            break;
        }
        case ECommandType::Playlist:
        {
            HandlePlaylistCommand(InCommandPrompt.Args);
            break;
        }
        case ECommandType::Unknown:
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR) + stp::msg::fnc::APP_FNC_UNKNOWN_CMD);
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
            ConsoleIO.PrintOutputMessage(stp::msg::APP_ERROR);
            ConsoleIO.PrintOutputMessage(stp::msg::fnc::APP_FNC_INVALID_ARG);
        }
    }
}

void MEngine::HandleScanCommand(const std::vector<std::string> &Args)
{
    if (Args.empty())
    {
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_LIST) + stp::msg::fnc::APP_FNC_LOW_ARG);
        return;
    }

    bool bRecursiveScan = false;
    std::string Path = {};
    if (Args[0] == ct::CommandFlagToString(ECommandFlag::Recursive))
    {
        if (Args.size() < 2)
        {
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_LIST) + stp::msg::fnc::APP_FNC_LOW_ARG);
            return;
        }

        bRecursiveScan = true;
        Path = Args[1];
    }
    else
    {
        Path = Args[0];
    }
    
    const std::filesystem::path ScanPath = ExpandUserPath(Path);
    FScanResult ScanResult;
    
    if (bRecursiveScan)
    {
        ScanResult = AudioFileScanner.ScanPathRecursive(ScanPath);
    }
    else
    {
        ScanResult = AudioFileScanner.ScanPath(ScanPath);
    }
    
    if (ScanResult.Status != EScanResultStatus::Success)
    {
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_LIST)
            + ScanResultStatusToMessage(ScanResult.Status));
        return;
    }
    
    {
        std::lock_guard<std::mutex> Lock(EngineMutex);
        TrackLibrary.SetBufferTracks(ScanResult.Tracks);
        TrackLibrary.SetActiveTrackListByName("buffer");
    }
    
    ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_LIST) + "Scanned path: " + ScanPath.string());
    ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_LIST) + "Found " + std::to_string(ScanResult.Tracks.size())
        + " tracks. Active list switched to buffer.");
}

void MEngine::HandlePlaylistCommand(const std::vector<std::string> &Args)
{
    if (Args.empty())
    {
        PrintTrackListSummaries();
        return;
    }
    
    const std::string& Target = Args[0];
    
    if (Target == "list")
    {
        PrintTrackListSummaries();
        return;
    }
    
    if (Target == "current")
    {
        std::lock_guard<std::mutex> Lock(EngineMutex);
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_LIST) + "Active list: " + TrackLibrary.GetActiveTrackListName());
        return;
    }
    
    if (Target == "use")
    {
        if (Args.size() < 2)
        {
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_LIST) + "Usage: pl use <buffer|all|favorite|custom>");
            return;
        }
        
        std::lock_guard<std::mutex> Lock(EngineMutex);
        
        const bool bChanged = CommandArgIsInt(Args[1])
            ? TrackLibrary.SetActiveTrackListByIndex(std::stoi(Args[1]))
            : TrackLibrary.SetActiveTrackListByName(Args[1]);
        
        ConsoleIO.PrintOutputMessage(bChanged
            ? std::string(stp::msg::APP_LIST) + "Active list: " + TrackLibrary.GetActiveTrackListName()
            : std::string(stp::msg::APP_LIST) + "Unknown list.");
        return;
    }
    
    if (Target == "create")
    {
        if (Args.size() < 2)
        {
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_LIST) + "Usage: pl create <name>");
            return;
        }
        
        std::lock_guard<std::mutex> Lock(EngineMutex);
        ConsoleIO.PrintOutputMessage(TrackLibrary.CreateCustomTrackList(Args[1])
            ? std::string(stp::msg::APP_LIST) + "Created playlist: " + Args[1]
            : std::string(stp::msg::APP_LIST) + "Failed to create playlist.");
        return;
    }
    
    if (Target == "delete")
    {
        if (Args.size() < 2)
        {
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_LIST) + "Usage: pl delete <name>");
            return;
        }
        
        std::lock_guard<std::mutex> Lock(EngineMutex);
        ConsoleIO.PrintOutputMessage(TrackLibrary.DeleteCustomTrackList(Args[1])
            ? std::string(stp::msg::APP_LIST) + "Deleted playlist: " + Args[1]
            : std::string(stp::msg::APP_LIST) + "Failed to delete playlist.");
        return;
    }
    
    if (Target == "all" && Args.size() >= 4 && Args[1] == "add" && Args[3] == "from")
    {
        if (Args.size() < 5 || Args[4] != "buffer")
        {
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_LIST) + "Usage: pl all add <index|all> from buffer");
            return;
        }
        
        std::lock_guard<std::mutex> Lock(EngineMutex);
        
        const bool bAddAll = Args[2] == "all";
        const int Index = bAddAll ? -1 : (CommandArgIsInt(Args[2]) ? std::stoi(Args[2]) : -1);
        const bool bChanged = TrackLibrary.ImportFromBuffer(bAddAll, Index);
        
        if (bChanged)
        {
            SaveAllTrackList();
        }
        
        ConsoleIO.PrintOutputMessage(bChanged
            ? std::string(stp::msg::APP_LIST) + "Imported to all."
            : std::string(stp::msg::APP_LIST) + "Nothing imported.");
        return;
    }
    
    if ((Target == "favorite" || Target == "fav") && Args.size() >= 5 && Args[1] == "add" && Args[3] == "from")
    {
        const int Index = CommandArgIsInt(Args[2]) ? std::stoi(Args[2]) : -1;
        
        std::lock_guard<std::mutex> Lock(EngineMutex);
        const bool bChanged = TrackLibrary.AddToFavoritesFromList(Args[4], Index);
        
        if (bChanged)
        {
            SaveAllTrackList();
        }
        
        ConsoleIO.PrintOutputMessage(bChanged
            ? std::string(stp::msg::APP_LIST) + "Added to favorite."
            : std::string(stp::msg::APP_LIST) + "Failed to add favorite.");
        return;
    }

    if ((Target == "favorite" || Target == "fav") && Args.size() >= 3 && Args[1] == "add")
    {
        const int Index = CommandArgIsInt(Args[2]) ? std::stoi(Args[2]) : -1;
        
        std::lock_guard<std::mutex> Lock(EngineMutex);
        const std::string SourceListName = TrackLibrary.GetActiveTrackListName();
        const bool bChanged = TrackLibrary.AddToFavoritesFromList(SourceListName, Index);
        
        if (bChanged)
        {
            SaveAllTrackList();
        }
        
        ConsoleIO.PrintOutputMessage(bChanged
            ? std::string(stp::msg::APP_LIST) + "Added to favorite from active list: " + SourceListName
            : std::string(stp::msg::APP_LIST) + "Failed to add favorite.");
        return;
    }
    
    if ((Target == "favorite" || Target == "fav") && Args.size() >= 3 && Args[1] == "remove")
    {
        const int Index = CommandArgIsInt(Args[2]) ? std::stoi(Args[2]) : -1;
        
        std::lock_guard<std::mutex> Lock(EngineMutex);
        ConsoleIO.PrintOutputMessage(TrackLibrary.RemoveFromTrackList("favorite", Index)
            ? std::string(stp::msg::APP_LIST) + "Removed from favorite."
            : std::string(stp::msg::APP_LIST) + "Failed to remove from favorite.");
        return;
    }
    
    if (Args.size() == 2 && Args[1] == "list")
    {
        {
            std::lock_guard<std::mutex> Lock(EngineMutex);
            if (!TrackLibrary.SetActiveTrackListByName(Target))
            {
                ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_LIST) + "Unknown list.");
                return;
            }
        }
        
        ConsoleIO.PrintTrackList(BuildUISnapshotData());
        return;
    }
    
    if (Args.size() >= 5 && Args[1] == "add" && Args[3] == "from")
    {
        const bool bAddAll = Args[2] == "all";
        const int Index = bAddAll ? -1 : (CommandArgIsInt(Args[2]) ? std::stoi(Args[2]) : -1);
        
        std::lock_guard<std::mutex> Lock(EngineMutex);
        const bool bChanged = TrackLibrary.AddToTrackListFromList(Target, Args[4], bAddAll, Index);
        
        if (bChanged)
        {
            SaveAllTrackList();
        }
        
        ConsoleIO.PrintOutputMessage(bChanged
            ? std::string(stp::msg::APP_LIST) + "Track added."
            : std::string(stp::msg::APP_LIST) + "Failed to add track.");
        return;
    }

    if (Args.size() >= 3 && Args[1] == "add")
    {
        const bool bAddAll = Args[2] == "all";
        const int Index = bAddAll ? -1 : (CommandArgIsInt(Args[2]) ? std::stoi(Args[2]) : -1);
        
        std::lock_guard<std::mutex> Lock(EngineMutex);
        const std::string SourceListName = TrackLibrary.GetActiveTrackListName();
        const bool bChanged = TrackLibrary.AddToTrackListFromList(Target, SourceListName, bAddAll, Index);
        
        if (bChanged)
        {
            SaveAllTrackList();
        }
        
        ConsoleIO.PrintOutputMessage(bChanged
            ? std::string(stp::msg::APP_LIST) + "Track added from active list: " + SourceListName
            : std::string(stp::msg::APP_LIST) + "Failed to add track.");
        return;
    }
    
    if (Args.size() >= 3 && Args[1] == "remove")
    {
        const int Index = CommandArgIsInt(Args[2]) ? std::stoi(Args[2]) : -1;
        
        std::lock_guard<std::mutex> Lock(EngineMutex);
        const bool bChanged = TrackLibrary.RemoveFromTrackList(Target, Index);
        
        if (bChanged && Target == "all")
        {
            SaveAllTrackList();
        }
        
        ConsoleIO.PrintOutputMessage(bChanged
            ? std::string(stp::msg::APP_LIST) + "Track removed."
            : std::string(stp::msg::APP_LIST) + "Failed to remove track.");
        return;
    }
    
    ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_LIST) + "Unknown playlist command.");
}

bool MEngine::SaveAllTrackList()
{
    return TrackLibraryStorage.SaveTrackPaths(TrackLibrary.GetAllTrackPaths());
}

void MEngine::PrintTrackListSummaries()
{
    std::vector<std::string> Lines;
    Lines.emplace_back(std::string(stp::msg::APP_LIST) + "Track lists:");
    
    {
        std::lock_guard<std::mutex> Lock(EngineMutex);
        
        for (const std::string& Summary : TrackLibrary.GetTrackListSummaries())
        {
            Lines.emplace_back("  " + Summary);
        }
        
        Lines.emplace_back("Active: " + TrackLibrary.GetActiveTrackListName());
    }
    
    ConsoleIO.PrintOutputLines(Lines);
}

std::filesystem::path MEngine::ExpandUserPath(const std::string &Path) const
{
    if (Path == "~")
    {
        const char* HomeEnv = std::getenv("HOME");
        return HomeEnv != nullptr ? std::filesystem::path(HomeEnv) : std::filesystem::path(Path);
    }
    
    if (Path.size() > 2 && Path[0] == '~' && Path[1] == '/')
    {
        const char* HomeEnv = std::getenv("HOME");
        
        if (HomeEnv != nullptr)
        {
            return std::filesystem::path(HomeEnv) / Path.substr(2);
        }
    }
    
    return std::filesystem::path(Path);
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
    else if (Arg == ct::CommandTypeToString(ECommandType::Scan))
    {
        ConsoleIO.PrintCommandHelpArg(ECommandType::Scan);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Playlist))
    {
        ConsoleIO.PrintCommandHelpArg(ECommandType::Playlist);
    }
    else if (Arg == ct::CommandTypeToString(ECommandType::Unknown))
    {
        return;
    }
    else
    {
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_HELP) + stp::msg::fnc::APP_FNC_UNKNOWN_HELP);
    }
}

void MEngine::HandlePlayCommand(const std::vector<std::string> &Args)
{
    if (Args.empty())
    {
        if (!TryPlay())
        {
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_WARNING) + stp::msg::fnc::APP_FNC_TRY_PLAY);
        }
        return;
    }
    
    if (Args.size() >= 2 && Args[0] == ct::CommandFlagToString(ECommandFlag::Deferred))
    {
        if (!CommandArgIsInt(Args[1]) || !DeferOrPlayTrackByIndex(std::stoi(Args[1])))
        {
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR) + stp::msg::fnc::APP_FNC_INVALID_ARG);
        }
        
        return;
    }
    
    if (!CommandArgIsInt(Args[0]) || !TryPlayTrackByIndex(std::stoi(Args[0])))
    {
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR) + stp::msg::fnc::APP_FNC_INVALID_ARG);
    }
}

void MEngine::HandleNextCommand(const std::vector<std::string> &Args)
{
    if (!Args.empty() && Args[0] == ct::CommandFlagToString(ECommandFlag::Deferred))
    {
        if (!DeferOrPlayNextTrack())
        {
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_WARNING) + stp::msg::fnc::APP_FNC_TRY_NEXT);
        }
        return;
    }
    
    if (!TryNext() && !TryPlayNextTrackOrFirst())
    {
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_WARNING) + stp::msg::fnc::APP_FNC_TRY_NEXT);
    }
}

void MEngine::HandlePrevCommand(const std::vector<std::string> &Args)
{
    if (!Args.empty() && Args[0] == ct::CommandFlagToString(ECommandFlag::Deferred))
    {
        if (!DeferOrPlayPrevTrack())
        {
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_WARNING) + stp::msg::fnc::APP_FNC_TRY_PREV);
        }
        return;
    }
    
    if (!TryPrev() && !TryPlayPrevTrackOrLast())
    {
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_WARNING) + stp::msg::fnc::APP_FNC_TRY_PREV);
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
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR) + stp::msg::fnc::APP_FNC_INVALID_ARG);
            return false;
        }

        if (!std::isfinite(Volume))
        {
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR) + stp::msg::fnc::APP_FNC_INVALID_ARG);
            return false;
        }

        if (Volume < gp::MIN_VOLUME || Volume > gp::MAX_VOLUME)
        {
            ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR) + stp::msg::fnc::APP_FNC_INVALID_ARG);
            return false;
        }

        CurrentVolume = Volume;
        return true;
    }
    catch (...)
    {
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR) + stp::msg::fnc::APP_FNC_INVALID_ARG);
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
    
    if (TrackLibraryStorage.EnsureDirectoryExists(DefaultContentDir))
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
    const FScanResult ScanResult = AudioFileScanner.ScanPath(InPath);
    TrackLibrary.SetAllTracks(ScanResult.Tracks);
    TrackLibrary.SetActiveTrackListByName("all");
}

void MEngine::RefreshTrackLibrary(const std::filesystem::path &InPath)
{
    const FScanResult ScanResult = AudioFileScanner.ScanPath(InPath);
    
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    if (AudioPlayerState == EAudioPlayerState::Playing || AudioPlayerState == EAudioPlayerState::Paused) 
    {
        AudioBackend.StopTrack();
        SetAudioPlayerState(EAudioPlayerState::Idle);
        ClearNowPlaying();
        ClearPendingPlayback();
    }
    
    TrackLibrary.SetAllTracks(ScanResult.Tracks);
    TrackLibrary.SetActiveTrackListByName("all");
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
        ClearNowPlaying();
        ClearPendingPlayback();
        bWantExit = true;
        return true;
    }
    
    return false;
}

bool MEngine::TryPrev() 
{   
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    std::filesystem::path Path = TrackLibrary.GetPrevTrackPath();
    if (!TrackLibrary.IsEmpty() && !Path.empty())
    {
        if (AudioBackend.PlayTrack(Path)) 
        {
            SetAudioPlayerState(EAudioPlayerState::Playing);
            TrackLibrary.SetPrevIndex();
            SetNowPlaying(Path);
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
            SetNowPlaying(Path);
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
            ClearNowPlaying();
            ClearPendingPlayback();
            bContinue = true;
        }
    }
    
    return bContinue;
}

bool MEngine::TryNext() 
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    std::filesystem::path Path = TrackLibrary.GetNextTrackPath();
    if (!TrackLibrary.IsEmpty() && !Path.empty())
    {
        if (AudioBackend.PlayTrack(Path)) 
        {
            SetAudioPlayerState(EAudioPlayerState::Playing);
            TrackLibrary.SetNextIndex();
            SetNowPlaying(Path);
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
            SetNowPlaying(FirstTrackPath);
            return true;
        }
        
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR) + stp::msg::fnc::APP_FNC_FAIL_PLAY_FIRST);
        return false;
    }
    
    if (AudioBackend.PlayTrack(Path)) 
    {
        TrackLibrary.SetNextIndex();
        SetAudioPlayerState(EAudioPlayerState::Playing);
        SetNowPlaying(Path);
        return true;
    }
    
    ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR) + stp::msg::fnc::APP_FNC_ERR_TRY_PLAY_FILE + TrackLibrary.GetCurrentTrackName());
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
            SetNowPlaying(LastTrackPath);
            return true;
        }
        
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR) + stp::msg::fnc::APP_FNC_FAIL_PLAY_LAST);
        return false;
    }
    
    if (AudioBackend.PlayTrack(Path)) 
    {
        TrackLibrary.SetPrevIndex();
        SetAudioPlayerState(EAudioPlayerState::Playing);
        SetNowPlaying(Path);
        return true;
    }
    
    ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR) + stp::msg::fnc::APP_FNC_ERR_TRY_PLAY_FILE + TrackLibrary.GetCurrentTrackName());
    return false;
}

bool MEngine::TryPlayRandomTrack() 
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    if (TrackLibrary.IsEmpty()) 
    {
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR) + stp::msg::fnc::APP_FNC_ERR_LIBRARY_EMPTY);
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
            SetNowPlaying(Path);
            return true;
        }
        
        ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR) + stp::msg::fnc::APP_FNC_FAIL_PLAY);
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
        SetNowPlaying(Path);
        return true;
    }
    
    ConsoleIO.PrintOutputMessage(std::string(stp::msg::APP_ERROR) + stp::msg::fnc::APP_FNC_ERR_TRY_PLAY_FILE + TrackLibrary.GetCurrentTrackName());
    return false;
}

bool MEngine::TryPlayCurrentTrack()
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    std::filesystem::path Path = TrackLibrary.GetCurrentTrackPath();
    if (Path.empty())
    {
        Path = TrackLibrary.GetTrackPathByIndex(0);
        if (!Path.empty())
        {
            TrackLibrary.SetCurrentIndex(0);
        }
    }
    
    if (!Path.empty() && AudioBackend.PlayTrack(Path))
    {
        SetAudioPlayerState(EAudioPlayerState::Playing);
        SetNowPlaying(Path);
        return true;
    }
    
    return false;
}

bool MEngine::TryPlayTrackByIndex(int TrackIndex)
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    if (TrackIndex < 0 || TrackIndex >= TrackLibrary.GetTrackListSize())
    {
        return false;
    }
    
    std::filesystem::path Path = TrackLibrary.GetTrackPathByIndex(TrackIndex);
    if (Path.empty())
    {
        return false;
    }
    
    if (AudioBackend.PlayTrack(Path))
    {
        TrackLibrary.SetCurrentIndex(TrackIndex);
        SetAudioPlayerState(EAudioPlayerState::Playing);
        SetNowPlaying(Path);
        ClearPendingPlayback();
        return true;
    }
    
    return false;
}

bool MEngine::DeferOrPlayTrackByIndex(int TrackIndex)
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    if (TrackIndex < 0 || TrackIndex >= TrackLibrary.GetTrackListSize())
    {
        return false;
    }
    
    std::filesystem::path Path = TrackLibrary.GetTrackPathByIndex(TrackIndex);
    if (Path.empty())
    {
        return false;
    }
    
    if (AudioPlayerState != EAudioPlayerState::Playing)
    {
        if (AudioBackend.PlayTrack(Path))
        {
            TrackLibrary.SetCurrentIndex(TrackIndex);
            SetAudioPlayerState(EAudioPlayerState::Playing);
            SetNowPlaying(Path);
            ClearPendingPlayback();
            return true;
        }
        
        return false;
    }
    
    SetPendingPlayback(Path, TrackIndex);
    return true;
}

bool MEngine::DeferOrPlayNextTrack()
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    if (TrackLibrary.IsEmpty())
    {
        return false;
    }
    
    int TargetIndex = TrackLibrary.GetCurrentIndex() + 1;
    if (TargetIndex >= TrackLibrary.GetTrackListSize())
    {
        TargetIndex = 0;
    }
    
    std::filesystem::path Path = TrackLibrary.GetTrackPathByIndex(TargetIndex);
    if (Path.empty())
    {
        return false;
    }
    
    if (AudioPlayerState != EAudioPlayerState::Playing)
    {
        if (AudioBackend.PlayTrack(Path))
        {
            TrackLibrary.SetCurrentIndex(TargetIndex);
            SetAudioPlayerState(EAudioPlayerState::Playing);
            SetNowPlaying(Path);
            ClearPendingPlayback();
            return true;
        }
        
        return false;
    }
    
    SetPendingPlayback(Path, TargetIndex);
    return true;
}

bool MEngine::DeferOrPlayPrevTrack()
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    if (TrackLibrary.IsEmpty())
    {
        return false;
    }
    
    int TargetIndex = TrackLibrary.GetCurrentIndex() - 1;
    if (TargetIndex < 0)
    {
        TargetIndex = TrackLibrary.GetTrackListSize() - 1;
    }
    
    std::filesystem::path Path = TrackLibrary.GetTrackPathByIndex(TargetIndex);
    if (Path.empty())
    {
        return false;
    }
    
    if (AudioPlayerState != EAudioPlayerState::Playing)
    {
        if (AudioBackend.PlayTrack(Path))
        {
            TrackLibrary.SetCurrentIndex(TargetIndex);
            SetAudioPlayerState(EAudioPlayerState::Playing);
            SetNowPlaying(Path);
            ClearPendingPlayback();
            return true;
        }
        
        return false;
    }
    
    SetPendingPlayback(Path, TargetIndex);
    return true;
}

bool MEngine::TryPlayPendingTrack()
{
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    if (!PendingPlaybackInfo.bValid)
    {
        return false;
    }
    
    const FPendingPlaybackInfo Pending = PendingPlaybackInfo;
    if (!TrackLibrary.SetActiveTrackListByName(Pending.SourceListName))
    {
        ClearPendingPlayback();
        return false;
    }
    
    if (Pending.SourceIndex < 0 || Pending.SourceIndex >= TrackLibrary.GetTrackListSize())
    {
        ClearPendingPlayback();
        return false;
    }
    
    if (AudioBackend.PlayTrack(Pending.Path))
    {
        TrackLibrary.SetCurrentIndex(Pending.SourceIndex);
        SetAudioPlayerState(EAudioPlayerState::Playing);
        SetNowPlaying(Pending.Path);
        ClearPendingPlayback();
        return true;
    }
    
    ClearPendingPlayback();
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

void MEngine::StartPowerEventWatcher()
{
    PowerEventWatcher.Start();
}

void MEngine::StopPowerEventWatcher()
{
    PowerEventWatcher.Stop();
}

void MEngine::HandleSystemSleepStart()
{
    std::lock_guard<std::mutex> Lock(EngineMutex);

    if (AudioPlayerState != EAudioPlayerState::Playing)
    {
        bPausedBySystemSleep = false;
        return;
    }

    AudioBackend.PauseTrack();
    SetAudioPlayerState(EAudioPlayerState::Paused);
    bPausedBySystemSleep = true;
}

void MEngine::HandleSystemResume()
{
    std::lock_guard<std::mutex> Lock(EngineMutex);

    bPausedBySystemSleep = false;
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
        SnapshotData.ActiveTrackListName = TrackLibrary.GetActiveTrackListName();
        if (NowPlayingInfo.bValid)
            SnapshotData.CurrentTrackName = NowPlayingInfo.TrackName;
        else
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
    std::lock_guard<std::mutex> Lock(EngineMutex);
    
    if (!NowPlayingInfo.bValid)
    {
        TrackInfo.DurationSec = 0.f;
        TrackInfo.PositionSec = 0.f;
        TrackInfo.RemainingSec = 0.f;
        return TrackInfo;
    }
    
    TrackInfo.DurationSec = AudioBackend.GetTrackDurationSec();
    TrackInfo.PositionSec = AudioBackend.GetTrackCurrentOffsetSec();
    TrackInfo.RemainingSec = AudioBackend.GetTrackRemainingSec();
    
    return TrackInfo;
}

void MEngine::SetNowPlaying(const std::filesystem::path &Path)
{
    ClearPendingPlayback();
    NowPlayingInfo.Path = Path;
    NowPlayingInfo.TrackName = Path.filename().string();
    NowPlayingInfo.SourceListName = TrackLibrary.GetActiveTrackListName();
    NowPlayingInfo.SourceIndex = TrackLibrary.GetCurrentIndex();
    NowPlayingInfo.bValid = true;
}

void MEngine::ClearNowPlaying()
{
    NowPlayingInfo = {};
}

void MEngine::SetPendingPlayback(const std::filesystem::path &Path, int SourceIndex)
{
    PendingPlaybackInfo.Path = Path;
    PendingPlaybackInfo.SourceListName = TrackLibrary.GetActiveTrackListName();
    PendingPlaybackInfo.SourceIndex = SourceIndex;
    PendingPlaybackInfo.bValid = true;
}

void MEngine::ClearPendingPlayback()
{
    PendingPlaybackInfo = {};
}
