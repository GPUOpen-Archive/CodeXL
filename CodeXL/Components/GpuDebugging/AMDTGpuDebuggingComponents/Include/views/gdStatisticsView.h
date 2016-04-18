//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStatisticsView.h
///
//==================================================================================

//------------------------------ gdStatisticsView.h ------------------------------

#ifndef __GDSTATISTICSVIEW
#define __GDSTATISTICSVIEW

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

#include <AMDTApplicationComponents/Include/acChartWindow.h>

// Foreward Declarations:
class acWXHTMLWindow;
class apDebuggedProcessRunSuspendedEvent;
class apStatistics;
class gdStatisticsPanel;
class afProgressBarWrapper;

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsViewBase.h>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTApplicationComponents/Include/acRawFileHandler.h>
#include <AMDTApplicationComponents/Include/acDataView.h>
#include <AMDTApplicationComponents/Include/acColorSampleBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

// Pre-Decleration:
class gdAPICallsHistoryView;

// ----------------------------------------------------------------------------------
// Class Name:           gdStatisticsView: public acFrame
//                       public apIEventsObserver
// General Description:  The Statistics viewer.
// ----------------------------------------------------------------------------------
class GD_API gdStatisticsView : public QTabWidget
{
    Q_OBJECT

    friend class gdStatisticsPanel;

public:
    // Constructors:
    gdStatisticsView(afProgressBarWrapper* pProgressBar, QWidget* pParent, gdStatisticsPanel* pParentStatisticsPanel, bool isAnalyzeModeEnabled = true);

    // Destructor
    ~gdStatisticsView();

    // Update utilities:
    bool updateStatisticsViewers();
    void applyPageOnClick(gdStatisticsViewIndex windowIndex);
    void clearAllViewers();

    // Layout:
    virtual void setFrameLayout(const QSize& viewSize);

    // Public UI events:
    void onUpdateShowDetailedBatchData(bool& isEnabled, bool& isChecked);
    void onDetailedBatchDataButton();
    void onUpdateDetailedBatchDataButton(bool& isEnabled);
    bool displayContext(const apContextID& contextID, bool forceUpdate = false);

    void exportStatistics(int pageIndex);
    int pageItemCount(int pageIndex);

    // Selected page:
    int selectedPageIndex();
    void setSelectedPage(int selectedPageIndex);
    void setSelectedPageFromGlobalIndex(int selectedPageIndex);
    gdStatisticsViewBase* selectedPage();

    void setActiveContext(apContextID activeContext) {_activeContext = activeContext;} ;

    // Edit menu commands
    virtual void onUpdateEdit_Copy(bool& isEnabled);
    virtual void onUpdateEdit_SelectAll(bool& isEnabled);

    virtual void onEdit_Copy();
    virtual void onEdit_SelectAll();

    // Debugged process event handlers:
    virtual void onTreeItemSelection(const apMonitoredObjectsTreeSelectedEvent& eve);
    void onBreakpointHitEvent();

    void setChartWindow(acChartWindow* pChartWindow) {_pChartWindow = pChartWindow;};

    // Get the requested statistics view:
    gdStatisticsViewBase* statisticsPage(gdStatisticsViewIndex windowIndex);

    /// \param[out] selectedText returns the text currently selected
    virtual void GetSelectedText(gtString& selectedText);

protected:

    // Layout creation functions:
    void createStatisticsPages();

    bool createRenderContextWindow();
    void initializeColorVectors();

    // Virtual function:
    // Raise the view and show it as top window:
    virtual void raiseView();

    // Chart help functions:
    void updateViewerChart(gdStatisticsViewIndex windowIndex, gdStatisticsViewBase* pViewer, int sampledDataIndex, bool useColorsBeforeSort, acChartType chartType);
    bool addItemToChart(gdStatisticsViewBase* pViewer, bool isNewData, int& amountOfNonZeroItems, bool useColorsBeforeSort, int itemIndex, int sampledDataIndex, bool isSelected);

    int windowIDToTabViewerID(int cmdId);

    void showPagesAccordingToContextType();

protected slots:

    void onPageChange(int newPageIndex);

    // List click event handlers:
    void onStateChangeStatsClick(QTableWidgetItem* pClickedItem);
    void onDeprecationStatsClick(QTableWidgetItem* pClickedItem);
    void onBatchStatsClick(QTableWidgetItem* pClickedItem);
    void onFunctionStatisticsClick(QTableWidgetItem* pClickedItem);
    void onTotalStatisticsClick(QTableWidgetItem* pClickedItem);
    void onListSelectionChanged();
    void onCallsHistoryClick(const QModelIndex& index);
    bool connectViewToHandlers();
    void onChartItemClicked(int chartItemIndex);

    virtual void onFindClick();
    virtual void onFindNext();
    void onColumnSorted();

protected:

    // Progress bar:
    afProgressBarWrapper* _pProgressBar;

    // Statistics Chart:
    acChartWindow* _pChartWindow;

    // Parent panel:
    gdStatisticsPanel* _pParentStatisticsPanel;

    // Contain the proportion for the top / bottom panels:
    float _bottomPanelProp;

    // Contain the proportion for the left / right bottom panels:
    float _leftPanelProp;

    // Currently active context
    apContextID _activeContext;

    // Previous active context:
    apContextID _previousDisplayedContext;

    // Holds the current statistics information:
    apStatistics* _pStatistics;

    // Remember which panes were shown when hiding the viewer
    bool _isChartShown;

    gdStatisticsViewIndex _lastSelectedNoteBookPane;

    // Contain true iff analyze mode is currently enabled:
    bool _isAnalyzeModeEnabled;

    // Vector of booleans containing true iff the page i is currently displayed:
    bool _shouldDisplayPages[GD_STATISTICS_VIEW_LAST_VIEWER_INDEX + 1];

    // Array of pages captions:
    QString m_pagesCaptions[GD_STATISTICS_VIEW_LAST_VIEWER_INDEX + 1];

    // Statistics viewer pages:
    gdStatisticsViewBase* _pages[GD_STATISTICS_VIEW_LAST_VIEWER_INDEX + 1];

    // Calls history view:
    gdAPICallsHistoryView* _pCallHistoryView;

    // Contain true iff the view is currently shown in VS:
    bool _isShown;

    // Contain true if we should ignore item activation events;
    bool _ignoreListSelectionEvents;

    // Force chart data update:
    bool _forceChartUpdate;

};

#endif  // __GDSTATISTICSVIEW
