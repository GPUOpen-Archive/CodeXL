//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileUtils.cpp
///
//==================================================================================

// Infra:
#include <AMDTBaseTools/Include/gtASCIIStringTokenizer.h>

// Qt:
#include <QtWidgets>

/// Local:
#include <inc/CPUProfileUtils.h>


QStringList CPUProfileUtils::m_hotSpotsBetterHigher;
QStringList CPUProfileUtils::m_hotSpotsCluMetric;


gtUInt32 CPUProfileUtils::ConvertAggregatedSampleToArray(const AggregatedSample& sm, gtVector<float>& dataVector, gtVector<float>& totalVector, SessionDisplaySettings* pSessionDisplaySettings, bool shouldClearTotal)
{
    gtUInt32 total = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT(pSessionDisplaySettings != nullptr)
    {
        if (shouldClearTotal)
        {
            totalVector.clear();
            totalVector.resize(pSessionDisplaySettings->m_availableDataColumnCaptions.size() + 1);
        }

        //CpuEventViewIndexMap::const_iterator eventToIdxItEnd = pSessionDisplaySettings->m_eventToIndexMap.end();
        CpuEventViewIndexMap::const_iterator eventToIdxItEnd = nullptr;
        TotalIndicesMap::const_iterator totalValuesItEnd = pSessionDisplaySettings->m_totalValuesMap.end();
        ComplexDependentMap::const_iterator complexItEnd = pSessionDisplaySettings->m_calculatedDataColumns.end();

        CpuProfileSampleMap::const_iterator sampleIt = sm.getBeginSample();
        CpuProfileSampleMap::const_iterator sampleItEnd = sm.getEndSample();

        for (; sampleIt != sampleItEnd; ++sampleIt)
        {
            gtUInt32 sampleCount = sampleIt->second;
            total += sampleCount;

            SampleKeyType key(sampleIt->first.cpu, sampleIt->first.event);

            CpuEventViewIndexMap::const_iterator eventToIdxIt = nullptr;
            //CpuEventViewIndexMap::const_iterator eventToIdxIt = pSessionDisplaySettings->m_eventToIndexMap.find(key);

            //if event is not shown for this view, skip
            if (eventToIdxItEnd == eventToIdxIt)
            {
                continue;
            }

            // Given cpu/event select from profile, find column index:
            int index = *eventToIdxIt;
            TotalIndicesMap::const_iterator totalValuesIt = pSessionDisplaySettings->m_totalValuesMap.find(index);

            if (totalValuesItEnd == totalValuesIt)
            {
                continue;
            }

            GT_IF_WITH_ASSERT((index >= 0) && (index < (int)dataVector.size()))
            {
                // Aggregate total:
                int totalIndex = totalValuesIt->first;
                totalVector[totalIndex] += sampleCount;

                // Aggregate data
                if (dataVector.empty())
                {
                    continue;
                }

                dataVector[index] += sampleCount;

                // If part of complex column, re-calculate:
                ComplexDependentMap::const_iterator complexIt = pSessionDisplaySettings->m_calculatedDataColumns.find(index);

                if (complexItEnd != complexIt)
                {
                    ListOfComplex::const_iterator cpxIt = complexIt->begin();
                    ListOfComplex::const_iterator cpxItEnd = complexIt->end();

                    for (; cpxIt != cpxItEnd; ++cpxIt)
                    {
                        float calc = SessionDisplaySettings::setComplex(*cpxIt, dataVector);
                        int complexIndex = (*cpxIt).columnIndex;
                        GT_IF_WITH_ASSERT((complexIndex >= 0) && (complexIndex < (int)dataVector.size()))
                        {
                            dataVector[complexIndex] = calc;
                        }
                    }
                }
            }
        }

        // Accumulate totals also:
        if (!dataVector.empty())
        {
            dataVector[pSessionDisplaySettings->m_availableDataColumnCaptions.size()] += total;
        }
    }

    return total;
}

void CPUProfileUtils::AddDataArrays(gtVector<float>& destVector, const gtVector<float>& sourceVector)
{
    unsigned int min = sourceVector.size() < destVector.size() ? sourceVector.size() : destVector.size();

    for (unsigned int i = 0; i < min; i++)
    {
        destVector[i] += sourceVector[i];
    }
}

