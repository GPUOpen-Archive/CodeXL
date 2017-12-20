//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CLAPIDefs.cpp $
/// \version $Revision: #14 $
/// \brief  This file contains definitions for CL API Functions
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CLAPIDefs.cpp#14 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#include <qtIgnoreCompilerWarnings.h>

#ifdef _WIN32
    #pragma warning(push, 1)
#endif
#include <QtCore>
#include <QtWidgets>
#ifdef _WIN32
    #pragma warning(pop)
#endif

#include <AMDTBaseTools/Include/gtAssert.h>

#include "CLAPIDefs.h"


CLAPIDefs::CLAPIDefs()
{
    m_openCLAPIString <<
                      "clGetPlatformIDs" <<
                      "clGetPlatformInfo" <<
                      "clGetDeviceIDs" <<
                      "clGetDeviceInfo" <<
                      "clCreateContext" <<
                      "clCreateContextFromType" <<
                      "clRetainContext" <<
                      "clReleaseContext" <<
                      "clGetContextInfo" <<
                      "clCreateCommandQueue" <<
                      "clRetainCommandQueue" <<
                      "clReleaseCommandQueue" <<
                      "clGetCommandQueueInfo" <<
                      "clCreateBuffer" <<
                      "clCreateSubBuffer" <<
                      "clCreateImage2D" <<
                      "clCreateImage3D" <<
                      "clRetainMemObject" <<
                      "clReleaseMemObject" <<
                      "clGetSupportedImageFormats" <<
                      "clGetMemObjectInfo" <<
                      "clGetImageInfo" <<
                      "clSetMemObjectDestructorCallback" <<
                      "clCreateSampler" <<
                      "clRetainSampler" <<
                      "clReleaseSampler" <<
                      "clGetSamplerInfo" <<
                      "clCreateProgramWithSource" <<
                      "clCreateProgramWithBinary" <<
                      "clRetainProgram" <<
                      "clReleaseProgram" <<
                      "clBuildProgram" <<
                      "clUnloadCompiler" <<
                      "clGetProgramInfo" <<
                      "clGetProgramBuildInfo" <<
                      "clCreateKernel" <<
                      "clCreateKernelsInProgram" <<
                      "clRetainKernel" <<
                      "clReleaseKernel" <<
                      "clSetKernelArg" <<
                      "clGetKernelInfo" <<
                      "clGetKernelWorkGroupInfo" <<
                      "clWaitForEvents" <<
                      "clGetEventInfo" <<
                      "clCreateUserEvent" <<
                      "clRetainEvent" <<
                      "clReleaseEvent" <<
                      "clSetUserEventStatus" <<
                      "clSetEventCallback" <<
                      "clGetEventProfilingInfo" <<
                      "clFlush" <<
                      "clFinish" <<
                      "clEnqueueReadBuffer" <<
                      "clEnqueueReadBufferRect" <<
                      "clEnqueueWriteBuffer" <<
                      "clEnqueueWriteBufferRect" <<
                      "clEnqueueCopyBuffer" <<
                      "clEnqueueCopyBufferRect" <<
                      "clEnqueueReadImage" <<
                      "clEnqueueWriteImage" <<
                      "clEnqueueCopyImage" <<
                      "clEnqueueCopyImageToBuffer" <<
                      "clEnqueueCopyBufferToImage" <<
                      "clEnqueueMapBuffer" <<
                      "clEnqueueMapImage" <<
                      "clEnqueueUnmapMemObject" <<
                      "clEnqueueNDRangeKernel" <<
                      "clEnqueueTask" <<
                      "clEnqueueNativeKernel" <<
                      "clEnqueueAcquireD3D10ObjectsKHR" <<
                      "clEnqueueReleaseD3D10ObjectsKHR" <<
                      "clEnqueueAcquireGLObjects" <<
                      "clEnqueueReleaseGLObjects" <<
                      "clEnqueueMarker" <<
                      "clEnqueueWaitForEvents" <<
                      "clEnqueueBarrier" <<
                      "clCreateFromGLBuffer" <<
                      "clCreateFromGLTexture2D" <<
                      "clCreateFromGLTexture3D" <<
                      "clCreateFromGLRenderbuffer" <<
                      "clGetGLObjectInfo" <<
                      "clGetGLTextureInfo" <<
                      "clCreateEventFromGLsyncKHR" <<
                      "clGetGLContextInfoKHR" <<
                      "clCreateSubDevicesEXT" <<
                      "clRetainDeviceEXT" <<
                      "clReleaseDeviceEXT" <<
                      "clGetDeviceIDsFromD3D10KHR" <<
                      "clCreateFromD3D10BufferKHR" <<
                      "clCreateFromD3D10Texture2DKHR" <<
                      "clCreateFromD3D10Texture3DKHR" <<
                      "clSetCommandQueueProperty" <<
                      "clCreateSubDevices" <<
                      "clRetainDevice" <<
                      "clReleaseDevice" <<
                      "clCreateImage" <<
                      "clCreateProgramWithBuiltInKernels" <<
                      "clCompileProgram" <<
                      "clLinkProgram" <<
                      "clUnloadPlatformCompiler" <<
                      "clGetKernelArgInfo" <<
                      "clEnqueueFillBuffer" <<
                      "clEnqueueFillImage" <<
                      "clEnqueueMigrateMemObjects" <<
                      "clEnqueueMarkerWithWaitList" <<
                      "clEnqueueBarrierWithWaitList" <<
                      "clGetExtensionFunctionAddressForPlatform" <<
                      "clCreateFromGLTexture" <<
                      "clGetExtensionFunctionAddress" <<
                      "clCreateCommandQueueWithProperties" <<
                      "clCreatePipe" <<
                      "clGetPipeInfo" <<
                      "clSVMAlloc" <<
                      "clSVMFree" <<
                      "clEnqueueSVMFree" <<
                      "clEnqueueSVMMemcpy" <<
                      "clEnqueueSVMMemFill" <<
                      "clEnqueueSVMMap" <<
                      "clEnqueueSVMUnmap" <<
                      "clCreateSamplerWithProperties" <<
                      "clSetKernelArgSVMPointer" <<
                      "clSetKernelExecInfo" <<
                      "clSVMAllocAMD" <<
                      "clSVMFreeAMD" <<
                      "clEnqueueSVMFreeAMD" <<
                      "clEnqueueSVMMemcpyAMD" <<
                      "clEnqueueSVMMemFillAMD" <<
                      "clEnqueueSVMMapAMD" <<
                      "clEnqueueSVMUnmapAMD" <<
                      "clSetKernelArgSVMPointerAMD" <<
                      "clSetKernelExecInfoAMD" <<
                      "clCreateSsgFileObjectAMD" <<
                      "clGetSsgFileObjectInfoAMD" <<
                      "clRetainSsgFileObjectAMD" <<
                      "clReleaseSsgFileObjectAMD" <<
                      "clEnqueueReadSsgFileAMD" <<
                      "clEnqueueWriteSsgFileAMD" <<
                      "";

    GT_ASSERT(m_openCLAPIString.length() - 1 == CL_FUNC_TYPE_Unknown);

    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetPlatformIDs], CL_FUNC_TYPE_clGetPlatformIDs);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetPlatformInfo], CL_FUNC_TYPE_clGetPlatformInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetDeviceIDs], CL_FUNC_TYPE_clGetDeviceIDs);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetDeviceInfo], CL_FUNC_TYPE_clGetDeviceInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateContext], CL_FUNC_TYPE_clCreateContext);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateContextFromType], CL_FUNC_TYPE_clCreateContextFromType);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clRetainContext], CL_FUNC_TYPE_clRetainContext);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clReleaseContext], CL_FUNC_TYPE_clReleaseContext);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetContextInfo], CL_FUNC_TYPE_clGetContextInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateCommandQueue], CL_FUNC_TYPE_clCreateCommandQueue);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clRetainCommandQueue], CL_FUNC_TYPE_clRetainCommandQueue);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clReleaseCommandQueue], CL_FUNC_TYPE_clReleaseCommandQueue);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetCommandQueueInfo], CL_FUNC_TYPE_clGetCommandQueueInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateBuffer], CL_FUNC_TYPE_clCreateBuffer);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateSubBuffer], CL_FUNC_TYPE_clCreateSubBuffer);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateImage2D], CL_FUNC_TYPE_clCreateImage2D);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateImage3D], CL_FUNC_TYPE_clCreateImage3D);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clRetainMemObject], CL_FUNC_TYPE_clRetainMemObject);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clReleaseMemObject], CL_FUNC_TYPE_clReleaseMemObject);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetSupportedImageFormats], CL_FUNC_TYPE_clGetSupportedImageFormats);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetMemObjectInfo], CL_FUNC_TYPE_clGetMemObjectInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetImageInfo], CL_FUNC_TYPE_clGetImageInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clSetMemObjectDestructorCallback], CL_FUNC_TYPE_clSetMemObjectDestructorCallback);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateSampler], CL_FUNC_TYPE_clCreateSampler);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clRetainSampler], CL_FUNC_TYPE_clRetainSampler);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clReleaseSampler], CL_FUNC_TYPE_clReleaseSampler);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetSamplerInfo], CL_FUNC_TYPE_clGetSamplerInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateProgramWithSource], CL_FUNC_TYPE_clCreateProgramWithSource);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateProgramWithBinary], CL_FUNC_TYPE_clCreateProgramWithBinary);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clRetainProgram], CL_FUNC_TYPE_clRetainProgram);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clReleaseProgram], CL_FUNC_TYPE_clReleaseProgram);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clBuildProgram], CL_FUNC_TYPE_clBuildProgram);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clUnloadCompiler], CL_FUNC_TYPE_clUnloadCompiler);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetProgramInfo], CL_FUNC_TYPE_clGetProgramInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetProgramBuildInfo], CL_FUNC_TYPE_clGetProgramBuildInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateKernel], CL_FUNC_TYPE_clCreateKernel);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateKernelsInProgram], CL_FUNC_TYPE_clCreateKernelsInProgram);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clRetainKernel], CL_FUNC_TYPE_clRetainKernel);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clReleaseKernel], CL_FUNC_TYPE_clReleaseKernel);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clSetKernelArg], CL_FUNC_TYPE_clSetKernelArg);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetKernelInfo], CL_FUNC_TYPE_clGetKernelInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetKernelWorkGroupInfo], CL_FUNC_TYPE_clGetKernelWorkGroupInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clWaitForEvents], CL_FUNC_TYPE_clWaitForEvents);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetEventInfo], CL_FUNC_TYPE_clGetEventInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateUserEvent], CL_FUNC_TYPE_clCreateUserEvent);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clRetainEvent], CL_FUNC_TYPE_clRetainEvent);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clReleaseEvent], CL_FUNC_TYPE_clReleaseEvent);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clSetUserEventStatus], CL_FUNC_TYPE_clSetUserEventStatus);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clSetEventCallback], CL_FUNC_TYPE_clSetEventCallback);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetEventProfilingInfo], CL_FUNC_TYPE_clGetEventProfilingInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clFlush], CL_FUNC_TYPE_clFlush);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clFinish], CL_FUNC_TYPE_clFinish);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueReadBuffer], CL_FUNC_TYPE_clEnqueueReadBuffer);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueReadBufferRect], CL_FUNC_TYPE_clEnqueueReadBufferRect);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueWriteBuffer], CL_FUNC_TYPE_clEnqueueWriteBuffer);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueWriteBufferRect], CL_FUNC_TYPE_clEnqueueWriteBufferRect);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueCopyBuffer], CL_FUNC_TYPE_clEnqueueCopyBuffer);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueCopyBufferRect], CL_FUNC_TYPE_clEnqueueCopyBufferRect);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueReadImage], CL_FUNC_TYPE_clEnqueueReadImage);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueWriteImage], CL_FUNC_TYPE_clEnqueueWriteImage);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueCopyImage], CL_FUNC_TYPE_clEnqueueCopyImage);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueCopyImageToBuffer], CL_FUNC_TYPE_clEnqueueCopyImageToBuffer);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueCopyBufferToImage], CL_FUNC_TYPE_clEnqueueCopyBufferToImage);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueMapBuffer], CL_FUNC_TYPE_clEnqueueMapBuffer);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueMapImage], CL_FUNC_TYPE_clEnqueueMapImage);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueUnmapMemObject], CL_FUNC_TYPE_clEnqueueUnmapMemObject);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueNDRangeKernel], CL_FUNC_TYPE_clEnqueueNDRangeKernel);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueTask], CL_FUNC_TYPE_clEnqueueTask);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueNativeKernel], CL_FUNC_TYPE_clEnqueueNativeKernel);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueMarker], CL_FUNC_TYPE_clEnqueueMarker);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueWaitForEvents], CL_FUNC_TYPE_clEnqueueWaitForEvents);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueBarrier], CL_FUNC_TYPE_clEnqueueBarrier);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateFromGLBuffer], CL_FUNC_TYPE_clCreateFromGLBuffer);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateFromGLTexture2D], CL_FUNC_TYPE_clCreateFromGLTexture2D);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateFromGLTexture3D], CL_FUNC_TYPE_clCreateFromGLTexture3D);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateFromGLRenderbuffer], CL_FUNC_TYPE_clCreateFromGLRenderbuffer);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetGLObjectInfo], CL_FUNC_TYPE_clGetGLObjectInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetGLTextureInfo], CL_FUNC_TYPE_clGetGLTextureInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueAcquireGLObjects], CL_FUNC_TYPE_clEnqueueAcquireGLObjects);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueReleaseGLObjects], CL_FUNC_TYPE_clEnqueueReleaseGLObjects);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetGLContextInfoKHR], CL_FUNC_TYPE_clGetGLContextInfoKHR);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateSubDevicesEXT], CL_FUNC_TYPE_clCreateSubDevicesEXT);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clRetainDeviceEXT], CL_FUNC_TYPE_clRetainDeviceEXT);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clReleaseDeviceEXT], CL_FUNC_TYPE_clReleaseDeviceEXT);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetDeviceIDsFromD3D10KHR], CL_FUNC_TYPE_clGetDeviceIDsFromD3D10KHR);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateFromD3D10BufferKHR], CL_FUNC_TYPE_clCreateFromD3D10BufferKHR);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateFromD3D10Texture2DKHR], CL_FUNC_TYPE_clCreateFromD3D10Texture2DKHR);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateFromD3D10Texture3DKHR], CL_FUNC_TYPE_clCreateFromD3D10Texture3DKHR);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueAcquireD3D10ObjectsKHR], CL_FUNC_TYPE_clEnqueueAcquireD3D10ObjectsKHR);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueReleaseD3D10ObjectsKHR], CL_FUNC_TYPE_clEnqueueReleaseD3D10ObjectsKHR);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateEventFromGLsyncKHR], CL_FUNC_TYPE_clCreateEventFromGLsyncKHR);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clSetCommandQueueProperty], CL_FUNC_TYPE_clSetCommandQueueProperty);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateSubDevices], CL_FUNC_TYPE_clCreateSubDevices);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clRetainDevice], CL_FUNC_TYPE_clRetainDevice);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clReleaseDevice], CL_FUNC_TYPE_clReleaseDevice);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateImage], CL_FUNC_TYPE_clCreateImage);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateProgramWithBuiltInKernels], CL_FUNC_TYPE_clCreateProgramWithBuiltInKernels);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCompileProgram], CL_FUNC_TYPE_clCompileProgram);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clLinkProgram], CL_FUNC_TYPE_clLinkProgram);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clUnloadPlatformCompiler], CL_FUNC_TYPE_clUnloadPlatformCompiler);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetKernelArgInfo], CL_FUNC_TYPE_clGetKernelArgInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueFillBuffer], CL_FUNC_TYPE_clEnqueueFillBuffer);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueFillImage], CL_FUNC_TYPE_clEnqueueFillImage);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueMigrateMemObjects], CL_FUNC_TYPE_clEnqueueMigrateMemObjects);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueMarkerWithWaitList], CL_FUNC_TYPE_clEnqueueMarkerWithWaitList);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueBarrierWithWaitList], CL_FUNC_TYPE_clEnqueueBarrierWithWaitList);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetExtensionFunctionAddressForPlatform], CL_FUNC_TYPE_clGetExtensionFunctionAddressForPlatform);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateFromGLTexture], CL_FUNC_TYPE_clCreateFromGLTexture);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetExtensionFunctionAddress], CL_FUNC_TYPE_clGetExtensionFunctionAddress);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateCommandQueueWithProperties], CL_FUNC_TYPE_clCreateCommandQueueWithProperties);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreatePipe], CL_FUNC_TYPE_clCreatePipe);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetPipeInfo], CL_FUNC_TYPE_clGetPipeInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clSVMAlloc], CL_FUNC_TYPE_clSVMAlloc);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clSVMFree], CL_FUNC_TYPE_clSVMFree);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueSVMFree], CL_FUNC_TYPE_clEnqueueSVMFree);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueSVMMemcpy], CL_FUNC_TYPE_clEnqueueSVMMemcpy);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueSVMMemFill], CL_FUNC_TYPE_clEnqueueSVMMemFill);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueSVMMap], CL_FUNC_TYPE_clEnqueueSVMMap);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueSVMUnmap], CL_FUNC_TYPE_clEnqueueSVMUnmap);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateSamplerWithProperties], CL_FUNC_TYPE_clCreateSamplerWithProperties);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clSetKernelArgSVMPointer], CL_FUNC_TYPE_clSetKernelArgSVMPointer);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clSetKernelExecInfo], CL_FUNC_TYPE_clSetKernelExecInfo);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clSVMAllocAMD], CL_FUNC_TYPE_clSVMAllocAMD);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clSVMFreeAMD], CL_FUNC_TYPE_clSVMFreeAMD);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueSVMFreeAMD], CL_FUNC_TYPE_clEnqueueSVMFreeAMD);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueSVMMemcpyAMD], CL_FUNC_TYPE_clEnqueueSVMMemcpyAMD);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueSVMMemFillAMD], CL_FUNC_TYPE_clEnqueueSVMMemFillAMD);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueSVMMapAMD], CL_FUNC_TYPE_clEnqueueSVMMapAMD);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueSVMUnmapAMD], CL_FUNC_TYPE_clEnqueueSVMUnmapAMD);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clSetKernelArgSVMPointerAMD], CL_FUNC_TYPE_clSetKernelArgSVMPointerAMD);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clSetKernelExecInfoAMD], CL_FUNC_TYPE_clSetKernelExecInfoAMD);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clCreateSsgFileObjectAMD], CL_FUNC_TYPE_clCreateSsgFileObjectAMD);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clGetSsgFileObjectInfoAMD], CL_FUNC_TYPE_clGetSsgFileObjectInfoAMD);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clRetainSsgFileObjectAMD], CL_FUNC_TYPE_clRetainSsgFileObjectAMD);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clReleaseSsgFileObjectAMD], CL_FUNC_TYPE_clReleaseSsgFileObjectAMD);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueReadSsgFileAMD], CL_FUNC_TYPE_clEnqueueReadSsgFileAMD);
    m_openCLAPIMap.insert(m_openCLAPIString[CL_FUNC_TYPE_clEnqueueWriteSsgFileAMD], CL_FUNC_TYPE_clEnqueueWriteSsgFileAMD);

    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetPlatformIDs, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetPlatformInfo, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetDeviceIDs, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetDeviceInfo, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateContext, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateContextFromType, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clRetainContext, CLAPIGroup_CLObjectRetain);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clReleaseContext, CLAPIGroup_CLObjectRelease);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetContextInfo, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateCommandQueue, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clRetainCommandQueue, CLAPIGroup_CLObjectRetain);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clReleaseCommandQueue, CLAPIGroup_CLObjectRelease);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetCommandQueueInfo, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateBuffer, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateSubBuffer, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateImage2D, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateImage3D, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clRetainMemObject, CLAPIGroup_CLObjectRetain);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clReleaseMemObject, CLAPIGroup_CLObjectRelease);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetSupportedImageFormats, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetMemObjectInfo, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetImageInfo, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clSetMemObjectDestructorCallback, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateSampler, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clRetainSampler, CLAPIGroup_CLObjectRetain);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clReleaseSampler, CLAPIGroup_CLObjectRelease);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetSamplerInfo, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateProgramWithSource, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateProgramWithBinary, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clRetainProgram, CLAPIGroup_CLObjectRetain);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clReleaseProgram, CLAPIGroup_CLObjectRelease);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clBuildProgram, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clUnloadCompiler, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetProgramInfo, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetProgramBuildInfo, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateKernel, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateKernelsInProgram, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clRetainKernel, CLAPIGroup_CLObjectRetain);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clReleaseKernel, CLAPIGroup_CLObjectRelease);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clSetKernelArg, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetKernelInfo, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetKernelWorkGroupInfo, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clWaitForEvents, CLAPIGroup_Synchronization);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetEventInfo, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateUserEvent, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clRetainEvent, CLAPIGroup_CLObjectRetain);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clReleaseEvent, CLAPIGroup_CLObjectRelease);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clSetUserEventStatus, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clSetEventCallback, CLAPIGroup_SetCallback);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetEventProfilingInfo, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clFlush, CLAPIGroup_Synchronization);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clFinish, CLAPIGroup_Synchronization);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueReadBuffer, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueReadBufferRect, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueWriteBuffer, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueWriteBufferRect, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueCopyBuffer, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueCopyBufferRect, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueReadImage, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueWriteImage, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueCopyImage, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueCopyImageToBuffer, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueCopyBufferToImage, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueMapBuffer, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueMapImage, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueUnmapMemObject, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueNDRangeKernel, CLAPIGroup_EnqueueKernel);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueTask, CLAPIGroup_EnqueueKernel);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueNativeKernel, CLAPIGroup_EnqueueKernel);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueMarker, CLAPIGroup_EnqueueOther);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueWaitForEvents, CLAPIGroup_EnqueueOther);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueBarrier, CLAPIGroup_EnqueueOther);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateFromGLBuffer, CLAPIGroup_OpenGLInterOp);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateFromGLTexture2D, CLAPIGroup_OpenGLInterOp);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateFromGLTexture3D, CLAPIGroup_OpenGLInterOp);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateFromGLRenderbuffer, CLAPIGroup_OpenGLInterOp);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetGLObjectInfo, CLAPIGroup_OpenGLInterOp);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetGLTextureInfo, CLAPIGroup_OpenGLInterOp);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueAcquireGLObjects, CLAPIGroup_OpenGLInterOp);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueReleaseGLObjects, CLAPIGroup_OpenGLInterOp);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetGLContextInfoKHR, CLAPIGroup_OpenGLInterOp);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateSubDevicesEXT, CLAPIGroup_Extensions);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clRetainDeviceEXT, CLAPIGroup_Extensions);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clReleaseDeviceEXT, CLAPIGroup_Extensions);
