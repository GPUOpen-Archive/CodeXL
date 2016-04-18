//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStopDebuggingCommand.cpp
///
//==================================================================================

//------------------------------ gdStopDebuggingCommand.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/views/afPropertiesView.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdStopDebuggingCommand.h>

// ---------------------------------------------------------------------------
// Name:        gdStopDebuggingCommand::~gdStopDebuggingCommand
// Description: Destructor.
// Author:      Avi Shapira
// Date:        10/1/2007
// ---------------------------------------------------------------------------
gdStopDebuggingCommand::~gdStopDebuggingCommand()
{
}

// ---------------------------------------------------------------------------
// Name:        gdStopDebuggingCommand::canExecuteSpecificCommand
// Description: Answers the question - can we stop debugging the currently
//              debugged process.
// Author:      Yaki Tebeka
// Date:        1/11/2003
// Implementation Notes:
//   We can stop debugging iff there is currently a debugged process.
// ---------------------------------------------------------------------------
bool gdStopDebuggingCommand::canExecuteSpecificCommand()
{
    bool rc = false;

    // Check if there is already a debugged process:
    bool debuggedProcessExists = gaDebuggedProcessExists();

    rc = debuggedProcessExists;
    return rc;
}


// ---------------------------------------------------------------------------
// Name:        gdStopDebuggingCommand::executeSpecificCommand
// Description: Terminates the debugged process.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
bool gdStopDebuggingCommand::executeSpecificCommand()
{
    bool rc = false;

    // Stop recording
    // Get the new recorder status:
    gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
    bool recording = globalVarsManager.recording();

    if (recording)
    {
        // Stop recording
        gaStopMonitoredFunctionsCallsLogFileRecording();

        // Remove the recoding warning:
        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
        {
            afPropertiesView* pPropertiesView = pApplicationCommands->propertiesView();

            if (pPropertiesView != NULL)
            {
                // Clear the properties view
                pPropertiesView->clearView();
            }
        }
    }

    // Terminate the debugged process:
    rc = gaTerminateDebuggedProcess();

    // Show a message if we failed:
    if (!rc && gaDebuggedProcessExists())
    {
        acMessageBox::instance().critical(AF_STR_ErrorA, GD_STR_ErrorMessageFailedTerminateDebugProcess);
    }

    return rc;
}
