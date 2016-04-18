//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csDirectXIntegrationWrappers.cpp
///
//==================================================================================

//------------------------------ csDirectXIntegrationWrappers.cpp------------------------------


// ------------------------------------------------------------------------
// File:
// This file contains a wrapper function for the DirectX (9, 10 and 11) integration
// OpenCL functions
// ------------------------------------------------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osTransferableObjectType.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/csGlobalVariables.h>
#include <src/csOpenCLMonitor.h>
#include <src/csOpenCLWrappersAidMacros.h>
#include <src/csMonitoredFunctionPointers.h>

// NOTICE: This include must come after csGlobalVariables include since it uses variables defined in csGlobalVariables:
#include <AMDTServerUtilities/Include/suInterceptionMacros.h>

// DX9 sharing:
cl_int CL_API_CALL clGetDeviceIDsFromDX9MediaAdapterKHR(cl_platform_id platform, cl_uint num_media_adapters, cl_dx9_media_adapter_type_khr* media_adapter_type, void* media_adapters, cl_dx9_media_adapter_set_khr media_adapter_set, cl_uint num_entries, cl_device_id* devices, cl_uint* num_devices)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetDeviceIDsFromDX9MediaAdapterKHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall(OA_CL_NULL_HANDLE, ap_clGetDeviceIDsFromDX9MediaAdapterKHR, 8, OS_TOBJ_ID_CL_HANDLE_PARAMETER, platform, OS_TOBJ_ID_CL_UINT_PARAMETER, num_media_adapters, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_UINT_PARAMETER, num_media_adapters, media_adapter_type, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_POINTER_PARAMETER, num_media_adapters, media_adapters, OS_TOBJ_ID_CL_ENUM_PARAMETER, media_adapter_set, OS_TOBJ_ID_CL_UINT_PARAMETER, num_entries, OS_TOBJ_ID_POINTER_PARAMETER, devices, OS_TOBJ_ID_CL_P_UINT_PARAMETER, num_devices);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clGetDeviceIDsFromDX9MediaAdapterKHR, (platform, num_media_adapters, media_adapter_type, media_adapters, media_adapter_set, num_entries, devices, num_devices), retVal);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetDeviceIDsFromDX9MediaAdapterKHR);

    return retVal;
}

cl_mem CL_API_CALL clCreateFromDX9MediaSurfaceKHR(cl_context context, cl_mem_flags flags, cl_dx9_media_adapter_type_khr adapter_type, void* surface_info, cl_uint plane, cl_int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateFromDX9MediaSurfaceKHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLContextHandle)context, ap_clCreateFromDX9MediaSurfaceKHR, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags, OS_TOBJ_ID_CL_ENUM_PARAMETER, adapter_type, OS_TOBJ_ID_POINTER_PARAMETER, surface_info, OS_TOBJ_ID_CL_UINT_PARAMETER, plane, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clCreateFromDX9MediaSurfaceKHR, (context, flags, adapter_type, surface_info, plane, errcode_ret), retVal);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the image was created successfully:
    if (retVal != NULL)
    {
        // Retain the image, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);
        cs_stat_realFunctionPointers.clRetainMemObject(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);

        // Get the context monitor for the context that the image is created at:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor((oaCLContextHandle)context);

        if (pContextMonitor != NULL)
        {
            // Get the image and buffer monitor for this context:
            csImagesAndBuffersMonitor& texturesMonitor = pContextMonitor->imagesAndBuffersMonitor();
            texturesMonitor.onImageCreationFromDirectX(retVal, flags, AP_2D_TEXTURE);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateFromDX9MediaSurfaceKHR);

    return retVal;
}


cl_int CL_API_CALL clEnqueueAcquireDX9MediaSurfacesKHR(cl_command_queue command_queue, cl_uint num_objects, const cl_mem* mem_objects, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueAcquireDX9MediaSurfacesKHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLCommandQueueHandle)command_queue, ap_clEnqueueAcquireDX9MediaSurfacesKHR, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_UINT_PARAMETER, num_objects, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_objects, mem_objects, OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clEnqueueAcquireDX9MediaSurfacesKHR, (command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event), retVal);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueAcquireDX9MediaSurfacesKHR);

    return retVal;
}