#ifdef _WIN32
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetDeviceIDsFromD3D10KHR, CLAPIGroup_DirectXInterOp);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateFromD3D10BufferKHR, CLAPIGroup_DirectXInterOp);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateFromD3D10Texture2DKHR, CLAPIGroup_DirectXInterOp);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateFromD3D10Texture3DKHR, CLAPIGroup_DirectXInterOp);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueAcquireD3D10ObjectsKHR, CLAPIGroup_DirectXInterOp);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueReleaseD3D10ObjectsKHR, CLAPIGroup_DirectXInterOp);
#endif
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateEventFromGLsyncKHR, CLAPIGroup_OpenGLInterOp);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clSetCommandQueueProperty, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateSubDevices, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clRetainDevice, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clReleaseDevice, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateImage, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateProgramWithBuiltInKernels, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCompileProgram, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clLinkProgram, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clUnloadPlatformCompiler, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetKernelArgInfo, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueFillBuffer, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueFillImage, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueMigrateMemObjects, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueMarkerWithWaitList, CLAPIGroup_EnqueueOther);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueBarrierWithWaitList, CLAPIGroup_EnqueueOther);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetExtensionFunctionAddressForPlatform, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateFromGLTexture, CLAPIGroup_OpenGLInterOp);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetExtensionFunctionAddress, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateCommandQueueWithProperties, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreatePipe, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetPipeInfo, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clSVMAlloc, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clSVMFree, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueSVMFree, CLAPIGroup_EnqueueOther);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueSVMMemcpy, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueSVMMemFill, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueSVMMap, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueSVMUnmap, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateSamplerWithProperties, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clSetKernelArgSVMPointer, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clSetKernelExecInfo, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clSVMAllocAMD, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clSVMFreeAMD, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueSVMFreeAMD, CLAPIGroup_EnqueueOther);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueSVMMemcpyAMD, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueSVMMemFillAMD, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueSVMMapAMD, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueSVMUnmapAMD, CLAPIGroup_EnqueueDataTransfer);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clSetKernelArgSVMPointerAMD, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clSetKernelExecInfoAMD, CLAPIGroup_Other);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clCreateSsgFileObjectAMD, CLAPIGroup_CLObjectCreate);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clGetSsgFileObjectInfoAMD, CLAPIGroup_QueryInfo);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clRetainSsgFileObjectAMD, CLAPIGroup_CLObjectRetain);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clReleaseSsgFileObjectAMD, CLAPIGroup_CLObjectRelease);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueReadSsgFileAMD, CLAPIGroup_EnqueueOther);
    m_clAPIGroupMap.insert(CL_FUNC_TYPE_clEnqueueWriteSsgFileAMD, CLAPIGroup_EnqueueOther);
}

