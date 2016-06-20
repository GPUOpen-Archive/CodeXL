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

#include <AMDTCpuProfilingRawData/inc/CpuProfileReader.h>

// Local:
#include <inc/StdAfx.h>
#include <inc/DisplayFilter.h>

class CpuProfileReader;
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

#define NUMBER_OF_HOTSPOT_ROWS  5

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define BEGIN_TICK_COUNT(field)    m_startTime[CPUProfileDataTable::field] = GetTickCount()
    #define END_TICK_COUNT(field)      m_elapsedTime[CPUProfileDataTable::field] += (GetTickCount() - m_startTime[CPUProfileDataTable::field])
#else
    #define BEGIN_TICK_COUNT(field)
    #define END_TICK_COUNT(field)
#endif

class HotSpotValue
{
public:
    HotSpotValue(): m_index(-1), m_percentValue(0) {}
    int m_index;
    float m_percentValue;
};
typedef gtVector<HotSpotValue> HotSpotValuesMap;
class HotSpotValuesMapCompareFunctor
{
public:
    bool operator()(const HotSpotValue& val1, const HotSpotValue& val2)
    {
        return (val1.m_percentValue > val2.m_percentValue);
    };
};


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

    /// Add the requested items to context menu:
    void extendContextMenu(const gtVector<TableContextMenuActionType>& contextMenuActions);

    /// Display the requested information from the profile reader, with the requested display filter
    /// \param pProfileReader the profile reader
    /// \param displayFilter the requested display data
    /// \return true / false is succeeded or failed
    bool displayProfileData(CpuProfileReader* pProfileReader);

	bool displayTableData(shared_ptr<cxlProfileDataReader> pProfDataRdr,
							shared_ptr<DisplayFilter> diplayFilter,
							AMDTProcessId procId,
							AMDTModuleId modId,
							std::vector<AMDTUInt64> moduleIdVec = {});

	virtual bool fillTableData(AMDTProcessId procId,
                            AMDTModuleId modId,
                            std::vector<AMDTUInt64> modIdVec = {}) = 0;

	bool displayTableSummaryData(shared_ptr<cxlProfileDataReader> pProfDataRdr, 
		shared_ptr<DisplayFilter> pDisplayFilter,
		int counterIdx);

    /// Sort the table according to the requested display filter:
    void sortTable();


    /// Returns the hot spot position for the requested row. This position is derived from the values spread in the table.
    /// \param percentValue - the percent value for the requested item
    /// \param pos[out] - what is the position of the row when the values are sorted from low to high (without the hidden cells)
    /// \param numberOfCells[out] - the position is of 'numberOfCells'
    void getRowHotSpotPosition(float percentValue, int& pos, int& numberOfCells);

    /// Static function that returns an icon according to file path:
    static QPixmap* moduleIcon(const osFilePath& filePath, bool is32Bit);

    QStyleOptionViewItem getViewOptions() const {return viewOptions();}

    /// Organize the table a the hot spot indicator values:
    bool organizeTableByHotSpotIndicator();

    /// Set the table display filter
    /// \param pTableSettings the new table display settings
    /// \param pSessionDisplaySettings the session display settings
    void setTableDisplaySettings(TableDisplaySettings* pTableSettings, SessionDisplaySettings* pSessionDisplaySettings)
    {
        m_pTableDisplaySettings = pTableSettings;
        m_pSessionDisplaySettings = pSessionDisplaySettings;
    }

    /// Accessors:
    TableDisplaySettings* tableDisplaySettings() {return m_pTableDisplaySettings;};

    /// Return the amount of rows that are not hidden:
    int amountOfShownRows() const ;

    void GetCluDataInRow(int row, int sampleIndex, gtVector<float>& cluData);

    static bool m_CLUNoteShown;

    /// get table real type modules/functions/processes
    /// \returns TableType
    virtual TableType GetTableType() const = 0;

    /// returns Empty Table message row number.
    /// \returns row number or -1 if cant find it
    int GetEmptyTableItemRowNum() const;

    /// returns "other" message row number.
    /// \returns row number or -1 if cant find it
    int GetOtherSamplesItemRowNum() const;

signals:
    void contextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType actionType, QTableWidgetItem* pTableItem);

public slots:

    virtual void onAboutToShowContextMenu();
    void onContextMenuAction();

