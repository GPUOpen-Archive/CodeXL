//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csOpenGLIntegrationWrappers.cpp
///
//==================================================================================

//------------------------------ csOpenGLIntegrationWrappers.cpp------------------------------


// ------------------------------------------------------------------------
// File:
// This file contains a wrapper function for the OpenGL integration
// OpenCL functions
// ------------------------------------------------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osTransferableObjectType.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/csGlobalVariables.h>
#include <src/csOpenCLMonitor.h>
#include <src/csOpenCLWrappersAidMacros.h>
#include <src/csMonitoredFunctionPointers.h>

// NOTICE: This include must come after csGlobalVariables include since it uses variables defined in csGlobalVariables:
#include <AMDTServerUtilities/Include/suInterceptionMacros.h>


// TO_DO: GL objects parameters
cl_mem CL_API_CALL clCreateFromGLBuffer(cl_context context, cl_mem_flags flags, GLuint bufobj, int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateFromGLBuffer);
    su_stat_interoperabilityHelper.onNestedFunctionEntered();

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle) context, ap_clCreateFromGLBuffer, 4, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER,
                                                  flags, OS_TOBJ_ID_GL_UINT_PARAMETER, bufobj, OS_TOBJ_ID_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clCreateFromGLBuffer, (context, flags, bufobj, errcode_ret), retVal);

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
            texBuffersMonitor.onBufferCreationFromGLVBO(retVal, flags, bufobj);
        }
    }

    su_stat_interoperabilityHelper.onNestedFunctionExited();
    SU_END_FUNCTION_WRAPPER(ap_clCreateFromGLBuffer);

    return retVal;
}
cl_mem CL_API_CALL clCreateFromGLTexture(cl_context context, cl_mem_flags flags, GLenum target, GLint miplevel, GLuint texture, cl_int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateFromGLTexture);
    su_stat_interoperabilityHelper.onNestedFunctionEntered();

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle) context, ap_clCreateFromGLTexture, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags, OS_TOBJ_ID_GL_ENUM_PARAMETER,
                                                  target, OS_TOBJ_ID_GL_INT_PARAMETER, miplevel, OS_TOBJ_ID_GL_UINT_PARAMETER, texture, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clCreateFromGLTexture, (context, flags, target, miplevel, texture, errcode_ret), retVal);

    if (NULL != errcode_ret)
    {
        if (CL_SUCCESS != *errcode_ret)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the texture was created successfully:
    if (NULL != retVal)
    {
        // Retain the image, so it won't get deleted without us knowing:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);
        cs_stat_realFunctionPointers.clRetainMemObject(retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainMemObject);

        // Get the context monitor for the context that the texture is created at:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor((oaCLContextHandle)context);

        if (NULL != pContextMonitor)
        {
            // Get the texture monitor for this context:
            csImagesAndBuffersMonitor& imagesMonitor = pContextMonitor->imagesAndBuffersMonitor();
            imagesMonitor.onImageCreationFromGLTexture(retVal, flags, target, miplevel, texture);
        }
    }

    su_stat_interoperabilityHelper.onNestedFunctionExited();
    SU_END_FUNCTION_WRAPPER(ap_clCreateFromGLTexture);

    return retVal;
}
cl_mem CL_API_CALL clCreateFromGLTexture2D(cl_context context, cl_mem_flags flags, GLenum target, GLint miplevel, GLuint texture, cl_int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateFromGLTexture2D);
    su_stat_interoperabilityHelper.onNestedFunctionEntered();

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle) context, ap_clCreateFromGLTexture2D, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags, OS_TOBJ_ID_GL_ENUM_PARAMETER,
                                                  target, OS_TOBJ_ID_GL_INT_PARAMETER, miplevel, OS_TOBJ_ID_GL_UINT_PARAMETER, texture, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clCreateFromGLTexture2D, (context, flags, target, miplevel, texture, errcode_ret), retVal);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the texture was created successfully:
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
            texturesMonitor.onImageCreationFromGL2DTexture(retVal, flags, AP_2D_TEXTURE, target, miplevel, texture);
        }
    }

    su_stat_interoperabilityHelper.onNestedFunctionExited();
    SU_END_FUNCTION_WRAPPER(ap_clCreateFromGLTexture2D);

    return retVal;
}
cl_mem CL_API_CALL clCreateFromGLTexture3D(cl_context context, cl_mem_flags flags, GLenum target, GLint miplevel, GLuint texture, cl_int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateFromGLTexture3D);
    su_stat_interoperabilityHelper.onNestedFunctionEntered();

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle) context, ap_clCreateFromGLTexture3D, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags, OS_TOBJ_ID_GL_ENUM_PARAMETER, target,
                                                  OS_TOBJ_ID_GL_INT_PARAMETER, miplevel, OS_TOBJ_ID_GL_UINT_PARAMETER, texture, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clCreateFromGLTexture3D, (context, flags, target, miplevel, texture, errcode_ret), retVal);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the texture was created successfully:
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
            texturesMonitor.onTextureCreationFromGL3DTexture(retVal, flags, AP_3D_TEXTURE, target, miplevel, texture);
        }
    }

    su_stat_interoperabilityHelper.onNestedFunctionExited();
    SU_END_FUNCTION_WRAPPER(ap_clCreateFromGLTexture3D);

    return retVal;
}
cl_mem CL_API_CALL clCreateFromGLRenderbuffer(cl_context context, cl_mem_flags flags, GLuint renderbuffer, cl_int* errcode_ret)
{
    cl_mem retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateFromGLRenderbuffer);
    su_stat_interoperabilityHelper.onNestedFunctionEntered();

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle) context, ap_clCreateFromGLRenderbuffer, 4, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER, flags,
                                                  OS_TOBJ_ID_GL_INT_PARAMETER, renderbuffer, OS_TOBJ_ID_CL_P_INT_PARAMETER, errcode_ret);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clCreateFromGLRenderbuffer, (context, flags, renderbuffer, errcode_ret), retVal);

    if (errcode_ret != NULL)
    {
        if (*errcode_ret != CL_SUCCESS)
        {
            // Handle OpenCL error:
            cs_stat_openCLMonitorInstance.onOpenCLError(*errcode_ret);
        }
    }

    // If the texture was created successfully:
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
            texturesMonitor.onTextureCreationFromGLRenderbuffer(retVal, flags, AP_2D_TEXTURE, renderbuffer);
        }
    }

    su_stat_interoperabilityHelper.onNestedFunctionExited();
    SU_END_FUNCTION_WRAPPER(ap_clCreateFromGLRenderbuffer);

    return retVal;
}
cl_int CL_API_CALL clGetGLObjectInfo(cl_mem memobj, cl_gl_object_type* gl_object_type, GLuint* gl_object_name)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetGLObjectInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle) memobj, ap_clGetGLObjectInfo, 3, OS_TOBJ_ID_CL_HANDLE_PARAMETER, memobj, OS_TOBJ_ID_CL_GL_OBJECT_TYPE_PARAMETER, gl_object_type, OS_TOBJ_ID_GL_UINT_PARAMETER, gl_object_name);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clGetGLObjectInfo, (memobj, gl_object_type, gl_object_name), retVal);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetGLObjectInfo);

    return retVal;

}
cl_int CL_API_CALL clGetGLTextureInfo(cl_mem memobj, cl_gl_texture_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetGLTextureInfo);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle) memobj, ap_clGetGLTextureInfo, 5, OS_TOBJ_ID_CL_HANDLE_PARAMETER, memobj, OS_TOBJ_ID_CL_GL_TEXTURE_INFO_PARAMETER, param_name,
                                                  OS_TOBJ_ID_SIZE_T_PARAMETER, param_value_size, OS_TOBJ_ID_POINTER_PARAMETER, param_value, OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size_ret);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clGetGLTextureInfo, (memobj, param_name, param_value_size, param_value, param_value_size_ret), retVal);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetGLTextureInfo);

    return retVal;
}
cl_int CL_API_CALL clEnqueueAcquireGLObjects(cl_command_queue command_queue, cl_uint num_objects, const cl_mem* mem_objects, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueAcquireGLObjects);
    su_stat_interoperabilityHelper.onNestedFunctionEntered();

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle) command_queue, ap_clEnqueueAcquireGLObjects, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_UINT_PARAMETER, num_objects, OS_TOBJ_ID_POINTER_PARAMETER, mem_objects,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle) command_queue, ap_clEnqueueAcquireGLObjects, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_UINT_PARAMETER, num_objects, OS_TOBJ_ID_POINTER_PARAMETER, mem_objects,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Mark that a command is about to be added:
    CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clEnqueueAcquireGLObjects, (command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, pEvent), retVal);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                new apCLAcquireGLObjectsCommand(num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event));

    su_stat_interoperabilityHelper.onNestedFunctionExited();
    SU_END_FUNCTION_WRAPPER(ap_clEnqueueAcquireGLObjects);

    return retVal;
}
cl_int CL_API_CALL clEnqueueReleaseGLObjects(cl_command_queue command_queue, cl_uint num_objects, const cl_mem* mem_objects, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clEnqueueReleaseGLObjects);
    su_stat_interoperabilityHelper.onNestedFunctionEntered();

    // Log the call to this function:
    if ((event_wait_list != NULL) && (num_events_in_wait_list > 0))
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle) command_queue, ap_clEnqueueReleaseGLObjects, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_UINT_PARAMETER, num_objects, OS_TOBJ_ID_POINTER_PARAMETER, mem_objects,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_CL_HANDLE_PARAMETER, num_events_in_wait_list, event_wait_list,
                                                      OS_TOBJ_ID_POINTER_PARAMETER, event);
    }
    else // (event_wait_list == NULL) || (num_events_in_wait_list <= 0)
    {
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle) command_queue, ap_clEnqueueReleaseGLObjects, 6, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_CL_UINT_PARAMETER, num_objects, OS_TOBJ_ID_POINTER_PARAMETER, mem_objects,
                                                      OS_TOBJ_ID_CL_UINT_PARAMETER, num_events_in_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event_wait_list, OS_TOBJ_ID_POINTER_PARAMETER, event);
    }

    // Mark that a command is about to be added:
    CS_BEFORE_ENQUEUEING_COMMAND(pEvent, event, wasEventCreatedBySpy, pContextMonitor, command_queue);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clEnqueueReleaseGLObjects, (command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, pEvent), retVal);

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    CS_AFTER_ENQUEUEING_COMMAND(pEvent, wasEventCreatedBySpy, pContextMonitor, command_queue, retVal,
                                new apCLReleaseGLObjectsCommand(num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event));

    su_stat_interoperabilityHelper.onNestedFunctionExited();
    SU_END_FUNCTION_WRAPPER(ap_clEnqueueReleaseGLObjects);

    return retVal;
}


