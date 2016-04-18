//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csGlobalVariables.cpp
///
//==================================================================================

//------------------------------ csGlobalVariables.cpp ------------------------------

// Local:
#include <src/csAMDKernelDebuggingFunctionPointers.h>
#include <src/csAMDKernelDebuggingManager.h>
#include <src/csExtensionsManager.h>
#include <src/csGlobalVariables.h>
#include <src/csMonitoredFunctionPointers.h>
#include <src/csOpenCLMonitor.h>

// -----------------------------------------------------------
//           Global variables instances
// -----------------------------------------------------------

// The OpenCL technology monitor:
csOpenCLMonitor& cs_stat_openCLMonitorInstance = csOpenCLMonitor::instance();

// The OpenCL extensions manager:
csExtensionsManager& cs_stat_extensionsManager = csExtensionsManager::instance();

// Contains pointers to the real implementation of the wrapped "base" functions:
csMonitoredFunctionPointers cs_stat_realFunctionPointers;

// Contains pointers to the AMD kernel debugging API functions:
csAMDKernelDebuggingFunctionPointers cs_stat_amdKernelDebuggingFunctionPointers;

// The AMD kernel debugging manager:
csAMDKernelDebuggingManager& cs_stat_amdKernelDebuggingManager = csAMDKernelDebuggingManager::instance();

// The shared manager instance - instantiated once we know if we are using hardware debugger or software debugger.
suIKernelDebuggingManager* cs_stat_pIKernelDebuggingManager = NULL;

// The OpenCL Server API thread id.
// (We are using this id to make sure we are not using it to read API calls after the debugged process starts terminating)
osThreadId cs_stat_SpyAPIThreadId = OS_NO_THREAD_ID;

// Contains true iff the OpenCL server is running in a "standalone" mode (without an attached debugger):
bool cs_state_isRunningInStandaloneMode = true;

// The system's OpenCL ICD module handle:
osModuleHandle cs_stat_systemOpenCLICDModuleHandle = OS_NO_MODULE_HANDLE;


// -----------------------------------------------------------
//           Global variables access functions
// -----------------------------------------------------------


// ---------------------------------------------------------------------------
// Name:        csSetAPIThreadID
// Description: Logs the OpenCL Server API thread id.
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
void csSetAPIThreadID(osThreadId& APIThreadId)
{
    cs_stat_SpyAPIThreadId = APIThreadId;
}


// ---------------------------------------------------------------------------
// Name:        csGetAPIThreadId
// Description: Returns the OpenCL Server API thread id.
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
osThreadId csGetAPIThreadId()
{
    return cs_stat_SpyAPIThreadId;
}


// ---------------------------------------------------------------------------
// Name:        csSetOpenCLServerRunMode
// Description: Sets the OpenCL Server run mode.
// Arguments:
//   isRunningInStandaloneMode - true - to mark that the OpenCl server runs in a
//                                      "standalone" mode (without an attached debugger)
//                             - false - the OpenCL server runs with a debugger.
// Return Val: void
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
void csSetOpenCLServerRunMode(bool isRunningInStandaloneMode)
{
    cs_state_isRunningInStandaloneMode = isRunningInStandaloneMode;
}


// ---------------------------------------------------------------------------
// Name:        csIsRunningInStandaloneMode
// Description:
//   Returns true iff the OpenCL server runs in a "standalone" mode (without an attached debugger).
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
bool csIsRunningInStandaloneMode()
{
    return cs_state_isRunningInStandaloneMode;
}


// ---------------------------------------------------------------------------
// Name:        csSetSystemOpenCLICDModuleHandle
// Description: Logs the loaded system's OpenCL ICD module handle.
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
void csSetSystemOpenCLICDModuleHandle(osModuleHandle moduleHandle)
{
    cs_stat_systemOpenCLICDModuleHandle = moduleHandle;
}


// ---------------------------------------------------------------------------
// Name:        csSystemOpenCLICDModuleHandle
// Description: Returns the loaded system's OpenCL ICD module handle.
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
osModuleHandle csSystemOpenCLICDModuleHandle()
{
    return cs_stat_systemOpenCLICDModuleHandle;
}

