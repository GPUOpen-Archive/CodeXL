//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file provides the HSA Memory Summarizer
//==============================================================================

#ifndef _HSA_MEM_SUMMARIZER_H_
#define _HSA_MEM_SUMMARIZER_H_

#include <set>
#include "../HSAFdnTrace/HSAAPIInfo.h"
#include "../Common/IParserListener.h"
#include "../Common/OSUtils.h"

//------------------------------------------------------------------------------------
/// Mem Duration compare
//------------------------------------------------------------------------------------
struct HSAMemDurationCmp
{
    bool operator()(const HSAMemoryTransferAPIInfo* left, const HSAMemoryTransferAPIInfo* right)
    {
        uint64_t durationLeft = left->m_transferEndTime - left->m_transferStartTime;
        uint64_t durationRight = right->m_transferEndTime - right->m_transferStartTime;
        return durationLeft < durationRight;
    }
};

//------------------------------------------------------------------------------------
/// Memory operation summarizer
//------------------------------------------------------------------------------------
class HSAMemSummarizer
    : public IParserListener<HSAAPIInfo>
{
public:
    /// Constructor
    HSAMemSummarizer(void);

    /// Destructor
    ~HSAMemSummarizer(void);

    /// Listener function
    /// \param pAPIInfo API Info object
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    void OnParse(HSAAPIInfo* pAPIInfo, bool& stopParsing);

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
    std::multiset<HSAMemoryTransferAPIInfo*, HSAMemDurationCmp> m_memAPISet; /// memory api map, sorted by duration
    unsigned int m_uiTopX;           ///< Number of top items to list
private:
    /// Copy constructor
    /// \param obj object
    HSAMemSummarizer(const HSAMemSummarizer& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const HSAMemSummarizer& operator = (const HSAMemSummarizer& obj);
};

#endif //_HSA_MEM_SUMMARIZER_H_
