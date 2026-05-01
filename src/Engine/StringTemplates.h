//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#pragma once

#include <string>

/** STRING TEMPLATES */
namespace stp 
{
    /** SEPARATOR */
    namespace sep
    {
        /** SubCategory Separator Type T: "├──" */
        constexpr const char* SUBCAT_SEP_T = "├──";
        
        /** SubCategory Separator Type L: "└──" */
        constexpr const char* SUBCAT_SEP_L = "└──";
        
        /** SubCategory Separator Type I: "│" */
        constexpr const char* SUBCAT_SEP_I = "│";
        
        /** SubCategory Separator Type TAB: "  " */
        constexpr const char* SUBCAT_SEP_TAB = "  ";
    }
    
    /** MESSAGE */
    namespace msg
    {
        constexpr const char* ENGINE_INIT_MSG = "TAP - Terminal Audio Player V.";
        constexpr const char* APP_WARNING_MSG = "[TAP::WARNING] ";
        constexpr const char* APP_ERROR_MSG = "[TAP::ERROR] ";
        constexpr const char* APP_LIBRARY_MSG = "[TAP::LIBRARY] ";
        constexpr const char* APP_HELP_MSG = "[TAP::HELP] ";
        constexpr const char* APP_STATUS_MSG = "[TAP::STATUS] ";
        constexpr const char* APP_SHUTDOWN_MSG = "[TAP::SHUTDOWN] Shutting down...";
    
        /** FUNCTION MESSAGE */
        namespace fnc
        {
            constexpr const char* APP_FNC_LOW_ARG_MSG = "...Too low arguments...";
            constexpr const char* APP_FNC_INVALID_ARG_MSG = "Invalid argument.";
            constexpr const char* APP_FNC_INVALID_INDEX_MSG = "Invalid index.";
            constexpr const char* APP_FNC_UNKNOWN_CMD_MSG = "Unknown command.";
            constexpr const char* APP_FNC_UNKNOWN_HELP_MSG = "Unknown help topic.";
            constexpr const char* APP_FNC_TOTAL_TRACKS_MSG = "Total tracks found: ";
            constexpr const char* APP_FNC_LIB_RESET_MSG = "Library refreshed.";
            constexpr const char* APP_FNC_CUR_TRACK_MSG = " <- current/selected";
            constexpr const char* APP_FNC_ERR_UNEXP_MSG = "Unexpected Error. Force exit.";
            constexpr const char* APP_FNC_ERR_TRY_PLAY_FILE_MSG = "Error while trying to play: ";
            constexpr const char* APP_FNC_ERR_LIBRARY_EMPTY_MSG = " rack library is empty.";
            constexpr const char* APP_FNC_PATH_EMPTY_MSG = "Track path is empty.";
            constexpr const char* APP_FNC_FAIL_SET_INDEX_MSG = "Failed to set current track index.";
            constexpr const char* APP_FNC_FAIL_PLAY_MSG = "Failed play.";
            constexpr const char* APP_FNC_FAIL_PLAY_FIRST_MSG = "Failed to play first track.";
            constexpr const char* APP_FNC_FAIL_PLAY_LAST_MSG = "Failed to play last track.";
            constexpr const char* APP_FNC_TRY_PLAY_MSG = "Track is already playing or not selected.";
            constexpr const char* APP_FNC_TRY_PAUSE_MSG = "Track is already paused or not playing.";
            constexpr const char* APP_FNC_TRY_STOP_MSG = "Track is already stopped.";
            constexpr const char* APP_FNC_TRY_NEXT_MSG = "No next track.";
            constexpr const char* APP_FNC_TRY_PREV_MSG = "No previous track.";
        };
    };
};