cl_int CL_API_CALL clEnqueueReleaseDX9MediaSurfacesKHR(cl_command_queue command_queue, cl_uint num_objects, const cl_mem* mem_objects, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueReleaseDX9MediaSurfacesKHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLCommandQueueHandle)command_queue, ap_clEnqueueReleaseDX9MediaSurfacesKHR, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_UINT_PARAMETER, num_objects, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_objects, mem_objects, OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clEnqueueReleaseDX9MediaSurfacesKHR, (command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event), retVal);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueReleaseDX9MediaSurfacesKHR);

    return retVal;
}


// D3D10 sharing:

cl_int CL_API_CALL clGetDeviceIDsFromD3D10KHR(cl_platform_id platform, cl_d3d10_device_source_khr d3d_device_source, void* d3d_object, cl_d3d10_device_set_khr d3d_device_set, cl_uint num_entries, cl_device_id* devices, cl_uint* num_devices)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetDeviceIDsFromD3D10KHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall(OA_CL_NULL_HANDLE, ap_clGetDeviceIDsFromD3D10KHR, 7, OS_TOBJ_ID_CL_HANDLE_PARAMETER, platform, OS_TOBJ_ID_CL_ENUM_PARAMETER, d3d_device_source, OS_TOBJ_ID_POINTER_PARAMETER, d3d_object, OS_TOBJ_ID_CL_ENUM_PARAMETER, d3d_device_set, OS_TOBJ_ID_CL_UINT_PARAMETER, num_entries, OS_TOBJ_ID_POINTER_PARAMETER, devices, OS_TOBJ_ID_CL_P_UINT_PARAMETER, num_devices);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clGetDeviceIDsFromD3D10KHR, (platform, d3d_device_source, d3d_object, d3d_device_set, num_entries, devices, num_devices), retVal);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetDeviceIDsFromD3D10KHR);

    return retVal;
}


cl_mem CL_API_CALL clCreateFromD3D10BufferKHR(cl_context context, cl_mem_flags flags, ID3D10Buffer* resource, cl_int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateFromD3D10BufferKHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLContextHandle)context, ap_clCreateFromD3D10BufferKHR, 4, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags, OS_TOBJ_ID_POINTER_PARAMETER, resource, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clCreateFromD3D10BufferKHR, (context, flags, resource, errcode_ret), retVal);

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
            texBuffersMonitor.onBufferCreationFromDirectX(retVal, flags);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateFromD3D10BufferKHR);

    return retVal;
}


cl_mem CL_API_CALL clCreateFromD3D10Texture2DKHR(cl_context context, cl_mem_flags flags, ID3D10Texture2D* resource, UINT subresource, cl_int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateFromD3D10Texture2DKHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLContextHandle)context, ap_clCreateFromD3D10Texture2DKHR, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags, OS_TOBJ_ID_POINTER_PARAMETER, resource, OS_TOBJ_ID_WIN32_UINT_PARAMETER, subresource, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clCreateFromD3D10Texture2DKHR, (context, flags, resource, subresource, errcode_ret), retVal);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the image was created successfully:
    if (retVal != NULL)
    {
        // Retain the image, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);
        cs_stat_realFunctionPointers.clRetainMemObject(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);

        // Get the context monitor for the context that the image is created at:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor((oaCLContextHandle)context);

        if (pContextMonitor != NULL)
        {
            // Get the image and buffer monitor for this context:
            csImagesAndBuffersMonitor& texturesMonitor = pContextMonitor->imagesAndBuffersMonitor();
            texturesMonitor.onImageCreationFromDirectX(retVal, flags, AP_2D_TEXTURE);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateFromD3D10Texture2DKHR);

    return retVal;
}