const QString& CLAPIDefs::GetOpenCLAPIString(CL_FUNC_TYPE type)
{
    if (type > CL_FUNC_TYPE_Unknown)
    {
        GT_ASSERT(type <= CL_FUNC_TYPE_Unknown);
        type = CL_FUNC_TYPE_Unknown;
    }

    return m_openCLAPIString[type];
}

CLAPIGroups CLAPIDefs::GetCLAPIGroup(CL_FUNC_TYPE type)
{
    CLAPIGroups retVal = CLAPIGroup_Unknown;

    if (m_clAPIGroupMap.contains(type))
    {
        retVal = m_clAPIGroupMap[type];
    }

    return retVal;
}

CL_FUNC_TYPE CLAPIDefs::ToCLAPIType(QString name)
{
    CL_FUNC_TYPE retVal = CL_FUNC_TYPE_Unknown;

    if (m_openCLAPIMap.contains(name))
    {
        retVal = m_openCLAPIMap[name];
    }

    return retVal;
}

const QString CLAPIDefs::GroupToString(CLAPIGroup group)
{
    switch (group)
    {
        case CLAPIGroup_CLObjectCreate:
            return "CLObjectCreate";

        case CLAPIGroup_CLObjectRetain:
            return "CLObjectRetain";

        case CLAPIGroup_CLObjectRelease:
            return "CLObjectRelease";

        case CLAPIGroup_QueryInfo:
            return "QueryInfo";

        case CLAPIGroup_EnqueueDataTransfer:
            return "EnqueueDataTransfer";

        case CLAPIGroup_EnqueueKernel:
            return "EnqueueKernel";

        case CLAPIGroup_EnqueueOther:
            return "EnqueueOther";

        case CLAPIGroup_OpenGLInterOp:
            return "OpenGLInterOp";

        case CLAPIGroup_DirectXInterOp:
            return "DirectXInterOp";

        case CLAPIGroup_Synchronization:
            return "Synchronization";

        case CLAPIGroup_SetCallback:
            return "SetCallback";

        case CLAPIGroup_Extensions:
            return "Extensions";

        case CLAPIGroup_Other:
            return "Other";

        default:
            return "Other";
    }
}


