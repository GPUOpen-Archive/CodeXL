//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afSourceCodeViewsManager.h
///
//==================================================================================

#ifndef __AFSOURCECODEVIEWSMANAGER_H
#define __AFSOURCECODEVIEWSMANAGER_H

// Qt:
#include <QWidget>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/Events/apKernelSourceBreakpointsUpdatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramDeletedEvent.h>

// Forward declaration:
class afSourceCodeView;
class afQMdiSubWindow;

// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          afSourceCodeViewsManager : public apIEventsObserver
// General Description: An object used for managing the creation and update of the
//                      application source code windows
// Author:               Sigal Algranaty
// Creation Date:        18/8/2011
// ----------------------------------------------------------------------------------
class AF_API afSourceCodeViewsManager : public apIEventsObserver
{
public:

    virtual ~afSourceCodeViewsManager();

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"SourceCodeViewsManager"; };

    // Static instance function:
    static afSourceCodeViewsManager& instance();

    // Events:
    void onSubWindowClose(afQMdiSubWindow* pSubWindowAboutToBeClosed);

    // Get of create source code windows by file type and item data:
    afSourceCodeView* getSourceCodeWindow(const osFilePath& sourceCodeFilePath, int lineNumber, int programCounterIndex, QWidget* pParent);
    afSourceCodeView* getExistingView(const osFilePath& filePath, int& viewIndex);
    void removeSourceCodeView(afSourceCodeView* pSourceCodeView);

    // Find the current line number and program counter index for the requested file path:
    bool getLineNumberAndProgramCounter(const osFilePath& filePath, int& lineNumber, int& programCounterIndex);
    void setLineNumberAndProgramCounter(const osFilePath& filePath, int lineNumber, int programCounterIndex);

    // Checks if a file is currently opened:
    bool isFileOpen(const osFilePath& filePath);

    // White spaces:
    void setViewLineNumbers(bool showLineNumbers);
    bool showLineNumbers() const {return _showLineNumbers;};

private:
    // Only afSingletonsDelete can delete my instance:
    friend class afSingletonsDelete;

protected:

    // Do not allow the use of my constructor:
    afSourceCodeViewsManager();

    // Utilities:
    void clearProgramCounters();
    void onProgramStatusChanged(int contextId, int programId, bool programDeleted);
    void onKernelSourceCodeUpdate(const apKernelSourceBreakpointsUpdatedEvent& eve);
    void bindExistingBreakpoints();
    void onProcessTerminate();

protected:
    typedef std::pair<int, int> afSourceLineAndPC;

    static afSourceCodeViewsManager* _pMySingleInstance;

    // Vector containing the displayed source code views:
    gtVector<afSourceCodeView*> _displayedSourceCodeViewsVector;

    // A mapping for the current line numbers and pc index for source code files:
    gtMap<gtString, afSourceLineAndPC > _sourceCodeFilesPCLineNumbers;

    // True iff the source code views should show white spaces:
    bool _showLineNumbers;
};


#endif //__afSourceCodeViewsManager_H

