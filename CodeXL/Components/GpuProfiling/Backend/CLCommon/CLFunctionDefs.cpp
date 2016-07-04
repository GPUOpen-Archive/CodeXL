//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief The helper file to initialize the OpenCL function pointers.
//==============================================================================

#include <cstring>

#include "CLFunctionDefs.h"
#include "OpenCLModule.h"
#include <cstring>

cl_icd_dispatch_table g_nextDispatchTable;
cl_icd_dispatch_table g_realDispatchTable;

CLExtensionFunctionTable g_realExtensionFunctionTable;

void InitNextCLFunctions(cl_icd_dispatch_table& table)
{
    InitRealCLFunctions();

    memcpy(&g_nextDispatchTable, &table, sizeof(g_nextDispatchTable));
}

#ifdef _WIN32
   #if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        #define AMDOCL_MODULE_NAME "amdocl64.dll"
    #else
        #define AMDOCL_MODULE_NAME "amdocl.dll"
    #endif
    #define FALLBACK_AMDOCL_MODULE_NAME ""
#else
    #ifdef __x86_64__
        #define AMDOCL_MODULE_NAME "libamdocl64.so"
        #define FALLBACK_AMDOCL_MODULE_NAME "libatiocl64.so"
    #else
        #define AMDOCL_MODULE_NAME "libamdocl32.so"
        #define FALLBACK_AMDOCL_MODULE_NAME "libatiocl64.so"
    #endif
#endif


