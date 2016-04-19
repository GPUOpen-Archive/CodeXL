//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This contains the initialization function for API tracing.
//==============================================================================

#ifndef _CL_API_TRACE_ENTRY_H_
#define _CL_API_TRACE_ENTRY_H_

/// \defgroup CLAPITraceEntry CLAPITraceEntry
/// This module implements Mine_* functions of all OpenCL API and create a MineDispatch table to replace the RealDispatch table
///
/// \ingroup CLTraceAgent
// @{

#include <CL/cl_ext.h>
#include <CL/internal/cl_agent_amd.h>
#include <string>

void CreateAPITraceDispatchTable(cl_icd_dispatch_table& dispatchTable);
void SetGlobalTraceFlags(bool queryRetStat, bool collapseClGetEventInfo);

// @}

#endif // _CL_API_TRACE_ENTRY_H_
