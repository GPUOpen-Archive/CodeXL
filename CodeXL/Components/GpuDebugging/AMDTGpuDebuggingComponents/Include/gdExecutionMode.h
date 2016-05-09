//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdExecutionMode.h
///
//==================================================================================

//------------------------------ gdExecutionMode.h ------------------------------

#ifndef __GDEXECUTIONMODE_H
#define __GDEXECUTIONMODE_H

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>

// Framework:
#include <AMDTApplicationFramework/Include/afIExecutionMode.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

class GD_API gdExecutionMode : public afIExecutionMode
{
public:
    gdExecutionMode();
    virtual ~gdExecutionMode();

    /// Mode name for identification
    virtual gtString modeName();

    /// Execution status relevance
    /// returns true if relevant
    virtual bool IsExecutionStatusRelevant() { return true; }

    /// The name of the action the mode encompasses
    virtual gtString modeActionString();

    /// The action verb the mode encompasses
    virtual gtString modeVerbString();

    /// Mode description for tooltips
    virtual gtString modeDescription();

    /// Perform a startup action. Return true iff the mode support the requested action:
    virtual bool ExecuteStartupAction(afStartupAction action);

    /// Perform a startup action. Return true iff the mode support the requested action:
    virtual bool IsStartupActionSupported(afStartupAction action) {return (action == AF_NO_PROJECT_USER_ACTION_CREATE_NEW_PROJECT_DEBUG);};

    /// Return true iff the execution mode supports remote host scenario for the requested session type:
    virtual bool IsRemoteEnabledForSessionType(const gtString& sessionType);

    /// Execute the command
    virtual void execute(afExecutionCommandId commandId);

    /// Handle the UI update
    virtual void updateUI(afExecutionCommandId commandId, QAction* pAction);

    /// Get the number of session type
    virtual int numberSessionTypes() { return 1; }

    /// Get the name of each session type
    virtual gtString sessionTypeName(int sessionTypeIndex);

    /// Get the icon of each session type
    virtual QPixmap* sessionTypeIcon(int sessionTypeIndex);

    /// Return the index for the requested session type:
    virtual int indexForSessionType(const gtString& sessionType) { (void)(sessionType); return 0;};

    /// return the layout name used for this mode at specific time:
    virtual afMainAppWindow::LayoutFormats layoutFormat() { return afMainAppWindow::LayoutDebug; };

    /// return the project settings path used for this mode
    virtual gtString ProjectSettingsPath() { return AF_STR_LayoutDebug; };

    /// get the properties view message to start the execution of the mode:
    virtual gtString HowToStartModeExecutionMessage();

    /// is the mode enabled at all
    virtual bool isModeEnabled();

    /// set if the mode is enabled. needed since the upper layer (the gw) knows this and hold the gdExecution as a member
    void SetModeEnabled(bool isModeEnabled) { m_isModeEnabled = isModeEnabled; }

protected:
    /// a flag to indicate if the mode is enabled. by default it is
    bool m_isModeEnabled;
};
#endif //__GDEXECUTIONMODE_H

