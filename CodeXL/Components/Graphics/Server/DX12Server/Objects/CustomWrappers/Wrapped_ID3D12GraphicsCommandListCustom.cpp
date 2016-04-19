//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   Wrapped_ID3D12GraphicsCommandListCustom.cpp
/// \brief  A special wrapper implementation extended from Wrapped_ID3D12GraphicsCommandList
///         to be used for collecting profiled GPU command results.
//=============================================================================

#include "../../DX12LayerManager.h"
#include "../../DX12Defines.h"
#include "../../../Common/StreamLog.h"
#include "../../../Common/OSWrappers.h"

#include "Wrapped_ID3D12GraphicsCommandListCustom.h"

//-----------------------------------------------------------------------------
/// A Custom constructor for our wrapped ID3D12GraphicsCommandList.
/// \param inRealCommandList The real runtime instance being wrapped.
//-----------------------------------------------------------------------------
Wrapped_ID3D12GraphicsCommandListCustom::Wrapped_ID3D12GraphicsCommandListCustom(ID3D12GraphicsCommandList* inRealCommandList) :
    Wrapped_ID3D12GraphicsCommandList(inRealCommandList),
    m_pProfiler(nullptr),
    m_profiledCallCount(0),
    m_potentialProfiledCallCount(0),
    m_potentialProfiledCallCountHighest(0)
{
    m_cmdListType = mRealGraphicsCommandList->GetType();
}

//-----------------------------------------------------------------------------
/// Destructor used to release any profilers.
//-----------------------------------------------------------------------------
Wrapped_ID3D12GraphicsCommandListCustom::~Wrapped_ID3D12GraphicsCommandListCustom()
{
    DestroyProfilers();
}

