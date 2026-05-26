//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".
#pragma once

#include <filesystem>
#include <optional>

#include "SFML/Audio.hpp"

class MAudioBackend 
{
public:
    MAudioBackend();

    bool PlayTrack(const std::filesystem::path &TrackPath);
    bool LoadTrackPaused(const std::filesystem::path &TrackPath, float PositionSec);
    bool ResumeTrack();
    bool PauseTrack();
    bool StopTrack();
    
    bool IsStopped() const;
    float GetTrackDurationSec() const;
    float GetTrackCurrentOffsetSec() const;
    float GetTrackRemainingSec() const;
    
    void SetLoop(bool bLoop);
    
    void SetVolume(float Volume);
    
    bool ShutDown();
    
    
private:

    bool OpenMusicFromFile(const std::filesystem::path &InPath, sf::Music& OutMusic);

    std::optional<sf::Music> Music;
    float CurrentVolume = 100.0f;
    bool bLooping = false;
};
