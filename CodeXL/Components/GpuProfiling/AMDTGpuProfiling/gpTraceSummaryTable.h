//=====================================================================

//=====================================================================
#ifndef _GPTRACESUMMARYTABLE_H_
#define _GPTRACESUMMARYTABLE_H_

#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTGpuProfiling/gpTraceDataContainer.h>

class gpTraceDataContainer;
class gpTraceView;
class APISummaryInfo;
class ProfileSessionDataItem;
class gpTraceSummaryTable;
class acTimelineBranch;
class CommandList;

typedef unsigned int CallIndexId;
/// Summary mode enum
enum eCallType
{
    /// API Summary
    API_CALL = 0,
    /// GPU Summary
    GPU_CALL,
    /// Command list
    COMMAND_LIST,
    MAX_TYPE
};



// ----------------------------------------------------------------------------------
// Class Name:          APISummaryInfo
// General Description: Contains summary data for a group of call name items
// ----------------------------------------------------------------------------------
class APISummaryInfo
{
public:

    APISummaryInfo() : m_pMinTimeItem(nullptr), m_pMaxTimeItem(nullptr)
    {

        m_maxTimeMs = 0;
        m_minTimeMs = std::numeric_limits<quint64>::max();
        m_numCalls = 0;
    }

    virtual void TableItemsAsString(QStringList& membersStringsList) = 0;
    quint64 m_maxTimeMs;
    quint64 m_minTimeMs;
    QColor m_typeColor;
    ProfileSessionDataItem* m_pMinTimeItem;
    ProfileSessionDataItem* m_pMaxTimeItem;
    int m_numCalls;
};

class APISummaryTraceInfo : public APISummaryInfo
{
public:

    APISummaryTraceInfo() : APISummaryInfo()
    {
        m_callIndex = 0;
        m_cumulativeTime = 0;
        m_percentageOfTotalTime = 0;
        m_avgTimeMs = 0;
    }
    void TableItemsAsString(QStringList& membersStringsList)
    {
        // m_avgTimeMs is calculated here, after all samples were gathered
        if (0 != m_numCalls)
        {
            m_avgTimeMs = (float)((float)m_cumulativeTime / (float)m_numCalls);

            membersStringsList << m_call;
            membersStringsList << QString::number(m_maxTimeMs, 'f', 6);
            membersStringsList << QString::number(m_minTimeMs, 'f', 6);
            membersStringsList << QString::number(m_avgTimeMs, 'f', 6);
            membersStringsList << QString::number(m_cumulativeTime, 'f', 6);
            membersStringsList << QString::number(m_percentageOfTotalTime, 'f', 3);
            membersStringsList << QString::number(m_numCalls);
        }
    }


    QString m_interface;
    QString m_call;
    int m_callIndex;
    quint64 m_cumulativeTime;
    float m_percentageOfTotalTime;
    float m_avgTimeMs;

};

class APISummaryCommandListInfo : public APISummaryInfo
{
public:

    APISummaryCommandListInfo() : APISummaryInfo()
    {
        m_executionTimeMS = 0;
    }

    void TableItemsAsString(QStringList& membersStringsList)
    {
        membersStringsList << m_index;
        membersStringsList << QString::number(m_executionTimeMS, 'f', 3);
        membersStringsList << QString::number(m_minTimeMs, 'f', 3);
        membersStringsList << QString::number(m_maxTimeMs, 'f', 3);
        membersStringsList << QString::number(m_numCalls);
        membersStringsList << m_gpuQueue;
        membersStringsList << m_address;
    }


    QString m_index;
    QString m_address;
    QString m_gpuQueue;
    QString m_gpuQueueAddress;
    quint64 m_executionTimeMS;

};

/////////////////////////////////////////////////////////////////////////////////////

class gpSummaryTable : public acListCtrl
{

