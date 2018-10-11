//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afIExecutionMode.h
///
//==================================================================================

#ifndef __AFIEXECUTIONMODE_H
#define __AFIEXECUTIONMODE_H

#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools//Include/gtString.h>

// This if the interface for the execution mode. It is used to supply the execution interface to the execution and the UI of the execution commands
// and for the session types the execution mode can supply.

enum afExecutionCommandId
{
    AF_EXECUTION_ID_START = 0,
    AF_EXECUTION_ID_BREAK,
    AF_EXECUTION_ID_STOP,
    AF_EXECUTION_ID_API_STEP,
    AF_EXECUTION_ID_DRAW_STEP,
    AF_EXECUTION_ID_FRAME_STEP,
    AF_EXECUTION_ID_STEP_IN,
    AF_EXECUTION_ID_STEP_OVER,
    AF_EXECUTION_ID_STEP_OUT,
    AF_EXECUTION_ID_BUILD,
    AF_EXECUTION_ID_CANCEL_BUILD,
    AF_EXECUTION_LAST_COMMAND_ID
};

/// Enumeration used for user selection in case where no project is loaded, and execute button is clicked:
enum afStartupAction
{
    AF_NO_PROJECT_USER_ACTION_CREATE_NEW_PROJECT,
    AF_NO_PROJECT_USER_ACTION_CREATE_NEW_PROJECT_DEBUG,
    AF_NO_PROJECT_USER_ACTION_CREATE_NEW_PROJECT_PROFILE,
    AF_NO_PROJECT_USER_ACTION_NEW_FILE_FOR_ANALYZE,
    AF_NO_PROJECT_USER_ACTION_ADD_FILE_FOR_ANALYZE,
    AF_NO_PROJECT_USER_ACTION_NONE
};

class AF_API afIExecutionMode
{
public:
    afIExecutionMode() {};
    virtual ~afIExecutionMode() {};

    /// Mode name for identification
    virtual gtString modeName() = 0;

    /// Execution status relevance
    /// returns true if relevant
    virtual bool IsExecutionStatusRelevant() = 0;

    /// The name of the action the mode encompasses (e.g. "Debugging", "Profiling", "Analysis", etc.)
    virtual gtString modeActionString() = 0;

    /// The action verb the mode encompasses (e.g. "debug", "profile", "analyze", etc.)
    virtual gtString modeVerbString() = 0;

    /// Mode description for tooltips
    virtual gtString modeDescription() = 0;

    /// Perform a startup action. Return true iff the mode support the requested action:
    virtual bool ExecuteStartupAction(afStartupAction action) = 0;

    /// Perform a startup action. Return true iff the mode support the requested action:
    virtual bool IsStartupActionSupported(afStartupAction action) = 0;

    /// Return true iff the execution mode supports the requested action when no project is loaded:
    virtual bool IsStartupActionSupportedWithNoProject(afExecutionCommandId commandId) { GT_UNREFERENCED_PARAMETER(commandId); return false; };

    /// Return true iff the execution mode supports remote host scenario for the requested session type:
    virtual bool IsRemoteEnabledForSessionType(const gtString& sessionType) { GT_UNREFERENCED_PARAMETER(sessionType); return false; };

    /// Get the name of the selected session type
    virtual gtString selectedSessionTypeName() {return L"";};

    /// Execute the command
    virtual void execute(afExecutionCommandId commandId) = 0;

    /// Handle the UI update
    virtual void updateUI(afExecutionCommandId commandId, QAction* pAction) = 0;

    /// Execute the session type change command
    virtual void execute(int sessionTypeIndex) { (void)(sessionTypeIndex); } ;

    /// Handle the session type UI update
    virtual void updateUI(int sessionTypeIndex, QAction* pAction) { (void)(sessionTypeIndex); (void)(pAction);};

    /// Get the number of session type
    virtual int numberSessionTypes() { return 0; }

    /// Get the name of each session type
    virtual gtString sessionTypeName(int sessionTypeIndex) = 0;

    /// Get the icon of each session type
    virtual QPixmap* sessionTypeIcon(int sessionTypeIndex) = 0;

    /// Return the index for the requested session type:
    virtual int indexForSessionType(const gtString& sessionType) = 0;

    /// return the layout name used for this mode at specific time:
    virtual afMainAppWindow::LayoutFormats layoutFormat() = 0;

    /// return the project settings path used for this mode
    virtual gtString ProjectSettingsPath() = 0;

    /// is the mode enabled at all
    virtual bool isModeEnabled() { return true; };

    /// get the properties view message to start the execution of the mode:
    /// The default is a generic message
    virtual gtString HowToStartModeExecutionMessage() { return AF_STR_PropertiesViewStartRunningComment; }

    /// Allow the mode to terminate gracefully at the end of CodeXL. by default nothing needs to be done
    virtual void Terminate() {};

    /// Get the toolbar start button text
    virtual void GetToolbarStartButtonText(gtString& buttonText, bool fullString = true) = 0;
};
#endif //__AFIEXECUTIONMODE_H

