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
        constexpr const char* ENGINE_INIT = "TAP - Terminal Audio Player V.";
        constexpr const char* APP_WARNING = "[TAP::WARNING] ";
        constexpr const char* APP_ERROR = "[TAP::ERROR] ";
        constexpr const char* APP_LIBRARY = "[TAP::LIBRARY] ";
        constexpr const char* APP_HELP = "[TAP::HELP] ";
        constexpr const char* APP_STATUS = "[TAP::STATUS] ";
        constexpr const char* APP_LIST = "[TAP::LIST] ";
        constexpr const char* APP_FIND = "[TAP::FIND] ";
        constexpr const char* APP_SHUTDOWN = "[TAP::SHUTDOWN] Shutting down...";

        /** FUNCTION MESSAGE */
        namespace fnc
        {
            constexpr const char* APP_FNC_LOW_ARG = "...Too low arguments...";
            constexpr const char* APP_FNC_INVALID_ARG = "Invalid argument.";
            constexpr const char* APP_FNC_INVALID_INDEX = "Invalid index.";
            constexpr const char* APP_FNC_UNKNOWN_CMD = "Unknown command.";
            constexpr const char* APP_FNC_UNKNOWN_HELP = "Unknown help topic.";
            constexpr const char* APP_FNC_TOTAL_TRACKS = "Total tracks found: ";
            constexpr const char* APP_FNC_LIB_RESET = "Library refreshed.";
            constexpr const char* APP_FNC_CUR_TRACK = " <- current/selected";
            constexpr const char* APP_FNC_ERR_UNEXP = "Unexpected Error.";
            constexpr const char* APP_FNC_ERR_TRY_PLAY_FILE = "Error while trying to play: ";
            constexpr const char* APP_FNC_ERR_LIBRARY_EMPTY = "Track library is empty.";
            constexpr const char* APP_FNC_PATH_EMPTY = "Path is empty.";
            constexpr const char* APP_FNC_PATH_NOT_EXISTS = "Path is not exists.";
            constexpr const char* APP_FNC_PATH_NOT_DIR = "Path is not a directory.";
            constexpr const char* APP_FNC_FAIL_SET_INDEX = "Failed to set current track index.";
            constexpr const char* APP_FNC_FAIL_PLAY = "Failed play.";
            constexpr const char* APP_FNC_FAIL_PLAY_FIRST = "Failed to play first track.";
            constexpr const char* APP_FNC_FAIL_PLAY_LAST = "Failed to play last track.";
            constexpr const char* APP_FNC_TRY_PLAY = "Track is already playing or not selected.";
            constexpr const char* APP_FNC_TRY_PAUSE = "Track is already paused or not playing.";
            constexpr const char* APP_FNC_TRY_STOP = "Track is already stopped.";
            constexpr const char* APP_FNC_TRY_NEXT = "No next track.";
            constexpr const char* APP_FNC_TRY_PREV = "No previous track.";
        };
    }; // namespace msg

    /** SAVE STATE */
    namespace svst
    {
        constexpr const char* S_ALL = "all";
        constexpr const char* S_FAV = "favorite";
        constexpr const char* S_CUSTOM = "custom";
        constexpr const char* S_ACTIVE_LIST = "active-list";
        constexpr const char* S_PLAYBACK_MODE = "playback-mode";
        constexpr const char* S_PLAYER_AUDIO_STATE= "player-audio-state";
        constexpr const char* S_VOLUME= "volume";
        constexpr const char* S_CURRENT_TRACK= "current-track";
        constexpr const char* S_CURRENT_POSITION= "current-position";
    }
};