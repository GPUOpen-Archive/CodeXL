//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csOpenCLWrappers.cpp
///
//==================================================================================

//------------------------------ csOpenCLWrappers.cpp ------------------------------


// ------------------------------------------------------------------------
// File:
// This file contains a wrapper function for the "base" OpenCL functions
// (functions that are exported from the system's OpenCL module)
// ------------------------------------------------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osTransferableObjectType.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTAPIClasses/Include/apCLEnqueuedCommands.h>
#include <AMDTAPIClasses/Include/apFrameTerminators.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suIKernelDebuggingManager.h>
#include <AMDTServerUtilities/Include/suInterceptionFunctions.h>
#include <AMDTServerUtilities/Include/suInterceptionMacros.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>

// Currently only enable hardware debugging on Windows:
#if defined(CS_USE_HD_HSA_HW_BASED_DEBUGGER) && (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    #include <AMDTHsaDebugging/Include/hdGlobalVariables.h>
    #include <AMDTHsaDebugging/Include/hdHSAHardwareBasedDebuggingManager.h>
#endif

// Local:
#include <src/csAMDKernelDebuggingFunctionPointers.h>
#include <src/csAMDKernelDebuggingManager.h>
#include <src/csExtensionsManager.h>
#include <src/csGlobalVariables.h>
#include <src/csMonitoredFunctionPointers.h>
#include <src/csOpenCLMonitor.h>
#include <src/csOpenCLWrappersAidMacros.h>
#include <src/csStringConstants.h>

// --------------------------------------------------------
//             OpenCL Wrapper functions
// --------------------------------------------------------

cl_int CL_API_CALL clGetPlatformIDs(cl_uint num_entries, cl_platform_id* platforms, cl_uint* num_platforms)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetPlatformIDs);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall(OA_CL_NULL_HANDLE, ap_clGetPlatformIDs, 3, OS_TOBJ_ID_CL_UINT_PARAMETER, num_entries, OS_TOBJ_ID_POINTER_PARAMETER, platforms, OS_TOBJ_ID_POINTER_PARAMETER, num_platforms);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clGetPlatformIDs(num_entries, platforms, num_platforms);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetPlatformIDs);

    return retVal;
}

cl_int CL_API_CALL clGetPlatformInfo(cl_platform_id platform, cl_platform_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetPlatformInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall(OA_CL_NULL_HANDLE, ap_clGetPlatformInfo, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, platform, OS_TOBJ_ID_CL_ENUM_PARAMETER, param_name,
                                                  OS_TOBJ_ID_SIZE_T_PARAMETER, param_value_size, OS_TOBJ_ID_POINTER_PARAMETER, param_value, OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clGetPlatformInfo(platform, param_name, param_value_size, param_value, param_value_size_ret);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetPlatformInfo);

    return retVal;
}


// Device APIs
cl_int CL_API_CALL clGetDeviceIDs(cl_platform_id platform, cl_device_type device_type, cl_uint num_entries, cl_device_id* devices, cl_uint* num_devices)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetDeviceIDs);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall(OA_CL_NULL_HANDLE, ap_clGetDeviceIDs, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, platform, OS_TOBJ_ID_CL_DEVICE_TYPE_PARAMETER, device_type,
                                                  OS_TOBJ_ID_CL_UINT_PARAMETER, num_entries, OS_TOBJ_ID_POINTER_PARAMETER, devices, OS_TOBJ_ID_CL_P_UINT_PARAMETER, num_devices);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clGetDeviceIDs(platform, device_type, num_entries, devices, num_devices);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetDeviceIDs);

    return retVal;
}

cl_int CL_API_CALL clGetDeviceInfo(cl_device_id device, cl_device_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetDeviceInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall(OA_CL_NULL_HANDLE, ap_clGetDeviceInfo, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, device, OS_TOBJ_ID_CL_ENUM_PARAMETER, param_name,
                                                  OS_TOBJ_ID_BYTES_SIZE_PARAMETER, param_value_size, OS_TOBJ_ID_POINTER_PARAMETER, param_value, OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clGetDeviceInfo(device, param_name, param_value_size, param_value, param_value_size_ret);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }
    else if (CL_DEVICE_REFERENCE_COUNT == param_name) // && retVal == CL_SUCCESS
    {
        // Allow for a 0, NULL query without asserting:
        if (0 < param_value_size)
        {
            // Return the modified reference count for the device:
            GT_IF_WITH_ASSERT((sizeof(cl_uint) <= param_value_size) && (NULL != param_value))
            {
                *(cl_uint*)param_value = cs_stat_openCLMonitorInstance.deviceExternalReferenceCount(device);
            }
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetDeviceInfo);

    return retVal;
}

cl_int CL_API_CALL clCreateSubDevices(cl_device_id in_device, const cl_device_partition_property* properties, cl_uint num_devices, cl_device_id* out_devices, cl_uint* num_devices_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clCreateSubDevices);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall(OA_CL_NULL_HANDLE, ap_clCreateSubDevices, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, in_device, OS_TOBJ_ID_POINTER_PARAMETER, properties,
                                                  OS_TOBJ_ID_CL_UINT_PARAMETER, num_devices, OS_TOBJ_ID_POINTER_PARAMETER, out_devices, OS_TOBJ_ID_CL_P_UINT_PARAMETER, num_devices_ret);

    // Call the real function:
    cl_uint numDevicesOut = 0;
    cl_uint* num_devices_ret_for_real_call = num_devices_ret;

    // We need the outputted number of devices, if device ids are created:
    if ((NULL != out_devices) && (NULL == num_devices_ret_for_real_call))
    {
        num_devices_ret_for_real_call = &numDevicesOut;
    }

    retVal = cs_stat_realFunctionPointers.clCreateSubDevices(in_device, properties, num_devices, out_devices, num_devices_ret_for_real_call);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }
    else if (NULL != out_devices)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainDevice);
        csDevicesMonitor& theDevicesMontior = cs_stat_openCLMonitorInstance.devicesMonitor();

        for (cl_uint i = 0; i < *num_devices_ret_for_real_call; i++)
        {
            cl_device_id currentDevice = out_devices[i];

            if (NULL != currentDevice)
            {
                // Do not log devices we already know:
                bool isNewDevice = true;
                const apCLDevice* pDevice = theDevicesMontior.getDeviceObjectDetails((oaCLDeviceID)currentDevice);

                if (NULL != pDevice)
                {
                    isNewDevice = pDevice->wasMarkedForDeletion();
                }

                if (isNewDevice)
                {
                    // Add the device to the monitor:
                    theDevicesMontior.onDeviceCreated((oaCLDeviceID)currentDevice);

                    // Retain each created device, to prevent them from being deleted before we're done with them:
                    cl_int rcRetain = cs_stat_realFunctionPointers.clRetainDevice(out_devices[i]);
                    GT_ASSERT(CL_SUCCESS == rcRetain);
                }
            }
        }

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainDevice);
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateSubDevices);

    return retVal;
}

cl_int CL_API_CALL clRetainDevice(cl_device_id device)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clRetainDevice);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLDeviceID)device, ap_clRetainDevice, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, device);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clRetainDevice(device);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clRetainDevice);

    return retVal;
}

cl_int CL_API_CALL clReleaseDevice(cl_device_id device)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clReleaseDevice);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLDeviceID)device, ap_clReleaseDevice, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, device);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clReleaseDevice(device);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    // Notify the OpenCL monitor we released (and possibly deleted) a context:
    cs_stat_openCLMonitorInstance.checkIfDeviceWasDeleted(device);

    SU_END_FUNCTION_WRAPPER(ap_clReleaseDevice);

    return retVal;
}


// TO_DO: properties parameter
// Context APIs
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
cl_context CL_API_CALL clCreateContext(cl_context_properties* properties, cl_uint num_devices, const cl_device_id* devices,
                                       void (*pfn_notify)(const char*, const void*, size_t, void*), void* user_data, cl_int* errcode_ret)
#else
cl_context CL_API_CALL clCreateContext(const cl_context_properties* properties, cl_uint num_devices, const cl_device_id* devices,
                                       void (CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*), void* user_data, cl_int* errcode_ret)
#endif
{
    cl_context retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateContext);
    su_stat_interoperabilityHelper.onNestedFunctionEntered();

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall(OA_CL_NULL_HANDLE, ap_clCreateContext, 6, OS_TOBJ_ID_CL_CONTEXT_PROPERTIES_LIST_PARAMETER, properties, OS_TOBJ_ID_CL_UINT_PARAMETER, num_devices, OS_TOBJ_ID_POINTER_PARAMETER, devices,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, pfn_notify, OS_TOBJ_ID_POINTER_PARAMETER, user_data, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    if (cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER == cs_stat_pIKernelDebuggingManager->kernelDebuggerType()))
    {
        retVal = cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptCreateContext(properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
    }
    else // !cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled()
    {
        retVal = cs_stat_realFunctionPointers.clCreateContext(properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
    }

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the context was created successfully:
    if (retVal != NULL)
    {
        // Retain the context, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainContext);
        cs_stat_realFunctionPointers.clRetainContext(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainContext);

        // Log the context creation:
        cs_stat_openCLMonitorInstance.onContextCreation(retVal, ap_clCreateContext);
    }

    su_stat_interoperabilityHelper.onNestedFunctionExited();
    SU_END_FUNCTION_WRAPPER(ap_clCreateContext);

    return retVal;
}

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
cl_context CL_API_CALL clCreateContextFromType(cl_context_properties* properties, cl_device_type device_type, void (*pfn_notify)(const char*, const void*, size_t, void*),
                                               void* user_data, cl_int* errcode_ret)
#else
cl_context CL_API_CALL clCreateContextFromType(const cl_context_properties* properties, cl_device_type device_type, void (CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*),
                                               void* user_data, cl_int* errcode_ret)
#endif
{
    cl_context retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateContextFromType);
    su_stat_interoperabilityHelper.onNestedFunctionEntered();

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall(OA_CL_NULL_HANDLE, ap_clCreateContextFromType, 5, OS_TOBJ_ID_CL_CONTEXT_PROPERTIES_LIST_PARAMETER, properties, OS_TOBJ_ID_CL_DEVICE_TYPE_PARAMETER, device_type,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, pfn_notify, OS_TOBJ_ID_POINTER_PARAMETER, user_data, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // This function calls clCreateContext and clGetDeviceIDs on Mac:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clCreateContext);
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetDeviceIDs);

    // Call the real function:
    if (cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER == cs_stat_pIKernelDebuggingManager->kernelDebuggerType()))
    {
        retVal = cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptCreateContextFromType(properties, device_type, pfn_notify, user_data, errcode_ret);
    }
    else // !cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled()
    {
        retVal = cs_stat_realFunctionPointers.clCreateContextFromType(properties, device_type, pfn_notify, user_data, errcode_ret);
    }

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clCreateContext);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetDeviceIDs);

    // If the context was created successfully:
    if (retVal != NULL)
    {
        // Retain the context, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainContext);
        cs_stat_realFunctionPointers.clRetainContext(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainContext);

        // Log the context creation:
        cs_stat_openCLMonitorInstance.onContextCreation(retVal, ap_clCreateContextFromType);
    }

    su_stat_interoperabilityHelper.onNestedFunctionExited();
    SU_END_FUNCTION_WRAPPER(ap_clCreateContextFromType);

    return retVal;
}

cl_int CL_API_CALL clRetainContext(cl_context context)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clRetainContext);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clRetainContext, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clRetainContext(context);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clRetainContext);

    return retVal;
}

cl_int CL_API_CALL clReleaseContext(cl_context context)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clReleaseContext);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clReleaseContext, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clReleaseContext(context);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    // Notify the OpenCL monitor we released (and possibly deleted) a context:
    cs_stat_openCLMonitorInstance.checkIfContextWasDeleted(context);

    SU_END_FUNCTION_WRAPPER(ap_clReleaseContext);

    return retVal;
}

cl_int CL_API_CALL clGetContextInfo(cl_context context, cl_context_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetContextInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clGetContextInfo, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_ENUM_PARAMETER, param_name,
                                                  OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size, OS_TOBJ_ID_POINTER_PARAMETER, param_value, OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clGetContextInfo(context, param_name, param_value_size, param_value, param_value_size_ret);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }
    else if (CL_CONTEXT_REFERENCE_COUNT == param_name) // && retVal == CL_SUCCESS
    {
        // Allow for a 0, NULL query without asserting:
        if (0 < param_value_size)
        {
            // Return the modified reference count for the context:
            GT_IF_WITH_ASSERT((sizeof(cl_uint) <= param_value_size) && (NULL != param_value))
            {
                *(cl_uint*)param_value = cs_stat_openCLMonitorInstance.contextExternalReferenceCount(context);
            }
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetContextInfo);

    return retVal;
}

// Command Queue APIs
cl_command_queue CL_API_CALL clCreateCommandQueue(cl_context context, cl_device_id device, cl_command_queue_properties properties, cl_int* errcode_ret)
{
    cl_command_queue retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateCommandQueue);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clCreateCommandQueue, 4, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_HANDLE_PARAMETER, device,
                                                  OS_TOBJ_ID_CL_COMMAND_QUEUE_PROPERTIES_PARAMETER, properties, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // The Mac implementation of clCreateCommandQueue calls clSetCommandQueueProperty internally:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetCommandQueueProperty);
    int deviceIndex = cs_stat_openCLMonitorInstance.devicesMonitor().getDeviceObjectAPIID((oaCLDeviceID)device);
    cl_command_queue_properties forcedProperties = properties;

    if (cs_stat_openCLMonitorInstance.isCommandQueueProfileModeForcedForDevice(deviceIndex))
    {
        // If the queue has the CL_QUEUE_PROFILING_ENABLE flag disabled, raise it:
        forcedProperties |= CL_QUEUE_PROFILING_ENABLE;
    }

    // Call the real function:
    if (cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER == cs_stat_pIKernelDebuggingManager->kernelDebuggerType()))
    {
        retVal = cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptCreateCommandQueue(context, device, forcedProperties, errcode_ret);
    }
    else // !cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled()
    {
        retVal = cs_stat_realFunctionPointers.clCreateCommandQueue(context, device, forcedProperties, errcode_ret);
    }

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetCommandQueueProperty);

    // If the queue was created successfully:
    if (retVal != NULL)
    {
        // Retain the queue, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainCommandQueue);
        cs_stat_realFunctionPointers.clRetainCommandQueue(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainCommandQueue);

        // Log the queue creation:
        cs_stat_openCLMonitorInstance.onCommandQueueCreation(retVal, context, device, properties);
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateCommandQueue);

    return retVal;
}

cl_int CL_API_CALL clRetainCommandQueue(cl_command_queue command_queue)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clRetainCommandQueue);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clRetainCommandQueue, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clRetainCommandQueue(command_queue);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clRetainCommandQueue);

    return retVal;
}

cl_int CL_API_CALL clReleaseCommandQueue(cl_command_queue command_queue)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clReleaseCommandQueue);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clReleaseCommandQueue, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clReleaseCommandQueue(command_queue);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    // Notify the OpenCL monitor we released (and possibly deleted) a command queue:
    cs_stat_openCLMonitorInstance.checkIfCommandQueueWasDeleted(command_queue, true);

    SU_END_FUNCTION_WRAPPER(ap_clReleaseCommandQueue);

    return retVal;
}

cl_int CL_API_CALL clGetCommandQueueInfo(cl_command_queue command_queue, cl_command_queue_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetCommandQueueInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clGetCommandQueueInfo, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_ENUM_PARAMETER, param_name,
                                                  OS_TOBJ_ID_SIZE_T_PARAMETER, param_value_size, OS_TOBJ_ID_POINTER_PARAMETER, param_value, OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clGetCommandQueueInfo(command_queue, param_name, param_value_size, param_value, param_value_size_ret);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }
    else if (CL_QUEUE_REFERENCE_COUNT == param_name) // && retVal == CL_SUCCESS
    {
        // Allow for a 0, NULL query without asserting:
        if (0 < param_value_size)
        {
            // Return the modified reference count for the queue:
            GT_IF_WITH_ASSERT((sizeof(cl_uint) <= param_value_size) && (NULL != param_value))
            {
                *(cl_uint*)param_value = cs_stat_openCLMonitorInstance.commandQueueExternalReferenceCount(command_queue);
            }
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetCommandQueueInfo);

    return retVal;
}

cl_int CL_API_CALL clSetCommandQueueProperty(cl_command_queue command_queue, cl_command_queue_properties properties, cl_bool enable, cl_command_queue_properties* old_properties)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clSetCommandQueueProperty);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clSetCommandQueueProperty, 4, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_COMMAND_QUEUE_PROPERTIES_PARAMETER, properties,
                                                  OS_TOBJ_ID_CL_BOOL_PARAMETER, enable, OS_TOBJ_ID_POINTER_PARAMETER, old_properties);

    // Check if the CL_QUEUE_PROFILING_ENABLE flag is forced for this queue:
    if (cs_stat_openCLMonitorInstance.isCommandQueueProfileModeForcedForQueue((oaCLCommandQueueHandle)command_queue))
    {
        // We need to calculate old_properties properly if we need to return them:
        bool wasProfilingModeEnabled = false;
        cl_command_queue_properties oldUnforcedProperties = 0;

        if (old_properties != NULL)
        {
            // Get the command queue monitor:
            csCommandQueueMonitor* pCommandQueueMtr = csOpenCLMonitor::instance().commandQueueMonitor((oaCLCommandQueueHandle)command_queue);
            GT_IF_WITH_ASSERT(pCommandQueueMtr != NULL)
            {
                // Get the unforced profiling mode flag:
                const apCLCommandQueue& queueInfo = pCommandQueueMtr->commandQueueInfo();
                wasProfilingModeEnabled = queueInfo.profilingModeEnable();

                // Get the real properties.
                // Note we do not need to clear the profiling mode here as it will be set back anyways according to wasProfilingModeEnabled
                // before returning it to the user:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetCommandQueueInfo);
                (void) cs_stat_realFunctionPointers.clGetCommandQueueInfo(command_queue, CL_QUEUE_PROPERTIES, sizeof(cl_command_queue_properties), &oldUnforcedProperties, NULL);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetCommandQueueInfo);
            }
        }

        // If we are trying to force only the CL_QUEUE_PROFILING_ENABLE mode, ignore this call.
        // Otherwise, don't change this mode, but change any others that are set:
        if (properties == CL_QUEUE_PROFILING_ENABLE)
        {
            retVal = CL_SUCCESS;

            if (old_properties != NULL)
            {
                // Return the value we got before:
                *old_properties = oldUnforcedProperties;
            }
        }
        else // properties != CL_QUEUE_PROFILING_ENABLE
        {
            // Call the real function:
            retVal = cs_stat_realFunctionPointers.clSetCommandQueueProperty(command_queue, properties, enable, old_properties);

            if (retVal != CL_SUCCESS)
            {
                // Handle OpenCL error:
                cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
            }
        }

        // If the user requested the old properties back:
        if ((old_properties != NULL) && (!wasProfilingModeEnabled))
        {
            // Remove the actual profiling flag (which will always be true, since we are forcing it on):
            *old_properties &= (~CL_QUEUE_PROFILING_ENABLE);
        }
    }
    else // !isCommandQueuesProfileModeForced()
    {
        // Call the real function:
        retVal = cs_stat_realFunctionPointers.clSetCommandQueueProperty(command_queue, properties, enable, old_properties);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }
    }

    // If the function succeeded, mark the change in our monitor:
    if (retVal == CL_SUCCESS)
    {
        cs_stat_openCLMonitorInstance.onCommandQueuePropertiesSet(command_queue, properties, enable);
    }

    SU_END_FUNCTION_WRAPPER(ap_clSetCommandQueueProperty);

    return retVal;
}

