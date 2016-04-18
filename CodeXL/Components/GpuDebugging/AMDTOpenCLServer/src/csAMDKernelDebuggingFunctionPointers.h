//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csAMDKernelDebuggingFunctionPointers.h
///
//==================================================================================

//------------------------------ csAMDKernelDebuggingFunctionPointers.h ------------------------------

#ifndef __CSAMDKERNELDEBUGGINGFUNCTIONPOINTERS_H
#define __CSAMDKERNELDEBUGGINGFUNCTIONPOINTERS_H

// AMD CL Debugging API:
#include <AMDOpenCLDebug.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>

// The number of functions in the csAMDKernelDebuggingFunctionPointers struct:
#define CS_NUMBER_OF_OPENCL_DEBUGGING_API_FUNCTIONS 29

// ----------------------------------------------------------------------------------
// Class Name:          csAMDKernelDebuggingFunctionPointers
// General Description: A structure holding pointers to all functions in the AMD kernel
//                      debugging APIs.
// Author:              Uri Shomroni
// Creation Date:       24/10/2010
// ----------------------------------------------------------------------------------
struct csAMDKernelDebuggingFunctionPointers
{
    //////////////////////////////////////////////////////////////////////////
    // Note: when adding a function pointer to this struct, add its name to
    // The openCLDebuggingAPIFunctionNames array in initializeAMDKernelDebugging
    // as well, IN THE SAME ORDER.
    //////////////////////////////////////////////////////////////////////////

    // Interception functions:
    cl_kernel(CL_API_CALL* amdclInterceptCreateKernel)(cl_program program, const char* kernel_name, cl_int* errcode_ret);
    cl_int(CL_API_CALL* amdclInterceptCreateKernelsInProgram)(cl_program program, cl_uint num_kernels, cl_kernel* kernels, cl_uint* num_kernels_ret);
    cl_int(CL_API_CALL* amdclInterceptRetainKernel)(cl_kernel kernel);
    cl_int(CL_API_CALL* amdclInterceptReleaseKernel)(cl_kernel kernel);
    cl_int(CL_API_CALL* amdclInterceptSetKernelArg)(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void* arg_value);
    cl_context(CL_API_CALL* amdclInterceptCreateContext)(const cl_context_properties* properties, cl_uint num_devices, const cl_device_id* devices, void (CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*), void* user_data, cl_int* errcode_ret);
    cl_context(CL_API_CALL* amdclInterceptCreateContextFromType)(const cl_context_properties* properties, cl_device_type device_type, void (CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*), void* user_data, cl_int* errcode_ret);
    cl_command_queue(CL_API_CALL* amdclInterceptCreateCommandQueue)(cl_context context, cl_device_id device, cl_command_queue_properties properties, cl_int* errcode_ret);
    cl_command_queue(CL_API_CALL* amdclInterceptCreateCommandQueueWithProperties)(cl_context context, cl_device_id device, const cl_queue_properties* properties, cl_int* errcode_ret);

    // Core debugger functions:
    cl_int(CL_API_CALL* amdclDebugEnqueueNDRangeKernel)(cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim, const size_t* global_work_offset, const size_t* global_work_size, const size_t* local_work_size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event, amdclDebugCallback callback, void* user_data);
    cl_int(CL_API_CALL* amdclDebugEnqueueTask)(cl_command_queue command_queue, cl_kernel kernel, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event, amdclDebugCallback callback, void* user_data);
    amdclDebugError(CL_API_CALL* amdclDebugGetProgramCounter)(amdclDebugContext debugContext, amdclDebugPC* programCounter);
    amdclDebugError(CL_API_CALL* amdclDebugSetBreakpoint)(amdclDebugContext debugContext, amdclDebugPC programCounter, cl_uint lookAhead, amdclDebugPC* actualPC);
    amdclDebugError(CL_API_CALL* amdclDebugClearBreakpoint)(amdclDebugContext debugContext, amdclDebugPC programCounter);
    amdclDebugError(CL_API_CALL* amdclDebugClearAllBreakpoints)(amdclDebugContext debugContext);
    amdclDebugError(CL_API_CALL* amdclDebugGetNumBreakpoints)(amdclDebugContext debugContext, size_t* numBreakpoints);
    amdclDebugError(CL_API_CALL* amdclDebugGetAllBreakpoints)(amdclDebugContext debugContext, amdclDebugPC* breakpointList, size_t listLength);
    amdclDebugError(CL_API_CALL* amdclDebugGetKernelBinarySize)(amdclDebugContext debugContext, size_t* debugBinarySize);
    amdclDebugError(CL_API_CALL* amdclDebugGetKernelBinary)(amdclDebugContext debugContext, void* debugBinary, size_t debugBinarySize);
    amdclDebugError(CL_API_CALL* amdclDebugGetExecutionMask)(amdclDebugContext debugContext, const cl_bool** executionMask, size_t* numElements);
    amdclDebugError(CL_API_CALL* amdclDebugSetTrackingRegisters)(amdclDebugContext debugContext, const amdclDebugRegisterLocator* debugRegisterLocators, size_t numDebugRegisterLocators);
    amdclDebugError(CL_API_CALL* amdclDebugGetNumberOfActiveRegisters)(amdclDebugContext debugContext, size_t* numActiveRegisters);
    amdclDebugError(CL_API_CALL* amdclDebugGetActiveRegisters)(amdclDebugContext debugContext, amdclDebugRegisterLocator* debugRegisterLocators, size_t numActiveRegisters);
    amdclDebugError(CL_API_CALL* amdclDebugGetRegisterValues)(amdclDebugContext debugContext, amdclDebugRegisterLocator debugRegisterLocator, const void** outputBuffer, size_t* outputStride, size_t* numElements);
    amdclDebugError(CL_API_CALL* amdclDebugGetPrivateMemoryValues)(amdclDebugContext debugContext, amdclDebugMemoryAddress address, size_t count, void* buffer, size_t* outputSize);
    amdclDebugError(CL_API_CALL* amdclDebugGetGlobalMemoryValues)(amdclDebugContext debugContext, amdclDebugMemoryResource resource, amdclDebugMemoryAddress address, size_t count, void* buffer, size_t* outputSize);
    amdclDebugError(CL_API_CALL* amdclDebugUtilGetLastError)(amdclDebugContext debugContext, size_t bufferSize, char* buffer, size_t* bufferSizeRet);


    // TO_DO: get real function signatures:
    /*  void (CL_API_CALL* amdclDebugGetCallStack)();
        void (CL_API_CALL* amdclDebugGetCallStackExecutionMask)();*/

    // Dispatch functions:
    cl_int(CL_API_CALL* amdclDebugGetDispatchTable)(cl_icd_dispatch_table* pCLDispatchTable);
    cl_int(CL_API_CALL* amdclDebugSetDispatchTable)(cl_icd_dispatch_table* pCLDispatchTable);

    // Utility functions:
    // We don't currently need / use these functions:
    /*  amdclDebugError (CL_API_CALL* amdclDebugUtilStringToDebugRegisterLocator)(const cl_char* registerName, amdclDebugRegisterLocator* debugRegisterLocator);
        amdclDebugError (CL_API_CALL* amdclDebugUtilDebugRegisterLocatorToString)(amdclDebugRegisterLocator debugRegisterLocator, size_t bufferSize, cl_char* registerNameBuffer, size_t* bufferSizeRet);*/
};

#endif //__CSAMDKERNELDEBUGGINGFUNCTIONPOINTERS_H

