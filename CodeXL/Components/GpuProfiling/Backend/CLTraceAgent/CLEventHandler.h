//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains the CL event handling code.
//==============================================================================

#ifndef _CL_EVENT_HANDLER_H_
#define _CL_EVENT_HANDLER_H_

/// \defgroup CLEventHandler CLEventHandler
/// This module implements CL event callback to retrieve GPU timestamp and interact with CLEventManager
///
/// \ingroup CLTraceAgent
// @{

#include <CL/internal/cl_agent_amd.h>
#include <CL/opencl.h>

// Create CLEvent Callback dispatch Table
// \param callback[inout] return callback dispatch table
void CreateCLEventCallbackDispatchTable(cl_agent_callbacks& callback);

// @}

#endif //_CL_EVENT_HANDLER_H_
