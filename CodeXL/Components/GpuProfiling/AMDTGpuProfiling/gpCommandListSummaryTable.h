//=====================================================================

//=====================================================================
#ifndef _gpCommandListSummaryTable_H_
#define _gpCommandListSummaryTable_H_

// Infra:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/gpTraceDataContainer.h>
#include <AMDTGpuProfiling/gpSummaryTable.h>

class gpTraceDataContainer;
class gpTraceView;
class APISummaryInfo;
class ProfileSessionDataItem;
class acTimelineBranch;
class CommandList;

// ----------------------------------------------------------------------------------
// Class Name:          gpCommandListSummaryTable
// General Description: displays all calls summary of a given type (API / GPU)
// ----------------------------------------------------------------------------------
class gpCommandListSummaryTable : public gpSummaryTable
{
    Q_OBJECT

public:
    enum CommandListSummaryColumnIndex
    {
        COLUMN_COMMAND_INDEX,
        COLUMN_EXECUTION_TIME,
        COLUMN_START_TIME,
        COLUMN_END_TIME,
        COLUMN_NUM_OF_COMMANDS,
        COLUMN_GPU_QUEUE,
        COLUMN_ADDRESS,
        COLUMN_COUNT
    };
    gpCommandListSummaryTable(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineAbsoluteStart);

    /// class destructor.
    virtual ~gpCommandListSummaryTable();
    
    /// Returns a ProfileSessionDataItem* which belong to the specified call
    ProfileSessionDataItem* GetRelatedItem(int rowIndex, int colIndex);


    /// Returns an item by table row index
    bool GetItemCommandList(int row, QString& callName)const;
    bool GetItemCommandListAddress(int row, QString& callName)const;
    bool GetItemQueueName(int row, QString& queueName)const;
    void SelectCommandList(const QString& commandListName);

protected slots:
    /// Change the cursor according to the entered item
    /// \param row the entered item row
    /// \param col the entered item column
    void OnCellEntered(int row, int column);
protected:
    /// Fills table with calls from m_logic data
    void FillTable();

    /// Adds a row to the table
    virtual void AddSummaryRow(APISummaryInfo* pInfo);

    virtual bool InitItems();

private:
    /// Fills table only with calls within the timeline start and end
    void BuildSummaryMapInTimelineScope(quint64 min, quint64 max);

    /// Collects calls into given map
    void CollateAllItemsIntoSummaryMap();

    APISummaryCommandListInfo GetSummaryInfo(const QString& callName);

    QMap<QString, APISummaryCommandListInfo> m_allInfoSummaryMap;
    QMap<QString, APISummaryCommandListInfo> m_apiCallInfoSummaryMap;

};

#endif //_GPTRACESUMMARYTABLE_H_
