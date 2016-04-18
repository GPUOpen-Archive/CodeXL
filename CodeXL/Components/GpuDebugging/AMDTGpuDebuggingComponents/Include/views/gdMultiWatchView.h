//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdMultiWatchView.h
///
//==================================================================================

//------------------------------ gdMultiWatchView.h ----------------------------

#ifndef __GDMULTIWATCHVIEW
#define __GDMULTIWATCHVIEW

#include <QtWidgets>

// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImageDataView.h>

// ----------------------------------------------------------------------------------
// Class Name:           GD_API gdMultiWatchView : public gdImageDataView, public apIEventsObserver
// General Description:  Represents a multi watch view - display multi values for a
//                       single variable within a kernel execution
// Author:               Sigal Algranaty
// Creation Date:        23/2/2011
// ----------------------------------------------------------------------------------
class GD_API gdMultiWatchView : public gdImageDataView, public apIEventsObserver
{
    Q_OBJECT

public:
    // Constructor:
    gdMultiWatchView(QWidget* pParent, afProgressBarWrapper* pProgressBar);

    // Destructor:
    ~gdMultiWatchView();

    // View initialization:
    virtual void initialize(QSize& viewSize);

    // Is shown / hidden:
    bool isShown() const {return m_isShown; }
    void setIsShown(bool isShown) {m_isShown = isShown; }

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"MultiWatchView"; };

    // Overrides gdImageDataView:
    virtual acRawFileHandler* getCurrentFilterRawDataHandler(unsigned int index);
    virtual void clearView();
    virtual bool handleDataCellDoubleClick(int canvasId, acImageItem* pImageItem, QPoint posOnImage);

    // Display / update a variable:
    bool displayVariable(const gtString& variableName);
    bool displaySelectedVariable();
    bool updateVariableDisplay();
    bool updateCurrentExecutionMask();
    bool updateVariableAvailabilityMask(const QString& varName);

protected slots:

    void onVariableNameComboSelected(const QString& text);
    virtual void onNoteBookPageChange(int currentPage);


protected:

    // Variable load function:
    bool loadVariable(const gtString& variableName);
    bool loadVariableFile(const gtString& variableName, const osFilePath& variableFile);
    bool updateCurrentWorkItem();


protected:

    // True iff the view is shown in VS:
    bool m_isShown;

    // True iff the debugged process is currently suspended:
    bool m_isDebuggedProcessSuspended;

    // True iff we're in the process of setting the work item, so ignore the changed event:
    bool m_ignoreWorkItemChangedEvent;

    // True iff image view was not shown yet:
    bool m_firstTimeImageIsShown;

    // Raw file handler representing the current execution mask:
    acRawFileHandler* m_pExecutionMaskRawFileHandler;

    // Raw file handler for the current variable's availability mask:
    acRawFileHandler* m_pCurrentVariableAvailabilityMaskRawFileHandler;

    // Holds the current displayed work item:
    int m_currentWorkItem[3];

};

#endif  // __GDMULTIWATCHVIEW
