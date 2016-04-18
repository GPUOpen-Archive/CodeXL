//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file defines the OpenCL function enum values as well as few utility functions
//==============================================================================

#ifndef _CL_FUNCTION_ENUM_DEFS_H_
#define _CL_FUNCTION_ENUM_DEFS_H_

#include <string>

/// OpenCL API type
enum CLAPIType
{
    CL_API = 0x1,
    CL_ENQUEUE_BASE_API = 0x2,
    CL_ENQUEUE_MEM = 0x7,
    CL_ENQUEUE_KERNEL = 0xB,
    CL_ENQUEUE_OTHER_OPERATIONS = 0x13,
    CL_ENQUEUE_DATA_OPERATIONS = 0x33,
    CL_UNKNOWN_API = 0x0
};

enum CL_FUNC_TYPE
{
    CL_FUNC_TYPE_clGetPlatformIDs,                         ///< cl function enum for clGetPlatformIDs
    CL_FUNC_TYPE_clGetPlatformInfo,                        ///< cl function enum for clGetPlatformInfo
    CL_FUNC_TYPE_clGetDeviceIDs,                           ///< cl function enum for clGetDeviceIDs
    CL_FUNC_TYPE_clGetDeviceInfo,                          ///< cl function enum for clGetDeviceInfo
    CL_FUNC_TYPE_clCreateContext,                          ///< cl function enum for clCreateContext
    CL_FUNC_TYPE_clCreateContextFromType,                  ///< cl function enum for clCreateContextFromType
    CL_FUNC_TYPE_clRetainContext,                          ///< cl function enum for clRetainContext
    CL_FUNC_TYPE_clReleaseContext,                         ///< cl function enum for clReleaseContext
    CL_FUNC_TYPE_clGetContextInfo,                         ///< cl function enum for clGetContextInfo
    CL_FUNC_TYPE_clCreateCommandQueue,                     ///< cl function enum for clCreateCommandQueue
    CL_FUNC_TYPE_clRetainCommandQueue,                     ///< cl function enum for clRetainCommandQueue
    CL_FUNC_TYPE_clReleaseCommandQueue,                    ///< cl function enum for clReleaseCommandQueue
    CL_FUNC_TYPE_clGetCommandQueueInfo,                    ///< cl function enum for clGetCommandQueueInfo
    CL_FUNC_TYPE_clCreateBuffer,                           ///< cl function enum for clCreateBuffer
    CL_FUNC_TYPE_clCreateSubBuffer,                        ///< cl function enum for clCreateSubBuffer
    CL_FUNC_TYPE_clCreateImage2D,                          ///< cl function enum for clCreateImage2D
    CL_FUNC_TYPE_clCreateImage3D,                          ///< cl function enum for clCreateImage3D
    CL_FUNC_TYPE_clRetainMemObject,                        ///< cl function enum for clRetainMemObject
    CL_FUNC_TYPE_clReleaseMemObject,                       ///< cl function enum for clReleaseMemObject
    CL_FUNC_TYPE_clGetSupportedImageFormats,               ///< cl function enum for clGetSupportedImageFormats
    CL_FUNC_TYPE_clGetMemObjectInfo,                       ///< cl function enum for clGetMemObjectInfo
    CL_FUNC_TYPE_clGetImageInfo,                           ///< cl function enum for clGetImageInfo
    CL_FUNC_TYPE_clSetMemObjectDestructorCallback,         ///< cl function enum for clSetMemObjectDestructorCallback
    CL_FUNC_TYPE_clCreateSampler,                          ///< cl function enum for clCreateSampler
    CL_FUNC_TYPE_clRetainSampler,                          ///< cl function enum for clRetainSampler
    CL_FUNC_TYPE_clReleaseSampler,                         ///< cl function enum for clReleaseSampler
    CL_FUNC_TYPE_clGetSamplerInfo,                         ///< cl function enum for clGetSamplerInfo
    CL_FUNC_TYPE_clCreateProgramWithSource,                ///< cl function enum for clCreateProgramWithSource
    CL_FUNC_TYPE_clCreateProgramWithBinary,                ///< cl function enum for clCreateProgramWithBinary
    CL_FUNC_TYPE_clRetainProgram,                          ///< cl function enum for clRetainProgram
    CL_FUNC_TYPE_clReleaseProgram,                         ///< cl function enum for clReleaseProgram
    CL_FUNC_TYPE_clBuildProgram,                           ///< cl function enum for clBuildProgram
    CL_FUNC_TYPE_clUnloadCompiler,                         ///< cl function enum for clUnloadCompiler
    CL_FUNC_TYPE_clGetProgramInfo,                         ///< cl function enum for clGetProgramInfo
    CL_FUNC_TYPE_clGetProgramBuildInfo,                    ///< cl function enum for clGetProgramBuildInfo
    CL_FUNC_TYPE_clCreateKernel,                           ///< cl function enum for clCreateKernel
    CL_FUNC_TYPE_clCreateKernelsInProgram,                 ///< cl function enum for clCreateKernelsInProgram
    CL_FUNC_TYPE_clRetainKernel,                           ///< cl function enum for clRetainKernel
    CL_FUNC_TYPE_clReleaseKernel,                          ///< cl function enum for clReleaseKernel
    CL_FUNC_TYPE_clSetKernelArg,                           ///< cl function enum for clSetKernelArg
    CL_FUNC_TYPE_clGetKernelInfo,                          ///< cl function enum for clGetKernelInfo
    CL_FUNC_TYPE_clGetKernelWorkGroupInfo,                 ///< cl function enum for clGetKernelWorkGroupInfo
    CL_FUNC_TYPE_clWaitForEvents,                          ///< cl function enum for clWaitForEvents
    CL_FUNC_TYPE_clGetEventInfo,                           ///< cl function enum for clGetEventInfo
    CL_FUNC_TYPE_clCreateUserEvent,                        ///< cl function enum for clCreateUserEvent
    CL_FUNC_TYPE_clRetainEvent,                            ///< cl function enum for clRetainEvent
    CL_FUNC_TYPE_clReleaseEvent,                           ///< cl function enum for clReleaseEvent
    CL_FUNC_TYPE_clSetUserEventStatus,                     ///< cl function enum for clSetUserEventStatus
    CL_FUNC_TYPE_clSetEventCallback,                       ///< cl function enum for clSetEventCallback
    CL_FUNC_TYPE_clGetEventProfilingInfo,                  ///< cl function enum for clGetEventProfilingInfo
    CL_FUNC_TYPE_clFlush,                                  ///< cl function enum for clFlush
    CL_FUNC_TYPE_clFinish,                                 ///< cl function enum for clFinish
    CL_FUNC_TYPE_clEnqueueReadBuffer,                      ///< cl function enum for clEnqueueReadBuffer
    CL_FUNC_TYPE_clEnqueueReadBufferRect,                  ///< cl function enum for clEnqueueReadBufferRect
    CL_FUNC_TYPE_clEnqueueWriteBuffer,                     ///< cl function enum for clEnqueueWriteBuffer
    CL_FUNC_TYPE_clEnqueueWriteBufferRect,                 ///< cl function enum for clEnqueueWriteBufferRect
    CL_FUNC_TYPE_clEnqueueCopyBuffer,                      ///< cl function enum for clEnqueueCopyBuffer
    CL_FUNC_TYPE_clEnqueueCopyBufferRect,                  ///< cl function enum for clEnqueueCopyBufferRect
    CL_FUNC_TYPE_clEnqueueReadImage,                       ///< cl function enum for clEnqueueReadImage
    CL_FUNC_TYPE_clEnqueueWriteImage,                      ///< cl function enum for clEnqueueWriteImage
    CL_FUNC_TYPE_clEnqueueCopyImage,                       ///< cl function enum for clEnqueueCopyImage
    CL_FUNC_TYPE_clEnqueueCopyImageToBuffer,               ///< cl function enum for clEnqueueCopyImageToBuffer
    CL_FUNC_TYPE_clEnqueueCopyBufferToImage,               ///< cl function enum for clEnqueueCopyBufferToImage
    CL_FUNC_TYPE_clEnqueueMapBuffer,                       ///< cl function enum for clEnqueueMapBuffer
    CL_FUNC_TYPE_clEnqueueMapImage,                        ///< cl function enum for clEnqueueMapImage
    CL_FUNC_TYPE_clEnqueueUnmapMemObject,                  ///< cl function enum for clEnqueueUnmapMemObject
    CL_FUNC_TYPE_clEnqueueNDRangeKernel,                   ///< cl function enum for clEnqueueNDRangeKernel
    CL_FUNC_TYPE_clEnqueueTask,                            ///< cl function enum for clEnqueueTask
    CL_FUNC_TYPE_clEnqueueNativeKernel,                    ///< cl function enum for clEnqueueNativeKernel
    CL_FUNC_TYPE_clEnqueueAcquireD3D10ObjectsKHR,          ///< cl function enum for clEnqueueAcquireD3D10ObjectsKHR
    CL_FUNC_TYPE_clEnqueueReleaseD3D10ObjectsKHR,          ///< cl function enum for clEnqueueReleaseD3D10ObjectsKHR
    CL_FUNC_TYPE_clEnqueueAcquireGLObjects,                ///< cl function enum for clEnqueueAcquireGLObjects
    CL_FUNC_TYPE_clEnqueueReleaseGLObjects,                ///< cl function enum for clEnqueueReleaseGLObjects
    CL_FUNC_TYPE_clEnqueueMarker,                          ///< cl function enum for clEnqueueMarker
    CL_FUNC_TYPE_clEnqueueWaitForEvents,                   ///< cl function enum for clEnqueueWaitForEvents
    CL_FUNC_TYPE_clEnqueueBarrier,                         ///< cl function enum for clEnqueueBarrier

