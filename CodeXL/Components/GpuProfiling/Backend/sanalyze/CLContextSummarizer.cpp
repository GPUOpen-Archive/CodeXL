//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file provides a CL Context Summarizer
//==============================================================================

#include <fstream>
#include <sstream>
#include "CLContextSummarizer.h"
#include "../Common/HTMLTable.h"
#include "../Common/StringUtils.h"

using std::ofstream;
using std::string;
using std::vector;
using std::pair;
using std::stringstream;

CLContextSummarizer::CLContextSummarizer(void)
{
}

CLContextSummarizer::~CLContextSummarizer(void)
{
}

void CLContextSummarizer::FlushTmpCounters(string& strCntx, ContextSummaryItems* pItems)
{
    std::map< std::string, CLObjectCounter >::iterator it = m_tmpCLObjCounter.find(strCntx);

    if (it != m_tmpCLObjCounter.end())
    {
        pItems->uiNumBuffer += it->second.uiBufferCount;
        pItems->uiNumImage += it->second.uiImageCount;
        pItems->uiNumQueue += it->second.uiQueueCount;
        // after we flush counters for image and buffer, remove it from tmp map so that when context handle get reused, we won't count
        // incorrect number of cl objects
        m_tmpCLObjCounter.erase(it);
    }
}

