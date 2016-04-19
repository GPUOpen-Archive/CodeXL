//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief The header file for detouring the CL entry functions.
//==============================================================================

#ifndef _CL_PROFILER_MINE_CL_ENTRY_H_
#define _CL_PROFILER_MINE_CL_ENTRY_H_

#include <CL/opencl.h>
#include <CL/internal/cl_agent_amd.h>

/// \addtogroup CLProfileAgent
// @{
void CreateMineDispatchTable(cl_icd_dispatch_table& dispatchTable);
// @}

#endif //_CL_PROFILER_MINE_CL_ENTRY_H_
