//=====================================================================

//=====================================================================
#ifndef _GPSUMMARYTABLE_H_
#define _GPSUMMARYTABLE_H_

// Infra:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>

class gpTraceDataContainer;
class gpTraceView;
class ProfileSessionDataItem;


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
        QString numCallsStr = (m_numCalls > 0) ? QString::number(m_numCalls) : AF_STR_NotAvailableA;
        membersStringsList << m_index;
        membersStringsList << QString::number(m_executionTimeMS, 'f', 3);
        membersStringsList << QString::number(m_minTimeMs, 'f', 3);
        membersStringsList << QString::number(m_maxTimeMs, 'f', 3);
        membersStringsList << numCallsStr;
        membersStringsList << m_gpuQueue;
        QString addressStr = (m_address.size() > 0) ? m_address : AF_STR_NotAvailableA;
        membersStringsList << addressStr;
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
    gpSummaryTable(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineAbsoluteStart); 
    virtual ~gpSummaryTable();
    
    virtual bool Init();

    // Save selection in summary table to be restored on tab switch
    // Restores selection
    void RestoreSelection();
    void SaveSelection(int row);

    // Clears selection
    void ClearSelection();

    /// Rebuilds summary table
    void Refresh(bool useTimelineScope, quint64 minTime, quint64 maxTime);

    /// obtains item from cell
    virtual ProfileSessionDataItem* GetRelatedItem(int rowIndex, int colIndex)=0;


protected:
    virtual bool InitItems() = 0;

    /// Fills table with calls from m_logic data
    virtual void FillTable()=0;

    /// Adds a row to the table
    virtual void AddSummaryRow(APISummaryInfo* pInfo) = 0;

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
            m_text = valStr;
            QTableWidgetItem::setData(role, valStr);
        }
        else
        {
            QTableWidgetItem::setData(role, value);
        }
    }
    QString text()const
    {
        return m_text;
    }
private:
    const char PERCENTAGE_SYMBOL[2] = "%";
    QString m_text;
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

#endif //_GPSUMMARYTABLE_H_
