//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/APITimelineItems.cpp $
/// \version $Revision: #10 $
/// \brief This file contains the base timeline item classes used for all APIs
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/APITimelineItems.cpp#10 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

// Infra:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>

// Local:
#include <AMDTGpuProfiling/APITimelineItems.h>
#include <AMDTGpuProfiling/TraceTable.h>
#include <AMDTGpuProfiling/gpStringConstants.h>


APITimelineItem::APITimelineItem() : acAPITimelineItem(std::numeric_limits<quint64>::max(), std::numeric_limits<quint64>::min(), -1), m_pTraceTableItem(NULL)
{
}
APITimelineItem::APITimelineItem(quint64 startTime, quint64 endTime, int apiIndex) : acAPITimelineItem(startTime, endTime, apiIndex), m_pTraceTableItem(NULL)
{
}

void APITimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
    // Add the base class tooltip items:
    acAPITimelineItem::tooltipItems(tooltip);

    if (m_pTraceTableItem != NULL)
    {
        QString strDeviceTime = m_pTraceTableItem->GetColumnData(TraceTableModel::TRACE_DEVICE_TIME_COLUMN).toString();

        if (!strDeviceTime.isEmpty())
        {
            quint64 deviceTime = (quint64)(strDeviceTime.toFloat() * 1000000);
            strDeviceTime = getDurationString(deviceTime);
            tooltip.add(tr("Device Time"), strDeviceTime);
        }
    }
}

DispatchAPITimelineItem::DispatchAPITimelineItem(acAPITimelineItem* deviceItem, acAPITimelineItem* item) : APITimelineItem(item->startTime(), item->endTime(), item->apiIndex()), m_deviceItem(deviceItem)
{
    m_strText = item->text();
    m_backgroundColor = item->backgroundColor();
    m_foregroundColor = item->foregroundColor();
}

DispatchAPITimelineItem::DispatchAPITimelineItem(const QString& text) : APITimelineItem(), m_deviceItem(nullptr)
{
    m_strText = text;
}

HostAPITimelineItem::HostAPITimelineItem(quint64 startTime, quint64 endTime, int apiIndex) : APITimelineItem(startTime, endTime, apiIndex), m_pHostItem(NULL)
{
}


PerfMarkerTimelineItem::PerfMarkerTimelineItem(quint64 startTime, quint64 endTime) : acTimelineItem(startTime, endTime), m_pTraceTableItem(NULL)
{

}


CommandListTimelineItem::CommandListTimelineItem(quint64 startTime, quint64 endTime, const QString& commandListPtr): 
    acTimelineItem(startTime, endTime), m_commandListPtr(commandListPtr)
{

}

void CommandListTimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
    tooltip.add(text(), m_commandListPtr);

    quint64 timelineStartTime = 0;
    if (m_pParentBranch != NULL)
    {
        acTimeline* timeline = m_pParentBranch->parentTimeline();

        if (timeline != NULL)
        {
            timelineStartTime = timeline->startTime();
        }
    }

    // Convert the start and end times to milliseconds
    double fnumStart = (m_nStartTime - timelineStartTime) / 1e6; 
    double fnumEnd = (m_nEndTime - timelineStartTime) / 1e6; // convert to milliseconds

    quint64 duration = m_nEndTime - m_nStartTime;
    QString durationStr = NanosecToTimeString(duration, true, false);

    QString tooltipLine = QString(GPU_STR_APITimeline_TimeTooltipLine).arg(fnumStart, 0, 'f', 3).arg(fnumEnd, 0, 'f', 3).arg(durationStr);
    tooltip.add("", tooltipLine);
}
