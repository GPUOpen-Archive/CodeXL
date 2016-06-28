//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStatisticsViewBase.h
///
//==================================================================================

//------------------------------ gdStatisticsViewBase.h ------------------------------

#ifndef __GDSTATISTICSVIEWBASE
#define __GDSTATISTICSVIEWBASE

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/apStatistics.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTAPIClasses/Include/apFunctionDeprecation.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afHTMLContent.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>

enum gdStatisticsViewIndex
{
    // Viewers indices:
    GD_STATISTICS_VIEW_UNKNOWN = -1,
    GD_STATISTICS_VIEW_TOTAL_INDEX = 0,
    GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX = 1,
    GD_STATISTICS_VIEW_DEPRECATION_INDEX = 2,
    GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX = 3,
    GD_STATISTICS_VIEW_BATCH_INDEX = 4,
    GD_STATISTICS_VIEW_LAST_VIEWER_INDEX = GD_STATISTICS_VIEW_BATCH_INDEX
};

// The default order of the types (excluding the state change items):
enum gdFuncCallsViewTypes
{
    GD_GET_FUNCTIONS_INDEX = 0,
    GD_REDUNDANT_STATE_CHANGE_FUNCTIONS_INDEX,
    GD_EFFECTIVE_STATE_CHANGE_FUNCTIONS_INDEX,
    GD_STATE_CHANGE_FUNCTIONS_INDEX,
    GD_DEPRECATED_FUNCTIONS_INDEX,
    GD_DRAW_FUNCTIONS_INDEX,
    GD_RASTER_FUNCTIONS_INDEX,
    GD_PROGRAM_AND_SHADERS_FUNCTIONS_INDEX,
    GD_TEXTURE_FUNCTIONS_INDEX,
    GD_MATRIX_FUNCTIONS_INDEX,
    GD_NAME_FUNCTIONS_INDEX,
    GD_QUERY_FUNCTIONS_INDEX,
    GD_BUFFER_FUNCTIONS_INDEX,
    GD_BUFFERS_AND_IMAGE_FUNCTIONS_INDEX,
    GD_PROGRAM_AND_KERNELS_FUNCTIONS_INDEX,
    GD_QUEUE_FUNCTIONS_INDEX,
    GD_SYNC_FUNCTIONS_INDEX,
    GD_FEEDBACK_FUNCTIONS_INDEX,
    GD_VERTEX_ARRAY_FUNCTIONS_INDEX,
    GD_DEBUG_FUNCTIONS_INDEX,
    GD_GL_NULL_CONTEXT_FUNCTIONS_INDEX,
    GD_CL_NULL_CONTEXT_FUNCTIONS_INDEX,
    GD_GL_CONTEXT_BOUND_FUNCTIONS_INDEX,
    GD_CL_CONTEXT_BOUND_FUNCTIONS_INDEX,

    GD_NUMBER_OF_FUNCTION_TYPES_INDICES
};

// Defines the items sort direction:
enum gdStatisticsViewSortDirection
{
    GD_STATISTICS_SORT_NONE,

    // Total view:
    GD_TOTAL_STATISTICS_SORT_BY_TYPE,
    GD_TOTAL_STATISTICS_SORT_BY_CALLS_PERCENT,
    GD_TOTAL_STATISTICS_SORT_BY_CALLS_AMOUNT,
    GD_TOTAL_STATISTICS_SORT_BY_CALLS_AVERAGE_PERCENT,
    GD_TOTAL_STATISTICS_SORT_BY_CALLS_AVERAGE_AMOUNT,

    // State change view:
    GD_SORT_STATE_CHANGE_STATISTICS_BY_NAME,
    GD_SORT_STATE_CHANGE_STATISTICS_BY_TOTAL_CALLS_AMOUNT,
    GD_SORT_STATE_CHANGE_STATISTICS_BY_EFFECTIVE_CALLS_AMOUNT,
    GD_SORT_STATE_CHANGE_STATISTICS_BY_REDUNDANT_CALLS_AMOUNT,
    GD_SORT_STATE_CHANGE_STATISTICS_BY_REDUNDANT_CALLS_PERCENTAGE,

    // Function calls view:
    GD_SORT_FUNCTION_CALLS_STATISTICS_BY_NAME,
    GD_SORT_FUNCTION_CALLS_STATISTICS_BY_CALLS_PERCENTAGE,
    GD_SORT_FUNCTION_CALLS_STATISTICS_BY_CALLS_AMOUNT,
    GD_SORT_FUNCTION_CALLS_STATISTICS_BY_TYPE,

