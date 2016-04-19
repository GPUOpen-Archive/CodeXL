//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdAPICallsHistoryView.h
///
//==================================================================================

//------------------------------ gdAPICallsHistoryView.h ------------------------------

#ifndef __GDAPICALLSHISTORYVIEW
#define __GDAPICALLSHISTORYVIEW

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apSearchDirection.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acVirtualListCtrl.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>
#include <AMDTApplicationFramework/Include/afHTMLContent.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>


// Forward declarations:
class afGlobalVariableChangedEvent;
class apBreakpointHitEvent;
class apMonitoredObjectsTreeSelectedEvent;
class apStatistics;
class afProgressBarWrapper;
class gdAPICallsHistoryView;

// ----------------------------------------------------------------------------------
// Class Name:          AC_API QAbstractTableModel : public QAbstractTableModel
// General Description: A class used for providing a data model for virtual list
// Author:              Sigal Algranaty
// Creation Date:       25/12/2011
// ----------------------------------------------------------------------------------
class GD_API gdAPICallsHistoryViewModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    // Constructor:
    gdAPICallsHistoryViewModel(gdAPICallsHistoryView* pParent);

    // Destructor:
    ~gdAPICallsHistoryViewModel();

    void updateModel();

    // When inheriting the model, implement these member functions, to provide the model functionality:
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex())const;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;

protected:

    // A pointer to the call history view:
    gdAPICallsHistoryView* _pAPICallsHistoryView;

};


// ----------------------------------------------------------------------------------
// Class Name:          gdAPICallsHistoryView: public acVirtualListCtrl , public afBaseView, public apIEventsObserver
// General Description: The view lists a log of the API function calls.
//                      The view is implemented as a virtual list control, this improves performance
//  dramatically when dealing with huge lists.
// Author:              Yaki Tebeka
// Creation Date:       1/11/2003
// ----------------------------------------------------------------------------------
class GD_API gdAPICallsHistoryView: public acVirtualListCtrl , public afBaseView, public apIEventsObserver
{
    Q_OBJECT

public:
    friend class gdAPICallsHistoryViewModel;
    gdAPICallsHistoryView(afProgressBarWrapper* pProgressBar, QWidget* pParent, bool isGlobal, bool shouldSetCaption);

    virtual ~gdAPICallsHistoryView();

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"APICallsHistoryView"; };

    // gdStatisticsViewBase override:
    virtual bool updateFunctionCallsStatisticsList(const apStatistics& currentStatistics);

    // Return the icon type according to an icon index:
    afIconType functionIconType(int iconIndex);

    // Initializes view:
    void initAfterCreation();

    // Sets active context:
    void setActiveContext(apContextID activeContext) {_activeContextId = activeContext;}

    // Set is suspended:
    void setIsSuspended(bool isDebuggedProcessSuspended) {_isDebuggedProcessSuspended = isDebuggedProcessSuspended;};

    bool findNextMarker(apSearchDirection searchDirection);
    bool canFindMarker();
    void clearList();

    // Edit menu commands
    virtual void onUpdateSelectAll(bool& isEnabled);
    virtual void onCopy();
    virtual void onSelectAll();
    virtual void onEdit_Find();
    virtual void onEdit_FindNext();

    void onUpdateEnableAllBreakpoints(bool& isEnabled, bool& isChecked);
    void onUpdateBreakpOnFunction(bool& isEnabled, bool& isChecked, QString& itemText);
    void onUpdateAddBreakpoints(bool& isEnabled);

public slots:
    virtual void onRowSelected(const QModelIndex& index);

protected slots:
    virtual void onCallsHistoryItemClicked(const QModelIndex& index);
    virtual void onBreakOnFunction();
    virtual void onAddBreakpoints();
    virtual void onEnableAllBreakpoints();
    virtual void onAboutToShowContextMenu();

protected:
    virtual void initializeListIcons();

    // Overrides QWidget:
    virtual void resizeEvent(QResizeEvent* pResizeEvent);

    int iconByFunctionType(int functionID) const;
    QIcon* getItemIcon(long item) const;

    bool isCurrentFunctionExistAsBreakpoint(QString& funcName, apMonitoredFunctionId& funcId, int& breakpointId);
    void initialize();
    // Debugged process events:
    void onProcessCreated();
    void onProcessTerminated();
    void onProcessRunSuspended(const apEvent& event);
    void onProcessRunResumed();
    void onBreakpointHit(const apBreakpointHitEvent& event);
    void onTreeItemSelection(const apMonitoredObjectsTreeSelectedEvent& eve);

    void setCaption();

    void updateList(apContextID contextId);
    void addIcon(acIconId iconId, int& iconIndex);
    bool shouldHighlightItem(int functionIndex);
    void extendContextMenu();
    void updatePropertiesAndStatusBar(int rowIndex);

protected:
    // Context menu:
    QAction* _pBreakOnAction;
    QAction* _pEnableDisaleAllBreakpointsAction;
    QAction* _pAddRemoveBreakpointsAction;

    // My data model:
    gdAPICallsHistoryViewModel* _pTableModel;

    // Still using the WX id, in order to enable old identification mechanism:
    bool m_isGlobal;

    // Contain the previous amount of logged functions:
    int _previousRowCount;

    /// Contain the current amount of function calls, as updated from the server
    int _amountOfFunctionCalls;

    /// True iff the data is updated from server for the current process suspension
    bool m_isDataUpdated;

    apContextID _processRunSuspendedInContext;

    // Contains true iff we are during second chance exception handling:
    bool _isDuringSecondChanceExceptionHandling;

    // True iff the process is suspended:
    bool _isDebuggedProcessSuspended;

    // Current context id:
    apContextID _activeContextId;

    // Current execution mode:
    apExecutionMode _executionMode;

    // Contain the icons for the functions:
    QVector<QIcon*> _itemsIconsVector;

    // Icon indices:
    int _GLCallIconIndex;
    int _CLCallIconIndex;
    int _GLExtCallIconIndex;
    int _osSpecificAPICallIconIndex;
    int _osSpecificExtensionAPICallIconIndex;
    int _stringMarkerIconIndex;
    int _textureIconIndex;
    int _glBufferIconIndex;
    int _clBufferIconIndex;
    int _queueIconIndex;
    int _nextFunctionCallIconIndex;
    int _programsAndShadersCallIconIndex;
    int _profileIconIndex;
    int _yellowWarningIconIndex;
    int _orangeWarningIconIndex;
    int _redWarningIconIndex;

    bool _shouldSetCaption;
};


#endif  // __GDAPICALLSHISTORYVIEW
