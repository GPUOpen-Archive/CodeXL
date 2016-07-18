//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CPUProfileDataTable.h
///
//==================================================================================

#ifndef __CPUPROFILEDATATABLE_H
#define __CPUPROFILEDATATABLE_H

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/Include/acTableWidgetItem.h>
#include <AMDTApplicationComponents/Include/acUserRoles.h>

// Local:
#include <inc/StdAfx.h>
#include <inc/DisplayFilter.h>

class CpuProfileModule;
class CpuProfileFunction;
class CPUProfileDataTable;
class SessionTreeNodeData;
class acTablePercentItemDelegate;

// A table row height:
#define CP_CPU_TABLE_ROW_HEIGHT 18

// Column indexes of the first sample:
#define SAMPLE_INDEX_IN_TABLE_PROCESS      2
#define SAMPLE_INDEX_IN_TABLE_MODULE       1
#define SAMPLE_INDEX_IN_TABLE_FUNCTION     2

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define BEGIN_TICK_COUNT(field)    m_startTime[CPUProfileDataTable::field] = GetTickCount()
    #define END_TICK_COUNT(field)      m_elapsedTime[CPUProfileDataTable::field] += (GetTickCount() - m_startTime[CPUProfileDataTable::field])
#else
    #define BEGIN_TICK_COUNT(field)
    #define END_TICK_COUNT(field)
#endif

#define MAX_FUNCTION_NAME_LEN 300
#define MAX_MODULE_NAME_LEN 300
#define MAX_PROCESS_NAME_LEN 300
#define SAMPLE_VALUE_PRECISION 8
#define SAMPLE_PERCENT_PRECISION 2

void mergedProfileDataVectors(gtVector<AMDTProfileData>& data);

/// -----------------------------------------------------------------------------------------------
/// \class Name: CPUProfileDataTable : public acListCtrl
/// \brief Description:  This class will be used to display a table of data in CPU profiling views
/// -----------------------------------------------------------------------------------------------
class CPUProfileDataTable : public acListCtrl
{
    Q_OBJECT

public:

    enum TableContextMenuActionType
    {
        DISPLAY_FUNCTION_IN_FUNCTIONS_VIEW,
        DISPLAY_FUNCTION_IN_CALLGRAPH_VIEW,
        DISPLAY_FUNCTION_IN_SOURCE_CODE_VIEW,
        DISPLAY_MODULE_IN_MODULES_VIEW,
        DISPLAY_MODULE_IN_FUNCTIONS_VIEW,
        DISPLAY_BY_MODULE_NAME,
        DISPLAY_PROCESS_IN_MODULE_VIEW,
        DISPLAY_PROCESS_IN_FUNCTIONS_VIEW,
        DISPLAY_BY_PROCESS_NAME,
        DISPLAY_CLU_NOTES
    };

    enum TableType
    {
        FUNCTION_DATA_TABLE,
        PROCESSES_DATA_TABLE,
        MODULES_DATA_TABLE
    };

    CPUProfileDataTable(QWidget* pParent,
                        const gtVector<TableContextMenuActionType>& additionalContextMenuActions,
                        SessionTreeNodeData* pSessionData);

    virtual ~CPUProfileDataTable();

    void extendContextMenu(const gtVector<TableContextMenuActionType>& contextMenuActions);

    bool displayTableData(std::shared_ptr<cxlProfileDataReader> pProfDataRdr,
                          std::shared_ptr<DisplayFilter> diplayFilter,
                          AMDTProcessId procId,
                          AMDTModuleId modId,
                          std::vector<AMDTUInt64> moduleIdVec = {});

    virtual bool fillTableData(AMDTProcessId procId,
                               AMDTModuleId modId,
                               std::vector<AMDTUInt64> modIdVec = {}) = 0;

    bool displayTableSummaryData(std::shared_ptr<cxlProfileDataReader> pProfDataRdr,
                                 std::shared_ptr<DisplayFilter> pDisplayFilter,
                                 int counterIdx, bool isCLU);

    /// Sort the table according to the requested display filter:
    void sortTable();

    // Static function that returns an icon according to file path:
    static QPixmap* moduleIcon(const osFilePath& filePath, bool is32Bit);

    // Organize the table a the hot spot indicator values:
    bool organizeTableByHotSpotIndicator();

    // Set the table display filter
    void setTableDisplaySettings(TableDisplaySettings* pTableSettings) { m_pTableDisplaySettings = pTableSettings; }

    // Accessors:
    TableDisplaySettings* tableDisplaySettings() {return m_pTableDisplaySettings;};

    static bool m_CLUNoteShown;

    void GetCluDataInRow(int row, int sampleIndex, gtVector<float>& cluData);

