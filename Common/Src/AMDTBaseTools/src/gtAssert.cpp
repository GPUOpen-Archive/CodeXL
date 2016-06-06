//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtAssert.cpp
///
//=====================================================================

//------------------------------ gtAssert.cpp ------------------------------

// Local:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtIAssertionFailureHandler.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Standard C:
#include <stdlib.h>

// C++:
#include <fstream>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // Windows:
    #define WIN32_LEAN_AND_MEAN 1
    #include <Windows.h>
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS

#else // AMDT_BUILD_TARGET
    #error Unknown build target!
#endif

// Holds a list of registered assertion failure handlers:
static gtVector<gtIAssertionFailureHandler*>* stat_pAssertionFailureHandlers = NULL;

// Contains true iff we are currently during assertion failure handling:
static bool stat_isDuringAssertionFailureHandling = false;

// This 10 MB buffer will be freed in extreme cases where the application runs out of memory, but
// needs memory in order to exit safely.
static char* stat_memoryBufferForFreeingwhenNeededToReportCrash = new char[10485760];



// ---------------------------------------------------------------------------
// Name:        gtGetOrCreateAssertionFailureHandlersArray
// Description: Returns a pointer to the assertion failure handlers array.
//              If it does not exist - creates it.
// Author:      AMD Developer Tools Team
// Date:        7/1/2010
// ---------------------------------------------------------------------------
gtVector<gtIAssertionFailureHandler*>* gtGetOrCreateAssertionFailureHandlersArray()
{
    // If the assertion failure handlers array does not yet exit - create it:
    if (stat_pAssertionFailureHandlers == NULL)
    {
        stat_pAssertionFailureHandlers = new gtVector<gtIAssertionFailureHandler*>;
        assert(stat_pAssertionFailureHandlers);
    }

    return stat_pAssertionFailureHandlers;
}


// ---------------------------------------------------------------------------
// Name:        gtDeleteAssertionFailureHandlersArray
// Description: Deletes the assertion failure handlers array.
// Author:      AMD Developer Tools Team
// Date:        1/2/2010
// ---------------------------------------------------------------------------
void gtDeleteAssertionFailureHandlersArray()
{
    if (stat_pAssertionFailureHandlers != NULL)
    {
        delete stat_pAssertionFailureHandlers;
        stat_pAssertionFailureHandlers = NULL;
    }
}


