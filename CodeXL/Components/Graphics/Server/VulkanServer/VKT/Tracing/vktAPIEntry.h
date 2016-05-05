//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktAPIEntry.h
/// \brief  A Vulkan-specific entry for a traced API call.
//=============================================================================

#ifndef __VKT_API_ENTRY_H__
#define __VKT_API_ENTRY_H__

#include "../../../Common/Tracing/APIEntry.h"
#include "../Util/vktUtil.h"

class VktWrappedCmdBuf;

//-----------------------------------------------------------------------------
/// Used to track all Vulkan API calls that are traced at runtime.
/// All API calls can be traced, and only some can be profiled.
//-----------------------------------------------------------------------------
class VktAPIEntry : public APIEntry
{
public:
    VktAPIEntry(UINT inThreadId, FuncId inFunctionId, const std::string& inArguments, VktWrappedCmdBuf* pWrappedCmdBuf);
    virtual ~VktAPIEntry() {}

    virtual const char* GetAPIName() const;
    virtual void AppendAPITraceLine(gtASCIIString& out, double startTime, double endTime) const;
    virtual bool IsDrawCall() const;

    void SetReturnValue(INT64 inReturnValue);

    /// Associate a traced API call with the profiler results coming out of QueueSubmit.
    UINT64 m_sampleId;

    /// The return value of the call.
    VkResult m_returnValue;

    /// Used for GPU trace.
    VktWrappedCmdBuf* m_pWrappedCmdBuf;
};

#endif // __VKT_API_ENTRY_H__