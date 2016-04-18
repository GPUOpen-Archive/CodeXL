//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains the entry point for the CLTraceAgent.
//==============================================================================

#ifndef _CL_TRACE_AGENT_H
#define _CL_TRACE_AGENT_H

/// \defgroup CLTraceAgent CLTraceAgent
/// This module is the CLTrace backend for the profiler.
/// It provides the following services: \n
/// 1) Injecting code into a OpenCL application through the use of
/// CLAgent provided by CL runtime. \n
/// 2) Keeping track of every OpenCL API call and saving parameter values
/// , return values, API start timestamp, API end timestamp.\n
/// 3) Managing OpenCL events to retrieve GPU time for every enqueue command. \n
///
/// \ingroup Backend
// @{
// @}
#include <CL/opencl.h>
#include <CL/internal/cl_agent_amd.h>

/// Real dispatch table accessor
/// \return real dispatch table pointer
cl_icd_dispatch_table* GetRealDispatchTable();

// @}

#endif //_CL_TRACE_AGENT_H