//-----------------------------------------------------------------------------
/// A custom implementation of ID3D12Device::Release. Neccessary to close an Profiler instance.
/// \returns D3D12 device reference count.
//-----------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandListCustom::Release()
{
    ULONG result = Wrapped_ID3D12GraphicsCommandList::Release();

    if (result == 0)
    {
        DestroyProfilers();
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Initialize a profiler per-command list.
/// \returns A new profiler instance to be used for this CommandList.
//-----------------------------------------------------------------------------
DX12CmdListProfiler* Wrapped_ID3D12GraphicsCommandListCustom::InitNewProfiler()
{
    DX12CmdListProfiler* pProfiler = nullptr;

    UINT measurementsPerGroup = 256;

#if DYNAMIC_PROFILER_GROUP_SIZING

    if (measurementsPerGroup < m_potentialProfiledCallCountHighest)
    {
        measurementsPerGroup = m_potentialProfiledCallCountHighest;
    }

#endif

    ID3D12Device* pDevice = nullptr;
    mRealGraphicsCommandList->GetDevice(__uuidof(ID3D12Device), reinterpret_cast<void**>(&pDevice));

    DX12CmdListProfilerConfig profilerConfig = {};
    profilerConfig.measurementsPerGroup   = measurementsPerGroup;
    profilerConfig.maxStaleResourceGroups = 0;
    profilerConfig.measurementTypeFlags   = PROFILER_MEASUREMENT_TYPE_TIMESTAMPS;
    profilerConfig.pDevice                = pDevice;
    profilerConfig.pCmdList               = this->mRealGraphicsCommandList;
    profilerConfig.resolveOnClose         = false;
    profilerConfig.newMemClear            = true;
    profilerConfig.newMemClearValue       = 0;

    pProfiler = DX12CmdListProfiler::Create(&profilerConfig);

    return pProfiler;
}

//-----------------------------------------------------------------------------
/// Inject a "Before" timestamp measurement instruction for a profiled call.
/// \param pIdInfo The measurement information for the profiled call.
/// \returns A ProfilerResultCode indicating the operation's success.
//-----------------------------------------------------------------------------
ProfilerResultCode Wrapped_ID3D12GraphicsCommandListCustom::BeginCmdMeasurement(const ProfilerMeasurementId* pIdInfo)
{
    ProfilerResultCode result = PROFILER_FAIL;

    if (m_pProfiler != nullptr)
    {
        result = m_pProfiler->BeginCmdMeasurement(pIdInfo);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Inject an "After" timestamp measurement instruction for a profiled call.
/// \returns A ProfilerResultCode indicating the operation's success.
//-----------------------------------------------------------------------------
ProfilerResultCode Wrapped_ID3D12GraphicsCommandListCustom::EndCmdMeasurement()
{
    ProfilerResultCode result = PROFILER_FAIL;

    if (m_pProfiler != nullptr)
    {
        result = m_pProfiler->EndCmdMeasurement();

        if (result == PROFILER_SUCCESS)
        {
            m_profiledCallCount++;
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// A custom implementation for reset which will notify the profiler of command list closure.
/// \returns D3D12 result code.
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandListCustom::Close()
{
    ScopeLock lock(&m_closedProfilersMutex);

    if (m_pProfiler != nullptr)
    {
        // Update profiler state
        m_pProfiler->NotifyCmdListClosure();

        // Stash the profiler
        m_closedProfilers.push_back(m_pProfiler);

        // We're now done with it
        m_pProfiler = nullptr;
    }

    return Wrapped_ID3D12GraphicsCommandList::Close();
}

//-----------------------------------------------------------------------------
/// A custom implementation for reset which will also reset our profiler.
/// \param pAllocator The CommandAllocator used with this CommandList.
/// \param pInitialState The initial pipeline state to reset the CommandList to.
/// \returns D3D12 result code.
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandListCustom::Reset(ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pInitialState)
{
    HRESULT result = Wrapped_ID3D12GraphicsCommandList::Reset(pAllocator, pInitialState);

    if (m_pProfiler != nullptr)
    {
        m_pProfiler->NotifyCmdListReset();
    }

    // Reset the number of profiled calls in case this CmdList is recycled.
    m_profiledCallCount = 0;

    return result;
}

//-----------------------------------------------------------------------------
/// Use a multi-threaded approach to get the results for the profiled workload on the given Queue.
/// \param pCmdQueue The CommandQueue to retrieve profiling results for.
/// \param targetExecId The Id of the ExecuteCommandLists to retrieve profiling results for.
/// \param outResults A vector of measurement results retrieved from the profiler.
/// \returns A ProfilerResultCode indicating the operation's success.
//-----------------------------------------------------------------------------
ProfilerResultCode Wrapped_ID3D12GraphicsCommandListCustom::GetCmdListResultsMT(
    ID3D12CommandQueue*          pCmdQueue,
    INT64                        targetExecId,
    std::vector<ProfilerResult>& outResults)
{
    ScopeLock lock(&m_closedProfilersMutex);

    ProfilerResultCode result = PROFILER_SUCCESS;

    for (UINT i = 0; i < m_closedProfilers.size(); i++)
    {
        DX12CmdListProfiler* pProfiler = m_closedProfilers[i];

        if (pProfiler != nullptr)
        {
            if (targetExecId == pProfiler->GetExecutionId())
            {
                result = pProfiler->GetCmdListResults(pCmdQueue, outResults);
            }
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Use a single-threaded approach to get the results for the profiled workload on the given Queue.
/// \param pCmdQueue The CommandQueue to retrieve profiling results for.
/// \param outResults A vector of measurement results retrieved from the profiler.
/// \returns A ProfilerResultCode indicating the operation's success.
//-----------------------------------------------------------------------------
ProfilerResultCode Wrapped_ID3D12GraphicsCommandListCustom::GetCmdListResultsST(ID3D12CommandQueue* pCmdQueue, std::vector<ProfilerResult>& outResults)
{
    ProfilerResultCode result = PROFILER_SUCCESS;

    for (UINT i = 0; i < m_closedProfilers.size(); i++)
    {
        DX12CmdListProfiler* pProfiler = m_closedProfilers[i];

        if (pProfiler != nullptr)
        {
            result = pProfiler->GetCmdListResults(pCmdQueue, outResults);
        }
    }

    DestroyProfilers();

    return result;
}

//-----------------------------------------------------------------------------
/// Add the incoming FuncId to an ordered list of calls invoked through this wrapped CommandList instance.
/// \param inFuncId The FuncId for the API call.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12GraphicsCommandListCustom::TrackCommandListCall(FuncId inFuncId)
{
    if (inFuncId == FuncId_ID3D12GraphicsCommandList_Reset)
    {
#if TRACK_CMD_LIST_COMMANDS
        m_commands.clear();
#endif
        m_potentialProfiledCallCount = 0;
    }
    else
    {
#if TRACK_CMD_LIST_COMMANDS
        m_commands.push_back(inFuncId);
#endif

        if (DX12FrameProfilerLayer::Instance()->ShouldProfileFunction(inFuncId))
        {
            // Update profiling state if this is the very first command
            if (m_potentialProfiledCallCount == 0)
            {
                if (DX12FrameProfilerLayer::Instance()->ShouldCollectGPUTime() == true)
                {
                    m_pProfiler = InitNewProfiler();
                }
            }

            m_potentialProfiledCallCount++;

            if (m_potentialProfiledCallCount > m_potentialProfiledCallCountHighest)
            {
                m_potentialProfiledCallCountHighest = m_potentialProfiledCallCount;
            }
        }
    }
}

//-----------------------------------------------------------------------------
/// Destroy all of the profiler objects used with this CommandList.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12GraphicsCommandListCustom::DestroyProfilers()
{
    ScopeLock lock(&m_closedProfilersMutex);

    for (UINT i = 0; i < m_closedProfilers.size(); i++)
    {
        if ((m_closedProfilers[i] != nullptr) && (m_closedProfilers[i] != m_pProfiler))
        {
            delete m_closedProfilers[i];
            m_closedProfilers[i] = nullptr;
        }
    }

    if (m_pProfiler != nullptr)
    {
        delete m_pProfiler;
        m_pProfiler = nullptr;
    }

    m_closedProfilers.clear();
}

//-----------------------------------------------------------------------------
/// Let the profiler know which group of command lists this profiler belongs to.
/// \param inExecutionId The unique Id for the ExecuteCommandLists call executing profiler commands.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12GraphicsCommandListCustom::SetProfilerExecutionId(INT64 inExecutionId)
{
    ScopeLock lock(&m_closedProfilersMutex);

    if (m_closedProfilers.size() > 0)
    {
        m_closedProfilers.back()->SetExecutionId(inExecutionId);
    }
}

#if TRACK_CMD_LIST_COMMANDS
//-----------------------------------------------------------------------------
/// Debug function used to print out command list contents
//-----------------------------------------------------------------------------
void Wrapped_ID3D12GraphicsCommandListCustom::PrintCommands()
{
    Log(logERROR, "===================================================\n");
    Log(logERROR, "CmdList: 0x%p\n", this);

    for (UINT i = 0; i < m_commands.size(); i++)
    {
        Log(logERROR, "%s\n", DX12TraceAnalyzerLayer::Instance()->GetFunctionNameFromId(m_commands[i]));
    }
}
#endif