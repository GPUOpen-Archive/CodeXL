//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ahDialogBasedAssertionFailureHandler.cpp
///
//==================================================================================

// Qt:
#include <QtWidgets>

// Windows:
#include <assert.h>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osMessageBox.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// Local:
#include <inc/ahCommandIDs.h>
#include <AMDTAssertionHandlers/Include/ahAssertDialog.h>
#include <AMDTAssertionHandlers/Include/ahDialogBasedAssertionFailureHandler.h>


// ---------------------------------------------------------------------------
// Name:        ahDialogBasedAssertionFailureHandler::ahDialogBasedAssertionFailureHandler
// Description: Constructor.
// Arguments: mainGUIThreadHandle - The main GUI thread id.
// Author:      Yaki Tebeka
// Date:        22/2/2007
// ---------------------------------------------------------------------------
ahDialogBasedAssertionFailureHandler::ahDialogBasedAssertionFailureHandler(osThreadId mainGUIThreadId)
    : _mainGUIThreadId(mainGUIThreadId)
{
}


// ---------------------------------------------------------------------------
// Name:        ahDialogBasedAssertionFailureHandler::ahDialogBasedAssertionFailureHandler
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        22/2/2007
// ---------------------------------------------------------------------------
ahDialogBasedAssertionFailureHandler::~ahDialogBasedAssertionFailureHandler()
{
    // Clear allocated data:
    _ignoredSourceLocations.deleteElementsAndClear();
}


// ---------------------------------------------------------------------------
// Name:        ahDialogBasedAssertionFailureHandler::onAssertionFailure
// Description: Is called when an assertion failure occur.
// Arguments:   functionName - The name of the function that triggered the assertion
//              fileName, lineNumber - The name of the file and line number that contains
//                                     the code that triggered the assertion.
//              message - An optional assertion message.
// Author:      Yaki Tebeka
// Date:        22/2/2007
// ---------------------------------------------------------------------------
void ahDialogBasedAssertionFailureHandler::onAssertionFailure(const wchar_t* functionName, const wchar_t* fileName, int lineNumber, const wchar_t* message)
{
    // If we should display the assertion failure dialog:
    bool shouldDisplayDlg = shouldDisplayDialog();

    if (shouldDisplayDlg)
    {
        osFilePath filePath;
        filePath.setFullPathFromString(fileName);

        // Display the assertion failure dialog:
        int usersDecision = displayAssertionDialog(functionName, filePath, lineNumber, message);

        // Handle the user's decision:
        handleUsersDecision(usersDecision, filePath, lineNumber);
    }
    else
    {
        // If we are in debug build and we don't display the message box,
        // output an assertion failure message to the standard error file (usually the console):
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
        {
            outputAssertionMessageToStderr(functionName, fileName, lineNumber, message);
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            {
                gtString asrtMsg = L"Assert in: ";
                asrtMsg.append(functionName).append(L"\n\rIn file: ").append(fileName).appendFormattedString(L":%d\n\rAssert Reason: ", lineNumber).append(message);
                osMessageBox msgBox(L"Assertion Failure", asrtMsg, osMessageBox::OS_STOP_SIGN_ICON);
                msgBox.display();
            }
#endif
        }
#endif
    }
}