    Q_OBJECT
public :
    gpSummaryTable(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, eCallType callType, quint64 timelineAbsoluteStart);
    virtual ~gpSummaryTable();
    // Save selection in summary table to be restored on tab switch
    // Restores selection
    void RestoreSelection();
    void SaveSelection(int row);

    // Clears selection
    void ClearSelection();

    /// Rebuilds summary table
    void Refresh(bool useTimelineScope, quint64 minTime, quint64 maxTime);

    virtual ProfileSessionDataItem* GetRelatedItem(int rowIndex, int colIndex)=0;


protected:
    /// Fills table with calls from m_logic data
    virtual void FillTable()=0;

    virtual     /// Adds a row to the table
    void AddSummaryRow(int rowIndex, APISummaryInfo* pInfo) = 0;

    /// Rebuilds the summary info map
    void RebuildSummaryMap(bool useScope, quint64 startTime, quint64 endTime);

    /// The session data container
    gpTraceDataContainer* m_pSessionDataContainer;

    /// A pointer to the parent session view
    gpTraceView* m_pTraceView;

    int m_lastSelectedRowIndex;
    quint64 m_timelineAbsoluteStart;

private:
    /// Fills table only with calls within the timeline start and end
    virtual void BuildSummaryMapInTimelineScope(quint64 min, quint64 max)=0;


    /// Collects calls into given map
    virtual void CollateAllItemsIntoSummaryMap()=0;

};

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
    gpTraceSummaryTable(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, eCallType callType, quint64 timelineAbsoluteStart);

    /// class destructor.
    virtual ~gpTraceSummaryTable();


    /// Returns a ProfileSessionDataItem* which belong to the specified call
    ProfileSessionDataItem* GetRelatedItem(int rowIndex, int colIndex);


    /// Returns a map of call items
    const QMap<CallIndexId, ProfileSessionDataItem*>& GetAllApiCallItemsMap()const;

    /// Returns an item by table row index
    bool GetItemCallIndex(int row, CallIndexId& callIndex, QString& callName)const;

    void Init(eCallType callType, gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView);


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
    void InitAPIItems(gpTraceDataContainer* pDataContainer);

    /// Fill APISummaryInfo with relevant data fromProfileSessionDataItem
    void AddSessionItemToSummaryInfo(APISummaryTraceInfo& info, ProfileSessionDataItem* pItem, unsigned int apiId);

    /// Fill map with GPU calls data
    void InitGPUItems(gpTraceDataContainer* pDataContainer);

    /// free data containers
    void Cleanup();

    /// Determines whether the table handles API or GPU items
    eCallType m_callType;

    /// The session data container
    gpTraceDataContainer* m_pSessionDataContainer;

    /// A pointer to the parent session view
    gpTraceView* m_pTraceView;

    /// Map containing UI information for each of the API calls. Each key may have multiple items
    QMap<CallIndexId, ProfileSessionDataItem*> m_allCallItemsMultiMap;

    /// Map containing UI information for each of the API calls summary table items collected by call name. One summary item per key
    QMap<CallIndexId, APISummaryTraceInfo> m_apiCallInfoSummaryMap;
    QMap<CallIndexId, APISummaryTraceInfo>::const_iterator m_apiCallInfoSummaryMapIter;
    QMap<osThreadId, acTimelineBranch*> m_threadTimetineBranchMap;
    quint64 m_totalTimeMs;
    quint64 m_numTotalCalls;
protected slots:
    /// Change the cursor according to the entered item
    /// \param row the entered item row
    /// \param col the entered item column
    void OnCellEntered(int row, int column);

protected:
    void InsertSummaryInfoToMap(APISummaryTraceInfo& info);
    /// Fills table with calls from m_logic data
    virtual void FillTable();

    /// Adds a row to the table
    void AddSummaryRow(int rowIndex, APISummaryInfo* pInfo);
};

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
    virtual void AddSummaryRow(int rowIndex, APISummaryInfo* pInfo);