void CLContextSummarizer::OnParse(CLAPIInfo* pAPIInfo, bool& stopParsing)
{
    stopParsing = false;

    if (pAPIInfo->m_Type == CL_ENQUEUE_MEM)
    {
        CLMemAPIInfo* pMAPI = (CLMemAPIInfo*)pAPIInfo;

        if (pMAPI->m_bInfoMissing)
        {
            return;
        }

        ContextSumMap::iterator it = m_ContextSumMap.find(pMAPI->m_uiContextID);

        ContextSummaryItems items;
        ContextSummaryItems* pItems;

        if (it != m_ContextSumMap.end())
        {
            pItems = &it->second;
        }
        else
        {
            pItems = &items;
            pItems->uiContextID = pMAPI->m_uiContextID;
        }

        FlushTmpCounters(pMAPI->m_strCntxHandle, pItems);

        if (pMAPI->m_strCMDType.find("COPY") != string::npos)
        {
            pItems->uiByteCopy += pMAPI->m_uiTransferSize;
            ULONGLONG dur = pMAPI->m_ullComplete - pMAPI->m_ullRunning;
            pItems->ullDurationCopy += dur;
            pItems->ullTotalMemDuration += dur;
            pItems->uiNumCopy++;
        }

        else if (pMAPI->m_strCMDType.find("READ") != string::npos)
        {
            pItems->uiByteRead += pMAPI->m_uiTransferSize;
            ULONGLONG dur = pMAPI->m_ullComplete - pMAPI->m_ullRunning;
            pItems->ullDurationRead += dur;
            pItems->ullTotalMemDuration += dur;
            pItems->uiNumRead++;
        }

        else if (pMAPI->m_strCMDType.find("WRITE") != string::npos)
        {
            pItems->uiByteWrite += pMAPI->m_uiTransferSize;
            ULONGLONG dur = pMAPI->m_ullComplete - pMAPI->m_ullRunning;
            pItems->ullDurationWrite += dur;
            pItems->ullTotalMemDuration += dur;
            pItems->uiNumWrite++;
        }

        else if (pMAPI->m_strCMDType.find("_MAP") != string::npos)
        {
            pItems->uiByteMap += pMAPI->m_uiTransferSize;
            ULONGLONG dur = pMAPI->m_ullComplete - pMAPI->m_ullRunning;
            pItems->ullDurationMap += dur;
            pItems->ullTotalMemDuration += dur;
            pItems->uiNumMap++;
        }

        pItems->uiNumMemOp++;

        if (it == m_ContextSumMap.end())
        {
            m_ContextSumMap[ pMAPI->m_uiContextID ] = items;
        }
    }
    else if (pAPIInfo->m_Type == CL_ENQUEUE_KERNEL)
    {
        CLKernelAPIInfo* pKAPI = (CLKernelAPIInfo*)pAPIInfo;

        if (pKAPI->m_bInfoMissing)
        {
            return;
        }

        ContextSumMap::iterator it = m_ContextSumMap.find(pKAPI->m_uiContextID);

        ContextSummaryItems items;
        ContextSummaryItems* pItems;

        if (it != m_ContextSumMap.end())
        {
            pItems = &it->second;
        }
        else
        {
            pItems = &items;
            pItems->uiContextID = pKAPI->m_uiContextID;
        }

        FlushTmpCounters(pKAPI->m_strCntxHandle, pItems);

        // search device
        KernelSumMap::iterator kit = pItems->KernelMap.find(pKAPI->m_strDevice);

        if (kit != pItems->KernelMap.end())
        {
            // updating existing device
            kit->second.ullTotalTime += (pKAPI->m_ullComplete - pKAPI->m_ullRunning);
            kit->second.uiNumCalls++;
        }
        else
        {
            // New device
            KernelSummaryItems kItem;
            kItem.strDeviceName = pKAPI->m_strDevice;
            kItem.strKernelName = pKAPI->m_strKernelName;
            kItem.ullTotalTime = (pKAPI->m_ullComplete - pKAPI->m_ullRunning);
            kItem.uiNumCalls = 1;
            pItems->KernelMap.insert(pair<string, KernelSummaryItems>(pKAPI->m_strDevice, kItem));

            // Update global device list
            bool bFound = false;

            for (vector<string>::iterator vit = m_vecDevices.begin(); vit != m_vecDevices.end(); vit++)
            {
                if (*vit == pKAPI->m_strDevice)
                {
                    bFound = true;
                    break;
                }
            }

            if (!bFound)
            {
                m_vecDevices.push_back(pKAPI->m_strDevice);
            }
        }

        if (it == m_ContextSumMap.end())
        {
            m_ContextSumMap[ pKAPI->m_uiContextID ] = items;
        }
    }
    else
    {
        if (pAPIInfo->m_strName.find("Create") != string::npos)
        {
            if (pAPIInfo->m_strName.find("Buffer") != string::npos)
            {
                size_t idx = pAPIInfo->m_ArgList.find_first_of(';');
                string strCntxHandle = pAPIInfo->m_ArgList.substr(0, idx);

                if (m_tmpCLObjCounter.find(strCntxHandle) != m_tmpCLObjCounter.end())
                {
                    CLObjectCounter& counter = m_tmpCLObjCounter[strCntxHandle];
                    counter.uiBufferCount++;
                }
                else
                {
                    CLObjectCounter counter;
                    counter.uiBufferCount = 1;
                    m_tmpCLObjCounter[strCntxHandle] = counter;
                }
            }
            else if (pAPIInfo->m_strName.find("Image") != string::npos)
            {
                size_t idx = pAPIInfo->m_ArgList.find_first_of(';');
                string strCntxHandle = pAPIInfo->m_ArgList.substr(0, idx);

                if (m_tmpCLObjCounter.find(strCntxHandle) != m_tmpCLObjCounter.end())
                {
                    CLObjectCounter& counter = m_tmpCLObjCounter[strCntxHandle];
                    counter.uiImageCount++;
                }
                else
                {
                    CLObjectCounter counter;
                    counter.uiImageCount = 1;
                    m_tmpCLObjCounter[strCntxHandle] = counter;
                }
            }
            else if (pAPIInfo->m_strName.find("CommandQueue") != string::npos)
            {
                size_t idx = pAPIInfo->m_ArgList.find_first_of(';');
                string strCntxHandle = pAPIInfo->m_ArgList.substr(0, idx);

                if (m_tmpCLObjCounter.find(strCntxHandle) != m_tmpCLObjCounter.end())
                {
                    CLObjectCounter& counter = m_tmpCLObjCounter[strCntxHandle];
                    counter.uiQueueCount++;
                }
                else
                {
                    CLObjectCounter counter;
                    counter.uiQueueCount = 1;
                    m_tmpCLObjCounter[strCntxHandle] = counter;
                }
            }
        }
    }

}

