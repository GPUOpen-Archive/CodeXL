//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains the function to initialize the profiler
//==============================================================================

#ifndef _CL_INIT_PROFILER_H_
#define _CL_INIT_PROFILER_H_

/// \addtogroup CLProfileAgent
// @{

/// This function gets parameters passed from sprofil and load profiler dll
/// For Agent, it gets parameters from tmp file
/// For Detours, it gets parameters through DetourFindPayload()
/// \return true if operation is successful
extern bool InitProfiler();

// @}

#endif