#define GET_INDEX_FOR_EVENT(index, evMask) \
    { \
        key.cpu = i; \
        key.event = evMask; \
        idxIt = nullptr; \
        index = *idxIt; \
    } \

    void CPUProfileUtils::CalculateCluMetrics(SessionDisplaySettings* pSessionDisplaySettings, gtVector<float>& dataVector)
    {
        EventMaskType evCluPercent = EncodeEvent(DE_IBS_CLU_PERCENTAGE, 0, true, true);
        EventMaskType evBytePerEvict = EncodeEvent(DE_IBS_CLU_BYTE_PER_EVICT, 0, true, true);
        EventMaskType evAccessPerEvict = EncodeEvent(DE_IBS_CLU_ACCESS_PER_EVICT, 0, true, true);
        EventMaskType evEvicts = EncodeEvent(DE_IBS_CLU_EVICT_COUNT, 0, true, true);
        EventMaskType evAccesses = EncodeEvent(DE_IBS_CLU_ACCESS_COUNT, 0, true, true);
        EventMaskType evBytes = EncodeEvent(DE_IBS_CLU_BYTE_COUNT, 0, true, true);

        gtVector<float> DataVectorIn;
        DataVectorIn.clear();
        DataVectorIn.insert(DataVectorIn.end(), dataVector.begin(), dataVector.end());

        for (int i = 0; i < pSessionDisplaySettings->m_cpuCount; i++)
        {
            if (pSessionDisplaySettings->m_cpuFilter.contains(i))
            {
                continue;
            }

            SampleKeyType key;
            CpuEventViewIndexMap::const_iterator idxIt;

            int idxCluPercent;
            GET_INDEX_FOR_EVENT(idxCluPercent, evCluPercent);
            int idxBytePerEvict;
            GET_INDEX_FOR_EVENT(idxBytePerEvict, evBytePerEvict);
            int idxAccessPerEvict;
            GET_INDEX_FOR_EVENT(idxAccessPerEvict, evAccessPerEvict);
            int idxEvicts;
            GET_INDEX_FOR_EVENT(idxEvicts, evEvicts);
            int idxAccesses;
            GET_INDEX_FOR_EVENT(idxAccesses, evAccesses);
            int idxBytes;
            GET_INDEX_FOR_EVENT(idxBytes, evBytes);

            if (DataVectorIn[idxEvicts] != 0)
            {
                dataVector[idxCluPercent] = ((DataVectorIn[idxCluPercent] / 64) / DataVectorIn[idxEvicts]) * 100;
                dataVector[idxAccessPerEvict] = DataVectorIn[idxAccesses] / DataVectorIn[idxEvicts];
                dataVector[idxBytePerEvict] = DataVectorIn[idxBytes] / DataVectorIn[idxEvicts];
            }
            else
            {
                dataVector[idxCluPercent] = 0;
                dataVector[idxAccessPerEvict] = 0;
                dataVector[idxBytePerEvict] = 0;
            }

            if ((pSessionDisplaySettings->m_separateBy == SEPARATE_BY_NONE))
            {
                // In case sample data is not separated by core/numa, no need to repeate the loop
                // calculation of clu metrics is done, avoid duplicate work
                break;
            }
        }
    }

    QString CPUProfileUtils::GetColumnFullName(ColumnType colType, const QString& title, const CpuEvent& hwEvent, const QString& postFix)
    {
        QString retVal = "";

        // For value give the event full name:
        if (colType == ColumnValue)
        {
            retVal = hwEvent.name + postFix;

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
                retVal = "Accessess";
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
        // Sanity check:
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
                    m_hotSpotsBetterHigher << cpuEv.name;
                    m_hotSpotsBetterHigher << cpuEv.abbrev;
                    m_hotSpotsBetterHigher << (cpuEv.name + SUFFIX_HOTSPOT_BETTER_HIGH);
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
                m_hotSpotsCluMetric << cpuEv.name;
            }
        }

        if (m_hotSpotsCluMetric.contains(hotSpotCaption))
        {
            isHotSpotCluMetric = true;
        }

        return isHotSpotCluMetric;
    }