// TO_DO: size parameter
cl_mem CL_API_CALL clCreateBuffer(cl_context context, cl_mem_flags flags, size_t size, void* host_ptr, cl_int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateBuffer);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clCreateBuffer, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags,
                                                  OS_TOBJ_ID_BYTES_SIZE_PARAMETER, size, OS_TOBJ_ID_POINTER_PARAMETER, host_ptr, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clCreateBuffer(context, flags, size, host_ptr, errcode_ret);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the buffer was created successfully:
    if (retVal != NULL)
    {
        // Retain the buffer, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);
        cs_stat_realFunctionPointers.clRetainMemObject(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);

        // Get the context monitor for the context that the buffer is created at:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor((oaCLContextHandle)context);

        if (pContextMonitor != NULL)
        {
            // Get the buffers monitor for this context:
            csImagesAndBuffersMonitor& texBuffersMonitor = pContextMonitor->imagesAndBuffersMonitor();
            texBuffersMonitor.onBufferCreation(retVal, flags, size);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateBuffer);

    return retVal;
}

cl_mem CL_API_CALL clCreateSubBuffer(cl_mem buffer, cl_mem_flags flags, cl_buffer_create_type buffer_create_type, const void* buffer_create_info, cl_int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateSubBuffer);

    // Log the call to this function:
    if (buffer_create_type == CL_BUFFER_CREATE_TYPE_REGION)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)buffer, ap_clCreateSubBuffer, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, buffer, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, buffer_create_type, OS_TOBJ_ID_CL_BUFFER_REGION_PARAMETER, buffer_create_info, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);
    }
    else
    {
        gtString message;
        message.appendFormattedString(L"Unsupported sub buffer create type: %d", buffer_create_type);
        GT_ASSERT_EX(false, message.asCharArray());
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)buffer, ap_clCreateSubBuffer, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, buffer, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, buffer_create_type, OS_TOBJ_ID_POINTER_PARAMETER, buffer_create_info, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);
    }

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clCreateSubBuffer(buffer, flags, buffer_create_type, buffer_create_info, errcode_ret);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // TO_DO: CL1.1 monitor sub buffers, and add parameters for cl_buffer_create_type and buffer_create_info:
    // If the sub buffer was created successfully:
    if (retVal != NULL)
    {
        // Retain the buffer, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);
        cs_stat_realFunctionPointers.clRetainMemObject(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);

        // Get the context monitor for the context that the buffer is created at:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.contextContainingMemObject((oaCLMemHandle)buffer);

        if (pContextMonitor != NULL)
        {
            // Get the buffers monitor for this context:
            csImagesAndBuffersMonitor& texBuffersMonitor = pContextMonitor->imagesAndBuffersMonitor();
            texBuffersMonitor.onSubBufferCreation(retVal, buffer, flags, buffer_create_type, (cl_buffer_region*)buffer_create_info);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateSubBuffer);

    return retVal;
}

cl_mem CL_API_CALL clCreateImage(cl_context context, cl_mem_flags flags, const cl_image_format* image_format, const cl_image_desc* image_desc, void* host_ptr, cl_int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateImage);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clCreateImage, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, image_format, OS_TOBJ_ID_CL_IMAGE_DESCRIPTION_PARAMETER, image_desc, OS_TOBJ_ID_POINTER_PARAMETER, host_ptr, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clCreateImage(context, flags, image_format, image_desc, host_ptr, errcode_ret);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the buffer was created successfully:
    if (retVal != NULL)
    {
        // Retain the image, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);
        cs_stat_realFunctionPointers.clRetainMemObject(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);

        // Get the context monitor for the context that the texture is created at:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor((oaCLContextHandle)context);

        if (pContextMonitor != NULL)
        {
            // Get the texture monitor for this context:
            csImagesAndBuffersMonitor& texturesMonitor = pContextMonitor->imagesAndBuffersMonitor();
            texturesMonitor.onImageCreation(retVal, image_format, flags, image_desc);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateImage);

    return retVal;
}

cl_mem CL_API_CALL clCreateImage2D(cl_context context, cl_mem_flags flags, const cl_image_format* image_format, size_t image_width, size_t image_height, size_t image_row_pitch, void* host_ptr, cl_int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateImage2D);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clCreateImage2D, 8, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, image_format, OS_TOBJ_ID_SIZE_T_PARAMETER, image_width, OS_TOBJ_ID_SIZE_T_PARAMETER, image_height,
                                                  OS_TOBJ_ID_BYTES_SIZE_PARAMETER, image_row_pitch, OS_TOBJ_ID_POINTER_PARAMETER, host_ptr, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clCreateImage2D(context, flags, image_format, image_width, image_height, image_row_pitch, host_ptr, errcode_ret);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the buffer was created successfully:
    if (retVal != NULL)
    {
        // Retain the image, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);
        cs_stat_realFunctionPointers.clRetainMemObject(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);

        // Get the context monitor for the context that the texture is created at:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor((oaCLContextHandle)context);

        if (pContextMonitor != NULL)
        {
            // Get the texture monitor for this context:
            csImagesAndBuffersMonitor& texturesMonitor = pContextMonitor->imagesAndBuffersMonitor();
            texturesMonitor.onImageCreation(retVal, image_format, flags, AP_2D_TEXTURE, image_width, image_height, 1);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateImage2D);

    return retVal;
}

cl_mem CL_API_CALL clCreateImage3D(cl_context context, cl_mem_flags flags, const cl_image_format* image_format, size_t image_width, size_t image_height, size_t image_depth, size_t image_row_pitch, size_t image_slice_pitch, void* host_ptr, cl_int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateImage3D);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clCreateImage3D, 10, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, image_format, OS_TOBJ_ID_SIZE_T_PARAMETER, image_width, OS_TOBJ_ID_SIZE_T_PARAMETER, image_height,
                                                  OS_TOBJ_ID_SIZE_T_PARAMETER, image_depth, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, image_row_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, image_slice_pitch,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, host_ptr, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clCreateImage3D(context, flags, image_format, image_width, image_height, image_depth, image_row_pitch, image_slice_pitch, host_ptr, errcode_ret);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the buffer was created successfully:
    if (retVal != NULL)
    {
        // Retain the image, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);
        cs_stat_realFunctionPointers.clRetainMemObject(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);

        // Get the context monitor for the context that the texture is created at:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor((oaCLContextHandle)context);

        if (pContextMonitor != NULL)
        {
            // Get the texture monitor for this context:
            csImagesAndBuffersMonitor& texturesMonitor = pContextMonitor->imagesAndBuffersMonitor();
            texturesMonitor.onImageCreation(retVal, image_format, flags, AP_3D_TEXTURE, image_width, image_height, image_depth);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateImage3D);

    return retVal;
}

cl_int CL_API_CALL clRetainMemObject(cl_mem memobj)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clRetainMemObject);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)memobj, ap_clRetainMemObject, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, memobj);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clRetainMemObject(memobj);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clRetainMemObject);

    return retVal;
}

cl_int CL_API_CALL clReleaseMemObject(cl_mem memobj)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clReleaseMemObject);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)memobj, ap_clReleaseMemObject, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, memobj);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clReleaseMemObject(memobj);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    // Notify the OpenCL monitor we released (and possibly deleted) a mem object:
    cs_stat_openCLMonitorInstance.checkIfMemObjectWasDeleted(memobj, true);

    SU_END_FUNCTION_WRAPPER(ap_clReleaseMemObject);

    return retVal;
}

cl_int CL_API_CALL clGetSupportedImageFormats(cl_context context, cl_mem_flags flags, cl_mem_object_type image_type,
                                              cl_uint num_entries, cl_image_format* image_formats, cl_uint* num_image_formats)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetSupportedImageFormats);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clGetSupportedImageFormats, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags, OS_TOBJ_ID_CL_ENUM_PARAMETER, image_type,
                                                  OS_TOBJ_ID_CL_P_UINT_PARAMETER, num_entries, OS_TOBJ_ID_POINTER_PARAMETER, image_formats, OS_TOBJ_ID_CL_P_UINT_PARAMETER, num_image_formats);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clGetSupportedImageFormats(context, flags, image_type, num_entries, image_formats, num_image_formats);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetSupportedImageFormats);

    return retVal;
}

cl_int CL_API_CALL clGetMemObjectInfo(cl_mem memobj, cl_mem_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetMemObjectInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)memobj, ap_clGetMemObjectInfo, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, memobj, OS_TOBJ_ID_CL_ENUM_PARAMETER, param_name, OS_TOBJ_ID_SIZE_T_PARAMETER, param_value_size,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, param_value, OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clGetMemObjectInfo(memobj, param_name, param_value_size, param_value, param_value_size_ret);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }
    else if (CL_MEM_REFERENCE_COUNT == param_name) // && retVal == CL_SUCCESS
    {
        // Allow for a 0, NULL query without asserting:
        if (0 < param_value_size)
        {
            // Return the modified reference count for the mem object:
            GT_IF_WITH_ASSERT((sizeof(cl_uint) <= param_value_size) && (NULL != param_value))
            {
                *(cl_uint*)param_value = cs_stat_openCLMonitorInstance.memObjectExternalReferenceCount(memobj);
            }
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetMemObjectInfo);

    return retVal;
}

cl_int CL_API_CALL clGetImageInfo(cl_mem image, cl_image_info param_name, size_t param_value_size,
                                  void* param_value, size_t* param_value_size_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetImageInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)image, ap_clGetImageInfo, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, image, OS_TOBJ_ID_CL_ENUM_PARAMETER, param_name, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, param_value_size,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, param_value, OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clGetImageInfo(image, param_name, param_value_size, param_value, param_value_size_ret);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetImageInfo);

    return retVal;
}


cl_int CL_API_CALL clSetMemObjectDestructorCallback(cl_mem memobj, void (CL_CALLBACK* pfn_notify)(cl_mem memobj, void* user_data), void* user_data)
{
    cl_uint retVal = 0;

    SU_START_FUNCTION_WRAPPER(ap_clSetMemObjectDestructorCallback);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)memobj, ap_clSetMemObjectDestructorCallback, 3, OS_TOBJ_ID_CL_HANDLE_PARAMETER, memobj, OS_TOBJ_ID_POINTER_PARAMETER, pfn_notify, OS_TOBJ_ID_POINTER_PARAMETER, user_data);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback(memobj, pfn_notify, user_data);

    if (CL_SUCCESS == retVal)
    {
        oaCLMemHandle hMem = (oaCLMemHandle)memobj;
        csContextMonitor* pContext = cs_stat_openCLMonitorInstance.contextContainingMemObject(hMem);
        GT_IF_WITH_ASSERT(NULL != pContext)
        {
            pContext->imagesAndBuffersMonitor().onMemObjectDestructorCallbackSet(hMem, (osProcedureAddress64)pfn_notify, (osProcedureAddress64)user_data);
        }
    }
    else // CL_SUCCESS != retVal
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clSetMemObjectDestructorCallback);

    return retVal;
}

cl_sampler CL_API_CALL clCreateSampler(cl_context context, cl_bool normalized_coords, cl_addressing_mode addressing_mode,
                                       cl_filter_mode filter_mode, cl_int* errcode_ret)
{
    cl_sampler retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateSampler);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clCreateSampler, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_BOOL_PARAMETER, normalized_coords, OS_TOBJ_ID_CL_ENUM_PARAMETER, addressing_mode,
                                                  OS_TOBJ_ID_CL_ENUM_PARAMETER, filter_mode, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clCreateSampler(context, normalized_coords, addressing_mode, filter_mode, errcode_ret);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    if (retVal != NULL)
    {
        // Retain the sampler, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainSampler);
        cs_stat_realFunctionPointers.clRetainSampler(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainSampler);

        cs_stat_openCLMonitorInstance.onSamplerCreation(context, retVal, normalized_coords, addressing_mode, filter_mode);
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateSampler);

    return retVal;
}

cl_int CL_API_CALL clRetainSampler(cl_sampler sampler)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clRetainSampler);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)sampler, ap_clRetainSampler, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, sampler);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clRetainSampler(sampler);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clRetainSampler);

    return retVal;
}

cl_int CL_API_CALL clReleaseSampler(cl_sampler sampler)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clReleaseSampler);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)sampler, ap_clReleaseSampler, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, sampler);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clReleaseSampler(sampler);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    // Notify the OpenCL monitor we released (and possibly deleted) a sampler:
    cs_stat_openCLMonitorInstance.checkIfSamplerWasDeleted(sampler, true);

    SU_END_FUNCTION_WRAPPER(ap_clReleaseSampler);

    return retVal;
}

cl_int CL_API_CALL clGetSamplerInfo(cl_sampler sampler, cl_sampler_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetSamplerInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)sampler, ap_clGetSamplerInfo, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, sampler, OS_TOBJ_ID_CL_ENUM_PARAMETER, param_name, OS_TOBJ_ID_SIZE_T_PARAMETER, param_value_size,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, param_value, OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clGetSamplerInfo(sampler, param_name, param_value_size, param_value, param_value_size_ret);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }
    else if (CL_SAMPLER_REFERENCE_COUNT == param_name) // && retVal == CL_SUCCESS
    {
        // Allow for a 0, NULL query without asserting:
        if (0 < param_value_size)
        {
            // Return the modified reference count for the sampler:
            GT_IF_WITH_ASSERT((sizeof(cl_uint) <= param_value_size) && (NULL != param_value))
            {
                *(cl_uint*)param_value = cs_stat_openCLMonitorInstance.samplerExternalReferenceCount(sampler);
            }
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetSamplerInfo);

    return retVal;
}

cl_program CL_API_CALL clCreateProgramWithSource(cl_context context, cl_uint count, const char** strings, const size_t* lengths, cl_int* errcode_ret)
{
    cl_program retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateProgramWithSource);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clCreateProgramWithSource, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_UINT_PARAMETER, count, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_P_CHAR_PARAMETER, (int)count, strings, /*OS_TOBJ_ID_CL_MULTI_STRING_PARAMETER, count, strings, lengths,*/
                                                  OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, (int)count, lengths, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    bool callRealFunction = suGetGlobalServerEnvironmentSettings().m_suDontFixCRInSourceStrings;

    if (!callRealFunction)
    {
        gtASCIIString modifiedSource;
        unsigned int* sourceLengths = new unsigned int[(count > 0) ? (unsigned int)count : 1];

        for (cl_uint i = 0; i < count; ++i)
        {
            sourceLengths[i] = (nullptr != lengths) ? (unsigned int)lengths[i] : 0;
        }

        bool callWithModified = suHandleCRInSources((unsigned int)count, strings, sourceLengths, modifiedSource);
        callRealFunction = !callWithModified;

        if (callWithModified)
        {
            // Call the real function with the modified values:
            const char* modifiedSourceAsCharArray = modifiedSource.asCharArray();
            size_t modifiedSourceLength = (size_t)modifiedSource.length();
            retVal = cs_stat_realFunctionPointers.clCreateProgramWithSource(context, 1, &modifiedSourceAsCharArray, &modifiedSourceLength, errcode_ret);

            // If this failed, try the real version:
            callRealFunction = ((cl_program)nullptr == retVal);
        }

        delete[] sourceLengths;
    }

    if (callRealFunction)
    {
        // Call the real function:
        retVal = cs_stat_realFunctionPointers.clCreateProgramWithSource(context, count, strings, lengths, errcode_ret);
    }

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the program was created successfully:
    if (retVal != NULL)
    {
        // Retain the program, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainProgram);
        cs_stat_realFunctionPointers.clRetainProgram(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainProgram);

        // Log the program creation:
        cs_stat_openCLMonitorInstance.onProgramCreation(context, retVal, count, strings, lengths);
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateProgramWithSource);

    return retVal;
}

// TO_DO: size parameters
cl_program CL_API_CALL clCreateProgramWithBinary(cl_context context, cl_uint num_devices, const cl_device_id* device_list, const size_t* lengths, const unsigned char** binaries,
                                                 cl_int* binary_status, cl_int* errcode_ret)
{
    cl_program retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateProgramWithBinary);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clCreateProgramWithBinary, 7, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_UINT_PARAMETER, num_devices, OS_TOBJ_ID_POINTER_PARAMETER, device_list, OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, lengths,
                                                  OS_TOBJ_ID_PP_CHAR_PARAMETER, binaries, OS_TOBJ_ID_P_INT_PARAMETER, binary_status, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clCreateProgramWithBinary(context, num_devices, device_list, lengths, binaries, binary_status, errcode_ret);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the program was created successfully:
    if (retVal != NULL)
    {
        // Retain the program, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainProgram);
        cs_stat_realFunctionPointers.clRetainProgram(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainProgram);

        // Add a message instead of creating an empty source file:
        gtASCIIString noSourceMessage = "// Source Code not available for programs created with clCreateProgramWithBinary";
        const char* noSourceMessageAsCharArray = noSourceMessage.asCharArray();

        // TO_DO: log the binaries somewhere?
        // Log the program creation:
        cs_stat_openCLMonitorInstance.onProgramCreation(context, retVal, 1, &noSourceMessageAsCharArray, NULL);
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateProgramWithBinary);

    return retVal;
}

cl_program CL_API_CALL clCreateProgramWithBuiltInKernels(cl_context context, cl_uint num_devices, const cl_device_id* device_list, const char* kernel_names, cl_int* errcode_ret)
{
    cl_program retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateProgramWithBuiltInKernels);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clCreateProgramWithBuiltInKernels, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_UINT_PARAMETER, num_devices,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, device_list, OS_TOBJ_ID_STRING_PARAMETER, kernel_names, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clCreateProgramWithBuiltInKernels(context, num_devices, device_list, kernel_names, errcode_ret);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the program was created successfully:
    if (retVal != NULL)
    {
        // Retain the program, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainProgram);
        cs_stat_realFunctionPointers.clRetainProgram(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainProgram);

        // Add a message instead of creating an empty source file:
        gtASCIIString noSourceMessage = "// Source Code not available for programs created with clCreateProgramWithBuiltInKernels";
        const char* noSourceMessageAsCharArray = noSourceMessage.asCharArray();

        // TO_DO: log the binaries somewhere?
        // Log the program creation:
#pragma message ("OpenCL1.2: Handle program creation")
        cs_stat_openCLMonitorInstance.onProgramCreation(context, retVal, 1, &noSourceMessageAsCharArray, NULL);
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateProgramWithBuiltInKernels);

    return retVal;
}


cl_int CL_API_CALL clRetainProgram(cl_program program)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clRetainProgram);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)program, ap_clRetainProgram, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, program);

    // Call the real function:
    cl_program programInternalHandle = cs_stat_openCLMonitorInstance.internalProgramHandleFromExternalHandle(program);
    retVal = cs_stat_realFunctionPointers.clRetainProgram(programInternalHandle);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clRetainProgram);

    return retVal;
}
cl_int CL_API_CALL clReleaseProgram(cl_program program)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clReleaseProgram);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)program, ap_clReleaseProgram, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, program);

    // Call the real function:
    cl_program programInternalHandle = cs_stat_openCLMonitorInstance.internalProgramHandleFromExternalHandle(program);
    retVal = cs_stat_realFunctionPointers.clReleaseProgram(programInternalHandle);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    // Notify the OpenCL monitor we released (and possibly deleted) a program:
    cs_stat_openCLMonitorInstance.checkIfProgramWasDeleted(program, true);

    SU_END_FUNCTION_WRAPPER(ap_clReleaseProgram);

    return retVal;
}

