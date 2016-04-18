//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This class generates the HSA kernel dispatch summary page
//==============================================================================

#include <sstream>
#include "AnalyzerHTMLUtils.h"
#include "HSAKernelSummarizer.h"
#include "../Common/Logger.h"

using namespace std;
using namespace GPULogger;

HSAKernelSummarizer::HSAKernelSummarizer(void)
    : KernelSummarizer<HSAAPIInfo>()
{
}

HSAKernelSummarizer::~HSAKernelSummarizer(void)
{
}

void HSAKernelSummarizer::OnParse(HSAAPIInfo* pAPIInfo, bool& stopParsing)
{
    stopParsing = false;

    // only look at non-APIs becuase in HSA, there is no kernel dispatch API
    if (!pAPIInfo->m_bIsAPI && HSA_API_Type_Non_API_Dispatch == pAPIInfo->m_apiID)
    {
        HSADispatchInfo* pDispatch = dynamic_cast<HSADispatchInfo*>(pAPIInfo);
        SpAssertRet(pDispatch != NULL);
        m_KernelAPISet.insert(pDispatch);

        // kernel summary
        ULONGLONG duration = pDispatch->m_ullEnd - pDispatch->m_ullStart;

        m_ullTotalDuration += duration;

        // Append device name to kernel name
        stringstream tmpss;
        tmpss << pDispatch->m_strKernelName << '[' << pDispatch->m_strDeviceName << ']';
        string keyKernelName = tmpss.str();

        KernelSumMap::iterator it = m_KernelSumMap.find(keyKernelName);

        if (it == m_KernelSumMap.end())
        {
            KernelSummaryItems si;
            si.strKernelName = pDispatch->m_strKernelName;
            si.strDeviceName = pDispatch->m_strDeviceName;
            si.uiNumCalls = 1;
            si.ullTotalTime = duration;
            si.ullAve = si.ullMax = si.ullMin = duration;
            si.uiMaxCallIndex = pDispatch->m_uiSeqID;
            si.uiMinCallIndex = pDispatch->m_uiSeqID;

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
                si.uiMaxCallIndex = pDispatch->m_uiSeqID;
            }
            else if (duration < si.ullMin)
            {
                si.ullMin = duration;
                si.uiMinCallIndex = pDispatch->m_uiSeqID;
            }

            si.ullAve = si.ullTotalTime / si.uiNumCalls;
        }
    }
}

void HSAKernelSummarizer::GenerateTopXKernelHTMLTable(std::ostream& sout)
{
    SP_TODO("Add back in Global Work Size and Work Group Size, once we can get that data for a dispatch");
    HTMLTable table;
    table.AddColumn("Kernel Name")
    .AddColumn("Device Name")
    .AddColumn("Duration(ms)", true, true);

    unsigned int c = 0;

    for (std::multiset<HSADispatchInfo*, cmp_type>::reverse_iterator it = m_KernelAPISet.rbegin(); it != m_KernelAPISet.rend(); it++)
    {
        if (c > m_uiTopX)
        {
            break;
        }

        HTMLTableRow row(&table);
        HSADispatchInfo* info = *it;

        std::string keyValues;
        keyValues = GenerateHTMLKeyValue(gs_SEQUENCE_ID_TAG, info->m_uiSeqID);
        keyValues = AppendHTMLKeyValue(keyValues, GenerateHTMLKeyValue(gs_VIEW_TAG, gs_VIEW_TIMELINE_DEVICE_NO_API_TAG));
        std::string hRef = GenerateHref(keyValues, info->m_strKernelName);

        row.AddItem(0, hRef)
        .AddItem(1, info->m_strDeviceName)
        .AddItem(2, StringUtils::NanosecToMillisec(info->m_ullEnd - info->m_ullStart));
        table.AddRow(row);
        c++;
    }

    table.WriteToStream(sout);
}

std::string HSAKernelSummarizer::GetMinMaxItemHRef(bool isMaxItem, KernelSummaryItems* pItem)
{
    std::stringstream ssMinMaxItem;
    unsigned int uiMinMaxIndex;
    unsigned long long uiMinMax;

    if (isMaxItem)
    {
        uiMinMaxIndex = pItem->uiMaxCallIndex;
        uiMinMax = pItem->ullMax;
    }
    else
    {
        uiMinMaxIndex = pItem->uiMinCallIndex;
        uiMinMax = pItem->ullMin;
    }

    std::string keyValues;
    keyValues = GenerateHTMLKeyValue(gs_SEQUENCE_ID_TAG, uiMinMaxIndex);
    keyValues = AppendHTMLKeyValue(keyValues, GenerateHTMLKeyValue(gs_VIEW_TAG, gs_VIEW_TIMELINE_DEVICE_NO_API_TAG));
    std::string hRef = GenerateHref(keyValues, StringUtils::NanosecToMillisec(uiMinMax));

    return hRef;
}