protected:

    /// This function performs operations that can be done only after first calculation stage
    /// 1. Add the percent columns
    /// 2. Filter the data by hot spot indicator
    /// \return true on success false on failure
    bool setHotSpotIndicatorValues();

    // Overrides acListCtrl:
    virtual QTableWidgetItem* allocateNewWidgetItem(const QString& text);

    /// Fill the list data according to the requested item:
    virtual bool fillListData();

	// Fill summary DataTables
	virtual bool fillSummaryTables(int counterIdx) = 0;

    /// Build the map of the current hot spot values. The function is virtual, since functions table for instance,
    /// needs to implement it in a more performance wise implementation
    /// \return true / false is succeeded or failed
    virtual bool buildHotSpotIndicatorMap();

    /// handles event of changing the hot-spot indicator combobox
    virtual bool HandleHotSpotIndicatorSet();

    /// Sets the list control columns according to the current displayed item:
    bool initializeListHeaders();

	bool initializeTableHeaders(shared_ptr<DisplayFilter> diplayFilter);

    /// \brief Name:        setModuleCellValue
    /// \brief Description: Sets the data for the requested module column index
    /// \param[in]          rowIndex - the module row index
    /// \param[in]          colIndex - the data column index
    /// \param[in]          dataIndex - the index of the data within moduleDataVector
    /// \param[in]          &dataVector - the data vector
    void setTableRowCellValue(int rowIndex, int colIndex, int dataIndex, const gtVector<float>& dataVector);

    /// Display the values for a row, according to values in data vector. Also handle hot spot indicator column
    /// \param rowIndex - the index of the row for which the values should be set
    /// \dataVector - the vector containing the collected data for this row
    void setTableDisplayedColumnsValues(int rowIndex, const gtVector<float>& dataVector);

    /// Fill the table with percent value if this is the requirement:
    /// \return true / false is succeeded or failed
    bool setPercentValues();

    /// Fill the table with percent values for CLU if this is the requirement:
    /// \return true / false is succeeded or failed
    void setCLUPercentValues();

    /// Checks if the table is empty, and display message if it is
    bool HandleEmptyTable();

    /// Checks if the table is empty
    bool IsTableEmpty() const;

    // handle 6th row in overview table 5 hotspots
    void SetLastRowForTop5(bool isTableEmpty);

    /// Sets the table item percent value
    /// \param rowIndex - the table item row index
    /// \param colIndex - the table item column index
    /// \param dataColIndex - the table item data index
    /// \return true / false is succeeded or failed
    bool setTableItemPercentValue(int rowIndex, int colIndex, int dataColIndex);

    /// Get a string from action type
    QString actionTypeToString(TableContextMenuActionType actionType);

    /// Hide the filtered columns according to the display filter:
    void hideFilteredColumns();

    /// Find the CLU percent column:
    /// \param cluSampleColumnIndexList the indexes on which the CLU percentage columns is in
    /// \param hotSpotCaption the current hot spot caption
    /// \param isHotSpotCluPercent if hotSpotCaption is not empty, will contain true if the hot spot caption has percent content
    void findCLUPercentColumn(QList<int>& cluSampleColumnIndexList, const QString& hotSpotCaption, bool& isHotSpotCluPercent);

    // updates the 6th row (of top 5 table - hotspot) items data, to the current table sort order
    void UpdateLastRowItemsSortOrder();

    /// checks the profiling mode and return true if Base Time profiling
    bool IsBaseTimeProfiling() const;

    /// is there a need to show the separate columns for percent values
    bool IsShowSeperatePercentColumns() const;

    /// checks the profiling mode and return true if Cache Line profiling
    bool IsCacheLineProfiling() const;
	bool delegateSamplePercent(int colNum);

	shared_ptr<DisplayFilter>  m_pDisplayFilter = nullptr;
  shared_ptr<cxlProfileDataReader> m_pProfDataRdr = nullptr;

protected slots:

    virtual void onContextMenuEvent(const QPoint& position);
    /// Handles user sort:
    void sortIndicatorChanged(int sortColumn, Qt::SortOrder order);

protected:

    TableDisplaySettings* m_pTableDisplaySettings = nullptr;                ///< Represents the currently displayed table filter
    SessionDisplaySettings* m_pSessionDisplaySettings = nullptr;        ///< Represents the currently displayed session filter
    CpuProfileReader* m_pProfileReader = nullptr;                      ///< A pointer to the session profile reader

    // Summarize the total value of sample counts:
    double m_totalSampleCount;
    HotSpotValuesMap m_hotSpotCellsMap;
    gtVector<float>       m_totalDataValuesVector;

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

    // saves the index of the percent columns
    QList<int> m_percentColsNum;

    bool m_tableRowHasIcon;

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
