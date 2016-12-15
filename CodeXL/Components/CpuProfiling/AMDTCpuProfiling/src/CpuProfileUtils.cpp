//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileUtils.cpp
///
//==================================================================================

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <inc/CPUProfileUtils.h>


#define GET_INDEX_FOR_EVENT(index, evMask) \
    { \
        key.cpu = i; \
        key.event = evMask; \
        idxIt = nullptr; \
        index = *idxIt; \
    } \


QStringList CPUProfileUtils::m_hotSpotsBetterHigher;
QStringList CPUProfileUtils::m_hotSpotsCluMetric;

void CPUProfileUtils::AddDataArrays(gtVector<float>& destVector, const gtVector<float>& sourceVector)
{
    unsigned int min = sourceVector.size() < destVector.size() ? sourceVector.size() : destVector.size();

    for (unsigned int i = 0; i < min; i++)
    {
        destVector[i] += sourceVector[i];
    }
}

QString CPUProfileUtils::GetColumnFullName(ColumnType colType, const QString& title, const CpuEvent& hwEvent, const QString& postFix)
{
    QString retVal;

    // For value give the event full name:
    if (colType == ColumnValue)
    {
        retVal = QString(hwEvent.m_name.data()) + postFix;

        if (title == "DTLB requests")
        {
            retVal = "DTLB Requests";
        }
    }
    else
    {
        if (title == "Branch rate")
        {
            retVal = "Branch Rate";
        }
        else if (title == "Ret misp branch")
        {
            retVal = "Retired Mispredict Branch";
        }
        else if (title == "Mispredict rate")
        {
            retVal = "Mispredict Rate";
        }
        else if (title == "Mispredict ratio")
        {
            retVal = "Mispredict Ratio";
        }
        else if (title == "DTLB requests")
        {
            retVal = "DTLB Requests";
        }
        else if (title == "DTLB request rate")
        {
            retVal = "DTLB Request Rate";
        }
        else if (title == "DTLB L1M L2M rate")
        {
            retVal = "L1 DTLB and L2 DTLB miss rate";
        }
        else if (title == "DTLB L1M L2M ratio")
        {
            retVal = "L1 DTLB and L2 DTLB miss ratio";
        }
        else if (title == "DC access rate")
        {
            retVal = "Data Cache Access Rate";
        }
        else if (title == "DC miss rate")
        {
            retVal = "Data Cache Miss Rate";
        }
        else if (title == "DC miss ratio")
        {
            retVal = "Data Cache Miss Ratio";
        }
        else if (title == "IPC")
        {
            retVal = "Retired Instruction Per CPU Clock";
        }
        else if (title == "CPI")
        {
            retVal = "CPU Clock Per Retired Instruction";
        }
        else if (title == "% of cache line utilization")
        {
            retVal = "Cache Line Utilization Percentage";
        }
        else if (title == "# of line boundary crossings")
        {
            retVal = "Line boundary crossings";
        }
        else if (title == "# of bytes/L1 eviction")
        {
            retVal = "Bytes/L1 eviction";
        }
        else if (title == "# of accesses/L1 eviction")
        {
            retVal = "Accesses/L1 eviction";
        }
        else if (title == "# of L1 evictions")
        {
            retVal = "L1 Evictions";
        }
        else if (title == "# of accesses")
        {
            retVal = "Accesses";
        }
        else if (title == "# of Bytes accessed")
        {
            retVal = "Bytes Accessed";
        }
        else if (title == "Ret near RET")
        {
            retVal = "Ret near RET";
        }
        else if (title == "Ret near RET misp")
        {
            retVal = "Ret near RET Mispredicted";
        }
        else if (title == "Misp rate")
        {
            retVal = "Mispredict Rate";
        }
        else if (title == "Misp ratio")
        {
            retVal = "Mispredict Ratio";
        }
        else if (title == "Instr per call")
        {
            retVal = "Instruction per Call";
        }
        else if (title == "Ret branch")
        {
            retVal = "Retired Branch";
        }
        else if (title == "Ret taken branch")
        {
            retVal = "Retired Taken Branch";
        }
        else if (title == "Branch rate")
        {
            retVal = "Branch Rate";
        }
        else if (title == "Branch taken rate")
        {
            retVal = "Branch Taken Rate";
        }
        else if (title == "DC refills L2/sys")
        {
            retVal = "Data Cache Refills L2/System";
        }
        else if (title == "DC refill rate")
        {
            retVal = "Data Cache Refill Rate";
        }
        else if (title == "Misalign rate")
        {
            retVal = "Misalign Rate";
        }
        else if (title == "Misalign ratio")
        {
            retVal = "Misalign Ratio";
        }
        else if (title == "Ave tag-to-ret")
        {
            retVal = "Average Tag to Retired";
        }
        else if (title == "Ave comp-to-ret")
        {
            retVal = "Average Complete to Retired";
        }
        else if (title == "Ave DC miss lat")
        {
            retVal = "Average Data Cache Missed Latency";
        }
        else if (title == "L1M L2H rate")
        {
            retVal = "L1M L2H Rate";
        }
        else if (title == "L1M L2M rate")
        {
            retVal = "L1M L2M Rate";
        }
        else if (title == "Ave local lat")
        {
            retVal = "Average Local Latency";
        }
        else if (title == "Ave remote lat")
        {
            retVal = "Average Remote Latency";
        }
        else if (title == "L1 ITLB miss ratio")
        {
            retVal = "L1 ITLB Miss Ratio";
        }
        else if (title == "L2 ITLB miss ratio")
        {
            retVal = "L2 ITLB Miss Ratio";
        }
        else if (title == "IC miss ratio")
        {
            retVal = "IC Miss Ratio";
        }
        else if (title == "Ave fetch latency")
        {
            retVal = "Average Fetch Latency";
        }
        else if (title == "DTLB L1M L2H rate")
        {
            retVal = "DTLB L1M L2H Rate";
        }
        else if (title == "ITLB request rate")
        {
            retVal = "ITLB Request Rate";
        }
        else if (title == "ITLB L1M L2H rate")
        {
            retVal = "ITLB L1M L2H Rate";
        }
        else if (title == "ITLB L1M L2M rate")
        {
            retVal = "ITLB L1M L2M Rate";
        }
        else if (title == "IC fetch rate")
        {
            retVal = "IC Fetch Rate";
        }
        else if (title == "IC miss rate")
        {
            retVal = "IC Miss Rate";
        }
        else if (title == "L2 read req rate")
        {
            retVal = "L2 Read Requests Rate";
        }
        else if (title == "L2 write req rate")
        {
            retVal = "L2 Write Requests Rate";
        }
        else if (title == "L2 miss rate")
        {
            retVal = "L2 Miss Rate";
        }
    }

    return retVal;
}

