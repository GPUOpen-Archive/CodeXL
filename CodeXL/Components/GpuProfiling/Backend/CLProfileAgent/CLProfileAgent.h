//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief A collection of functions to handle the interaction with
///        the OpenCL Profiling Agent API.
//==============================================================================

#ifndef _CL_PROFILE_AGENT_H
#define _CL_PROFILE_AGENT_H

/// \defgroup CLProfileAgent CLProfileAgent
/// This module is the OpenCL backend for the profiler.
/// It provides the following services: \n
/// 1) Injecting code into an OpenCL application through the use of
/// clAgent.\n
/// 2) Interfacing with the GPUPerfAPI library to query and output
/// GPU performance counters. \n
/// 3) Tracking OpenCL kernel arguments for loading and saving memory state to
/// be able to dispatch the kernel multiple times consistently. \n
/// 4) Generating OpenCL IL and ISA through the use of OpenCL BIF format (ELF).
///
/// \ingroup Backend

/// \addtogroup CLProfileAgent
// @{

#include <CL/internal/cl_agent_amd.h>

// @}

#endif //_CL_PROFILE_AGENT_H