void InitRealCLFunctions()
{
    /// static flag indicating if we've initialized the g_realDispatchTable with the amd ocl entry points
    static bool g_OpenCLModuleInit = false;

    /// static instance used to get entry points from amdocl lib
    static OpenCLModule g_OpenCLModule(AMDOCL_MODULE_NAME);

    if (!g_OpenCLModuleInit)
    {
        if (g_OpenCLModule.OpenCLLoaded() == OpenCLModule::OpenCL_None)
        {
            // AMDOCL_MODULE_NAME not loaded, try to load FALLBACK_AMDOCL_MODULE_NAME
            g_OpenCLModule.UnloadModule();
            g_OpenCLModule.LoadModule(FALLBACK_AMDOCL_MODULE_NAME);
        }

        if (g_OpenCLModule.OpenCLLoaded() == OpenCLModule::OpenCL_None)
        {
            return;
        }

        g_OpenCLModuleInit = true;
        g_realDispatchTable.GetPlatformIDs = g_OpenCLModule.GetPlatformIDs;
        g_realDispatchTable.GetPlatformInfo = g_OpenCLModule.GetPlatformInfo;
        g_realDispatchTable.GetDeviceIDs = g_OpenCLModule.GetDeviceIDs;
        g_realDispatchTable.GetDeviceInfo = g_OpenCLModule.GetDeviceInfo;
        g_realDispatchTable.CreateContext = g_OpenCLModule.CreateContext;
        g_realDispatchTable.CreateContextFromType = g_OpenCLModule.CreateContextFromType;
        g_realDispatchTable.RetainContext = g_OpenCLModule.RetainContext;
        g_realDispatchTable.ReleaseContext = g_OpenCLModule.ReleaseContext;
        g_realDispatchTable.GetContextInfo = g_OpenCLModule.GetContextInfo;
        g_realDispatchTable.CreateCommandQueue = g_OpenCLModule.CreateCommandQueue;
        g_realDispatchTable.RetainCommandQueue = g_OpenCLModule.RetainCommandQueue;
        g_realDispatchTable.ReleaseCommandQueue = g_OpenCLModule.ReleaseCommandQueue;
        g_realDispatchTable.GetCommandQueueInfo = g_OpenCLModule.GetCommandQueueInfo;
        g_realDispatchTable.SetCommandQueueProperty = g_OpenCLModule.SetCommandQueueProperty;
        g_realDispatchTable.CreateProgramWithSource = g_OpenCLModule.CreateProgramWithSource;
        g_realDispatchTable.CreateProgramWithBinary = g_OpenCLModule.CreateProgramWithBinary;
        g_realDispatchTable.RetainProgram = g_OpenCLModule.RetainProgram;
        g_realDispatchTable.ReleaseProgram = g_OpenCLModule.ReleaseProgram;
        g_realDispatchTable.BuildProgram = g_OpenCLModule.BuildProgram;
        g_realDispatchTable.UnloadCompiler = g_OpenCLModule.UnloadCompiler;
        g_realDispatchTable.GetProgramInfo = g_OpenCLModule.GetProgramInfo;
        g_realDispatchTable.GetProgramBuildInfo = g_OpenCLModule.GetProgramBuildInfo;
        g_realDispatchTable.CreateKernel = g_OpenCLModule.CreateKernel;
        g_realDispatchTable.CreateKernelsInProgram = g_OpenCLModule.CreateKernelsInProgram;
        g_realDispatchTable.RetainKernel = g_OpenCLModule.RetainKernel;
        g_realDispatchTable.ReleaseKernel = g_OpenCLModule.ReleaseKernel;
        g_realDispatchTable.SetKernelArg = g_OpenCLModule.SetKernelArg;
        g_realDispatchTable.GetKernelInfo = g_OpenCLModule.GetKernelInfo;
        g_realDispatchTable.GetKernelWorkGroupInfo = g_OpenCLModule.GetKernelWorkGroupInfo;
        g_realDispatchTable.WaitForEvents = g_OpenCLModule.WaitForEvents;
        g_realDispatchTable.GetEventInfo = g_OpenCLModule.GetEventInfo;
        g_realDispatchTable.RetainEvent = g_OpenCLModule.RetainEvent;
        g_realDispatchTable.ReleaseEvent = g_OpenCLModule.ReleaseEvent;
        g_realDispatchTable.GetEventProfilingInfo = g_OpenCLModule.GetEventProfilingInfo;
        g_realDispatchTable.Flush = g_OpenCLModule.Flush;
        g_realDispatchTable.Finish = g_OpenCLModule.Finish;
        g_realDispatchTable.EnqueueNDRangeKernel = g_OpenCLModule.EnqueueNDRangeKernel;
        g_realDispatchTable.EnqueueTask = g_OpenCLModule.EnqueueTask;
        g_realDispatchTable.EnqueueNativeKernel = g_OpenCLModule.EnqueueNativeKernel;
        g_realDispatchTable.EnqueueMarker = g_OpenCLModule.EnqueueMarker;
        g_realDispatchTable.EnqueueWaitForEvents = g_OpenCLModule.EnqueueWaitForEvents;
        g_realDispatchTable.EnqueueBarrier = g_OpenCLModule.EnqueueBarrier;
        g_realDispatchTable.CreateBuffer = g_OpenCLModule.CreateBuffer;
        g_realDispatchTable.CreateImage2D = g_OpenCLModule.CreateImage2D;
        g_realDispatchTable.CreateImage3D = g_OpenCLModule.CreateImage3D;
        g_realDispatchTable.RetainMemObject = g_OpenCLModule.RetainMemObject;
        g_realDispatchTable.ReleaseMemObject = g_OpenCLModule.ReleaseMemObject;
        g_realDispatchTable.GetSupportedImageFormats = g_OpenCLModule.GetSupportedImageFormats;
        g_realDispatchTable.GetMemObjectInfo = g_OpenCLModule.GetMemObjectInfo;
        g_realDispatchTable.GetImageInfo = g_OpenCLModule.GetImageInfo;
        g_realDispatchTable.CreateSampler = g_OpenCLModule.CreateSampler;
        g_realDispatchTable.RetainSampler = g_OpenCLModule.RetainSampler;
        g_realDispatchTable.ReleaseSampler = g_OpenCLModule.ReleaseSampler;
        g_realDispatchTable.GetSamplerInfo = g_OpenCLModule.GetSamplerInfo;
        g_realDispatchTable.EnqueueReadBuffer = g_OpenCLModule.EnqueueReadBuffer;
        g_realDispatchTable.EnqueueWriteBuffer = g_OpenCLModule.EnqueueWriteBuffer;
        g_realDispatchTable.EnqueueReadImage = g_OpenCLModule.EnqueueReadImage;
        g_realDispatchTable.EnqueueWriteImage = g_OpenCLModule.EnqueueWriteImage;
        g_realDispatchTable.EnqueueMapBuffer = g_OpenCLModule.EnqueueMapBuffer;
        g_realDispatchTable.EnqueueMapImage = g_OpenCLModule.EnqueueMapImage;
        g_realDispatchTable.EnqueueUnmapMemObject = g_OpenCLModule.EnqueueUnmapMemObject;
        g_realDispatchTable.EnqueueCopyBuffer = g_OpenCLModule.EnqueueCopyBuffer;
        g_realDispatchTable.EnqueueCopyImage = g_OpenCLModule.EnqueueCopyImage;
        g_realDispatchTable.EnqueueCopyImageToBuffer = g_OpenCLModule.EnqueueCopyImageToBuffer;
        g_realDispatchTable.EnqueueCopyBufferToImage = g_OpenCLModule.EnqueueCopyBufferToImage;
        // OpenCL 1.1
        g_realDispatchTable.CreateUserEvent = g_OpenCLModule.CreateUserEvent;
        g_realDispatchTable.SetUserEventStatus = g_OpenCLModule.SetUserEventStatus;
        g_realDispatchTable.SetEventCallback = g_OpenCLModule.SetEventCallback;
        g_realDispatchTable.CreateSubBuffer = g_OpenCLModule.CreateSubBuffer;
        g_realDispatchTable.SetMemObjectDestructorCallback = g_OpenCLModule.SetMemObjectDestructorCallback;
        g_realDispatchTable.EnqueueReadBufferRect = g_OpenCLModule.EnqueueReadBufferRect;
        g_realDispatchTable.EnqueueWriteBufferRect = g_OpenCLModule.EnqueueWriteBufferRect;
        g_realDispatchTable.EnqueueCopyBufferRect = g_OpenCLModule.EnqueueCopyBufferRect;

        g_realDispatchTable.CreateFromGLBuffer = g_OpenCLModule.CreateFromGLBuffer;
        g_realDispatchTable.CreateFromGLTexture2D = g_OpenCLModule.CreateFromGLTexture2D;
        g_realDispatchTable.CreateFromGLTexture3D = g_OpenCLModule.CreateFromGLTexture3D;
        g_realDispatchTable.CreateFromGLRenderbuffer = g_OpenCLModule.CreateFromGLRenderbuffer;
        g_realDispatchTable.GetGLObjectInfo = g_OpenCLModule.GetGLObjectInfo;
        g_realDispatchTable.GetGLTextureInfo = g_OpenCLModule.GetGLTextureInfo;
        g_realDispatchTable.EnqueueAcquireGLObjects = g_OpenCLModule.EnqueueAcquireGLObjects;
        g_realDispatchTable.EnqueueReleaseGLObjects = g_OpenCLModule.EnqueueReleaseGLObjects;
        g_realDispatchTable.GetGLContextInfoKHR = g_OpenCLModule.GetGLContextInfoKHR;
        // clCreateEventFromGLsyncKHR is not an addition to OpenCL 1.2, but the
        // dispatch table in pre 1.2 versions did not contain an entry for this API
        g_realDispatchTable.CreateEventFromGLsyncKHR = g_OpenCLModule.CreateEventFromGLsyncKHR;
        g_realDispatchTable.GetExtensionFunctionAddress = g_OpenCLModule.GetExtensionFunctionAddress;

        g_realDispatchTable._reservedForDeviceFissionEXT[0] = (void*)g_OpenCLModule.CreateSubDevicesEXT;
        g_realDispatchTable._reservedForDeviceFissionEXT[1] = (void*)g_OpenCLModule.RetainDeviceEXT;
        g_realDispatchTable._reservedForDeviceFissionEXT[2] = (void*)g_OpenCLModule.ReleaseDeviceEXT;
#ifdef _WIN32
        g_realDispatchTable._reservedForD3D10KHR[0] = (void*)g_OpenCLModule.GetDeviceIDsFromD3D10KHR;
        g_realDispatchTable._reservedForD3D10KHR[1] = (void*)g_OpenCLModule.CreateFromD3D10BufferKHR;
        g_realDispatchTable._reservedForD3D10KHR[2] = (void*)g_OpenCLModule.CreateFromD3D10Texture2DKHR;
        g_realDispatchTable._reservedForD3D10KHR[3] = (void*)g_OpenCLModule.CreateFromD3D10Texture3DKHR;
        g_realDispatchTable._reservedForD3D10KHR[4] = (void*)g_OpenCLModule.EnqueueAcquireD3D10ObjectsKHR;
        g_realDispatchTable._reservedForD3D10KHR[5] = (void*)g_OpenCLModule.EnqueueReleaseD3D10ObjectsKHR;
#endif
        // OpenCL 1.2
        g_realDispatchTable.CreateSubDevices = g_OpenCLModule.CreateSubDevices;
        g_realDispatchTable.RetainDevice = g_OpenCLModule.RetainDevice;
        g_realDispatchTable.ReleaseDevice = g_OpenCLModule.ReleaseDevice;
        g_realDispatchTable.CreateImage = g_OpenCLModule.CreateImage;
        g_realDispatchTable.CreateProgramWithBuiltInKernels = g_OpenCLModule.CreateProgramWithBuiltInKernels;
        g_realDispatchTable.CompileProgram = g_OpenCLModule.CompileProgram;
        g_realDispatchTable.LinkProgram = g_OpenCLModule.LinkProgram;
        g_realDispatchTable.UnloadPlatformCompiler = g_OpenCLModule.UnloadPlatformCompiler;
        g_realDispatchTable.GetKernelArgInfo = g_OpenCLModule.GetKernelArgInfo;
        g_realDispatchTable.EnqueueFillBuffer = g_OpenCLModule.EnqueueFillBuffer;
        g_realDispatchTable.EnqueueFillImage = g_OpenCLModule.EnqueueFillImage;
        g_realDispatchTable.EnqueueMigrateMemObjects = g_OpenCLModule.EnqueueMigrateMemObjects;
        g_realDispatchTable.EnqueueMarkerWithWaitList = g_OpenCLModule.EnqueueMarkerWithWaitList;
        g_realDispatchTable.EnqueueBarrierWithWaitList = g_OpenCLModule.EnqueueBarrierWithWaitList;
        g_realDispatchTable.GetExtensionFunctionAddressForPlatform = g_OpenCLModule.GetExtensionFunctionAddressForPlatform;
        g_realDispatchTable.CreateFromGLTexture = g_OpenCLModule.CreateFromGLTexture;

        g_realDispatchTable._reservedD3DExtensions[0] = g_OpenCLModule._reservedD3DExtensions[0];
        g_realDispatchTable._reservedD3DExtensions[1] = g_OpenCLModule._reservedD3DExtensions[1];
        g_realDispatchTable._reservedD3DExtensions[2] = g_OpenCLModule._reservedD3DExtensions[2];
        g_realDispatchTable._reservedD3DExtensions[3] = g_OpenCLModule._reservedD3DExtensions[3];
        g_realDispatchTable._reservedD3DExtensions[4] = g_OpenCLModule._reservedD3DExtensions[4];
        g_realDispatchTable._reservedD3DExtensions[5] = g_OpenCLModule._reservedD3DExtensions[5];
        g_realDispatchTable._reservedD3DExtensions[6] = g_OpenCLModule._reservedD3DExtensions[6];
        g_realDispatchTable._reservedD3DExtensions[7] = g_OpenCLModule._reservedD3DExtensions[7];
        g_realDispatchTable._reservedD3DExtensions[8] = g_OpenCLModule._reservedD3DExtensions[8];
        g_realDispatchTable._reservedD3DExtensions[9] = g_OpenCLModule._reservedD3DExtensions[9];

        g_realDispatchTable._reservedEGLExtensions[0] = g_OpenCLModule._reservedEGLExtensions[0];
        g_realDispatchTable._reservedEGLExtensions[1] = g_OpenCLModule._reservedEGLExtensions[1];
        g_realDispatchTable._reservedEGLExtensions[2] = g_OpenCLModule._reservedEGLExtensions[2];
        g_realDispatchTable._reservedEGLExtensions[3] = g_OpenCLModule._reservedEGLExtensions[3];

        g_realDispatchTable.CreateCommandQueueWithProperties = g_OpenCLModule.CreateCommandQueueWithProperties;
        g_realDispatchTable.CreatePipe = g_OpenCLModule.CreatePipe;
        g_realDispatchTable.GetPipeInfo = g_OpenCLModule.GetPipeInfo;
        g_realDispatchTable.SVMAlloc = g_OpenCLModule.SVMAlloc;
        g_realDispatchTable.SVMFree = g_OpenCLModule.SVMFree;
        g_realDispatchTable.EnqueueSVMFree = g_OpenCLModule.EnqueueSVMFree;
        g_realDispatchTable.EnqueueSVMMemcpy = g_OpenCLModule.EnqueueSVMMemcpy;
        g_realDispatchTable.EnqueueSVMMemFill = g_OpenCLModule.EnqueueSVMMemFill;
        g_realDispatchTable.EnqueueSVMMap = g_OpenCLModule.EnqueueSVMMap;
        g_realDispatchTable.EnqueueSVMUnmap = g_OpenCLModule.EnqueueSVMUnmap;
        g_realDispatchTable.CreateSamplerWithProperties = g_OpenCLModule.CreateSamplerWithProperties;
        g_realDispatchTable.SetKernelArgSVMPointer = g_OpenCLModule.SetKernelArgSVMPointer;
        g_realDispatchTable.SetKernelExecInfo = g_OpenCLModule.SetKernelExecInfo;
    }
}

