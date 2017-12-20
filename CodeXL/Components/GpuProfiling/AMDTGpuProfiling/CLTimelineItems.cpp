//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CLTimelineItems.cpp $
/// \version $Revision: #23 $
/// \brief  This file contains the timeline item classes used for CL APIs
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CLTimelineItems.cpp#23 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>
#include "CLTimelineItems.h"
#include "TraceTable.h"


CLAPITimelineItem::CLAPITimelineItem(quint64 startTime, quint64 endTime, int apiIndex)
    : HostAPITimelineItem(startTime, endTime, apiIndex)
#ifdef SHOW_KERNEL_LAUNCH_AND_COMPLETION_LATENCY
    , m_nCompletionLatencyTime(0)
#endif
{
}

void CLAPITimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
    tooltip.add(tr("Name"), m_strText);
    tooltip.add(tr("Device"), m_strDeviceType);
    tooltip.add(tr("Command Type"), m_strCommandType);

    acTimeline* timeline = m_pParentBranch->parentTimeline();

    double fnum = (m_nQueueTime - timeline->startTime()) / 1e6; // convert to milliseconds
    QString strNum = QString(tr("%1 millisecond")).arg(fnum, 0, 'f', 3);
    tooltip.add(tr("Queued Time"), strNum);

    fnum = (m_nSubmitTime - timeline->startTime()) / 1e6; // convert to milliseconds
    strNum = QString(tr("%1 millisecond")).arg(fnum, 0, 'f', 3);
    tooltip.add(tr("Submit Time"), strNum);

    fnum = (m_nStartTime - timeline->startTime()) / 1e6; // convert to milliseconds
    strNum = QString(tr("%1 millisecond")).arg(fnum, 0, 'f', 3);
    tooltip.add(tr("Start Time"), strNum);

    fnum = (m_nEndTime - timeline->startTime()) / 1e6; // convert to milliseconds
    strNum = QString(tr("%1 millisecond")).arg(fnum, 0, 'f', 3);
    tooltip.add(tr("End Time"), strNum);

#ifdef SHOW_KERNEL_LAUNCH_AND_COMPLETION_LATENCY
    fnum = (m_nStartTime - hostItem()->startTime()) / 1e6; // convert to milliseconds
    strNum = QString(tr("%1 milliseconds")).arg(fnum, 0, 'f', 3);
    strNum.prepend("<font color=#FF0000>");
    strNum.append("</font>");
    tooltip.add(tr("<font color=#FF0000>Launch Latency</font>"), strNum);

    if (m_nCompletionLatencyTime != 0)
    {
        fnum = (m_nCompletionLatencyTime - m_nEndTime) / 1e6; // convert to milliseconds
        strNum = QString(tr("%1 milliseconds")).arg(fnum, 0, 'f', 3);
        strNum.prepend("<font color=#FF0000>");
        strNum.append("</font>");
        tooltip.add(tr("<font color=#FF0000>Completion Latency</font>"), strNum);
    }

#endif

    QString name = m_strText.toLower();

    // Add the base class tooltip items:
    QString strCPUTime;

    if (m_pTraceTableItem != NULL)
    {
        strCPUTime = m_pTraceTableItem->GetColumnData(TraceTableModel::TRACE_CPU_TIME_COLUMN).toString();

        if (!strCPUTime.isEmpty())
        {
            quint64 cpuTime = (quint64)(strCPUTime.toFloat() * 1000000);
            strCPUTime = getDurationString(cpuTime);
        }

        QString strDeviceTime = m_pTraceTableItem->GetColumnData(TraceTableModel::TRACE_DEVICE_TIME_COLUMN).toString();

        if (!strDeviceTime.isEmpty())
        {
            quint64 deviceTime = (quint64)(strDeviceTime.toFloat() * 1000000);
            strDeviceTime = getDurationString(deviceTime);
            tooltip.add(tr("Device Time"), strDeviceTime);
        }
    }
    else
    {
        quint64 duration = m_nEndTime - m_nStartTime;

        if (name.indexOf("image") >= 0 && duration < 1000)
        {
            strCPUTime = "N/A";
        }
        else
        {
            strCPUTime = getDurationString(duration);
        }
    }

    tooltip.add(tr("Duration"), strCPUTime);

    acAPITimelineItem* localHostItem = hostItem();

    tooltip.add(tr("clEnqueue API Name"), localHostItem->text());

    fnum = (localHostItem->startTime() - timeline->startTime()) / 1e6; // convert to milliseconds
    strNum = QString(tr("%1 millisecond")).arg(fnum, 0, 'f', 3);
    tooltip.add(tr("clEnqueue API Start Time"), strNum);

    fnum = (localHostItem->endTime() - timeline->startTime()) / 1e6; // convert to milliseconds
    strNum = QString(tr("%1 millisecond")).arg(fnum, 0, 'f', 3);
    tooltip.add(tr("clEnqueue API End Time"), strNum);

    quint64 duration = localHostItem->endTime() - localHostItem->startTime();
    tooltip.add(tr("clEnqueue API Duration"), getDurationString(duration));

    strNum.setNum(localHostItem->apiIndex());
    tooltip.add(tr("clEnqueue Call Index"), strNum);
}

