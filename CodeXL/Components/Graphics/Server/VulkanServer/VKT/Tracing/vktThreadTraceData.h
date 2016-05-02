//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktThreadTraceData.h
/// \brief  A Vulkan-specific implementation of a traced thread's data.
//=============================================================================

#ifndef __VKT_THREAD_TRACE_DATA_H__
#define __VKT_THREAD_TRACE_DATA_H__

#include "../../../Common/Tracing/ThreadTraceData.h"
#include "../Util/vktUtil.h"

class VktAPIEntry;

//-----------------------------------------------------------------------------
/// A buffer that can be used on a per-thread basis to log function calls without
/// having to deal with locking a single buffer and serializing call timings.
//-----------------------------------------------------------------------------
class VktThreadTraceData : public ThreadTraceData
{
public:
    VktThreadTraceData() : ThreadTraceData() {}
    virtual ~VktThreadTraceData() {}

    VktAPIEntry* FindInvocationBySampleId(uint64 inSampleId);
};

#endif // __VKT_THREAD_TRACE_DATA_H__