cl_mem CL_API_CALL clCreateFromD3D10Texture3DKHR(cl_context context, cl_mem_flags flags, ID3D10Texture3D* resource, UINT subresource, cl_int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateFromD3D10Texture3DKHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLContextHandle)context, ap_clCreateFromD3D10Texture3DKHR, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags, OS_TOBJ_ID_POINTER_PARAMETER, resource, OS_TOBJ_ID_WIN32_UINT_PARAMETER, subresource, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clCreateFromD3D10Texture3DKHR, (context, flags, resource, subresource, errcode_ret), retVal);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the image was created successfully:
    if (retVal != NULL)
    {
        // Retain the image, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);
        cs_stat_realFunctionPointers.clRetainMemObject(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);

        // Get the context monitor for the context that the image is created at:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor((oaCLContextHandle)context);

        if (pContextMonitor != NULL)
        {
            // Get the image and buffer monitor for this context:
            csImagesAndBuffersMonitor& texturesMonitor = pContextMonitor->imagesAndBuffersMonitor();
            texturesMonitor.onImageCreationFromDirectX(retVal, flags, AP_3D_TEXTURE);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateFromD3D10Texture3DKHR);

    return retVal;
}


cl_int CL_API_CALL clEnqueueAcquireD3D10ObjectsKHR(cl_command_queue command_queue, cl_uint num_objects, const cl_mem* mem_objects, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueAcquireD3D10ObjectsKHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLCommandQueueHandle)command_queue, ap_clEnqueueAcquireD3D10ObjectsKHR, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_UINT_PARAMETER, num_objects, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_objects, mem_objects, OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clEnqueueAcquireD3D10ObjectsKHR, (command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event), retVal);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueAcquireD3D10ObjectsKHR);

    return retVal;
}


cl_int CL_API_CALL clEnqueueReleaseD3D10ObjectsKHR(cl_command_queue command_queue, cl_uint num_objects, const cl_mem* mem_objects, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueReleaseD3D10ObjectsKHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLCommandQueueHandle)command_queue, ap_clEnqueueReleaseD3D10ObjectsKHR, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_UINT_PARAMETER, num_objects, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_objects, mem_objects, OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clEnqueueReleaseD3D10ObjectsKHR, (command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event), retVal);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueReleaseD3D10ObjectsKHR);

    return retVal;
}


// D3D11 sharing:

cl_int CL_API_CALL clGetDeviceIDsFromD3D11KHR(cl_platform_id platform, cl_d3d11_device_source_khr d3d_device_source, void* d3d_object, cl_d3d11_device_set_khr d3d_device_set, cl_uint num_entries, cl_device_id* devices, cl_uint* num_devices)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetDeviceIDsFromD3D11KHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall(OA_CL_NULL_HANDLE, ap_clGetDeviceIDsFromD3D11KHR, 7, OS_TOBJ_ID_CL_HANDLE_PARAMETER, platform, OS_TOBJ_ID_CL_ENUM_PARAMETER, d3d_device_source, OS_TOBJ_ID_POINTER_PARAMETER, d3d_object, OS_TOBJ_ID_CL_ENUM_PARAMETER, d3d_device_set, OS_TOBJ_ID_CL_UINT_PARAMETER, num_entries, OS_TOBJ_ID_POINTER_PARAMETER, devices, OS_TOBJ_ID_CL_P_UINT_PARAMETER, num_devices);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clGetDeviceIDsFromD3D11KHR, (platform, d3d_device_source, d3d_object, d3d_device_set, num_entries, devices, num_devices), retVal);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetDeviceIDsFromD3D11KHR);

    return retVal;
}


