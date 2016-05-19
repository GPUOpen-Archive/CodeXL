
// Infra:
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTGpuProfiling/DXTimelineItems.h>
#include <AMDTGpuProfiling/gpStringConstants.h>



gpAPITimelineItem::gpAPITimelineItem(quint64 startTime, quint64 endTime, int apiIndex)
    : APITimelineItem(startTime, endTime, apiIndex)
{
}

void gpAPITimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
    QString functionName;

    if (m_interfaceName.isEmpty())
    {
        functionName = m_strText;
    }
    else
    {
        functionName = QString("%1::%2").arg(m_interfaceName).arg(m_strText);
    }

    quint64 timelineStartTime = 0;

    if (m_pParentBranch != NULL)
    {
        acTimeline* pTimeline = m_pParentBranch->parentTimeline();

        if (pTimeline != NULL)
        {
            timelineStartTime = pTimeline->startTime();
        }
    }

    double fnumStart = (m_nStartTime - timelineStartTime) / 1e6; // convert to milliseconds
    double fnumEnd = (m_nEndTime - timelineStartTime) / 1e6; // convert to milliseconds

    quint64 duration = m_nEndTime - m_nStartTime;
    QString durationStr = NanosecToTimeString(duration, true, false);

    QString tooltipLine1 = QString(GPU_STR_DXAPITimeline_tooltipLine1).arg(m_nApiIndex).arg(functionName);
    QString tooltipLine2 = QString(GPU_STR_APITimeline_TimeTooltipLine).arg(fnumStart, 0, 'f', 3).arg(fnumEnd, 0, 'f', 3).arg(durationStr);
    tooltip.add("", tooltipLine1);
    tooltip.add("", tooltipLine2);
}
