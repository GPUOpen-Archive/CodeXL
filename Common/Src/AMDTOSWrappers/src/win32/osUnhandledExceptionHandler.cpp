//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osUnhandledExceptionHandler.cpp
///
//=====================================================================

//------------------------------ osUnhandledExceptionHandler.cpp ------------------------------

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osWin32DebugSymbolsManager.h>
#include <AMDTOSWrappers/Include/osUnhandledExceptionHandler.h>

// Static members initialization:
osUnhandledExceptionHandler* osUnhandledExceptionHandler::_pMySingleInstance = 0;


// ---------------------------------------------------------------------------
// Name:        osUnhandledExceptionFilter
// Description: Is called when an unhandled exception occur.
// Arguments: pExceptionInfo - A structure representing the unhandled exception.
// Return Val: Always returns EXCEPTION_EXECUTE_HANDLER, asking the system to terminate the process.
// Author:      AMD Developer Tools Team
// Date:        21/4/2009
// ---------------------------------------------------------------------------
LONG osUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* pExceptionInfo)
{
    LONG retVal = EXCEPTION_EXECUTE_HANDLER;

    // Will get the exception code, context and call stack:
    osExceptionCode exceptionCode = 0;
    PCONTEXT pExceptionContext = NULL;

    if (pExceptionInfo != NULL)
    {
        // Get a record describing the exception reason:
        PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;

        if (pExceptionRecord != NULL)
        {
            // Get the exception code:
            exceptionCode = pExceptionRecord->ExceptionCode;
        }

        // Get the execution context in which the unhandled exception occur:
        pExceptionContext =  pExceptionInfo->ContextRecord;
    }

    // Load debug symbols. This will enable us, later on, display a readable crash call stack:
    HANDLE hCurrProcess = GetCurrentProcess();
    osWin32DebugSymbolsManager dbgSymbolMgr;
    dbgSymbolMgr.initializeProcessSymbolsServer(hCurrProcess, true);

    // Report the unhandled exception:
    osUnhandledExceptionHandler* pExceptionHandler = osUnhandledExceptionHandler::instance();
    GT_IF_WITH_ASSERT(pExceptionHandler != NULL)
    {
        pExceptionHandler->onUnhandledException(exceptionCode, pExceptionContext);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osUnhandledExceptionHandler::osUnhandledExceptionHandler
// Description: Constructor.
// Author:      AMD Developer Tools Team
// Date:        21/4/2009
// ---------------------------------------------------------------------------
osUnhandledExceptionHandler::osUnhandledExceptionHandler()
{
}


// ---------------------------------------------------------------------------
// Name:        osUnhandledExceptionHandler::~osUnhandledExceptionHandler
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        21/4/2009
// ---------------------------------------------------------------------------
osUnhandledExceptionHandler::~osUnhandledExceptionHandler()
{
}


// ---------------------------------------------------------------------------
// Name:        osUnhandledExceptionHandler::registerSingleInstance
// Description:
//  Logs the single instance of this class and registeres it as the unhandles
//  exceptions hanlder for the calling process.
// Arguments: singleInstance - The single instance of this class to be registered.
// Author:      AMD Developer Tools Team
// Date:        22/4/2009
// ---------------------------------------------------------------------------
void osUnhandledExceptionHandler::registerSingleInstance(osUnhandledExceptionHandler& singleInstance)
{
    // Log the single instance of this class:
    _pMySingleInstance = &singleInstance;

    // Register osUnhandledExceptionFilter as the unhandles exceptions hanlder for the calling process:
    ::SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&osUnhandledExceptionFilter);
}


// ---------------------------------------------------------------------------
// Name:        osUnhandledExceptionHandler::instance
// Description: Returns the single instance of this, which was registered by
//              registerSingleInstance.
// Author:      AMD Developer Tools Team
// Date:        22/4/2009
// ---------------------------------------------------------------------------
osUnhandledExceptionHandler* osUnhandledExceptionHandler::instance()
{
    return _pMySingleInstance;
};


