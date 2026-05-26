//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#include "AudioBackend.h"

#include "Engine/GlobalParameters.h"

#include <iostream>
#include <string>

MAudioBackend::MAudioBackend()
{
}

bool MAudioBackend::PlayTrack(const std::filesystem::path &TrackPath)
{
    if (Sound.has_value()) Sound->stop();

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
        Sound.emplace(Buffer);
        Sound->setVolume(CurrentVolume);
        Sound->setLooping(bLooping);
        Sound->play();
        if (Sound->getStatus() == sf::Sound::Status::Playing)
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

bool MAudioBackend::LoadTrackPaused(const std::filesystem::path &TrackPath, float PositionSec)
{
    if (Sound.has_value()) Sound->stop();

    if (!LoadBufferFromFile(TrackPath))
    {
        return false;
    }

    Sound.emplace(Buffer);
    Sound->setVolume(CurrentVolume);
    Sound->setLooping(bLooping);
    Sound->setPlayingOffset(sf::seconds(PositionSec));
    Sound->play();
    Sound->pause();

    return Sound->getStatus() == sf::Sound::Status::Paused;
}

bool MAudioBackend::ResumeTrack()
{
    if (!Sound.has_value()) return false;

    if (Sound->getStatus() != sf::Sound::Status::Paused) return false;

    Sound->play();
    if (gp::bPrintDebugInfo)
    {
        std::cout << "[AudioBackend] Track resumed." << std::endl;
    }
    if (Sound->getStatus() == sf::Sound::Status::Playing) return true;

    return false;
}

bool MAudioBackend::PauseTrack()
{
    if (!Sound.has_value()) return false;

    if (Sound->getStatus() == sf::Sound::Status::Playing)
    {
        Sound->pause();
        if (Sound->getStatus() == sf::Sound::Status::Paused)
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
    if (!Sound.has_value()) return false;

    if (Sound->getStatus() == sf::Sound::Status::Playing || Sound->getStatus() == sf::Sound::Status::Paused)
    {
        Sound->stop();
        if (Sound->getStatus() == sf::Sound::Status::Stopped)
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
    if (!Sound.has_value()) return true;

    return Sound->getStatus() == sf::Sound::Status::Stopped;
}

float MAudioBackend::GetTrackDurationSec() const
{
    return float(Buffer.getDuration().asSeconds());
}

float MAudioBackend::GetTrackCurrentOffsetSec() const
{
    if (!Sound.has_value()) return 0.f;

    return float(Sound->getPlayingOffset().asSeconds());
}

float MAudioBackend::GetTrackRemainingSec() const
{
    float TotalDurationSec = GetTrackDurationSec();
    float TotalCurrentOffsetSec = GetTrackCurrentOffsetSec();
    return TotalDurationSec - TotalCurrentOffsetSec;
}

void MAudioBackend::SetLoop(bool bLoop)
{
    bLooping = bLoop;

    if (!Sound.has_value()) return;

    Sound->setLooping(bLoop);
}

void MAudioBackend::SetVolume(float Volume)
{
    CurrentVolume = Volume;

    if (!Sound.has_value()) return;

    Sound->setVolume(Volume);
}

bool MAudioBackend::ShutDown()
{
    if (!Sound.has_value()) return false;

    Sound->stop();

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
