//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief A collection of functions to handle the interaction with
///        the OpenCL Profiling Agent API.
//==============================================================================

#include <string.h>
#include <iostream>
#include "CLFunctionDefs.h"
#include "CLProfileAgent.h"
#include "CLProfilerMineCLEntry.h"
#include "CLInitProfiler.h"
#include "CLGPAProfiler.h"
#include "../Common/Logger.h"
#include "../Common/Defs.h"
#include "../Common/Version.h"
#include "../Common/FileUtils.h"

static cl_icd_dispatch_table original_dispatch;
static cl_icd_dispatch_table modified_dispatch;

extern CLGPAProfiler g_Profiler;

extern "C" DLL_PUBLIC void amdtCodeXLStopProfiling()
{
    g_Profiler.EnableProfiling(false);
}

extern "C" DLL_PUBLIC void amdtCodeXLResumeProfiling()
{
    g_Profiler.EnableProfiling(true);
}

cl_int CL_CALLBACK
clAgent_OnLoad(cl_agent* agent)
{
#ifdef _DEBUG
    FileUtils::CheckForDebuggerAttach();
#endif

    std::cout << "CodeXL GPU Profiler " << GPUPROFILER_BACKEND_VERSION_STRING << " is Enabled\n";

    cl_int err = agent->GetICDDispatchTable(
                     agent, &original_dispatch, sizeof(original_dispatch));

    if (err != CL_SUCCESS)
    {
        return err;
    }

    memcpy(&modified_dispatch, &original_dispatch, sizeof(modified_dispatch));

    InitNextCLFunctions(original_dispatch);
    CreateMineDispatchTable(modified_dispatch);

    err = agent->SetICDDispatchTable(
              agent, &modified_dispatch, sizeof(modified_dispatch));

    if (false == InitProfiler())
    {
        return CL_PROFILING_INFO_NOT_AVAILABLE;
    }

    if (err != CL_SUCCESS)
    {
        return err;
    }

    return CL_SUCCESS;
}
