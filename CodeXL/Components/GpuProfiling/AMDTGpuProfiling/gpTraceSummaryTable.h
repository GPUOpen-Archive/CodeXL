//=====================================================================

//=====================================================================
#ifndef _GPTRACESUMMARYTABLE_H_
#define _GPTRACESUMMARYTABLE_H_

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
class gpTraceSummaryTable;
class acTimelineBranch;
class CommandList;

typedef unsigned int CallIndexId;

// ----------------------------------------------------------------------------------
// Class Name:          gpTraceSummaryTable
// General Description: displays all calls summary of a given type (API / GPU)
// ----------------------------------------------------------------------------------
class gpTraceSummaryTable : public gpSummaryTable
{
    Q_OBJECT

public :
    enum TraceSummaryColumnIndex
    {
        COLUMN_CALL_NAME,
        COLUMN_MAX_TIME,
        COLUMN_MIN_TIME,
        COLUMN_AVG_TIME,
        COLUMN_CUMULATIVE_TIME,
        COLUMN_PERCENTAGE_OF_TOTAL_TIME,
        COLUMN_NUM_OF_CALLS,
        COLUMN_COUNT
    };
    gpTraceSummaryTable(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineAbsoluteStart);

    /// class destructor.
    virtual ~gpTraceSummaryTable();

    //virtual bool Init();

    /// Returns a ProfileSessionDataItem* which belong to the specified call
    ProfileSessionDataItem* GetRelatedItem(int rowIndex, int colIndex);


    /// Returns a map of call items
    const QMap<CallIndexId, ProfileSessionDataItem*>& GetAllApiCallItemsMap()const;

    /// Returns an item by table row index
    bool GetItemCallIndex(int row, CallIndexId& callIndex, QString& callName)const;

    /// Returns a summary info item by call name
    APISummaryTraceInfo GetSummaryInfo(int apiCall);

    /// Returns a map of summary info items
    const QMap<CallIndexId, APISummaryTraceInfo>& GetApiCallInfoSummaryMap()const;

    /// Selects the row which describes the given call
    void SelectRowByCallName(const QString& callName);

private:
    /// Fills table only with calls within the timeline start and end
    void BuildSummaryMapInTimelineScope(quint64 min, quint64 max);


    /// Collects calls into given map
    void CollateAllItemsIntoSummaryMap();


    /// Fill map with API calls data
    //bool InitAPIItems();

    /// Fill map with GPU calls data
    //bool InitGPUItems();

    /// free data containers
    void Cleanup();

    /// Map containing UI information for each of the API calls summary table items collected by call name. One summary item per key
    QMap<CallIndexId, APISummaryTraceInfo> m_apiCallInfoSummaryMap;
    QMap<CallIndexId, APISummaryTraceInfo>::const_iterator m_apiCallInfoSummaryMapIter;
    //QMap<osThreadId, acTimelineBranch*> m_threadTimetineBranchMap;

    protected slots:
    /// Change the cursor according to the entered item
    /// \param row the entered item row
    /// \param col the entered item column
    void OnCellEntered(int row, int column);

protected:
    /// Fill APISummaryInfo with relevant data fromProfileSessionDataItem
    virtual void AddSessionItemToSummaryInfo(APISummaryTraceInfo& info, ProfileSessionDataItem* pItem, unsigned int apiId)=0;

    void InsertSummaryInfoToMap(APISummaryTraceInfo& info);
    /// Fills table with calls from m_logic data
    virtual void FillTable();

    /// Adds a row to the table
    void AddSummaryRow(int rowIndex, APISummaryInfo* pInfo);

    /// Map containing UI information for each of the API calls. Each key may have multiple items
    QMap<CallIndexId, ProfileSessionDataItem*> m_allCallItemsMultiMap;

    quint64 m_totalTimeMs;
    quint64 m_numTotalCalls;
};

class gpCPUTraceSummaryTable : public gpTraceSummaryTable
{
    Q_OBJECT
public:
    gpCPUTraceSummaryTable(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineAbsoluteStart);
protected:
    /// Fill APISummaryInfo with relevant data fromProfileSessionDataItem
    virtual void AddSessionItemToSummaryInfo(APISummaryTraceInfo& info, ProfileSessionDataItem* pItem, unsigned int apiId);
    virtual bool InitItems();
};

class gpGPUTraceSummaryTable : public gpTraceSummaryTable
{
    Q_OBJECT
public:

    gpGPUTraceSummaryTable(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineAbsoluteStart);
protected:
    /// Fill APISummaryInfo with relevant data fromProfileSessionDataItem
    virtual void AddSessionItemToSummaryInfo(APISummaryTraceInfo& info, ProfileSessionDataItem* pItem, unsigned int apiId);
    virtual bool InitItems();
};

#endif //_GPTRACESUMMARYTABLE_H_