private:
    /// Fills table only with calls within the timeline start and end
    void BuildSummaryMapInTimelineScope(quint64 min, quint64 max);
    void InitCommandListItems();

    /// Collects calls into given map
    void CollateAllItemsIntoSummaryMap();

    void InitCommandListItems(gpTraceDataContainer* pSessionDataContainer);

    APISummaryCommandListInfo GetSummaryInfo(const QString& callName);

    /// The session data container
    gpTraceDataContainer* m_pSessionDataContainer;

    /// A pointer to the parent session view
    gpTraceView* m_pTraceView;

    int m_lastSelectedRowIndex;
    QMap<QString, APISummaryCommandListInfo> m_allInfoSummaryMap;
    QMap<QString, APISummaryCommandListInfo> m_apiCallInfoSummaryMap;

};

////////////////////////////////////////////////////////////

class PercentageItem : public QTableWidgetItem
{

public:
    bool operator< (const QTableWidgetItem& other) const
    {
        QString thisVal = this->text();
        QString otherVal = other.text();
        thisVal.remove(PERCENTAGE_SYMBOL);
        otherVal.remove(PERCENTAGE_SYMBOL);
        return (thisVal.toDouble() < otherVal.toDouble());
    }
    virtual void setData(int role, const QVariant& value)
    {
        if (role == Qt::DisplayRole)
        {
            QString valStr = QString::number(value.toDouble());
            valStr.append(PERCENTAGE_SYMBOL);
            QTableWidgetItem::setData(role, valStr);
        }
        else
        {
            QTableWidgetItem::setData(role, value);
        }
    }

private:
    const char PERCENTAGE_SYMBOL[2] = "%";
};

class FormattedTimeItem : public QTableWidgetItem
{
public:
    FormattedTimeItem() : m_isLink(false) {}

    void SetAsLink(bool isLink)
    {
        m_isLink = isLink;
    }

    bool operator< (const QTableWidgetItem& other) const
    {
        QString thisVal = this->text();
        QString otherVal = other.text();

        QStringList strListThis = thisVal.split(AC_STR_SpaceA);
        QStringList strListOther = otherVal.split(AC_STR_SpaceA);

        if (strListThis[1] == strListOther[1])
        {
            // same units
            double valThis(strListThis[0].toDouble());
            double valOther(strListOther[0].toDouble());
            return (valThis < valOther);
        }
        else
        {
            double valNanoThis = StrToNanoSec(strListThis[0], strListThis[1]);
            double valNanoOther = StrToNanoSec(strListOther[0], strListOther[1]);
            return (valNanoThis < valNanoOther);
        }
    }
    virtual void setData(int role, const QVariant& value)
    {
        if (role == Qt::DisplayRole)
        {

            if (m_isLink)
            {
                // Set the brush color to blue
                QBrush brush = this->foreground();
                brush.setColor(Qt::blue);
                setForeground(brush);

                // set alignment
                int itemAlignment = Qt::AlignVCenter | Qt::AlignLeft;
                setTextAlignment(itemAlignment);

                // set underline
                QFont font = this->font();
                font.setUnderline(true);
                setFont(font);
            }

            QString formattedStr = NanosecToTimeStringFormatted(value.toDouble(), true);
            QStringList strList = formattedStr.split(AC_STR_SpaceA);

            if (strList.size() == 2)
            {
                QString valStr = strList[0];
                valStr.append(AC_STR_SpaceA + strList[1]);
                QTableWidgetItem::setData(role, valStr);
            }
        }
        else
        {
            QTableWidgetItem::setData(role, value);
        }
    }
    double StrToNanoSec(const QString& orgVal, const QString& strUnit)const
    {
        double val = orgVal.toDouble();

        if (strUnit == AC_MSEC_POSTFIX)
        {
            val *= 1000000;
        }

        if (strUnit == AC_MICROSEC_POSTFIX)
        {
            val *= 1000;
        }

        return val;
    }

private:
    bool m_isLink;

};




#endif //_GPTRACESUMMARYTABLE_H_
