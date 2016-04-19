//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file provides the CL Memory Summarizer
//==============================================================================

#ifndef _CL_MEM_SUMMARIZER_H_
#define _CL_MEM_SUMMARIZER_H_

#include <set>
#include "../CLTraceAgent/CLAPIInfo.h"
#include "../Common/IParserListener.h"
#include "../Common/OSUtils.h"

//------------------------------------------------------------------------------------
/// Mem Duration compare
//------------------------------------------------------------------------------------
struct MemDurationCmp
{
    bool operator()(const CLMemAPIInfo* left, const CLMemAPIInfo* right)
    {
        ULONGLONG ullDurationLeft = left->m_ullComplete - left->m_ullRunning;
        ULONGLONG ullDurationRight = right->m_ullComplete - right->m_ullRunning;
        return ullDurationLeft < ullDurationRight;
    }
};

//------------------------------------------------------------------------------------
/// Memory operation summarizer
//------------------------------------------------------------------------------------
class CLMemSummarizer
    : public IParserListener<CLAPIInfo>
{
public:
    /// Constructor
    CLMemSummarizer(void);

    /// Destructor
    ~CLMemSummarizer(void);

    /// Listener function
    /// \param pAPIInfo API Info object
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    void OnParse(CLAPIInfo* pAPIInfo, bool& stopParsing);

    /// Generate HTML table from statistic data and write to std::ostream
    /// \param sout output stream
    void GenerateHTMLTable(std::ostream& sout);

    /// Generate simple HTML page
    /// \param szFileName file name
    /// \return true if the page was generated, false otherwise
    bool GenerateHTMLPage(const char* szFileName);

    /// Debug
    void Debug();

protected:
    std::multiset<CLMemAPIInfo*, MemDurationCmp> m_MemAPISet; /// memory api map, sorted by duration
    unsigned int m_uiTopX;           ///< Number of top items to list
private:
    /// Copy constructor
    /// \param obj object
    CLMemSummarizer(const CLMemSummarizer& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const CLMemSummarizer& operator = (const CLMemSummarizer& obj);
};

#endif //_CL_MEM_SUMMARIZER_H_