    CL_FUNC_TYPE_clCreateFromGLBuffer,                     ///< cl function enum for clCreateFromGLBuffer
    CL_FUNC_TYPE_clCreateFromGLTexture2D,                  ///< cl function enum for clCreateFromGLTexture2D
    CL_FUNC_TYPE_clCreateFromGLTexture3D,                  ///< cl function enum for clCreateFromGLTexture3D
    CL_FUNC_TYPE_clCreateFromGLRenderbuffer,               ///< cl function enum for clCreateFromGLRenderbuffer
    CL_FUNC_TYPE_clGetGLObjectInfo,                        ///< cl function enum for clGetGLObjectInfo
    CL_FUNC_TYPE_clGetGLTextureInfo,                       ///< cl function enum for clGetGLTextureInfo
    CL_FUNC_TYPE_clCreateEventFromGLsyncKHR,               ///< cl function enum for clCreateEventFromGLsyncKHR
    CL_FUNC_TYPE_clGetGLContextInfoKHR,                    ///< cl function enum for clGetGLContextInfoKHR

    CL_FUNC_TYPE_clCreateSubDevicesEXT,                    ///< cl function enum for clCreateSubDevicesEXT
    CL_FUNC_TYPE_clRetainDeviceEXT,                        ///< cl function enum for clRetainDeviceEXT
    CL_FUNC_TYPE_clReleaseDeviceEXT,                       ///< cl function enum for clReleaseDeviceEXT