    // get table real type modules/functions/processes
    virtual TableType GetTableType() const = 0;

    // returns Empty Table message row number.
    int GetEmptyTableItemRowNum() const;

    // returns "other" message row number.
    int GetOtherSamplesItemRowNum() const;

signals:
    void contextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType actionType, QTableWidgetItem* pTableItem);

public slots:

    virtual void onAboutToShowContextMenu();
    void onContextMenuAction();

protected:

    // Overrides acListCtrl:
    virtual QTableWidgetItem* allocateNewWidgetItem(const QString& text);

    // Fill summary DataTables
    virtual bool fillSummaryTables(int counterIdx) = 0;

    // handles event of changing the hot-spot indicator combobox
    virtual bool HandleHotSpotIndicatorSet();

    bool initializeTableHeaders(std::shared_ptr<DisplayFilter> diplayFilter, bool isSummary = false);

    /// \brief Name:        setModuleCellValue
    /// \brief Description: Sets the data for the requested module column index
    /// \param[in]          rowIndex - the module row index
    /// \param[in]          colIndex - the data column index
    /// \param[in]          dataIndex - the index of the data within moduleDataVector
    /// \param[in]          &dataVector - the data vector
    void setTableRowCellValue(int rowIndex, int colIndex, int dataIndex, const gtVector<float>& dataVector);

    /// Checks if the table is empty, and display message if it is
    bool HandleEmptyTable();

    /// Checks if the table is empty
    bool IsTableEmpty() const;

    /// Sets the table item percent value
    /// \param rowIndex - the table item row index
    /// \param colIndex - the table item column index
    /// \param dataColIndex - the table item data index
    /// \return true / false is succeeded or failed
    bool setTableItemPercentValue(int rowIndex, int colIndex, int dataColIndex);

    /// Get a string from action type
    QString actionTypeToString(TableContextMenuActionType actionType);


    /// Find the CLU percent column:
    /// \param cluSampleColumnIndexList the indexes on which the CLU percentage columns is in
    /// \param hotSpotCaption the current hot spot caption
    /// \param isHotSpotCluPercent if hotSpotCaption is not empty, will contain true if the hot spot caption has percent content
    void findCLUPercentColumn(QList<int>& cluSampleColumnIndexList, const QString& hotSpotCaption, bool& isHotSpotCluPercent);

    // updates the 6th row (of top 5 table - hotspot) items data, to the current table sort order
    void UpdateLastRowItemsSortOrder();

    /// checks the profiling mode and return true if Base Time profiling
    bool IsBaseTimeProfiling() const;


    /// checks the profiling mode and return true if Cache Line profiling
    bool IsCacheLineProfiling() const;
    bool delegateSamplePercent(int colNum);
    void SetIcon(gtString modulePath,
                 AMDTUInt32 rowIndex,
                 AMDTUInt32 iconColIndex,
                 AMDTUInt32 toolTipColidx,
                 bool is32Bit, int idxRole);


    std::shared_ptr<DisplayFilter>  m_pDisplayFilter = nullptr;
    std::shared_ptr<cxlProfileDataReader> m_pProfDataRdr = nullptr;
    bool m_isCLU = false;

protected slots:

    virtual void onContextMenuEvent(const QPoint& position);
    /// Handles user sort:
    void sortIndicatorChanged(int sortColumn, Qt::SortOrder order);

protected:

    TableDisplaySettings* m_pTableDisplaySettings = nullptr;

    // Icons:
    static gtPtrVector<QPixmap*> m_sTableIcons;
    static bool m_sIconsInitialized;

    /// Contain the tree item data for the currently displayed session:
    SessionTreeNodeData* m_pDisplaySessionData;

    /// Contain the object that is responsible for the delegate of the CLU percent items:
    acTablePercentItemDelegate* m_pCLUDelegate = nullptr;

    // saves the empty table message row item
    QTableWidgetItem* m_pEmptyRowTableItem = nullptr;

    // saves the other samples message row item
    QTableWidgetItem* m_pOtherSamplesRowItem = nullptr;

    enum
    {
        DisplayProfileData,             // CPUProfileDataTable
        FillListData,                   // CPUProfileDataTable
        SetHotSpotIndicatorValues,      // CPUProfileDataTable

        BuildHotSpot,                   // FunctionsDataTable
        IncrementProgressBar,           // FunctionsDataTable

        CPU_TABLE_MAX_VALUES
    };

    gtUInt32 m_elapsedTime[CPU_TABLE_MAX_VALUES];
    gtUInt32 m_startTime[CPU_TABLE_MAX_VALUES];

};

#endif //__CPUPROFILEDATATABLE_H
