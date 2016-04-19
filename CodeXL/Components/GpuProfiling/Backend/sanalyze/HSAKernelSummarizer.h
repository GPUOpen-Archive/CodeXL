//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This class generates the HSA kernel dispatch summary page
//==============================================================================

#ifndef _HSA_KERNEL_SUMMARIZER_H_
#define _HSA_KERNEL_SUMMARIZER_H_

#include <set>
#include <map>
#include "../HSAFdnTrace/HSAAPIInfo.h"
#include "KernelSummarizer.h"
#include "../Common/OSUtils.h"

//------------------------------------------------------------------------------------
/// Kernel Duration compare
//------------------------------------------------------------------------------------
struct HSAKernelDurationCmp
{
    bool operator()(const HSADispatchInfo* left, const HSADispatchInfo* right)
    {
        ULONGLONG ullDurationLeft = left->m_ullEnd - left->m_ullStart;
        ULONGLONG ullDurationRight = right->m_ullEnd - right->m_ullStart;
        return ullDurationLeft < ullDurationRight;
    }
};

/// HSA kernel dispatch API traits
template <>
struct kernel_dispatch_api_traits<HSAAPIInfo>
{
    typedef HSADispatchInfo dispatch_api_type;
    typedef HSAKernelDurationCmp dispatch_duration_cmp_type;
};

//------------------------------------------------------------------------------------
/// Kernel summarizer
//------------------------------------------------------------------------------------
class HSAKernelSummarizer
    : public KernelSummarizer<HSAAPIInfo>
{
public:
    /// Constructor
    HSAKernelSummarizer(void);

    /// Destructor
    ~HSAKernelSummarizer(void);

    /// Listener function
    /// \param pAPIInfo API Info object
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    void OnParse(HSAAPIInfo* pAPIInfo, bool& stopParsing);

    /// Generate Top X HTML table from statistic data and write to std::ostream
    /// \param sout output stream
    void GenerateTopXKernelHTMLTable(std::ostream& sout);

protected:
    /// Gets the href tag for a min/max kernel dispatch time
    /// \param isMaxItem flag indicating whether we are requesting the max (true) or min (false) href
    /// \param pItem the item whose min/max value is needed
    /// \return the href for the min/max item
    std::string GetMinMaxItemHRef(bool isMaxItem, KernelSummaryItems* pItem);

private:
    /// Copy constructor
    /// \param obj object
    HSAKernelSummarizer(const HSAKernelSummarizer& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const HSAKernelSummarizer& operator = (const HSAKernelSummarizer& obj);
};

#endif
