//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file AMDTActivityLoggerProfileControl.cpp
/// \brief  Implementation of the AMDTActivityLogger Profile Control singleton
//==============================================================================

#include "AMDTActivityLoggerProfileControl.h"

bool AMDTActivityLoggerProfileControl::CallProfileControlEntryPointFromLibrary(LIB_HANDLE& libHandle, const char* pLibName, ProfilingControlProc& profilingControlProc, const char* pProcName)
{
    bool retVal = false;

    if (libHandle == NULL)
    {
        libHandle = OSUtils::Instance()->GetLibraryHandle(pLibName);
    }

    if (libHandle != NULL)
    {
        if (profilingControlProc == NULL)
        {
            profilingControlProc = reinterpret_cast<ProfilingControlProc>(OSUtils::Instance()->GetSymbolAddr(libHandle, pProcName));
        }

        if (profilingControlProc != NULL)
        {
            profilingControlProc();
            retVal = true;
        }
    }

    return retVal;
}

bool AMDTActivityLoggerProfileControl::CallProfileControlEntryPointFromLibraryWithMode(LIB_HANDLE& libHandle, const char* pLibName, ProfilingControlProcWithMode& profilingControlProc, const char* pProcName, amdtProfilingControlMode mode)
{
    bool retVal = false;

    if (libHandle == NULL)
    {
        libHandle = OSUtils::Instance()->GetLibraryHandle(pLibName);
    }

    if (libHandle != NULL)
    {
        if (profilingControlProc == NULL)
        {
            profilingControlProc = reinterpret_cast<ProfilingControlProcWithMode>(OSUtils::Instance()->GetSymbolAddr(libHandle, pProcName));
        }

        if (profilingControlProc != NULL)
        {
            profilingControlProc(mode);
            retVal = true;
        }
    }

    return retVal;
}

// NOTE: the following 2 functions will only work with an internal build if you're
//       also using an internal version of the ActivityLogger library

int AMDTActivityLoggerProfileControl::StopProfiling(amdtProfilingControlMode profilingControlMode)
{
    int retVal = AL_FAILED_TO_ATTACH_TO_PROFILER;
    bool procCalled = false;

    if ((profilingControlMode & AMDT_TRACE_PROFILING) == AMDT_TRACE_PROFILING)
    {
        procCalled = CallProfileControlEntryPointFromLibrary(m_clTraceAgentHandle, CL_TRACE_AGENT_DLL, m_pCLTraceStopProfilingProc, "amdtCodeXLStopProfiling");
        procCalled |= CallProfileControlEntryPointFromLibrary(m_hsaTraceAgentHandle, HSA_TRACE_AGENT_DLL, m_pHSATraceStopProfilingProc, "amdtCodeXLStopProfiling");
    }

    if ((profilingControlMode & AMDT_PERF_COUNTER_PROFILING) == AMDT_PERF_COUNTER_PROFILING)
    {
        procCalled = CallProfileControlEntryPointFromLibrary(m_clProfilingAgentHandle, CL_PROFILE_AGENT_DLL, m_pCLPerfCounterStopProfilingProc, "amdtCodeXLStopProfiling");
        procCalled |= CallProfileControlEntryPointFromLibrary(m_hsaProfilingAgentHandle, HSA_PROFILE_AGENT_DLL, m_pHSAPerfCounterStopProfilingProc, "amdtCodeXLStopProfiling");
    }

    procCalled |= CallProfileControlEntryPointFromLibraryWithMode(m_clOccupancyAgentHandle, CL_OCCUPANCY_AGENT_DLL, m_pCLOccupancyStopProfilingProc, "amdtCodeXLStopProfiling", profilingControlMode);

    if (procCalled)
    {
        retVal = AL_SUCCESS;
    }

    return retVal;
}

int AMDTActivityLoggerProfileControl::ResumeProfiling(amdtProfilingControlMode profilingControlMode)
{
    int retVal = AL_FAILED_TO_ATTACH_TO_PROFILER;
    bool procCalled = false;

    if ((profilingControlMode & AMDT_TRACE_PROFILING) == AMDT_TRACE_PROFILING)
    {
        procCalled = CallProfileControlEntryPointFromLibrary(m_clTraceAgentHandle, CL_TRACE_AGENT_DLL, m_pCLTraceResumeProfilingProc, "amdtCodeXLResumeProfiling");
        procCalled |= CallProfileControlEntryPointFromLibrary(m_hsaTraceAgentHandle, HSA_TRACE_AGENT_DLL, m_pHSATraceResumeProfilingProc, "amdtCodeXLResumeProfiling");
    }

    if ((profilingControlMode & AMDT_PERF_COUNTER_PROFILING) == AMDT_PERF_COUNTER_PROFILING)
    {
        procCalled = CallProfileControlEntryPointFromLibrary(m_clProfilingAgentHandle, CL_PROFILE_AGENT_DLL, m_pCLPerfCounterResumeProfilingProc, "amdtCodeXLResumeProfiling");
        procCalled |= CallProfileControlEntryPointFromLibrary(m_hsaProfilingAgentHandle, HSA_PROFILE_AGENT_DLL, m_pHSAPerfCounterResumeProfilingProc, "amdtCodeXLResumeProfiling");
    }

    procCalled |= CallProfileControlEntryPointFromLibraryWithMode(m_clOccupancyAgentHandle, CL_OCCUPANCY_AGENT_DLL, m_pCLOccupancyResumeProfilingProc, "amdtCodeXLResumeProfiling", profilingControlMode);

    if (procCalled)
    {
        retVal = AL_SUCCESS;
    }

    return retVal;
}

