//=====================================================================

//=====================================================================
#ifndef _GPTRACESUMMARYTAB_H_
#define _GPTRACESUMMARYTAB_H_

#include <AMDTGpuProfiling/gpSummaryTab.h>


class gpTraceSummaryTab : public gpSummaryTab
{
    Q_OBJECT
public:
    gpTraceSummaryTab(eCallType callType, quint64 timelineAbsoluteStart) :gpSummaryTab(callType, timelineAbsoluteStart) {}
    virtual void RefreshAndMaintainSelection(bool check);

public slots :
    /// Handles selection change in summary table: updates top20 table accordingly
    virtual void OnSummaryTableSelectionChanged();
    virtual void OnTop20TableCellDoubleClicked(int row, int col);
    virtual void OnSummaryTableCellClicked(int row, int col);
    virtual void OnSummaryTableCellDoubleClicked(int, int) {}

protected:
    virtual void FillTop20List(CallIndexId callIndex, const QString& callName);
    virtual void SetTop20TableCaption(const QString& callName);

};

class gpCPUTraceSummaryTab : public gpTraceSummaryTab
{
    Q_OBJECT
public:
    gpCPUTraceSummaryTab(eCallType callType, quint64 timelineAbsoluteStart) :gpTraceSummaryTab(callType, timelineAbsoluteStart) {}

    /// obtains call type
    virtual eCallType getCallType()const { return eCallType::API_CALL; }

    /// obtains call type
    virtual QString getCaption()const{ return GPU_STR_API_Summary; };

};

class gpGPUTraceSummaryTab : public gpTraceSummaryTab
{
    Q_OBJECT
public:
    gpGPUTraceSummaryTab(eCallType callType, quint64 timelineAbsoluteStart) :gpTraceSummaryTab(callType, timelineAbsoluteStart) {}

    /// obtains call type
    virtual eCallType getCallType()const { return eCallType::GPU_CALL; }

    /// obtains call type
    virtual QString getCaption()const{ return GPU_STR_GPU_Summary; };

};

#endif //_GPTRACESUMMARYTAB_H_
