//=====================================================================

//=====================================================================
#ifndef _GPTRACESUMMARYTABLE_H_
#define _GPTRACESUMMARYTABLE_H_

#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

class gpTraceDataContainer;
class gpTraceView;
class APISummaryInfo;
class ProfileSessionDataItem;
class gpTraceSummaryTable;
class acTimelineBranch;

/// Summary mode enum
enum eCallType
{
    /// API Summary
    API_CALL = 0,
    /// GPU Summary
    GPU_CALL,
    MAX_TYPE
};


typedef unsigned int CallIndexId;
/////////////////////////////////////////////////////////////////////////////EOF//////////////////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------------------------
// Class Name:          gpSummaryLogic
// General Description: Collects all calls of a given type and collates them by call name
// ----------------------------------------------------------------------------------
class gpSummaryLogic
{
public:
    /// class constructor.
    gpSummaryLogic(): m_pSessionDataContainer(nullptr), m_pTraceView(nullptr) {}

    /// class destructor.
    virtual ~gpSummaryLogic() { Cleanup(); }

    void Init(eCallType callType, gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView);

    /// Rebuilds the summary info map
    void RebuildSummaryMap(bool useScope, quint64 startTime, quint64 endTime);

    /// Returns a summary info item by call name
    APISummaryInfo GetSummaryInfo(int apiCall);

    /// Returns a map of call items
    const QMap<CallIndexId, ProfileSessionDataItem*>& GetAllApiCallItemsMap()const;

    /// Returns a map of summary info items
    const QMap<CallIndexId, APISummaryInfo>& GetApiCallInfoSummaryMap()const;

private:
    /// Fills table only with calls within the timeline start and end
    void BuildSummaryMapInTimelineScope(quint64 min, quint64 max);

    void InsertSummaryInfoToMap(APISummaryInfo& info);

    /// Collects calls into given map
    void CollateAllItemsIntoSummaryMap(const QMap<CallIndexId, ProfileSessionDataItem*>& callMap);

    /// Fill map with API calls data
    void InitAPIItems(gpTraceDataContainer* pDataContainer);

    /// Fill APISummaryInfo with relevant data fromProfileSessionDataItem
    void AddSessionItemToSummaryInfo(APISummaryInfo& info, ProfileSessionDataItem* pItem, unsigned int apiId);

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
    QMap<CallIndexId, APISummaryInfo> m_apiCallInfoSummaryMap;
    QMap<CallIndexId, APISummaryInfo>::const_iterator m_apiCallInfoSummaryMapIter;
    QMap<osThreadId, acTimelineBranch*> m_threadTimetineBranchMap;
    quint64 m_totalTimeMs;
    quint64 m_numTotalCalls;
};


// ----------------------------------------------------------------------------------
// Class Name:          gpTraceSummaryTable
// General Description: displays all calls summary of a given type (API / GPU)
// ----------------------------------------------------------------------------------
class gpTraceSummaryTable : public acListCtrl
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
    /// class constructor.
    gpTraceSummaryTable(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, eCallType callType);

    /// class destructor.
    virtual ~gpTraceSummaryTable();

    /// Selects the row which describes the given call
    void SelectRowByCallName(const QString& callName);

    /// Returns a ProfileSessionDataItem* which belong to the specified call
    ProfileSessionDataItem* GetRelatedItem(int rowIndex, int colIndex);

    /// Rebuilds summary table
    void Refresh(bool useTimelineScope, quint64 minTime, quint64 maxTime);

    /// Returns a map of call items
    const QMap<CallIndexId, ProfileSessionDataItem*>& GetAllApiCallItemsMap()const;

    /// Returns an item by table row index
    bool GetItemCallIndex(int row, CallIndexId& callIndex, QString& callName)const;

    // Save selection in summary table to be restored on tab switch
    // Restores selection
    void RestoreSelection();
    void SaveSelection(int row);

    // Clears selection
    void ClearSelection();

protected slots:
    /// Change the cursor according to the entered item
    /// \param row the entered item row
    /// \param col the entered item column
    void OnCellEntered(int row, int column);

private:
    /// Fills table with calls from m_logic data
    void FillTable();

    /// Adds a row to the table
    void AddSummaryRow(int rowIndex, APISummaryInfo* pInfo);

    /// Collects API / GPU calls and sorts them
    gpSummaryLogic m_logic;

    /// Determines whether the table handles API or GPU items
    eCallType m_callType;

    /// The session data container
    gpTraceDataContainer* m_pSessionDataContainer;

    /// A pointer to the parent session view
    gpTraceView* m_pTraceView;

    int m_lastSelectedRowIndex;
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
        m_callIndex = 0;
        m_cumulativeTime = 0;
        m_percentageOfTotalTime = 0;
        m_numCalls  = 0;
        m_avgTimeMs = 0;
        m_maxTimeMs = 0;
        m_minTimeMs = INT_MIN;
    }
    void TableItemsAsString(QStringList& membersStringsList)
    {
        // m_avgTimeMs is calculated here, after all samples were gathered
        m_avgTimeMs = (float)((float)m_cumulativeTime / (float)m_numCalls);

        membersStringsList << m_call;
        membersStringsList << QString::number(m_maxTimeMs, 'f', 6);
        membersStringsList << QString::number(m_minTimeMs, 'f', 6);
        membersStringsList << QString::number(m_avgTimeMs, 'f', 6);
        membersStringsList << QString::number(m_cumulativeTime, 'f', 6);
        membersStringsList << QString::number(m_percentageOfTotalTime, 'f', 3);
        membersStringsList << QString::number(m_numCalls);
    }

    QString m_interface;
    QString m_call;
    int m_callIndex;
    quint64 m_cumulativeTime;
    float m_percentageOfTotalTime;
    int m_numCalls;
    float m_avgTimeMs;
    float m_maxTimeMs;
    float m_minTimeMs;
    QColor m_typeColor;
    ProfileSessionDataItem* m_pMinTimeItem;
    ProfileSessionDataItem* m_pMaxTimeItem;
};

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
        QString valStr = QString::number(value.toDouble());
        valStr.append(PERCENTAGE_SYMBOL);
        QTableWidgetItem::setData(role, valStr);
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
