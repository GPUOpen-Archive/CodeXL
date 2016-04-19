//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file provides the CL Kernel Summarizer
//==============================================================================

#include <sstream>
#include "AnalyzerHTMLUtils.h"
#include "CLKernelSummarizer.h"

using namespace std;

CLKernelSummarizer::CLKernelSummarizer(void)
    : KernelSummarizer<CLAPIInfo>()
{
}

CLKernelSummarizer::~CLKernelSummarizer(void)
{
}

void CLKernelSummarizer::OnParse(CLAPIInfo* pAPIInfo, bool& stopParsing)
{
    stopParsing = false;

    if (pAPIInfo->m_Type == CL_ENQUEUE_KERNEL)
    {
        CLKernelAPIInfo* pKAPI = (CLKernelAPIInfo*)pAPIInfo;

        if (!pKAPI->m_bInfoMissing)
        {
            m_KernelAPISet.insert(pKAPI);

            if (pKAPI->m_ullComplete <= pKAPI->m_ullRunning)
            {
                return;
            }

            // kernel summary
            ULONGLONG duration = pKAPI->m_ullComplete - pKAPI->m_ullRunning;

            m_ullTotalDuration += duration;

            // Append device name to kernel name
            stringstream tmpss;
            tmpss << pKAPI->m_strKernelName << '[' << pKAPI->m_strDevice << ']';
            string keyKernelName = tmpss.str();

            KernelSumMap::iterator it = m_KernelSumMap.find(keyKernelName);

            if (it == m_KernelSumMap.end())
            {
                KernelSummaryItems si;
                si.strKernelName = pKAPI->m_strKernelName;
                si.strDeviceName = pKAPI->m_strDevice;
                si.uiNumCalls = 1;
                si.ullTotalTime = duration;
                si.ullAve = si.ullMax = si.ullMin = duration;
                si.uiMaxCallIndex = pKAPI->m_uiSeqID;
                si.uiMinCallIndex = pKAPI->m_uiSeqID;
                si.maxTid = pKAPI->m_tid;
                si.minTid = pKAPI->m_tid;

                // new api type
                m_KernelSumMap[ keyKernelName ] = si;
            }
            else
            {
                KernelSummaryItems& si = it->second;
                si.ullTotalTime += duration;
                si.uiNumCalls++;

                if (duration > si.ullMax)
                {
                    si.ullMax = duration;
                    si.uiMaxCallIndex = pKAPI->m_uiSeqID;
                    si.maxTid = pKAPI->m_tid;
                }
                else if (duration < si.ullMin)
                {
                    si.ullMin = duration;
                    si.uiMinCallIndex = pKAPI->m_uiSeqID;
                    si.minTid = pKAPI->m_tid;
                }

                si.ullAve = si.ullTotalTime / si.uiNumCalls;
            }
        }
    }
}

void CLKernelSummarizer::GenerateTopXKernelHTMLTable(std::ostream& sout)
{
    HTMLTable table;
    table.AddColumn("Kernel Name")
    .AddColumn("Context ID")
    .AddColumn("Command Queue ID")
    .AddColumn("Device Name")
    .AddColumn("Duration(ms)", true, true)
    .AddColumn("Global Work Size")
    .AddColumn("Work Group Size")
    .AddColumn("Thread ID")
    .AddColumn("Call Index");

    unsigned int c = 0;

    for (std::multiset<CLKernelAPIInfo*, cmp_type>::reverse_iterator it = m_KernelAPISet.rbegin(); it != m_KernelAPISet.rend(); it++)
    {
        if (c > m_uiTopX)
        {
            break;
        }

        HTMLTableRow row(&table);
        CLKernelAPIInfo* info = *it;

        std::string keyValues;
        keyValues = GenerateHTMLKeyValue(gs_THREAD_ID_TAG, info->m_tid);
        keyValues = AppendHTMLKeyValue(keyValues, GenerateHTMLKeyValue(gs_SEQUENCE_ID_TAG, info->m_uiSeqID));
        keyValues = AppendHTMLKeyValue(keyValues, GenerateHTMLKeyValue(gs_VIEW_TAG, gs_VIEW_TIMELINE_DEVICE_TAG));
        std::string hRef = GenerateHref(keyValues, info->m_strKernelName);

        row.AddItem(0, hRef)
        .AddItem(1, StringUtils::ToString(info->m_uiContextID))
        .AddItem(2, StringUtils::ToString(info->m_uiQueueID))
        .AddItem(3, info->m_strDevice)
        .AddItem(4, StringUtils::NanosecToMillisec(info->m_ullComplete - info->m_ullRunning))
        .AddItem(5, info->m_strGlobalWorkSize)
        .AddItem(6, info->m_strGroupWorkSize)
        .AddItem(7, StringUtils::ToString(info->m_tid))
        .AddItem(8, info->m_bHasDisplayableSeqId ? StringUtils::ToString(info->m_uiDisplaySeqID) : "N/A");
        table.AddRow(row);
        c++;
    }

    table.WriteToStream(sout);
}

std::string CLKernelSummarizer::GetMinMaxItemHRef(bool isMaxItem, KernelSummaryItems* pItem)
{
    unsigned int uiMinMaxIndex;
    unsigned long long uiMinMax;
    osThreadId minMaxThreadId;

    if (isMaxItem)
    {
        uiMinMaxIndex = pItem->uiMaxCallIndex;
        uiMinMax = pItem->ullMax;
        minMaxThreadId = pItem->maxTid;
    }
    else
    {
        uiMinMaxIndex = pItem->uiMinCallIndex;
        uiMinMax = pItem->ullMin;
        minMaxThreadId = pItem->minTid;
    }

    std::string keyValues;
    keyValues = GenerateHTMLKeyValue(gs_THREAD_ID_TAG, minMaxThreadId);
    keyValues = AppendHTMLKeyValue(keyValues, GenerateHTMLKeyValue(gs_SEQUENCE_ID_TAG, uiMinMaxIndex));
    keyValues = AppendHTMLKeyValue(keyValues, GenerateHTMLKeyValue(gs_VIEW_TAG, gs_VIEW_TIMELINE_DEVICE_TAG));
    std::string hRef = GenerateHref(keyValues, StringUtils::NanosecToMillisec(uiMinMax));

    return hRef;
}
