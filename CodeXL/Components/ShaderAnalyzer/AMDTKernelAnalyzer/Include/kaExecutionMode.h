//------------------------------ kaExecutionMode.h ------------------------------

#ifndef __kaExecutionMode_H
#define __kaExecutionMode_H

// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>

// Framework:
#include <AMDTApplicationFramework/Include/afIExecutionMode.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

class KA_API kaExecutionMode : public afIExecutionMode
{
public:
    kaExecutionMode();
    virtual ~kaExecutionMode();

    // Mode name for identification
    virtual gtString modeName();

    /// Execution status relevance
    /// returns true if relevant
    virtual bool IsExecutionStatusRelevant() { return false; }


    // The name of the action the mode encompasses
    virtual gtString modeActionString();

    // The action verb the mode encompasses
    virtual gtString modeVerbString();

    // Mode description for tooltips
    virtual gtString modeDescription();

    /// Perform a startup action. Return true iff the mode support the requested action:
    virtual bool ExecuteStartupAction(afStartupAction action);

    /// Perform a startup action. Return true iff the mode support the requested action:
    virtual bool IsStartupActionSupported(afStartupAction action);

    // Execute the command
    virtual void execute(afExecutionCommandId commandId);

    // Handle the UI update
    virtual void updateUI(afExecutionCommandId commandId, QAction* pAction);

    // Get the number of session type
    virtual int numberSessionTypes() { return 1; }

    // Get the name of each session type
    virtual gtString sessionTypeName(int sessionTypeIndex);

    // Get the icon of each session type
    virtual QPixmap* sessionTypeIcon(int sessionTypeIndex);

    // Return the index for the requested session type:
    virtual int indexForSessionType(const gtString& sessionType) { (void)(sessionType); return 0;};

    // return the layout name used for this mode at specific time:
    virtual afMainAppWindow::LayoutFormats layoutFormat() { return afMainAppWindow::LayoutKernelAnalyzer; };

    /// return the project settings path used for this mode
    virtual gtString ProjectSettingsPath() { return KA_STR_projectSettingExtensionDisplayName; };

    /// is the mode enabled at all
    virtual bool isModeEnabled();

    /// get the properties view message to start the execution of the mode:
    virtual gtString HowToStartModeExecutionMessage();

    /// Get the toolbar start button text
    virtual void GetToolbarStartButtonText(gtString& buttonText, bool fullString = true) override;
};
#endif //__kaExecutionMode_H

