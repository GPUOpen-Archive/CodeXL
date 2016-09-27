//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file provides the CL Memory Summarizer
//==============================================================================

#include <fstream>
#include <sstream>
#include "HSAMemSummarizer.h"
#include "AnalyzerHTMLUtils.h"
#include "../Common/HTMLTable.h"
#include "../Common/StringUtils.h"

using std::stringstream;
using std::ofstream;
using std::multiset;
using std::string;

HSAMemSummarizer::HSAMemSummarizer(void)
{
    m_uiTopX = 10;
}

HSAMemSummarizer::~HSAMemSummarizer(void)
{
}

void HSAMemSummarizer::OnParse(HSAAPIInfo* pAPIInfo, bool& stopParsing)
{
    stopParsing = false;

    if (pAPIInfo->m_apiID == HSA_API_Type_hsa_amd_memory_async_copy)
    {
        HSAMemoryTransferAPIInfo* pMemTransferAPI = dynamic_cast<HSAMemoryTransferAPIInfo*>(pAPIInfo);

        if (pMemTransferAPI->m_transferEndTime < pMemTransferAPI->m_transferStartTime)
        {
            Log(logERROR, "HSAMemSummarizer: mem transfer end time is less than start time\n");
            return;
        }

        m_memAPISet.insert(pMemTransferAPI);
    }
}

void HSAMemSummarizer::Debug()
{
}

/// Generate HTML table from statistic data and write to std::ostream
/// \param sout output stream
void HSAMemSummarizer::GenerateHTMLTable(std::ostream& sout)
{
    HTMLTable table;

    unsigned int count = 0;

    table.AddColumn("Source Agent Handle")
    .AddColumn("Destination Agent Handle")
    .AddColumn("Duration(ms)", true, true)
    .AddColumn("Transfer Size", true, true)
    .AddColumn("Transfer Rate(MB/s)")
    .AddColumn("Thread ID")
    .AddColumn("Call Index");

    for (multiset<HSAMemoryTransferAPIInfo*, HSAMemDurationCmp>::reverse_iterator it = m_memAPISet.rbegin(); it != m_memAPISet.rend(); it++)
    {
        if (count > m_uiTopX)
        {
            break;
        }

        HSAMemoryTransferAPIInfo* info = *it;

        ULONGLONG ullDuration = info->m_transferEndTime - info->m_transferStartTime;
        string strRate;
        string strSize = StringUtils::InsertLeadingSpace(StringUtils::GetDataSizeStr(info->m_size, 2), 15);

        if (ullDuration == 0)
        {
            // Runtime return incorrect timing for Image type object
            // Show NA for zero copy as well.
            strRate = "NA";
        }
        else
        {
            unsigned int mb = 1 << 20;
            double dSize = (double)info->m_size / mb;
            double dRate = dSize / ((double)ullDuration * 1e-9);
            strRate = StringUtils::ToStringPrecision(dRate, 3);  // + "MB/s"; //StringUtils::GetDataSizeStr( (unsigned int)ullRate, 3 ) + "/s";
        }

        HTMLTableRow row(&table);

        std::string keyValues;
        keyValues = GenerateHTMLKeyValue(gs_THREAD_ID_TAG, info->m_tid);
        keyValues = AppendHTMLKeyValue(keyValues, GenerateHTMLKeyValue(gs_SEQUENCE_ID_TAG, info->m_uiSeqID));
        keyValues = AppendHTMLKeyValue(keyValues, GenerateHTMLKeyValue(gs_VIEW_TAG, gs_VIEW_TIMELINE_DEVICE_TAG));
        std::string hRef = GenerateHref(keyValues, info->m_uiSeqID);

        row.AddItem(0, info->m_strSrcAgent)
        .AddItem(1, info->m_strDstAgent)
        .AddItem(2, StringUtils::NanosecToMillisec(ullDuration))
        .AddItem(3, strSize)
        .AddItem(4, strRate)
        .AddItem(5, StringUtils::ToString(info->m_tid))
        .AddItem(6, hRef);
        table.AddRow(row);

        count++;
    }

    table.WriteToStream(sout);
}

bool HSAMemSummarizer::GenerateHTMLPage(const char* szFileName)
{
    bool retVal = false;

    if (!m_memAPISet.empty())
    {
        ofstream fout(szFileName);
        fout <<
             "<!-- saved from url=(0014)about:internet -->\n"      // add this line so that java script is enabled automatically
             "<html>\n"
             "<head>\n"
             "<title>Top " << m_uiTopX << " Memory Operation Summary Page</title>\n"
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
