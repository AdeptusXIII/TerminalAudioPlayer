//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#include "FileManager.h"

#include "Engine/GlobalParameters.h"

#include <iostream>

MFileManager::MFileManager()
{
}

void MFileManager::Init() 
{
}

bool MFileManager::CreateDir(const std::string &Dir) const
{
    bool bSuccess = false;
    
    if (!std::filesystem::is_directory(Dir)) 
    {
        if (std::filesystem::create_directories(Dir))
        {
            if (gp::bPrintDebugInfo) 
            {
                std::cout << "[FileManager] Directory " << Dir << " created successfully." << std::endl;
            }
            bSuccess = true;
        }      
    }
    
    return bSuccess;
}