cl_int CL_API_CALL clBuildProgram(cl_program program, cl_uint num_devices, const cl_device_id* device_list, const char* options,
                                  void (CL_CALLBACK* pfn_notify)(cl_program program, void* user_data), void* user_data)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clBuildProgram);

    // Notify the CL Monitor that the program is about to be built:
    // This should be performed before addFunctionCall since we want the event to be thrown before that breakpoint is triggered:
    cs_stat_openCLMonitorInstance.onBeforeBuildProgram(program, num_devices, device_list, options);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)program, ap_clBuildProgram, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, program, OS_TOBJ_ID_CL_UINT_PARAMETER, num_devices, OS_TOBJ_ID_POINTER_PARAMETER, device_list, OS_TOBJ_ID_STRING_PARAMETER, options,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, pfn_notify, OS_TOBJ_ID_POINTER_PARAMETER, user_data);

    // Append the original options to the options string if there are:
    gtASCIIString optionsAsGTString;

    if (options != NULL)
    {
        optionsAsGTString.append(options);
    }

    // Uri, 14/8/13 - Due to a bug in the OpenCL runtime (present at least in drivers 13.1, 13.4 and 13.6), clBuildProgram thinks the program
    // has kernels if it has a reference count > 1. Since we add 1 to the reference count of each program, we need to lower this each time we build
    // and restore it afterwards:
    cs_stat_openCLMonitorInstance.releaseProgramForBuild(program);

    // If we want to use kernel debugging, we need to add the "-g" and "-cl-opt-disable" flags to the compiler:
    bool isKernelDebuggingOn = cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled();
    bool isProgramOnAMDDevice = cs_stat_openCLMonitorInstance.isProgramOnAMDPlatform((oaCLProgramHandle)program);
    gtString programDebuggingBlockedReason;
    bool areProgramBuildFlagsSupported = cs_stat_pIKernelDebuggingManager->programBuildFlagsSupported(options, programDebuggingBlockedReason);
    bool modifiedFlags = false;

    if (isKernelDebuggingOn && isProgramOnAMDDevice && areProgramBuildFlagsSupported && (!suGetGlobalServerEnvironmentSettings().m_csDontAddDebuggingBuildFlags))
    {
        modifiedFlags = true;
        optionsAsGTString.append(' ');
        optionsAsGTString.append(SU_STR_kernelDebuggingForcedBuildOptionsASCII);
        int amdRuntimeVer = cs_stat_openCLMonitorInstance.devicesMonitor().amdRuntimeVersion();

        if (cs_stat_pIKernelDebuggingManager->shouldAppendLegacyBuildFlags(optionsAsGTString, amdRuntimeVer))
        {
            optionsAsGTString.append(SU_STR_kernelDebuggingForcedBuildOptionLegacyASCII);
        }
    }

    // Call the real function:
    cl_program programInternalHandle = cs_stat_openCLMonitorInstance.internalProgramHandleFromExternalHandle(program);
    const char* modifiedOptions = modifiedFlags ? (optionsAsGTString.isEmpty() ? nullptr : optionsAsGTString.asCharArray()) : options;
    retVal = cs_stat_realFunctionPointers.clBuildProgram(programInternalHandle, num_devices, device_list, modifiedOptions, pfn_notify, user_data);

    // If we failed to build:
    if (modifiedFlags && (retVal != CL_SUCCESS))
    {
        // Try building without the flags:
        cl_int originalRetVal = retVal;
        retVal = cs_stat_realFunctionPointers.clBuildProgram(programInternalHandle, num_devices, device_list, options, pfn_notify, user_data);

        // If this succeeded:
        if (retVal == CL_SUCCESS)
        {
            // Report that the build failed due to the debug flags:
            cs_stat_openCLMonitorInstance.onBuildProgramFailedWithDebugFlags(program, originalRetVal);
        }
    }

    // Uri, 14/8/13 - see comment above:
    cs_stat_openCLMonitorInstance.restoreProgramFromBuild(program);

    // Notify the CL Monitor to, so an event will be created:
    cs_stat_openCLMonitorInstance.onBuildProgram(program, num_devices, device_list, options, areProgramBuildFlagsSupported, programDebuggingBlockedReason, true);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clBuildProgram);

    return retVal;
}


cl_int CL_API_CALL clCompileProgram(cl_program program, cl_uint num_devices, const cl_device_id* device_list, const char* options, cl_uint num_input_headers,
                                    const cl_program* input_headers, const char** header_include_names,
                                    void (CL_CALLBACK* pfn_notify)(cl_program program, void* user_data), void* user_data)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clCompileProgram);

    // Notify the CL Monitor that the program is about to be built:
    // This should be performed before addFunctionCall since we want the event to be thrown before that breakpoint is triggered:
#pragma message ("Adapt the before / after failed callbacks to compile link options")
    cs_stat_openCLMonitorInstance.onBeforeBuildProgram(program, num_devices, device_list, options);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)program, ap_clCompileProgram, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, program, OS_TOBJ_ID_CL_UINT_PARAMETER, num_devices,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, device_list, OS_TOBJ_ID_STRING_PARAMETER, options, OS_TOBJ_ID_CL_UINT_PARAMETER, num_input_headers,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, input_headers, OS_TOBJ_ID_PP_CHAR_PARAMETER, header_include_names,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, pfn_notify, OS_TOBJ_ID_POINTER_PARAMETER, user_data);

    // Append the original options to the options string if there are:
    gtASCIIString optionsAsGTString;

    if (options != NULL)
    {
        optionsAsGTString.append(options);
    }

    // If we want to use kernel debugging, we need to add the "-g" and "-cl-opt-disable" flags to the compiler:
    bool isKernelDebuggingOn = cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled();
    bool isProgramOnAMDDevice = cs_stat_openCLMonitorInstance.isProgramOnAMDPlatform((oaCLProgramHandle)program);
    gtString programDebuggingBlockedReason;
    bool areProgramBuildFlagsSupported = cs_stat_pIKernelDebuggingManager->programBuildFlagsSupported(options, programDebuggingBlockedReason);
    bool modifiedFlags = false;

    if (isKernelDebuggingOn && isProgramOnAMDDevice && areProgramBuildFlagsSupported && (!suGetGlobalServerEnvironmentSettings().m_csDontAddDebuggingBuildFlags))
    {
        modifiedFlags = true;
        optionsAsGTString.append(' ');
        optionsAsGTString.append(SU_STR_kernelDebuggingForcedBuildOptionsASCII);
        int amdRuntimeVer = cs_stat_openCLMonitorInstance.devicesMonitor().amdRuntimeVersion();

        if (cs_stat_pIKernelDebuggingManager->shouldAppendLegacyBuildFlags(optionsAsGTString, amdRuntimeVer))
        {
            optionsAsGTString.append(SU_STR_kernelDebuggingForcedBuildOptionLegacyASCII);
        }
    }

    // Uri, 14/8/13 - Due to a bug in the OpenCL runtime (present at least in drivers 13.1, 13.4 and 13.6), clBuildProgram thinks the program
    // has kernels if it has a reference count > 1. Since we add 1 to the reference count of each program, we need to lower this each time we build
    // and restore it afterwards:
    cs_stat_openCLMonitorInstance.releaseProgramForBuild(program);

    // Call the real function:
    cl_program programInternalHandle = cs_stat_openCLMonitorInstance.internalProgramHandleFromExternalHandle(program);
    const char* modifiedOptions = modifiedFlags ? (optionsAsGTString.isEmpty() ? nullptr : optionsAsGTString.asCharArray()) : options;
    retVal = cs_stat_realFunctionPointers.clCompileProgram(programInternalHandle, num_devices, device_list, modifiedOptions, num_input_headers, input_headers, header_include_names, pfn_notify, user_data);

    // If we failed to build:
    if (modifiedFlags && (retVal != CL_SUCCESS))
    {
        // Try building without the flags:
        cl_int originalRetVal = retVal;
        retVal = cs_stat_realFunctionPointers.clCompileProgram(programInternalHandle, num_devices, device_list, options, num_input_headers, input_headers, header_include_names, pfn_notify, user_data);

        // If this succeeded:
        if (retVal == CL_SUCCESS)
        {
            // Report that the build failed due to the debug flags:
            cs_stat_openCLMonitorInstance.onBuildProgramFailedWithDebugFlags(program, originalRetVal);
        }
    }

    // Uri, 14/8/13 - see comment above:
    cs_stat_openCLMonitorInstance.restoreProgramFromBuild(program);

    if (CL_SUCCESS == retVal)
    {
        // Add the related programs:
        cs_stat_openCLMonitorInstance.onProgramsLinked(program, num_input_headers, input_headers);

        // Notify the CL Monitor to, so an event will be created:
        cs_stat_openCLMonitorInstance.onBuildProgram(program, num_devices, device_list, options, areProgramBuildFlagsSupported, programDebuggingBlockedReason, false);
    }
    else // retVal != CL_SUCCESS
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clCompileProgram);

    return retVal;
}

cl_program CL_API_CALL clLinkProgram(cl_context context, cl_uint num_devices, const cl_device_id* device_list, const char* options,
                                     cl_uint num_input_programs, const cl_program* input_programs,
                                     void (CL_CALLBACK* pfn_notify)(cl_program program, void* user_data), void* user_data, cl_int* errcode_ret)
{
    cl_program retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clLinkProgram);

    // Notify the CL Monitor that the program is about to be built:
    // This should be performed before addFunctionCall since we want the event to be thrown before that breakpoint is triggered:
#pragma message ("Adapt the before / after failed callbacks to compile link options")
    // cs_stat_openCLMonitorInstance.onBeforeLinkPrograms(program, num_devices, device_list, options);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clLinkProgram, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_UINT_PARAMETER, num_devices,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, device_list, OS_TOBJ_ID_STRING_PARAMETER, options, OS_TOBJ_ID_CL_UINT_PARAMETER, num_input_programs,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, input_programs,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, pfn_notify, OS_TOBJ_ID_POINTER_PARAMETER, user_data, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Append the original options to the options string if there are:
    gtASCIIString optionsAsGTString;

    if (options != NULL)
    {
        optionsAsGTString.append(options);
    }

    // If we want to use kernel debugging, we need to add the "-g" and "-cl-opt-disable" flags to the compiler:
    bool isKernelDebuggingOn = cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled();
    bool modifiedFlags = false;
    bool allAMDPrograms = (0 != num_input_programs);

    // Get the program handles:
    cl_program* pProgramsInternalHandlers = new cl_program[num_input_programs];

    for (int i = 0; i < (int)num_input_programs; i++)
    {
        pProgramsInternalHandlers[i] = cs_stat_openCLMonitorInstance.internalProgramHandleFromExternalHandle(input_programs[i]);
        allAMDPrograms = allAMDPrograms && cs_stat_openCLMonitorInstance.isProgramOnAMDPlatform((oaCLProgramHandle)pProgramsInternalHandlers[i]);
    }

    // Only add out build flags if we are entirely on the AMD platform:
    if (isKernelDebuggingOn && allAMDPrograms && (!suGetGlobalServerEnvironmentSettings().m_csDontAddDebuggingBuildFlags))
    {
        optionsAsGTString.append(' ');
        optionsAsGTString.append(SU_STR_kernelDebuggingForcedBuildOptionsASCII);
        int amdRuntimeVer = cs_stat_openCLMonitorInstance.devicesMonitor().amdRuntimeVersion();

        if (cs_stat_pIKernelDebuggingManager->shouldAppendLegacyBuildFlags(optionsAsGTString, amdRuntimeVer))
        {
            optionsAsGTString.append(SU_STR_kernelDebuggingForcedBuildOptionLegacyASCII);
        }

        modifiedFlags = true;
    }

    // Call the real function:
    const char* modifiedOptions = modifiedFlags ? (optionsAsGTString.isEmpty() ? NULL : optionsAsGTString.asCharArray()) : options;
    retVal = cs_stat_realFunctionPointers.clLinkProgram(context, num_devices, device_list, modifiedOptions, num_input_programs, pProgramsInternalHandlers, pfn_notify, user_data, errcode_ret);

    // If we failed to build:
    if (isKernelDebuggingOn && (retVal == NULL))
    {
        // Try building without the flags:
        // cl_program originalRetVal = retVal;
        retVal = cs_stat_realFunctionPointers.clLinkProgram(context, num_devices, device_list, options, num_input_programs, pProgramsInternalHandlers, pfn_notify, user_data, errcode_ret);

        // If this succeeded:
        if (retVal != NULL)
        {
            // Report that the build failed due to the debug flags:
            // cs_stat_openCLMonitorInstance.onBuildProgramFailedWithDebugFlags(program, originalRetVal);
        }
    }

    // If the program was created successfully:
    if (NULL != retVal)
    {
        // Retain the program, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainProgram);
        cs_stat_realFunctionPointers.clRetainProgram(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainProgram);

        // Add a message instead of creating an empty source file:
        gtASCIIString noSourceMessage = "// Source Code not available for programs created with clLinkProgram";
        const char* noSourceMessageAsCharArray = noSourceMessage.asCharArray();

        // Log the program creation:
        cs_stat_openCLMonitorInstance.onProgramCreation(context, retVal, 1, &noSourceMessageAsCharArray, NULL);

        // Mark the input programs as related:
        cs_stat_openCLMonitorInstance.onProgramsLinked(retVal, num_input_programs, input_programs);
    }

    SU_END_FUNCTION_WRAPPER(ap_clLinkProgram);

    return retVal;
}

cl_int CL_API_CALL clUnloadCompiler()
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clUnloadCompiler);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall(OA_CL_NULL_HANDLE, ap_clUnloadCompiler, 0);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clUnloadCompiler();

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clUnloadCompiler);

    return retVal;
}

cl_int CL_API_CALL clUnloadPlatformCompiler(cl_platform_id platform)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clUnloadPlatformCompiler);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall(OA_CL_NULL_HANDLE, ap_clUnloadPlatformCompiler, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, platform);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clUnloadPlatformCompiler(platform);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clUnloadPlatformCompiler);

    return retVal;
}


cl_int CL_API_CALL clGetProgramInfo(cl_program program, cl_program_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetProgramInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)program, ap_clGetProgramInfo, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, program, OS_TOBJ_ID_CL_ENUM_PARAMETER, param_name, OS_TOBJ_ID_SIZE_T_PARAMETER,
                                                  param_value_size, OS_TOBJ_ID_POINTER_PARAMETER, param_value, OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size_ret);

    // Call the real function:
    cl_program programInternalHandle = cs_stat_openCLMonitorInstance.internalProgramHandleFromExternalHandle(program);
    retVal = cs_stat_realFunctionPointers.clGetProgramInfo(programInternalHandle, param_name, param_value_size, param_value, param_value_size_ret);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }
    else if (CL_PROGRAM_REFERENCE_COUNT == param_name) // && retVal == CL_SUCCESS
    {
        // Allow for a 0, NULL query without asserting:
        if (0 < param_value_size)
        {
            // Return the modified reference count for the program:
            GT_IF_WITH_ASSERT((sizeof(cl_uint) <= param_value_size) && (NULL != param_value))
            {
                *(cl_uint*)param_value = cs_stat_openCLMonitorInstance.programExternalReferenceCount(program);
            }
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetProgramInfo);

    return retVal;
}
cl_int CL_API_CALL clGetProgramBuildInfo(cl_program program, cl_device_id device, cl_program_build_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetProgramBuildInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)program, ap_clGetProgramBuildInfo, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, program, OS_TOBJ_ID_POINTER_PARAMETER, device,
                                                  OS_TOBJ_ID_CL_ENUM_PARAMETER, param_name, OS_TOBJ_ID_SIZE_T_PARAMETER, param_value_size, OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size_ret);

    // Call the real function:
    cl_program programInternalHandle = cs_stat_openCLMonitorInstance.internalProgramHandleFromExternalHandle(program);
    retVal = cs_stat_realFunctionPointers.clGetProgramBuildInfo(programInternalHandle, device, param_name, param_value_size, param_value, param_value_size_ret);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetProgramBuildInfo);

    return retVal;
}

cl_kernel CL_API_CALL clCreateKernel(cl_program program, const char* kernel_name, cl_int* errcode_ret)
{
    cl_kernel retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateKernel);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)program, ap_clCreateKernel, 3, OS_TOBJ_ID_CL_HANDLE_PARAMETER, program, OS_TOBJ_ID_STRING_PARAMETER, kernel_name, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    cl_program programInternalHandle = cs_stat_openCLMonitorInstance.internalProgramHandleFromExternalHandle(program);

    if (cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER == cs_stat_pIKernelDebuggingManager->kernelDebuggerType()))
    {
        retVal = cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptCreateKernel(programInternalHandle, kernel_name, errcode_ret);
    }
    else // !cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled()
    {
        retVal = cs_stat_realFunctionPointers.clCreateKernel(programInternalHandle, kernel_name, errcode_ret);
    }

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the program was created successfully:
    if (retVal != NULL)
    {
        // Retain the kernel, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainKernel);
        cs_stat_realFunctionPointers.clRetainKernel(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainKernel);

        // Log the kernel creation:
        gtString kernelName;
        kernelName.fromASCIIString(kernel_name);
        cs_stat_openCLMonitorInstance.onKernelCreation(program, retVal, kernelName);
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateKernel);

    return retVal;
}
cl_int CL_API_CALL clCreateKernelsInProgram(cl_program program, cl_uint num_kernels, cl_kernel* kernels, cl_uint* num_kernels_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clCreateKernelsInProgram);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)program, ap_clCreateKernelsInProgram, 4, OS_TOBJ_ID_CL_HANDLE_PARAMETER, program, OS_TOBJ_ID_CL_UINT_PARAMETER, num_kernels,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, kernels, OS_TOBJ_ID_CL_P_UINT_PARAMETER, num_kernels_ret);

    // Force the num_kernels_ret parameter to be non-NULL (this shouldn't affect the behavior of the function, as long as we're not changing 0, NULL, NULL to 0, NULL, <pointer>):
    cl_uint out_num_kernels_ret = 0;
    cl_uint* pUsed_num_kernels_ret = ((NULL != kernels) && (NULL == num_kernels_ret)) ? &out_num_kernels_ret : num_kernels_ret;

    // Call the real function:
    cl_program programInternalHandle = cs_stat_openCLMonitorInstance.internalProgramHandleFromExternalHandle(program);

    if (cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER == cs_stat_pIKernelDebuggingManager->kernelDebuggerType()))
    {
        retVal = cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptCreateKernelsInProgram(programInternalHandle, num_kernels, kernels, pUsed_num_kernels_ret);
    }
    else // !cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled()
    {
        retVal = cs_stat_realFunctionPointers.clCreateKernelsInProgram(programInternalHandle, num_kernels, kernels, pUsed_num_kernels_ret);
    }

    // On success, handle the created kernels:
    if (CL_SUCCESS == retVal)
    {
        // If the function created kernels rather than just querying their numbers:
        if (NULL != kernels)
        {
            // According to the OpenCL specification, the function should not succeed with the pUsed_num_kernels_ret being NULL:
            GT_IF_WITH_ASSERT(NULL != pUsed_num_kernels_ret)
            {
                cl_uint returnedKernels = *pUsed_num_kernels_ret;

                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainKernel);
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetKernelInfo);

                for (cl_uint i = 0; i < returnedKernels; i++)
                {
                    cl_kernel currentKernel = kernels[i];

                    if (NULL != currentKernel)
                    {
                        // Retain the kernel, so it won't get deleted without us knowing:
                        cs_stat_realFunctionPointers.clRetainKernel(currentKernel);

                        // Get the kernel's name:
                        gtString kernelName = L"(Unknown)";

                        size_t kernelNameLength = 0;
                        cl_int rcKerName = cs_stat_realFunctionPointers.clGetKernelInfo(currentKernel, CL_KERNEL_FUNCTION_NAME, 0, NULL, &kernelNameLength);

                        if ((CL_SUCCESS == rcKerName) && (0 < kernelNameLength))
                        {
                            char* pKernelName = new char[kernelNameLength + 1];
                            rcKerName = cs_stat_realFunctionPointers.clGetKernelInfo(currentKernel, CL_KERNEL_FUNCTION_NAME, kernelNameLength, pKernelName, NULL);
                            pKernelName[kernelNameLength] = (char)0;

                            if (CL_SUCCESS == rcKerName)
                            {
                                kernelName.fromASCIIString(pKernelName);
                            }

                            delete[] pKernelName;
                        }

                        // Log the kernel creation:
                        cs_stat_openCLMonitorInstance.onKernelCreation(program, currentKernel, kernelName);
                    }
                }

                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetKernelInfo);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainKernel);
            }
        }
    }
    else // retVal != CL_SUCCESS
    {
        // On failure, Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateKernelsInProgram);

    return retVal;
}
cl_int CL_API_CALL clRetainKernel(cl_kernel kernel)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clRetainKernel);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)kernel, ap_clRetainKernel, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, kernel);

    // Call the real function:
    cl_kernel kernelInternalHandle = cs_stat_openCLMonitorInstance.internalKernelHandleFromExternalHandle(kernel);

    if (cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER == cs_stat_pIKernelDebuggingManager->kernelDebuggerType()))
    {
        retVal = cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptRetainKernel(kernelInternalHandle);
    }
    else // !cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled()
    {
        retVal = cs_stat_realFunctionPointers.clRetainKernel(kernelInternalHandle);
    }

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clRetainKernel);

    return retVal;
}
cl_int CL_API_CALL clReleaseKernel(cl_kernel kernel)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clReleaseKernel);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)kernel, ap_clReleaseKernel, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, kernel);

    // Call the real function:
    cl_kernel kernelInternalHandle = cs_stat_openCLMonitorInstance.internalKernelHandleFromExternalHandle(kernel);

    if (cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER == cs_stat_pIKernelDebuggingManager->kernelDebuggerType()))
    {
        retVal = cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptReleaseKernel(kernelInternalHandle);
    }
    else // !cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled()
    {
        retVal = cs_stat_realFunctionPointers.clReleaseKernel(kernelInternalHandle);
    }

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    // Notify the OpenCL monitor we released (and possibly deleted) a kernel:
    cs_stat_openCLMonitorInstance.checkIfKernelWasDeleted(kernel, true);

    SU_END_FUNCTION_WRAPPER(ap_clReleaseKernel);

    return retVal;
}

