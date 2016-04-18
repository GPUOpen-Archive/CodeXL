//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file defines the OpenCL function enum values as well as few utility functions
//==============================================================================

#include <map>
#include "CLFunctionEnumDefs.h"
#include "Logger.h"

using namespace std;

static bool s_bInit = false;
static map<string, CL_FUNC_TYPE> s_CLAPIMap;

using namespace GPULogger;

CL_FUNC_TYPE ToCLFuncType(const std::string& strName)
{
    if (!s_bInit)
    {
        s_bInit = true;
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetPlatformIDs"), CL_FUNC_TYPE_clGetPlatformIDs));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetPlatformInfo"), CL_FUNC_TYPE_clGetPlatformInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetDeviceIDs"), CL_FUNC_TYPE_clGetDeviceIDs));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetDeviceInfo"), CL_FUNC_TYPE_clGetDeviceInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateContext"), CL_FUNC_TYPE_clCreateContext));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateContextFromType"), CL_FUNC_TYPE_clCreateContextFromType));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clRetainContext"), CL_FUNC_TYPE_clRetainContext));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clReleaseContext"), CL_FUNC_TYPE_clReleaseContext));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetContextInfo"), CL_FUNC_TYPE_clGetContextInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateCommandQueue"), CL_FUNC_TYPE_clCreateCommandQueue));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clRetainCommandQueue"), CL_FUNC_TYPE_clRetainCommandQueue));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clReleaseCommandQueue"), CL_FUNC_TYPE_clReleaseCommandQueue));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetCommandQueueInfo"), CL_FUNC_TYPE_clGetCommandQueueInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clSetCommandQueueProperty"), CL_FUNC_TYPE_clSetCommandQueueProperty));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateBuffer"), CL_FUNC_TYPE_clCreateBuffer));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateSubBuffer"), CL_FUNC_TYPE_clCreateSubBuffer));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateImage2D"), CL_FUNC_TYPE_clCreateImage2D));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateImage3D"), CL_FUNC_TYPE_clCreateImage3D));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clRetainMemObject"), CL_FUNC_TYPE_clRetainMemObject));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clReleaseMemObject"), CL_FUNC_TYPE_clReleaseMemObject));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetSupportedImageFormats"), CL_FUNC_TYPE_clGetSupportedImageFormats));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetMemObjectInfo"), CL_FUNC_TYPE_clGetMemObjectInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetImageInfo"), CL_FUNC_TYPE_clGetImageInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clSetMemObjectDestructorCallback"), CL_FUNC_TYPE_clSetMemObjectDestructorCallback));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateSampler"), CL_FUNC_TYPE_clCreateSampler));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clRetainSampler"), CL_FUNC_TYPE_clRetainSampler));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clReleaseSampler"), CL_FUNC_TYPE_clReleaseSampler));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetSamplerInfo"), CL_FUNC_TYPE_clGetSamplerInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateProgramWithSource"), CL_FUNC_TYPE_clCreateProgramWithSource));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateProgramWithBinary"), CL_FUNC_TYPE_clCreateProgramWithBinary));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clRetainProgram"), CL_FUNC_TYPE_clRetainProgram));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clReleaseProgram"), CL_FUNC_TYPE_clReleaseProgram));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clBuildProgram"), CL_FUNC_TYPE_clBuildProgram));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clUnloadCompiler"), CL_FUNC_TYPE_clUnloadCompiler));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetProgramInfo"), CL_FUNC_TYPE_clGetProgramInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetProgramBuildInfo"), CL_FUNC_TYPE_clGetProgramBuildInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateKernel"), CL_FUNC_TYPE_clCreateKernel));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateKernelsInProgram"), CL_FUNC_TYPE_clCreateKernelsInProgram));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clRetainKernel"), CL_FUNC_TYPE_clRetainKernel));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clReleaseKernel"), CL_FUNC_TYPE_clReleaseKernel));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clSetKernelArg"), CL_FUNC_TYPE_clSetKernelArg));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetKernelInfo"), CL_FUNC_TYPE_clGetKernelInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetKernelWorkGroupInfo"), CL_FUNC_TYPE_clGetKernelWorkGroupInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clWaitForEvents"), CL_FUNC_TYPE_clWaitForEvents));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetEventInfo"), CL_FUNC_TYPE_clGetEventInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateUserEvent"), CL_FUNC_TYPE_clCreateUserEvent));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clRetainEvent"), CL_FUNC_TYPE_clRetainEvent));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clReleaseEvent"), CL_FUNC_TYPE_clReleaseEvent));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clSetUserEventStatus"), CL_FUNC_TYPE_clSetUserEventStatus));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clSetEventCallback"), CL_FUNC_TYPE_clSetEventCallback));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetEventProfilingInfo"), CL_FUNC_TYPE_clGetEventProfilingInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clFlush"), CL_FUNC_TYPE_clFlush));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clFinish"), CL_FUNC_TYPE_clFinish));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueReadBuffer"), CL_FUNC_TYPE_clEnqueueReadBuffer));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueReadBufferRect"), CL_FUNC_TYPE_clEnqueueReadBufferRect));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueWriteBuffer"), CL_FUNC_TYPE_clEnqueueWriteBuffer));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueWriteBufferRect"), CL_FUNC_TYPE_clEnqueueWriteBufferRect));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueCopyBuffer"), CL_FUNC_TYPE_clEnqueueCopyBuffer));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueCopyBufferRect"), CL_FUNC_TYPE_clEnqueueCopyBufferRect));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueReadImage"), CL_FUNC_TYPE_clEnqueueReadImage));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueWriteImage"), CL_FUNC_TYPE_clEnqueueWriteImage));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueCopyImage"), CL_FUNC_TYPE_clEnqueueCopyImage));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueCopyImageToBuffer"), CL_FUNC_TYPE_clEnqueueCopyImageToBuffer));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueCopyBufferToImage"), CL_FUNC_TYPE_clEnqueueCopyBufferToImage));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueMapBuffer"), CL_FUNC_TYPE_clEnqueueMapBuffer));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueMapImage"), CL_FUNC_TYPE_clEnqueueMapImage));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueUnmapMemObject"), CL_FUNC_TYPE_clEnqueueUnmapMemObject));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueNDRangeKernel"), CL_FUNC_TYPE_clEnqueueNDRangeKernel));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueTask"), CL_FUNC_TYPE_clEnqueueTask));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueNativeKernel"), CL_FUNC_TYPE_clEnqueueNativeKernel));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueMarker"), CL_FUNC_TYPE_clEnqueueMarker));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueWaitForEvents"), CL_FUNC_TYPE_clEnqueueWaitForEvents));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueBarrier"), CL_FUNC_TYPE_clEnqueueBarrier));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateFromGLBuffer"), CL_FUNC_TYPE_clCreateFromGLBuffer));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateFromGLTexture2D"), CL_FUNC_TYPE_clCreateFromGLTexture2D));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateFromGLTexture3D"), CL_FUNC_TYPE_clCreateFromGLTexture3D));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateFromGLRenderbuffer"), CL_FUNC_TYPE_clCreateFromGLRenderbuffer));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetGLObjectInfo"), CL_FUNC_TYPE_clGetGLObjectInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetGLTextureInfo"), CL_FUNC_TYPE_clGetGLTextureInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueAcquireGLObjects"), CL_FUNC_TYPE_clEnqueueAcquireGLObjects));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueReleaseGLObjects"), CL_FUNC_TYPE_clEnqueueReleaseGLObjects));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetGLContextInfoKHR"), CL_FUNC_TYPE_clGetGLContextInfoKHR));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateSubDevicesEXT"), CL_FUNC_TYPE_clCreateSubDevicesEXT));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clRetainDeviceEXT"), CL_FUNC_TYPE_clRetainDeviceEXT));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clReleaseDeviceEXT"), CL_FUNC_TYPE_clReleaseDeviceEXT));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetDeviceIDsFromD3D10KHR"), CL_FUNC_TYPE_clGetDeviceIDsFromD3D10KHR));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateFromD3D10BufferKHR"), CL_FUNC_TYPE_clCreateFromD3D10BufferKHR));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateFromD3D10Texture2DKHR"), CL_FUNC_TYPE_clCreateFromD3D10Texture2DKHR));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateFromD3D10Texture3DKHR"), CL_FUNC_TYPE_clCreateFromD3D10Texture3DKHR));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueAcquireD3D10ObjectsKHR"), CL_FUNC_TYPE_clEnqueueAcquireD3D10ObjectsKHR));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueReleaseD3D10ObjectsKHR"), CL_FUNC_TYPE_clEnqueueReleaseD3D10ObjectsKHR));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateEventFromGLsyncKHR"), CL_FUNC_TYPE_clCreateEventFromGLsyncKHR));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateSubDevices"), CL_FUNC_TYPE_clCreateSubDevices));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clRetainDevice"), CL_FUNC_TYPE_clRetainDevice));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clReleaseDevice"), CL_FUNC_TYPE_clReleaseDevice));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateImage"), CL_FUNC_TYPE_clCreateImage));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateProgramWithBuiltInKernels"), CL_FUNC_TYPE_clCreateProgramWithBuiltInKernels));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCompileProgram"), CL_FUNC_TYPE_clCompileProgram));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clLinkProgram"), CL_FUNC_TYPE_clLinkProgram));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clUnloadPlatformCompiler"), CL_FUNC_TYPE_clUnloadPlatformCompiler));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetKernelArgInfo"), CL_FUNC_TYPE_clGetKernelArgInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueFillBuffer"), CL_FUNC_TYPE_clEnqueueFillBuffer));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueFillImage"), CL_FUNC_TYPE_clEnqueueFillImage));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueMigrateMemObjects"), CL_FUNC_TYPE_clEnqueueMigrateMemObjects));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueMarkerWithWaitList"), CL_FUNC_TYPE_clEnqueueMarkerWithWaitList));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueBarrierWithWaitList"), CL_FUNC_TYPE_clEnqueueBarrierWithWaitList));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetExtensionFunctionAddressForPlatform"), CL_FUNC_TYPE_clGetExtensionFunctionAddressForPlatform));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateFromGLTexture"), CL_FUNC_TYPE_clCreateFromGLTexture));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetExtensionFunctionAddress"), CL_FUNC_TYPE_clGetExtensionFunctionAddress));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateCommandQueueWithProperties"), CL_FUNC_TYPE_clCreateCommandQueueWithProperties));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreateSamplerWithProperties"), CL_FUNC_TYPE_clCreateSamplerWithProperties));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clSVMAlloc"), CL_FUNC_TYPE_clSVMAlloc));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clSVMFree"), CL_FUNC_TYPE_clSVMFree));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clSetKernelArgSVMPointer"), CL_FUNC_TYPE_clSetKernelArgSVMPointer));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clSetKernelExecInfo"), CL_FUNC_TYPE_clSetKernelExecInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueSVMFree"), CL_FUNC_TYPE_clEnqueueSVMFree));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueSVMMemcpy"), CL_FUNC_TYPE_clEnqueueSVMMemcpy));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueSVMMemFill"), CL_FUNC_TYPE_clEnqueueSVMMemFill));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueSVMMap"), CL_FUNC_TYPE_clEnqueueSVMMap));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueSVMUnmap"), CL_FUNC_TYPE_clEnqueueSVMUnmap));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clCreatePipe"), CL_FUNC_TYPE_clCreatePipe));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clGetPipeInfo"), CL_FUNC_TYPE_clGetPipeInfo));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clSVMAllocAMD"), CL_FUNC_TYPE_clSVMAllocAMD));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clSVMFreeAMD"), CL_FUNC_TYPE_clSVMFreeAMD));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueSVMFreeAMD"), CL_FUNC_TYPE_clEnqueueSVMFreeAMD));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueSVMMemcpyAMD"), CL_FUNC_TYPE_clEnqueueSVMMemcpyAMD));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueSVMMemFillAMD"), CL_FUNC_TYPE_clEnqueueSVMMemFillAMD));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueSVMMapAMD"), CL_FUNC_TYPE_clEnqueueSVMMapAMD));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clEnqueueSVMUnmapAMD"), CL_FUNC_TYPE_clEnqueueSVMUnmapAMD));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clSetKernelArgSVMPointerAMD"), CL_FUNC_TYPE_clSetKernelArgSVMPointerAMD));
        s_CLAPIMap.insert(pair<string, CL_FUNC_TYPE>(string("clSetKernelExecInfoAMD"), CL_FUNC_TYPE_clSetKernelExecInfoAMD));
    }

    map<string, CL_FUNC_TYPE>::iterator it = s_CLAPIMap.find(strName);
    SpAssert(it != s_CLAPIMap.end());

    if (it != s_CLAPIMap.end())
    {
        return it->second;
    }
    else
    {
        return CL_FUNC_TYPE_Unknown;
    }
}

bool IsEnqueueAPI(const unsigned int uiAPIId)
{
    return (uiAPIId >= CL_FUNC_TYPE_clEnqueueReadBuffer && uiAPIId <= CL_FUNC_TYPE_clEnqueueMarker) ||
           (uiAPIId >= CL_FUNC_TYPE_clEnqueueFillBuffer && uiAPIId <= CL_FUNC_TYPE_clEnqueueBarrierWithWaitList) ||
           (uiAPIId >= CL_FUNC_TYPE_clEnqueueSVMFree && uiAPIId <= CL_FUNC_TYPE_clEnqueueSVMUnmap) ||
           (uiAPIId >= CL_FUNC_TYPE_clEnqueueSVMFreeAMD && uiAPIId <= CL_FUNC_TYPE_clEnqueueSVMUnmapAMD);
}
