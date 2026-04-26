//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#include "UI/ConsoleIO/ConsoleIO.h"

#include "../UISnapshot.h"

#include <iostream>
#include <limits>

MConsoleIO::MConsoleIO() 
{
    
}

void MConsoleIO::PrintMainMenuAndState(const FUISnapshotData &UISnapshot)
{
    PrintAudioPlayerState(UISnapshot.AudioPlayerState);
    PrintPlaybackMode(UISnapshot.PlaybackMode);
    PrintCurrentTrack(UISnapshot.CurrentTrackName);
    PrintMenuSection(UISnapshot.CurrentMenuSection);
    
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

void MConsoleIO::PrintLibraryMenuAndState(const FUISnapshotData &UISnapshot)
{
    PrintAudioPlayerState(UISnapshot.AudioPlayerState);
    PrintPlaybackMode(UISnapshot.PlaybackMode);
    PrintCurrentTrack(UISnapshot.CurrentTrackName);
    PrintMenuSection(UISnapshot.CurrentMenuSection);
    
    std::cout << "Choose option: " << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(1) - [ List ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(2) - [ Select track by index ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(3) - [ Refresh ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_L << "(0) - [ Back ]" << std::endl;
}

void MConsoleIO::PrintPlaybackModeMenuAndState(const FUISnapshotData &UISnapshot)
{
    PrintAudioPlayerState(UISnapshot.AudioPlayerState);
    PrintPlaybackMode(UISnapshot.PlaybackMode);
    PrintCurrentTrack(UISnapshot.CurrentTrackName);
    PrintMenuSection(UISnapshot.CurrentMenuSection);
    
    std::cout << "Choose option: " << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(1) - [ Once ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(2) - [ Loop One ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(3) - [ Loop All ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_T << "(4) - [ Loop Shuffle ]" << std::endl;
    std::cout << SUBCAT_SEP_TAB << SUBCAT_SEP_L << "(0) - [ Back ]" << std::endl;
}

void MConsoleIO::PrintTrackList(const FUISnapshotData &UISnapshot)
{
    
    std::cout << std::endl;
    PrintMenuSection(UISnapshot.CurrentMenuSection);
    std::cout << "Total tracks found: " << UISnapshot.TrackCount << std::endl;

    for (int i = 0; i < UISnapshot.TrackCount; i++)
    {
        std::string CurrentTrack = (i == UISnapshot.CurrentTrackIndex) ? " <- current/selected" : "";
        std::string SubCatTorL = (i == UISnapshot.TrackCount - 1) ? SUBCAT_SEP_L : SUBCAT_SEP_T;

        std::cout << SUBCAT_SEP_TAB + SubCatTorL 
                  << "Index(" << i << ")" 
                  << "[" << UISnapshot.TrackList[i] 
                  << "] "
                  << CurrentTrack
                  << std::endl;
    }
}

void MConsoleIO::PrintTotalTracksNum(const int &InTotalTracks) 
{
    std::cout << "[TAP::LIBRARY] Library refreshed. Total tracks found: "
                          << InTotalTracks << "." << std::endl;
}

int MConsoleIO::ReadIntInRange(int Min, int Max, const std::string& Prompt)
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

void MConsoleIO::PrintAudioPlayerState(const EAudioPlayerState &InAudioPlayerState)
{
    EAudioPlayerState CurrentAudioPlayerState = EAudioPlayerState::None;
    
    CurrentAudioPlayerState = InAudioPlayerState;
    
    switch (CurrentAudioPlayerState)
    {
        case EAudioPlayerState::Idle:
            std::cout << std::endl << "[TAP::STATE::IDLE]" << std::endl;
            break;

        case EAudioPlayerState::Playing:
            std::cout << std::endl <<  "[TAP::STATE::PLAYING]" << std::endl;
            break;

        case EAudioPlayerState::Paused:
            std::cout << std::endl << "[TAP::STATE::PAUSED]" << std::endl;
            break;
    }
}

void MConsoleIO::PrintPlaybackMode(const EPlaybackMode &InPlaybackMode) 
{
    EPlaybackMode CurrentPlaybackMode = EPlaybackMode::None;
    
    CurrentPlaybackMode = InPlaybackMode;
    
    switch (CurrentPlaybackMode) 
    {
        case EPlaybackMode::Once:
            std::cout << "[TAP::PLAYBACK-MODE::ONCE]" << std::endl;
            break;
            
        case EPlaybackMode::LoopOne:
            std::cout << "[TAP::PLAYBACK-MODE::LOOP-ONE]" << std::endl;
            break;
            
        case EPlaybackMode::LoopAll:
            std::cout<< "[TAP::PLAYBACK-MODE::LOOP-ALL]" << std::endl;
            break;
            
        case EPlaybackMode::LoopShuffle:
            std::cout << "[TAP::PLAYBACK-MODE::LOOP-SHUFFLE]" << std::endl;
            break;        
    }
}

void MConsoleIO::PrintCurrentTrack(const std::string &InTrackName) 
{
    std::string CurrentTrackName = "None";
    
    CurrentTrackName = InTrackName;
    
    std::cout << "[TAP::SELECTED TRACK:: " << CurrentTrackName << "]" << std::endl;
}

void MConsoleIO::PrintMenuSection(const EMenuSection &InMenuSection) 
{
    EMenuSection CurrentMenuSection = InMenuSection;
    
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