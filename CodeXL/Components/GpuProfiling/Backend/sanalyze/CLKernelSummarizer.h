//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file provides the CL Kernel Summarizer
//==============================================================================

#ifndef _CL_KERNEL_SUMMARIZER_H_
#define _CL_KERNEL_SUMMARIZER_H_

#include <set>
#include <map>
#include "../CLTraceAgent/CLAPIInfo.h"
#include "../Common/IParserListener.h"
#include "KernelSummarizer.h"
#include "../Common/OSUtils.h"

//------------------------------------------------------------------------------------
/// Kernel Duration compare
//------------------------------------------------------------------------------------
struct CLKernelDurationCmp
{
    bool operator()(const CLKernelAPIInfo* left, const CLKernelAPIInfo* right)
    {
        ULONGLONG ullDurationLeft = left->m_ullComplete - left->m_ullRunning;
        ULONGLONG ullDurationRight = right->m_ullComplete - right->m_ullRunning;
        return ullDurationLeft < ullDurationRight;
    }
};

/// OpenCL kernel dispatch API traits
template <>
struct kernel_dispatch_api_traits<CLAPIInfo>
{
    typedef CLKernelAPIInfo dispatch_api_type;
    typedef CLKernelDurationCmp dispatch_duration_cmp_type;
};

//------------------------------------------------------------------------------------
/// Kernel summarizer
//------------------------------------------------------------------------------------
class CLKernelSummarizer
    : public KernelSummarizer<CLAPIInfo>
{
public:
    /// Constructor
    CLKernelSummarizer(void);

    /// Destructor
    ~CLKernelSummarizer(void);

    /// Listener function
    /// \param pAPIInfo API Info object
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    void OnParse(CLAPIInfo* pAPIInfo, bool& stopParsing);

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
    CLKernelSummarizer(const CLKernelSummarizer& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const CLKernelSummarizer& operator = (const CLKernelSummarizer& obj);
};

#endif
