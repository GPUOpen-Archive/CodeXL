//==============================================================================
// Copyright (c) 2012-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools
/// \file
/// \brief This class manages the dynamic loading of OpenCL.
//==============================================================================

#ifndef _OPENCL_MODULE_H_
#define _OPENCL_MODULE_H_

#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <CL/additions/cl_additions.h>
#include <CL/internal/cl_icd_amd.h>
#include <CL/internal/cl_profile_amd.h>
#include <CL/internal/cl_kernel_info_amd.h>
#ifdef _WIN32
    #include <CL/cl_dx9_media_sharing.h>
    #include <d3d10_1.h>
    #include <CL/cl_d3d10.h>
#endif
#include "DynamicLibraryModule.h"

// these are missing from CL\internal\cl_profile_amd.h
typedef CL_API_ENTRY cl_perfcounter_amd
(CL_API_CALL* clCreatePerfCounterAMD_fn)(
    cl_device_id                /* device */,
    cl_perfcounter_property*    /* properties */,
    cl_int*                     /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int
(CL_API_CALL* clReleasePerfCounterAMD_fn)(
    cl_perfcounter_amd  /* perf_counter */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int
(CL_API_CALL* clRetainPerfCounterAMD_fn)(
    cl_perfcounter_amd  /* perf_counter */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int
(CL_API_CALL* clEnqueueBeginPerfCounterAMD_fn)(
    cl_command_queue    /* command_queue */,
    cl_uint             /* num_perf_counters */,
    cl_perfcounter_amd* /* perf_counters */,
    cl_uint             /* num_events_in_wait_list */,
    const cl_event*     /* event_wait_list */,
    cl_event*           /* event */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int
(CL_API_CALL* clEnqueueEndPerfCounterAMD_fn)(
    cl_command_queue    /* command_queue */,
    cl_uint             /* num_perf_counters */,
    cl_perfcounter_amd* /* perf_counters */,
    cl_uint             /* num_events_in_wait_list */,
    const cl_event*     /* event_wait_list */,
    cl_event*           /* event */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int
(CL_API_CALL* clGetPerfCounterInfoAMD_fn)(
    cl_perfcounter_amd  /* perf_counter */,
    cl_perfcounter_info /* param_name */,
    size_t              /* param_value_size */,
    void*               /* param_value */,
    size_t*             /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;


// Gleaned from drivers\opencl\api\opencl\amdocl\cl_thread_trace_amd.h
typedef struct _cl_threadtrace_amd* cl_threadtrace_amd;
typedef CL_API_ENTRY cl_threadtrace_amd
(CL_API_CALL* clCreateThreadTraceAMD_fn)(
    cl_device_id                /* device */,
    cl_int*                     /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int
(CL_API_CALL* clReleaseThreadTraceAMD_fn)(
    cl_threadtrace_amd  /* threadTrace */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int
(CL_API_CALL* clRetainThreadTraceAMD_fn)(
    cl_threadtrace_amd  /* threadTrace */) CL_API_SUFFIX__VERSION_1_0;

typedef cl_uint cl_thread_trace_param;
typedef CL_API_ENTRY cl_int
(CL_API_CALL* clSetThreadTraceParamAMD_fn)(
    cl_threadtrace_amd    /*thread_trace*/ ,
    cl_thread_trace_param /*config_param*/ ,
    cl_uint                /*param_value*/)CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int
(CL_API_CALL* clEnqueueBindThreadTraceBufferAMD_fn)(
    cl_command_queue    command_queue ,
    cl_threadtrace_amd  /*thread_trace*/ ,
    cl_mem*             /*mem_objects*/ ,
    cl_uint             /*mem_objects_num*/ ,
    cl_uint             /*buffer_size*/ ,
    cl_uint             /*num_events_in_wait_list*/ ,
    const cl_event*     /*event_wait_list*/ ,
    cl_event*           /*event*/) CL_API_SUFFIX__VERSION_1_0;

typedef cl_uint cl_threadtrace_info;
typedef CL_API_ENTRY cl_int
(CL_API_CALL* clGetThreadTraceInfoAMD_fn)(
    cl_threadtrace_amd    /* thread_trace */,
    cl_threadtrace_info   /*thread_trace_info_param*/,
    size_t                /*param_value_size*/,
    void*                 /*param_value*/,
    size_t*               /*param_value_size_ret*/) CL_API_SUFFIX__VERSION_1_0;

typedef enum  _cl_threadtrace_command_name_amd
{
    CL_THREAD_TRACE_BEGIN_COMMAND,
    CL_THREAD_TRACE_END_COMMAND,
    CL_THREAD_TRACE_PAUSE_COMMAND,
    CL_THREAD_TRACE_RESUME_COMMAND
} cl_threadtrace_command_name_amd;

typedef CL_API_ENTRY cl_int
(CL_API_CALL* clEnqueueThreadTraceCommandAMD_fn)(
    cl_command_queue     /*command_queue*/ ,
    cl_threadtrace_amd   /*thread_trace*/ ,
    cl_threadtrace_command_name_amd /*command_name*/ ,
    cl_uint              /*num_events_in_wait_list*/ ,
    const cl_event*      /*event_wait_list*/,
    cl_event*            /*event*/)CL_API_SUFFIX__VERSION_1_0;


// from drivers\opencl\api\opencl\amdocl\cl_d3d11.cpp
typedef CL_API_ENTRY cl_mem
(CL_API_CALL* clGetPlaneFromImageAMD_fn)(
    cl_context context,
    cl_mem     mem,
    cl_uint    plane,
    cl_int*    errcode_ret) CL_API_SUFFIX__VERSION_1_0;

// missing from cl_kernel_info_amd.h
typedef CL_API_ENTRY cl_int
(CL_API_CALL* clGetKernelInfoAMD_fn)(
    cl_kernel           /* kernel */,
    cl_device_id        /* device */,
    cl_kernel_info_amd  /* param_name */,
    size_t              /* param_value_size */,
    void*               /* param_value */,
    size_t*             /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;


// missing from cl_platform_amd.h
typedef CL_API_ENTRY cl_int
(CL_API_CALL* clUnloadPlatformAMD_fn)(
    cl_platform_id platform) CL_API_SUFFIX__VERSION_1_0;




// These match those in cl_icd_amd.h struct _cl_icd_dispatch_table.
// I've kept them in order.
// We'll handle the reserved items with open code.
// Because we need to talk about these interfaces 4 different times
// (declaration, set, reset, check) enumerate them using X macro tables.
// It saves a lot of typing and potential errors.
// It also makes the code later clearer, I think.

#define OPENCL10_API_TABLE \
    X(GetPlatformIDs) \
    X(GetPlatformInfo) \
    X(GetDeviceIDs) \
    X(GetDeviceInfo) \
    X(CreateContext) \
    X(CreateContextFromType) \
    X(RetainContext) \
    X(ReleaseContext) \
    X(GetContextInfo) \
    X(CreateCommandQueue) \
    X(RetainCommandQueue) \
    X(ReleaseCommandQueue) \
    X(GetCommandQueueInfo) \
    X(SetCommandQueueProperty) \
    X(CreateBuffer) \
    X(CreateImage2D) \
    X(CreateImage3D) \
    X(RetainMemObject) \
    X(ReleaseMemObject) \
    X(GetSupportedImageFormats) \
    X(GetMemObjectInfo) \
    X(GetImageInfo) \
    X(CreateSampler) \
    X(RetainSampler) \
    X(ReleaseSampler) \
    X(GetSamplerInfo) \
    X(CreateProgramWithSource) \
    X(CreateProgramWithBinary) \
    X(RetainProgram) \
    X(ReleaseProgram) \
    X(BuildProgram) \
    X(UnloadCompiler) \
    X(GetProgramInfo) \
    X(GetProgramBuildInfo) \
    X(CreateKernel) \
    X(CreateKernelsInProgram) \
    X(RetainKernel) \
    X(ReleaseKernel) \
    X(SetKernelArg) \
    X(GetKernelInfo) \
    X(GetKernelWorkGroupInfo) \
    X(WaitForEvents) \
    X(GetEventInfo) \
    X(RetainEvent) \
    X(ReleaseEvent) \
    X(GetEventProfilingInfo) \
    X(Flush) \
    X(Finish) \
    X(EnqueueReadBuffer) \
    X(EnqueueWriteBuffer) \
    X(EnqueueCopyBuffer) \
    X(EnqueueReadImage) \
    X(EnqueueWriteImage) \
    X(EnqueueCopyImage) \
    X(EnqueueCopyImageToBuffer) \
    X(EnqueueCopyBufferToImage) \
    X(EnqueueMapBuffer) \
    X(EnqueueMapImage) \
    X(EnqueueUnmapMemObject) \
    X(EnqueueNDRangeKernel) \
    X(EnqueueTask) \
    X(EnqueueNativeKernel) \
    X(EnqueueMarker) \
    X(EnqueueWaitForEvents) \
    X(EnqueueBarrier) \
    X(GetExtensionFunctionAddress) \
    X(CreateFromGLBuffer) \
    X(CreateFromGLTexture2D) \
    X(CreateFromGLTexture3D) \
    X(CreateFromGLRenderbuffer) \
    X(GetGLObjectInfo) \
    X(GetGLTextureInfo) \
    X(EnqueueAcquireGLObjects) \
    X(EnqueueReleaseGLObjects)

// This entry point seems to have disappeared on Windows and Linux.
// Since the checks run off these tables, we need a special table for this one.
#define OPENCL10_API_TABLE_SPECIAL \
    X(GetGLContextInfoKHR)

#define OPENCL11_API_TABLE \
    X(SetEventCallback) \
    X(CreateSubBuffer) \
    X(SetMemObjectDestructorCallback) \
    X(CreateUserEvent) \
    X(SetUserEventStatus) \
    X(EnqueueReadBufferRect) \
    X(EnqueueWriteBufferRect) \
    X(EnqueueCopyBufferRect)

#define OPENCL12_API_TABLE \
    X(CreateSubDevices) \
    X(RetainDevice) \
    X(ReleaseDevice) \
    X(CreateImage) \
    X(CreateProgramWithBuiltInKernels) \
    X(CompileProgram) \
    X(LinkProgram) \
    X(UnloadPlatformCompiler) \
    X(GetKernelArgInfo) \
    X(EnqueueFillBuffer) \
    X(EnqueueFillImage) \
    X(EnqueueMigrateMemObjects) \
    X(EnqueueMarkerWithWaitList) \
    X(EnqueueBarrierWithWaitList) \
    X(GetExtensionFunctionAddressForPlatform) \
    X(CreateFromGLTexture)

#define OPENCL20_API_TABLE \
    X(CreateCommandQueueWithProperties) \
    X(CreatePipe) \
    X(GetPipeInfo) \
    X(SVMAlloc) \
    X(SVMFree) \
    X(EnqueueSVMFree) \
    X(EnqueueSVMMemcpy) \
    X(EnqueueSVMMemFill) \
    X(EnqueueSVMMap) \
    X(EnqueueSVMUnmap) \
    X(CreateSamplerWithProperties) \
    X(SetKernelArgSVMPointer) \
    X(SetKernelExecInfo)

#ifdef _WIN32
#define D3D10_KHR_RESERVED_TABLE_AVAILABLE 1
#define D3D10_KHR_RESERVED_TABLE \
    X(GetDeviceIDsFromD3D10KHR) \
    X(CreateFromD3D10BufferKHR) \
    X(CreateFromD3D10Texture2DKHR) \
    X(CreateFromD3D10Texture3DKHR) \
    X(EnqueueAcquireD3D10ObjectsKHR) \
    X(EnqueueReleaseD3D10ObjectsKHR)
#define D3D10_KHR_RESERVED_TABLE_2 \
    X2(_reservedForD3D10KHR[0], GetDeviceIDsFromD3D10KHR) \
    X2(_reservedForD3D10KHR[1], CreateFromD3D10BufferKHR) \
    X2(_reservedForD3D10KHR[2], CreateFromD3D10Texture2DKHR) \
    X2(_reservedForD3D10KHR[3], CreateFromD3D10Texture3DKHR) \
    X2(_reservedForD3D10KHR[4], EnqueueAcquireD3D10ObjectsKHR) \
    X2(_reservedForD3D10KHR[5], EnqueueReleaseD3D10ObjectsKHR)
#else
#define D3D10_KHR_RESERVED_TABLE
#define D3D10_KHR_RESERVED_TABLE_2
#endif

#define DEVICE_FISSION_EXT_RESERVE_DTABLE \
    X(CreateSubDevicesEXT) \
    X(RetainDeviceEXT) \
    X(ReleaseDeviceEXT)

#define DEVICE_FISSION_EXT_RESERVE_DTABLE_2 \
    X2(_reservedForDeviceFissionEXT[0], CreateSubDevicesEXT) \
    X2(_reservedForDeviceFissionEXT[1], RetainDeviceEXT) \
    X2(_reservedForDeviceFissionEXT[2], ReleaseDeviceEXT)

#define CREATE_EVENT_FROM_GL_SYNC_RESERVED_TABLE \
    X(CreateEventFromGLsyncKHR)

// The rest of these are OpenCL extensions.
// These get set up with a separate OpenCLModule::LoadExtensions call.
// The call is separate because it requires a cl_platform.
// That platform will be determined by the user of this code.
//
// I have removed the device_fision, GL & d3d10 items that have exposed OpenCL interfaces.
// I think this is OK because everyone should be using OpenCL V1.2 by now.
//
// Cribbed from:
// .../drivers/opencl/api/opencl/amdocl/cl_context.cpp
//
// Here is the documentation of clGetExtensionFunctionAddressForPlatform from that file:
//
//  Returns the address of the extension function named by
//  funcname for a given platform. The pointer returned should be cast
//  to a function pointer type matching the extension function’s definition
//  defined in the appropriate extension specification and header file.
//  A return value of NULL indicates that the specified function does not
//  exist for the implementation or platform is not a valid platform.
//  A non-NULL return value for \a clGetExtensionFunctionAddressForPlatform
//  does not guarantee that an extension function is actually supported by
//  the platform. The application must also make a corresponding query using
//  \a clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, … ) or
//  \a clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, … ) to determine if
//  an extension is supported by the OpenCL implementation.
//
//  \version 1.2r07

#define EXTENSIONS_TABLE \
    X(CreatePerfCounterAMD) \
    X(CreateThreadTraceAMD) \
    X(CreateKeyAMD) \
    X(EnqueueBeginPerfCounterAMD) \
    X(EnqueueEndPerfCounterAMD) \
    X(EnqueueBindThreadTraceBufferAMD) \
    X(EnqueueThreadTraceCommandAMD) \
    X(EnqueueWaitSignalAMD) \
    X(EnqueueWriteSignalAMD) \
    X(EnqueueMakeBuffersResidentAMD) \
    X(GetKernelInfoAMD) \
    X(GetPerfCounterInfoAMD) \
    X(GetThreadTraceInfoAMD) \
    X(IcdGetPlatformIDsKHR) \
    X(ObjectGetValueForKeyAMD) \
    X(ObjectSetValueForKeyAMD) \
    X(ReleasePerfCounterAMD) \
    X(RetainPerfCounterAMD) \
    X(ReleaseThreadTraceAMD) \
    X(RetainThreadTraceAMD) \
    X(SetThreadTraceParamAMD) \
    X(UnloadPlatformAMD) \
    X(GetKernelSubGroupInfoKHR) \
    X(TerminateContextKHR)

// Windows only extensions.
#define WINDOWS_ONLY_EXTENSIONS_TABLE \
    X(CreateFromDX9MediaSurfaceKHR) \
    X(EnqueueAcquireDX9MediaSurfacesKHR) \
    X(EnqueueReleaseDX9MediaSurfacesKHR) \
    X(GetDeviceIDsFromDX9MediaAdapterKHR) \
    X(GetPlaneFromImageAMD)

/// This class handles the dynamic loading of OpenCL.dll/libOpenCL.so.
/// \note There will typically be one of these objects.
///       That instance will be global.
///       There is a trap for the unwary.
///       The order of global ctors is only defined within a single compilation unit.
///       So, one should not use these interfaces before "main" is reached.
///       This is different than calling these functions when the .dll/.so is linked against.
class OpenCLModule
{
public:
    /// Which shared image is loaded?
    enum OpenCLVersion
    {
        OpenCL_None,              ///< No OpenCL version is loaded
        OpenCL_1_0,               ///< OpenCL V1.0 is loaded
        OpenCL_1_1,               ///< OpenCL V1.1 is loaded
        OpenCL_1_2,               ///< OpenCL V1.2 is loaded
        OpenCL_2_0,               ///< OpenCL V2.0 is loaded
    };

    enum OpenCLExt
    {
        OpenCL_Ext_Invalid,        ///< Not a good value.
        OpenCL_khr_d3d10_sharing,  ///< cl_khr_d3d10_sharing extension.
        OpenCL_ext_device_fission, ///< cl_ext_device_fission extension.
        OpenCL_khr_gl_event,       ///< cl_khr_gl_event extension.
        OpenCL_amd_open_video,     ///< cl_amd_open_video
    };

    /// Default name to use for construction.
    /// This is usually OpenCL.dll or libOpenCL.so.
    static const char* s_DefaultModuleName;

    /// Constructor
    /// \param module to load.
    OpenCLModule(const std::string& moduleName = s_DefaultModuleName);

    /// Constructor
    /// \param module to load.  Attempt to load in order, until a success occurs
    OpenCLModule(const std::vector<std::string>& modules);

    /// destructor
    ~OpenCLModule();

    /// Checks if the module is loaded
    bool IsModuleLoaded() const
    {
        return (m_openCLVersion != OpenCL_None);
    }
    /// Load module.
    ///  Uses system default paths for module if just a file name is given.
    ///  Explicit use of amdocl.dll and libamdocl.so will override the ICD
    ///  dispatch table & get the "real" entry points.
    /// \param[in] name The module name (ususally OpenCL.dll or libOpenCL.so).
    /// \return         OpenCL version loaded.
    OpenCLVersion LoadModule(const std::string& name = s_DefaultModuleName);

    /// Attempt to load module (iterate over the names) until a success occurs.
    ///  Uses system default paths for module if just a file name is given.
    ///  Explicit use of amdocl.dll and libamdocl.so will override the ICD
    ///  dispatch table & get the "real" entry points.
    ///  Ex: libOpenCL.so, libOpenCL.so.2 libOpenCL.so.1
    /// \param[in] name The module name (ususally OpenCL.dll or libOpenCL.so).
    /// \return         OpenCL version loaded.
    OpenCLVersion LoadModule(const std::vector<std::string>& names);

    /// Unload OpenCL.dll
    void UnloadModule();

    /// Which OpenCL have we got?
    /// \return enumeration value to answer query.
    OpenCLVersion OpenCLLoaded();

    /// Load OpenCL Extensions.
    /// This will use GetExtensionFunctionAddress or
    /// GetExtensionFunctionAddressForPlatform depending upon OpenCL version
    /// to load all of the extension function pointers that we know about (so far).
    /// After this some of the function pointers may still be NULL.
    /// The user code is responsible for checking the pointers before making calls.
    /// \param   platform  The cl_platform_id we are interested in.
    /// \returns true      if nothing horrible happened.
    bool LoadOpenCLExtensions(cl_platform_id platform);

    /// Check if an extension is supported.
    /// \param ext Which extension are we checking?
    /// \return    supported.
    bool IsExtensionSupported(OpenCLExt ext);

    /// Get currently loaded module's function pointers in the same
    /// form as the OpenCL ICD structure.
    /// \param[out] dispatchTable    Dispatch table to place function pointers into.
    void GetAsCLDispatchTable(cl_icd_dispatch_table& dispatchTable);

    /// Set currently loaded module's function pointers from
    /// an OpenCL ICD structure.
    /// \param[in] dispatchTable    Dispatch table to obtain function pointers from.
    void SetFromCLDispatchTable(const cl_icd_dispatch_table& dispatchTable);

#define X(SYM) cl##SYM##_fn SYM;
    OPENCL10_API_TABLE;
    OPENCL10_API_TABLE_SPECIAL;
    D3D10_KHR_RESERVED_TABLE;
    OPENCL11_API_TABLE;
    DEVICE_FISSION_EXT_RESERVE_DTABLE;
    CREATE_EVENT_FROM_GL_SYNC_RESERVED_TABLE;
    OPENCL12_API_TABLE;
    void*                         _reservedD3DExtensions[10];
    void*                         _reservedEGLExtensions[4];
    OPENCL20_API_TABLE;
    EXTENSIONS_TABLE;
#ifdef _WIN32
    WINDOWS_ONLY_EXTENSIONS_TABLE;
#endif
#undef X

private:
    /// Initialize the internal data
    void Initialize();

    /// Helper.
    DynamicLibraryModule m_DynamicLibraryHelper;

    /// Is module loaded
    OpenCLVersion m_openCLVersion;
};

/// Stream out operator for OpenCLModule.
/// \param[inout] output    The output stream.
/// \param[in]    module    The OpenCLModule to stream out.
/// \return                 The modified output stream.
std::ostream& operator<<(std::ostream& output, const OpenCLModule& module);

#endif