CL_FUNC_TYPE InitExtensionFunction(const char* pFuncName, void* pFuncPtr)
{
    CL_FUNC_TYPE retVal = CL_FUNC_TYPE_Unknown;

    if (strcmp(pFuncName, "clSVMAllocAMD") == 0)
    {
        retVal = CL_FUNC_TYPE_clSVMAllocAMD;
        g_realExtensionFunctionTable.SVMAllocAMD = (clSVMAllocAMD_fn)pFuncPtr;
    }
    else if (strcmp(pFuncName, "clSVMFreeAMD") == 0)
    {
        retVal = CL_FUNC_TYPE_clSVMFreeAMD;
        g_realExtensionFunctionTable.SVMFreeAMD = (clSVMFreeAMD_fn)pFuncPtr;
    }
    else if (strcmp(pFuncName, "clEnqueueSVMFreeAMD") == 0)
    {
        retVal = CL_FUNC_TYPE_clEnqueueSVMFreeAMD;
        g_realExtensionFunctionTable.EnqueueSVMFreeAMD = (clEnqueueSVMFreeAMD_fn)pFuncPtr;
    }
    else if (strcmp(pFuncName, "clEnqueueSVMMemcpyAMD") == 0)
    {
        retVal = CL_FUNC_TYPE_clEnqueueSVMMemcpyAMD;
        g_realExtensionFunctionTable.EnqueueSVMMemcpyAMD = (clEnqueueSVMMemcpyAMD_fn)pFuncPtr;
    }
    else if (strcmp(pFuncName, "clEnqueueSVMMemFillAMD") == 0)
    {
        retVal = CL_FUNC_TYPE_clEnqueueSVMMemFillAMD;
        g_realExtensionFunctionTable.EnqueueSVMMemFillAMD = (clEnqueueSVMMemFillAMD_fn)pFuncPtr;
    }
    else if (strcmp(pFuncName, "clEnqueueSVMMapAMD") == 0)
    {
        retVal = CL_FUNC_TYPE_clEnqueueSVMMapAMD;
        g_realExtensionFunctionTable.EnqueueSVMMapAMD = (clEnqueueSVMMapAMD_fn)pFuncPtr;
    }
    else if (strcmp(pFuncName, "clEnqueueSVMUnmapAMD") == 0)
    {
        retVal = CL_FUNC_TYPE_clEnqueueSVMUnmapAMD;
        g_realExtensionFunctionTable.EnqueueSVMUnmapAMD = (clEnqueueSVMUnmapAMD_fn)pFuncPtr;
    }

    if (strcmp(pFuncName, "clSetKernelArgSVMPointerAMD") == 0)
    {
        retVal = CL_FUNC_TYPE_clSetKernelArgSVMPointerAMD;
        g_realExtensionFunctionTable.SetKernelArgSVMPointerAMD = (clSetKernelArgSVMPointerAMD_fn)pFuncPtr;
    }
    else if (strcmp(pFuncName, "clSetKernelExecInfoAMD") == 0)
    {
        retVal = CL_FUNC_TYPE_clSetKernelExecInfoAMD;
        g_realExtensionFunctionTable.SetKernelExecInfoAMD = (clSetKernelExecInfoAMD_fn)pFuncPtr;
    }

    return retVal;
}