    CL_FUNC_TYPE_clGetDeviceIDsFromD3D10KHR,               ///< cl function enum for clGetDeviceIDsFromD3D10KHR
    CL_FUNC_TYPE_clCreateFromD3D10BufferKHR,               ///< cl function enum for clCreateFromD3D10BufferKHR
    CL_FUNC_TYPE_clCreateFromD3D10Texture2DKHR,            ///< cl function enum for clCreateFromD3D10Texture2DKHR
    CL_FUNC_TYPE_clCreateFromD3D10Texture3DKHR,            ///< cl function enum for clCreateFromD3D10Texture3DKHR

    CL_FUNC_TYPE_clSetCommandQueueProperty,                ///< cl function enum for clSetCommandQueueProperty
    CL_FUNC_TYPE_clCreateSubDevices,                       ///< cl function enum for clCreateSubDevices
    CL_FUNC_TYPE_clRetainDevice,                           ///< cl function enum for clRetainDevice
    CL_FUNC_TYPE_clReleaseDevice,                          ///< cl function enum for clReleaseDevice
    CL_FUNC_TYPE_clCreateImage,                            ///< cl function enum for clCreateImage
    CL_FUNC_TYPE_clCreateProgramWithBuiltInKernels,        ///< cl function enum for clCreateProgramWithBuiltInKernels
    CL_FUNC_TYPE_clCompileProgram,                         ///< cl function enum for clCompileProgram
    CL_FUNC_TYPE_clLinkProgram,                            ///< cl function enum for clLinkProgram
    CL_FUNC_TYPE_clUnloadPlatformCompiler,                 ///< cl function enum for clUnloadPlatformCompiler
    CL_FUNC_TYPE_clGetKernelArgInfo,                       ///< cl function enum for clGetKernelArgInfo
    CL_FUNC_TYPE_clEnqueueFillBuffer,                      ///< cl function enum for clEnqueueFillBuffer
    CL_FUNC_TYPE_clEnqueueFillImage,                       ///< cl function enum for clEnqueueFillImage
    CL_FUNC_TYPE_clEnqueueMigrateMemObjects,               ///< cl function enum for clEnqueueMigrateMemObjects
    CL_FUNC_TYPE_clEnqueueMarkerWithWaitList,              ///< cl function enum for clEnqueueMarkerWithWaitList
    CL_FUNC_TYPE_clEnqueueBarrierWithWaitList,             ///< cl function enum for clEnqueueBarrierWithWaitList
    CL_FUNC_TYPE_clGetExtensionFunctionAddressForPlatform, ///< cl function enum for clGetExtensionFunctionAddressForPlatform
    CL_FUNC_TYPE_clCreateFromGLTexture,                    ///< cl function enum for clCreateFromGLTexture
    CL_FUNC_TYPE_clGetExtensionFunctionAddress,            ///< cl function enum for clGetExtensionFunctionAddress

