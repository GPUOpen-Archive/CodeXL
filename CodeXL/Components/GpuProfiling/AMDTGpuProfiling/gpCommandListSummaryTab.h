//=====================================================================

//=====================================================================
#ifndef _GPCOMMANDLISTSUMMARYTAB_H_
#define _GPCOMMANDLISTSUMMARYTAB_H_

#include <AMDTGpuProfiling/gpTraceSummaryTab.h>

class APISummaryInfo;
class ProfileSessionDataItem;

class gpCommandListSummaryTab : public gpSummaryTab
{
    Q_OBJECT
public:
    gpCommandListSummaryTab(eCallType callType, quint64 timelineAbsoluteStart) :gpSummaryTab(callType, timelineAbsoluteStart) {}
    virtual void RefreshAndMaintainSelection(bool check);

    /// obtains call type
    virtual eCallType getCallType()const { return eCallType::COMMAND_LIST; }

    /// obtains call type
    virtual QString getCaption()const;

public slots :
    /// Handles selection change in summary table: updates top20 table accordingly
    virtual void OnSummaryTableSelectionChanged();
    virtual void OnSummaryTableCellClicked(int row, int col);
    virtual void OnSummaryTableCellDoubleClicked(int row, int col);
    virtual void OnTop20TableCellDoubleClicked(int row, int col);
    void SelectCommandList(const QString& commandListName);

protected:
    virtual void FillTop20List(CallIndexId callIndex, const QString& callName);
    virtual void SetTop20TableCaption(const QString& callName);
};

#endif //_GPCOMMANDLISTSUMMARYTAB_H_
