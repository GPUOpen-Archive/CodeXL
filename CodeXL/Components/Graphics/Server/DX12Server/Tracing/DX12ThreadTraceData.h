//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12ThreadTraceData.h
/// \brief  A DX12-specific implementation of a traced thread's data.
//==============================================================================

#ifndef DX12THREADTRACEDATA_H
#define DX12THREADTRACEDATA_H

#include "../../Common/Tracing/ThreadTraceData.h"

//-----------------------------------------------------------------------------
/// A buffer that can be used on a per-thread basis to log function calls without
/// having to deal with locking a single buffer and serializing call timings.
//-----------------------------------------------------------------------------
class DX12ThreadTraceData : public ThreadTraceData
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor does nothing except initialize the baseclass.
    //-----------------------------------------------------------------------------
    DX12ThreadTraceData() : ThreadTraceData() { }

    //-----------------------------------------------------------------------------
    /// Default destructor.
    //-----------------------------------------------------------------------------
    virtual ~DX12ThreadTraceData() { }
};

#endif // DX12THREADTRACEDATA_H