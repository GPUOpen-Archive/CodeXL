//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12Interceptor.h
/// \brief  The DX12Interceptor contains the mechanisms responsible for
///         instrumenting DX12 objects and function calls through hooking.
//=============================================================================

#include "DX12Interceptor.h"
#include "Tracing/DX12TraceAnalyzerLayer.h"
#include "DX12LayerManager.h"
#include "Objects/DX12CreateInfoStructs.h"
#include "Objects/DX12ObjectDatabaseProcessor.h"
#include "Profiling/DX12FrameProfilerLayer.h"
#include "Tracing/DX12APIEntry.h"

#include "../Common/OSWrappers.h"
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#ifndef DLL_REPLACEMENT
    #include "Interceptor.h"
#endif

#include "../Common/SharedGlobal.h"
#include "DX12Defines.h"

//-----------------------------------------------------------------------------
/// Retrieve the LayerManager that owns this interceptor.
/// \returns The singleton DX12LayerManager instance.
//-----------------------------------------------------------------------------
ModernAPILayerManager* DX12Interceptor::GetParentLayerManager()
{
    return GetDX12LayerManager();
}

//-----------------------------------------------------------------------------
/// Handler used before the real runtime implementation of an API call has been invoked.
/// \param inWrappedInterface An instance of a wrapped DX12 interface.
/// \param inFunctionId The function ID for the call being traced.
/// \param inNumParameters The number of parameters for this API call.
/// \param pParameters pointer to an array of parameter attributes.
/// \returns Pointer to an APIEntry structure.
//-----------------------------------------------------------------------------
DX12APIEntry* DX12Interceptor::PreCall(IUnknown* inWrappedInterface, FuncId inFunctionId, int inNumParameters, ParameterEntry* pParameters)
{
    DX12TraceAnalyzerLayer* pTraceAnalyzerLayer = DX12TraceAnalyzerLayer::Instance();

    DWORD threadId = osGetCurrentThreadId();
    DX12APIEntry* pNewEntry = new DX12APIEntry(threadId, inWrappedInterface, inFunctionId, inNumParameters, pParameters);

    // Forward the info to the frame profiler for now. In the future, the DX12Interceptor won't exist, and this will be handled automatically.
    DX12FrameProfilerLayer* frameProfiler = static_cast<DX12FrameProfilerLayer*>(GetParentLayerManager()->GetFrameProfilerLayer());

    if (frameProfiler != nullptr)
    {
        frameProfiler->PreCall(inWrappedInterface, inFunctionId);
    }

    pTraceAnalyzerLayer->BeforeAPICall();
    return pNewEntry;
}

//-----------------------------------------------------------------------------
/// Responsible for the post-call instrumentation of every DX12 API call.
/// \param pNewEntry Pointer to an APIEntry structure returned from PreCall()
/// \param inReturnValue The return value for the function. If void, use FUNCTION_RETURNS_VOID.
/// \param inReturnValueFlags A flag indicating how the return value should be displayed
//-----------------------------------------------------------------------------
void DX12Interceptor::PostCall(DX12APIEntry* pNewEntry, INT64 inReturnValue, ReturnDisplayType inReturnValueFlags)
{
    DX12TraceAnalyzerLayer* pTraceAnalyzerLayer = DX12TraceAnalyzerLayer::Instance();

    pTraceAnalyzerLayer->LogAPICall(pNewEntry);

    // Forward the info to the frame profiler for now. In the future, the DX12Interceptor won't exist, and this will be handled automatically.
    DX12FrameProfilerLayer* frameProfiler = static_cast<DX12FrameProfilerLayer*>(GetParentLayerManager()->GetFrameProfilerLayer());

    if (frameProfiler != nullptr)
    {
        frameProfiler->PostCall(pNewEntry, pNewEntry->mWrapperInterface, pNewEntry->mFunctionId);
    }

    pNewEntry->SetReturnValue(inReturnValue, inReturnValueFlags);
}