//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afCommandIds.h
///
//==================================================================================

#ifndef __AFCOMMANDIDS_H
#define __AFCOMMANDIDS_H


enum afGeneralCommmands
{
    // Edit menu:
    ID_CUT = 0,
    afFirstEditCommand = ID_CUT,
    ID_COPY,
    ID_PASTE,
    ID_SELECT_ALL,
    ID_FIND,
    ID_FIND_NEXT,
    ID_FIND_PREV,
    ID_SHOW_LINE_NUMBERS,
    AF_ID_SAVE_FILE,
    AF_ID_SAVE_FILE_AS,
    ID_GO_TO,

    AF_ID_NEW_PROJECT,
    AF_ID_OPEN_STARTUP_PAGE,
    AF_ID_OPEN_PROJECT,
    AF_ID_CLOSE_PROJECT,
    AF_ID_SAVE_PROJECT,
    AF_ID_SAVE_PROJECT_AS,
    AF_ID_PROJECT_SETTINGS,
    AF_ID_OPEN_FILE,
    AF_ID_RESET_GUI_LAYOUTS,
    AF_ID_TOOLS_SYSTEM_INFO,
    AF_ID_TOOLS_OPTIONS,

    AF_ID_HELP_USER_GUIDE,
    AF_ID_HELP_QUICK_START,
    AF_ID_HELP_UPDATES,
    AF_ID_HELP_DEV_TOOLS_SUPPORT_FORUM,
    AF_ID_HELP_LOAD_TEAPOT_SAMPLE,
    AF_ID_HELP_ABOUT,
    AF_ID_EXIT,

    GPU_DEBUGGER_FIRST_COMMAND_ID = 100,
    SHARED_PROFILING_FIRST_COMMAND_ID = 2000,
    GPU_PROFILER_FIRST_COMMAND_ID = 2100,
    CPU_PROFILER_FIRST_COMMAND_ID = 2200,
    STATIC_ANALYZER_FIRST_COMMAND_ID = 2300,
    POWER_PROFILER_FIRST_COMMAND_ID = 2400
};


enum afRecentlyUsedProjects
{
    // Recently used projects names:
    ID_RECENTLY_USED_PROJECT_0 = 0,
    ID_RECENTLY_USED_PROJECT_1,
    ID_RECENTLY_USED_PROJECT_2,
    ID_RECENTLY_USED_PROJECT_3,
    ID_RECENTLY_USED_PROJECT_4,
    ID_RECENTLY_USED_PROJECT_5,
    ID_RECENTLY_USED_PROJECT_6,
    ID_RECENTLY_USED_PROJECT_7,
    ID_RECENTLY_USED_PROJECT_8,
    ID_RECENTLY_USED_PROJECT_9,
    ID_RECENT_PROJECTS_SUB_MENU,
    afAmountOfRecentlyUsedCommands = ID_RECENTLY_USED_PROJECT_9
};

// The maximum number of recent files that we present on the Recent Projects subMenu.
// Notice that each entry has a predefined ID (in gdCommandID.h). Currently there are ID_0 to ID_9.
// So, if you want to change and allow for more than 10 recent entries, you should else change the appropriate ID file and allocate more ID's
#define AF_MAX_NUMBER_OF_RECENT_PROJECTS_TO_SHOW (ID_RECENTLY_USED_PROJECT_9 - ID_RECENTLY_USED_PROJECT_0 + 1)


#endif //__AFCOMMANDIDS_H

