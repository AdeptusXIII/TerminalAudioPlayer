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
    
    bool LoadBufferFromFile(const std::filesystem::path &InPath);
    
    sf::SoundBuffer Buffer;
    std::optional<sf::Sound> Sound;
};