cl_int CL_API_CALL clSetKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void* arg_value)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clSetKernelArg);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)kernel, ap_clSetKernelArg, 4, OS_TOBJ_ID_CL_HANDLE_PARAMETER, kernel, OS_TOBJ_ID_CL_UINT_PARAMETER, arg_index,
                                                  OS_TOBJ_ID_SIZE_T_PARAMETER, arg_size, OS_TOBJ_ID_POINTER_PARAMETER, arg_value);

    // Call the real function:
    cl_kernel kernelInternalHandle = cs_stat_openCLMonitorInstance.internalKernelHandleFromExternalHandle(kernel);

    if (cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER == cs_stat_pIKernelDebuggingManager->kernelDebuggerType()))
    {
        retVal = cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptSetKernelArg(kernelInternalHandle, arg_index, arg_size, arg_value);
    }
    else // !cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled()
    {
        retVal = cs_stat_realFunctionPointers.clSetKernelArg(kernelInternalHandle, arg_index, arg_size, arg_value);
    }

    if (retVal == CL_SUCCESS)
    {
        // Mark the kernel argument value:
        cs_stat_openCLMonitorInstance.onKernelArgumentSet(kernel, arg_index, arg_size, arg_value, false);
    }
    else // retVal != CL_SUCCESS
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);

        // TO_DO: On failure we can validate the argument size in comparison to clGetDeviceInfo(CL_DEVICE_MAX_PARAMETER_SIZE)? Should be useful for device emulation
    }

    SU_END_FUNCTION_WRAPPER(ap_clSetKernelArg);

    return retVal;
}

cl_int CL_API_CALL clGetKernelInfo(cl_kernel kernel, cl_kernel_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetKernelInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)kernel, ap_clGetKernelInfo, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, kernel, OS_TOBJ_ID_CL_ENUM_PARAMETER, param_name,
                                                  OS_TOBJ_ID_SIZE_T_PARAMETER, param_value_size, OS_TOBJ_ID_POINTER_PARAMETER, param_value, OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size_ret);

    // Call the real function:
    cl_kernel kernelInternalHandle = cs_stat_openCLMonitorInstance.internalKernelHandleFromExternalHandle(kernel);
    retVal = cs_stat_realFunctionPointers.clGetKernelInfo(kernelInternalHandle, param_name, param_value_size, param_value, param_value_size_ret);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }
    else if (CL_KERNEL_REFERENCE_COUNT == param_name) // && retVal == CL_SUCCESS
    {
        // Allow for a 0, NULL query without asserting:
        if (0 < param_value_size)
        {
            // Return the modified reference count for the kernel:
            GT_IF_WITH_ASSERT((sizeof(cl_uint) <= param_value_size) && (NULL != param_value))
            {
                *(cl_uint*)param_value = cs_stat_openCLMonitorInstance.kernelExternalReferenceCount(kernel);
            }
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetKernelInfo);

    return retVal;
}


cl_int CL_API_CALL clGetKernelArgInfo(cl_kernel kernel, cl_uint arg_indx, cl_kernel_arg_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetKernelArgInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)kernel, ap_clGetKernelArgInfo, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, kernel,
                                                  OS_TOBJ_ID_CL_P_UINT_PARAMETER, arg_indx, OS_TOBJ_ID_CL_ENUM_PARAMETER, param_name,
                                                  OS_TOBJ_ID_SIZE_T_PARAMETER, param_value_size, OS_TOBJ_ID_POINTER_PARAMETER, param_value, OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size_ret);

    // Call the real function:
    cl_kernel kernelInternalHandle = cs_stat_openCLMonitorInstance.internalKernelHandleFromExternalHandle(kernel);
    retVal = cs_stat_realFunctionPointers.clGetKernelArgInfo(kernelInternalHandle, arg_indx, param_name, param_value_size, param_value, param_value_size_ret);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetKernelArgInfo);

    return retVal;
}

cl_int CL_API_CALL clGetKernelWorkGroupInfo(cl_kernel kernel, cl_device_id device, cl_kernel_work_group_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetKernelWorkGroupInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)kernel, ap_clGetKernelWorkGroupInfo, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, kernel, OS_TOBJ_ID_POINTER_PARAMETER, device, OS_TOBJ_ID_CL_ENUM_PARAMETER, param_name,
                                                  OS_TOBJ_ID_SIZE_T_PARAMETER, param_value_size, OS_TOBJ_ID_POINTER_PARAMETER, param_value, OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size_ret);

    // Call the real function:
    cl_kernel kernelInternalHandle = cs_stat_openCLMonitorInstance.internalKernelHandleFromExternalHandle(kernel);
    retVal = cs_stat_realFunctionPointers.clGetKernelWorkGroupInfo(kernelInternalHandle, device, param_name, param_value_size, param_value, param_value_size_ret);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetKernelWorkGroupInfo);

    return retVal;
}

cl_int CL_API_CALL clWaitForEvents(cl_uint num_events, const cl_event* event_list)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clWaitForEvents);

    // Log the call to this function:
    if ((event_list != NULL) && (num_events > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)event_list[0], ap_clWaitForEvents, 2, OS_TOBJ_ID_CL_UINT_PARAMETER, num_events, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events, event_list);
    }
    else // (event_list == NULL) || (num_events <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall(OA_CL_NULL_HANDLE, ap_clWaitForEvents, 2, OS_TOBJ_ID_CL_UINT_PARAMETER, num_events, OS_TOBJ_ID_POINTER_PARAMETER, event_list);
    }

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clWaitForEvents(num_events, event_list);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    // If we are defined as a frame terminator:
    unsigned int frameTerminatorsMask = suFrameTerminatorsMask();

    if (frameTerminatorsMask & AP_CL_WAIT_FOR_EVENTS_TERMINATOR)
    {
        if (retVal == CL_INVALID_CONTEXT)
        {
            // This is attempting to wait on events from more than one context. That is an illegal operation.
            // We print to the log that we only terminate for the first event specified.
            OS_OUTPUT_DEBUG_LOG(CS_STR_DebugLog_clWaitForEventsTerminatingFrameOnlyForFirstContextInevents_list, OS_DEBUG_LOG_DEBUG);
        }

        if ((retVal != CL_INVALID_VALUE) && (retVal != CL_INVALID_EVENT))
        {
            // num_events <= should generate CL_INVALID_VALUE, so we shouldn't get here like that:
            GT_IF_WITH_ASSERT(num_events > 0)
            {
                // Get the command queue containing the first event in the vector:
                oaCLEventHandle firstEvent = (oaCLEventHandle)(event_list[0]);
                csContextMonitor* pContextMtr = cs_stat_openCLMonitorInstance.contextContainingEvent(firstEvent);
                GT_IF_WITH_ASSERT(pContextMtr != NULL)
                {
                    // Terminate the current frame:
                    cs_stat_openCLMonitorInstance.onFrameTerminatorCall(pContextMtr->contextHandle());
                }
            }
        }
        else // ((retVal == CL_INVALID_VALUE) || (retVal == CL_INVALID_EVENT))
        {
            // There are no events or one of the events specified
            OS_OUTPUT_DEBUG_LOG(CS_STR_DebugLog_clWaitForEventsWithBadParametersIgnoredAsTerminator, OS_DEBUG_LOG_DEBUG);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clWaitForEvents);

    return retVal;
}
cl_int CL_API_CALL clGetEventInfo(cl_event event, cl_event_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetEventInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)event, ap_clGetEventInfo, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, event, OS_TOBJ_ID_CL_ENUM_PARAMETER, param_name,
                                                  OS_TOBJ_ID_SIZE_T_PARAMETER, param_value_size, OS_TOBJ_ID_POINTER_PARAMETER, param_value, OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clGetEventInfo(event, param_name, param_value_size, param_value, param_value_size_ret);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }
    else if (CL_EVENT_REFERENCE_COUNT == param_name) // && retVal == CL_SUCCESS
    {
        // Allow for a 0, NULL query without asserting:
        if (0 < param_value_size)
        {
            // Return the modified reference count for the event:
            GT_IF_WITH_ASSERT((sizeof(cl_uint) <= param_value_size) && (NULL != param_value))
            {
                *(cl_uint*)param_value = cs_stat_openCLMonitorInstance.eventExternalReferenceCount(event);
            }
        }
    }

    // We add to events' reference count, so we need to change the value back here if that is what the user wanted:
    if ((param_name == CL_EVENT_REFERENCE_COUNT) && (retVal == CL_SUCCESS) && (param_value != NULL) && (param_value_size >= sizeof(cl_uint)))
    {
        // Get the command queue holding this event:
        oaCLEventHandle eveHandle = (oaCLEventHandle)event;
        const csContextMonitor* pContextMtr = cs_stat_openCLMonitorInstance.contextContainingEvent(eveHandle);
        GT_IF_WITH_ASSERT(pContextMtr != NULL)
        {
            // Get the event object:
            const apCLEvent* pEvent = pContextMtr->eventsMonitor().eventDetails(eveHandle);
            GT_IF_WITH_ASSERT(pEvent != NULL)
            {
                // If this object is still retained by the spy:
                if (pEvent->isRetainedBySpy())
                {
                    // Subtract one from the value:
                    cl_uint* paramValueAsCLUInt = (cl_uint*)param_value;
                    (*paramValueAsCLUInt)--;

                    // If the modified value is 0 (i.e. the event is retained ONLY by the spy):
                    if (*paramValueAsCLUInt == 0)
                    {
                        // The event would have been released by the OpenCL implementation if
                        // not for our intervention, so this handle should be invalid:
                        retVal = CL_INVALID_EVENT;
                    }
                }
            }
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetEventInfo);

    return retVal;
}

cl_event CL_API_CALL clCreateUserEvent(cl_context context, cl_int* errcode_ret)
{
    cl_event retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateUserEvent);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clCreateUserEvent, 2, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clCreateUserEvent(context, errcode_ret);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    if (NULL != retVal)
    {
        // Retain the event, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainEvent);
        cs_stat_realFunctionPointers.clRetainEvent(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainEvent);

        // Log the event creation:
        csContextMonitor* pContext = cs_stat_openCLMonitorInstance.clContextMonitor((oaCLContextHandle)context);
        GT_IF_WITH_ASSERT(NULL != pContext)
        {
            pContext->eventsMonitor().onEventCreated((oaCLEventHandle)retVal);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateUserEvent);

    return retVal;
}

cl_int CL_API_CALL clRetainEvent(cl_event event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clRetainEvent);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)event, ap_clRetainEvent, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, event);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clRetainEvent(event);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clRetainEvent);

    return retVal;
}

cl_int CL_API_CALL clReleaseEvent(cl_event event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clReleaseEvent);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)event, ap_clReleaseEvent, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, event);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clReleaseEvent(event);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    // Notify the OpenCL monitor we released (and possibly deleted) an event:
    cs_stat_openCLMonitorInstance.checkIfEventWasDeleted(event, true);

    SU_END_FUNCTION_WRAPPER(ap_clReleaseEvent);

    return retVal;
}

cl_int CL_API_CALL clSetUserEventStatus(cl_event event, cl_int execution_status)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clSetUserEventStatus);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)event, ap_clSetUserEventStatus, 2, OS_TOBJ_ID_CL_HANDLE_PARAMETER, event, OS_TOBJ_ID_CL_INT_PARAMETER, execution_status);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clSetUserEventStatus(event, execution_status);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clSetUserEventStatus);

    return retVal;
}

cl_int CL_API_CALL clSetEventCallback(cl_event event, cl_int command_exec_callback_type,
                                      void (CL_CALLBACK* pfn_notify)(cl_event, cl_int, void*), void* user_data)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clSetEventCallback);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)event, ap_clSetEventCallback, 4, OS_TOBJ_ID_CL_HANDLE_PARAMETER, event, OS_TOBJ_ID_CL_INT_PARAMETER, command_exec_callback_type,
                                                  OS_TOBJ_ID_POINTER_PARAMETER, pfn_notify, OS_TOBJ_ID_POINTER_PARAMETER, user_data);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clSetEventCallback(event, command_exec_callback_type, pfn_notify, user_data);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clSetEventCallback);

    return retVal;
}


cl_int CL_API_CALL clGetEventProfilingInfo(cl_event event, cl_profiling_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetEventProfilingInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)event, ap_clGetEventProfilingInfo, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, event, OS_TOBJ_ID_CL_ENUM_PARAMETER, param_name,
                                                  OS_TOBJ_ID_SIZE_T_PARAMETER, param_value_size, OS_TOBJ_ID_POINTER_PARAMETER, param_value, OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clGetEventProfilingInfo(event, param_name, param_value_size, param_value, param_value_size_ret);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetEventProfilingInfo);

    return retVal;
}

// Flush and Finish APIs
cl_int CL_API_CALL clFlush(cl_command_queue command_queue)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clFlush);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clFlush, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clFlush(command_queue);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    // If we are defined as a frame terminator:
    unsigned int frameTerminatorsMask = suFrameTerminatorsMask();

    if (frameTerminatorsMask & AP_CL_FLUSH_TERMINATOR)
    {
        // Get the command queue monitor:
        const csCommandQueueMonitor* pCommandQueueMtr = cs_stat_openCLMonitorInstance.commandQueueMonitor((oaCLCommandQueueHandle)command_queue);
        GT_IF_WITH_ASSERT(pCommandQueueMtr != NULL)
        {
            // Get the context related to this queue:
            int contextId = pCommandQueueMtr->contextSpyId();
            csContextMonitor* pContext = cs_stat_openCLMonitorInstance.clContextMonitor(contextId);
            GT_IF_WITH_ASSERT(pContext != NULL)
            {
                // Terminate the current frame:
                cs_stat_openCLMonitorInstance.onFrameTerminatorCall(pContext->contextHandle());
            }
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clFlush);

    return retVal;
}
cl_int CL_API_CALL clFinish(cl_command_queue command_queue)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clFinish);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clFinish, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clFinish(command_queue);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    // If we are defined as a frame terminator:
    unsigned int frameTerminatorsMask = suFrameTerminatorsMask();

    if (frameTerminatorsMask & AP_CL_FINISH_TERMINATOR)
    {
        // Get the command queue monitor:
        const csCommandQueueMonitor* pCommandQueueMtr = cs_stat_openCLMonitorInstance.commandQueueMonitor((oaCLCommandQueueHandle)command_queue);
        GT_IF_WITH_ASSERT(pCommandQueueMtr != NULL)
        {
            // Get the context related to this queue:
            int contextId = pCommandQueueMtr->contextSpyId();
            csContextMonitor* pContext = cs_stat_openCLMonitorInstance.clContextMonitor(contextId);
            GT_IF_WITH_ASSERT(pContext != NULL)
            {
                // Terminate the current frame:
                cs_stat_openCLMonitorInstance.onFrameTerminatorCall(pContext->contextHandle());
            }
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clFinish);

    return retVal;
}

cl_int CL_API_CALL clEnqueueReadBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read, size_t offset, size_t cb, void* ptr, cl_uint num_events_in_wait_list,
                                       const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueReadBuffer);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueReadBuffer, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, buffer, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_read,
                                                      OS_TOBJ_ID_SIZE_T_PARAMETER, offset, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, cb, OS_TOBJ_ID_POINTER_PARAMETER, ptr,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueReadBuffer, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, buffer, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_read,
                                                      OS_TOBJ_ID_SIZE_T_PARAMETER, offset, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, cb, OS_TOBJ_ID_POINTER_PARAMETER, ptr,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Check if read execution is on:
    bool areReadOperationsOn = false;
    areReadOperationsOn = cs_stat_openCLMonitorInstance.isOpenCLOperationExecutionOn(AP_OPENCL_READ_OPERATION_EXECUTION);

    if (areReadOperationsOn)
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function:
        retVal = cs_stat_realFunctionPointers.clEnqueueReadBuffer(command_queue, buffer, blocking_read, offset, cb, ptr, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLReadBufferCommand((oaCLMemHandle)buffer, blocking_read, offset, cb, (osProcedureAddress64)ptr, num_events_in_wait_list, event_wait_list, event));
    }
    else // !areReadOperationsOn
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function, but stub the operation size to 1 byte:
        retVal = cs_stat_realFunctionPointers.clEnqueueReadBuffer(command_queue, buffer, blocking_read, offset, 1, ptr, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLReadBufferCommand((oaCLMemHandle)buffer, blocking_read, offset, 1, (osProcedureAddress64)ptr, num_events_in_wait_list, event_wait_list, event));
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueReadBuffer);

    return retVal;
}

cl_int CL_API_CALL clEnqueueReadBufferRect(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read,
                                           const size_t* buffer_origin, const size_t* host_origin, const size_t* region,
                                           size_t buffer_row_pitch, size_t buffer_slice_pitch, size_t host_row_pitch, size_t host_slice_pitch,
                                           void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueReadBufferRect);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueReadBufferRect, 14, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, buffer, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_read,
                                                      OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, buffer_origin, OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, host_origin, OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, region,
                                                      OS_TOBJ_ID_BYTES_SIZE_PARAMETER, buffer_row_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, buffer_slice_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, host_row_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, host_slice_pitch,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, ptr, OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER,
                                                      OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueReadBufferRect, 14, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, buffer, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_read,
                                                      OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, buffer_origin, OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, host_origin, OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, region,
                                                      OS_TOBJ_ID_BYTES_SIZE_PARAMETER, buffer_row_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, buffer_slice_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, host_row_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, host_slice_pitch,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, ptr, OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Check if read execution is on:
    bool areReadOperationsOn = false;
    areReadOperationsOn = cs_stat_openCLMonitorInstance.isOpenCLOperationExecutionOn(AP_OPENCL_READ_OPERATION_EXECUTION);

    if (areReadOperationsOn)
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function:
        retVal = cs_stat_realFunctionPointers.clEnqueueReadBufferRect(command_queue, buffer, blocking_read, buffer_origin, host_origin, region,
                                                                      buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLReadBufferRectCommand((oaCLMemHandle)buffer, blocking_read, buffer_origin, host_origin, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, (osProcedureAddress64)ptr, num_events_in_wait_list, event_wait_list, event));
    }
    else // !areReadOperationsOn
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function, but stub the operation size to 1 pixel:
        static const size_t stub_region[3] = {1, 1, 1};
        retVal = cs_stat_realFunctionPointers.clEnqueueReadBufferRect(command_queue, buffer, blocking_read, buffer_origin, host_origin, stub_region,
                                                                      buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLReadBufferRectCommand((oaCLMemHandle)buffer, blocking_read, buffer_origin, host_origin, stub_region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, (osProcedureAddress64)ptr, num_events_in_wait_list, event_wait_list, event));
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueReadBufferRect);

    return retVal;
}

