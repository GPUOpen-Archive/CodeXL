//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class parses kernel profile result file (perf counter, occupancy
///        and sub-kernel profiler) and provides easy access to the results.
//==============================================================================

#include "KernelProfileResultParser.h"
#include "../Common/Defs.h"
#include "../Common/Logger.h"

using namespace std;

bool KernelProfileResult::GetAvailableResultNames(size_t nRowIdx, std::vector<std::string>& availableResNames) const
{
    SpAssertRet(nRowIdx < m_results.size()) false;

    const CSVRow& row = *m_results[nRowIdx];
    const vector<string>& cols = row.GetColumns();

    for (vector<string>::const_iterator it = cols.begin(); it != cols.end(); ++it)
    {
        if (!row.GetRowData(*it).empty())
        {
            availableResNames.push_back(*it);
        }
    }

    return true;
}

bool KernelProfileResult::GetValue(size_t nRowIdx, const std::string& resultName, std::string& outValue) const
{
    SpAssertRet(nRowIdx < m_results.size()) false;
    const CSVRow& row = *m_results[nRowIdx];
    outValue = row.GetRowData(resultName);
    return !outValue.empty();
}

KernelProfileResultParser::KernelProfileResultParser(void)
    : m_bDoneParsing(false)
{
}

KernelProfileResultParser::~KernelProfileResultParser(void)
{
    for (std::list<KernelProfileResult*>::iterator it = m_ProfileResults.begin(); it != m_ProfileResults.end(); ++it)
    {
        SAFE_DELETE(*it);
    }

    m_ProfileResults.clear();
}

bool KernelProfileResultParser::Parse(const std::string& strFilename)
{
    if (!m_parser.LoadFile(strFilename.c_str()))
    {
        return false;
    }

    m_parser.AddListener(this);

    m_bDoneParsing = m_parser.Parse();
    m_parser.Close();

    return m_bDoneParsing;
}

void KernelProfileResultParser::OnParse(CSVRow* pCSVRow, bool& stopParsing)
{
    SpAssertRet(pCSVRow != NULL);
    stopParsing = false;

    CSVRow& row = *pCSVRow;
    KernelProfileResult* pResult;

    SP_TODO("rename Method to Kernel Name in perf counter mode.");
    string kernelName = row.GetRowData("Method");

    if (kernelName.empty())
    {
        kernelName = row.GetRowData("Kernel Name");
    }

    if (kernelName.empty())
    {
        // Sub kernel result
        SpAssertRet(!m_ProfileResults.empty());
        pResult = m_ProfileResults.front();
    }
    else
    {
        // new kernel result
        pResult = new KernelProfileResult(kernelName);
        m_ProfileResults.push_back(pResult);
    }

    SpAssertRet(pResult != NULL);
    pResult->AddResultRow(pCSVRow);
}
