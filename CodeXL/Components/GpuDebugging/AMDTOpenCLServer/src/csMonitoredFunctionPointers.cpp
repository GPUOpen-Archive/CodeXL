//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csMonitoredFunctionPointers.cpp
///
//==================================================================================

//------------------------------ csMonitoredFunctionPointers.cpp ------------------------------

// OpenCL ICD header:
// Uri, 22/3/12 - Use the internal AMD version, since it has the 1.2 pointers:
// #include <CL/cl_icd_amd.h> // This file is included from a specific location in AMDOpenCLDebug.h
#include <AMDOpenCLDebug.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <src/csMonitoredFunctionPointers.h>


// ---------------------------------------------------------------------------
// Name:        csFillICDDispatchTableWithFunctionPointers
// Description: Fills the icdDispatch struct with the function pointers from our struct
// Author:      Uri Shomroni
// Date:        14/12/2010
// ---------------------------------------------------------------------------
void csFillICDDispatchTableWithFunctionPointers(cl_icd_dispatch_table& icdDispatch, const csMonitoredFunctionPointers& functionPointers)
{
    icdDispatch.GetPlatformIDs = functionPointers.clGetPlatformIDs;
    icdDispatch.GetPlatformInfo = functionPointers.clGetPlatformInfo;
    icdDispatch.GetDeviceIDs = functionPointers.clGetDeviceIDs;
    icdDispatch.GetDeviceInfo = functionPointers.clGetDeviceInfo;
    icdDispatch.CreateSubDevices = functionPointers.clCreateSubDevices;
    icdDispatch.RetainDevice = functionPointers.clRetainDevice;
    icdDispatch.ReleaseDevice = functionPointers.clReleaseDevice;
    icdDispatch.CreateContext = functionPointers.clCreateContext;
    icdDispatch.CreateContextFromType = functionPointers.clCreateContextFromType;
    icdDispatch.RetainContext = functionPointers.clRetainContext;
    icdDispatch.ReleaseContext = functionPointers.clReleaseContext;
    icdDispatch.GetContextInfo = functionPointers.clGetContextInfo;
    icdDispatch.CreateCommandQueue = functionPointers.clCreateCommandQueue;
    icdDispatch.RetainCommandQueue = functionPointers.clRetainCommandQueue;
    icdDispatch.ReleaseCommandQueue = functionPointers.clReleaseCommandQueue;
    icdDispatch.GetCommandQueueInfo = functionPointers.clGetCommandQueueInfo;
    icdDispatch.SetCommandQueueProperty = functionPointers.clSetCommandQueueProperty;
    icdDispatch.CreateBuffer = functionPointers.clCreateBuffer;
    icdDispatch.CreateSubBuffer = functionPointers.clCreateSubBuffer;
    icdDispatch.CreateImage = functionPointers.clCreateImage;
    icdDispatch.CreateImage2D = functionPointers.clCreateImage2D;
    icdDispatch.CreateImage3D = functionPointers.clCreateImage3D;
    icdDispatch.RetainMemObject = functionPointers.clRetainMemObject;
    icdDispatch.ReleaseMemObject = functionPointers.clReleaseMemObject;
    icdDispatch.GetSupportedImageFormats = functionPointers.clGetSupportedImageFormats;
    icdDispatch.GetMemObjectInfo = functionPointers.clGetMemObjectInfo;
    icdDispatch.GetImageInfo = functionPointers.clGetImageInfo;
    icdDispatch.SetMemObjectDestructorCallback = functionPointers.clSetMemObjectDestructorCallback;
    icdDispatch.CreateSampler = functionPointers.clCreateSampler;
    icdDispatch.RetainSampler = functionPointers.clRetainSampler;
    icdDispatch.ReleaseSampler = functionPointers.clReleaseSampler;
    icdDispatch.GetSamplerInfo = functionPointers.clGetSamplerInfo;
    icdDispatch.CreateProgramWithSource = functionPointers.clCreateProgramWithSource;
    icdDispatch.CreateProgramWithBinary = functionPointers.clCreateProgramWithBinary;
    icdDispatch.CreateProgramWithBuiltInKernels = functionPointers.clCreateProgramWithBuiltInKernels;
    icdDispatch.RetainProgram = functionPointers.clRetainProgram;
    icdDispatch.ReleaseProgram = functionPointers.clReleaseProgram;
    icdDispatch.BuildProgram = functionPointers.clBuildProgram;
    icdDispatch.CompileProgram = functionPointers.clCompileProgram;
    icdDispatch.LinkProgram = functionPointers.clLinkProgram;
    icdDispatch.UnloadPlatformCompiler = functionPointers.clUnloadPlatformCompiler;
    icdDispatch.UnloadCompiler = functionPointers.clUnloadCompiler;
    icdDispatch.GetProgramInfo = functionPointers.clGetProgramInfo;
    icdDispatch.GetProgramBuildInfo = functionPointers.clGetProgramBuildInfo;
    icdDispatch.CreateKernel = functionPointers.clCreateKernel;
    icdDispatch.CreateKernelsInProgram = functionPointers.clCreateKernelsInProgram;
    icdDispatch.RetainKernel = functionPointers.clRetainKernel;
    icdDispatch.ReleaseKernel = functionPointers.clReleaseKernel;
    icdDispatch.SetKernelArg = functionPointers.clSetKernelArg;
    icdDispatch.GetKernelInfo = functionPointers.clGetKernelInfo;
    icdDispatch.GetKernelArgInfo = functionPointers.clGetKernelArgInfo;
    icdDispatch.GetKernelWorkGroupInfo = functionPointers.clGetKernelWorkGroupInfo;
    icdDispatch.WaitForEvents = functionPointers.clWaitForEvents;
    icdDispatch.GetEventInfo = functionPointers.clGetEventInfo;
    icdDispatch.CreateUserEvent = functionPointers.clCreateUserEvent;
    icdDispatch.RetainEvent = functionPointers.clRetainEvent;
    icdDispatch.ReleaseEvent = functionPointers.clReleaseEvent;
    icdDispatch.SetUserEventStatus = functionPointers.clSetUserEventStatus;
    icdDispatch.SetEventCallback = functionPointers.clSetEventCallback;
    icdDispatch.GetEventProfilingInfo = functionPointers.clGetEventProfilingInfo;
    icdDispatch.Flush = functionPointers.clFlush;
    icdDispatch.Finish = functionPointers.clFinish;
    icdDispatch.EnqueueReadBuffer = functionPointers.clEnqueueReadBuffer;
    icdDispatch.EnqueueReadBufferRect = functionPointers.clEnqueueReadBufferRect;
    icdDispatch.EnqueueWriteBuffer = functionPointers.clEnqueueWriteBuffer;
    icdDispatch.EnqueueWriteBufferRect = functionPointers.clEnqueueWriteBufferRect;
    icdDispatch.EnqueueFillBuffer = functionPointers.clEnqueueFillBuffer;
    icdDispatch.EnqueueCopyBuffer = functionPointers.clEnqueueCopyBuffer;
    icdDispatch.EnqueueCopyBufferRect = functionPointers.clEnqueueCopyBufferRect;
    icdDispatch.EnqueueReadImage = functionPointers.clEnqueueReadImage;
    icdDispatch.EnqueueWriteImage = functionPointers.clEnqueueWriteImage;
    icdDispatch.EnqueueFillImage = functionPointers.clEnqueueFillImage;
    icdDispatch.EnqueueCopyImage = functionPointers.clEnqueueCopyImage;
    icdDispatch.EnqueueCopyImageToBuffer = functionPointers.clEnqueueCopyImageToBuffer;
    icdDispatch.EnqueueCopyBufferToImage = functionPointers.clEnqueueCopyBufferToImage;
    icdDispatch.EnqueueMapBuffer = functionPointers.clEnqueueMapBuffer;
    icdDispatch.EnqueueMapImage = functionPointers.clEnqueueMapImage;
    icdDispatch.EnqueueUnmapMemObject = functionPointers.clEnqueueUnmapMemObject;
    icdDispatch.EnqueueMigrateMemObjects = functionPointers.clEnqueueMigrateMemObjects;
    icdDispatch.EnqueueNDRangeKernel = functionPointers.clEnqueueNDRangeKernel;
    icdDispatch.EnqueueTask = functionPointers.clEnqueueTask;
    icdDispatch.EnqueueMarkerWithWaitList = functionPointers.clEnqueueMarkerWithWaitList;
    icdDispatch.EnqueueBarrierWithWaitList = functionPointers.clEnqueueBarrierWithWaitList;
    // icdDispatch.SetPrintfCallback = functionPointers.clSetPrintfCallback;
    icdDispatch.GetExtensionFunctionAddressForPlatform = functionPointers.clGetExtensionFunctionAddressForPlatform;
    icdDispatch.EnqueueNativeKernel = functionPointers.clEnqueueNativeKernel;
    icdDispatch.EnqueueMarker = functionPointers.clEnqueueMarker;
    icdDispatch.EnqueueWaitForEvents = functionPointers.clEnqueueWaitForEvents;
    icdDispatch.EnqueueBarrier = functionPointers.clEnqueueBarrier;
    icdDispatch.GetExtensionFunctionAddress = functionPointers.clGetExtensionFunctionAddress;
    icdDispatch.CreateFromGLBuffer = functionPointers.clCreateFromGLBuffer;
    icdDispatch.CreateFromGLTexture = functionPointers.clCreateFromGLTexture;
    icdDispatch.CreateFromGLTexture2D = functionPointers.clCreateFromGLTexture2D;
    icdDispatch.CreateFromGLTexture3D = functionPointers.clCreateFromGLTexture3D;
    icdDispatch.CreateFromGLRenderbuffer = functionPointers.clCreateFromGLRenderbuffer;
    icdDispatch.GetGLObjectInfo = functionPointers.clGetGLObjectInfo;
    icdDispatch.GetGLTextureInfo = functionPointers.clGetGLTextureInfo;
    icdDispatch.EnqueueAcquireGLObjects = functionPointers.clEnqueueAcquireGLObjects;
    icdDispatch.EnqueueReleaseGLObjects = functionPointers.clEnqueueReleaseGLObjects;
    icdDispatch.GetGLContextInfoKHR = functionPointers.clGetGLContextInfoKHR;
    // icdDispatch.CreateEventFromGLsyncKHR = functionPointers.clCreateEventFromGLsyncKHR;

    // OpenCL 2.0
    icdDispatch.CreateCommandQueueWithProperties = functionPointers.clCreateCommandQueueWithProperties;
    icdDispatch.CreatePipe = functionPointers.clCreatePipe;
    icdDispatch.GetPipeInfo = functionPointers.clGetPipeInfo;
    icdDispatch.SVMAlloc = functionPointers.clSVMAlloc;
    icdDispatch.SVMFree = functionPointers.clSVMFree;
    icdDispatch.EnqueueSVMFree = functionPointers.clEnqueueSVMFree;
    icdDispatch.EnqueueSVMMemcpy = functionPointers.clEnqueueSVMMemcpy;
    icdDispatch.EnqueueSVMMemFill = functionPointers.clEnqueueSVMMemFill;
    icdDispatch.EnqueueSVMMap = functionPointers.clEnqueueSVMMap;
    icdDispatch.EnqueueSVMUnmap = functionPointers.clEnqueueSVMUnmap;
    icdDispatch.CreateSamplerWithProperties = functionPointers.clCreateSamplerWithProperties;
    icdDispatch.SetKernelArgSVMPointer = functionPointers.clSetKernelArgSVMPointer;
    icdDispatch.SetKernelExecInfo = functionPointers.clSetKernelExecInfo;
    // icdDispatch.GetKernelSubGroupInfoKHR = functionPointers.clGetKernelSubGroupInfoKHR;
    // icdDispatch.TerminateContextKHR = functionPointers.clTerminateContextKHR;

    // Verify there are no new members in the struct:
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    GT_ASSERT((sizeof(cl_icd_dispatch_table) == 138 * sizeof(void*)));
#endif
}

