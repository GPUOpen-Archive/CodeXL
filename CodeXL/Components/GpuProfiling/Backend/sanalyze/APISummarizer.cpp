//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This class generates an API summary page.
//==============================================================================

#include <iostream>
#include <set>
#include <fstream>
#include "APISummarizer.h"
#include "AnalyzerHTMLUtils.h"
#include "../Common/Logger.h"
#include "../Common/HTMLTable.h"
#include "../Common/StringUtils.h"

using std::ofstream;
using std::cout;
using std::endl;
using std::multiset;
using namespace GPULogger;

template<class T>
APISummarizer<T>::APISummarizer(void)
{
    m_ullTotalTime = 0;
}

template<class T>
APISummarizer<T>::~APISummarizer(void)
{
}

template<class T>
void APISummarizer<T>::OnParse(T* pAPIInfo, bool& stopParsing)
{
    stopParsing = false;
    // do static check, not sure if GCC supports it
    //static_assert(std::is_base_of<APIInfo, T>::value, "Template type must be children of APIInfo");
    ULONGLONG duration;

    if (pAPIInfo->m_ullEnd <= pAPIInfo->m_ullStart)
    {
        Log(logWARNING, "Incorrect input timestamp.\n");
        duration = 0;
    }
    else
    {
        duration = pAPIInfo->m_ullEnd - pAPIInfo->m_ullStart;
    }

    m_ullTotalTime += duration;

    if (pAPIInfo->m_strName.length() > 0)
    {
        APICountMap::iterator it = m_APICountMap.find(pAPIInfo->m_strName);

        if (it == m_APICountMap.end())
        {
            APISummaryItems si;
            si.strName = pAPIInfo->m_strName;
            si.uiNumCalls = 1;
            si.ullTotalTime = duration;
            si.ullAve = si.ullMax = duration;
            si.uiMaxCallIndex = pAPIInfo->m_uiSeqID;
            si.uiMinCallIndex = pAPIInfo->m_uiSeqID;
            si.maxTid = pAPIInfo->m_tid;
            si.minTid = pAPIInfo->m_tid;

            if (duration != 0)
            {
                si.ullMin = duration;
            }

            // new api type
            m_APICountMap[pAPIInfo->m_strName] = si;
        }
        else
        {
            APISummaryItems& si = it->second;
            si.ullTotalTime += duration;
            si.uiNumCalls++;

            if (duration > si.ullMax)
            {
                si.ullMax = duration;
                si.uiMaxCallIndex = pAPIInfo->m_uiSeqID;
                si.maxTid = pAPIInfo->m_tid;
            }
            else if (duration < si.ullMin && duration != 0)
            {
                si.ullMin = duration;
                si.uiMinCallIndex = pAPIInfo->m_uiSeqID;
                si.minTid = pAPIInfo->m_tid;
            }

            si.ullAve = si.ullTotalTime / si.uiNumCalls;
        }
    }
}

template<class T>
void APISummarizer<T>::Debug()
{
    //unsigned int sum = 0;
    //for( APICountMap::iterator it = m_APICountMap.begin(); it != m_APICountMap.end(); it++ )
    //{
    //   cout << "# of " << it->first << " = " << it->second.uiNumCalls << endl;
    //   cout << "Total time " << it->first << " = " << it->second.ullTotalTime << endl;
    //   cout << "Max time " << it->first << " = " << it->second.ullMax << endl;
    //   cout << "Min time " << it->first << " = " << it->second.ullMin << endl;
    //   cout << "Ave time " << it->first << " = " << it->second.ullAve << endl;
    //   sum += it->second.uiNumCalls;
    //}
    //cout << "Total = " << sum << endl;

    multiset<APISummaryItems*, MemberCmp<APISummaryItems, ULONGLONG, &APISummaryItems::ullTotalTime> > sortedList;

    for (APICountMap::iterator it = m_APICountMap.begin(); it != m_APICountMap.end(); it++)
    {
        sortedList.insert(&it->second);
    }

    for (multiset<APISummaryItems*, MemberCmp<APISummaryItems, ULONGLONG, &APISummaryItems::ullTotalTime> >::iterator it = sortedList.begin(); it != sortedList.end(); it++)
    {
        cout << (*it)->strName << " | " << (*it)->uiNumCalls << " | " << (*it)->ullMin << " | " << (*it)->ullMax << " | " << (*it)->ullAve << endl;
    }
}

