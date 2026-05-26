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
    sf::Music NewMusic;
    if (!OpenMusicFromFile(TrackPath, NewMusic))
    {
        if (gp::bPrintDebugInfo)
        {
            std::cout << "[AudioBackend] Failed to load track." << std::endl;
        }

        return false;
    }

    if (Music.has_value()) Music->stop();

    Music.emplace(std::move(NewMusic));
    Music->setVolume(CurrentVolume);
    Music->setLooping(bLooping);
    Music->play();
    if (Music->getStatus() == sf::SoundSource::Status::Playing)
    {
        if (gp::bPrintDebugInfo)
        {
            std::cout << "[AudioBackend] Track playing." << std::endl;
        }

        return true;
    }

    return false;
}

bool MAudioBackend::LoadTrackPaused(const std::filesystem::path &TrackPath, float PositionSec)
{
    sf::Music NewMusic;
    if (!OpenMusicFromFile(TrackPath, NewMusic))
    {
        return false;
    }

    if (Music.has_value()) Music->stop();

    Music.emplace(std::move(NewMusic));
    Music->setVolume(CurrentVolume);
    Music->setLooping(bLooping);
    Music->setPlayingOffset(sf::seconds(PositionSec));
    Music->play();
    Music->pause();

    return Music->getStatus() == sf::SoundSource::Status::Paused;
}

bool MAudioBackend::ResumeTrack()
{
    if (!Music.has_value()) return false;

    if (Music->getStatus() != sf::SoundSource::Status::Paused) return false;

    Music->play();
    if (gp::bPrintDebugInfo)
    {
        std::cout << "[AudioBackend] Track resumed." << std::endl;
    }
    if (Music->getStatus() == sf::SoundSource::Status::Playing) return true;

    return false;
}

bool MAudioBackend::PauseTrack()
{
    if (!Music.has_value()) return false;

    if (Music->getStatus() == sf::SoundSource::Status::Playing)
    {
        Music->pause();
        if (Music->getStatus() == sf::SoundSource::Status::Paused)
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
    if (!Music.has_value()) return false;

    if (Music->getStatus() == sf::SoundSource::Status::Playing || Music->getStatus() == sf::SoundSource::Status::Paused)
    {
        Music->stop();
        if (Music->getStatus() == sf::SoundSource::Status::Stopped)
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
    if (!Music.has_value()) return true;

    return Music->getStatus() == sf::SoundSource::Status::Stopped;
}

float MAudioBackend::GetTrackDurationSec() const
{
    if (!Music.has_value()) return 0.f;

    return float(Music->getDuration().asSeconds());
}

float MAudioBackend::GetTrackCurrentOffsetSec() const
{
    if (!Music.has_value()) return 0.f;

    return float(Music->getPlayingOffset().asSeconds());
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

    if (!Music.has_value()) return;

    Music->setLooping(bLoop);
}

void MAudioBackend::SetVolume(float Volume)
{
    CurrentVolume = Volume;

    if (!Music.has_value()) return;

    Music->setVolume(Volume);
}

bool MAudioBackend::ShutDown()
{
    if (!Music.has_value()) return false;

    Music->stop();

    return true;
}

bool MAudioBackend::OpenMusicFromFile(const std::filesystem::path& InPath, sf::Music& OutMusic)
{
    if (InPath.empty() || !(std::filesystem::exists(InPath) && std::filesystem::is_regular_file(InPath)))
    {
        return false;
    }

    return OutMusic.openFromFile(InPath);
}