bool CPUProfileUtils::IsHotSpotBetterHigher(EventsFile* pEventsFile, const QString& hotSpotCaption)
{
    GT_IF_WITH_ASSERT(pEventsFile != nullptr)
    {
        if (0 == hotSpotCaption.compare("Timer", Qt::CaseInsensitive))
        {
            // For a timer event, event file is not needed, pEventsFile may be a null
            // return false as timer is not better higher
            return false;
        }

        bool isHotSpotBetterHigher = false;

        if (m_hotSpotsBetterHigher.empty())
        {
            for (unsigned int i = 0; i < (sizeof(EventsBetterHigher) / sizeof(unsigned int)); i++)
            {
                CpuEvent cpuEv;
                pEventsFile->FindEventByValue(EventsBetterHigher[i], cpuEv);
                m_hotSpotsBetterHigher << QString(cpuEv.m_name.data());
                m_hotSpotsBetterHigher << QString(cpuEv.m_abbrev.data());
                m_hotSpotsBetterHigher << (QString(cpuEv.m_name.data()) + SUFFIX_HOTSPOT_BETTER_HIGH);
            }
        }

        if (m_hotSpotsBetterHigher.contains(hotSpotCaption))
        {
            isHotSpotBetterHigher = true;
        }

        return isHotSpotBetterHigher;
    }

    return false;
}

bool CPUProfileUtils::IsHotSpotCluMetric(EventsFile* pEventsFile, const QString& hotSpotCaption)
{
    bool isHotSpotCluMetric = false;

    if (m_hotSpotsCluMetric.empty() && (pEventsFile != nullptr))
    {
        int cluEndOffset = IBS_CLU_OFFSET(IBS_CLU_END);

        for (int i = 0; i <= cluEndOffset; i++)
        {
            CpuEvent cpuEv;
            pEventsFile->FindEventByValue((IBS_CLU_BASE + i), cpuEv);
            m_hotSpotsCluMetric << cpuEv.m_name.data();
        }
    }

    if (m_hotSpotsCluMetric.contains(hotSpotCaption))
    {
        isHotSpotCluMetric = true;
    }

    return isHotSpotCluMetric;
}
