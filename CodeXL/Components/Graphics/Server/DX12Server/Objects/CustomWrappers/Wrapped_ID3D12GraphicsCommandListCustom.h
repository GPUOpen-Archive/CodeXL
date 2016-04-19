//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   Wrapped_ID3D12GraphicsCommandListCustom.h
/// \brief  A special wrapper implementation extended from Wrapped_ID3D12GraphicsCommandList
///         to be used for collecting profiled GPU command results.
//=============================================================================

#ifndef Wrapped_ID3D12_GRAPHICS_COMMAND_LIST_CUSTOM_H
#define Wrapped_ID3D12_GRAPHICS_COMMAND_LIST_CUSTOM_H

#include "../BaseWrappers/Wrapped_ID3D12GraphicsCommandList.h"
#include "../../Util/DX12Utilities.h"
#include "../../Profiling/DX12FrameProfilerLayer.h"
#include "../../Profiling/DX12CmdListProfiler.h"
#include "../../Profiling/DX12WorkerInfo.h"

//-----------------------------------------------------------------------------
/// Extension of the Wrapped_ID3D12GraphicsCommandList wrapper used to profile GPU commands.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12GraphicsCommandListCustom : public Wrapped_ID3D12GraphicsCommandList
{
public:
    //-----------------------------------------------------------------------------
    /// A Custom constructor for our wrapped ID3D12GraphicsCommandList.
    /// \param inRealCommandList The real runtime instance being wrapped.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12GraphicsCommandListCustom(ID3D12GraphicsCommandList* inRealCommandList);

    //-----------------------------------------------------------------------------
    /// Destructor used to release any profilers.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12GraphicsCommandListCustom();

    //-----------------------------------------------------------------------------
    /// A custom implementation of ID3D12Device::Release. Neccessary to close an Profiler instance.
    /// \returns D3D12 device reference count.
    //-----------------------------------------------------------------------------
    virtual ULONG STDMETHODCALLTYPE Release();

    //-----------------------------------------------------------------------------
    /// A custom implementation for reset which will notify the profiler of command list closure.
    /// \returns D3D12 result code.
    //-----------------------------------------------------------------------------
    virtual HRESULT STDMETHODCALLTYPE Close();

    //-----------------------------------------------------------------------------
    /// A custom implementation for reset which will also reset our profiler.
    /// \param pAllocator The CommandAllocator used with this CommandList.
    /// \param pInitialState The initial pipeline state to reset the CommandList to.
    /// \returns D3D12 result code.
    //-----------------------------------------------------------------------------
    virtual HRESULT STDMETHODCALLTYPE Reset(ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pInitialState);

    //-----------------------------------------------------------------------------
    /// Add the incoming FuncId to an ordered list of calls invoked through this wrapped CommandList instance.
    /// \param inFuncId The FuncId for the API call.
    //-----------------------------------------------------------------------------
    virtual void TrackCommandListCall(FuncId inFuncId);

    //-----------------------------------------------------------------------------
    /// Inject a "Before" timestamp measurement instruction for a profiled call.
    /// \param pIdInfo The measurement information for the profiled call.
    /// \returns A ProfilerResultCode indicating the operation's success.
    //-----------------------------------------------------------------------------
    ProfilerResultCode BeginCmdMeasurement(const ProfilerMeasurementId* pIdInfo);

    //-----------------------------------------------------------------------------
    /// Inject an "After" timestamp measurement instruction for a profiled call.
    /// \returns A ProfilerResultCode indicating the operation's success.
    //-----------------------------------------------------------------------------
    ProfilerResultCode EndCmdMeasurement();

    //-----------------------------------------------------------------------------
    /// Use a multi-threaded approach to get the results for the profiled workload on the given Queue.
    /// \param pCmdQueue The CommandQueue to retrieve profiling results for.
    /// \param targetExecId The Id of the ExecuteCommandLists to retrieve profiling results for.
    /// \param outResults A vector of measurement results retrieved from the profiler.
    /// \returns A ProfilerResultCode indicating the operation's success.
    //-----------------------------------------------------------------------------
    ProfilerResultCode GetCmdListResultsMT(ID3D12CommandQueue* pCmdQueue, INT64 targetExecId, std::vector<ProfilerResult>& outResults);

    //-----------------------------------------------------------------------------
    /// Use a single-threaded approach to get the results for the profiled workload on the given Queue.
    /// \param pCmdQueue The CommandQueue to retrieve profiling results for.
    /// \param outResults A vector of measurement results retrieved from the profiler.
    /// \returns A ProfilerResultCode indicating the operation's success.
    //-----------------------------------------------------------------------------
    ProfilerResultCode GetCmdListResultsST(ID3D12CommandQueue* pCmdQueue, std::vector<ProfilerResult>& outResults);

    //-----------------------------------------------------------------------------
    /// Check if profiling is currently enabled.
    /// \returns True if profiling is currently enabled.
    //-----------------------------------------------------------------------------
    bool IsProfilingEnabled() { return m_pProfiler != nullptr; }

    //-----------------------------------------------------------------------------
    /// Get the number of profiled calls for this CommandList.
    /// \returns The number of profiled calls for this CommandList.
    //-----------------------------------------------------------------------------
    UINT GetProfiledCallCount() const { return m_profiledCallCount; }

    //-----------------------------------------------------------------------------
    /// Get the type of this CommandList.
    /// \returns The type of this CommandList.
    //-----------------------------------------------------------------------------
    D3D12_COMMAND_LIST_TYPE GetCmdListType() { return m_cmdListType; }

    //-----------------------------------------------------------------------------
    /// Destroy all of the profiler objects used with this CommandList.
    //-----------------------------------------------------------------------------
    void DestroyProfilers();

    //-----------------------------------------------------------------------------
    /// Let the profiler know which group of command lists this profiler belongs to.
    /// \param inExecutionId The unique Id for the ExecuteCommandLists call executing profiler commands.
    //-----------------------------------------------------------------------------
    void SetProfilerExecutionId(INT64 executionId);

#if TRACK_CMD_LIST_COMMANDS
    //-----------------------------------------------------------------------------
    /// Debug function used to print out command list contents
    //-----------------------------------------------------------------------------
    void PrintCommands();
#endif

private:
    //-----------------------------------------------------------------------------
    /// Initialize a profiler per-command list.
    /// \returns A new profiler instance to be used for this CommandList.
    //-----------------------------------------------------------------------------
    DX12CmdListProfiler* InitNewProfiler();

    /// The active profiler object for this CommandList.
    DX12CmdListProfiler* m_pProfiler;

    /// A vector of closed profiler objects for this CommandList.
    std::vector<DX12CmdListProfiler*> m_closedProfilers;

    /// Mutex to lock the usage of the internal profiler instances.
    mutex m_closedProfilersMutex;

    /// The number of GPU commands profiled within this CommandList.
    UINT m_profiledCallCount;

    /// The potential number of profiled calls we might expect to see in this CommandList.
    UINT m_potentialProfiledCallCount;

    /// The highest number of profiled calls we've seen for this CommandList.
    UINT m_potentialProfiledCallCountHighest;

    /// The type of this CommandList.
    D3D12_COMMAND_LIST_TYPE m_cmdListType;

#if TRACK_CMD_LIST_COMMANDS
    /// An ordered vector of each GPU command that was inserted into this CommandList.
    std::vector<FuncId> m_commands;
#endif
};

#endif // Wrapped_ID3D12_GRAPHICS_COMMAND_LIST_CUSTOM_H
