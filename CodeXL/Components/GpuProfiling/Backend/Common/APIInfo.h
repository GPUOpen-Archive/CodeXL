//==============================================================================
// Copyright (c) 2010 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file APIInfo.h
/// \brief  Base class to hold API info for API tracing.
//==============================================================================

#ifndef _API_INFO_H_
#define _API_INFO_H_

#include <string>

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#include "../Common/OSUtils.h"

//------------------------------------------------------------------------------------
/// API Info
//------------------------------------------------------------------------------------
class APIInfo
{
public:
    /// Constructor
    APIInfo(void) : m_tid(0) {}

    /// Virtual destructor
    virtual ~APIInfo(void) {}
public:
    ULONGLONG m_ullStart = 0;               ///< Start time stamp
    ULONGLONG m_ullEnd = 0;                 ///< End time stamp
    unsigned int m_uiSeqID = 0;             ///< Sequence ID -- this is basically an index in the api list array
    unsigned int m_uiDisplaySeqID = 0;      ///< Displayable sequence ID -- this is the call index shown in the profiler UI
    bool m_bHasDisplayableSeqId = false;    ///< Flag indicating whether or not this API has a displayable sequence id
    osThreadId m_tid = 0;                   ///< Thread ID
    std::string m_strRet;                   ///< Return value string
    std::string m_strName;                  ///< API Name
    std::string m_ArgList;                  ///< Argument List
};

#endif