    CL_FUNC_TYPE_clCreateCommandQueueWithProperties,       ///< cl function enum for clCreateCommandQueueWithProperties
    CL_FUNC_TYPE_clCreatePipe,                             ///< cl function enum for clCreatePipe
    CL_FUNC_TYPE_clGetPipeInfo,                            ///< cl function enum for clGetPipeInfo
    CL_FUNC_TYPE_clSVMAlloc,                               ///< cl function enum for clSVMAlloc
    CL_FUNC_TYPE_clSVMFree,                                ///< cl function enum for clSVMFree
    CL_FUNC_TYPE_clEnqueueSVMFree,                         ///< cl function enum for clEnqueueSVMFree
    CL_FUNC_TYPE_clEnqueueSVMMemcpy,                       ///< cl function enum for clEnqueueSVMMemcpy
    CL_FUNC_TYPE_clEnqueueSVMMemFill,                      ///< cl function enum for clEnqueueSVMMemFill
    CL_FUNC_TYPE_clEnqueueSVMMap,                          ///< cl function enum for clEnqueueSVMMap
    CL_FUNC_TYPE_clEnqueueSVMUnmap,                        ///< cl function enum for clEnqueueSVMUnmap
    CL_FUNC_TYPE_clCreateSamplerWithProperties,            ///< cl function enum for clCreateSamplerWithProperties
    CL_FUNC_TYPE_clSetKernelArgSVMPointer,                 ///< cl function enum for clSetKernelArgSVMPointer
    CL_FUNC_TYPE_clSetKernelExecInfo,                      ///< cl function enum for clSetKernelExecInfo

    CL_FUNC_TYPE_clSVMAllocAMD,                            ///< cl function enum for clSVMAllocAMD
    CL_FUNC_TYPE_clSVMFreeAMD,                             ///< cl function enum for clSVMFreeAMD
    CL_FUNC_TYPE_clEnqueueSVMFreeAMD,                      ///< cl function enum for clEnqueueSVMFreeAMD
    CL_FUNC_TYPE_clEnqueueSVMMemcpyAMD,                    ///< cl function enum for clEnqueueSVMMemcpyAMD
    CL_FUNC_TYPE_clEnqueueSVMMemFillAMD,                   ///< cl function enum for clEnqueueSVMMemFillAMD
    CL_FUNC_TYPE_clEnqueueSVMMapAMD,                       ///< cl function enum for clEnqueueSVMMapAMD
    CL_FUNC_TYPE_clEnqueueSVMUnmapAMD,                     ///< cl function enum for clEnqueueSVMUnmapAMD
    CL_FUNC_TYPE_clSetKernelArgSVMPointerAMD,              ///< cl function enum for clSetKernelArgSVMPointerAMD
    CL_FUNC_TYPE_clSetKernelExecInfoAMD,                   ///< cl function enum for clSetKernelExecInfoAMD

    CL_FUNC_TYPE_Unknown                                   ///< cl unknown function
};

/// Convert OpenCL API name string to enum
/// \param strName API name string
/// \return enum representation of OpenCL API
CL_FUNC_TYPE ToCLFuncType(const std::string& strName);

/// Checks if a given API Id is an enqueue API
/// \param[in]    uiAPIId     API Id
/// \return flag indicating whether or not the specified API is represents an enqueue API
bool IsEnqueueAPI(const unsigned int uiAPIId);

#endif //_CL_FUNCTION_ENUM_DEFS_H_
