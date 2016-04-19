//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atInitializeInfrastructureCommand.cpp
///
//==================================================================================

//------------------------------ atInitializeInfrastructureCommand.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// Local:
#include <inc/atStringConstants.h>
#include <inc/atInitializeInfrastructureCommand.h>


// ---------------------------------------------------------------------------
// Name:        atInitializeInfrastructureCommand::atInitializeInfrastructureCommand
// Description: Constructor.
// Author:      Merav Zanany
// Date:        29/11/2011
// ---------------------------------------------------------------------------
atInitializeInfrastructureCommand::atInitializeInfrastructureCommand()
{
}


// ---------------------------------------------------------------------------
// Name:        atInitializeInfrastructureCommand::~atInitializeInfrastructureCommand
// Description: Destructor.
// Author:      Merav Zanany
// Date:        29/11/2011
// ---------------------------------------------------------------------------
atInitializeInfrastructureCommand::~atInitializeInfrastructureCommand()
{
}


// ---------------------------------------------------------------------------
// Name:        atInitializeInfrastructureCommand::execute
// Description: Implements this class work - Performs required application initializations.
// Return Val: bool  - Success / failure.
// Author:      Merav Zanany
// Date:        29/11/2011
// ---------------------------------------------------------------------------
bool atInitializeInfrastructureCommand::execute()
{
    bool retVal = false;

    // Initialize the un-handled exceptions handler:
    initializeUnhandledExceptionHandler();

    // Initialize the debug log file:
    bool rc1 = initializeDebugLogFile();

    if (!rc1)
    {
        assert(rc1);
    }

    // osDebugLog::instance().setLoggedSeverity(OS_DEBUG_LOG_EXTENSIVE);

    // Output an "Application init begin" log printout:
    OS_OUTPUT_DEBUG_LOG(AT_STR_LOGMSG_APPINITBEGIN, OS_DEBUG_LOG_INFO);

    // Name the application's main thread in Visual Studio's thread's list:
    nameMainThreadInDebugger();

    // Initialize the API package:
    bool rc5 = gaIntializeAPIPackage(false);
    GT_IF_WITH_ASSERT_EX(rc5, AT_STR_LOGMSG_FAILEDTOINITAPI)
    {
        retVal = true;
    }

    // Output an "Application init ended" log printout:
    OS_OUTPUT_DEBUG_LOG(AT_STR_LOGMSG_APPINITENDED, OS_DEBUG_LOG_INFO);

    if (retVal)
    {
        OS_OUTPUT_DEBUG_LOG(AT_STR_LOGMSG_APPINITCMDSUCCEEDED, OS_DEBUG_LOG_INFO);
    }

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        atInitializeInfrastructureCommand::initializeDebugLogFile
// Description: Initialize the debug log file.
// Return Val:  bool - Success / failure.
// Author:      Merav Zanany
// Date:        29/11/2011
// ---------------------------------------------------------------------------
bool atInitializeInfrastructureCommand::initializeDebugLogFile()
{
    bool retVal = false;

    // Set the thread naming prefix:
    osThread::setThreadNamingPrefix("CodeXL GPU debugging tester");

    // Initialize the log file:
    osDebugLog& theDebugLog = osDebugLog::instance();
    retVal = theDebugLog.initialize(AT_AUTOMATIC_TESTER_PRODUCT_NAME, AT_AUTOMATIC_TESTER_PRODUCT_NAME);

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        atInitializeInfrastructureCommand::initializeUnhandledExceptionHandler
// Description:
//  Initializes the unhandled exception handler. This handler will be called when
//  this application crashes.
//
// Author:      Merav Zanany
// Date:        29/11/2011
// ---------------------------------------------------------------------------
void atInitializeInfrastructureCommand::initializeUnhandledExceptionHandler()
{
    /*
    // gdUnhandledExceptionHandler is currently supported on Windows only:
    #if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        // Create and initialize the unhandled exceptions handler:
        gdUnhandledExceptionHandler& unhandledExceptionHandler = gdUnhandledExceptionHandler::instance();
    }
    #endif
    */
}


// ---------------------------------------------------------------------------
// Name:        atInitializeInfrastructureCommand::nameThreadInDebugger
// Description:
//   Name the application's main thread in Visual Studio's thread's list.
//   (We assume that the main application's thread is the thread that uses atInitializeInfrastructureCommand)
// Author:      Merav Zanany
// Date:        29/11/2011
// ---------------------------------------------------------------------------
void atInitializeInfrastructureCommand::nameMainThreadInDebugger()
{
    // Naming threads in a debugger is only supported on Windows:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        // Name the application's main thread in Visual Studio's thread's list:
        osThreadId mainThreadId = osGetCurrentThreadId();
        gtASCIIString mainThreadName(AT_STR_MAINT_THREAD_NAME);
        osNameThreadInDebugger(mainThreadId, mainThreadName);
    }
#endif
}