    // Deprecation view:
    GD_SORT_DEPRECATION_STATISTICS_BY_NAME,
    GD_SORT_DEPRECATION_STATISTICS_BY_DEPRACATION,
    GD_SORT_DEPRECATION_STATISTICS_BY_TOTAL_CALLS_AMOUNT,
    GD_SORT_DEPRECATION_STATISTICS_BY_CALLS_PERCENTAGE,
    GD_SORT_DEPRECATION_STATISTICS_BY_DEPRECATED_VERSION,
    GD_SORT_DEPRECATION_STATISTICS_BY_REMOVED_VERSION,

    // Batch view:
    GD_BATCH_STATISTICS_SORT_BY_RANGE,
    GD_BATCH_STATISTICS_SORT_BY_NUM_OF_BATCHES,
    GD_BATCH_STATISTICS_SORT_BY_PERCENTAGE_OF_BATCHES,
    GD_BATCH_STATISTICS_SORT_BY_NUM_OF_VERTICES,
    GD_BATCH_STATISTICS_SORT_BY_PERCENTAGE_OF_VERTICES,
};

struct gdStatisticsViewSortInfo
{
    gdStatisticsViewSortDirection _sortType;
    Qt::SortOrder _sortOrder;
};

// ----------------------------------------------------------------------------------
// Class Name:           GD_API gdBatchStatisticsView: public acListCtrl , public apIEventsObserver
// General Description: This class represents a batch statistics viewer. This viewer is added to the Statistics
//                      viewer as one of the notebook pages.
// Author:               Sigal
// Creation Date:        14/5/2009
// ----------------------------------------------------------------------------------
class GD_API gdStatisticsViewBase: public acListCtrl, public apIEventsObserver
{
    Q_OBJECT

public:

    gdStatisticsViewBase(QWidget* pParent, gdStatisticsViewIndex windowIndex, const gtString& statisticsViewShortName);
    virtual ~gdStatisticsViewBase();

    // Update function - should be implemented in children:
    virtual bool updateFunctionCallsStatisticsList(const apStatistics& currentStatistics);

    // Clears the statistics:
    virtual void clearAllStatisticsItems();

    // A data structure that is attached to this view's list control items:
    class gdStatisticsViewItemData
    {
    public:
        gdStatisticsViewItemData();
        ~gdStatisticsViewItemData();

        // Item icon type:
        afIconType _iconType;

        // Information for total statistics view:
        gdFuncCallsViewTypes _functionType;
        gtInt64 _averageAmountOfTimesCalled;
        float _averagePercentOfCalls;
        bool _isItemAvailable;

        // Information for the state change view:
        gtString _functionName;
        apMonitoredFunctionId _functionId;
        gtInt64 _amountOfRedundantTimesCalled;
        gtInt64 _amountOfEffectiveTimesCalled;
        gtInt64 _totalAmountOfTimesCalled;
        float _percentageOfRedundantTimesCalled;

        // Information for deprecation view:
        float _percentageOfTimesCalled;
        apAPIVersion _deprecatedAtVersion;
        apAPIVersion _removedAtVersion;
        apFunctionDeprecationStatus _deprecationStatus;

        // Information for function call statistics view:
        gtString _functionTypeStr;

        // Information for batch view:
        int _minRange;
        int _maxRange;
        long _itemChartColor;
        int _numOfBatches;
        gtUInt64  _numOfVertices;
        float _percentageOfBatches;
        float _percentageOfVertices;

    };

    class GD_API gdStatisticsTableWidgetItem : public QTableWidgetItem
    {

    public:

        gdStatisticsTableWidgetItem(const QString& text, gdStatisticsViewBase* pParent);
        ~gdStatisticsTableWidgetItem();

        virtual bool operator<(const QTableWidgetItem& other) const;

    protected:
        gdStatisticsViewBase* _pParent;
    };

    // Sets active context:
    void setActiveContext(apContextID activeContext) {_activeContextId = activeContext;}

    // Set is suspended:
    void setIsSuspended(bool isDebuggedProcessSuspended) {_isDebuggedProcessSuspended = isDebuggedProcessSuspended;};

    // Return the color index of the item when it was first added to chart:
    long getItemChartColor(int itemIndex);
    bool setItemChartColor(int itemIndex, long itemColor);

    virtual void onEvent(const apEvent& eve, bool& vetoEvent);

