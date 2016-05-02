//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktThreadTraceData.cpp
/// \brief  A Vulkan-specific implementation of a traced thread's data.
//=============================================================================

#include "VktThreadTraceData.h"
#include "VktAPIEntry.h"

//-----------------------------------------------------------------------------
/// Given a SampleId, find the callData that was logged during thread tracing.
/// \param inSampleId The SampleId associated with the API invocation to be found.
/// \returns The buffered CallData with a matching SampleId, or nullptr if it doesn't exist.
//-----------------------------------------------------------------------------
VktAPIEntry* VktThreadTraceData::FindInvocationBySampleId(uint64 inSampleId)
{
    VktAPIEntry* resultInvocation = nullptr;

    for (unsigned int i = 0; i < mLoggedCallVector.size(); i++)
    {
        VktAPIEntry* entry = static_cast<VktAPIEntry*>(mLoggedCallVector[i]);

        if (entry->m_sampleId == inSampleId)
        {
            resultInvocation = entry;
            break;
        }
    }

    return resultInvocation;
}