QString CLAPITimelineItem::getDataSizeString(quint64 dataSizeInBytes, int precision)
{
    quint64 kb = 1 << 10;
    quint64 mb = 1 << 20;
    quint64 gb = 1 << 30;

    double transferSize = (double)dataSizeInBytes;
    QString transferSizeStr;

    if (transferSize > gb)
    {
        transferSize /= double(gb);
        transferSizeStr = QString(tr("%1 GB")).arg(transferSize, 0, 'f', precision);
    }
    else if (transferSize > mb)
    {
        transferSize /= double(mb);
        transferSizeStr = QString(tr("%1 MB")).arg(transferSize, 0, 'f', precision);
    }
    else if (transferSize > kb)
    {
        transferSize /= double(kb);
        transferSizeStr = QString(tr("%1 KB")).arg(transferSize, 0, 'f', precision);
    }
    else
    {
        transferSizeStr = QString(tr("%1 Byte")).arg(transferSize, 0, 'f', precision);
    }

    return transferSizeStr;
}

CLKernelTimelineItem::CLKernelTimelineItem(quint64 startTime, quint64 endTime, int apiIndex)
    : CLAPITimelineItem(startTime, endTime, apiIndex) , m_pOccupancyInfo(nullptr)
{
}

void CLKernelTimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
    CLAPITimelineItem::tooltipItems(tooltip);

    tooltip.add(tr("Global Work Size"), m_strGlobalWorkSize);
    tooltip.add(tr("Local Work Size"), m_strLocalWorkSize);

    //kernels on the CPU won't have occupancy info
    if (m_pOccupancyInfo != NULL && m_pOccupancyInfo->GetOccupancy() >= 0)
    {
        tooltip.add(tr("Kernel Occupancy"), QString("%1").number(m_pOccupancyInfo->GetOccupancy(), 'f', 2).append('%'));
    }
}

CLMemTimelineItem::CLMemTimelineItem(quint64 startTime, quint64 endTime, int apiIndex)
    : CLAPITimelineItem(startTime, endTime, apiIndex)
{
}

void CLMemTimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
    CLAPITimelineItem::tooltipItems(tooltip);

    static const quint64 kb = 1 << 10;
    static const quint64 mb = 1 << 20;
    static const quint64 gb = 1 << 30;

    quint64 elapsed = m_nEndTime - m_nStartTime;
    double elapsedInSec = elapsed * 1e-9;
    double transferRate = (double)m_nDataTransferSize / elapsedInSec;

    QString name = m_strText.toLower();
    QString transferRateStr;

    if (((name.indexOf("image") >= 0 || name.indexOf("map") >= 0) && elapsed < 1000) ||
        0 == elapsed)
    {
        transferRateStr = "N/A";
    }
    else
    {
        if (transferRate > gb)
        {
            transferRate /= double(gb);
            transferRateStr = QString(tr("%1 GB/s")).arg(transferRate, 0, 'f', 3);
        }
        else if (transferRate > mb)
        {
            transferRate /= double(mb);
            transferRateStr = QString(tr("%1 MB/s")).arg(transferRate, 0, 'f', 3);
        }
        else if (transferRate > kb)
        {
            transferRate /= double(kb);
            transferRateStr = QString(tr("%1 KB/s")).arg(transferRate, 0, 'f', 3);
        }
        else
        {
            transferRateStr = QString(tr("%1 Byte/s")).arg(transferRate, 0, 'f', 3);
        }
    }

    tooltip.add(tr("Transfer Rate"), transferRateStr);
    tooltip.add(tr("Transfer Size"), getDataSizeString(m_nDataTransferSize, 3));

}

void CLDataEnqueueOperationsTimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
    CLOtherEnqueueOperationsTimelineItem::tooltipItems(tooltip);

    tooltip.add(tr("Data Size"), getDataSizeString(m_nDataSize, 3));
}

void CLGetEventInfoTimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
    APITimelineItem::tooltipItems(tooltip);

    // remove existing "Call Index" item from ancestor
    tooltip.remove(tooltip.count() - 1);

    QString callIndexInfo;

    if (apiIndex() <= 0)
    {
        // the rare case where clGetEventInfo is the first API called
        callIndexInfo = tr("N/A (precedes index 0)");
    }
    else
    {
        callIndexInfo = QString(tr("N/A (follows index %1)")).arg(apiIndex());
    }

    // add a "Call Index" item which indicates where in the api list this api was called
    tooltip.add(tr("Call Index"), callIndexInfo);
}