cl_int CL_API_CALL clEnqueueWriteBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t cb, const void* ptr,
                                        cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueWriteBuffer);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueWriteBuffer, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, buffer, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_write,
                                                      OS_TOBJ_ID_SIZE_T_PARAMETER, offset, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, cb, OS_TOBJ_ID_POINTER_PARAMETER, ptr,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueWriteBuffer, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, buffer, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_write,
                                                      OS_TOBJ_ID_SIZE_T_PARAMETER, offset, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, cb, OS_TOBJ_ID_POINTER_PARAMETER, ptr,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Check if write execution is on:
    bool areWriteOperationsOn = false;
    areWriteOperationsOn = cs_stat_openCLMonitorInstance.isOpenCLWriteOperationExecutionOn(buffer);

    if (areWriteOperationsOn)
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function:
        retVal = cs_stat_realFunctionPointers.clEnqueueWriteBuffer(command_queue, buffer, blocking_write, offset, cb, ptr, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLWriteBufferCommand((oaCLMemHandle)buffer, blocking_write, offset, cb, (osProcedureAddress64)ptr, num_events_in_wait_list, event_wait_list, event));

        // Notify the OpenCL monitor the write command was executed:
        cs_stat_openCLMonitorInstance.onMemoryObjectWriteCommand((oaCLMemHandle)buffer);
    }
    else // !areWriteOperationsOn
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function, but stub the operation size to 1 byte:
        oaCLMemHandle stub_buffer = cs_stat_openCLMonitorInstance.stubBufferFromRealBuffer((oaCLMemHandle)buffer);
        retVal = cs_stat_realFunctionPointers.clEnqueueWriteBuffer(command_queue, (cl_mem)stub_buffer, blocking_write, 0, 1, ptr, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLWriteBufferCommand((oaCLMemHandle)buffer, blocking_write, 0, 1, (osProcedureAddress64)ptr, num_events_in_wait_list, event_wait_list, event));

        // Notify the OpenCL monitor the write command was executed:
        cs_stat_openCLMonitorInstance.onMemoryObjectWriteCommand((oaCLMemHandle)buffer);
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueWriteBuffer);

    return retVal;
}

cl_int CL_API_CALL clEnqueueWriteBufferRect(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write,
                                            const size_t* buffer_origin, const size_t* host_origin, const size_t* region,
                                            size_t buffer_row_pitch, size_t buffer_slice_pitch, size_t host_row_pitch, size_t host_slice_pitch,
                                            const void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueWriteBufferRect);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueWriteBufferRect, 14, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, buffer, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_write,
                                                      OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, buffer_origin, OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, host_origin, OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, region,
                                                      OS_TOBJ_ID_BYTES_SIZE_PARAMETER, buffer_row_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, buffer_slice_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, host_row_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, host_slice_pitch,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, ptr, OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER,
                                                      OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueWriteBufferRect, 14, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, buffer, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_write,
                                                      OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, buffer_origin, OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, host_origin, OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, region,
                                                      OS_TOBJ_ID_BYTES_SIZE_PARAMETER, buffer_row_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, buffer_slice_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, host_row_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, host_slice_pitch,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, ptr, OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Check if write execution is on:
    bool areWriteOperationsOn = false;
    areWriteOperationsOn = cs_stat_openCLMonitorInstance.isOpenCLWriteOperationExecutionOn(buffer);

    if (areWriteOperationsOn)
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function:
        retVal = cs_stat_realFunctionPointers.clEnqueueWriteBufferRect(command_queue, buffer, blocking_write, buffer_origin, host_origin, region,
                                                                       buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLWriteBufferRectCommand((oaCLMemHandle)buffer, blocking_write, buffer_origin, host_origin, region,  buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, (osProcedureAddress64)ptr, num_events_in_wait_list, event_wait_list, event));

        // Notify the OpenCL monitor the write command was executed:
        cs_stat_openCLMonitorInstance.onMemoryObjectWriteCommand((oaCLMemHandle)buffer);
    }
    else // !areWriteOperationsOn
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function, but stub the operation size to 1 pixel:
        oaCLMemHandle stub_buffer = cs_stat_openCLMonitorInstance.stubBufferFromRealBuffer((oaCLMemHandle)buffer);
        static const size_t stub_buffer_origin[3] = {0, 0, 0};
        static const size_t stub_region[3] = {1, 1, 1};
        retVal = cs_stat_realFunctionPointers.clEnqueueWriteBufferRect(command_queue, (cl_mem)stub_buffer, blocking_write, stub_buffer_origin, host_origin, stub_region,
                                                                       buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLWriteBufferRectCommand((oaCLMemHandle)buffer, blocking_write, stub_buffer_origin, host_origin, stub_region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, (osProcedureAddress64)ptr, num_events_in_wait_list, event_wait_list, event));

        // Notify the OpenCL monitor the write command was executed:
        cs_stat_openCLMonitorInstance.onMemoryObjectWriteCommand((oaCLMemHandle)buffer);
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueWriteBufferRect);

    return retVal;
}

cl_int CL_API_CALL clEnqueueFillBuffer(cl_command_queue command_queue, cl_mem buffer, const void* pattern, size_t pattern_size, size_t offset, size_t size,
                                       cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueFillBuffer);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueFillBuffer, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, buffer,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, pattern, OS_TOBJ_ID_SIZE_T_PARAMETER, pattern_size, OS_TOBJ_ID_SIZE_T_PARAMETER, offset, OS_TOBJ_ID_SIZE_T_PARAMETER, size,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER,
                                                      OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueFillBuffer, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, buffer,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, pattern, OS_TOBJ_ID_SIZE_T_PARAMETER, pattern_size, OS_TOBJ_ID_SIZE_T_PARAMETER, offset, OS_TOBJ_ID_SIZE_T_PARAMETER, size,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Check if write execution is on (We currently do not separate fill from write):
    bool areFillOperationsOn = cs_stat_openCLMonitorInstance.isOpenCLWriteOperationExecutionOn(buffer);

    if (areFillOperationsOn)
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function:
        retVal = cs_stat_realFunctionPointers.clEnqueueFillBuffer(command_queue, buffer, pattern, pattern_size, offset, size, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLFillBufferCommand((oaCLMemHandle)buffer, pattern, pattern_size, offset, size, num_events_in_wait_list, event_wait_list, event));

        // Notify the OpenCL monitor the write command was executed:
        cs_stat_openCLMonitorInstance.onMemoryObjectWriteCommand((oaCLMemHandle)buffer);
    }
    else // !areFillOperationsOn
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function, but stub the operation size to 1 byte:
        oaCLMemHandle stub_buffer = cs_stat_openCLMonitorInstance.stubBufferFromRealBuffer((oaCLMemHandle)buffer);
        retVal = cs_stat_realFunctionPointers.clEnqueueFillBuffer(command_queue, (cl_mem)stub_buffer, pattern, 1, 0, 1, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLFillBufferCommand((oaCLMemHandle)buffer, pattern, 1, 0, 1, num_events_in_wait_list, event_wait_list, event));

        // Notify the OpenCL monitor the write command was executed:
        cs_stat_openCLMonitorInstance.onMemoryObjectWriteCommand((oaCLMemHandle)buffer);
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueFillBuffer);

    return retVal;
}

// TO_DO: size parameter clEnqueue*
cl_int CL_API_CALL clEnqueueCopyBuffer(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer, size_t src_offset, size_t dst_offset, size_t cb,
                                       cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueCopyBuffer);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueCopyBuffer, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, src_buffer, OS_TOBJ_ID_CL_HANDLE_PARAMETER, dst_buffer,
                                                      OS_TOBJ_ID_SIZE_T_PARAMETER, src_offset, OS_TOBJ_ID_SIZE_T_PARAMETER, dst_offset, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, cb,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueCopyBuffer, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, src_buffer, OS_TOBJ_ID_CL_HANDLE_PARAMETER, dst_buffer,
                                                      OS_TOBJ_ID_SIZE_T_PARAMETER, src_offset, OS_TOBJ_ID_SIZE_T_PARAMETER, dst_offset, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, cb,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Check if copy execution is on:
    bool areCopyOperationsOn = false;
    areCopyOperationsOn = cs_stat_openCLMonitorInstance.isOpenCLOperationExecutionOn(AP_OPENCL_COPY_OPERATION_EXECUTION);

    if (areCopyOperationsOn)
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function:
        retVal = cs_stat_realFunctionPointers.clEnqueueCopyBuffer(command_queue, src_buffer, dst_buffer, src_offset, dst_offset, cb, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLCopyBufferCommand((oaCLMemHandle)src_buffer, (oaCLMemHandle)dst_buffer, src_offset, dst_offset, cb, num_events_in_wait_list, event_wait_list, event));
    }
    else // !areCopyOperationsOn
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function, but stub the operation size to 1 byte:
        oaCLMemHandle stub_dst_buffer = cs_stat_openCLMonitorInstance.stubBufferFromRealBuffer((oaCLMemHandle)dst_buffer);
        retVal = cs_stat_realFunctionPointers.clEnqueueCopyBuffer(command_queue, src_buffer, (cl_mem)stub_dst_buffer, src_offset, 0, 1, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLCopyBufferCommand((oaCLMemHandle)src_buffer, (oaCLMemHandle)dst_buffer, src_offset, 0, 1, num_events_in_wait_list, event_wait_list, event));
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueCopyBuffer);

    return retVal;
}

cl_int CL_API_CALL clEnqueueCopyBufferRect(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer, const size_t* src_origin, const size_t* dst_origin,
                                           const size_t* region, size_t src_row_pitch, size_t src_slice_pitch, size_t dst_row_pitch, size_t dst_slice_pitch,
                                           cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueCopyBufferRect);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueCopyBufferRect, 13, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, src_buffer, OS_TOBJ_ID_CL_HANDLE_PARAMETER, dst_buffer,
                                                      OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, src_origin, OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, dst_origin, OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, region,
                                                      OS_TOBJ_ID_BYTES_SIZE_PARAMETER, src_row_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, src_slice_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, dst_row_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, dst_slice_pitch,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueCopyBufferRect, 13, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, src_buffer, OS_TOBJ_ID_CL_HANDLE_PARAMETER, dst_buffer,
                                                      OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, src_origin, OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, dst_origin, OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, region,
                                                      OS_TOBJ_ID_BYTES_SIZE_PARAMETER, src_row_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, src_slice_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, dst_row_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, dst_slice_pitch,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Check if copy execution is on:
    bool areCopyOperationsOn = false;
    areCopyOperationsOn = cs_stat_openCLMonitorInstance.isOpenCLOperationExecutionOn(AP_OPENCL_COPY_OPERATION_EXECUTION);

    if (areCopyOperationsOn)
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function:
        retVal = cs_stat_realFunctionPointers.clEnqueueCopyBufferRect(command_queue, src_buffer, dst_buffer, src_origin, dst_origin, region, src_row_pitch, src_slice_pitch, dst_row_pitch,
                                                                      dst_slice_pitch, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLCopyBufferRectCommand((oaCLMemHandle)src_buffer, (oaCLMemHandle)dst_buffer, src_origin, dst_origin, region, src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch, num_events_in_wait_list, event_wait_list, event));
    }
    else // !areCopyOperationsOn
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function, but stub the operation size to 1 pixel:
        oaCLMemHandle stub_dst_buffer = cs_stat_openCLMonitorInstance.stubBufferFromRealBuffer((oaCLMemHandle)dst_buffer);
        static const size_t stub_dst_origin[3] = {0, 0, 0};
        static const size_t stub_region[3] = {1, 1, 1};
        retVal = cs_stat_realFunctionPointers.clEnqueueCopyBufferRect(command_queue, src_buffer, (cl_mem)stub_dst_buffer, src_origin, stub_dst_origin, stub_region, src_row_pitch, src_slice_pitch, dst_row_pitch,
                                                                      dst_slice_pitch, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLCopyBufferRectCommand((oaCLMemHandle)src_buffer, (oaCLMemHandle)dst_buffer, src_origin, stub_dst_origin, stub_region, src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch, num_events_in_wait_list, event_wait_list, event));
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueCopyBufferRect);

    return retVal;
}

cl_int CL_API_CALL clEnqueueReadImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_read, const size_t* origin/*[3]*/, const size_t* region/*[3]*/, size_t row_pitch,
                                      size_t slice_pitch, void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueReadImage);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueReadImage, 11, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, image, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_read,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, origin, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, region,
                                                      OS_TOBJ_ID_SIZE_T_PARAMETER, row_pitch, OS_TOBJ_ID_SIZE_T_PARAMETER, slice_pitch, OS_TOBJ_ID_POINTER_PARAMETER, ptr,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueReadImage, 11, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, image, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_read,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, origin, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, region,
                                                      OS_TOBJ_ID_SIZE_T_PARAMETER, row_pitch, OS_TOBJ_ID_SIZE_T_PARAMETER, slice_pitch, OS_TOBJ_ID_POINTER_PARAMETER, ptr,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Check if read execution is on:
    bool areReadOperationsOn = false;
    areReadOperationsOn = cs_stat_openCLMonitorInstance.isOpenCLOperationExecutionOn(AP_OPENCL_READ_OPERATION_EXECUTION);

    if (areReadOperationsOn)
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function:
        retVal = cs_stat_realFunctionPointers.clEnqueueReadImage(command_queue, image, blocking_read, origin, region, row_pitch, slice_pitch, ptr, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLReadImageCommand((oaCLMemHandle)image, blocking_read, origin, region, row_pitch, slice_pitch, (osProcedureAddress64)ptr, num_events_in_wait_list, event_wait_list, event));
    }
    else // !areReadOperationsOn
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function, but stub the operation size to 1 pixel:
        static const size_t stub_region[3] = {1, 1, 1};
        retVal = cs_stat_realFunctionPointers.clEnqueueReadImage(command_queue, image, blocking_read, origin, stub_region, row_pitch, slice_pitch, ptr, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLReadImageCommand((oaCLMemHandle)image, blocking_read, origin, stub_region, row_pitch, slice_pitch, (osProcedureAddress64)ptr, num_events_in_wait_list, event_wait_list, event));
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueReadImage);

    return retVal;
}

cl_int CL_API_CALL clEnqueueWriteImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_write, const size_t* origin/*[3]*/, const size_t* region/*[3]*/,
                                       size_t input_row_pitch, size_t input_slice_pitch, const void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueWriteImage);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueWriteImage, 11, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, image, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_write,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, origin, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, region,
                                                      OS_TOBJ_ID_BYTES_SIZE_PARAMETER, input_row_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, input_slice_pitch, OS_TOBJ_ID_POINTER_PARAMETER, ptr,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueWriteImage, 11, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, image, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_write,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, origin, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, region,
                                                      OS_TOBJ_ID_BYTES_SIZE_PARAMETER, input_row_pitch, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, input_slice_pitch, OS_TOBJ_ID_POINTER_PARAMETER, ptr,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Check if write execution is on:
    bool areWriteOperationsOn = false;
    areWriteOperationsOn = cs_stat_openCLMonitorInstance.isOpenCLWriteOperationExecutionOn(image);

    if (areWriteOperationsOn)
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function:
        retVal = cs_stat_realFunctionPointers.clEnqueueWriteImage(command_queue, image, blocking_write, origin, region, input_row_pitch, input_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLWriteImageCommand((oaCLMemHandle)image, blocking_write, origin, region, input_row_pitch, input_slice_pitch, (osProcedureAddress64)ptr, num_events_in_wait_list, event_wait_list, event));

        // Notify the OpenCL monitor the write command was executed:
        cs_stat_openCLMonitorInstance.onMemoryObjectWriteCommand((oaCLMemHandle)image);
    }
    else // !areWriteOperationsOn
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function, but stub the operation size to 1 pixel:
        oaCLMemHandle stub_image = cs_stat_openCLMonitorInstance.stubImageFromRealImage((oaCLMemHandle)image);
        static const size_t stub_origin[3] = {0, 0, 0};
        static const size_t stub_region[3] = {1, 1, 1};
        retVal = cs_stat_realFunctionPointers.clEnqueueWriteImage(command_queue, (cl_mem)stub_image, blocking_write, stub_origin, stub_region, input_row_pitch, input_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLWriteImageCommand((oaCLCommandQueueHandle)image, blocking_write, stub_origin, stub_region, input_row_pitch, input_slice_pitch, (osProcedureAddress64)ptr, num_events_in_wait_list, event_wait_list, event));

        // Notify the OpenCL monitor the write command was executed:
        cs_stat_openCLMonitorInstance.onMemoryObjectWriteCommand((oaCLMemHandle)image);
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueWriteImage);

    return retVal;
}

cl_int CL_API_CALL clEnqueueFillImage(cl_command_queue command_queue, cl_mem image, const void* fill_color, const size_t* origin, const size_t* region,
                                      cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueFillImage);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueFillImage, 8, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, image,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, fill_color,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, origin, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, region,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueFillImage, 8, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, image,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, fill_color,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, origin, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, region,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Mark that a command is about to be added:
    CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

    // The fill color structure depends on the channel data type and order of the image:
    // Get the image channel data format & type:
    /*
        // Get the texture monitor for this context:
        cl_uint dataFormat = 0;
        cl_uint dataType = 0;
        csImagesAndBuffersMonitor& imagesAndBuffersMonitor = pContextMonitor->imagesAndBuffersMonitor();
        apCLMemObject* pMemObject = imagesAndBuffersMonitor.getMemObjectDetails((oaCLMemHandle)image);
        GT_IF_WITH_ASSERT(pMemObject != NULL)
        {
            GT_IF_WITH_ASSERT(pMemObject->type() == OS_TOBJ_ID_CL_IMAGE)
            {
                apCLImage* pImage = (apCLImage*)pMemObject;
                GT_IF_WITH_ASSERT(pImage != NULL)
                {
                    // Get the data format and type from the image:
                    dataFormat = pImage->dataFormat();
                    dataType = pImage->dataType();
                }
            }
        }
        */

    // Check if write execution is on (We currently do not separate fill from write):
    bool areFillOperationsOn = cs_stat_openCLMonitorInstance.isOpenCLWriteOperationExecutionOn(image);

    if (areFillOperationsOn)
    {
        // Call the real function:
        retVal = cs_stat_realFunctionPointers.clEnqueueFillImage(command_queue, image, fill_color, origin, region, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLFillImageCommand((oaCLMemHandle)image, fill_color, origin, region, num_events_in_wait_list, event_wait_list, event));

        // Notify the OpenCL monitor the write command was executed:
        cs_stat_openCLMonitorInstance.onMemoryObjectWriteCommand((oaCLMemHandle)image);
    }
    else // !areFillOperationsOn
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEventInner, event, wasEventCreatedBySpy, pContextMonitorInner, command_queue);

        // Call the real function, but stub the operation size to 1 pixel:
        oaCLMemHandle stub_image = cs_stat_openCLMonitorInstance.stubImageFromRealImage((oaCLMemHandle)image);
        static const size_t stub_origin[3] = {0, 0, 0};
        static const size_t stub_region[3] = {1, 1, 1};
        retVal = cs_stat_realFunctionPointers.clEnqueueFillImage(command_queue, (cl_mem)stub_image, fill_color, stub_origin, stub_region, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEventInner, wasEventCreatedBySpy, pContextMonitorInner, command_queue, retVal,
                                    new apCLFillImageCommand((oaCLCommandQueueHandle)image, fill_color, stub_origin, stub_region, num_events_in_wait_list, event_wait_list, event));

        // Notify the OpenCL monitor the write command was executed:
        cs_stat_openCLMonitorInstance.onMemoryObjectWriteCommand((oaCLMemHandle)image);
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueFillImage);

    return retVal;
}

