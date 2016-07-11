//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdApplicationCommands.cpp
///
//==================================================================================

//------------------------------ gdApplicationCommands.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QtWidgets>

// moved here because of undef of bool
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTAPIClasses/Include/apAIDFunctions.h>
#include <AMDTAPIClasses/Include/apCLDevice.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>
#include <AMDTAPIClasses/Include/apKernelDebuggingCommand.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointsUpdatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFileLauncher.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/commands/afSystemInformationCommand.h>
#include <AMDTApplicationFramework/Include/views/afPropertiesView.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdAPICallsHistoryPanel.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdFlushTexturesImagesCommand.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdSaveVarStateCommand.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdStartDebuggingCommand.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdStopDebuggingCommand.h>
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdGLDebugOutputSettingsDialog.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdAPICallsHistoryView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdCallStackView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdDebuggedProcessEventsView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMultiWatchView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStateVariablesView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdWatchView.h>

// Static member initialization:
gdApplicationCommands* gdApplicationCommands::_pMySingleInstance = NULL;

#define GD_VALID_BUILD_NUMBER 793

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::gdApplicationCommands
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
gdApplicationCommands::gdApplicationCommands()
{

}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::~gdApplicationCommands
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
gdApplicationCommands::~gdApplicationCommands()
{

}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::gdInstance
// Description: Return my single instance
// Return Val:  gdApplicationCommands*
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
gdApplicationCommands* gdApplicationCommands::gdInstance()
{
    return _pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::registerInstance
// Description: Register my single instance
// Arguments:   gdApplicationCommands* pApplicationCommandsInstance
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
bool gdApplicationCommands::registerGDInstance(gdApplicationCommands* pApplicationCommandsInstance)
{
    bool retVal = false;

    // Do not allow multiple registration for my instance:
    GT_IF_WITH_ASSERT(_pMySingleInstance == NULL)
    {
        _pMySingleInstance = pApplicationCommandsInstance;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::cleanupGDInstance
// Description: Cleans up the instance member. Allows avoiding the deletion
//              if the implementing class is deleted otherwise
// Author:      Uri Shomroni
// Date:        12/9/2012
// ---------------------------------------------------------------------------
void gdApplicationCommands::cleanupGDInstance(bool deleteInstance)
{
    if (NULL != _pMySingleInstance)
    {
        static bool onlyOnce = true;
        GT_ASSERT(onlyOnce);
        onlyOnce = false;

        if (deleteInstance)
        {
            delete _pMySingleInstance;
        }

        _pMySingleInstance = NULL;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::isEnableAllBreakpointsCommandEnabled
// Description: Check if "Enable All Breakpoints" command is enabled
// Arguments:   bool& isChecked
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
bool gdApplicationCommands::isEnableAllBreakpointsCommandEnabled(bool& isChecked)
{
    bool retVal = true;

    // Get the "Enable all breakpoints" status from the Infra:
    gaGetEnableAllBreakpointsStatus(isChecked, retVal);

    // Get current execution mode:
    apExecutionMode currentExecMode = AP_DEBUGGING_MODE;
    gaGetDebuggedProcessExecutionMode(currentExecMode);

    // Disable event for profiling mode:
    if (currentExecMode == AP_PROFILING_MODE)
    {
        retVal = false;
    }

    return retVal;

}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::enableAllBreakpoints
// Description: Executes "Enable All Breakpoints" command
// Arguments:   bool isChecked
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
bool gdApplicationCommands::enableAllBreakpoints(bool isChecked)
{
    bool retVal = false;

    // Get the amount of breakpoints:
    int amountOfBreakpoints = 0;
    bool rc = gaGetAmountOfBreakpoints(amountOfBreakpoints);
    GT_IF_WITH_ASSERT(rc)
    {
        // An array to hold current breakpoints' data.
        apBreakPoint** pBreakpointsArray = new apBreakPoint*[amountOfBreakpoints];

        // Iterate on the active breakpoints
        for (int i = 0; i < amountOfBreakpoints; i++)
        {
            // Get the current breakpoint:
            gtAutoPtr<apBreakPoint> aptrBreakpoint;
            rc = gaGetBreakpoint(i, aptrBreakpoint);
            GT_IF_WITH_ASSERT(rc)
            {
                // Set the current breakpoint:
                pBreakpointsArray[i] = aptrBreakpoint.releasePointedObjectOwnership();

                // change all the breakpoints' statuses to the new state:
                pBreakpointsArray[i]->setEnableStatus(isChecked);
            }
        }

        // Remove all breakpoints, and add them one by one again with the new enable status:
        rc = gaRemoveAllBreakpoints();
        GT_IF_WITH_ASSERT(rc)
        {
            retVal = true;

            for (int i = 0; i < amountOfBreakpoints; i++)
            {
                // Get the current breakpoint:
                apBreakPoint* pBreakpoint = pBreakpointsArray[i];
                GT_IF_WITH_ASSERT(pBreakpoint != NULL)
                {
                    rc = gaSetBreakpoint(*pBreakpoint);
                    retVal = retVal && rc;
                }
            }
        }
    }

    // Trigger breakpoints update event:
    // -1 - all the breakpoints are updated:
    apBreakpointsUpdatedEvent eve(-1);
    apEventsHandler::instance().registerPendingDebugEvent(eve);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onFileSaveStateVariables
//
// Description:
//   Is called when the user press the File -> Save state variables command.
//   Saves the current state of the OpenGL state machine variables into a file
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
void gdApplicationCommands::onFileSaveStateVariables()
{
    // Set the default directory to be OS_USER_DOCUMENTS
    osFilePath defaultFilePath(osFilePath::OS_USER_DOCUMENTS);

    // Get the current project name:
    QString defaultFileName = acGTStringToQString(afProjectManager::instance().currentProjectSettings().projectName());
    defaultFileName.append("-");
    defaultFileName.append(GD_STR_stateVarFileSample);

    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(NULL != pApplicationCommands)
    {
        // Open Save Dialog:
        QString selectedFilePath = pApplicationCommands->ShowFileSelectionDialog(GD_STR_saveStateVariablesDialogHeader, defaultFileName, GD_STR_stateVarFileDetails, NULL, true);

        if (!selectedFilePath.isEmpty())
        {
            // Run the Save Variable status command:
            gtString pathStr = acQStringToGTString(selectedFilePath);
            gdSaveVarStateCommand saveVarStateCommand(pathStr);
            bool rc = saveVarStateCommand.execute();

            if (!rc)
            {
                acMessageBox::instance().critical(AF_STR_ErrorA, GD_STR_ErrorMessageSaveStateVariablesFailed, QMessageBox::Ok);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onUpdateFileSaveStateVariables
// Description: Checks the disabled / enable state of the save state variables
//              command
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onUpdateFileSaveStateVariables(bool& isEnabled)
{
    // If there is already a debugged process running:
    isEnabled = gaIsDebuggedProcessSuspended() && gaDebuggedProcessExists();
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onUpdateEditCopy
// Description: Check if the edit command should be enabled
// Arguments:   bool &isEnabled
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onUpdateEditCopy(bool& isEnabled)
{
    // By default the action is not enabled:
    isEnabled = false;
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onUpdateEditCopy
// Description: Check if the find command should be enabled
// Arguments:   bool &isEnabled
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onUpdateEditFind(bool& isEnabled)
{
    // By default the action is not enabled:
    isEnabled = false;
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onUpdateEditFindNext
// Description: Check if the find next command should be enabled
// Arguments:   bool &isEnabled
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onUpdateEditFindNext(bool& isEnabled)
{
    // By default the action is not enabled:
    isEnabled = false;
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onUpdateEditSelectAll
// Description: Update the edit -> select all command
// Arguments:   bool &isEnabled
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onUpdateEditSelectAll(bool& isEnabled)
{
    // By default the action is not enabled:
    isEnabled = false;
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onUpdateEditMarker
// Description:
// Arguments:   bool &isEnabled
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onUpdateEditMarker(bool& isEnabled)
{
    (void)(isEnabled);  // unused
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onEditCopy
// Description: Perform the edit copy command
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onEditCopy()
{
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onEditSelectAll
// Description: Perform the edit select all command
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onEditSelectAll()
{
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onEditFind
// Description: Perform the edit find command
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onEditFind()
{
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onEditFindNext
// Description: Perform the edit find next command
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onEditFindNext()
{
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onEdit_MarkerPrevious
// Description: Goto the previous marker function
// Arguments:   wxCommandEvent& event
// Author:      Avi Shapira
// Date:        1/2/2005
// ---------------------------------------------------------------------------
void gdApplicationCommands::onEditMarkerPrev()
{
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onEditMarkerNext
// Description: Goto the next marker function
// Arguments:   wxCommandEvent& event
// Author:      Avi Shapira
// Date:        1/2/2005
// ---------------------------------------------------------------------------
void gdApplicationCommands::onEditMarkerNext()
{
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onDebugStart
// Description: Start debug command
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onDebugStart()
{
    bool canStart = false;
    onUpdateDebugStart(canStart);

    if (canStart)
    {
        // Start the session in log file:
        osDebugLog::instance().StartSession();

        bool isProjectLoaded = !afProjectManager::instance().currentProjectFilePath().isEmpty();

        if (isProjectLoaded)
        {
            // Resume debugging:
            bool rc = resumeDebugging();
            GT_ASSERT(rc);
        }
        else
        {
            // Display the startup dialog:
            afExecutionModeManager::instance().DisplayStartupDialog();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onDebugFrameStep
// Description: Debug frame step command
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onDebugFrameStep()
{
    bool canStep = false;
    onUpdateDebugStep(canStep);

    if (canStep)
    {
        // Set the draw step flag:
        bool rc = gaBreakOnNextFrame();

        if (rc)
        {
            // Resume debugging:
            rc = resumeDebugging();
            GT_ASSERT(rc);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onDebugDrawStep
// Description: Debug draw step command
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onDebugDrawStep()
{
    bool canStep = false;
    onUpdateDebugStep(canStep);

    if (canStep)
    {
        // Set the draw step flag:
        bool rc = gaBreakOnNextDrawFunctionCall();

        if (rc)
        {
            // Resume debugging:
            rc = resumeDebugging();
            GT_ASSERT(rc);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onDebugStep
// Description: Debug step command
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onDebugAPIStep()
{
    bool canStep = false;
    onUpdateDebugStep(canStep);

    if (canStep)
    {
        bool shouldResume = false;

        // Interpret "step over" as a single step:
        bool rcStp = gaBreakOnNextMonitoredFunctionCall();
        GT_IF_WITH_ASSERT(rcStp)
        {
            if (!gaDebuggedProcessExists())
            {
                onDebugStart();
                shouldResume = false;
            }
            else
            {
                shouldResume = true;
            }
        }

        // After setting up any step conditions, resume the debugged process run:
        if (shouldResume)
        {
            bool rcRes = gaResumeDebuggedProcess();
            GT_ASSERT(rcRes);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onDebugStep
// Description: Debug step command
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onDebugStepOver()
{
    bool canStep = false;
    onUpdateDebugStepOut(canStep);

    if (canStep)
    {
        gdGDebuggerGlobalVariablesManager& globalVars = gdGDebuggerGlobalVariablesManager::instance();
        int chosenThread = globalVars.chosenThread();

        bool shouldResume = false;

        if (gaIsInKernelDebugging())
        {
            // Update the stepping work item:
            gaUpdateKernelSteppingWorkItemToCurrentCoordinate();

            // Interpret "step over" as a kernel step:
            bool rcStp = gaSetKernelDebuggingCommand(AP_KERNEL_STEP_OVER);
            GT_IF_WITH_ASSERT(rcStp)
            {
                shouldResume = true;
            }
        }
        else if (gaIsInHSAKernelBreakpoint())
        {
            // Interpret "step over" as a kernel step:
            bool rcStp = gaHSASetNextDebuggingCommand(AP_KERNEL_STEP_OVER);
            GT_IF_WITH_ASSERT(rcStp)
            {
                shouldResume = true;
            }
        }
        else
        {
            if (gaCanGetHostDebugging())
            {
                osThreadId currentTID = OS_NO_THREAD_ID;
                bool rcThrd = gaGetThreadId(chosenThread, currentTID);
                GT_IF_WITH_ASSERT(rcThrd)
                {
                    // Perform a host step in:
                    bool rcStp = gaHostDebuggerStepOver(currentTID);
                    GT_ASSERT(rcStp);
                }
            }
        }


        // After setting up any step conditions, resume the debugged process run:
        if (shouldResume)
        {
            bool rcRes = gaResumeDebuggedProcess();
            GT_ASSERT(rcRes);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onDebugStepOut
// Description: Handle step out command
// Author:      Sigal Algranaty
// Date:        17/8/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onDebugStepOut()
{
    bool canStep = false;
    onUpdateDebugStepOut(canStep);

    if (canStep)
    {
        gdGDebuggerGlobalVariablesManager& globalVars = gdGDebuggerGlobalVariablesManager::instance();
        int chosenThread = globalVars.chosenThread();

        bool shouldResume = false;

        if (gaIsInKernelDebugging())
        {
            // Update the stepping work item:
            gaUpdateKernelSteppingWorkItemToCurrentCoordinate();

            // Interpret "step out" as a kernel step:
            bool rcStp = gaSetKernelDebuggingCommand(AP_KERNEL_STEP_OUT);
            GT_IF_WITH_ASSERT(rcStp)
            {
                shouldResume = true;
            }
        }
        else if (gaIsInHSAKernelBreakpoint())
        {
            // Interpret "step out" as a kernel step:
            bool rcStp = gaHSASetNextDebuggingCommand(AP_KERNEL_STEP_OUT);
            GT_IF_WITH_ASSERT(rcStp)
            {
                shouldResume = true;
            }
        }
        else
        {
            if (gaCanGetHostDebugging())
            {
                osThreadId currentTID = OS_NO_THREAD_ID;
                bool rcThrd = gaGetThreadId(chosenThread, currentTID);
                GT_IF_WITH_ASSERT(rcThrd)
                {
                    // Perform a host step in:
                    bool rcStp = gaHostDebuggerStepOut(currentTID);
                    GT_ASSERT(rcStp);
                }
            }
            else
            {
                // Interpret "step out" as frame step:
                bool rcStp = gaBreakOnNextFrame();
                GT_IF_WITH_ASSERT(rcStp)
                {
                    shouldResume = true;
                }
            }
        }

        // After setting up any step conditions, resume the debugged process run:
        if (shouldResume)
        {
            bool rcRes = gaResumeDebuggedProcess();
            GT_ASSERT(rcRes);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onDebugStepIn
// Description: Handle step out command
// Author:      Sigal Algranaty
// Date:        17/8/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onDebugStepIn()
{
    bool canStep = false;
    onUpdateDebugStepIn(canStep);

    if (canStep)
    {
        bool shouldResume = false;

        gdGDebuggerGlobalVariablesManager& globalVars = gdGDebuggerGlobalVariablesManager::instance();
        int chosenThread = globalVars.chosenThread();
        bool isKernelThreadChosen = globalVars.isKernelDebuggingThreadChosen();

        if (gaIsInKernelDebugging() && isKernelThreadChosen)
        {
            // Update the stepping work item:
            gaUpdateKernelSteppingWorkItemToCurrentCoordinate();

            // Interpret "step in" as a kernel step:
            bool rcStp = gaSetKernelDebuggingCommand(AP_KERNEL_STEP_IN);
            GT_IF_WITH_ASSERT(rcStp)
            {
                shouldResume = true;
            }
        }
        else if (gaIsInHSAKernelBreakpoint())
        {
            // Interpret "step in" as a kernel step:
            bool rcStp = gaHSASetNextDebuggingCommand(AP_KERNEL_STEP_IN);
            GT_IF_WITH_ASSERT(rcStp)
            {
                shouldResume = true;
            }
        }
        else
        {
            // If we are at an API breakpoint:
            bool APIStepInIssued = false;

            if (!gaIsHostBreakPoint())
            {
                // Get the breakpoint thread index:
                int bpThreadIdx = -1;
                bool rcBPThd = gaGetBreakpointTriggeringThreadIndex(bpThreadIdx);
                GT_IF_WITH_ASSERT(rcBPThd)
                {
                    // If the API breakpoint thread is selected:
                    if (bpThreadIdx == chosenThread)
                    {
                        bool canStepIn = canStepIntoCurrentFunction();

                        if (canStepIn)
                        {
                            // Perform a "step in":
                            bool rcStp = gaBreakInMonitoredFunctionCall();
                            GT_IF_WITH_ASSERT(rcStp)
                            {
                                APIStepInIssued = true;
                                shouldResume = true;
                            }
                        }
                    }
                }
            }

            // If we didn't perform an API step-in
            if (!APIStepInIssued)
            {
                // Step in should be disabled if we don't have host debugging:
                GT_IF_WITH_ASSERT(gaCanGetHostDebugging())
                {
                    osThreadId currentTID = OS_NO_THREAD_ID;
                    bool rcThrd = gaGetThreadId(chosenThread, currentTID);
                    GT_IF_WITH_ASSERT(rcThrd)
                    {
                        // Perform a host step in:
                        bool rcStp = gaHostDebuggerStepIn(currentTID);
                        GT_ASSERT(rcStp);
                    }
                }
            }
        }

        // After setting up any step conditions, resume the debugged process run:
        if (shouldResume)
        {
            bool rcRes = gaResumeDebuggedProcess();
            GT_ASSERT(rcRes);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onDebugBreak
// Description: Debug break command
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onDebugBreak()
{
    bool canBreak = false;
    onUpdateDebugBreak(canBreak);

    if (canBreak)
    {
        bool rc = gaSuspendDebuggedProcess();
        GT_ASSERT(rc);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onDebugStopDebugging
// Description: Debug stop command
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onDebugStopDebugging()
{
    bool canStop = false;
    onUpdateDebugStopDebugging(canStop);

    if (canStop)
    {
        // End the session in the log file:
        osDebugLog::instance().EndSession();

        // Stop debugging the current debugged process:
        gdStopDebuggingCommand stopDebuggingCmd;
        bool rc = stopDebuggingCmd.execute();
        GT_ASSERT(rc);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onUpdateDebugStart
// Description: Update of the debug start command
// Arguments:   bool &isEnabled
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onUpdateDebugStart(bool& isEnabled)
{
    isEnabled = canResumeDebugging(true, true);
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onUpdateDebugStep
// Description: Update of the debug step commands
// Arguments:   bool &isEnabled
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onUpdateDebugStep(bool& isEnabled)
{
    isEnabled = canResumeDebugging(true, false);
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onUpdateDebugStepIn
// Description: Update of the debug step in command
// Arguments:   bool &isEnabled
// Author:      Sigal Algranaty
// Date:        20/3/2012
// ---------------------------------------------------------------------------
void gdApplicationCommands::onUpdateDebugStepIn(bool& isEnabled)
{
    isEnabled = canResumeDebugging(false, false) && (gaCanGetHostDebugging() || canStepIntoCurrentFunction() || gaIsInKernelDebugging() || gaIsInHSAKernelBreakpoint());
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onUpdateDebugStepOut
// Description: Enable only in kernel debugging
// Arguments:   bool &isEnabled
// Author:      Sigal Algranaty
// Date:        18/8/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onUpdateDebugStepOut(bool& isEnabled)
{
    isEnabled = canResumeDebugging(false, false) && (gaCanGetHostDebugging() || gaIsInKernelDebugging() || gaIsInHSAKernelBreakpoint());
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onUpdateDebugBreak
// Description: Update of the debug break command
// Arguments:   bool &isEnabled
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onUpdateDebugBreak(bool& isEnabled)
{
    if (afExecutionModeManager::instance().isActiveMode(GD_STR_executionMode))
    {
        isEnabled = false;

        // If there is already a debugged process running:
        if (gaDebuggedProcessExists())
        {
            // If the process is not currently suspended
            if (!gaIsDebuggedProcessSuspended())
            {
                isEnabled = true;
            }
        }
    }
    else
    {
        isEnabled = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onUpdateDebugStopDebugging
// Description: Update of the debug stop command
// Arguments:   bool &isEnabled
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onUpdateDebugStopDebugging(bool& isEnabled)
{
    if (afExecutionModeManager::instance().isActiveMode(GD_STR_executionMode))
    {
        gdStopDebuggingCommand stopDebuggingCmd;
        isEnabled = stopDebuggingCmd.canExecute();
    }
    else
    {
        isEnabled = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::resumeDebugging
//
// Description:
//  Is called when the user resume debugging.
//
// Author:      Avi Shapira
// Date:        28/5/2006
// ---------------------------------------------------------------------------
bool gdApplicationCommands::resumeDebugging()
{
    bool rc = false;

    // Check if there is already a debugged process running:
    if (gaDebuggedProcessExists())
    {
        // Resume the debugged process:
        rc = gaResumeDebuggedProcess();
    }
    else
    {
        // Get the current process creation data from the gdGDebuggerGlobalVariablesManager:
        gdGDebuggerGlobalVariablesManager& theStateManager = gdGDebuggerGlobalVariablesManager::instance();

        // Get the process creation data:
        apDebugProjectSettings processCreationData = theStateManager.currentDebugProjectSettings();

        // Update the debug settings with the generic project settings:
        processCreationData.copyFrom(afProjectManager::instance().currentProjectSettings());

        // Set the updated settings to the manager:
        theStateManager.setCurrentDebugProjectSettings(processCreationData);

        // Start debugging the input process:
        gdStartDebuggingCommand startDebuggingCmd(processCreationData);
        rc = startDebuggingCmd.execute();
    }

    return rc;
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::canResumeDebugging
// Description: Returns true iff resumeDebugging() can be called.
// Author:      Uri Shomroni
// Date:        25/11/2014
// ---------------------------------------------------------------------------
bool gdApplicationCommands::canResumeDebugging(bool allowStart, bool allowProf)
{
    bool retVal = false;

    // We only only handle these commands in debug mode:
    if (afExecutionModeManager::instance().isActiveMode(GD_STR_executionMode))
    {
        // We disable resuming while events are being handled:
        if (!apEventsHandler::instance().isDuringEventHandling())
        {
            if (gaDebuggedProcessExists())
            {
                if (gaIsDebuggedProcessSuspended())
                {
                    retVal = true;
                }
            }
            else if (allowStart)
            {
                retVal = true;
            }

            if (retVal && !allowProf)
            {
                // Do not allow in GPUD profiling mode
                apExecutionMode currentExecMode = AP_DEBUGGING_MODE;
                gaGetDebuggedProcessExecutionMode(currentExecMode);

                if (currentExecMode == AP_PROFILING_MODE)
                {
                    retVal = false;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onBreakGeneric
// Description: Generic breakpoint command handler
// Arguments:   apGenericBreakpointType breakpointType - the requested breakpoint type
//              bool shouldBreak - true iff the generic breakpoint is on
// Author:      Sigal Algranaty
// Date:        24/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onBreakGeneric(apGenericBreakpointType breakpointType, bool shouldBreak)
{
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    if (breakpointType == AP_BREAK_ON_DEBUG_OUTPUT)
    {
        gaEnableGLDebugOutputLogging(shouldBreak);
    }

#endif

    // Set / Remove the generic breakpoint:
    if (shouldBreak)
    {
        apGenericBreakpoint breakpoint(breakpointType);
        breakpoint.setEnableStatus(shouldBreak);
        bool rc = gaSetBreakpoint(breakpoint);
        GT_ASSERT(rc);
    }
    else
    {
        bool rc = gaRemoveGenericBreakpoint(breakpointType);
        GT_ASSERT(rc);
    }

    // Trigger breakpoints update event:
    apBreakpointsUpdatedEvent eve(-1);
    apEventsHandler::instance().registerPendingDebugEvent(eve);
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onUpdateBreakGeneric
// Description: Generic breakpoint update commands function
// Arguments:   apGenericBreakpointType breakpointType - the requested breakpoint type
//              bool& isEnabled - should the breakpoint command be enabled
//              bool& isChecked - should the breakpoint command be checked
// Author:      Sigal Algranaty
// Date:        24/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onUpdateBreakGeneric(apGenericBreakpointType breakpointType, bool& isEnabled, bool& isChecked)
{
    // Initialize output:
    isChecked = false;
    isEnabled = true;

    // Get current execution mode:
    apExecutionMode currentExecMode = AP_DEBUGGING_MODE;
    gaGetDebuggedProcessExecutionMode(currentExecMode);

    // Disable menu item for profiling mode:
    if ((currentExecMode == AP_PROFILING_MODE) || ((breakpointType == AP_BREAK_ON_REDUNDANT_STATE_CHANGE) && currentExecMode != AP_ANALYZE_MODE))
    {
        isEnabled = false;
    }

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    if (breakpointType == AP_BREAK_ON_DEBUG_OUTPUT)
    {
        gaGetGLDebugOutputLoggingEnabledStatus(isChecked);
    }
    else
#endif
    {
        // Get the generic breakpoint status from the Infra:
        bool doesExist = false, isBreakPointEnabled;
        gaGetGenericBreakpointStatus(breakpointType, doesExist, isBreakPointEnabled);
        isChecked = doesExist && isBreakPointEnabled;
    }

    // If not in debug mode change to disabled in any case:
    if (!afExecutionModeManager::instance().isActiveMode(GD_STR_executionMode))
    {
        isEnabled = false;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onUpdateOutputSettingDialog
// Description: Update function for the debug output setting dialog event
// Arguments:   bool& isEnabled
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        24/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onUpdateOutputSettingDialog(bool& isEnabled)
{
    isEnabled = false;

    if (afExecutionModeManager::instance().isActiveMode(GD_STR_executionMode))
    {
        // GLDebugOutput is not supported in Profile mode:
        apExecutionMode currentExecMode;
        bool rcExecMode = gaGetDebuggedProcessExecutionMode(currentExecMode);
        GT_IF_WITH_ASSERT(rcExecMode)
        {
            if (currentExecMode != AP_PROFILING_MODE)
            {
                isEnabled = gaIsGLDebugOutputSupported();
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::onBreakDebugOutputSetting
// Description: Handling the debug output settings command
// Author:      Sigal Algranaty
// Date:        24/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::onBreakDebugOutputSetting()
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(NULL != pApplicationCommands)
    {
        afMainAppWindow* pMainWindow = afMainAppWindow::instance();
        gdGLDebugOutputSettingsDialog dialog(pMainWindow);
        dialog.exec();
    }
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    GT_ASSERT(false);
#else
#error Unsupported platform...
#endif
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::clearCurrentStatistics
// Description: Clears current statistics both in the API, and in the statistics viewer
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/8/2008
// ---------------------------------------------------------------------------
bool gdApplicationCommands::clearCurrentStatistics()
{
    bool retVal = false;

    retVal = gaClearFunctionCallsStatistics();

    // TO_DO: CodeXL frame - clear the view!
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::displayOpenCLProgramSourceCode
// Description:
// Arguments:   gdDebugApplicationTreeData* pProgramItemData
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/7/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::displayOpenCLProgramSourceCode(afApplicationTreeItemData* pProgramItemData)
{
    (void)(pProgramItemData);  // unused
    GT_ASSERT_EX(false, L"Implement me!");
}

void gdApplicationCommands::displayOpenGLSLShaderCode(afApplicationTreeItemData* pShaderItemData)
{
    (void)(pShaderItemData);  // unused
    GT_ASSERT_EX(false, L"Implement me!");
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::displayOpenCLQueue
// Description: Display an OpenCL command queue
// Arguments:   gdDebugApplicationTreeData* pQueueItemData
// Author:      Sigal Algranaty
// Date:        4/1/2012
// ---------------------------------------------------------------------------
void gdApplicationCommands::displayOpenCLQueue(afApplicationTreeItemData* pQueueItemData)
{
    // Sanity check
    GT_IF_WITH_ASSERT(pQueueItemData != NULL)
    {
        // Get the command queues view:
        /*      gdCommandQueuesView* pCommandQueuesView = commandQueuesView();
        GT_IF_WITH_ASSERT(pCommandQueuesView != NULL)
        {
        // Dispaly the command queue in the command queues view:
        pCommandQueuesView->displayCommandQueue(pQueueItemData);

        // Raise the view:
        raiseCommandQueuesView();
        } */
    }
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::raiseStatisticsView
// Description: Raise statistics view - should be implemented in child classes
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        31/7/2011
// ---------------------------------------------------------------------------
bool gdApplicationCommands::raiseStatisticsView()
{
    bool retVal = false;
    GT_ASSERT_EX(false, L"We should not get here");
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::raiseCommandQueuesView
// Description: Raise command queues view - should be implemented in child classes
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/1/2012
// ---------------------------------------------------------------------------
bool gdApplicationCommands::raiseCommandQueuesView()
{
    bool retVal = false;
    GT_ASSERT_EX(false, L"We should not get here");
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::raiseMemoryView(
// Description: Raise memory view - should be implemented in child classes
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        31/7/2011
// ---------------------------------------------------------------------------
bool gdApplicationCommands::raiseMemoryView()
{
    bool retVal = false;
    GT_ASSERT_EX(false, L"We should not get here");
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::raiseMultiWatchView
// Description: Raise the requested multiwatch view
// Arguments:   gdMultiWatchView* pMultiWatchView
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/9/2011
// ---------------------------------------------------------------------------
bool gdApplicationCommands::raiseMultiWatchView(gdMultiWatchView* pMultiWatchView)
{
    (void)(pMultiWatchView);  // unused
    bool retVal = false;
    GT_ASSERT_EX(false, L"We should not get here");
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::showMessageBox
// Description: Shows a default (WX) message box
// Author:      Sigal Algranaty
// Date:        30/8/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::showMessageBox(const QString& caption, const QString& message, osMessageBox::osMessageBoxIcon icon)
{
    switch (icon)
    {
        case osMessageBox::OS_EXCLAMATION_POINT_ICON:
        {
            acMessageBox::instance().warning(caption, message, QMessageBox::Ok);
        }
        break;

        case osMessageBox::OS_QUESTION_MARK_ICON:
        {
            acMessageBox::instance().question(caption, message, QMessageBox::Ok);
        }
        break;

        case osMessageBox::OS_STOP_SIGN_ICON:
        {
            acMessageBox::instance().critical(caption, message, QMessageBox::Ok);
        }
        break;

        case osMessageBox::OS_DISPLAYED_INFO_ICON:
        default:
        {
            QMessageBox::information(NULL, caption, message, QMessageBox::Ok);
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::isAMDOpenCLDevicePresent
// Description: Returns true iff the machine contains an AMD OpenCL device
// Author:      Uri Shomroni
// Date:        13/3/2012
// ---------------------------------------------------------------------------
bool gdApplicationCommands::isAMDOpenCLDevicePresent()
{
    // Get the CL devices:
    bool retVal = false;
    afSystemInformationCommand sysInfoCmd;
    gtPtrVector<apCLDevice*> devicesList;
    bool rcGetDevices = sysInfoCmd.CollectOpenCLDevicesInformation(devicesList, true);
    GT_IF_WITH_ASSERT(rcGetDevices)
    {
        int devicesAmount = devicesList.size();

        for (int i = 0; i < devicesAmount; i++)
        {
            // Get the current device:
            const apCLDevice* pCurrentDevice = devicesList[i];
            GT_IF_WITH_ASSERT(pCurrentDevice != NULL)
            {
                gtString vendorName = pCurrentDevice->deviceVendor();

                if (vendorName == GD_STR_driverVendorName)
                {
                    retVal = true;
                }
            }
        }
    }

    // Clean up:
    devicesList.deleteElementsAndClear();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::validClVersion
// Description: validate correct cl version exist
// Author:      Gilad Yarnitzky
// Date:        24/5/2011
// ---------------------------------------------------------------------------
bool gdApplicationCommands::validClVersion()
{
    bool foundValidCLVersion = false;

    // Find the cl platform information
    afSystemInformationCommand sysInfoCmd;
    gtList <bool> vendorValidList;
    gtList< gtList <gtString> > openCLPlatformsInfoData;
    bool rcOCLPlatform = sysInfoCmd.collectOpenCLPlatformsInformation(openCLPlatformsInfoData, true);

    // Do not assert for machine with no OpenCL installed:
    if (rcOCLPlatform)
    {
        gtList< gtList <gtString> >::const_iterator linesIterator = openCLPlatformsInfoData.begin();

        while (linesIterator != openCLPlatformsInfoData.end())
        {
            // Get the current line:
            const gtList <gtString>& currentLine = *linesIterator++;

            // Iterate the current line columns:
            gtList <gtString>::const_iterator columnsIterator = currentLine.begin();
            const gtString& currentString = *columnsIterator;

            // check if it is the "vendor name":
            if (currentString == AF_STR_SystemInformationCommandVendor)
            {
                // Push an item to sync between this list and openCLPlatformsInfoData:
                vendorValidList.push_back(false);

                // Pass through vendor list and find AMD platform:
                while (++columnsIterator != currentLine.end())
                {
                    const gtString& currentString1 = *columnsIterator;
                    vendorValidList.push_back(currentString1 == GD_STR_driverVendorName);
                }
            }
            else if (currentString == AF_STR_SystemInformationCommandVersion)
            {
                gtList <bool>::const_iterator vendorIterator = vendorValidList.begin();
                columnsIterator++;
                vendorIterator++;

                // Pass through version and check for AMD platform the version:
                while (columnsIterator != currentLine.end())
                {
                    // If the vendor is AMD parse the version string:
                    if (*vendorIterator)
                    {
                        const gtString& currentString1 = *columnsIterator;

                        // Find the AMD-APP string to mark build location start:
                        static const gtString oclLookForString = GD_STR_openclLookFor;
                        int buildStart = currentString1.find(oclLookForString);

                        if (buildStart != -1)
                        {
                            static const int oclLookForStringLen = oclLookForString.length();
                            // Start looking after the string we looked for:
                            buildStart += oclLookForStringLen;

                            // Find the starting parenthesis that mark the build number
                            buildStart = currentString1.find('(', buildStart);

                            if (buildStart != -1)
                            {
                                // Find the first digit of build number start:
                                // search for the first digit of the build
                                while ((currentString1[buildStart] < '0' || currentString1[buildStart] > '9') && (buildStart < currentString1.length()))
                                {
                                    buildStart++;
                                }

                                // if we found a start of build number look for the end of it
                                if (buildStart < currentString1.length())
                                {
                                    int buildEnd = buildStart;

                                    while (((currentString1[buildEnd] >= '0') && (currentString1[buildEnd] <= '9')) && (buildEnd < currentString1.length()))
                                    {
                                        buildEnd++;
                                    }

                                    // check the version
                                    if (buildEnd > buildStart + 1)
                                    {
                                        gtString buildString;
                                        currentString1.getSubString(buildStart, buildEnd - 1, buildString);

                                        // convert the string to int
                                        int buildNumber = -1;
                                        buildString.toIntNumber(buildNumber);

                                        if (buildNumber >= GD_VALID_BUILD_NUMBER)
                                        {
                                            foundValidCLVersion = true;
                                        }
                                    }
                                }
                            }
                        }

                        // Check if this is an internal version and if so it is also valid
                        if (currentString1.find(GD_STR_openclInternalVersion) != -1)
                        {
                            foundValidCLVersion = true;
                        }
                    }

                    columnsIterator++;
                    vendorIterator++;
                }
            }
        }
    }

    return foundValidCLVersion;
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::validateDriverAndGPU
// Description: validate correct driver is installed and there is a ATI GPU
// Author:      Gilad Yarnitzky
// Date:        5/5/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::validateDriverAndGPU()
{
    QString userMessage;
    bool notifyUser = false;

    bool foundDevice = isAMDOpenCLDevicePresent();

    bool findClVersion = validClVersion();

    // Notify user he is missing AMD and project is openCL:
    if (!foundDevice)
    {
        userMessage = GD_STR_driverNoAMDWarning;
        notifyUser = true;
    }
    else if (!findClVersion)
    {
        userMessage = GD_STR_driverVeryOldDriverWarning;
        notifyUser = true;
    }

    // HSA currently only supported on Linux:
#ifdef GD_ALLOW_HSA_DEBUGGING

    if (notifyUser)
    {
        // Ignore OpenCL warnings if we are debugging HSA:
        notifyUser = !gdGDebuggerGlobalVariablesManager::instance().currentDebugProjectSettings().shouldDebugHSAKernels();
    }

#elif defined (GD_DISALLOW_HSA_DEBUGGING)
#else
#error GD_ALLOW_HSA_DEBUGGING and GD_DISALLOW_HSA_DEBUGGING both not defined. Please include gdApplicationCommands.h!
#endif // GD_ALLOW_HSA_DEBUGGING

    // If there is a user message because of a problem show it

    if (notifyUser)
    {
        showMessageBox(AF_STR_WarningA, userMessage, osMessageBox::OS_EXCLAMATION_POINT_ICON);
    }

    // Block kernel debugging if using wrong opencl version
    gaSetKernelDebuggingEnable(findClVersion);
}
// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::buildProcessStopString
// Description:
// Arguments:   gtString& propertiesInfo
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        30/8/2011
// ---------------------------------------------------------------------------
void gdApplicationCommands::buildProcessStopString(afHTMLContent& htmlContent)
{
    htmlContent.setTitle(AF_STR_PropertiesProcessNotRunning);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, AF_STR_PropertiesViewStartRunningComment);
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::breakpointsView
// Description: Get the application breakpoints view
// Return Val:  gdBreakpointsView*
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
gdBreakpointsView* gdApplicationCommands::breakpointsView()
{
    GT_ASSERT_EX(false, L"Not supposed to get here");
    return NULL;
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::watchView
// Description: Get the application watch view
// Return Val:  gdWatchView*
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
gdWatchView* gdApplicationCommands::watchView()
{
    GT_ASSERT_EX(false, L"Not supposed to get here");
    return NULL;
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::localsView
// Description: Get the application locals view
// Return Val:  gdLocalsView*
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
gdLocalsView* gdApplicationCommands::localsView()
{
    GT_ASSERT_EX(false, L"Not supposed to get here");
    return NULL;
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::multiWatchView
// Description: Get the application multi watch view
// Return Val:  gdMultiWatchView*
// Author:      Sigal Algranaty
// Date:        26/9/2011
// ---------------------------------------------------------------------------
gdMultiWatchView* gdApplicationCommands::multiWatchView(int viewIndex)
{
    (void)(viewIndex);  // unused
    GT_ASSERT_EX(false, L"Not supposed to get here");
    return NULL;
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::displayMultiwatchVariable
// Description: Display a multi watch variable
// Arguments:   const gtString& watchVariable
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/9/2011
// ---------------------------------------------------------------------------
bool gdApplicationCommands::displayMultiwatchVariable(const gtString& watchVariable)
{
    bool retVal = false;

    gdMultiWatchView* pMultiWatchView = multiWatchView(0);
    GT_IF_WITH_ASSERT(pMultiWatchView != NULL)
    {
        retVal = pMultiWatchView->displayVariable(watchVariable);

        // Raise the multiwatch view:
        raiseMultiWatchView(pMultiWatchView);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::addWatchVariable
// Description: Add a watch variable to watch view
// Arguments:   const gtString& watchVariable
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/9/2011
// ---------------------------------------------------------------------------
bool gdApplicationCommands::addWatchVariable(const gtString& watchVariable)
{
    bool retVal = false;

    // Get the watch view instance:
    gdWatchView* pWatchView = watchView();
    GT_IF_WITH_ASSERT(pWatchView != NULL)
    {
        retVal = pWatchView->addWatch(watchVariable);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdApplicationCommands::openFileAtLineWithAdditionSourceDir
// Description: Open a source file but also take into account addition source file directory
//              if it is defined in the project settings
// Author:      Gilad Yarnitzky
// Date:        15/7/2012
// ---------------------------------------------------------------------------
bool gdApplicationCommands::openFileAtLineWithAdditionSourceDir(const osFilePath& filePath, const gtString& modulePath, int fileLine, int programCounterIndex)
{
    bool retVal = false;
    osFilePath filePathToOpen = filePath;

    // check if the file exists in the current location open it regularly:
    if (filePath.exists())
    {
        retVal = afApplicationCommands::instance()->OpenFileAtLine(filePath, fileLine, programCounterIndex);
    }
    else
    {
        // Get the file extension:
        gtString ext;
        filePath.getFileExtension(ext);
        bool sourceFile = ((ext != AF_STR_CodeXMLImageBuffersFilesExtension) && (ext != AF_STR_CpuProfileFileExtension) &&
                           (ext != AF_STR_GpuProfileTraceFileExtension) && (ext != AF_STR_GpuProfileSessionFileExtension) &&
                           (ext != AF_STR_FrameAnalysisTraceFileExtension) && (ext != AF_STR_FrameAnalysisPerfCountersFileExtension));

        if (!sourceFile)
        {
            retVal = afApplicationCommands::instance()->OpenFileAtLine(filePath, fileLine, programCounterIndex);
        }
        else
        {
            // If it is a source file look at addition directory if defined and if the file is source file:
            gtString additionalSourceCodeDir = afProjectManager::instance().currentProjectSettings().SourceFilesDirectories();

            // Mark termination of process that the file was found:
            bool fileExistsOnDisk = false;

            if (!additionalSourceCodeDir.isEmpty())
            {
                fileExistsOnDisk = apLookForFileInAdditionalDirectories(filePath, additionalSourceCodeDir, filePathToOpen);
            }

            if (!fileExistsOnDisk)
            {
                // Check with a new source code root directory
                gtString additionalRootDir = afProjectManager::instance().currentProjectSettings().SourceCodeRootLocation();

                if (!additionalRootDir.isEmpty())
                {
                    fileExistsOnDisk = apLookForFileInAdditionalDirectories(filePath, additionalRootDir, filePathToOpen);
                }
            }

            if (fileExistsOnDisk)
            {
                retVal = afApplicationCommands::instance()->OpenFileAtLine(filePathToOpen, fileLine, programCounterIndex);
            }
            else
            {
                gtVector<osFilePath> openedFiles;
                afApplicationCommands::instance()->GetListOfOpenedWindowsForFile(L"", openedFiles);

                gtString sourceFileName;
                filePath.getFileNameAndExtension(sourceFileName);

                for (auto it: openedFiles)
                {
                    gtString itFileName;
                     
                    it.getFileNameAndExtension(itFileName);

                    if (itFileName == sourceFileName)
                    {
                        retVal = afApplicationCommands::instance()->OpenFileAtLine(it, fileLine, programCounterIndex);
                        break;
                    }
                }
            }
        }
    }

    // If file does not exists or fail to open log it
    if (!retVal)
    {
        gtString logString(L"file does not exists:");
        logString.append(filePath.asString());
        OS_OUTPUT_DEBUG_LOG(logString.asCharArray(), OS_DEBUG_LOG_DEBUG);

        osFilePath nonConstFilePath = filePath;
        retVal = ShowNoSourceMdi(nonConstFilePath, modulePath, fileLine);
    }

    return retVal;
}

bool gdApplicationCommands::ShowNoSourceMdi(osFilePath& filePath, const gtString& modulePath, int lineNumber)
{
    bool retVal = false;
    gtString fileNameAndExt;

    filePath.getFileNameAndExtension(fileNameAndExt);

    if (!fileNameAndExt.isEmpty())
    {
        osFilePath tempPath = afGlobalVariablesManager::instance().logFilesDirectoryPath();

        if (tempPath.exists())
        {
            tempPath.setFileName(acQStringToGTString(AF_STR_HtmlNoSourceFoundFileName));
            tempPath.setFileExtension(AF_STR_htmlFileExtension);

            QFont defaultFont;
            QString htmlFileData;
            if (lineNumber != 1)
            {
                htmlFileData = QString(AF_STR_HtmlFindSourceWebpage).arg(defaultFont.defaultFamily()).arg(acGTStringToQString(fileNameAndExt)).arg(lineNumber);
            }
            else
            {
                htmlFileData = QString(AF_STR_HtmlNoDebugInformationWebpage).arg(defaultFont.defaultFamily()).arg(acGTStringToQString(fileNameAndExt));
            }

            QString fileName(acGTStringToQString(tempPath.asString()));
            QFile fileToWrite(fileName);
            bool rc = fileToWrite.open(QIODevice::WriteOnly | QIODevice::Text);
            GT_IF_WITH_ASSERT(rc)
            {
                QTextStream outStream(&fileToWrite);
                outStream << htmlFileData;
                fileToWrite.close();
            }

            retVal = afApplicationCommands::instance()->OpenFileAtLine(tempPath, lineNumber);
        }
    }
    else
    {
        osFilePath tempPath = afGlobalVariablesManager::instance().logFilesDirectoryPath();

        if (tempPath.exists())
        {
            tempPath.setFileName(acQStringToGTString(AF_STR_HtmlNoSourceFoundFileName));
            tempPath.setFileExtension(AF_STR_htmlFileExtension);

            QString htmlFileData = QString(AF_STR_HtmlNoDebugInformationNoFileWebpage).arg(acGTStringToQString(modulePath));

            QString fileName(acGTStringToQString(tempPath.asString()));
            QFile fileToWrite(fileName);
            bool rc = fileToWrite.open(QIODevice::WriteOnly | QIODevice::Text);
            GT_IF_WITH_ASSERT(rc)
            {
                QTextStream outStream(&fileToWrite);
                outStream << htmlFileData;
                fileToWrite.close();
            }

            retVal = afApplicationCommands::instance()->OpenFileAtLine(tempPath, lineNumber);
        }
    }

    return retVal;
}

bool gdApplicationCommands::canStepIntoCurrentFunction()
{
    bool retVal = false;

    // Get the context:
    apContextID bpCtx;
    bool rcBPCtx = gaGetBreakpointTriggeringContextId(bpCtx);
    GT_IF_WITH_ASSERT(rcBPCtx)
    {
        // Get the breaked on function:
        gtAutoPtr<apFunctionCall> aptrFunctionCall;
        bool rcFunc = gaGetLastFunctionCall(bpCtx, aptrFunctionCall);
        GT_IF_WITH_ASSERT(rcFunc)
        {
            apMonitoredFunctionId funcId = aptrFunctionCall->functionId();

            if ((ap_clEnqueueNDRangeKernel == funcId) || (ap_clEnqueueTask == funcId))
            {
                retVal = true;
            }
        }
    }

    return retVal;
}
