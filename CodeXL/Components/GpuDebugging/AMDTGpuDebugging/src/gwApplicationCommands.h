//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwApplicationCommands.h
///
//==================================================================================

//------------------------------ gwApplicationCommands.h ---------------------------

#ifndef __GWGAPPLICATIONCOMMANDS_H
#define __GWGAPPLICATIONCOMMANDS_H

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>

// Local:
#include <AMDTGpuDebugging/Include/gwgDEBuggerAppWrapperDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:              gwApplicationCommands : public gdApplicationCommands
// General Description:     This class is handling application commands for CodeXL
//                          standalone application.
//                          The class contain only commands with different implementations
//                          in standalone and VS package, and that are used from somewhere
//                          else then the application menu
// Author:                  Sigal Algranaty
// Creation Date:           20/7/2011
// ----------------------------------------------------------------------------------
class GW_API gwApplicationCommands : public gdApplicationCommands
{
    friend class gwgDEBuggerAppWrapper;

public:

    virtual ~gwApplicationCommands();

    // The following functions should be implemented for each inherited class of this class:

    // Breakpoints:
    virtual bool openBreakpointsDialog();
    virtual bool isBreakpointsDialogCommandEnabled();
    virtual void displayOpenCLProgramSourceCode(afApplicationTreeItemData* pProgramItemData);
    virtual void displayOpenGLSLShaderCode(afApplicationTreeItemData* pShaderItemData);
    virtual bool displayImageBufferObject(afApplicationTreeItemData* pItemData, const gtString& itemText);

    // Accessors for the single instance view objects:
    virtual gdPropertiesEventObserver* propertiesEventObserver();
    virtual gdMemoryView* memoryView();
    virtual gdStatisticsPanel* statisticsPanel();
    virtual gdAPICallsHistoryPanel* callsHistoryPanel();
    virtual gdCallStackView* callStackView();
    virtual gdDebuggedProcessEventsView* debuggedProcessEventsView();
    virtual gdStateVariablesView* stateVariablesView();
    virtual gdCommandQueuesView* commandQueuesView();
    virtual gdBreakpointsView* breakpointsView();
    virtual gdWatchView* watchView();
    virtual gdLocalsView* localsView();
    virtual gdMultiWatchView* multiWatchView(int viewIndex);
    virtual gdMultiWatchView* multiWatchView2();
    virtual gdMultiWatchView* multiWatchView3();

    // Update UI:
    virtual void updateToolbarCommands();
    virtual void updateToolbars();

    // Raise view commands:
    virtual bool raiseStatisticsView();
    virtual bool raiseCommandQueuesView();
    virtual bool raiseMemoryView();
    virtual bool raiseMultiWatchView(gdMultiWatchView* pMultiWatchView);

protected:
    // Do not allow the use of my constructor:
    gwApplicationCommands();
};


#endif  // __gwgApplicationCommands_H
