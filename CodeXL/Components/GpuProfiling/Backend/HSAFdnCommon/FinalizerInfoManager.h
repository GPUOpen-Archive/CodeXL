//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file manages tracked info from the finalizer APIs
//==============================================================================

#ifndef _FINALIZER_INFO_MANAGER_H_
#define _FINALIZER_INFO_MANAGER_H_

#include <stdint.h>
#include <map>

#include <TSingleton.h>

/// struct used to track information from finalizer APIs to
/// provide access to kernel symbol names to the profiler
struct FinalizerInfoManager : public TSingleton<FinalizerInfoManager>
{
public:

    /// Map from code handle to symbol handle
    std::map<uint64_t, uint64_t>    m_codeHandleToSymbolHandleMap;

    /// Map from symbol handle to symbol name
    std::map<uint64_t, std::string> m_symbolHandleToNameMap;
};

#endif // _FINALIZER_INFO_MANAGER_H_
