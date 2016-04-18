//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class generates a summary page for kernel dispatches
//==============================================================================

#ifndef _KERNEL_SUMMARIZER_H_
#define _KERNEL_SUMMARIZER_H_

#include <set>
#include <map>
#include <sstream>
#include <fstream>

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#include "../Common/StringUtils.h"
#include "../Common/IParserListener.h"
#include "../Common/OSUtils.h"
#include "../Common/HTMLTable.h"

//------------------------------------------------------------------------------------
/// Kernel Summary table header
//------------------------------------------------------------------------------------
class KernelSummaryItems
{
public:
    std::string strKernelName;   ///< Kernel name
    std::string strDeviceName;   ///< Device name
    ULONGLONG ullTotalTime;      ///< Total time
    unsigned int uiNumCalls;     ///< Number of Calls
    ULONGLONG ullMax;            ///< Max time
    ULONGLONG ullMin;            ///< Min time
    ULONGLONG ullAve;            ///< Ave time
    unsigned int uiMaxCallIndex; ///< Call index of instance with max time
    unsigned int uiMinCallIndex; ///< Call index of instance with min time
    osThreadId maxTid;           ///< Thread id of instance with max time
    osThreadId minTid;           ///< Thread id of instance with min time

    /// Constructor
    KernelSummaryItems()
    {
        strKernelName.clear();
        strDeviceName.clear();
        uiNumCalls = 0;
        ullTotalTime = 0;
        ullMax = 0;
        ullMin = (ULONGLONG) - 1; // init to maximum ulonglong
        ullAve = 0;
        uiMaxCallIndex = 0;
        uiMinCallIndex = 0;
        maxTid = 0;
        minTid = 0;
    }

    /// Copy constructor
    /// \param obj object
    KernelSummaryItems(const KernelSummaryItems& obj)
    {
        strKernelName = obj.strKernelName;
        strDeviceName = obj.strDeviceName;
        uiNumCalls = obj.uiNumCalls;
        ullTotalTime = obj.ullTotalTime;
        ullMax = obj.ullMax;
        ullMin = obj.ullMin;
        ullAve = obj.ullAve;
        uiMaxCallIndex = obj.uiMaxCallIndex;
        uiMinCallIndex = obj.uiMinCallIndex;
        maxTid = obj.maxTid;
        minTid = obj.minTid;
    }

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const KernelSummaryItems& operator = (const KernelSummaryItems& obj)
    {
        if (this != &obj)
        {
            strKernelName = obj.strKernelName;
            strDeviceName = obj.strDeviceName;
            uiNumCalls = obj.uiNumCalls;
            ullTotalTime = obj.ullTotalTime;
            ullMax = obj.ullMax;
            ullMin = obj.ullMin;
            ullAve = obj.ullAve;
            uiMaxCallIndex = obj.uiMaxCallIndex;
            uiMinCallIndex = obj.uiMinCallIndex;
            maxTid = obj.maxTid;
            minTid = obj.minTid;
        }

        return *this;
    }
};

typedef std::map< std::string, KernelSummaryItems > KernelSumMap;

/// Kernel dispatch api traits
template <class T>
struct kernel_dispatch_api_traits {};