cl_mem CL_API_CALL clCreateFromD3D11BufferKHR(cl_context context, cl_mem_flags flags, ID3D11Buffer* resource, cl_int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateFromD3D11BufferKHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLContextHandle)context, ap_clCreateFromD3D11BufferKHR, 4, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags, OS_TOBJ_ID_POINTER_PARAMETER, resource, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clCreateFromD3D11BufferKHR, (context, flags, resource, errcode_ret), retVal);

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
            texBuffersMonitor.onBufferCreationFromDirectX(retVal, flags);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateFromD3D11BufferKHR);

    return retVal;
}


cl_mem CL_API_CALL clCreateFromD3D11Texture2DKHR(cl_context context, cl_mem_flags flags, ID3D11Texture2D* resource, UINT subresource, cl_int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateFromD3D11Texture2DKHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLContextHandle)context, ap_clCreateFromD3D11Texture2DKHR, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags, OS_TOBJ_ID_POINTER_PARAMETER, resource, OS_TOBJ_ID_WIN32_UINT_PARAMETER, subresource, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clCreateFromD3D11Texture2DKHR, (context, flags, resource, subresource, errcode_ret), retVal);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the image was created successfully:
    if (retVal != NULL)
    {
        // Retain the image, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);
        cs_stat_realFunctionPointers.clRetainMemObject(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);

        // Get the context monitor for the context that the image is created at:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor((oaCLContextHandle)context);

        if (pContextMonitor != NULL)
        {
            // Get the image and buffer monitor for this context:
            csImagesAndBuffersMonitor& texturesMonitor = pContextMonitor->imagesAndBuffersMonitor();
            texturesMonitor.onImageCreationFromDirectX(retVal, flags, AP_2D_TEXTURE);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateFromD3D11Texture2DKHR);

    return retVal;
}


cl_mem CL_API_CALL clCreateFromD3D11Texture3DKHR(cl_context context, cl_mem_flags flags, ID3D11Texture3D* resource, UINT subresource, cl_int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateFromD3D11Texture3DKHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLContextHandle)context, ap_clCreateFromD3D11Texture3DKHR, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags, OS_TOBJ_ID_POINTER_PARAMETER, resource, OS_TOBJ_ID_WIN32_UINT_PARAMETER, subresource, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clCreateFromD3D11Texture3DKHR, (context, flags, resource, subresource, errcode_ret), retVal);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the image was created successfully:
    if (retVal != NULL)
    {
        // Retain the image, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);
        cs_stat_realFunctionPointers.clRetainMemObject(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);

        // Get the context monitor for the context that the image is created at:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor((oaCLContextHandle)context);

        if (pContextMonitor != NULL)
        {
            // Get the image and buffer monitor for this context:
            csImagesAndBuffersMonitor& texturesMonitor = pContextMonitor->imagesAndBuffersMonitor();
            texturesMonitor.onImageCreationFromDirectX(retVal, flags, AP_3D_TEXTURE);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_clCreateFromD3D11Texture3DKHR);

    return retVal;
}


cl_int CL_API_CALL clEnqueueAcquireD3D11ObjectsKHR(cl_command_queue command_queue, cl_uint num_objects, const cl_mem* mem_objects, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueAcquireD3D11ObjectsKHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLCommandQueueHandle)command_queue, ap_clEnqueueAcquireD3D11ObjectsKHR, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_UINT_PARAMETER, num_objects, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_objects, mem_objects, OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clEnqueueAcquireD3D11ObjectsKHR, (command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event), retVal);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueAcquireD3D11ObjectsKHR);

    return retVal;
}


cl_int CL_API_CALL clEnqueueReleaseD3D11ObjectsKHR(cl_command_queue command_queue, cl_uint num_objects, const cl_mem* mem_objects, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueReleaseD3D11ObjectsKHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLCommandQueueHandle)command_queue, ap_clEnqueueReleaseD3D11ObjectsKHR, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_UINT_PARAMETER, num_objects, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_objects, mem_objects, OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clEnqueueReleaseD3D11ObjectsKHR, (command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event), retVal);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clEnqueueReleaseD3D11ObjectsKHR);

    return retVal;
}