cl_int CL_API_CALL clEnqueueCopyImage(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_image, const size_t* src_origin/*[3]*/, const size_t* dst_origin/*[3]*/, const size_t* region/*[3]*/,
                                      cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueCopyImage);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueCopyImage, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, src_image, OS_TOBJ_ID_CL_HANDLE_PARAMETER, dst_image,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, src_origin, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, dst_origin, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, region,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueCopyImage, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, src_image, OS_TOBJ_ID_CL_HANDLE_PARAMETER, dst_image,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, src_origin, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, dst_origin, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, region,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Check if copy execution is on:
    bool areCopyOperationsOn = false;
    areCopyOperationsOn = cs_stat_openCLMonitorInstance.isOpenCLOperationExecutionOn(AP_OPENCL_COPY_OPERATION_EXECUTION);

    if (areCopyOperationsOn)
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function:
        retVal = cs_stat_realFunctionPointers.clEnqueueCopyImage(command_queue, src_image, dst_image, src_origin, dst_origin, region, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLCopyImageCommand((oaCLMemHandle)src_image, (oaCLMemHandle)dst_image, src_origin, dst_origin, region, num_events_in_wait_list, event_wait_list, event));
    }
    else // !areCopyOperationsOn
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function, but stub the operation size to 1 pixel:
        oaCLMemHandle stub_dst_image = cs_stat_openCLMonitorInstance.stubImageFromRealImage((oaCLMemHandle)dst_image);
        static const size_t stub_dst_origin[3] = {0, 0, 0};
        static const size_t stub_region[3] = {1, 1, 1};
        retVal = cs_stat_realFunctionPointers.clEnqueueCopyImage(command_queue, src_image, (cl_mem)stub_dst_image, src_origin, stub_dst_origin, stub_region, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLCopyImageCommand((oaCLMemHandle)src_image, (oaCLMemHandle)dst_image, src_origin, stub_dst_origin, stub_region, num_events_in_wait_list, event_wait_list, event));
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueCopyImage);

    return retVal;
}

cl_int CL_API_CALL clEnqueueCopyImageToBuffer(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_buffer, const size_t* src_origin/*[3]*/, const size_t* region/*[3]*/,
                                              size_t dst_offset, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueCopyImageToBuffer);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueCopyImageToBuffer, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, src_image, OS_TOBJ_ID_CL_HANDLE_PARAMETER, dst_buffer,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, src_origin, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, region, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, dst_offset,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueCopyImageToBuffer, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, src_image, OS_TOBJ_ID_CL_HANDLE_PARAMETER, dst_buffer,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, src_origin, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, region, OS_TOBJ_ID_SIZE_T_PARAMETER, dst_offset,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Check if copy execution is on:
    bool areCopyOperationsOn = false;
    areCopyOperationsOn = cs_stat_openCLMonitorInstance.isOpenCLOperationExecutionOn(AP_OPENCL_COPY_OPERATION_EXECUTION);

    if (areCopyOperationsOn)
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function:
        retVal = cs_stat_realFunctionPointers.clEnqueueCopyImageToBuffer(command_queue, src_image, dst_buffer, src_origin, region, dst_offset, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLCopyImageToBufferCommand((oaCLMemHandle)src_image, (oaCLMemHandle)dst_buffer, src_origin, region, dst_offset, num_events_in_wait_list, event_wait_list, event));
    }
    else // !areCopyOperationsOn
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function, but stub the operation size to 1 pixel:
        oaCLMemHandle stub_dst_buffer = cs_stat_openCLMonitorInstance.stubBufferFromRealBuffer((oaCLMemHandle)dst_buffer);
        static const size_t stub_region[3] = {1, 1, 1};
        retVal = cs_stat_realFunctionPointers.clEnqueueCopyImageToBuffer(command_queue, src_image, (cl_mem)stub_dst_buffer, src_origin, stub_region, 0, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLCopyImageToBufferCommand((oaCLMemHandle)src_image, (oaCLMemHandle)dst_buffer, src_origin, stub_region, 0, num_events_in_wait_list, event_wait_list, event));
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueCopyImageToBuffer);

    return retVal;
}

cl_int CL_API_CALL clEnqueueCopyBufferToImage(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_image, size_t src_offset, const size_t* dst_origin/*[3]*/,
                                              const size_t* region/*[3]*/, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueCopyBufferToImage);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueCopyBufferToImage, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, src_buffer, OS_TOBJ_ID_CL_HANDLE_PARAMETER, dst_image,
                                                      OS_TOBJ_ID_SIZE_T_PARAMETER, src_offset, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, dst_origin, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, 3, region,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueCopyBufferToImage, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, src_buffer, OS_TOBJ_ID_CL_HANDLE_PARAMETER, dst_image,
                                                      OS_TOBJ_ID_SIZE_T_PARAMETER, src_offset, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, dst_origin, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_BYTES_SIZE_PARAMETER, 3, region,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Check if copy execution is on:
    bool areCopyOperationsOn = false;
    areCopyOperationsOn = cs_stat_openCLMonitorInstance.isOpenCLOperationExecutionOn(AP_OPENCL_COPY_OPERATION_EXECUTION);

    if (areCopyOperationsOn)
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function:
        retVal = cs_stat_realFunctionPointers.clEnqueueCopyBufferToImage(command_queue, src_buffer, dst_image, src_offset, dst_origin, region, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLCopyBufferToImageCommand((oaCLMemHandle)src_buffer, (oaCLMemHandle)dst_image, src_offset, dst_origin, region, num_events_in_wait_list, event_wait_list, event));
    }
    else // !areCopyOperationsOn
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function, but stub the operation size to 1 pixel:
        oaCLMemHandle stub_dst_image = cs_stat_openCLMonitorInstance.stubImageFromRealImage((oaCLMemHandle)dst_image);
        static const size_t stub_dst_origin[3] = {0, 0, 0};
        static const size_t stub_region[3] = {1, 1, 1};
        retVal = cs_stat_realFunctionPointers.clEnqueueCopyBufferToImage(command_queue, src_buffer, (cl_mem)stub_dst_image, src_offset, stub_dst_origin, stub_region, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLCopyBufferToImageCommand((oaCLMemHandle)src_buffer, (oaCLMemHandle)dst_image, src_offset, stub_dst_origin, stub_region, num_events_in_wait_list, event_wait_list, event));
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueCopyBufferToImage);

    return retVal;
}

void* CL_API_CALL clEnqueueMapBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_map, cl_map_flags map_flags, size_t offset,
                                     size_t cb, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event, cl_int* errcode_ret)
{
    void* retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueMapBuffer);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueMapBuffer, 10, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, buffer, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_map,
                                                      OS_TOBJ_ID_CL_MAP_FLAGS_PARAMETER, map_flags, OS_TOBJ_ID_SIZE_T_PARAMETER, offset, OS_TOBJ_ID_SIZE_T_PARAMETER, cb,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueMapBuffer, 10, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, buffer, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_map,
                                                      OS_TOBJ_ID_CL_MAP_FLAGS_PARAMETER, map_flags, OS_TOBJ_ID_SIZE_T_PARAMETER, offset, OS_TOBJ_ID_SIZE_T_PARAMETER, cb,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);
    }

    // Mark that a command is about to be added:
    CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clEnqueueMapBuffer(command_queue, buffer, blocking_map, map_flags, offset, cb, num_events_in_wait_list, event_wait_list, pEvent, errcode_ret);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, ((errcode_ret != NULL) ? (*errcode_ret) : CL_SUCCESS),
                                new apCLMapBufferCommand((oaCLMemHandle)buffer, blocking_map, map_flags, offset, cb, num_events_in_wait_list, event_wait_list, event, (osProcedureAddress64)errcode_ret));

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueMapBuffer);

    return retVal;
}

void* CL_API_CALL clEnqueueMapImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_map, cl_map_flags map_flags, const size_t* origin/*[3]*/, const size_t* region/*[3]*/,
                                    size_t* image_row_pitch, size_t* image_slice_pitch, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event, cl_int* errcode_ret)
{
    void* retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueMapImage);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueMapImage, 12, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, image, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_map, OS_TOBJ_ID_CL_MAP_FLAGS_PARAMETER, map_flags,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, origin, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, region,
                                                      OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, image_row_pitch, OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, image_slice_pitch,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueMapImage, 12, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, image, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_map, OS_TOBJ_ID_CL_MAP_FLAGS_PARAMETER, map_flags,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, origin, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, 3, region,
                                                      OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, image_row_pitch, OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER, image_slice_pitch,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);
    }

    // Mark that a command is about to be added:
    CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clEnqueueMapImage(command_queue, image, blocking_map, map_flags, origin, region, image_row_pitch, image_slice_pitch, num_events_in_wait_list, event_wait_list, pEvent, errcode_ret);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, ((errcode_ret != NULL) ? (*errcode_ret) : CL_SUCCESS),
                                new apCLMapImageCommand((oaCLMemHandle)image, blocking_map, map_flags, origin, region, image_row_pitch, image_slice_pitch, num_events_in_wait_list, event_wait_list, event, (osProcedureAddress64)errcode_ret));

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueMapImage);

    return retVal;
}

cl_int CL_API_CALL clEnqueueUnmapMemObject(cl_command_queue command_queue, cl_mem memobj, void* mapped_ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueUnmapMemObject);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueUnmapMemObject, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, memobj, OS_TOBJ_ID_POINTER_PARAMETER, mapped_ptr,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueUnmapMemObject, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, memobj, OS_TOBJ_ID_POINTER_PARAMETER, mapped_ptr,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Mark that a command is about to be added:
    CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clEnqueueUnmapMemObject(command_queue, memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, pEvent);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                new apCLUnmapMemObjectCommand((oaCLMemHandle)memobj, (osProcedureAddress64)mapped_ptr, num_events_in_wait_list, event_wait_list, event));

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueUnmapMemObject);

    return retVal;
}

cl_int CL_API_CALL clEnqueueMigrateMemObjects(cl_command_queue command_queue, cl_uint num_mem_objects, const cl_mem* mem_objects, cl_mem_migration_flags flags,
                                              cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueMigrateMemObjects);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueMigrateMemObjects, 7, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_UINT_PARAMETER, num_mem_objects,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, mem_objects, OS_TOBJ_ID_CL_MEM_MIGRATION_FLAGS_PARAMETER, flags,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueMigrateMemObjects, 7, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_UINT_PARAMETER, num_mem_objects,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, mem_objects, OS_TOBJ_ID_CL_MEM_MIGRATION_FLAGS_PARAMETER, flags,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Mark that a command is about to be added:
    CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clEnqueueMigrateMemObjects(command_queue, num_mem_objects, mem_objects, flags, num_events_in_wait_list, event_wait_list, pEvent);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                new apCLMigrateMemObjectsCommand(num_mem_objects, mem_objects, flags, num_events_in_wait_list, event_wait_list, event));

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueMigrateMemObjects);

    return retVal;
}

cl_int CL_API_CALL clEnqueueNDRangeKernel(cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim, const size_t* global_work_offset, const size_t* global_work_size, const size_t* local_work_size,
                                          cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueNDRangeKernel);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueNDRangeKernel, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, kernel, OS_TOBJ_ID_CL_UINT_PARAMETER, work_dim,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, work_dim, global_work_offset, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, work_dim, global_work_size,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, work_dim, local_work_size, OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueNDRangeKernel, 9, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, kernel, OS_TOBJ_ID_CL_UINT_PARAMETER, work_dim,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, work_dim, global_work_offset, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, work_dim, global_work_size, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_SIZE_T_PARAMETER, work_dim, local_work_size,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Check if kernel execution is on:
    bool areKernelOperationsOn = false;
    areKernelOperationsOn = cs_stat_openCLMonitorInstance.isOpenCLOperationExecutionOn(AP_OPENCL_KERNEL_EXECUTION);

    if (areKernelOperationsOn)
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function:
        cl_kernel kernelInternalHandle = cs_stat_openCLMonitorInstance.internalKernelHandleFromExternalHandle(kernel);
        suIKernelDebuggingManager::KernelDebuggingSessionReason debugReason = suIKernelDebuggingManager::STEP_IN_COMMAND;

        osCriticalSectionDelayedLocker delayedLocker;

        if (cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && cs_stat_openCLMonitorInstance.shouldDebugKernel((oaCLKernelHandle)kernel, debugReason, delayedLocker))
        {
            OS_OUTPUT_DEBUG_LOG(L"Enqueueing kernel for debug", OS_DEBUG_LOG_EXTENSIVE);
            suIKernelDebuggingManager::KernelDebuggingManagerType debuggerType = cs_stat_pIKernelDebuggingManager->kernelDebuggerType();

            switch (debuggerType)
            {
                case suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER:
                {
                    // If we're using the software debugger, we must use the special enqueue function
                    OS_OUTPUT_DEBUG_LOG(L"Kernel debugging manager = CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER", OS_DEBUG_LOG_EXTENSIVE);

                    // Check if any of the kernel arguments are not supported (e.g. SVM buffers):
                    bool kernelHasUnsupportedArgs = false;
                    bool programIsNotSupported = false;
                    gtString errorString;
                    cl_int originalCLError = CL_SUCCESS;

                    // Get the context containing the kernel object:
                    oaCLKernelHandle hKernel = (oaCLKernelHandle)kernel;
                    const csContextMonitor* pContext = cs_stat_openCLMonitorInstance.contextContainingKernel(hKernel);

                    if (NULL != pContext)
                    {
                        const csProgramsAndKernelsMonitor& programsAndKernelsMon = pContext->programsAndKernelsMonitor();
                        const csCLKernel* pKernel = programsAndKernelsMon.kernelMonitor(hKernel);
                        GT_IF_WITH_ASSERT(NULL != pKernel)
                        {
                            // See if any of the kernel arguments is an SVM buffer, or if the kernel is set to use SVM pointers:
                            kernelHasUnsupportedArgs = pKernel->kernelHasSVMArguments() || pKernel->kernelUsesSVMPointers();

                            if (kernelHasUnsupportedArgs)
                            {
                                errorString = CS_STR_CouldNotDebugKernelInvalidArgsDetails;
                            }

                            // Also get the program containing this kernel:
                            const apCLProgram* pProgram = programsAndKernelsMon.programMonitor(pKernel->programHandle());
                            GT_IF_WITH_ASSERT(NULL != pProgram)
                            {
                                gtString notSupportedReason;
                                programIsNotSupported = !pProgram->canDebugProgram(notSupportedReason);

                                if (programIsNotSupported)
                                {
                                    errorString = notSupportedReason;
                                }
                            }
                        }
                    }

                    if (!(kernelHasUnsupportedArgs || programIsNotSupported))
                    {
                        // Let the OpenCL monitor know we are about to debug a kernel:
                        cs_stat_openCLMonitorInstance.beforeKernelDebuggingEnqueued();

                        // Create the debug session struct:
                        csAMDKernelDebuggingSession* pKernelDebuggingSession = new csAMDKernelDebuggingSession((oaCLKernelHandle)kernel, ap_clEnqueueNDRangeKernel, debugReason, (unsigned int)work_dim, global_work_offset, global_work_size, local_work_size);

                        // Call the debug version of EnqueuNDRangeKernel:
                        retVal = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugEnqueueNDRangeKernel(command_queue, kernelInternalHandle, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, pEvent, &csAMDKernelDebuggingManager::csAMDclDebugCallback, (void*)pKernelDebuggingSession);

                        // Let the OpenCL monitor know we've finished debugging the kernel:
                        cs_stat_openCLMonitorInstance.afterKernelDebuggingEnqueued();

                        // If we cannot debug, we get a special return value that we cannot return to the user:
                        originalCLError = retVal;

                        if (retVal == CL_SUCCESS)
                        {
                            // Note that we successfully started debugging:
                            suIKernelDebuggingManager::setDisptachInFlight(true);

                            if (pKernelDebuggingSession->_canDebugSessionBeDeleted)
                            {
                                // The synchronous execution is over successfully, delete the structure:
                                delete pKernelDebuggingSession;
                            }
                            else // pKernelDebuggingSession->_canDebugSessionBeDeleted
                            {
                                // We can now allow releasing this session structure for asynchronous debugging:
                                pKernelDebuggingSession->_canDebugSessionBeDeleted = true;
                            }
                        }
                        else // retVal != CL_SUCCESS
                        {
                            // Clear the kernel debugging manager:
                            cs_stat_openCLMonitorInstance.onKernelDebuggingEnqueueFailure();

                            // Delete the unused session structure:
                            delete pKernelDebuggingSession;
                        }
                    }
                    else if (programIsNotSupported)
                    {
                        // Don't enqueue the kernel for debug:
                        originalCLError = CL_KERNEL_NOT_DEBUGGABLE_AMD;
                    }
                    else if (kernelHasUnsupportedArgs)
                    {
                        // Don't enqueue the kernel for debug:
                        originalCLError = CL_INVALID_ARG_VALUE;
                    }

                    if (CL_SUCCESS != retVal)
                    {
                        // Call the real function to execute the kernel and get a return value:
                        retVal = cs_stat_realFunctionPointers.clEnqueueNDRangeKernel(command_queue, kernelInternalHandle, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, pEvent);
                    }

                    if ((originalCLError != CL_SUCCESS) || (retVal != CL_SUCCESS))
                    {
                        // If the kernel args are all supported, this is a DBE or API failure:
                        if ((retVal != CL_SUCCESS) || !kernelHasUnsupportedArgs)
                        {
                            cs_stat_openCLMonitorInstance.reportKernelDebuggingFailure(originalCLError, retVal, errorString);
                        }
                        else
                        {
                            suIKernelDebuggingManager::reportKernelDebuggingFailure(retVal, apKernelDebuggingFailedEvent::AP_KERNEL_ARGS_NOT_SUPPORTED, errorString);
                        }

                        // Break at the next function call, since we failed to step in / break at the kernel:
                        su_stat_theBreakpointsManager.setBreakpointAtNextMonitoredFunctionCall();
                    }
                }
                break;

                case suIKernelDebuggingManager::HD_HARDWARE_KERNEL_DEBUGGER:
                {
                    // Currently only enable hardware debugging on Windows:
                    OS_OUTPUT_DEBUG_LOG(L"Kernel debugging manager = HD_HARDWARE_KERNEL_DEBUGGER", OS_DEBUG_LOG_EXTENSIVE);
#if defined(CS_USE_HD_HSA_HW_BASED_DEBUGGER) && (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
                    // This is the HSA hardware-based kernel debugging manager

                    // Let the OpenCL monitor know we are about to debug a kernel:
                    cs_stat_openCLMonitorInstance.beforeKernelDebuggingEnqueued();

                    // Lock the kernel debugging manager:
                    hd_stat_hsaHardwareBasedDebuggingManager.AcquireHSADispatchLock();

                    gtString logMessage;
                    logMessage.appendFormattedString(L"clEnqueueNDRangeKernel with kernel %p", kernel);
                    OS_OUTPUT_DEBUG_LOG(logMessage.asCharArray(), OS_DEBUG_LOG_INFO);

                    // In HSA, we must force the creation of the pEvent variable, even if it wasn't created otherwise
                    // by the CS_BEFORE_ENQUEUEING_COMMAND macro:
                    bool allocatedEventForHWDbg = false;

                    if (NULL == pEvent)
                    {
                        pEvent = new cl_event;
                        *pEvent = NULL;
                        allocatedEventForHWDbg = true;
                    }

                    // Call the real function:
                    retVal = cs_stat_realFunctionPointers.clEnqueueNDRangeKernel(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, pEvent);

                    // Get the kernel's context Id:
                    apContextID contextId = pContextMonitor->contextID();

                    // Get the containing program:
                    oaCLProgramHandle programHandle = cs_stat_openCLMonitorInstance.programContainingKernel((oaCLKernelHandle)kernel);

                    // Get the program and kernel details:
                    const apCLProgram* pProgramDetails = pContextMonitor->programsAndKernelsMonitor().programMonitor(programHandle);
                    GT_ASSERT(NULL != pProgramDetails);
                    const csCLKernel* pKernelDetails = pContextMonitor->programsAndKernelsMonitor().kernelMonitor((oaCLKernelHandle)kernel);
                    GT_ASSERT(NULL != pKernelDetails);

                    static const gtString emptyStr;

                    // Get the source code path of the currently debugged kernel:
                    const gtString& programSourceFilePath = (NULL != pProgramDetails) ? pProgramDetails->sourceCodeFilePath().asString() : emptyStr;

                    // Get the kernel name:
                    const gtString& kernelFnName = (NULL != pKernelDetails) ? pKernelDetails->kernelFunctionName() : emptyStr;

                    // Set the event to be debugged:
                    oaCLEventHandle hEve = (oaCLEventHandle) * pEvent;

                    if (allocatedEventForHWDbg)
                    {
                        delete pEvent;
                        pEvent = NULL;
                    }

                    // Create the kernel debugging session:
                    hd_stat_hsaHardwareBasedDebuggingManager.CreateKernelDebugSession(ap_clEnqueueNDRangeKernel, contextId,
                                                                                      programHandle, programSourceFilePath, (oaCLKernelHandle)kernel, kernelFnName, hEve, debugReason,
                                                                                      (unsigned int)work_dim, global_work_offset, global_work_size, local_work_size);

                    // Note that we successfully started debugging:
                    suIKernelDebuggingManager::setDisptachInFlight(true);

                    // Unlock the kernel debugging manager:
                    hd_stat_hsaHardwareBasedDebuggingManager.ReleaseHSADispatchLock();

                    // Let the OpenCL monitor know we've finished debugging the kernel:
                    cs_stat_openCLMonitorInstance.afterKernelDebuggingEnqueued();

                    if (retVal != CL_SUCCESS)
                    {
                        // Clear the kernel debugging manager:
                        cs_stat_openCLMonitorInstance.onKernelDebuggingEnqueueFailure();

                        // If we cannot debug, we get a special return value that we cannot return to the user:
                        gtString lastErrorString;
                        cl_int originalCLError = retVal;
                        cs_stat_openCLMonitorInstance.reportKernelDebuggingFailure(originalCLError, retVal, lastErrorString);

                        // Break at the next function call, since we failed to step in / break at the kernel:
                        su_stat_theBreakpointsManager.setBreakpointAtNextMonitoredFunctionCall();
                    }

#else // !defined(CS_USE_HD_HSA_HW_BASED_DEBUGGER) || AMDT_BUILD_TARGET != AMDT_WINDOWS_OS
                    GT_ASSERT(false);
#endif
                }
                break;

                default:
                {
                    // Unexpected debugger type:
                    GT_ASSERT(false);
                }
                break;
            }
        }
        else // !cs_stat_pIKernelDebuggingManager->shouldDebugKernel((oaCLKernelHandle)kernel)
        {
            retVal = cs_stat_realFunctionPointers.clEnqueueNDRangeKernel(command_queue, kernelInternalHandle, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, pEvent);
        }

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        // Handle Enqueue range kernel operation:
        cs_stat_openCLMonitorInstance.onEnqueueNDRangeKernel((oaCLCommandQueueHandle)command_queue);

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLNDRangeKernelCommand((oaCLKernelHandle)kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event));
    }
    else // !areKernelOperationsOn
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function, but stub the kernel to an empty one:
        oaCLKernelHandle stub_kernel = cs_stat_openCLMonitorInstance.stubKernelFromRealKernel((oaCLKernelHandle)kernel);
        retVal = cs_stat_realFunctionPointers.clEnqueueNDRangeKernel(command_queue, (cl_kernel)stub_kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        // Handle Enqueue range kernel operation:
        cs_stat_openCLMonitorInstance.onEnqueueNDRangeKernel((oaCLCommandQueueHandle)command_queue);

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLNDRangeKernelCommand((oaCLKernelHandle)kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event));
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueNDRangeKernel);

    return retVal;
}

