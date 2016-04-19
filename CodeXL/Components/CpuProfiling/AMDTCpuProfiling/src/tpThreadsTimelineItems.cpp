//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpThreadsTimelineItems.cpp
///
//==================================================================================

//------------------------------ tpThreadsView.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <inc/StringConstants.h>
#include <inc/tpDisplayInfo.h>
#include <inc/tpThreadsTimelineItems.h>



tpThreadsTimelineItem::tpThreadsTimelineItem(quint64 startTime, quint64 endTime) : acTimelineItem(startTime, endTime)
{
    m_sampleStartTime = startTime;
    m_sampleEndTime = endTime;
    m_sampleDuration = endTime - startTime;
}

void tpThreadsTimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
    QString sampleStr1, sampleStr2, sampleStr3;
    tpDisplayInfo::Instance().TimeStampToString(m_sampleStartTime, sampleStr1);
    tpDisplayInfo::Instance().TimeStampToString(m_sampleEndTime, sampleStr2);
    tpDisplayInfo::Instance().TimeStampToString(m_sampleDuration, sampleStr3);
    tooltip.add(CP_STR_ThreadsTimelineSampleTooltipStartTime, sampleStr1);
    tooltip.add(CP_STR_ThreadsTimelineSampleTooltipEndTime, sampleStr2);
    tooltip.add(CP_STR_ThreadsTimelineSampleTooltipDuration, sampleStr3);
    tooltip.add(CP_STR_ThreadsTimelineSampleTooltipThread, QString(CP_STR_ThreadsTimelineSampleTooltipItemFormat).arg(m_tid));
    tooltip.add(CP_STR_ThreadsTimelineSampleTooltipCore, QString(CP_STR_ThreadsTimelineSampleTooltipItemFormat).arg(m_coreId));
    tooltip.add(CP_STR_ThreadsTimelineSampleTooltipState, m_threadStateStr);

    if (!m_waitReasonStr.isEmpty())
    {
        tooltip.add(CP_STR_ThreadsTimelineSampleTooltipWaitReason, m_waitReasonStr);
    }

    if (!m_callstack.isEmpty())
    {
        tooltip.add("Callstack:", "");
    }

    foreach (QString str, m_callstack)
    {
        tooltip.add("", str);
    }
}

void tpThreadsTimelineItem::SetSampleData(AMDTThreadId tid, AMDTUInt32 coreId, const QString& threadStateStr, const QString& waitReasonStr)
{
    m_tid = tid;
    m_coreId = coreId;
    m_threadStateStr = threadStateStr;
    m_waitReasonStr = waitReasonStr;
}