// ---------------------------------------------------------------------------
// Name:        ahDialogBasedAssertionFailureHandler::shouldDisplayDialog
// Description: Returns true iff we should display the assertion failure
//              dialog to the user.
// Author:      Yaki Tebeka
// Date:        9/7/2007
// ---------------------------------------------------------------------------
bool ahDialogBasedAssertionFailureHandler::shouldDisplayDialog() const
{
    bool retVal = false;

    // The dialog will be displayed only in debug configuration:
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    {
        // Yaki - 9/7/2007:
        // On Linux - the dialog will be displayed only when the
        // assertion is triggered from the main GUI thread.
        // (displaying a message box on wxGTK from a thread different
        //  that the main application thread sometimes crashes the
        //  application)
        // On Windows - calling the dialog from the debugger thread
        //  causes problems, so we call an osMessageBox instead.
        osThreadId currThreadId = osGetCurrentThreadId();

        if (currThreadId == _mainGUIThreadId)
        {
            retVal = true;
        }
    }
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        ahDialogBasedAssertionFailureHandler::isInIgnoredSourceLocations
// Description:
//  Inputs a source location (path + line number) and returns true iff
//  it appears in the "ignored source locations".
//  I.E: The user presses "Ignore all" on this source location.
//
// Author:     s Yaki Tebeka
// Date:        9/7/2007
// ---------------------------------------------------------------------------
bool ahDialogBasedAssertionFailureHandler::isInIgnoredSourceLocations(const osFilePath& filePath, const int lineNumber)
{
    bool retVal = false;

    // Iterate the "ignored source locations":
    gtPtrVector<ahSourceCodeLocation*>::const_iterator iter = _ignoredSourceLocations.begin();
    gtPtrVector<ahSourceCodeLocation*>::const_iterator endIter = _ignoredSourceLocations.end();

    while (iter != endIter)
    {
        // Get the current ignored source location:
        const ahSourceCodeLocation* pCurrIgnoredLoc = *iter;

        if (pCurrIgnoredLoc)
        {
            // If the input source location is the current source location:
            if ((pCurrIgnoredLoc->_filePath == filePath) &&
                (pCurrIgnoredLoc->_lineNumber == lineNumber))
            {
                retVal = true;
            }
        }

        iter++;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        ahDialogBasedAssertionFailureHandler::displayAssertionDialog
// Description:
//   Displays the "Asserting failure" dialog to the user.
//
// Arguments: functionName - The name of the function that triggered the assertion
//            filePath, lineNumber - The file path and line number that contains
//                                   the code that triggered the assertion.
//            message - An optional assertion message.
//
// Return Val: int - The user's decision.
//
// Author:      Yaki Tebeka
// Date:        9/7/2007
// ---------------------------------------------------------------------------
int ahDialogBasedAssertionFailureHandler::displayAssertionDialog(const wchar_t* functionName, const osFilePath& filePath,
                                                                 int lineNumber, const wchar_t* message)
{
    (void)(functionName); // unused
    int usersDecision = ID_AH_ASSERT_IGNORE_BUTTON;

    // If the assertion failure resides in an ignored source location:
    bool isIgnoredSourceLocation = isInIgnoredSourceLocations(filePath, lineNumber);

    if (isIgnoredSourceLocation)
    {
        // Do not display the dialog:
        usersDecision = ID_AH_ASSERT_IGNORE_BUTTON;
    }
    else
    {
        // Display an assert dialog:
        gtString assertReason(message);
        ahAssertDialog assertDlg(filePath, lineNumber, assertReason.asASCIICharArray(), NULL);
        usersDecision = assertDlg.exec();
    }

    return usersDecision;
}


// ---------------------------------------------------------------------------
// Name:        ahDialogBasedAssertionFailureHandler::handleUsersDecision
//
// Description: Handles the user's assertion handling decision.
//
// Arguments: decision - Contains the id of the command that the
//                       user choose in the assertion dialog.
//            filePath, lineNumber - The file path and line number that contains
//                                   the code that triggered the assertion.
//
// Author:      Yaki Tebeka
// Date:        22/2/2007
// ---------------------------------------------------------------------------
void ahDialogBasedAssertionFailureHandler::handleUsersDecision(int decision, const osFilePath& filePath, int lineNumber)
{
    switch (decision)
    {
        case ID_AH_ASSERT_IGNORE_BUTTON:
        {
            // The user wishes to ignore this assertion failure.
        }
        break;

        case ID_AH_ASSERT_IGNORE_ALL_BUTTON:
        {
            // Add the current source location to the ignored source locations:
            ahSourceCodeLocation* pNewIgnoredSourceLocation = new ahSourceCodeLocation;

            if (pNewIgnoredSourceLocation != NULL)
            {
                pNewIgnoredSourceLocation->_filePath = filePath;
                pNewIgnoredSourceLocation->_lineNumber = lineNumber;
                _ignoredSourceLocations.push_back(pNewIgnoredSourceLocation);
            }
        }
        break;

        case ID_AH_ASSERT_OPEN_BUTTON:
        {
            // The user asked to open the file in an editor:
            osOpenFileInSourceCodeEditor(filePath, lineNumber);
        }
        break;

        case ID_AH_ASSERT_DEBUG_BUTTON:
        {
            // Trow a breakpoint exception. This should raise a dialog asking the user
            // if we want to debug this process:
            osThrowBreakpointException();
        }
        break;

        case ID_AH_ASSERT_EXIT_BUTTON:
        {
            // Exit this process:
            exit(0);
        }

        default:
        {
            // Unknown user decision:
            // To prevent an infinite loop, trigger the system's assertion handler:
            assert(false);
        }
        break;
    }
}



// ---------------------------------------------------------------------------
// Name:        ahDialogBasedAssertionFailureHandler::outputAssertionMessageToStderr
// Description:
//   Outputs an "Asserting failure" message to the standard output file
//   (which is usually the  console).
//
// Arguments: functionName - The name of the function that triggered the assertion
//              fileName, lineNumber - The name of the file and line number that contains
//                                     the code that triggered the assertion.
//              message - An optional assertion message.
// Author:      Yaki Tebeka
// Date:        9/7/2007
// ---------------------------------------------------------------------------
void ahDialogBasedAssertionFailureHandler::outputAssertionMessageToStderr(const wchar_t* functionName, const wchar_t* fileName,
        int lineNumber, const wchar_t* message)
{
    // Get the current time:
    osTime currentTime;
    currentTime.setFromCurrentTime();
    gtString timeAsString;
    currentTime.timeAsString(timeAsString, osTime::WINDOWS_STYLE, osTime::LOCAL);

    // Build the error message:
    gtString errorMsg = timeAsString;
    errorMsg += L" - Assertion failure: ";
    errorMsg += message;
    errorMsg += L" ";
    errorMsg += functionName;
    errorMsg += L" ";
    errorMsg += fileName;
    errorMsg.appendFormattedString(L" line %d\r\n", lineNumber);

    // Output the error message to the standard error file:
    osWPerror(errorMsg.asCharArray());
}


