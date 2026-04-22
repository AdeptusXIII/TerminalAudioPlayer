//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".
#pragma once

#include <filesystem>
#include "SFML/Audio.hpp"

class MAudioBackend 
{
public:
    MAudioBackend();
    
    void Init(bool bPrintDebugInfo);

    bool PlayTrack(const std::filesystem::path &TrackPath);
    bool ResumeTrack();
    bool PauseTrack();
    bool StopTrack();
    
    bool IsStopped() const;
    bool ShutDown();
    
    
private:
    
    bool LoadBufferFromFile(const std::filesystem::path &InPath);
    
    bool bPrintDebugInfo;
    sf::SoundBuffer Buffer;
    sf::Sound Sound;
};