void CLContextSummarizer::GenerateHTMLTable(std::ostream& sout)
{
    HTMLTable table;
    table.AddColumn("Context ID")
    .AddColumn("# of Queues", true, true)
    .AddColumn("# of Buffers", true, true)
    .AddColumn("# of Images", true, true);


    stringstream tmpss;
    int tableIdx = 4;

    // Add all devices to header
    for (vector<string>::iterator it = m_vecDevices.begin(); it != m_vecDevices.end(); it++)
    {
        tmpss << "# of Kernel Dispatch - " << *it;
        table.AddColumn(tmpss.str().c_str(), true, true);
        tmpss.str("");
        tmpss << "Total Kernel Time(ms) - " << *it;
        table.AddColumn(tmpss.str().c_str(), true, true);
        tmpss.str("");
        tableIdx += 2;
    }

    // Add rest of headers
    table.AddColumn("# of Memory Transfer", true, true)
    .AddColumn("Total Memory Time(ms)", true, true)
    .AddColumn("# of Read", true, true)
    .AddColumn("Total Read Time(ms)", true, true)
    .AddColumn("Size of Read", true, true)
    .AddColumn("# of Write", true, true)
    .AddColumn("Total Write Time(ms)", true, true)
    .AddColumn("Size of Write", true, true)
    .AddColumn("# of Map", true, true)
    .AddColumn("Total Map Time(ms)", true, true)
    .AddColumn("Size of Map", true, true)
    .AddColumn("# of Copy", true, true)
    .AddColumn("Total Copy Time(ms)", true, true)
    .AddColumn("Size of Copy", true, true);

    ContextSummaryItems sum;

    int itemIdx = 0;

    for (ContextSumMap::iterator it = m_ContextSumMap.begin(); it != m_ContextSumMap.end(); it++)
    {
        itemIdx = 4;
        HTMLTableRow row(&table);

        ContextSummaryItems& items = it->second;

        row.AddItem(0, StringUtils::ToString(items.uiContextID))
        .AddItem(1, StringUtils::ToString(items.uiNumQueue))
        .AddItem(2, StringUtils::ToString(items.uiNumBuffer))
        .AddItem(3, StringUtils::ToString(items.uiNumImage));

        // Add per device stats
        for (vector<string>::iterator vit = m_vecDevices.begin(); vit != m_vecDevices.end(); vit++)
        {
            // Iterate through all global devices
            KernelSumMap::iterator kIt = items.KernelMap.find(*vit);

            if (kIt != items.KernelMap.end())
            {
                row.AddItem(itemIdx + 0, StringUtils::ToString(kIt->second.uiNumCalls))
                .AddItem(itemIdx + 1, StringUtils::NanosecToMillisec(kIt->second.ullTotalTime));
            }
            else
            {
                // Some devices may not be applicable for this context, print out NA
                row.AddItem(itemIdx + 0, "NA")
                .AddItem(itemIdx + 1, "NA");
            }

            itemIdx += 2;
        }

        row.AddItem(tableIdx + 0, StringUtils::ToString(items.uiNumMemOp))
        .AddItem(tableIdx + 1, StringUtils::NanosecToMillisec(items.ullTotalMemDuration))
        .AddItem(tableIdx + 2, StringUtils::ToString(items.uiNumRead))
        .AddItem(tableIdx + 3, StringUtils::NanosecToMillisec(items.ullDurationRead))
        .AddItem(tableIdx + 4, StringUtils::InsertLeadingSpace(StringUtils::GetDataSizeStr(items.uiByteRead, 2), 15))
        .AddItem(tableIdx + 5, StringUtils::ToString(items.uiNumWrite))
        .AddItem(tableIdx + 6, StringUtils::NanosecToMillisec(items.ullDurationWrite))
        .AddItem(tableIdx + 7, StringUtils::InsertLeadingSpace(StringUtils::GetDataSizeStr(items.uiByteWrite, 2), 15))
        .AddItem(tableIdx + 8, StringUtils::ToString(items.uiNumMap))
        .AddItem(tableIdx + 9, StringUtils::NanosecToMillisec(items.ullDurationMap))
        .AddItem(tableIdx + 10, StringUtils::InsertLeadingSpace(StringUtils::GetDataSizeStr(items.uiByteMap, 2), 15))
        .AddItem(tableIdx + 11, StringUtils::ToString(items.uiNumCopy))
        .AddItem(tableIdx + 12, StringUtils::NanosecToMillisec(items.ullDurationCopy))
        .AddItem(tableIdx + 13, StringUtils::InsertLeadingSpace(StringUtils::GetDataSizeStr(items.uiByteCopy, 2), 15));

        sum += items;

        table.AddRow(row);
    }

    // Add Total Row
    if (!m_ContextSumMap.empty())
    {
        HTMLTableRow totalRow(&table);
        totalRow.AddItem(0, string("Total"))
        .AddItem(1, StringUtils::ToString(sum.uiNumQueue))
        .AddItem(2, StringUtils::ToString(sum.uiNumBuffer))
        .AddItem(3, StringUtils::ToString(sum.uiNumImage));

        itemIdx = 4;

        for (vector<string>::iterator vit = m_vecDevices.begin(); vit != m_vecDevices.end(); vit++)
        {
            KernelSumMap::iterator kIt = sum.KernelMap.find(*vit);

            if (kIt != sum.KernelMap.end())
            {
                totalRow.AddItem(itemIdx + 0, StringUtils::ToString(kIt->second.uiNumCalls))
                .AddItem(itemIdx + 1, StringUtils::NanosecToMillisec(kIt->second.ullTotalTime));
            }
            else
            {
                totalRow.AddItem(itemIdx + 0, "NA")
                .AddItem(itemIdx + 1, "NA");
            }

            itemIdx += 2;
        }

        totalRow.AddItem(tableIdx + 0, StringUtils::ToString(sum.uiNumMemOp))
        .AddItem(tableIdx + 1, StringUtils::NanosecToMillisec(sum.ullTotalMemDuration))
        .AddItem(tableIdx + 2, StringUtils::ToString(sum.uiNumRead))
        .AddItem(tableIdx + 3, StringUtils::NanosecToMillisec(sum.ullDurationRead))
        .AddItem(tableIdx + 4, StringUtils::InsertLeadingSpace(StringUtils::GetDataSizeStr(sum.uiByteRead, 2), 15))
        .AddItem(tableIdx + 5, StringUtils::ToString(sum.uiNumWrite))
        .AddItem(tableIdx + 6, StringUtils::NanosecToMillisec(sum.ullDurationWrite))
        .AddItem(tableIdx + 7, StringUtils::InsertLeadingSpace(StringUtils::GetDataSizeStr(sum.uiByteWrite, 2), 15))
        .AddItem(tableIdx + 8, StringUtils::ToString(sum.uiNumMap))
        .AddItem(tableIdx + 9, StringUtils::NanosecToMillisec(sum.ullDurationMap))
        .AddItem(tableIdx + 10, StringUtils::InsertLeadingSpace(StringUtils::GetDataSizeStr(sum.uiByteMap, 2), 15))
        .AddItem(tableIdx + 11, StringUtils::ToString(sum.uiNumCopy))
        .AddItem(tableIdx + 12, StringUtils::NanosecToMillisec(sum.ullDurationCopy))
        .AddItem(tableIdx + 13, StringUtils::InsertLeadingSpace(StringUtils::GetDataSizeStr(sum.uiByteCopy, 2), 15));

        table.AddRow(totalRow, true);
    }

    table.WriteToStream(sout);
}

bool CLContextSummarizer::GenerateHTMLPage(const char* szFileName)
{
    bool retVal = false;

    if (!m_ContextSumMap.empty())
    {
        ofstream fout(szFileName);
        fout <<
             "<!-- saved from url=(0014)about:internet -->\n"      // add this line so that java script is enabled automatically
             "<html>\n"
             "<head>\n"
             "<title>Context Summary Page</title>\n"
             "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=ISO-8859-1\">\n";

        HTMLTable::WriteTableStyle(fout);
        HTMLTable::WriteSortableTableScript(fout);

        fout << "</head>\n";
        fout << "<body>\n";

        GenerateHTMLTable(fout);

        fout << "\n";

        fout <<
             "</body>"
             "</html>";

        fout.close();
        retVal = true;
    }

    return retVal;
}