// ---------------------------------------------------------------------------
// Name:        gtRegisterAssertionFailureHandler
// Description: Registers an assertion failure handler to be called when assertion failures occur.
// Arguments:   pAssertionFailureHandler - A pointer to the assertion failure handler to be registered.
// Author:      AMD Developer Tools Team
// Date:        30/8/2005
// ---------------------------------------------------------------------------
void gtRegisterAssertionFailureHandler(gtIAssertionFailureHandler* pAssertionFailureHandler)
{
    // Sanity check:
    assert(pAssertionFailureHandler);

    if (pAssertionFailureHandler != NULL)
    {
        // Get the assertion failure handlers array:
        gtVector<gtIAssertionFailureHandler*>* pAssertionFailureHandlersArray = gtGetOrCreateAssertionFailureHandlersArray();

        if (pAssertionFailureHandlersArray != NULL)
        {
            pAssertionFailureHandlersArray->push_back(pAssertionFailureHandler);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gtUnRegisterAssertionFailureHandler
// Description: Unregisters an assertion failure handler registered by gtRegisterAssertionFailureHandler.
// Author:      AMD Developer Tools Team
// Date:        11/6/2009
// ---------------------------------------------------------------------------
void gtUnRegisterAssertionFailureHandler(gtIAssertionFailureHandler* pAssertionFailureHandler)
{
    // Get the assertion failure handlers array:
    gtVector<gtIAssertionFailureHandler*>* pAssertionFailureHandlersArray = gtGetOrCreateAssertionFailureHandlersArray();

    if (pAssertionFailureHandlersArray != NULL)
    {
        // Find the handler in the vector, then move back all the items after it:
        bool handlerFound = false;
        size_t numberOfHandlers = pAssertionFailureHandlersArray->size();

        for (size_t i = 0; i < numberOfHandlers; i++)
        {
            if ((*pAssertionFailureHandlersArray)[i] == pAssertionFailureHandler)
            {
                // We found the item we want:
                handlerFound = true;
            }
            else if (handlerFound)
            {
                // We already found the handler, move all the rest of them back:
                (*pAssertionFailureHandlersArray)[(i - 1)] = (*pAssertionFailureHandlersArray)[i];
            }
        }

        // If we found the handler, the last two items should be the same now, remove the latter one:
        if (handlerFound)
        {
            pAssertionFailureHandlersArray->pop_back();
        }
        else
        {
            // We tried to remove a non-existent handler:
            assert(handlerFound);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gtTriggerAssertonFailureHandler
// Description: Is called when assertion failures occur.
//              Triggers the registered assertion failure handlers.
// Arguments:   functionName - The name of the function in which the assertion
//                             failure occurred.
//              fileName, lineNumber - The file name and line number where the
//                                     assertion occurred.
//              message - An optional message.
// Author:      AMD Developer Tools Team
// Date:        30/8/2005
// ---------------------------------------------------------------------------
void gtTriggerAssertonFailureHandler(const wchar_t* functionName, const wchar_t* fileName,
                                     int lineNumber, const wchar_t* message)
{
    // The below section is for debugging purposes - leave it commented out!
    /*
        ofstream outputFileStream;
        outputFileStream.open("c:\\temp\\assertToFile.txt", ios_base::out | ios_base::app);
        gtString dbgStr = "Assert in: ";
        dbgStr += functionName;
        dbgStr += " File: ";
        dbgStr += fileName;
        dbgStr += " Line: ";
        dbgStr.appendFormattedString(L"%d", lineNumber);
        dbgStr += " Msg: ";
        dbgStr += message;
        dbgStr += "\n";
        outputFileStream << dbgStr.asCharArray();
        outputFileStream.flush();
        outputFileStream.close();
    */

    // Get the assertion failure handlers array:
    gtVector<gtIAssertionFailureHandler*>* pAssertionFailureHandlersArray = gtGetOrCreateAssertionFailureHandlersArray();

    if (pAssertionFailureHandlersArray != NULL)
    {
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD

        // If there is no registered assertion failure handler:
        if (pAssertionFailureHandlersArray->empty())
        {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            // Use the system's assertion mechanism:
            assert(false);
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
            // In Linux and Mac, also output an error message to the standard error file (usually the console):
            gtString errorMsg = GT_STR_ASSERTION_FAILURE_MSG_PREFIX;
            errorMsg += message;
            errorMsg += GT_STR_ASSERTION_FAILURE_MSG_SUFFIX;
            errorMsg += L" ";
            errorMsg += functionName;
            errorMsg += L" ";
            errorMsg += fileName;
            errorMsg.appendFormattedString(L" line: %d\n", lineNumber);
            perror(errorMsg.asASCIICharArray());
#else // AMDT_BUILD_TARGET
#error Unknown build target!
#endif // AMDT_BUILD_TARGET
        }

#endif // AMDT_BUILD_CONFIGURATION

        // If we are not currently during assertion failure handling:
        // (This test is done to ensure that we don't get an infinite loop when an assertion failure handler
        //  triggers an assertion)
        if (!stat_isDuringAssertionFailureHandling)
        {
            stat_isDuringAssertionFailureHandling = true;

            // Iterate the registered assertion failure handlers:
            size_t numberOfHandlers = pAssertionFailureHandlersArray->size();

            for (size_t i = 0; i < numberOfHandlers; i++)
            {
                // Trigger the current assertion failure handler:
                ((*pAssertionFailureHandlersArray)[i])->onAssertionFailure(functionName, fileName, lineNumber, message);
            }

            stat_isDuringAssertionFailureHandling = false;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gtTriggerAssertonFailureHandler
// Description: none unicode version of the function gtTriggerAssertonFailureHandler
// Author:      AMD Developer Tools Team
// Date:        19/6/2011
// ---------------------------------------------------------------------------
void gtTriggerAssertonFailureHandler(const char* functionName, const char* fileName, int lineNumber, const wchar_t* message)
{
    gtString functionNameStr;
    gtString fileNameStr;
    int functionNameLength = (int)strlen(functionName);
    functionNameStr.fromASCIIString(functionName, functionNameLength);
    int fileNameLength = (int)strlen(fileName);
    fileNameStr.fromASCIIString(fileName, fileNameLength);

    gtTriggerAssertonFailureHandler(functionNameStr.asCharArray(), fileNameStr.asCharArray(), lineNumber, message);
}

// ---------------------------------------------------------------------------
// Name:        gtExitCurrentProcess
// Description: Exits the current process.
// Author:      AMD Developer Tools Team
// Date:        1/2/2010
// ---------------------------------------------------------------------------
void gtExitCurrentProcess()
{
    exit(0);
}

void gtCrashDialog()
{
    // Free the dummy buffer so the dialog can be displayed:
    if (stat_memoryBufferForFreeingwhenNeededToReportCrash != NULL)
    {
        delete[] stat_memoryBufferForFreeingwhenNeededToReportCrash;
        stat_memoryBufferForFreeingwhenNeededToReportCrash = NULL;
    }

    // Create the info strings:
    gtString gCrashTitle(GT_STR_MEMORY_ALLOCATION_FAILURE_USER_TITLE);
    gtString gCrashMessage(GT_STR_MEMORY_ALLOCATION_FAILURE_USER_MESSAGE);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // display the dialogs:
    MessageBox(NULL, gCrashMessage.asCharArray(), gCrashTitle.asCharArray(), MB_OK);
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
#ifdef AMDT_LINUX_HEADLESS_BUILD
    // No display can be expected, just print to stderr:
    ::perror(gCrashMessage.asASCIICharArray());
#else // ndef AMDT_LINUX_HEADLESS_BUILD

    fprintf(stderr, "%s", gCrashTitle.asASCIICharArray());
    fprintf(stderr, "%s", gCrashMessage.asASCIICharArray());
#endif // ndef AMDT_LINUX_HEADLESS_BUILD
#else // AMDT_BUILD_TARGET
#error Unknown build target!
#endif
}

// ---------------------------------------------------------------------------
// Name:        gtUnregsiterAllAssertionFailureHandlers
// Description: remove all registered assertion handlers
// Author:      AMD Developer Tools Team
// Date:        27/7/2013
// ---------------------------------------------------------------------------
void gtUnregsiterAllAssertionFailureHandlers()
{
    gtVector<gtIAssertionFailureHandler*>* pAssertionFailureHandlersArray = gtGetOrCreateAssertionFailureHandlersArray();

    if (pAssertionFailureHandlersArray != NULL)
    {
        pAssertionFailureHandlersArray->clear();
    }
}

// ---------------------------------------------------------------------------
// Name:        gtFreeReservedMemory
// Description: Frees the reserved memory which was allocated at the application startup.
//              This function should only be called in extreme cases where all memory allocations fail,
//              and memory must be allocated for the sake of exiting the application properly.
// Author:      AMD Developer Tools Team
// Date:        30/1/2014
// ---------------------------------------------------------------------------
GT_API void gtFreeReservedMemory()
{
    if (stat_memoryBufferForFreeingwhenNeededToReportCrash != NULL)
    {
        delete[] stat_memoryBufferForFreeingwhenNeededToReportCrash;
        stat_memoryBufferForFreeingwhenNeededToReportCrash = NULL;
    }
}
