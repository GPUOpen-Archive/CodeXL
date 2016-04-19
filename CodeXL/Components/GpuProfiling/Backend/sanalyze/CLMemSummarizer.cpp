//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file provides the CL Memory Summarizer
//==============================================================================

#include <fstream>
#include <sstream>
#include "CLMemSummarizer.h"
#include "AnalyzerHTMLUtils.h"
#include "../Common/HTMLTable.h"
#include "../Common/StringUtils.h"

using std::stringstream;
using std::ofstream;
using std::multiset;
using std::string;

CLMemSummarizer::CLMemSummarizer(void)
{
    m_uiTopX = 10;
}

CLMemSummarizer::~CLMemSummarizer(void)
{
}

void CLMemSummarizer::OnParse(CLAPIInfo* pAPIInfo, bool& stopParsing)
{
    stopParsing = false;

    if (pAPIInfo->m_Type == CL_ENQUEUE_MEM)
    {
        CLMemAPIInfo* pMAPI = (CLMemAPIInfo*)pAPIInfo;

        if (!pMAPI->m_bInfoMissing)
        {
            //CL_FUNC_TYPE_clEnqueueMapBuffer = 63
            //CL_FUNC_TYPE_clEnqueueMapImage = 64
            //CL_FUNC_TYPE_clEnqueueUnmapMemObject = 65
            if (pMAPI->m_ullComplete < pMAPI->m_ullRunning && (pMAPI->m_uiAPIID < CL_FUNC_TYPE_clEnqueueMapBuffer || pMAPI->m_uiAPIID > CL_FUNC_TYPE_clEnqueueUnmapMemObject))
            {
                // Complete and Running can be the same for Zero copy.
                return;
            }

            m_MemAPISet.insert(pMAPI);
        }
    }
}

void CLMemSummarizer::Debug()
{}

/// Generate HTML table from statistic data and write to std::ostream
/// \param sout output stream
void CLMemSummarizer::GenerateHTMLTable(std::ostream& sout)
{
    HTMLTable table;

    unsigned int count = 0;

    table.AddColumn("Command Type")
    .AddColumn("Context ID")
    .AddColumn("Command Queue ID")
    .AddColumn("Duration(ms)", true, true)
    .AddColumn("Transfer Size", true, true)
    .AddColumn("Transfer Rate(MB/s)")
    .AddColumn("Thread ID")
    .AddColumn("Call Index");

    for (multiset<CLMemAPIInfo*, MemDurationCmp>::reverse_iterator it = m_MemAPISet.rbegin(); it != m_MemAPISet.rend(); it++)
    {
        if (count > m_uiTopX)
        {
            break;
        }

        CLMemAPIInfo* info = *it;

        ULONGLONG ullDuration = info->m_ullComplete - info->m_ullRunning;
        string strRate;
        string strSize = StringUtils::InsertLeadingSpace(StringUtils::GetDataSizeStr(info->m_uiTransferSize, 2), 15);

        if (ullDuration == 0 || ((info->m_strCMDType.find("IMAGE") != string::npos || info->m_strCMDType.find("MAP") != string::npos) && ullDuration < 1000))
        {
            // Runtime return incorrect timing for Image type object
            // Show NA for zero copy as well.
            strRate = "NA";
        }
        else
        {
            unsigned int mb = 1 << 20;
            double dSize = (double)info->m_uiTransferSize / mb;
            double dRate = dSize / ((double)ullDuration * 1e-9);
            strRate = StringUtils::ToString(dRate, 3);  // + "MB/s"; //StringUtils::GetDataSizeStr( (unsigned int)ullRate, 3 ) + "/s";
        }

        HTMLTableRow row(&table);

        std::string keyValues;
        keyValues = GenerateHTMLKeyValue(gs_THREAD_ID_TAG, info->m_tid);
        keyValues = AppendHTMLKeyValue(keyValues, GenerateHTMLKeyValue(gs_SEQUENCE_ID_TAG, info->m_uiSeqID));
        keyValues = AppendHTMLKeyValue(keyValues, GenerateHTMLKeyValue(gs_VIEW_TAG, gs_VIEW_TIMELINE_DEVICE_TAG));
        std::string hRef = GenerateHref(keyValues, info->m_strCMDType.substr(11));

        row.AddItem(0, hRef)
        .AddItem(1, StringUtils::ToString(info->m_uiContextID))
        .AddItem(2, StringUtils::ToString(info->m_uiQueueID))
        .AddItem(3, StringUtils::NanosecToMillisec(ullDuration))
        .AddItem(4, strSize)
        .AddItem(5, strRate)
        .AddItem(6, StringUtils::ToString(info->m_tid))
        .AddItem(7, info->m_bHasDisplayableSeqId ? StringUtils::ToString(info->m_uiDisplaySeqID) : "N/A");
        table.AddRow(row);

        count++;
    }

    table.WriteToStream(sout);
}

bool CLMemSummarizer::GenerateHTMLPage(const char* szFileName)
{
    bool retVal = false;

    if (!m_MemAPISet.empty())
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
