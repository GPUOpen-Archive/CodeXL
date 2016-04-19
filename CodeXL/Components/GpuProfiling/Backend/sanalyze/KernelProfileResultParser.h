//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class parses kernel profile result file (perf counter, occupancy
///        and sub-kernel profiler) and provides easy access to the results.
//==============================================================================

#ifndef _KERNEL_PROFILE_RESULT_PARSER_H_
#define _KERNEL_PROFILE_RESULT_PARSER_H_

#include <string>
#include <list>
#include <vector>
#include "../Common/IParserListener.h"
#include "../Common/CSVFileParser.h"

class KernelProfileResultParser;

//------------------------------------------------------------------------------------
// Kernel profile result
// One kernel could have multiple rows of profile results - usually sub-kernel profile
// results.
// Standard usage of this class:
// 1. Get number of rows
// 2. For each row, get available result names
// 3. Feed available results into analysis module
//------------------------------------------------------------------------------------
class KernelProfileResult
{
    friend class KernelProfileResultParser;
public:
    /// Get number of result rows
    /// \return number of result rows
    size_t GetNumResultRows() const
    {
        return m_results.size();
    }

    /// Get available result names for specific row
    /// \param nRowIdx Row index
    /// \param availableResNames List of output result names
    /// \return true if succeeded
    bool GetAvailableResultNames(size_t nRowIdx, std::vector<std::string>& availableResNames) const;

    /// Get result value for specific row and result name
    /// \param nRowIdx Row index
    /// \param resultName Result name
    /// \param outValue Output result
    /// \return true if succeeded
    bool GetValue(size_t nRowIdx, const std::string& resultName, std::string& outValue) const;
protected:
    /// Constructor
    /// \param strKernelName Kernel name
    KernelProfileResult(const std::string& strKernelName)
        :  m_strKernelName(strKernelName)
    {}

    /// Destructor
    virtual ~KernelProfileResult() {}

    /// Add result row
    /// \param pRow CSV row
    void AddResultRow(const CSVRow* pRow)
    {
        m_results.push_back(pRow);
    }

    std::string                m_strKernelName;  ///< Kernel name
    std::vector<const CSVRow*> m_results;        ///< List of CSV row objects
};

//------------------------------------------------------------------------------------
// Kernel profile result parser
//------------------------------------------------------------------------------------
class KernelProfileResultParser
    : public IParserListener<CSVRow>
{
public:
    /// Constructor
    KernelProfileResultParser();

    /// Destructor
    virtual ~KernelProfileResultParser(void);

    /// Kick off CSV parser
    /// \param strFilename input file name
    /// \return true if no parse error
    bool Parse(const std::string& strFilename);

    /// Listener function
    /// \param pAPIInfo API Info object
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    void OnParse(CSVRow* pCSVRow, bool& stopParsing);

    /// Get profile results
    /// \return const ref to profile results
    const std::list<KernelProfileResult*>& GetProfileResults() const
    {
        return m_ProfileResults;
    }

private:
    CSVFileParser                    m_parser;            ///< CSV file parser
    std::list<KernelProfileResult*>  m_ProfileResults;    ///< List of profile results
    bool                             m_bDoneParsing;      ///< A flag indicating whether profile results are available or not
};

#endif //_KERNEL_PROFILE_RESULT_PARSER_H_