template<class T>
void APISummarizer<T>::GenerateHTMLTable(std::ostream& sout)
{
    multiset<APISummaryItems*, MemberCmp<APISummaryItems, ULONGLONG, &APISummaryItems::ullTotalTime> > sortedList;

    for (APICountMap::iterator it = m_APICountMap.begin(); it != m_APICountMap.end(); it++)
    {
        sortedList.insert(&it->second);
    }

    HTMLTable table;
    table.AddColumn("API Name")
    .AddColumn("Cumulative Time(ms)", true, true)
    .AddColumn("% of Total Time", true, true)
    .AddColumn("# of Calls", true, true)
    .AddColumn("Avg Time(ms)", true, true)
    .AddColumn("Max Time(ms)", true, true)
    .AddColumn("Min Time(ms)", true, true);

    for (multiset<APISummaryItems*, MemberCmp<APISummaryItems, ULONGLONG, &APISummaryItems::ullTotalTime> >::reverse_iterator it = sortedList.rbegin(); it != sortedList.rend(); it++)
    {
        HTMLTableRow row(&table);

        row.AddItem(0, (*it)->strName);
        row.AddItem(1, StringUtils::NanosecToMillisec((*it)->ullTotalTime));
        double percentage = ((double)(*it)->ullTotalTime / (double)m_ullTotalTime) * 100.0;
        row.AddItem(2, StringUtils::ToStringPrecision(percentage, 5));
        row.AddItem(3, StringUtils::ToString((*it)->uiNumCalls));
        row.AddItem(4, StringUtils::NanosecToMillisec((*it)->ullAve));

        std::string keyValues;
        keyValues = GenerateHTMLKeyValue(gs_THREAD_ID_TAG, (*it)->maxTid);
        keyValues = AppendHTMLKeyValue(keyValues, GenerateHTMLKeyValue(gs_SEQUENCE_ID_TAG, (*it)->uiMaxCallIndex));
        keyValues = AppendHTMLKeyValue(keyValues, GenerateHTMLKeyValue(gs_VIEW_TAG, gs_VIEW_TIMELINE_HOST_TAG));
        std::string hRef = GenerateHref(keyValues, StringUtils::NanosecToMillisec((*it)->ullMax));

        row.AddItem(5, hRef);

        // if all instances of this API reported zero (or negative) time, then show zero for the minimum -- see bug 6307
        ULONGLONG min = (*it)->ullMin;

        if (min == (ULONGLONG) - 1)
        {
            min = 0;
        }

        keyValues = GenerateHTMLKeyValue(gs_THREAD_ID_TAG, (*it)->minTid);
        keyValues = AppendHTMLKeyValue(keyValues, GenerateHTMLKeyValue(gs_SEQUENCE_ID_TAG, (*it)->uiMinCallIndex));
        keyValues = AppendHTMLKeyValue(keyValues, GenerateHTMLKeyValue(gs_VIEW_TAG, gs_VIEW_TIMELINE_HOST_TAG));
        hRef = GenerateHref(keyValues, StringUtils::NanosecToMillisec(min));

        row.AddItem(6, hRef);
        table.AddRow(row);
    }

    table.WriteToStream(sout);
}

template<class T>
bool APISummarizer<T>::GenerateHTMLPage(const char* szFileName)
{
    bool retVal = false;

    if (!m_APICountMap.empty())
    {
        ofstream fout(szFileName);
        fout <<
             "<!-- saved from url=(0014)about:internet -->\n"      // add this line so that java script is enabled automatically
             "<html>\n"
             "<head>\n"
             "<title>API Summary Page</title>\n"
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

template class APISummarizer<CLAPIInfo>;
template class APISummarizer<HSAAPIInfo>;
