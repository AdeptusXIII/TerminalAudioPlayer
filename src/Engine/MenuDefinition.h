//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".
#pragma once

#include <string>

const int MAX_MAIN_MENU_OPTION_COUNT = 7;
const int MAX_LIBRARY_MENU_OPTION_COUNT = 3;
const int MAX_PLAYBACK_MENU_OPTION_COUNT = 4;

/** SubCategory Separator Type T: "├──" */
const std::string SUBCAT_SEP_T = "├──";

/** SubCategory Separator Type L: "└──" */
const std::string SUBCAT_SEP_L = "└──";

/** SubCategory Separator Type I: "│" */
const std::string SUBCAT_SEP_I = "│";

/** SubCategory Separator Type TAB: "  " */
const std::string SUBCAT_SEP_TAB = "  ";

enum class EMenuSection 
{
    MainMenu,
    LibraryMenu,
    PlaybackMenu,
    
    None
};

enum class EMainMenuOption
{
    Exit = 0,
    Prev = 1,
    Play = 2,
    Pause = 3,
    Stop = 4,
    Next = 5,
    Library = 6,
    PlaybackMode = 7,
    
    None = 99
};

enum class ELibraryMenuOption
{
    Back = 0,
    List = 1,
    SelectByIndex = 2,
    Refresh = 3,
    
    None = 99
};

enum class EPlaybackMenuOption 
{
    Back = 0,
    Once = 1,
    LoopOne = 2,
    LoopAll = 3,
    LoopShuffle = 4,
    
    None = 99
};