    // Get selected function name:
    bool getSelectedFunctionName(gtString& functionName);

    // Get item icon type:
    bool getItemIconType(int itemIndex, afIconType& iconType);

    // Get item chart color:
    virtual bool getItemChartColor(int itemIndex, int& amountOfCurrentItemsForColorSelection, bool useSavedColors, unsigned long& color);
    void addChartColor(unsigned long color) {_chartColors.push_back(color);};
    int amountOfChartColors() {return (int)_chartColors.size();};

    // Get an item tooltip:
    virtual gtString getItemTooltip(int itemIndex) { (void)(itemIndex); return L"";};

    gtString functionTypeToString(gdFuncCallsViewTypes functionType);

    // Should be called from base class constructor:
    void init();

    gdStatisticsViewSortDirection getSortTypeByViewerId(int columnIndex);

    virtual void setListCtrlColumns();

    virtual void initListCtrlColumns() {};
    virtual void setListHeadingToolTips() {};
    virtual void initializeImageList() {};
    QPixmap* icon(int iconIndex);

    friend class gdSaveFunctionCallsStatisticsCommand;

    // gdStatisticsViewBaseAbstract overrides:
    virtual gdStatisticsViewItemData* getItemData(int itemIndex);

    /// \param[out] selectedText returns the text currently selected
    virtual void GetSelectedText(gtString& selectedText) { acListCtrl::GetSelectedText(selectedText);  };

    // Resets columns:
    void resetColumnsWidth();

    // Selected item index:
    int getSelectedItemIndex();
    bool isItemSelected(int itemIndex);

    gdStatisticsViewIndex windowIndex() const {return _windowIndex;}

    virtual void extendContextMenu();
    void addEmptyListItem();
    bool doesCurrentFunctionExistAsBreakpoint(QString& funcName, apMonitoredFunctionId& funcId, int& breakpointId);
    void onUpdateSetSingleFunctionAsBreakpoint(bool& isEnabled, bool& isChecked, QString& commandName);
    void onUpdateSetMultipleFunctionAsBreakpoint(bool& isEnabled);

    // Sort 2 items:
    bool isItemSmallerThen(gdStatisticsViewItemData* pItemData1, gdStatisticsViewItemData* pItemData2);

    virtual const char* saveStatisticsDataFileName() = 0;

public slots:
    virtual void onFind();
    virtual void onFindNext();
    virtual void onCopy();
    virtual void onSelectAll();
    virtual void onSaveStatisticsData();
    virtual void onSetSingleFunctionAsBreakpoint();
    virtual void onSetMultipleFunctionsAsBreakpoints();
    virtual void onAboutToShowContextMenu();

    // Events are declared public, since we call them from the statistics viewer:
    virtual void onUpdateCopy(bool& isEnabled);
    virtual void onUpdateSelectAll(bool& isEnabled);
    virtual void onUpdateFind(bool& isEnabled);
    virtual void onUpdateFindNext(bool& isEnabled);
    virtual void onUpdateSaveStatisticsData(bool& isEnabled);

protected slots:

    // Override when using inherited widget item (can be used for customized sort for instance):
    virtual QTableWidgetItem* allocateNewWidgetItem(const QString& text);
    void onColumnHeaderClick(int columnIndex);

protected:

    // Context menu actions:
    QAction* _pFindAction;
    QAction* _pFindNextAction;
    QAction* _pSaveStatisticsAction;
    QAction* _pSetBreakpointAction;
    QAction* _pSetBreakpointMultipleAction;

    // Titles:
    gtVector<QString> _listControlColumnTitles;

    // Icons:
    gtPtrVector<QPixmap*> _listIconsVec;

    // Chart colors for items in view:
    gtVector<unsigned long> _chartColors;

    // Column width:
    gtVector<float> _listControlColumnWidths;
    int _widestColumnIndex;
    int _initialSortColumnIndex;

    // Context id:
    apContextID _activeContextId;

    // Execution mode:
    apExecutionMode _executionMode;

    // Sort info:
    gdStatisticsViewSortInfo _sortInfo;

    // Is the debugged process suspended?
    bool _isDebuggedProcessSuspended;

    // Member for general features for all viewers:
    gtString _statisticsViewShortName;

    // Window index:
    gdStatisticsViewIndex _windowIndex;

    bool _addBreakpointActions;

signals:

    //emitted after header click column sort
    void columnSorted();

};


#endif  // __GDBATCHSTATISTICSVIEW


