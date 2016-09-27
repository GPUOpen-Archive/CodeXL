//=====================================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/HSATimelineItems.cpp $
/// \version $Revision: #8 $
/// \brief  This file contains the timeline item classes used for HSA APIs
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/HSATimelineItems.cpp#8 $
// Last checkin:   $DateTime: 2016/01/07 12:10:47 $
// Last edited by: $Author: chesik $
// Change list:    $Change: 554414 $
//=====================================================================


#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>
#include "HSATimelineItems.h"


HSADispatchTimelineItem::HSADispatchTimelineItem(quint64 startTime, quint64 endTime, int apiIndex)
    : HostAPITimelineItem(startTime, endTime, apiIndex), m_pOccupancyInfo(NULL)
{
}

void HSADispatchTimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
    tooltip.add(tr("Name"), m_strText);
    tooltip.add(tr("Device"), m_strDeviceType);

    acTimeline* timeline = m_pParentBranch->parentTimeline();

    double fnum = (m_nStartTime - timeline->startTime()) / 1e6; // convert to milliseconds
    QString strNum = QString(tr("%1 millisecond")).arg(fnum, 0, 'f', 3);
    tooltip.add(tr("Start Time"), strNum);

    fnum = (m_nEndTime - timeline->startTime()) / 1e6; // convert to milliseconds
    strNum = QString(tr("%1 millisecond")).arg(fnum, 0, 'f', 3);
    tooltip.add(tr("End Time"), strNum);

    QString name = m_strText.toLower();
    quint64 duration = m_nEndTime - m_nStartTime;

    tooltip.add(tr("Duration"), getDurationString(duration));

    // remove temporarily until we add HSA support back into CodeXL for CodeXL 1.8
    /*
    qcAPITimelineItem* localHostItem = hostItem();

    tooltip.add(tr("HSA Dispatch API Name"), localHostItem->text());

    fnum = (localHostItem->startTime() - timeline->startTime()) / 1e6; // convert to milliseconds
    strNum = QString(tr("%1 millisecond")).arg(fnum, 0, 'f', 3);
    tooltip.add(tr("HSA Dispatch Start Time"), strNum);

    fnum = (localHostItem->endTime() - timeline->startTime()) / 1e6; // convert to milliseconds
    strNum = QString(tr("%1 millisecond")).arg(fnum, 0, 'f', 3);
    tooltip.add(tr("HSA Dispatch API End Time"), strNum);

    duration = localHostItem->endTime() - localHostItem->startTime();
    tooltip.add(tr("HSA Dispatch API Duration"), getDurationString(duration));

    strNum.setNum(localHostItem->apiIndex());
    tooltip.add(tr("HSA Dispatch Call Index"), strNum);

    tooltip.add(tr("Global Work Size"), m_strGlobalWorkSize);
    tooltip.add(tr("Local Work Size"), m_strLocalWorkSize);

    //kernels on the CPU won't have occupancy info
    if (m_pOccupancyInfo != NULL && m_pOccupancyInfo->GetOccupancy() >= 0)
    {
        tooltip.add(tr("Kernel Occupancy"), QString("%1").number(m_pOccupancyInfo->GetOccupancy(), 'f', 2).append('%'));
    }
    */
}

HSAMemoryTimelineItem::HSAMemoryTimelineItem(quint64 startTime, quint64 endTime, int apiIndex, size_t size)
    : HostAPITimelineItem(startTime, endTime, apiIndex), m_size(size)
{
}

void HSAMemoryTimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
    // Add the base class tooltip items:
    acAPITimelineItem::tooltipItems(tooltip);

    tooltip.add(tr("Size"), QString(tr("%1")).arg(m_size));
}

HSAMemoryTransferTimelineItem::HSAMemoryTransferTimelineItem(quint64 transferStartTime, quint64 transferEndTime, int apiIndex, size_t size, QString srcAgent, QString dstAgent)
    : HSAMemoryTimelineItem(transferStartTime, transferEndTime, apiIndex, size), m_srcAgent(srcAgent), m_dstAgent(dstAgent)
{
}

void HSAMemoryTransferTimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
    tooltip.add(tr("Name"), m_strText);
    tooltip.add(tr("Source Agent Handle"), m_srcAgent);
    tooltip.add(tr("Destination Agent Handle"), m_dstAgent);

    acTimeline* timeline = m_pParentBranch->parentTimeline();

    double fnum = (m_nStartTime - timeline->startTime()) / 1e6; // convert to milliseconds
    QString strNum = QString(tr("%1 millisecond")).arg(fnum, 0, 'f', 3);
    tooltip.add(tr("Data Transfer Start Time"), strNum);

    fnum = (m_nEndTime - timeline->startTime()) / 1e6; // convert to milliseconds
    strNum = QString(tr("%1 millisecond")).arg(fnum, 0, 'f', 3);
    tooltip.add(tr("Data Transfer End Time"), strNum);

    QString name = m_strText.toLower();
    quint64 duration = m_nEndTime - m_nStartTime;

    tooltip.add(tr("Data Transfer Duration"), getDurationString(duration));

    static const quint64 kb = 1 << 10;
    static const quint64 mb = 1 << 20;
    static const quint64 gb = 1 << 30;

    double elapsedInSec = duration * 1e-9;
    double transferRate = (double)m_size / elapsedInSec;

    QString transferRateStr;

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

    tooltip.add(tr("Transfer Rate"), transferRateStr);

    acAPITimelineItem* localHostItem = hostItem();

    tooltip.add(tr("Host API Name"), localHostItem->text());

    fnum = (localHostItem->startTime() - timeline->startTime()) / 1e6; // convert to milliseconds
    strNum = QString(tr("%1 millisecond")).arg(fnum, 0, 'f', 3);
    tooltip.add(tr("Host API Start Time"), strNum);

    fnum = (localHostItem->endTime() - timeline->startTime()) / 1e6; // convert to milliseconds
    strNum = QString(tr("%1 millisecond")).arg(fnum, 0, 'f', 3);
    tooltip.add(tr("Host API End Time"), strNum);

    duration = localHostItem->endTime() - localHostItem->startTime();
    tooltip.add(tr("Host API Duration"), getDurationString(duration));

    strNum.setNum(localHostItem->apiIndex());
    tooltip.add(tr("Host Call Index"), strNum);
}
