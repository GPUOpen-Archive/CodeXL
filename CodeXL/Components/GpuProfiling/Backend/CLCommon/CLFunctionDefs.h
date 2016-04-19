//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief The helper file to initialize the OpenCL function pointers.
//==============================================================================

#ifndef _CL_FUNCTIONDEFS_H_
#define _CL_FUNCTIONDEFS_H_

#include <CL/opencl.h>
#ifdef _WIN32
    #include <CL/cl_d3d11.h>
    #include <CL/cl_d3d10.h>
#endif

#include <CL/additions/cl_additions.h>
#include <CL/internal/cl_agent_amd.h>
#include "CLFunctionEnumDefs.h"

/// \addtogroup Common
// @{

/// Pseudo-dispatch table for extension functions that are not included
/// in the ICD (dispatch) table. This works the same as the normal ICD
/// table, allowing us to trace extension functions in the profiler
typedef struct _CLExtensionFunctionTable
{
    clSVMAllocAMD_fn                           SVMAllocAMD;
    clSVMFreeAMD_fn                            SVMFreeAMD;
    clEnqueueSVMFreeAMD_fn                     EnqueueSVMFreeAMD;
    clEnqueueSVMMemcpyAMD_fn                   EnqueueSVMMemcpyAMD;
    clEnqueueSVMMemFillAMD_fn                  EnqueueSVMMemFillAMD;
    clEnqueueSVMMapAMD_fn                      EnqueueSVMMapAMD;
    clEnqueueSVMUnmapAMD_fn                    EnqueueSVMUnmapAMD;
    clSetKernelArgSVMPointerAMD_fn             SetKernelArgSVMPointerAMD;
    clSetKernelExecInfoAMD_fn                  SetKernelExecInfoAMD;

} CLExtensionFunctionTable;

#define OPENCL_FISSION_EXT_SUPPORT ///< undefine this line to remove Device Fission Extension support

/// Dispatch table representing the table read by an agent in clAgent_OnLoad via GetICDDispatchTable.
/// A detoured API should always call the equivalent function in the g_nextDispatchTable.  This call
/// will go to either the next agent in the dispatch table or down to the actual runtime.
/// See "Rules" below.
extern cl_icd_dispatch_table g_nextDispatchTable;

/// Dispatch table representing the real runtime API.  Any agent that needs to call into OpenCL (other
/// that to call a "real" function from a detoured function) should make those OpenCL calls via the
/// g_realDispatchTable. This way agents which are "below" this one in the agent chain will not see
/// calls made by this agent.  They will only see calls made by the application being profiled.
/// See "Rules" below.
extern cl_icd_dispatch_table g_realDispatchTable;

// Pseudo-dispatch table used for detouring extension functions that are not exposed by cl_icd_dispatch_table
// This allows the profiler to intercept those APIs, if necessary.
extern CLExtensionFunctionTable g_realExtensionFunctionTable;

// Rules:
//  1) When calling an API from within a detoured version of that API, you should always call via the "g_nextDispatchTable" global dispatch table.  This ensures that all agents see all APIs called by the user application.
//  2) When calling an API to interact with the OpenCL API, you should always call via the "g_realDispatchTable" global dispatch table.  This ensures that agents lower in the chain do not see calls made by the profiler (and not by the application)
//  3) In general, unless you are adding a detour for a previously-un-detoured API, you should always make OpenCL calls via "g_realDispatchTable".

/// Initializes the g_nextDispatchTable table with the dispatch table read by an agent in
/// clAgent_OnLoad via GetICDDispatchTable
/// \param table the dispatch table read by an agent before replacing the table items with its own detoured functions.
void InitNextCLFunctions(cl_icd_dispatch_table& table);

/// Initializes the g_realDispatchTable with entry points read from the amdocl library.
void InitRealCLFunctions();

/// Attempts to initialize an item in the g_realExtensionFunctionTable table
/// \param pFuncName the name of the extension function being initialized
/// \param pFuncPtr the address of the real extension function pointer
/// \return the CL_FUNC_TYPE of the function detoured, or CL_FUNC_TYPE_Unknown if it is not a detoured function
CL_FUNC_TYPE InitExtensionFunction(const char* pFuncName, void* pFuncPtr);

// *INDENT-OFF*
#ifdef CL_UNITTEST_MOCK
void SetRealDispatchTblToMock(void);
void SetRealDispatchTblToMock(cl_icd_dispatch_table& table);
#endif
// *INDENT-ON*
// this function pointer type should have been defined in cl_gl_ext.h,
// but it is not, so we define it here
typedef CL_API_ENTRY cl_event(CL_API_CALL* clCreateEventFromGLsyncKHR_fn)(
    cl_context              /* context */,
    cl_GLsync               /* cl_GLsync */,
    cl_int*                 /* errcode_ret */) CL_EXT_SUFFIX__VERSION_1_1;


// @}

#endif // _CL_FUNCTIONDEFS_H_
