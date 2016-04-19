//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdApplicationCommands.h
///
//==================================================================================

//------------------------------ gdApplicationCommands.h ------------------------------

#ifndef __GDAPPLICATIONCOMMANDS_H
#define __GDAPPLICATIONCOMMANDS_H

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osMessageBox.h>

// WX:
#include <AMDTAPIClasses/Include/apGenericBreakpoint.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>


// forward-declaration:
class osCallStack;
class acSendErrorReportDialog;
class afApplicationTreeItemData;
class gdDebugApplicationTreeHandler;
class gdMemoryView;
class gdStatisticsPanel;
class gdAPICallsHistoryPanel;
class gdCallStackView;
class gdDebuggedProcessEventsView;
class gdStateVariablesView;
class gdCommandQueuesView;
class gdPerformanceGraphView;
class gdPerformanceDashboardView;
class gdMultiWatchView;
class gdBreakpointsView;
class gdWatchView;
class gdLocalsView;
class acGUILayoutsManager;
class gdDebugApplicationTreeData;

// Common definitions:

// HSA debugging only currently supported on Linux:
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define GD_ALLOW_HSA_DEBUGGING 1
#else
    #define GD_DISALLOW_HSA_DEBUGGING 1
#endif

// ----------------------------------------------------------------------------------
// Class Name:          gdApplicationCommands
// General Description:
//                      Supplies an implementation to application commands.
//                      The class is used for application commands which have a different
//                      implementation
// Author:              Sigal Algranaty
// Creation Date:       7/2/2011
// ----------------------------------------------------------------------------------
class GD_API gdApplicationCommands/* : public afApplicationCommands*/
{
    friend class gdSingletonsDelete;

public:
    virtual ~gdApplicationCommands();

    // The following functions should be implemented for each inherited class of this class:
    virtual bool openBreakpointsDialog() = 0;
    virtual bool isBreakpointsDialogCommandEnabled() = 0;
    virtual void displayOpenCLProgramSourceCode(afApplicationTreeItemData* pProgramItemData);
    virtual void displayOpenGLSLShaderCode(afApplicationTreeItemData* pShaderItemData);
    virtual bool displayImageBufferObject(afApplicationTreeItemData* pItemData, const gtString& itemText) = 0;
    virtual void displayOpenCLQueue(afApplicationTreeItemData* pQueueItemData);
    virtual bool isEnableAllBreakpointsCommandEnabled(bool& isChecked);
    virtual bool enableAllBreakpoints(bool isChecked);

    // Show a message box:
    virtual void showMessageBox(const QString& caption, const QString& message, osMessageBox::osMessageBoxIcon icon = osMessageBox::OS_DISPLAYED_INFO_ICON);

    // Start debugging command:
    virtual bool isAMDOpenCLDevicePresent();
    virtual bool validClVersion();
    virtual void validateDriverAndGPU();
    virtual void buildProcessStopString(afHTMLContent& htmlContent);

    // Accessors for the single instance view objects:
    virtual gdMemoryView* memoryView() = 0;
    virtual gdStatisticsPanel* statisticsPanel() = 0;
    virtual gdAPICallsHistoryPanel* callsHistoryPanel() = 0;
    virtual gdCallStackView* callStackView() = 0;
    virtual gdDebuggedProcessEventsView* debuggedProcessEventsView() = 0;
    virtual gdStateVariablesView* stateVariablesView() = 0;
    virtual gdCommandQueuesView* commandQueuesView() = 0;
    virtual gdMultiWatchView* multiWatchView(int viewIndex);
    virtual gdBreakpointsView* breakpointsView();
    virtual gdWatchView* watchView();
    virtual gdLocalsView* localsView();

    // Raise view commands:
    virtual bool raiseStatisticsView();
    virtual bool raiseCommandQueuesView();
    virtual bool raiseMemoryView();
    virtual bool raiseMultiWatchView(gdMultiWatchView* pMultiWatchView);

    // File menu commands:
    virtual void onFileSaveStateVariables();

    // File menu update commands:
    virtual void onUpdateFileSaveStateVariables(bool& isEnabled);

    // Edit menu commands:
    virtual void onEditCopy();
    virtual void onEditFind();
    virtual void onEditFindNext();
    virtual void onEditMarkerNext();
    virtual void onEditMarkerPrev();
    virtual void onEditSelectAll();

    // Edit menu update commands:
    virtual void onUpdateEditCopy(bool& isEnabled);
    virtual void onUpdateEditFind(bool& isEnabled);
    virtual void onUpdateEditFindNext(bool& isEnabled);
    virtual void onUpdateEditMarker(bool& isEnabled);
    virtual void onUpdateEditSelectAll(bool& isEnabled);

    // Debug menu commands:
    virtual void onDebugStart();
    virtual void onDebugFrameStep();
    virtual void onDebugDrawStep();
    virtual void onDebugAPIStep();
    virtual void onDebugStepOver();
    virtual void onDebugStepIn();
    virtual void onDebugStepOut();
    virtual void onDebugBreak();
    virtual void onDebugStopDebugging();

    // Debug menu update commands:
    virtual void onUpdateDebugStart(bool& isEnabled);
    virtual void onUpdateDebugStep(bool& isEnabled);
    virtual void onUpdateDebugStepIn(bool& isEnabled);
    virtual void onUpdateDebugStepOut(bool& isEnabled);
    virtual void onUpdateDebugBreak(bool& isEnabled);
    virtual void onUpdateDebugStopDebugging(bool& isEnabled);

    // Breakpoints menu command handlers:
    virtual void onBreakGeneric(apGenericBreakpointType breakpointType, bool shouldBreak);
    virtual void onBreakDebugOutputSetting();

    // Breakpoints menu update commands:
    virtual void onUpdateBreakGeneric(apGenericBreakpointType breakpointType, bool& isEnabled, bool& isChecked);
    virtual void onUpdateOutputSettingDialog(bool& isEnabled);

    // Debug utilities:
    virtual bool resumeDebugging();
    virtual bool canResumeDebugging(bool allowStart, bool allowProf);

    // Clear current statistics:
    virtual bool clearCurrentStatistics();

    // Watch / Multiwatch:
    virtual bool addWatchVariable(const gtString& watchVariable);
    virtual bool displayMultiwatchVariable(const gtString& watchVariable);

    // Open file at a line with addition source file taken into account:
    virtual bool openFileAtLineWithAdditionSourceDir(const osFilePath& filePath, const gtString& modulePath, int fileLine, int programCounterIndex);

    // Return my single instance:
    static gdApplicationCommands* gdInstance();

    static bool registerGDInstance(gdApplicationCommands* pApplicationCommandsInstance);

    /// Show no source mdi window for a specific source file
    bool ShowNoSourceMdi(osFilePath& filePath, const gtString& modulePath, int lineNumber);

    // Helper functions:
    bool canStepIntoCurrentFunction();

protected:
    // Do not allow the use of my constructor:
    gdApplicationCommands();

public:
    static void cleanupGDInstance(bool deleteInstance);

protected:
    // My single instance:
    static gdApplicationCommands* _pMySingleInstance;
};


#endif //__GDAPPLICATIONCOMMANDS_H

