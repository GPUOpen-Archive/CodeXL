//------------------------------ kaCommandIDs.h ------------------------------

#ifndef __KACOMMANDIDS
#define __KACOMMANDIDS

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommandIds.h>

// ------ This file contains wxWindows IDs for CodeXLApp and CodeXL commands: ------

enum kaMainMenuItemCommands
{
    ID_KA_OPENCL_BUILD = STATIC_ANALYZER_FIRST_COMMAND_ID,
    ID_KA_CANCEL_BUILD,
    ID_KA_MODE,
    ID_KA_NEW_FILE,
    ID_KA_NEW_FILE_ANALYZE_MENU,
    ID_KA_ADD_FILE,
    ID_KA_ADD_FILE_ANALYZE_MENU,
    ID_KA_ANALYZE_SETTING,

    kaAmountOfMainMenuCommands = ID_KA_ANALYZE_SETTING + 1
};

#endif  // __KACOMMANDIDS