cl_int CL_API_CALL clEnqueueTask(cl_command_queue command_queue, cl_kernel kernel, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueTask);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueTask, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, kernel,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueTask, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_HANDLE_PARAMETER, kernel,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Check if kernel execution is on:
    bool areKernelOperationsOn = false;
    areKernelOperationsOn = cs_stat_openCLMonitorInstance.isOpenCLOperationExecutionOn(AP_OPENCL_KERNEL_EXECUTION);

    if (areKernelOperationsOn)
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function:
        cl_kernel kernelInternalHandle = cs_stat_openCLMonitorInstance.internalKernelHandleFromExternalHandle(kernel);
        suIKernelDebuggingManager::KernelDebuggingSessionReason debugReason = suIKernelDebuggingManager::STEP_IN_COMMAND;

        osCriticalSectionDelayedLocker delayedLocker;

        if (cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && cs_stat_openCLMonitorInstance.shouldDebugKernel((oaCLKernelHandle)kernel, debugReason, delayedLocker))
        {
            suIKernelDebuggingManager::KernelDebuggingManagerType debuggerType = cs_stat_pIKernelDebuggingManager->kernelDebuggerType();

            size_t workSize = 1;
            size_t workOffset = 0;

            switch (debuggerType)
            {
                case suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER:
                {
                    // If we're using the software debugger, we must use the special enqueue function

                    // Check if any of the kernel arguments are not supported (e.g. SVM buffers):
                    bool kernelHasUnsupportedArgs = false;
                    bool programIsNotSupported = false;
                    gtString errorString;
                    cl_int originalCLError = CL_SUCCESS;

                    // Get the context containing the kernel object:
                    oaCLKernelHandle hKernel = (oaCLKernelHandle)kernel;
                    const csContextMonitor* pContext = cs_stat_openCLMonitorInstance.contextContainingKernel(hKernel);

                    if (NULL != pContext)
                    {
                        const csProgramsAndKernelsMonitor& programsAndKernelsMon = pContext->programsAndKernelsMonitor();
                        const csCLKernel* pKernel = programsAndKernelsMon.kernelMonitor(hKernel);
                        GT_IF_WITH_ASSERT(NULL != pKernel)
                        {
                            // See if any of the kernel arguments is an SVM buffer, or if the kernel is set to use SVM pointers:
                            kernelHasUnsupportedArgs = pKernel->kernelHasSVMArguments() || pKernel->kernelUsesSVMPointers();

                            if (kernelHasUnsupportedArgs)
                            {
                                errorString = CS_STR_CouldNotDebugKernelInvalidArgsDetails;
                            }

                            // Also get the program containing this kernel:
                            const apCLProgram* pProgram = programsAndKernelsMon.programMonitor(pKernel->programHandle());
                            GT_IF_WITH_ASSERT(NULL != pProgram)
                            {
                                gtString notSupportedReason;
                                programIsNotSupported = !pProgram->canDebugProgram(notSupportedReason);

                                if (programIsNotSupported)
                                {
                                    errorString = notSupportedReason;
                                }
                            }
                        }
                    }

                    if (!(kernelHasUnsupportedArgs || programIsNotSupported))
                    {
                        // Let the OpenCL monitor know we are about to debug a kernel:
                        cs_stat_openCLMonitorInstance.beforeKernelDebuggingEnqueued();

                        // Create the debug session struct:
                        csAMDKernelDebuggingSession* pKernelDebuggingSession = new csAMDKernelDebuggingSession((oaCLKernelHandle)kernel, ap_clEnqueueTask, debugReason, 1, &workOffset, &workSize, &workSize);

                        // Call the debug version of EnqueuNDRangeKernel. Note that EnqueueTask is the same as calling EnqueueNDRangeKernel with all the size parameters set to 1, and the offset as NULL
                        retVal = cs_stat_amdKernelDebuggingFunctionPointers.amdclDebugEnqueueTask(command_queue, kernelInternalHandle, num_events_in_wait_list, event_wait_list, event, &csAMDKernelDebuggingManager::csAMDclDebugCallback, (void*)pKernelDebuggingSession);

                        // Let the OpenCL monitor know we've finished debugging the kernel:
                        cs_stat_openCLMonitorInstance.afterKernelDebuggingEnqueued();

                        // If we cannot debug, we get a special return value that we cannot return to the user:
                        originalCLError = retVal;

                        if (retVal == CL_SUCCESS)
                        {
                            // Note that we successfully started debugging:
                            suIKernelDebuggingManager::setDisptachInFlight(true);

                            if (pKernelDebuggingSession->_canDebugSessionBeDeleted)
                            {
                                // The synchronous execution is over successfully, delete the structure:
                                delete pKernelDebuggingSession;
                            }
                            else // pKernelDebuggingSession->_canDebugSessionBeDeleted
                            {
                                // We can now allow releasing this session structure for asynchronous debugging:
                                pKernelDebuggingSession->_canDebugSessionBeDeleted = true;
                            }
                        }
                        else // retVal != CL_SUCCESS
                        {
                            // Clear the kernel debugging manager:
                            cs_stat_openCLMonitorInstance.onKernelDebuggingEnqueueFailure();

                            // Delete the unused session structure:
                            delete pKernelDebuggingSession;
                        }
                    }
                    else if (programIsNotSupported)
                    {
                        // Don't enqueue the kernel for debug:
                        originalCLError = CL_KERNEL_NOT_DEBUGGABLE_AMD;
                    }
                    else if (kernelHasUnsupportedArgs)
                    {
                        // Don't enqueue the kernel for debug:
                        originalCLError = CL_INVALID_ARG_VALUE;
                    }

                    if (CL_SUCCESS != retVal)
                    {
                        // Call the real function to execute the kernel and get a return value:
                        retVal = cs_stat_realFunctionPointers.clEnqueueTask(command_queue, kernelInternalHandle, num_events_in_wait_list, event_wait_list, pEvent);
                    }

                    if ((originalCLError != CL_SUCCESS) || (retVal != CL_SUCCESS))
                    {
                        // If the kernel args are all supported, this is a DBE or API failure:
                        if ((retVal != CL_SUCCESS) || !kernelHasUnsupportedArgs)
                        {
                            cs_stat_openCLMonitorInstance.reportKernelDebuggingFailure(originalCLError, retVal, errorString);
                        }
                        else
                        {
                            suIKernelDebuggingManager::reportKernelDebuggingFailure(retVal, apKernelDebuggingFailedEvent::AP_KERNEL_ARGS_NOT_SUPPORTED, errorString);
                        }

                        // Break at the next function call, since we failed to step in / break at the kernel:
                        su_stat_theBreakpointsManager.setBreakpointAtNextMonitoredFunctionCall();
                    }
                }
                break;

                case suIKernelDebuggingManager::HD_HARDWARE_KERNEL_DEBUGGER:
                {
                    // Currently only enable hardware debugging on Windows:
#if defined(CS_USE_HD_HSA_HW_BASED_DEBUGGER) && (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
                    // This is the HSA hardware-based kernel debugging manager

                    // Let the OpenCL monitor know we are about to debug a kernel:
                    cs_stat_openCLMonitorInstance.beforeKernelDebuggingEnqueued();

                    // Lock the kernel debugging manager:
                    hd_stat_hsaHardwareBasedDebuggingManager.AcquireHSADispatchLock();

                    // In HSA, we must force the creation of the pEvent variable, even if it wasn't created otherwise
                    // by the CS_BEFORE_ENQUEUEING_COMMAND macro:
                    bool allocatedEventForHWDbg = false;

                    if (NULL == pEvent)
                    {
                        pEvent = new cl_event;
                        *pEvent = NULL;
                        allocatedEventForHWDbg = true;
                    }

                    // Call the real function:
                    retVal = cs_stat_realFunctionPointers.clEnqueueTask(command_queue, kernelInternalHandle, num_events_in_wait_list, event_wait_list, pEvent);

                    // Get the kernel's context Id:
                    apContextID contextId = pContextMonitor->contextID();

                    // Get the containing program:
                    oaCLProgramHandle programHandle = cs_stat_openCLMonitorInstance.programContainingKernel((oaCLKernelHandle)kernel);

                    // Get the program and kernel details:
                    const apCLProgram* pProgramDetails = pContextMonitor->programsAndKernelsMonitor().programMonitor(programHandle);
                    GT_ASSERT(NULL != pProgramDetails);
                    const csCLKernel* pKernelDetails = pContextMonitor->programsAndKernelsMonitor().kernelMonitor((oaCLKernelHandle)kernel);
                    GT_ASSERT(NULL != pKernelDetails);

                    static const gtString emptyStr;

                    // Get the source code path of the currently debugged kernel:
                    const gtString& programSourceFilePath = (NULL != pProgramDetails) ? pProgramDetails->sourceCodeFilePath().asString() : emptyStr;

                    // Get the kernel name:
                    const gtString& kernelFnName = (NULL != pKernelDetails) ? pKernelDetails->kernelFunctionName() : emptyStr;

                    // Set the event to be debugged:
                    oaCLEventHandle hEve = (oaCLEventHandle) * pEvent;

                    if (allocatedEventForHWDbg)
                    {
                        delete pEvent;
                        pEvent = NULL;
                    }

                    // Create the kernel debugging session:
                    hd_stat_hsaHardwareBasedDebuggingManager.CreateKernelDebugSession(ap_clEnqueueTask, contextId, programHandle, programSourceFilePath,
                                                                                      (oaCLKernelHandle)kernel, kernelFnName, hEve, debugReason, 1, &workOffset, &workSize, &workSize);

                    // Note that we successfully started debugging:
                    suIKernelDebuggingManager::setDisptachInFlight(true);

                    // Unlock the kernel debugging manager:
                    hd_stat_hsaHardwareBasedDebuggingManager.ReleaseHSADispatchLock();

                    // Let the OpenCL monitor know we've finished debugging the kernel:
                    cs_stat_openCLMonitorInstance.afterKernelDebuggingEnqueued();

                    if (retVal != CL_SUCCESS)
                    {
                        // Clear the kernel debugging manager:
                        cs_stat_openCLMonitorInstance.onKernelDebuggingEnqueueFailure();

                        // If we cannot debug, we get a special return value that we cannot return to the user:
                        gtString lastErrorString;
                        cl_int originalCLError = retVal;
                        cs_stat_openCLMonitorInstance.reportKernelDebuggingFailure(originalCLError, retVal, lastErrorString);

                        // Break at the next function call, since we failed to step in / break at the kernel:
                        su_stat_theBreakpointsManager.setBreakpointAtNextMonitoredFunctionCall();
                    }

#else // !defined(CS_USE_HD_HSA_HW_BASED_DEBUGGER) || AMDT_BUILD_TARGET != AMDT_WINDOWS_OS
                    GT_ASSERT(false);
#endif
                }
                break;

                default:
                {
                    // Unexpected debugger type:
                    GT_ASSERT(false);
                }
                break;
            }
        }
        else // !cs_stat_pIKernelDebuggingManager->shouldDebugKernel((oaCLKernelHandle)kernel)
        {
            retVal = cs_stat_realFunctionPointers.clEnqueueTask(command_queue, kernelInternalHandle, num_events_in_wait_list, event_wait_list, pEvent);
        }

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLTaskCommand((oaCLKernelHandle)kernel, num_events_in_wait_list, event_wait_list, event));
    }
    else // !areKernelOperationsOn
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function, but stub the kernel to an empty one:
        oaCLKernelHandle stub_kernel = cs_stat_openCLMonitorInstance.stubKernelFromRealKernel((oaCLKernelHandle)kernel);
        retVal = cs_stat_realFunctionPointers.clEnqueueTask(command_queue, (cl_kernel)stub_kernel, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLTaskCommand((oaCLKernelHandle)kernel, num_events_in_wait_list, event_wait_list, event));
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueTask);

    return retVal;
}

cl_int CL_API_CALL clEnqueueMarkerWithWaitList(cl_command_queue command_queue, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueMarkerWithWaitList);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        // Log the call to this function:
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueMarkerWithWaitList, 4, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER,
                                                      OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueMarkerWithWaitList, 4, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Mark that a command is about to be added:
    CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clEnqueueMarkerWithWaitList(command_queue, num_events_in_wait_list, event_wait_list, pEvent);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                new apCLMarkerWithWaitListCommand(num_events_in_wait_list, event_wait_list, event));

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueMarkerWithWaitList);

    return retVal;
}

cl_int CL_API_CALL clEnqueueBarrierWithWaitList(cl_command_queue command_queue, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueBarrierWithWaitList);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        // Log the call to this function:
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueBarrierWithWaitList, 4, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER,
                                                      OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueBarrierWithWaitList, 4, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Mark that a command is about to be added:
    CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clEnqueueBarrierWithWaitList(command_queue, num_events_in_wait_list, event_wait_list, pEvent);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                new apCLBarrierWithWaitListCommand(num_events_in_wait_list, event_wait_list, event));

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueBarrierWithWaitList);

    return retVal;
}

cl_int CL_API_CALL clSetPrintfCallback(cl_context context, void (CL_CALLBACK* pfn_notify)(cl_context program, cl_uint printf_data_len, char* printf_data_ptr, void* user_data), void* user_data)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clSetPrintfCallback);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall(OA_CL_NULL_HANDLE, ap_clSetPrintfCallback, 3, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_POINTER_PARAMETER, pfn_notify, OS_TOBJ_ID_POINTER_PARAMETER, user_data);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clSetPrintfCallback(context, pfn_notify, user_data);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clSetPrintfCallback);

    return retVal;
}

void* CL_API_CALL clGetExtensionFunctionAddressForPlatform(cl_platform_id platform, const char* func_name)
{
    void* retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clGetExtensionFunctionAddressForPlatform);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall(OA_CL_NULL_HANDLE, ap_clGetExtensionFunctionAddressForPlatform, 2, OS_TOBJ_ID_CL_HANDLE_PARAMETER, platform, OS_TOBJ_ID_STRING_PARAMETER, func_name);

    // Try and get the wrapper address from our extensions manager (this should return the real pointer for unsupported functions):
    gtString extensionFunctionName;
    extensionFunctionName.fromASCIIString(func_name);
    retVal = (void*)(cs_stat_extensionsManager.wrapperFunctionAddress(platform, extensionFunctionName));

    // If that didn't work for some reason, call the real function:
    if (retVal == NULL)
    {
        gtString logMsg;

        if (NULL != func_name)
        {
            logMsg.fromASCIIString(func_name);
        }

        logMsg.prependFormattedString(L"Could not get OpenCL extension address from platform %p. Function name: \"", (void*)platform);

        retVal = cs_stat_realFunctionPointers.clGetExtensionFunctionAddressForPlatform(platform, func_name);

        logMsg.appendFormattedString(L"\". Pointer from real API: %p", retVal);
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_INFO);
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetExtensionFunctionAddressForPlatform);

    return retVal;
}

