//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Entry point for computing occupancy
//==============================================================================

#include <CL/opencl.h>
#include <CL/internal/cl_agent_amd.h>
#include <string>
#include <sstream>
#include <iostream>
#include <cstring>
#include "../Common/Logger.h"
#include "../Common/Version.h"
#include "../Common/FileUtils.h"
#include "../Common/GlobalSettings.h"
#include "../Common/StringUtils.h"
#include "../CLCommon/CLFunctionDefs.h"
#include "AMDTActivityLogger/AMDTActivityLogger.h"
#include "CLIntercept.h"
#include "CLOccupancyInfoManager.h"

static cl_icd_dispatch_table dispatch;
static cl_icd_dispatch_table agentDispatch;

using namespace std;
using namespace GPULogger;

void DumpOccupancy()
{
    static bool alreadyDumped = false;

    if (!alreadyDumped)
    {
        alreadyDumped = true;

        if (!OccupancyInfoManager::Instance()->IsTimeOutMode())
        {
            OccupancyInfoManager::Instance()->SaveToOccupancyFile();
        }

        OccupancyInfoManager::Instance()->Release();
    }
}

void TimerThread(void* params)
{
    SP_UNREFERENCED_PARAMETER(params);

    unsigned int interval = OccupancyInfoManager::Instance()->GetInterval();

    if (interval == 0)
    {
        interval = 1; // safety net in case interval is zero (it shouldn't be...)
    }

    const unsigned int sleepInterval = interval < 10 ? interval : 10; // sleep at most 10 ms at a time
    const unsigned int sleepsBeforeFlush = sleepInterval == 0 ? 1 : interval / sleepInterval;

    unsigned int iterationNum = 1;

    while (OccupancyInfoManager::Instance()->IsRunning())
    {
        OSUtils::Instance()->SleepMillisecond(sleepInterval);

        if (iterationNum == sleepsBeforeFlush)
        {
            iterationNum = 1;
            OccupancyInfoManager::Instance()->TrySwapBuffer();
            OccupancyInfoManager::Instance()->FlushTraceData();
        }
        else
        {
            iterationNum++;
        }
    }
}

extern "C" DLL_PUBLIC void amdtCodeXLStopProfiling(amdtProfilingControlMode mode)
{
    bool isCLTraceProfiling = GlobalSettings::GetInstance()->m_params.m_bTrace;
    bool isCLPerfCounterProfiling = GlobalSettings::GetInstance()->m_params.m_bPerfCounter;

    bool shouldStop = false;

    if (isCLTraceProfiling)
    {
        // stop if user asked to stop tracing, and we're also tracing
        shouldStop = (mode & AMDT_TRACE_PROFILING) == AMDT_TRACE_PROFILING;
    }
    else if (isCLPerfCounterProfiling)
    {
        // stop if user asked to stop perf counting, and we're also perf counting
        shouldStop = (mode & AMDT_PERF_COUNTER_PROFILING) == AMDT_PERF_COUNTER_PROFILING;
    }
    else
    {
        // stop if user asked to stop all profiling if we're not tracing or perf counting
        shouldStop = mode == AMDT_ALL_PROFILING;
    }

    if (shouldStop)
    {
        OccupancyInfoManager::Instance()->EnableProfiling(false);
    }
}

extern "C" DLL_PUBLIC void amdtCodeXLResumeProfiling(amdtProfilingControlMode mode)
{
    bool isCLTraceProfiling = GlobalSettings::GetInstance()->m_params.m_bTrace;
    bool isCLPerfCounterProfiling = GlobalSettings::GetInstance()->m_params.m_bPerfCounter;

    bool shouldStart = false;

    if (isCLTraceProfiling)
    {
        // start if user asked to start tracing, and we're also tracing
        shouldStart = (mode & AMDT_TRACE_PROFILING) == AMDT_TRACE_PROFILING;
    }
    else if (isCLPerfCounterProfiling)
    {
        // start if user asked to start perf counting, and we're also perf counting
        shouldStart = (mode & AMDT_PERF_COUNTER_PROFILING) == AMDT_PERF_COUNTER_PROFILING;
    }
    else
    {
        // start if user asked to start all profiling if we're not tracing or perf counting
        shouldStart = mode == AMDT_ALL_PROFILING;
    }

    if (shouldStart)
    {
        OccupancyInfoManager::Instance()->EnableProfiling(true);
    }
}

cl_int CL_CALLBACK
clAgent_OnLoad(cl_agent* agent)
{
#ifdef _DEBUG
    FileUtils::CheckForDebuggerAttach();
#endif

    cl_int status = CL_SUCCESS;

    std::cout << "CodeXL GPU Profiler Kernel occupancy module is enabled" << std::endl;

    status = agent->GetICDDispatchTable(agent, &dispatch, sizeof(cl_icd_dispatch_table));

    if (status != CL_SUCCESS)
    {
        return status;
    }

    InitNextCLFunctions(dispatch);

    memcpy(&agentDispatch, &dispatch, sizeof(cl_icd_dispatch_table));

    agentDispatch.EnqueueNDRangeKernel = CL_OCCUPANCY_API_ENTRY_EnqueueNDRangeKernel;
    agentDispatch.ReleaseContext = CL_OCCUPANCY_API_ENTRY_ReleaseContext;

    status = agent->SetICDDispatchTable(agent, &agentDispatch, sizeof(cl_icd_dispatch_table));

    std::string strLogFile = FileUtils::GetDefaultOutputPath() + "cloccupancyagent.log";
    LogFileInitialize(strLogFile.c_str());

    Parameters params;
    FileUtils::GetParametersFromFile(params);
    OccupancyInfoManager::Instance()->SetOutputFile(params.m_strOutputFile);
    GlobalSettings::GetInstance()->m_params = params;
    OccupancyInfoEntry::m_cListSeparator = params.m_cOutputSeparator;

    if (params.m_bTimeOutBasedOutput)
    {
        OccupancyInfoManager::Instance()->SetInterval(params.m_uiTimeOutInterval);

        if (!OccupancyInfoManager::Instance()->StartTimer(TimerThread))
        {
            std::cout << "Failed to initialize CLOccupancyAgent." << std::endl;
        }
    }

    return status;
}


#ifdef _WIN32

extern "C" DLL_PUBLIC void OnExitProcess()
{
    DumpOccupancy();
}

// On Windows, we can't dump data in OnUnload() because of ocl runtime bug
BOOL APIENTRY DllMain(HMODULE,
                      DWORD   ul_reason_for_call,
                      LPVOID)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_PROCESS_DETACH:
        {
            // Dump all data out
            DumpOccupancy();
        }
        break;

        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}
#endif
