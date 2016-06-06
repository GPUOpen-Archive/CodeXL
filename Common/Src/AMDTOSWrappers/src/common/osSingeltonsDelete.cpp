//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSingeltonsDelete.cpp
///
//=====================================================================

//------------------------------ osSingeltonsDelete.cpp ------------------------------

// Local:
#include <common/osSingeltonsDelete.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osTransferableObjectCreatorsManager.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <AMDTOSWrappers/Include/osUnhandledExceptionHandler.h>
#endif

// A static instance of the singleton deleter class. Its destructor will delete all
// the singletons instances.
static osSingeltonsDelete singeltonDeleter;


// ---------------------------------------------------------------------------
// Name:        osSingeltonsDelete::~osSingeltonsDelete
// Description: Destructor - deletes all the singleton instances.
// Author:      AMD Developer Tools Team
// Date:        24/4/2004
// ---------------------------------------------------------------------------
osSingeltonsDelete::~osSingeltonsDelete()
{
    // Delete os_stat_applicationDllsPath:
    delete os_stat_applicationDllsPath;
    os_stat_applicationDllsPath = NULL;

    // Delete the osTransferableObjectsCreatorsManager single instance:
    delete osTransferableObjectCreatorsManager::_pMySingleInstance;
    osTransferableObjectCreatorsManager::_pMySingleInstance = NULL;

    // Delete the debug log:
    delete osDebugLog::_pMySingleInstance;
    osDebugLog::_pMySingleInstance = NULL;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // Delete the osUnhandledExceptionHandler single instance:
    delete osUnhandledExceptionHandler::_pMySingleInstance;
    osUnhandledExceptionHandler::_pMySingleInstance = NULL;
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
}



