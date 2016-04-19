//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csGlobalVariables.h
///
//==================================================================================

//------------------------------ csGlobalVariables.h ------------------------------

#ifndef __CSGLOBALVARIABLES_H
#define __CSGLOBALVARIABLES_H

// ----------------------------------------------------------------------------------
// File Name:   csGlobalVariables
// General Description:
//   This file (and its .cpp) contains global variables used by various OpenCL Server files.
//   All global variables are grouped in one file to ensure proper variables
//   initialization sequence; variables that should be initialized first should
//   appear first in the csGlobalVariables.cpp file (C++ ensures right initialization
//   sequence within the same file, but not across files).
//
//   When applicable (performance wise, etc), prefer using global variables access functions
//   and not these global variables directly. This will enable locating the call stack that
//   led to a global variable change.
//
// Author:               Yaki Tebeka
// Creation Date:       29/10/2009
// ----------------------------------------------------------------------------------

// Forward decelerations:
class suIKernelDebuggingManager;
class csOpenCLMonitor;
class csExtensionsManager;
struct csMonitoredFunctionPointers;
struct csAMDKernelDebuggingFunctionPointers;
// class csAMDKernelDebuggingManager;

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>


// -----------------------------------------------------------
//           Global variables access functions
// -----------------------------------------------------------

void csSetAPIThreadID(osThreadId& APIThreadId);
osThreadId csGetAPIThreadId();
void csSetOpenCLServerRunMode(bool isRunningInStandaloneMode);
bool csIsRunningInStandaloneMode();
void csSetSystemOpenCLICDModuleHandle(osModuleHandle moduleHandle);
osModuleHandle csSystemOpenCLICDModuleHandle();

// -----------------------------------------------------------
//            Global variables direct access
// (Use only when performance considerations does not allow the
//  use of the global variables access functions)
// -----------------------------------------------------------

// The OpenCL technology monitor:
extern csOpenCLMonitor& cs_stat_openCLMonitorInstance;

// The OpenCL extensions manager:
extern csExtensionsManager& cs_stat_extensionsManager;

// Contains pointers to the real implementation of the wrapped "base" functions:
extern csMonitoredFunctionPointers cs_stat_realFunctionPointers;

// Contains pointers to the AMD kernel debugging API functions:
extern csAMDKernelDebuggingFunctionPointers cs_stat_amdKernelDebuggingFunctionPointers;

// The AMD kernel debugging manager:
// The specific class instance should not be explicitly used by other classes:
// extern csAMDKernelDebuggingManager& cs_stat_amdKernelDebuggingManager;
extern suIKernelDebuggingManager* cs_stat_pIKernelDebuggingManager;


#endif //__CSGLOBALVARIABLES_H

