//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#include "AudioBackend.h"

#include "Engine/GlobalParameters.h"

#include <iostream>
#include <string>

MAudioBackend::MAudioBackend() 
{
}

void MAudioBackend::Init() 
{
}

bool MAudioBackend::PlayTrack(const std::filesystem::path &TrackPath)
{
    if (!LoadBufferFromFile(TrackPath))
    {
        if (gp::bPrintDebugInfo) 
        {
            std::cout << "[AudioBackend] Failed to load track." << std::endl;
        }
            
        return false;
    }
    else 
    {
        Sound.stop();
        Sound.setBuffer(Buffer);
        Sound.play();
        if (Sound.getStatus() == sf::Sound::Status::Playing) 
        {
            if (gp::bPrintDebugInfo)
            {
                std::cout << "[AudioBackend] Track playing." << std::endl;
            }
            
            return true;
        }
    }
    
    return false;
}

bool MAudioBackend::ResumeTrack() 
{
    if (Sound.getStatus() != sf::Sound::Status::Paused) return false;
    
    Sound.play();
    if (gp::bPrintDebugInfo) 
    {
        std::cout << "[AudioBackend] Track resumed." << std::endl;
    }
    if (Sound.getStatus() == sf::Sound::Status::Playing) return true;
    
    return false;
}

bool MAudioBackend::PauseTrack()
{
    if (Sound.getStatus() == sf::Sound::Status::Playing) 
    {
        Sound.pause();
        if (Sound.getStatus() == sf::Sound::Status::Paused) 
        {
            if (gp::bPrintDebugInfo)
            {
                std::cout << "[AudioBackend] Track paused." << std::endl;
            }
        
            return true;
        }
    }
    
    return false;
}

bool MAudioBackend::StopTrack() 
{
    if (Sound.getStatus() == sf::Sound::Status::Playing || Sound.getStatus() == sf::Sound::Status::Paused) 
    {
        Sound.stop();
        if (Sound.getStatus() == sf::Sound::Status::Stopped) 
        {
            if (gp::bPrintDebugInfo)
            {
                std::cout << "[AudioBackend] Track Stopped." << std::endl;
            }
            return true;
        }
    }
    
    return false;
}

bool MAudioBackend::IsStopped() const
{
    return Sound.getStatus() == sf::Sound::Status::Stopped;
}

float MAudioBackend::GetTrackDurationSec() const
{
    return float(Buffer.getDuration().asSeconds());
}

float MAudioBackend::GetTrackCurrentOffsetSec() const
{
    return float(Sound.getPlayingOffset().asSeconds());
}

float MAudioBackend::GetTrackRemainingSec() const
{
    float TotalDurationSec = Buffer.getDuration().asSeconds();
    float TotalCurrentOffsetSec = Sound.getPlayingOffset().asSeconds();
    return TotalDurationSec - TotalCurrentOffsetSec;
}

void MAudioBackend::SetLoop(bool bLoop) 
{
    Sound.setLoop(bLoop);
}

void MAudioBackend::SetVolume(float Volume)
{
    Sound.setVolume(Volume);
}

bool MAudioBackend::ShutDown() 
{
    Sound.stop();
    
    return true;
}

bool MAudioBackend::LoadBufferFromFile(const std::filesystem::path& InPath) 
{
    
    if (InPath.empty() || !(std::filesystem::exists(InPath) && std::filesystem::is_regular_file(InPath)))
    {
        return false;
    }

    return Buffer.loadFromFile(InPath.string());
}

