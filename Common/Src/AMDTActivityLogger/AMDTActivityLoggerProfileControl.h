//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file AMDTActivityLoggerProfileControl.cpp
/// \brief  Implementation of the AMDTActivityLogger Profile Control singleton
//==============================================================================

#ifndef _AMDT_ACTIVITY_LOGGER_PROFILE_CONTROL_H_
#define _AMDT_ACTIVITY_LOGGER_PROFILE_CONTROL_H_

#include "TSingleton.h"

#include "Common/OSUtils.h"

#include "AMDTActivityLogger.h"


/// function typedef for the profiling control functions exported from the agent libraries
typedef void(*ProfilingControlProc)();

/// function typedef for the profiling control functions exported from the agent libraries -- this version passes the mode being started/stopped
typedef void(*ProfilingControlProcWithMode)(amdtProfilingControlMode);

/// Singleton class to interact with agents to stop/resume profiling
class AMDTActivityLoggerProfileControl : public TSingleton <AMDTActivityLoggerProfileControl>
{
public:
    /// Tell the profiler to stop profiling
    /// \param profilingControlMode the profiling mode being stopped
    /// \return AL_SUCCESS if at least one of the profiling agents is loaded and the the Stop Profiling entry point is called, AL_FAILED_TO_ATTACH_TO_PROFILER otherwise
    int StopProfiling(amdtProfilingControlMode profilingControlMode);

    /// Tell the profiler to resume profiling
    /// \param profilingControlMode the profiling mode being resumed
    /// \return AL_SUCCESS if at least one of the profiling agents is loaded and the the Resume Profiling entry point is called, AL_FAILED_TO_ATTACH_TO_PROFILER otherwise
    int ResumeProfiling(amdtProfilingControlMode profilingControlMode);

private:
    /// Helper function to initialize lib handles and function pointers, and to call the specified function pointer
    /// \param[in,out] libHandle the handle of the library to initialize the entry point from
    /// \param[in] pLibName the name of the library to initialize the entry point from
    /// \param[in,out] profilingControlProc the function pointer for the entry point
    /// \param[in] pProcName the name of the entry point
    /// \return true on success, false otherwise
    bool CallProfileControlEntryPointFromLibrary(LIB_HANDLE& libHandle, const char* pLibName, ProfilingControlProc& profilingControlProc, const char* pProcName);

    /// Helper function to initialize lib handles and function pointers, and to call the specified function pointer -- this method passes the mode param to the agent
    /// \param[in,out] libHandle the handle of the library to initialize the entry point from
    /// \param[in] pLibName the name of the library to initialize the entry point from
    /// \param[in,out] profilingControlProc the function pointer for the entry point
    /// \param[in] pProcName the name of the entry point
    /// \param[in] mode the profiling mode being stopped or resumed
    /// \return true on success, false otherwise
    bool CallProfileControlEntryPointFromLibraryWithMode(LIB_HANDLE& libHandle, const char* pLibName, ProfilingControlProcWithMode& profilingControlProc, const char* pProcName, amdtProfilingControlMode mode);

    LIB_HANDLE           m_clTraceAgentHandle = NULL;                 ///< handle to the CL Tracing Agent module
    LIB_HANDLE           m_hsaTraceAgentHandle = NULL;                ///< handle to the HSA Tracing Agent module
    LIB_HANDLE           m_clProfilingAgentHandle = NULL;             ///< handle to the CL Profiling Agent module
    LIB_HANDLE           m_hsaProfilingAgentHandle = NULL;            ///< handle to the HSA Profiling Agent module
    LIB_HANDLE           m_clOccupancyAgentHandle = NULL;             ///< handle to the CL Occupancy Agent module

    ProfilingControlProc m_pCLTraceStopProfilingProc = NULL;          ///< Pointer to the CL StopTracing entry point
    ProfilingControlProc m_pCLTraceResumeProfilingProc = NULL;        ///< Pointer to the CL ResumeTracing entry point
    ProfilingControlProc m_pCLPerfCounterStopProfilingProc = NULL;    ///< Pointer to the CL StopProfiling entry point
    ProfilingControlProc m_pCLPerfCounterResumeProfilingProc = NULL;  ///< Pointer to the CL ResumeProfiling entry point

    ProfilingControlProc m_pHSATraceStopProfilingProc = NULL;         ///< Pointer to the HSA StopTracing entry point
    ProfilingControlProc m_pHSATraceResumeProfilingProc = NULL;       ///< Pointer to the HSA ResumeTracing entry point
    ProfilingControlProc m_pHSAPerfCounterStopProfilingProc = NULL;   ///< Pointer to the HSA StopProfiling entry point
    ProfilingControlProc m_pHSAPerfCounterResumeProfilingProc = NULL; ///< Pointer to the HSA ResumeProfiling entry point

    ProfilingControlProcWithMode m_pCLOccupancyStopProfilingProc = NULL;   ///< Pointer to the CL Occupancy StopProfiling entry point
    ProfilingControlProcWithMode m_pCLOccupancyResumeProfilingProc = NULL; ///< Pointer to the CL Occupancy ResumeProfiling entry point
};

#endif // _AMDT_ACTIVITY_LOGGER_PROFILE_CONTROL_H_