cl_int CL_API_CALL clEnqueueNativeKernel(cl_command_queue command_queue, void (CL_CALLBACK* user_func)(void*), void* args, size_t cb_args, cl_uint num_mem_objects, const cl_mem* mem_list,
                                         const void** args_mem_loc, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueNativeKernel);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueNativeKernel, 10, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_POINTER_PARAMETER, user_func, OS_TOBJ_ID_POINTER_PARAMETER, args,
                                                      OS_TOBJ_ID_BYTES_SIZE_PARAMETER, cb_args, OS_TOBJ_ID_CL_UINT_PARAMETER, num_mem_objects, OS_TOBJ_ID_POINTER_PARAMETER, mem_list,
                                                      OS_TOBJ_ID_PP_VOID_PARAMETER, args_mem_loc, OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueNativeKernel, 10, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_POINTER_PARAMETER, user_func, OS_TOBJ_ID_POINTER_PARAMETER, args,
                                                      OS_TOBJ_ID_BYTES_SIZE_PARAMETER, cb_args, OS_TOBJ_ID_CL_UINT_PARAMETER, num_mem_objects, OS_TOBJ_ID_POINTER_PARAMETER, mem_list,
                                                      OS_TOBJ_ID_PP_VOID_PARAMETER, args_mem_loc, OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Check if kernel execution is on:
    bool areKernelOperationsOn = false;
    areKernelOperationsOn = cs_stat_openCLMonitorInstance.isOpenCLOperationExecutionOn(AP_OPENCL_KERNEL_EXECUTION);

    if (areKernelOperationsOn)
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function:
        retVal = cs_stat_realFunctionPointers.clEnqueueNativeKernel(command_queue, user_func, args, cb_args, num_mem_objects, mem_list, args_mem_loc, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLNativeKernelCommand((osProcedureAddress64)user_func, (osProcedureAddress64)args, cb_args, num_mem_objects, mem_list, args_mem_loc, num_events_in_wait_list, event_wait_list, event));
    }
    else // !areKernelOperationsOn
    {
        // Mark that a command is about to be added:
        CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

        // Call the real function, but stub the kernel to an empty one:
        retVal = cs_stat_realFunctionPointers.clEnqueueNativeKernel(command_queue, &csOpenCLMonitor::stubNativeKernel, args, cb_args, num_mem_objects, mem_list, args_mem_loc, num_events_in_wait_list, event_wait_list, pEvent);

        if (retVal != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
        }

        CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                    new apCLNativeKernelCommand((osProcedureAddress64)user_func, (osProcedureAddress64)args, cb_args, num_mem_objects, mem_list, args_mem_loc, num_events_in_wait_list, event_wait_list, event));
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueNativeKernel);

    return retVal;
}

cl_int CL_API_CALL clEnqueueMarker(cl_command_queue command_queue, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueMarker);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueMarker, 2, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_POINTER_PARAMETER, event);

    // Mark that a command is about to be added:
    CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clEnqueueMarker(command_queue, pEvent);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                new apCLMarkerCommand(event));

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueMarker);

    return retVal;
}
cl_int CL_API_CALL clEnqueueWaitForEvents(cl_command_queue command_queue, cl_uint num_events, const cl_event* event_list)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueWaitForEvents);

    // Log the call to this function:
    if ((event_list != NULL) && (num_events > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueWaitForEvents, 3, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_UINT_PARAMETER, num_events, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events, event_list);
    }
    else // (event_list == NULL) || (num_events <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueWaitForEvents, 3, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_UINT_PARAMETER, num_events, OS_TOBJ_ID_POINTER_PARAMETER, event_list);
    }

    // Mark that a command is about to be added:
    CS_BEFORE_ENQUEUEING_COMMAND_NO_EVENT(pContextMonitor, command_queue);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clEnqueueWaitForEvents(command_queue, num_events, event_list);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    CS_AFTER_ENQUEUEING_COMMAND_NO_EVENT(pContextMonitor, command_queue, retVal,
                                         new apCLWaitForEventsCommand(num_events, event_list));

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueWaitForEvents);

    return retVal;
}
cl_int CL_API_CALL clEnqueueBarrier(cl_command_queue command_queue)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueBarrier);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueBarrier, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue);

    // Mark that a command is about to be added:

    csCommandQueueMonitor* pQueueMtr = cs_stat_openCLMonitorInstance.commandQueueMonitor((oaCLCommandQueueHandle)command_queue);
    GT_IF_WITH_ASSERT(pQueueMtr != NULL)
    {
        pQueueMtr->beforeCommandAddedToQueue();
    }

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clEnqueueBarrier(command_queue);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    CS_AFTER_ENQUEUEING_COMMAND_NO_EVENT(pContextMonitor, command_queue, retVal,
                                         new apCLBarrierCommand);

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueBarrier);

    return retVal;
}

void* CL_API_CALL clGetExtensionFunctionAddress(const char* func_name)
{
    void* retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clGetExtensionFunctionAddress);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall(OA_CL_NULL_HANDLE, ap_clGetExtensionFunctionAddress, 1, OS_TOBJ_ID_STRING_PARAMETER, func_name);

    // Try and get the wrapper address from our extensions manager (this should return the real pointer for unsupported functions):
    gtString extensionFunctionName;
    extensionFunctionName.fromASCIIString(func_name);
    retVal = (void*)(cs_stat_extensionsManager.wrapperFunctionAddress(extensionFunctionName));

    // If that didn't work for some reason, call the real function:
    if (retVal == NULL)
    {
        retVal = cs_stat_realFunctionPointers.clGetExtensionFunctionAddress(func_name);
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetExtensionFunctionAddress);

    return retVal;
}

////////////////
// OpenCL 2.0 //
////////////////
cl_command_queue CL_API_CALL clCreateCommandQueueWithProperties(cl_context context, cl_device_id device, const cl_queue_properties* properties, cl_int* errcode_ret)
{
    cl_command_queue retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateCommandQueueWithProperties);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLContextHandle)context, ap_clCreateCommandQueueWithProperties, 4, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_HANDLE_PARAMETER, device, OS_TOBJ_ID_CL_COMMAND_QUEUE_PROPERTIES_LIST_PARAMETER, properties, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:

    // The Mac implementation of clCreateCommandQueue calls clSetCommandQueueProperty internally:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetCommandQueueProperty);

    const cl_queue_properties* forcedProperties = properties;

    // TO_DO handle queue profiling mode as needed:
    /*
    int deviceIndex = cs_stat_openCLMonitorInstance.devicesMonitor().getDeviceObjectAPIID((oaCLDeviceID)device);

    if (cs_stat_openCLMonitorInstance.isCommandQueueProfileModeForcedForDevice(deviceIndex))
    {
        // If the queue has the CL_QUEUE_PROFILING_ENABLE flag disabled, raise it:
        forcedProperties |= CL_QUEUE_PROFILING_ENABLE;
    }
    */

    // Call the real function:
    if (cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER == cs_stat_pIKernelDebuggingManager->kernelDebuggerType()))
    {
        retVal = cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptCreateCommandQueueWithProperties(context, device, forcedProperties, errcode_ret);
    }
    else // !cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled()
    {
        retVal = cs_stat_realFunctionPointers.clCreateCommandQueueWithProperties(context, device, forcedProperties, errcode_ret);
    }

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetCommandQueueProperty);

    // If the queue was created successfully:
    if (retVal != NULL)
    {
        // Retain the queue, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainCommandQueue);
        cs_stat_realFunctionPointers.clRetainCommandQueue(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainCommandQueue);

        // Log the queue creation:
        cs_stat_openCLMonitorInstance.onCommandQueueCreationWithProperties(retVal, context, device, properties);
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateCommandQueueWithProperties);

    return retVal;
}

cl_mem CL_API_CALL clCreatePipe(cl_context context, cl_mem_flags flags, cl_uint pipe_packet_size, cl_uint pipe_max_packets, const cl_pipe_properties* properties, cl_int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreatePipe);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLContextHandle)context, ap_clCreatePipe, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags, OS_TOBJ_ID_CL_UINT_PARAMETER, pipe_packet_size, OS_TOBJ_ID_CL_UINT_PARAMETER, pipe_max_packets, OS_TOBJ_ID_CL_PIPE_PROPERTIES_LIST_PARAMETER, properties, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clCreatePipe(context, flags, pipe_packet_size, pipe_max_packets, properties, errcode_ret);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the pipe was created successfully:
    if (retVal != NULL)
    {
        // Retain the pipe, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);
        cs_stat_realFunctionPointers.clRetainMemObject(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);

        // Get the context monitor for the context that the buffer is created at:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor((oaCLContextHandle)context);

        if (pContextMonitor != NULL)
        {
            // Get the buffers monitor for this context:
            csImagesAndBuffersMonitor& texBuffersMonitor = pContextMonitor->imagesAndBuffersMonitor();
            texBuffersMonitor.onPipeCreation(retVal, flags, pipe_packet_size, pipe_max_packets);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreatePipe);

    return retVal;
}

cl_int CL_API_CALL clGetPipeInfo(cl_mem pipe, cl_pipe_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetPipeInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLMemHandle)pipe, ap_clGetPipeInfo, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, pipe, OS_TOBJ_ID_CL_ENUM_PARAMETER, param_name, OS_TOBJ_ID_SIZE_T_PARAMETER, param_value_size, OS_TOBJ_ID_POINTER_PARAMETER, param_value, OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clGetPipeInfo(pipe, param_name, param_value_size, param_value, param_value_size_ret);

    SU_END_FUNCTION_WRAPPER(ap_clGetPipeInfo);

    return retVal;
}


void* CL_API_CALL clSVMAlloc(cl_context context, cl_svm_mem_flags flags, size_t size, cl_uint alignment)
{
    void* retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clSVMAlloc);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLContextHandle)context, ap_clSVMAlloc, 4, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_SVM_MEM_FLAGS_PARAMETER, flags, OS_TOBJ_ID_SIZE_T_PARAMETER, size, OS_TOBJ_ID_CL_UINT_PARAMETER, alignment);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clSVMAlloc(context, flags, size, alignment);

    SU_END_FUNCTION_WRAPPER(ap_clSVMAlloc);

    return retVal;
}

void CL_API_CALL clSVMFree(cl_context context, void* svm_pointer)
{
    SU_START_FUNCTION_WRAPPER(ap_clSVMFree);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLContextHandle)context, ap_clSVMFree, 2, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_POINTER_PARAMETER, svm_pointer);

    // Call the real function:
    cs_stat_realFunctionPointers.clSVMFree(context, svm_pointer);

    SU_END_FUNCTION_WRAPPER(ap_clSVMFree);
}

cl_sampler CL_API_CALL clCreateSamplerWithProperties(cl_context context, const cl_sampler_properties* normalized_coords, cl_int* errcode_ret)
{
    cl_sampler retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateSamplerWithProperties);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLContextHandle)context, ap_clCreateSamplerWithProperties, 3, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_SAMPLER_PROPERTIES_LIST_PARAMETER, normalized_coords, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clCreateSamplerWithProperties(context, normalized_coords, errcode_ret);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    if (retVal != NULL)
    {
        // Retain the sampler, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainSampler);
        cs_stat_realFunctionPointers.clRetainSampler(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainSampler);

        cs_stat_openCLMonitorInstance.onSamplerCreationWithProperties(context, retVal, normalized_coords);
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateSamplerWithProperties);

    return retVal;
}


cl_int CL_API_CALL clSetKernelArgSVMPointer(cl_kernel kernel, cl_uint arg_index, const void* arg_value)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clSetKernelArgSVMPointer);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)kernel, ap_clSetKernelArgSVMPointer, 3, OS_TOBJ_ID_CL_HANDLE_PARAMETER, kernel, OS_TOBJ_ID_CL_UINT_PARAMETER, arg_index, OS_TOBJ_ID_POINTER_PARAMETER, arg_value);

    // Call the real function:
    cl_kernel kernelInternalHandle = cs_stat_openCLMonitorInstance.internalKernelHandleFromExternalHandle(kernel);

    if (cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER == cs_stat_pIKernelDebuggingManager->kernelDebuggerType()))
    {
        // Uri, 17/12/13 - This is not supported for the software debugger!
        // retVal = cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptSetKernelArgSVMPointer(kernelInternalHandle, arg_index, arg_value);

        OS_OUTPUT_DEBUG_LOG(L"Attempting to set SVM pointer to kernel managed by the software kernel debugging manager. This will fail the kernel debugging or have unexpected results if attempted.", OS_DEBUG_LOG_INFO);
        retVal = cs_stat_realFunctionPointers.clSetKernelArgSVMPointer(kernelInternalHandle, arg_index, arg_value);
    }
    else // !cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled()
    {
        retVal = cs_stat_realFunctionPointers.clSetKernelArgSVMPointer(kernelInternalHandle, arg_index, arg_value);
    }

    if (retVal == CL_SUCCESS)
    {
        // Mark the kernel argument value:
        cs_stat_openCLMonitorInstance.onKernelArgumentSet(kernel, arg_index, sizeof(const void*), &arg_value, true);
    }
    else // retVal != CL_SUCCESS
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clSetKernelArgSVMPointer);

    return retVal;
}

cl_int CL_API_CALL clSetKernelExecInfo(cl_kernel kernel, cl_kernel_exec_info param_name, size_t param_value_size, const void* param_value)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clSetKernelExecInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)kernel, ap_clSetKernelExecInfo, 4, OS_TOBJ_ID_CL_HANDLE_PARAMETER, kernel, OS_TOBJ_ID_CL_ENUM_PARAMETER, param_name, OS_TOBJ_ID_SIZE_T_PARAMETER, param_value_size, OS_TOBJ_ID_POINTER_PARAMETER, param_value);

    // Call the real function:
    cl_kernel kernelInternalHandle = cs_stat_openCLMonitorInstance.internalKernelHandleFromExternalHandle(kernel);
    retVal = cs_stat_realFunctionPointers.clSetKernelExecInfo(kernelInternalHandle, param_name, param_value_size, param_value);

    if (retVal == CL_SUCCESS)
    {
        // If this sets values to notify the kernel is using SVM, mark the kernel as not debuggable by the software debugger:
        if (((CL_KERNEL_EXEC_INFO_SVM_PTRS == param_name) && (0 < param_value_size)) ||
            ((CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM == param_name) && (CL_TRUE == *((const cl_bool*)param_value))))
        {
            // Mark the kernel as using SVM:
            cs_stat_openCLMonitorInstance.onKernelArgumentSet(kernel, ((cl_uint) - 1), 0, NULL, true);
        }
    }
    else // retVal != CL_SUCCESS
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clSetKernelExecInfo);

    return retVal;
}

cl_int CL_API_CALL clEnqueueSVMFree(cl_command_queue command_queue, cl_uint num_svm_pointers, void* svm_pointers[], void (CL_CALLBACK* pfn_free_func)(cl_command_queue queue, cl_uint num_svm_pointers, void* svm_pointers[], void* user_data), void* user_data, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueSVMFree);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueSVMFree, 8, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_UINT_PARAMETER, num_svm_pointers,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_POINTER_PARAMETER, num_svm_pointers, svm_pointers, OS_TOBJ_ID_POINTER_PARAMETER, pfn_free_func, OS_TOBJ_ID_POINTER_PARAMETER, user_data,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueSVMFree, 8, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_UINT_PARAMETER, num_svm_pointers,
                                                      OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_POINTER_PARAMETER, num_svm_pointers, svm_pointers, OS_TOBJ_ID_POINTER_PARAMETER, pfn_free_func, OS_TOBJ_ID_POINTER_PARAMETER, user_data,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Mark that a command is about to be added:
    CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clEnqueueSVMFree(command_queue, num_svm_pointers, svm_pointers, pfn_free_func, user_data, num_events_in_wait_list, event_wait_list, pEvent);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                new apCLSVMFreeCommand(num_svm_pointers, svm_pointers, pfn_free_func, user_data, num_events_in_wait_list, event_wait_list, event));

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueSVMFree);

    return retVal;
}

cl_int CL_API_CALL clEnqueueSVMMemcpy(cl_command_queue command_queue, cl_bool blocking_copy, void* dst_ptr, const void* src_ptr, size_t size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueSVMMemcpy);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueSVMMemcpy, 8, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_copy,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, dst_ptr, OS_TOBJ_ID_POINTER_PARAMETER, src_ptr, OS_TOBJ_ID_SIZE_T_PARAMETER, size,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueSVMMemcpy, 8, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_copy,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, dst_ptr, OS_TOBJ_ID_POINTER_PARAMETER, src_ptr, OS_TOBJ_ID_SIZE_T_PARAMETER, size,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Mark that a command is about to be added:
    CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clEnqueueSVMMemcpy(command_queue, blocking_copy, dst_ptr, src_ptr, size, num_events_in_wait_list, event_wait_list, pEvent);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                new apCLSVMMemcpyCommand(blocking_copy, dst_ptr, src_ptr, size, num_events_in_wait_list, event_wait_list, event));

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueSVMMemcpy);

    return retVal;
}

cl_int CL_API_CALL clEnqueueSVMMemFill(cl_command_queue command_queue, void* svm_ptr, const void* pattern, size_t pattern_size, size_t size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueSVMMemFill);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueSVMMemFill, 8, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_POINTER_PARAMETER, svm_ptr,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, pattern, OS_TOBJ_ID_SIZE_T_PARAMETER, pattern_size, OS_TOBJ_ID_SIZE_T_PARAMETER, size,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueSVMMemFill, 8, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_POINTER_PARAMETER, svm_ptr,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, pattern, OS_TOBJ_ID_SIZE_T_PARAMETER, pattern_size, OS_TOBJ_ID_SIZE_T_PARAMETER, size,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Mark that a command is about to be added:
    CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clEnqueueSVMMemFill(command_queue, svm_ptr, pattern, pattern_size, size, num_events_in_wait_list, event_wait_list, pEvent);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                new apCLSVMMemFillCommand(svm_ptr, pattern, pattern_size, size, num_events_in_wait_list, event_wait_list, event));

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueSVMMemFill);

    return retVal;
}

cl_int CL_API_CALL clEnqueueSVMMap(cl_command_queue command_queue, cl_bool blocking_map, cl_map_flags flags, void* svm_ptr, size_t size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueSVMMap);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueSVMMap, 8, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_map,
                                                      OS_TOBJ_ID_CL_MAP_FLAGS_PARAMETER, flags, OS_TOBJ_ID_POINTER_PARAMETER, svm_ptr, OS_TOBJ_ID_SIZE_T_PARAMETER, size,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueSVMMap, 8, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_BOOL_PARAMETER, blocking_map,
                                                      OS_TOBJ_ID_CL_MAP_FLAGS_PARAMETER, flags, OS_TOBJ_ID_POINTER_PARAMETER, svm_ptr, OS_TOBJ_ID_SIZE_T_PARAMETER, size,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Mark that a command is about to be added:
    CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clEnqueueSVMMap(command_queue, blocking_map, flags, svm_ptr, size, num_events_in_wait_list, event_wait_list, pEvent);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                new apCLSVMMapCommand(blocking_map, flags, svm_ptr, size, num_events_in_wait_list, event_wait_list, event));

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueSVMMap);

    return retVal;
}

cl_int CL_API_CALL clEnqueueSVMUnmap(cl_command_queue command_queue, void* svm_ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueSVMUnmap);

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueSVMUnmap, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_POINTER_PARAMETER, svm_ptr,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clEnqueueSVMUnmap, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_POINTER_PARAMETER, svm_ptr,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Mark that a command is about to be added:
    CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

    // Call the real function:
    retVal = cs_stat_realFunctionPointers.clEnqueueSVMUnmap(command_queue, svm_ptr, num_events_in_wait_list, event_wait_list, pEvent);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                new apCLSVMUnmapCommand(svm_ptr, num_events_in_wait_list, event_wait_list, event));

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueSVMUnmap);

    return retVal;
}

