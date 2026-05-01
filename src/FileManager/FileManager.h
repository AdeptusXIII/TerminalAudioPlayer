//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#pragma once

#include <string>
#include <filesystem>

class MFileManager 
{
public:
    MFileManager();
    
    void Init();
    bool CreateDir(const std::string &Dir) const;
    
private:
};