//------------------------------------------------------------------------------------
/// Kernel summarizer
//------------------------------------------------------------------------------------
template <class T>
class KernelSummarizer
    : public IParserListener<T>
{
public:
    typedef typename kernel_dispatch_api_traits<T>::dispatch_api_type dispatch_type;
    typedef typename kernel_dispatch_api_traits<T>::dispatch_duration_cmp_type cmp_type;
    /// Constructor
    KernelSummarizer(void)
    {
        m_uiTopX = 10;
        m_ullTotalDuration = 0;
    }

    /// Destructor
    ~KernelSummarizer(void) {}

    /// Listener function
    /// \param pAPIInfo API Info object
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    virtual void OnParse(T* pAPIInfo, bool& stopParsing) = 0;

    /// Generate Top X HTML table from statistic data and write to std::ostream
    /// \param sout output stream
    virtual void GenerateTopXKernelHTMLTable(std::ostream& sout) = 0;

    /// Generate kernel summary HTML table from statistic data and write to std::ostream
    /// \param sout output stream
    void GenerateKernelSummaryHTMLTable(std::ostream& sout)
    {
        // create a list sorted by ullTotalTime
        std::multiset<KernelSummaryItems*, MemberCmp<KernelSummaryItems, ULONGLONG, &KernelSummaryItems::ullTotalTime> > sortedList;

        for (KernelSumMap::iterator it = m_KernelSumMap.begin(); it != m_KernelSumMap.end(); it++)
        {
            sortedList.insert(&it->second);
        }

        HTMLTable table;
        table.AddColumn("Kernel Name")
        .AddColumn("Device Name")
        .AddColumn("# of Calls", true, true)
        .AddColumn("Total Time(ms)", true, true)
        .AddColumn("% of Total Time", true, true)
        .AddColumn("Avg Time(ms)", true, true)
        .AddColumn("Max Time(ms)", true, true)
        .AddColumn("Min Time(ms)", true, true);

        for (std::multiset<KernelSummaryItems*, MemberCmp<KernelSummaryItems, ULONGLONG, &KernelSummaryItems::ullTotalTime> >::reverse_iterator it = sortedList.rbegin(); it != sortedList.rend(); it++)
        {
            HTMLTableRow row(&table);

            std::string strMaxItem = GetMinMaxItemHRef(true, (*it));
            std::string strMinItem = GetMinMaxItemHRef(false, (*it));
            double percentage = m_ullTotalDuration == 0 ? 0 : ((double)(*it)->ullTotalTime / (double)m_ullTotalDuration) * 100.0;

            row.AddItem(0, (*it)->strKernelName)
            .AddItem(1, (*it)->strDeviceName)
            .AddItem(2, StringUtils::ToString((*it)->uiNumCalls))
            .AddItem(3, StringUtils::NanosecToMillisec((*it)->ullTotalTime))
            .AddItem(4, StringUtils::ToString(percentage, 2))
            .AddItem(5, StringUtils::NanosecToMillisec((*it)->ullAve))
            .AddItem(6, strMaxItem)
            .AddItem(7, strMinItem);
            table.AddRow(row);
        }

        table.WriteToStream(sout);
    }


    /// Generate simple HTML page
    /// \param szFileName file name
    /// \return true if the page was generated, false otherwise
    bool GenerateTopXKernelHTMLPage(const char* szFileName)
    {
        bool retVal = false;

        if (!m_KernelAPISet.empty())
        {
            std::ofstream fout(szFileName);
            fout <<
                 "<!-- saved from url=(0014)about:internet -->\n"      // add this line so that java script is enabled automatically
                 "<html>\n"
                 "<head>\n"
                 "<title>Top" << m_uiTopX << " Kernels</title>\n"
                 "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=ISO-8859-1\">\n";

            HTMLTable::WriteTableStyle(fout);
            HTMLTable::WriteSortableTableScript(fout);

            fout << "</head>\n";
            fout << "<body>\n";

            GenerateTopXKernelHTMLTable(fout);

            fout << "\n";

            fout <<
                 "</body>"
                 "</html>";

            fout.close();
            retVal = true;
        }

        return retVal;
    }

    /// Generate simple HTML page
    /// \param szFileName file name
    /// \return true if the page was generated, false otherwise
    bool GenerateKernelSummaryHTMLPage(const char* szFileName)
    {
        bool retVal = false;

        if (!m_KernelSumMap.empty())
        {
            std::ofstream fout(szFileName);
            fout <<
                 "<!-- saved from url=(0014)about:internet -->\n"      // add this line so that java script is enabled automatically
                 "<html>\n"
                 "<head>\n"
                 "<title>Kernel Summary Page</title>\n"
                 "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=ISO-8859-1\">\n";

            HTMLTable::WriteTableStyle(fout);
            HTMLTable::WriteSortableTableScript(fout);

            fout << "</head>\n";
            fout << "<body>\n";

            GenerateKernelSummaryHTMLTable(fout);

            fout << "\n";

            fout <<
                 "</body>"
                 "</html>";

            fout.close();
            retVal = true;
        }

        return retVal;
    }

protected:
    /// Gets the href tag for a min/max kernel dispatch time
    /// \param isMaxItem flag indicating whether we are requesting the max (true) or min (false) href
    /// \param pItem the item whose min/max value is needed
    /// \return the href for the min/max item
    virtual std::string GetMinMaxItemHRef(bool isMaxItem, KernelSummaryItems* pItem) = 0;

    std::multiset<dispatch_type*, cmp_type> m_KernelAPISet;   ///< kernel api map, sorted by duration
    SP_TODO("NOTE! Different kernels can share the same name, identify them by doing hashing of source binary in CLTraceAgent later")
    KernelSumMap m_KernelSumMap;                                         ///< kernel summary map
    ULONGLONG m_ullTotalDuration;                                        ///< total duration
    unsigned int m_uiTopX;                                               ///< Number of top items to list
private:
    /// Copy constructor
    /// \param obj object
    KernelSummarizer(const KernelSummarizer& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const KernelSummarizer& operator = (const KernelSummarizer& obj);
};

#endif