cl_int CL_API_CALL clGetGLContextInfoKHR(const cl_context_properties* properties, cl_gl_context_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    cl_int retVal = CL_INVALID_VALUE;

    SU_START_FUNCTION_WRAPPER(ap_clGetGLContextInfoKHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall(OA_CL_NULL_HANDLE, ap_clGetGLContextInfoKHR, 5, OS_TOBJ_ID_CL_CONTEXT_PROPERTIES_LIST_PARAMETER, properties, OS_TOBJ_ID_CL_ENUM_PARAMETER, param_name,
                                                  OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size, OS_TOBJ_ID_POINTER_PARAMETER, param_value, OS_TOBJ_ID_P_SIZE_T_PARAMETER, param_value_size_ret);

    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clGetGLContextInfoKHR, (properties, param_name, param_value_size, param_value, param_value_size_ret), retVal)

    if (retVal != CL_SUCCESS)
    {
        // Handle OpenCL error:
        cs_stat_openCLMonitorInstance.onOpenCLError(retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_clGetGLContextInfoKHR);

    return retVal;
}

//////////////////////////////////////////////////////////////////////////
// cl_khr_gl_event
//////////////////////////////////////////////////////////////////////////
cl_event CL_API_CALL clCreateEventFromGLsyncKHR(cl_context context, cl_GLsync sync, cl_int* errcode_ret)
{
    cl_event retVal = NULL;

    SU_START_FUNCTION_WRAPPER(ap_clCreateEventFromGLsyncKHR);

    // Log the call to this function:
    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clCreateEventFromGLsyncKHR, 3, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_CL_HANDLE_PARAMETER, sync, OS_TOBJ_ID_CL_UINT_PARAMETER, errcode_ret);

    // Call the real function:
    SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(clCreateEventFromGLsyncKHR, (context, sync, errcode_ret), retVal);

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

    SU_END_FUNCTION_WRAPPER(ap_clCreateEventFromGLsyncKHR);

    return retVal;
}


