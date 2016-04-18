//=====================================================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief Class thta catches un-handled exceptions that will cause the profiled application
/// to crash. Reports the crash including its call-stack into our log file
//=====================================================================================

// Win32:
#include <Windows.h>

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osCallsStackReader.h>
#include <AMDTOSWrappers/Include/osExceptionReason.h>
#include <AMDTInterceptor/Interceptor.h>

// Local:
#include "../Common/Logger.h"
#include "UnhandledExceptionHandler.h"

/// The address of the UnhandledExceptionFilter function residing in Kernel32:
static void* stat_pRealSetUnhandledExceptionFilter = NULL;

/// The address of the currently registered top level unhandled exception filter:
static PTOP_LEVEL_EXCEPTION_FILTER stat_pCurrentUnhandledTopLevelExceptionFilter = NULL;


//--------------------------------------------------------------------------
/// Our version of SetUnhandledExceptionFilter, which will be replacing,
/// by interception, the Kernel32.dll SetUnhandledExceptionFilter function
//--------------------------------------------------------------------------
PTOP_LEVEL_EXCEPTION_FILTER WINAPI MySetUnhandledExceptionFilter(_In_ LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
    // Get the currently registered top level exception filter:
    PTOP_LEVEL_EXCEPTION_FILTER retVal = stat_pCurrentUnhandledTopLevelExceptionFilter;

    // Replace it with the new value we got:
    stat_pCurrentUnhandledTopLevelExceptionFilter = lpTopLevelExceptionFilter;

    // Log the fact that someone called SetUnhandledExceptionFilter:
    Log(logDEBUG, "SetUnhandledExceptionFilter was called\n");

    return retVal;
}


//--------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------
UnhandledExceptionHandler::UnhandledExceptionHandler()
{
}

//--------------------------------------------------------------------------
/// Virtual destructor
//--------------------------------------------------------------------------
UnhandledExceptionHandler::~UnhandledExceptionHandler()
{
}

//--------------------------------------------------------------------------
/// Creates the single instance of this class and registers it as the
/// profiled process un-handled exception handler
//--------------------------------------------------------------------------
bool UnhandledExceptionHandler::init()
{
    bool retVal = false;

    // If my single instance was not already created:
    osUnhandledExceptionHandler* pSignleInstance = osUnhandledExceptionHandler::instance();

    if (pSignleInstance == nullptr)
    {
        // Create it:
        pSignleInstance = new UnhandledExceptionHandler;

        if (pSignleInstance != nullptr)
        {
            // Register this instance as the unheldled exception handler:
            osUnhandledExceptionHandler::registerSingleInstance(*pSignleInstance);

            // Hook the Win32 SetUnhandledExceptionFilter:
            retVal = hookSetUnhandledExceptionFilter();
        }
    }

    return retVal;
}


//--------------------------------------------------------------------------
/// Hook the Win32 SetUnhandledExceptionFilter function, to block others
/// (C runtime, application, etc.) from replacing the top level
/// unhandled exceptions filter that this class registers
//--------------------------------------------------------------------------
bool UnhandledExceptionHandler::hookSetUnhandledExceptionFilter()
{
    bool retVal = false;

    // Get a handle to the loaded Kernel32.dll module:
    HMODULE hLoadedKernel32DLL = ::GetModuleHandle("Kernel32.dll");

    if (hLoadedKernel32DLL == NULL)
    {
        Log(logERROR, "Cannot get handle to Kernel32.dll\n");
    }
    else
    {
        // Get the address of SetUnhandledExceptionFilter residing in Kernel32.dll:
        stat_pRealSetUnhandledExceptionFilter = (void*)::GetProcAddress(hLoadedKernel32DLL, "SetUnhandledExceptionFilter");

        if (stat_pRealSetUnhandledExceptionFilter == NULL)
        {
            Log(logERROR, "Cannot get the address of SetUnhandledExceptionFilter in Kernel32.dll\n");
        }
        else
        {
            // Hook to replace Kernel32.dll SetUnhandledExceptionFilter with MySetUnhandledExceptionFilter:
            AMDT::BeginHook();
            AMDT::HookAPICall(&stat_pRealSetUnhandledExceptionFilter, MySetUnhandledExceptionFilter);

            if (AMDT::EndHook() != NO_ERROR)
            {
                Log(logERROR, "Hooking of SetUnhandledExceptionFilter failed\n");
            }
            else
            {
                retVal = true;
            }
        }
    }

    return retVal;
}


//--------------------------------------------------------------------------
/// Is called when an unhandled exception is caught.
/// \param exceptionCode - The exception code.
/// \param pExceptionContext - A pointer to the exception context (cast into void*)
//--------------------------------------------------------------------------
void UnhandledExceptionHandler::onUnhandledException(osExceptionCode exceptionCode, void* pExceptionContext)
{
    Log(logERROR, "Un-handled exception was caught:\n");
    Log(logERROR, "===============================\n");

    // Log the exception reason:
    osExceptionReason exceptionReason = osExceptionCodeToExceptionReason(exceptionCode);
    gtString exceptionReasonAsStr;
    osExceptionReasonToString(exceptionReason, exceptionReasonAsStr);
    Log(logERROR, "Exception reason: %s\n", exceptionReasonAsStr.asASCIICharArray());

    // Get the exception's associated call stack:
    osCallStack exceptionCallStack;
    osCallsStackReader callStackReader;
    bool gotExceptionCallStack = callStackReader.getCallStack(exceptionCallStack, pExceptionContext, false);

    if (gotExceptionCallStack)
    {
        // Translate the exception's call stack to a string:
        gtString callStackStr, callStackTitleStr;
        bool isSpyRelatedCallStack = false;
        exceptionCallStack.asString(callStackTitleStr, callStackStr, isSpyRelatedCallStack, true);

        // Log the exception's call stack:
        Log(logERROR, "Exception call stack:\n");
        Log(logERROR, "%s:\n", callStackStr.asASCIICharArray());
    }
}


