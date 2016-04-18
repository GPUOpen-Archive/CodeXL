//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief The header file for detouring the CL entry functions.
//==============================================================================

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include "CLProfilerMineCLEntry.h"
#include "CLFunctionDefs.h"
#include "CLProfilerMineCLMemory.h"
#include "KernelStats.h"
#include "CLGPAProfiler.h"
#include "CLUtils.h"
#include "CLInternalFunctionDefs.h"
#include "../Common/OSUtils.h"
#include "../Common/GlobalSettings.h"
#include "DeviceInfoUtils.h"

#include "../Common/SeqIDGenerator.h"
#include "AMDTMutex.h"

using std::cout;

CLGPAProfiler g_Profiler;
static AMDTMutex s_mtx("CLProfile");

/// Assigns a real function pointer to an entry in g_realExtensionFunctionTable and
/// returns the Mine_* version
/// \param pFuncName the name of the extension function whose pointer should be assigned
/// \param pRealFuncPtr the address of the real function pointer for the specified extension
/// \return the address of the Mine_ version for the specified extension function
void* AssignExtensionFunctionPointer(const char* pFuncName, void* pRealFuncPtr);

cl_int CL_API_CALL
Mine_clBuildProgram(cl_program          program,
                    cl_uint             nDevices,
                    const cl_device_id* pDevices,
                    const char*         pszOptions,
                    void (CL_CALLBACK*  pfn_notify)(cl_program, void*),
                    void*               pUserData)
{
    SeqIDGenerator::Instance()->GenerateID();
    std::string strOptions = (pszOptions != NULL) ? std::string(pszOptions) : "";

    // request the CL run-time to retain the CPU assembly and
    // the .amdil section (required starting from SDK 2.5) in the elf binary
    strOptions += " -fbin-as -fbin-amdil";

    cl_int ret = g_nextDispatchTable.BuildProgram(program,
                                                  nDevices,
                                                  pDevices,
                                                  strOptions.c_str(),
                                                  pfn_notify,
                                                  pUserData);

    if (CL_INVALID_BUILD_OPTIONS == ret)
    {
        // -fbin-as flag is not supported, let's rebuild the program again
        // Note: calling the nextDispatchTable again may cause other agents to see two BuildProgram calls
        ret = g_nextDispatchTable.BuildProgram(program,
                                               nDevices,
                                               pDevices,
                                               pszOptions,
                                               pfn_notify,
                                               pUserData);
    }

    return ret;
}

cl_context CL_API_CALL
Mine_clCreateContext(const cl_context_properties* pProperties,
                     cl_uint                      nDevices,
                     const cl_device_id*          pDevices,
                     void (CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*),
                     void*                        pUserData,
                     cl_int*                      pErrorCode)
{
    SeqIDGenerator::Instance()->GenerateID();
    cl_context context = g_nextDispatchTable.CreateContext(pProperties,
                                                           nDevices,
                                                           pDevices,
                                                           pfn_notify,
                                                           pUserData,
                                                           pErrorCode);

    if (context == NULL)
    {
        // there is an error with the function so we don't need to continue;
        return context;
    }

    g_Profiler.AddContext(context);

    if (NULL == pDevices)
    {
        return context;
    }

    // set the GPU flag by checking whether there is a GPU device
    if (CLUtils::HasDeviceType(nDevices, pDevices, CL_DEVICE_TYPE_GPU))
    {
        g_Profiler.SetGPUFlag(true);
    }

    return context;
}

cl_context CL_API_CALL
Mine_clCreateContextFromType(const cl_context_properties* pProperties,
                             cl_device_type               deviceType,
                             void (CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*),
                             void*                        pUserData,
                             cl_int*                      pErrorCode)
{
    SeqIDGenerator::Instance()->GenerateID();
    cl_context context = g_nextDispatchTable.CreateContextFromType(pProperties,
                                                                   deviceType,
                                                                   pfn_notify,
                                                                   pUserData,
                                                                   pErrorCode);

    if (context == NULL)
    {
        // there is an error with the function so we don't need to continue;
        return context;
    }

    g_Profiler.AddContext(context);

    if ((deviceType & CL_DEVICE_TYPE_GPU) == CL_DEVICE_TYPE_GPU)
    {
        g_Profiler.SetGPUFlag(true);
    }
    else if (deviceType != CL_DEVICE_TYPE_CPU)
    {
        // for deviceType other than GPU and CPU, we need to check
        // whether the context contains a GPU device
        if (CLUtils::HasDeviceType(context, CL_DEVICE_TYPE_GPU))
        {
            g_Profiler.SetGPUFlag(true);
        }
    }

    return context;
}

cl_kernel CL_API_CALL
Mine_clCreateKernel(cl_program  program,
                    const char* pszKernelName,
                    cl_int*     pErrorCode)
{
    SeqIDGenerator::Instance()->GenerateID();
    cl_kernel kernel = g_nextDispatchTable.CreateKernel(program, pszKernelName, pErrorCode);

    if (pErrorCode != 0 && *pErrorCode != 0)
    {
        return kernel;
    }

    g_Profiler.AddKernel(kernel);

    return kernel;
}

/// Helper function to manage the list of cached kernel indices.  This is used by GetKernelSuffix.
/// This also allows kernels to be removed when a kernel is released.
/// \param kernel a CL kernel instance
/// \param doAdd true if the kernel should be added to the list, false if it should be removed
/// \return index of the item added or removed.  -1 in requested to remove a kernel that was not in the list
static int AddOrRemoveKernel(cl_kernel kernel, bool doAdd)
{
    static AMDTMutex s_Mutex("AddOrRemoveKernel mutex");
    AMDTScopeLock sl(s_Mutex);

    // map a kernel to a unique id
    static std::map<cl_kernel, unsigned int> s_kernelMap;
    // number of kernels for this session
    static unsigned int s_nKernelCount = 0;

    std::map<cl_kernel, unsigned int>::iterator kernelIt = s_kernelMap.find(kernel);
    unsigned int nKernelID;

    if (kernelIt != s_kernelMap.end())
    {
        nKernelID = kernelIt->second;

        if (!doAdd)
        {
            s_kernelMap.erase(kernelIt);
        }
    }
    else if (doAdd)
    {
        nKernelID = ++s_nKernelCount;
        s_kernelMap[kernel] = nKernelID;
    }
    else
    {
        //request was to remove, but this item did not exist in list
        return -1;
    }

    return nKernelID;
}

/// Given a kernel and a device, return a suffix string to uniquely identify
/// the kernel instance.  The returned string will be unique for a given session
/// for unique kernel and device combinations.  This function maps kernels to
/// unique integers and uses those integers in the suffix string.  For devices,
/// this function uses "CPU" for CPU devices, and the actual device name for
/// GPU devices.  For GPU devices, a session-unique integer is also used, in case
/// there is more than one GPU device.
/// \param kernel a CL kernel instance
/// \param device a CL device instance
/// \return string suffix that can be appended to a kernel name to uniquely identify the kernel instance
static std::string GetKernelSuffix(cl_kernel kernel, cl_device_id device)
{
    static AMDTMutex s_Mutex("GetKernelSuffix mutex");
    AMDTScopeLock sl(s_Mutex);
    // map a device to a unique id
    static std::map<cl_device_id, unsigned int> s_deviceMap;

    unsigned int nKernelID = AddOrRemoveKernel(kernel, true);

    std::string strDeviceName;
    bool bIncludeDeviceNum = true;

    if (CLUtils::IsDeviceType(device, CL_DEVICE_TYPE_CPU))
    {
        strDeviceName = "CPU";
        bIncludeDeviceNum = false;
    }
    else if (CLUtils::GetDeviceName(device, strDeviceName) != CL_SUCCESS)
    {
        strDeviceName = "device";
    }

    std::ostringstream suffix;
    suffix << "__k" << nKernelID << "_" << strDeviceName;

    if (bIncludeDeviceNum)
    {
        std::map<cl_device_id, unsigned int>::iterator deviceIt = s_deviceMap.find(device);
        unsigned int nDeviceID;

        // number of devices for this session
        static unsigned int s_nDeviceCount = 0;

        if (deviceIt != s_deviceMap.end())
        {
            nDeviceID = deviceIt->second;
        }
        else
        {
            nDeviceID = ++s_nDeviceCount;
            s_deviceMap[device] = nDeviceID;
        }

        suffix << nDeviceID;
    }

    return suffix.str();
}

cl_int CL_API_CALL
Mine_clReleaseKernel(cl_kernel kernel)
{
    SeqIDGenerator::Instance()->GenerateID();
    cl_uint kernelRefCount;
    cl_int status = g_realDispatchTable.GetKernelInfo(kernel, CL_KERNEL_REFERENCE_COUNT, sizeof(cl_uint), &kernelRefCount, NULL);

    if (status == CL_SUCCESS && kernelRefCount == 1)
    {
        g_Profiler.RemoveKernel(kernel);

        AddOrRemoveKernel(kernel, false);
    }

    return g_nextDispatchTable.ReleaseKernel(kernel);
}

cl_int CL_API_CALL
Mine_clSetKernelArg(cl_kernel   kernel,
                    cl_uint     argIndex,
                    size_t      argSize,
                    const void* pArgValue)
{
    SeqIDGenerator::Instance()->GenerateID();
    cl_int status = g_nextDispatchTable.SetKernelArg(kernel, argIndex, argSize, pArgValue);

    if (CL_SUCCESS != status)
    {
        return status;
    }

    g_Profiler.AddKernelArg(kernel, argIndex, pArgValue);

    return status;
}

cl_int CL_API_CALL
Mine_clEnqueueNDRangeKernel(cl_command_queue commandQueue,
                            cl_kernel        kernel,
                            cl_uint          uWorkDim,
                            const size_t*    pGlobalWorkOffset,
                            const size_t*    pGlobalWorkSize,
                            const size_t*    pLocalWorkSize,
                            cl_uint          uEventWaitList,
                            const cl_event*  pEventWaitList,
                            cl_event*        pEvent)
{
    osThreadId tid;
    unsigned int seqid = 0;
    SeqIDGenerator::Instance()->GenerateID(&tid, &seqid);
    AMDTScopeLock lock(s_mtx);

    CLUserEvent* userEvent = g_Profiler.HasUserEvent(pEventWaitList, uEventWaitList);

    bool bSinglePass = GlobalSettings::GetInstance()->m_params.m_bForceSinglePassPMC;
    bool bSkipKernelWithSVMPointerArgs = g_Profiler.HasKernelArgSVMPointer(kernel) && !bSinglePass;
    bool bSkipKernelWithPipeArgs = g_Profiler.HasKernelArgPipe(kernel) && !bSinglePass;
    bool bProfilerLoaded = g_Profiler.Loaded();
    bool bMaxKernelsProfiled = g_Profiler.HasKernelMaxBeenReached();
    bool bIsProfilingEnabled = g_Profiler.IsProfilingEnabled();
    bool bNoCounters = !bProfilerLoaded || userEvent != NULL || bSkipKernelWithSVMPointerArgs || bSkipKernelWithPipeArgs || bMaxKernelsProfiled || !bIsProfilingEnabled;

    // Set Kernel statistics to output to a csv file
    KernelStats kernelStats;
    kernelStats.m_threadId = tid;
    kernelStats.m_uSequenceId = seqid;

    gpa_uint32 uSessionIDOut = 0;
    cl_int returnCode = CL_SUCCESS;

    KernelFilterList enabledKernels = GlobalSettings::GetInstance()->m_params.m_kernelFilterList;

    bool skipProfiling = bMaxKernelsProfiled || !bIsProfilingEnabled;

    char kernelName[SP_MAX_PATH];
    cl_int kernelInfoResult = g_realDispatchTable.GetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, SP_MAX_PATH, kernelName, NULL);
    std::string strKernelName;

    if (kernelInfoResult == CL_SUCCESS)
    {
        strKernelName = std::string(kernelName);

        if (!skipProfiling)
        {
            skipProfiling = (!enabledKernels.empty()) && (enabledKernels.find(strKernelName) == enabledKernels.end());
        }
    }

    if (skipProfiling)
    {
        return g_nextDispatchTable.EnqueueNDRangeKernel(commandQueue,
                                                        kernel,
                                                        uWorkDim,
                                                        pGlobalWorkOffset,
                                                        pGlobalWorkSize,
                                                        pLocalWorkSize,
                                                        uEventWaitList,
                                                        pEventWaitList,
                                                        pEvent);
    }

    if (bNoCounters)
    {
        if (!bProfilerLoaded)
        {
            Log(logERROR, "Error: Profiler not loaded.\n");
        }

        if (userEvent != NULL)
        {
            Log(logWARNING, "The profiler does not support user events. clEnqueueNDRangeKernel calls that have a dependency on a user event will not be profiled.\n");
            std::cout << "Warning: The profiler does not support user events. clEnqueueNDRangeKernel calls that have a dependency on a user event will not be profiled.\n";
        }

        if (bSkipKernelWithSVMPointerArgs)
        {
            Log(logWARNING, "The profiler cannot perform a multi-pass profile with kernels that use SVM kernel args. Use --singlepass to allow profiling these kernels\n");
            std::cout << "The profiler cannot perform a multi-pass profile with kernels that use SVM kernel args. Use --singlepass to allow profiling these kernels\n";
        }

        if (bSkipKernelWithPipeArgs)
        {
            Log(logWARNING, "The profiler cannot perform a multi-pass profile with kernels that use pipe args. Use --singlepass to allow profiling these kernels\n");
            std::cout << "The profiler cannot perform a multi-pass profile with kernels that use pipe args. Use --singlepass to allow profiling these kernels\n";
        }

        cl_event* pTmpEvent = pEvent;
        cl_event event1;

        if (pTmpEvent == NULL)
        {
            pTmpEvent = &event1;
        }

        // 1) GPA is not loaded, 2) kernel uses a user event, or 3) kernel uses SVM kernel or pipe args, so don't do any GPU profiling
        returnCode = g_nextDispatchTable.EnqueueNDRangeKernel(commandQueue,
                                                              kernel,
                                                              uWorkDim,
                                                              pGlobalWorkOffset,
                                                              pGlobalWorkSize,
                                                              pLocalWorkSize,
                                                              uEventWaitList,
                                                              pEventWaitList,
                                                              pTmpEvent);

        if (returnCode == CL_SUCCESS)
        {
            if (GlobalSettings::GetInstance()->m_params.m_bGPUTimePMC)
            {
                CLUtils::GetElapsedTimeFromEvent(pTmpEvent, kernelStats.m_dTime);
            }

            if (pEvent != NULL && userEvent != NULL)
            {
                userEvent->AddDependentEvent(*pEvent);
            }
        }
        else
        {
            return returnCode;
        }

    }
    else
    {
        if (g_Profiler.Open(commandQueue))
        {
            g_Profiler.EnableCounters(commandQueue);
        }

        // perform profiling with a full set of GPA public counters
        g_Profiler.FullProfile(commandQueue,
                               kernel,
                               uWorkDim,
                               pGlobalWorkOffset,
                               pGlobalWorkSize,
                               pLocalWorkSize,
                               uEventWaitList,
                               pEventWaitList,
                               pEvent,
                               returnCode,
                               uSessionIDOut,
                               kernelStats.m_dTime);
    }

    if (returnCode == CL_SUCCESS)
    {
        cl_int result = 0;
        kernelStats.m_strName = strKernelName;
        kernelStats.m_uWorkDim = uWorkDim;

        cl_device_id device = 0;
        // get device from the command queue
        result = g_realDispatchTable.GetCommandQueueInfo(commandQueue, CL_QUEUE_DEVICE, sizeof(cl_device_id), &device, NULL);

        // get device type
        cl_device_type deviceType;
        result = g_realDispatchTable.GetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type), &deviceType, NULL);

        std::string strKernelNameInner(kernelStats.m_strName);

        // append kernel name with a suffix containing unique IDs for the kernel and device
        kernelStats.m_strName += GetKernelSuffix(kernel, device);

        // Check whether we need to generate the kernel source, IL or ISA (based on the kernel name and
        // the kernel + device IDs)
        // Generate those files if they haven't been generated yet
        if (CL_SUCCESS == result)
        {
            g_Profiler.GenerateKernelAssembly(commandQueue, kernel, strKernelNameInner, kernelStats.m_strName);
            kernelStats.m_kernelInfo = g_Profiler.GetKernelInfoFromKernelAssembly(kernelStats.m_strName);
        }

        // we need to always query the LDS size -- there are cases where the same kernel gets dispatched
        // with different workgroup sizes.  Each dispatch will have a different amount of LDS usage, but
        // due to caching in KernelAssembly::Generate and KernelAssembly::GetKernelInfo, the amount of LDS
        // reported for each dispatch will be the amount used by the first dispatch.  Ideally, we should alter
        // the caching mechanism to take workgroup size into account, but for the 2.4 release, it is safer
        // to just always query CL_KERNEL_LOCAL_MEM_SIZE using clGetKernelWorkGroupInfo for each dispatch.
        // NOTE: this is the cause of the regression when profiling the DwtHaar1D SDK sample.  It's kernel
        // gets enqueued twice once with a work-group size of 256, and again with a work-group size of 1.
        // Both dispatches are reported as using 2048 LDS, when the second one should only be using 8
        //
        // if (kernelStats.kernelInfo.m_nUsedLDSSize == KERNELINFO_NONE)
        //{
        cl_ulong ulLocalMem = 0;
        result = g_realDispatchTable.GetKernelWorkGroupInfo(kernel, device,
                                                            CL_KERNEL_LOCAL_MEM_SIZE,
                                                            sizeof(cl_ulong), &ulLocalMem, NULL);
        kernelStats.m_kernelInfo.m_nUsedLDSSize = (unsigned long)ulLocalMem;
        //}

        // check that they are not null
        if (NULL != pGlobalWorkSize)
        {
            for (cl_uint i = 0; i < uWorkDim; i++)
            {
                kernelStats.m_globalWorkSize[i] = pGlobalWorkSize[i];
            }
        }
        else
        {
            // this is an error according to the OpenCL spec
            assert(!"clEnqueueNDRangeKernel: GlobalWorkSize is NULL");

            for (cl_uint i = 0; i < 3; i++)
            {
                kernelStats.m_globalWorkSize[i] = 0;
            }
        }

        if (NULL != pLocalWorkSize)
        {
            for (cl_uint i = 0; i < uWorkDim; i++)
            {
                kernelStats.m_workGroupSize[i] = pLocalWorkSize[i];
            }
        }
        else
        {
            // this is possible according to the OpenCL spec: the OpenCL implementation will pick the optimal
            // implementation for the work group size.
            for (cl_uint i = 0; i < 3; i++)
            {
                kernelStats.m_workGroupSize[i] = 0;
            }
        }

        if (!bNoCounters)
        {
            // write the result out to a csv file
            g_Profiler.DumpSession(uSessionIDOut, kernelStats);
        }
        else
        {
            KernelProfileResultManager::Instance()->BeginKernelInfo();
            g_Profiler.DumpKernelStats(kernelStats);
            KernelProfileResultManager::Instance()->EndKernelInfo();
        }
    }

    if (!bNoCounters)
    {
        g_Profiler.Close();
    }


    return returnCode;
}

cl_command_queue CL_API_CALL
Mine_clCreateCommandQueue(cl_context                  context,
                          cl_device_id                device,
                          cl_command_queue_properties properties,
                          cl_int*                     pErrorCode)
{
    SeqIDGenerator::Instance()->GenerateID();

    // enable OpenCL profiling
    cl_command_queue result = g_nextDispatchTable.CreateCommandQueue(context,
                                                                     device,
                                                                     properties | CL_QUEUE_PROFILING_ENABLE,
                                                                     pErrorCode);

    return result;
}

cl_int CL_API_CALL
Mine_clReleaseCommandQueue(cl_command_queue commandQueue)
{
    SeqIDGenerator::Instance()->GenerateID();
    cl_int result = g_nextDispatchTable.ReleaseCommandQueue(commandQueue);

    if (CL_SUCCESS != result)
    {
        // there is a cl error, so we don't need to continue
        return result;
    }

    return result;
}

cl_int CL_API_CALL
Mine_clRetainCommandQueue(cl_command_queue commandQueue)
{
    SeqIDGenerator::Instance()->GenerateID();
    cl_int result = g_nextDispatchTable.RetainCommandQueue(commandQueue);

    return result;
}

cl_event CL_API_CALL Mine_clCreateUserEvent(
    cl_context    context,
    cl_int*       errcode_ret)
{
    cl_event event = g_nextDispatchTable.CreateUserEvent(context, errcode_ret);

    if (event != NULL)
    {
        // track user event
        g_Profiler.AddUserEvent(event);
    }

    return event;
}

cl_int CL_API_CALL Mine_clSetUserEventStatus(
    cl_event   event,
    cl_int     execution_status)
{
    SeqIDGenerator::Instance()->GenerateID();
    cl_int ret = g_nextDispatchTable.SetUserEventStatus(event, execution_status);

    if (execution_status == CL_COMPLETE)
    {
        g_Profiler.RemoveUserEvent(event);
    }

    return ret;
}

cl_int CL_API_CALL Mine_clReleaseEvent(cl_event event)
{
    SeqIDGenerator::Instance()->GenerateID();
    g_Profiler.RemoveUserEvent(event);
    cl_int ret = g_nextDispatchTable.ReleaseEvent(event);
    return ret;
}

//************ Seq track only *****************//

cl_int CL_API_CALL Mine_clGetPlatformIDs(
    cl_uint           num_entries ,
    cl_platform_id*   platform_list ,
    cl_uint*          num_platforms)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.GetPlatformIDs(
                     num_entries,
                     platform_list,
                     num_platforms);

    return ret;
}

cl_int CL_API_CALL Mine_clGetPlatformInfo(
    cl_platform_id    platform ,
    cl_platform_info  param_name ,
    size_t            param_value_size ,
    void*             param_value ,
    size_t*           param_value_size_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.GetPlatformInfo(
                     platform,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    return ret;
}

cl_int CL_API_CALL Mine_clGetDeviceIDs(
    cl_platform_id    platform ,
    cl_device_type    device_type ,
    cl_uint           num_entries ,
    cl_device_id*     device_list ,
    cl_uint*          num_devices)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.GetDeviceIDs(
                     platform,
                     device_type,
                     num_entries,
                     device_list,
                     num_devices);

    return ret;
}

cl_int CL_API_CALL Mine_clGetDeviceInfo(
    cl_device_id     device ,
    cl_device_info   param_name ,
    size_t           param_value_size ,
    void*            param_value ,
    size_t*          param_value_size_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.GetDeviceInfo(
                     device,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    return ret;
}

cl_int CL_API_CALL Mine_clRetainContext(cl_context  context)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.RetainContext(context);

    return ret;
}

cl_int CL_API_CALL Mine_clReleaseContext(cl_context  context)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_uint nRefCount;
    cl_int nRet = g_realDispatchTable.GetContextInfo(context, CL_CONTEXT_REFERENCE_COUNT, sizeof(cl_uint), &nRefCount, NULL);

    if (nRet != CL_SUCCESS)
    {
        // If this failed, we can't do anything but calling real function.
        return g_nextDispatchTable.ReleaseContext(context);
    }

    if (nRefCount == 1)
    {
        // mem object is going to be deleted.
        // remove it from context
        g_Profiler.RemoveContext(context);
    }

    cl_int ret = g_nextDispatchTable.ReleaseContext(context);

    return ret;
}

cl_int CL_API_CALL Mine_clGetContextInfo(
    cl_context          context ,
    cl_context_info     param_name ,
    size_t              param_value_size ,
    void*               param_value ,
    size_t*             param_value_size_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.GetContextInfo(
                     context,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    return ret;
}

cl_int CL_API_CALL Mine_clGetCommandQueueInfo(
    cl_command_queue       command_queue ,
    cl_command_queue_info  param_name ,
    size_t                 param_value_size ,
    void*                  param_value ,
    size_t*                param_value_size_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.GetCommandQueueInfo(
                     command_queue,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    return ret;
}

cl_int CL_API_CALL Mine_clSetCommandQueueProperty(
    cl_command_queue             command_queue,
    cl_command_queue_properties  properties,
    cl_bool                      enable,
    cl_command_queue_properties* old_properties)
{
    SeqIDGenerator::Instance()->GenerateID();

    return g_nextDispatchTable.SetCommandQueueProperty(
               command_queue,
               properties,
               enable,
               old_properties);
}

cl_int CL_API_CALL Mine_clRetainMemObject(cl_mem  memobj)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.RetainMemObject(memobj);

    return ret;
}

cl_int CL_API_CALL Mine_clReleaseMemObject(cl_mem  memobj)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_uint nRefCount;
    cl_int nRet = g_realDispatchTable.GetMemObjectInfo(memobj, CL_MEM_REFERENCE_COUNT, sizeof(cl_uint), &nRefCount, NULL);

    if (nRet != CL_SUCCESS)
    {
        // If this failed, we can't do anything but calling real function.
        return g_nextDispatchTable.ReleaseMemObject(memobj);
    }

    if (nRefCount == 1)
    {
        // mem object is going to be deleted.
        // remove it from context
        g_Profiler.RemoveMemObject(memobj);
    }

    cl_int ret = g_nextDispatchTable.ReleaseMemObject(memobj);

    return ret;
}

cl_int CL_API_CALL Mine_clGetSupportedImageFormats(
    cl_context            context ,
    cl_mem_flags          flags ,
    cl_mem_object_type    image_type ,
    cl_uint               num_entries ,
    cl_image_format*      image_formats ,
    cl_uint*              num_image_formats)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.GetSupportedImageFormats(
                     context,
                     flags,
                     image_type,
                     num_entries,
                     image_formats,
                     num_image_formats);

    return ret;
}

cl_int CL_API_CALL Mine_clGetMemObjectInfo(
    cl_mem            memobj ,
    cl_mem_info       param_name ,
    size_t            param_value_size ,
    void*             param_value ,
    size_t*           param_value_size_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.GetMemObjectInfo(
                     memobj,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    return ret;
}

cl_int CL_API_CALL Mine_clGetImageInfo(
    cl_mem            image ,
    cl_image_info     param_name ,
    size_t            param_value_size ,
    void*             param_value ,
    size_t*           param_value_size_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.GetImageInfo(
                     image,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    return ret;
}

cl_int CL_API_CALL Mine_clSetMemObjectDestructorCallback(
    cl_mem  memobj ,
    void (CL_CALLBACK* pfn_notify)(cl_mem  memobj , void* user_data) ,
    void* user_data)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.SetMemObjectDestructorCallback(
                     memobj,
                     pfn_notify,
                     user_data);

    return ret;
}

cl_sampler CL_API_CALL Mine_clCreateSampler(
    cl_context           context ,
    cl_bool              normalized_coords ,
    cl_addressing_mode   addressing_mode ,
    cl_filter_mode       filter_mode ,
    cl_int*              errcode_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_sampler ret = g_nextDispatchTable.CreateSampler(
                         context,
                         normalized_coords,
                         addressing_mode,
                         filter_mode,
                         errcode_ret);

    return ret;
}

cl_int CL_API_CALL Mine_clRetainSampler(cl_sampler  sampler)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.RetainSampler(sampler);

    return ret;
}

cl_int CL_API_CALL Mine_clReleaseSampler(cl_sampler  sampler)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.ReleaseSampler(sampler);

    return ret;
}

cl_int CL_API_CALL Mine_clGetSamplerInfo(
    cl_sampler          sampler ,
    cl_sampler_info     param_name ,
    size_t              param_value_size ,
    void*               param_value ,
    size_t*             param_value_size_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.GetSamplerInfo(
                     sampler,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    return ret;
}

cl_program CL_API_CALL Mine_clCreateProgramWithSource(
    cl_context         context ,
    cl_uint            count ,
    const char**       strings ,
    const size_t*      lengths ,
    cl_int*            errcode_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_program ret = g_nextDispatchTable.CreateProgramWithSource(
                         context,
                         count,
                         strings,
                         lengths,
                         errcode_ret);

    return ret;
}

cl_program CL_API_CALL Mine_clCreateProgramWithBinary(
    cl_context                      context ,
    cl_uint                         num_devices ,
    const cl_device_id*             device_list ,
    const size_t*                   lengths ,
    const unsigned char**           binaries ,
    cl_int*                         binary_status ,
    cl_int*                         errcode_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_program ret = g_nextDispatchTable.CreateProgramWithBinary(
                         context,
                         num_devices,
                         device_list,
                         lengths,
                         binaries,
                         binary_status,
                         errcode_ret);

    return ret;
}

cl_int CL_API_CALL Mine_clRetainProgram(cl_program  program)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.RetainProgram(program);

    return ret;
}

cl_int CL_API_CALL Mine_clReleaseProgram(cl_program  program)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.ReleaseProgram(program);

    return ret;
}

cl_int CL_API_CALL Mine_clUnloadCompiler(void)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.UnloadCompiler();

    return ret;
}

cl_int CL_API_CALL Mine_clGetProgramInfo(
    cl_program          program ,
    cl_program_info     param_name ,
    size_t              param_value_size ,
    void*               param_value ,
    size_t*             param_value_size_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.GetProgramInfo(
                     program,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    return ret;
}

cl_int CL_API_CALL Mine_clGetProgramBuildInfo(
    cl_program             program ,
    cl_device_id           device ,
    cl_program_build_info  param_name ,
    size_t                 param_value_size ,
    void*                  param_value ,
    size_t*                param_value_size_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.GetProgramBuildInfo(
                     program,
                     device,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    return ret;
}

cl_int CL_API_CALL Mine_clCreateKernelsInProgram(
    cl_program      program ,
    cl_uint         num_kernels ,
    cl_kernel*      kernels ,
    cl_uint*        num_kernels_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_uint substituted_ret;

    if (num_kernels_ret == NULL)
    {
        num_kernels_ret = &substituted_ret;
    }

    cl_int ret = g_nextDispatchTable.CreateKernelsInProgram(
                     program,
                     num_kernels,
                     kernels,
                     num_kernels_ret);

    if (ret == CL_SUCCESS && kernels != NULL)
    {
        for (cl_uint i = 0; i < *num_kernels_ret; i++)
        {
            g_Profiler.AddKernel(kernels[i]);
        }
    }

    return ret;
}

cl_int CL_API_CALL Mine_clRetainKernel(cl_kernel     kernel)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.RetainKernel(kernel);

    return ret;
}

cl_int CL_API_CALL Mine_clGetKernelInfo(
    cl_kernel        kernel ,
    cl_kernel_info   param_name ,
    size_t           param_value_size ,
    void*            param_value ,
    size_t*          param_value_size_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.GetKernelInfo(
                     kernel,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    return ret;
}

cl_int CL_API_CALL Mine_clGetKernelWorkGroupInfo(
    cl_kernel                   kernel ,
    cl_device_id                device ,
    cl_kernel_work_group_info   param_name ,
    size_t                      param_value_size ,
    void*                       param_value ,
    size_t*                     param_value_size_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.GetKernelWorkGroupInfo(
                     kernel,
                     device,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    return ret;
}

cl_int CL_API_CALL Mine_clWaitForEvents(
    cl_uint              num_events ,
    const cl_event*      event_list)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.WaitForEvents(
                     num_events,
                     event_list);

    return ret;
}

cl_int CL_API_CALL Mine_clGetEventInfo(
    cl_event          event ,
    cl_event_info     param_name ,
    size_t            param_value_size ,
    void*             param_value ,
    size_t*           param_value_size_ret)
{
    // don't generate a sequence ID for clGetEventInfo, since one is not generated by the client
    // SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.GetEventInfo(
                     event,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    return ret;
}

cl_int CL_API_CALL Mine_clRetainEvent(cl_event  event)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.RetainEvent(event);

    return ret;
}

cl_int CL_API_CALL Mine_clSetEventCallback(
    cl_event     event ,
    cl_int       command_exec_callback_type ,
    void (CL_CALLBACK*   pfn_notify)(cl_event, cl_int, void*) ,
    void*        user_data)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.SetEventCallback(
                     event,
                     command_exec_callback_type,
                     pfn_notify,
                     user_data);

    return ret;
}

cl_int CL_API_CALL Mine_clGetEventProfilingInfo(
    cl_event             event ,
    cl_profiling_info    param_name ,
    size_t               param_value_size ,
    void*                param_value ,
    size_t*              param_value_size_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.GetEventProfilingInfo(
                     event,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    return ret;
}

cl_int CL_API_CALL Mine_clFlush(cl_command_queue  command_queue)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.Flush(command_queue);

    return ret;
}

cl_int CL_API_CALL Mine_clFinish(cl_command_queue  command_queue)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.Finish(command_queue);

    return ret;
}

cl_int CL_API_CALL Mine_clEnqueueTask(
    cl_command_queue   command_queue ,
    cl_kernel          kernel ,
    cl_uint            num_events_in_wait_list ,
    const cl_event*    event_wait_list ,
    cl_event*          event)
{

    cl_int ret = g_nextDispatchTable.EnqueueTask(
                     command_queue,
                     kernel,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    if (ret != CL_SUCCESS)
    {
        // only generate a sequence id if the clEnqueueTask api fails.  If it succeeds, the sequence id is generated by the clEnqueueNDRangeKernel call
        SeqIDGenerator::Instance()->GenerateID();
    }

    return ret;
}

cl_int CL_API_CALL Mine_clEnqueueNativeKernel(
    cl_command_queue   command_queue ,
    void (CL_CALLBACK* user_func)(void*) ,
    void*              args ,
    size_t             cb_args ,
    cl_uint            num_mem_objects ,
    const cl_mem*      mem_list ,
    const void**       args_mem_loc ,
    cl_uint            num_events_in_wait_list ,
    const cl_event*    event_wait_list ,
    cl_event*          event)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.EnqueueNativeKernel(
                     command_queue,
                     user_func,
                     args,
                     cb_args,
                     num_mem_objects,
                     mem_list,
                     args_mem_loc,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    return ret;
}

cl_int CL_API_CALL Mine_clEnqueueMarker(
    cl_command_queue     command_queue ,
    cl_event*            event)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.EnqueueMarker(
                     command_queue,
                     event);

    return ret;
}

cl_int CL_API_CALL Mine_clEnqueueWaitForEvents(
    cl_command_queue  command_queue ,
    cl_uint           num_events ,
    const cl_event*   event_list)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.EnqueueWaitForEvents(
                     command_queue,
                     num_events,
                     event_list);

    return ret;
}

cl_int CL_API_CALL Mine_clEnqueueBarrier(cl_command_queue  command_queue)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_int ret = g_nextDispatchTable.EnqueueBarrier(command_queue);

    return ret;
}


//******CL GL Interop************************//
cl_mem CL_API_CALL
Mine_clCreateFromGLTexture2D(cl_context      context,
                             cl_mem_flags    flags,
                             cl_GLenum       target,
                             cl_GLint        miplevel,
                             cl_GLuint       texture,
                             cl_int*         errcode_ret)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.CreateFromGLTexture2D(context, flags, target, miplevel, texture, errcode_ret);
}

cl_mem CL_API_CALL
Mine_clCreateFromGLTexture3D(cl_context      context,
                             cl_mem_flags    flags,
                             cl_GLenum       target,
                             cl_GLint        miplevel,
                             cl_GLuint       texture,
                             cl_int*         errcode_ret)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.CreateFromGLTexture3D(context, flags, target, miplevel, texture, errcode_ret);
}

cl_int CL_API_CALL
Mine_clGetGLObjectInfo(cl_mem                 memobj,
                       cl_gl_object_type*     gl_object_type,
                       cl_GLuint*             gl_object_name)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.GetGLObjectInfo(memobj, gl_object_type, gl_object_name);
}

cl_int CL_API_CALL
Mine_clGetGLTextureInfo(cl_mem               memobj,
                        cl_gl_texture_info   param_name,
                        size_t               param_value_size,
                        void*                param_value,
                        size_t*              param_value_size_ret)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.GetGLTextureInfo(memobj, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int CL_API_CALL
Mine_clEnqueueAcquireGLObjects(cl_command_queue command_queue,
                               cl_uint          num_objects,
                               const cl_mem*    mem_objects,
                               cl_uint          num_events_in_wait_list,
                               const cl_event*  event_wait_list,
                               cl_event*        event)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.EnqueueAcquireGLObjects(command_queue,
                                                       num_objects,
                                                       mem_objects,
                                                       num_events_in_wait_list,
                                                       event_wait_list,
                                                       event);
}

cl_int CL_API_CALL
Mine_clEnqueueReleaseGLObjects(cl_command_queue command_queue,
                               cl_uint          num_objects,
                               const cl_mem*    mem_objects,
                               cl_uint          num_events_in_wait_list,
                               const cl_event*  event_wait_list,
                               cl_event*        event)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.EnqueueReleaseGLObjects(command_queue,
                                                       num_objects,
                                                       mem_objects,
                                                       num_events_in_wait_list,
                                                       event_wait_list,
                                                       event);
}

cl_int CL_API_CALL
Mine_clGetGLContextInfoKHR(const cl_context_properties* properties,
                           cl_gl_context_info           param_name,
                           size_t                       param_value_size,
                           void*                        param_value,
                           size_t*                      param_value_size_ret)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.GetGLContextInfoKHR(properties, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_event CL_API_CALL
Mine_clCreateEventFromGLsyncKHR(cl_context context,
                                cl_GLsync  cl_GLsync,
                                cl_int*    errcode_ret)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.CreateEventFromGLsyncKHR(context, cl_GLsync, errcode_ret);
}


//******End of GL/CL Interop*********************//


//******Sub Devices************************//


cl_int CL_API_CALL
Mine_clCreateSubDevicesEXT(cl_device_id     in_device,
                           const cl_device_partition_property_ext* partition_properties,
                           cl_uint          num_entries,
                           cl_device_id*    out_devices,
                           cl_uint*         num_devices)
{
    SeqIDGenerator::Instance()->GenerateID();
    return ((clCreateSubDevicesEXT_fn)(g_nextDispatchTable._reservedForDeviceFissionEXT[0]))(
               in_device,
               partition_properties,
               num_entries,
               out_devices,
               num_devices);
}

cl_int CL_API_CALL
Mine_clRetainDeviceEXT(cl_device_id device)
{
    SeqIDGenerator::Instance()->GenerateID();
    return ((clRetainDeviceEXT_fn)(g_nextDispatchTable._reservedForDeviceFissionEXT[1]))(device);
}

cl_int CL_API_CALL
Mine_clReleaseDeviceEXT(cl_device_id device)
{
    SeqIDGenerator::Instance()->GenerateID();
    return ((clReleaseDeviceEXT_fn)(g_nextDispatchTable._reservedForDeviceFissionEXT[2]))(device);;
}

//******End of Sub Devices*********************//


//******DX10/CL Interop************************//

#ifdef _WIN32
cl_int CL_API_CALL
Mine_clGetDeviceIDsFromD3D10KHR(cl_platform_id             platform,
                                cl_d3d10_device_source_khr d3d_device_source,
                                void*                      d3d_object,
                                cl_d3d10_device_set_khr    d3d_device_set,
                                cl_uint                    num_entries,
                                cl_device_id*              devices,
                                cl_uint*                   num_devices)
{
    SeqIDGenerator::Instance()->GenerateID();
    return ((clGetDeviceIDsFromD3D10KHR_fn)(g_nextDispatchTable._reservedForD3D10KHR[0]))(
               platform,
               d3d_device_source,
               d3d_object,
               d3d_device_set,
               num_entries,
               devices,
               num_devices);
}

cl_mem CL_API_CALL
Mine_clCreateFromD3D10Texture2DKHR(cl_context        context,
                                   cl_mem_flags      flags,
                                   ID3D10Texture2D*  resource,
                                   UINT              subresource,
                                   cl_int*           errcode_ret)
{
    SeqIDGenerator::Instance()->GenerateID();
    return ((clCreateFromD3D10Texture2DKHR_fn)(g_nextDispatchTable._reservedForD3D10KHR[2]))(
               context,
               flags,
               resource,
               subresource,
               errcode_ret);
}

cl_mem CL_API_CALL
Mine_clCreateFromD3D10Texture3DKHR(cl_context        context,
                                   cl_mem_flags      flags,
                                   ID3D10Texture3D*  resource,
                                   UINT              subresource,
                                   cl_int*           errcode_ret)
{
    SeqIDGenerator::Instance()->GenerateID();
    return ((clCreateFromD3D10Texture3DKHR_fn)(g_nextDispatchTable._reservedForD3D10KHR[3]))(
               context,
               flags,
               resource,
               subresource,
               errcode_ret);
}

cl_int CL_API_CALL
Mine_clEnqueueAcquireD3D10ObjectsKHR(cl_command_queue command_queue,
                                     cl_uint          num_objects,
                                     const cl_mem*    mem_objects,
                                     cl_uint          num_events_in_wait_list,
                                     const cl_event*  event_wait_list,
                                     cl_event*        event)
{
    SeqIDGenerator::Instance()->GenerateID();
    return ((clEnqueueAcquireD3D10ObjectsKHR_fn)(g_nextDispatchTable._reservedForD3D10KHR[4]))(
               command_queue,
               num_objects,
               mem_objects,
               num_events_in_wait_list,
               event_wait_list,
               event);
}

cl_int CL_API_CALL
Mine_clEnqueueReleaseD3D10ObjectsKHR(cl_command_queue command_queue,
                                     cl_uint          num_objects,
                                     const cl_mem*    mem_objects,
                                     cl_uint          num_events_in_wait_list,
                                     const cl_event*  event_wait_list,
                                     cl_event*        event)
{
    SeqIDGenerator::Instance()->GenerateID();
    return ((clEnqueueReleaseD3D10ObjectsKHR_fn)(g_nextDispatchTable._reservedForD3D10KHR[5]))(
               command_queue,
               num_objects,
               mem_objects,
               num_events_in_wait_list,
               event_wait_list,
               event);
}
#endif

//******OpenCL 1.2************************//

cl_int CL_API_CALL
Mine_clCreateSubDevices(cl_device_id     device,
                        const cl_device_partition_property* pPartitionProperties,
                        cl_uint          nEntries,
                        cl_device_id*    pDevices,
                        cl_uint*         pNumDevices)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.CreateSubDevices(device,
                                                pPartitionProperties,
                                                nEntries,
                                                pDevices,
                                                pNumDevices);
}

cl_int CL_API_CALL
Mine_clRetainDevice(cl_device_id device)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.RetainDevice(device);
}

cl_int CL_API_CALL
Mine_clReleaseDevice(cl_device_id device)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.ReleaseDevice(device);
}

cl_program CL_API_CALL
Mine_clCreateProgramWithBuiltInKernels(cl_context          context,
                                       cl_uint             nDevices,
                                       const cl_device_id* pDevices,
                                       const char*         pszKernelNames,
                                       cl_int*             pErrcode)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_program ret = g_nextDispatchTable.CreateProgramWithBuiltInKernels(
                         context,
                         nDevices,
                         pDevices,
                         pszKernelNames,
                         pErrcode);

    return ret;
}

cl_int CL_API_CALL
Mine_clCompileProgram(cl_program          program,
                      cl_uint             nDevices,
                      const cl_device_id* pDevices,
                      const char*         pszOptions,
                      cl_uint             nInputHeaders,
                      const cl_program*   pInputHeaders,
                      const char**        ppszHeaderIncludeNames,
                      void (CL_CALLBACK* pfn_notify)(cl_program, void*),
                      void*               pUserData)
{
    SeqIDGenerator::Instance()->GenerateID();
    std::string strOptions = (pszOptions != NULL) ? std::string(pszOptions) : "";

    // request the CL run-time to retain the CPU assembly and
    // the .amdil section (required starting from SDK 2.5) in the elf binary
    strOptions += " -fbin-as -fbin-amdil";

    cl_int ret = g_nextDispatchTable.CompileProgram(program,
                                                    nDevices,
                                                    pDevices,
                                                    strOptions.c_str(),
                                                    nInputHeaders,
                                                    pInputHeaders,
                                                    ppszHeaderIncludeNames,
                                                    pfn_notify,
                                                    pUserData);

    if (CL_INVALID_COMPILER_OPTIONS == ret)
    {
        // -fbin-as flag is not supported, let's rebuild the program again
        // Note: calling the nextDispatchTable again may cause other agents to see two CompileProgram calls
        ret = g_nextDispatchTable.CompileProgram(program,
                                                 nDevices,
                                                 pDevices,
                                                 strOptions.c_str(),
                                                 nInputHeaders,
                                                 pInputHeaders,
                                                 ppszHeaderIncludeNames,
                                                 pfn_notify,
                                                 pUserData);
    }

    return ret;
}

cl_program CL_API_CALL
Mine_clLinkProgram(cl_context          context,
                   cl_uint             nDevices,
                   const cl_device_id* pDevices,
                   const char*         pszOptions,
                   cl_uint             nInputPrograms,
                   const cl_program*   pInputPrograms,
                   void (CL_CALLBACK* pfn_notify)(cl_program, void*),
                   void*               pUserData,
                   cl_int*             pErrCode)
{
    {
        SeqIDGenerator::Instance()->GenerateID();
        std::string strOptions = (pszOptions != NULL) ? std::string(pszOptions) : "";

        cl_program ret = g_nextDispatchTable.LinkProgram(context,
                                                         nDevices,
                                                         pDevices,
                                                         strOptions.c_str(),
                                                         nInputPrograms,
                                                         pInputPrograms,
                                                         pfn_notify,
                                                         pUserData,
                                                         pErrCode);

        return ret;
    }
}

cl_int CL_API_CALL
Mine_clUnloadPlatformCompiler(cl_platform_id platform)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.UnloadPlatformCompiler(platform);
}

cl_int CL_API_CALL
Mine_clGetKernelArgInfo(cl_kernel          kernel,
                        cl_uint            arg_index,
                        cl_kernel_arg_info param_name,
                        size_t             param_value_size,
                        void*              param_value,
                        size_t*            param_value_size_ret)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.GetKernelArgInfo(kernel,
                                                arg_index,
                                                param_name,
                                                param_value_size,
                                                param_value,
                                                param_value_size_ret);
}

cl_int CL_API_CALL Mine_clEnqueueMarkerWithWaitList(cl_command_queue command_queue,
                                                    cl_uint          uEventWaitList,
                                                    const cl_event*  pEventWaitList,
                                                    cl_event*        pEvent)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.EnqueueMarkerWithWaitList(command_queue, uEventWaitList, pEventWaitList, pEvent);
}

cl_int CL_API_CALL Mine_clEnqueueBarrierWithWaitList(cl_command_queue command_queue,
                                                     cl_uint          uEventWaitList,
                                                     const cl_event*  pEventWaitList,
                                                     cl_event*        pEvent)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.EnqueueBarrierWithWaitList(command_queue, uEventWaitList, pEventWaitList, pEvent);
}

void* CL_API_CALL
Mine_clGetExtensionFunctionAddressForPlatform(cl_platform_id platform ,
                                              const char*    funcname)
{
    SeqIDGenerator::Instance()->GenerateID();
    void* pFuncPtr = g_nextDispatchTable.GetExtensionFunctionAddressForPlatform(platform,
                     funcname);

    return AssignExtensionFunctionPointer(funcname, pFuncPtr);
}

cl_mem CL_API_CALL
Mine_clCreateFromGLTexture(cl_context   context,
                           cl_mem_flags flags,
                           cl_GLenum    texture_target,
                           cl_GLint     miplevel,
                           cl_GLuint    texture,
                           cl_int*      errcode_ret)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.CreateFromGLTexture(context,
                                                   flags,
                                                   texture_target,
                                                   miplevel,
                                                   texture,
                                                   errcode_ret);
}

void* CL_API_CALL
Mine_clGetExtensionFunctionAddress(const char* funcname)
{
    SeqIDGenerator::Instance()->GenerateID();
    void* pFuncPtr = g_nextDispatchTable.GetExtensionFunctionAddress(funcname);

    return AssignExtensionFunctionPointer(funcname, pFuncPtr);
}

//******OpenCL 2.0************************//

cl_command_queue CL_API_CALL
Mine_clCreateCommandQueueWithProperties(cl_context                 context,
                                        cl_device_id               device,
                                        const cl_queue_properties* properties,
                                        cl_int*                    errcode_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    CLUtils::QueuePropertiesList propList;
    CLUtils::EnableQueueProfiling(properties, propList);

    return g_nextDispatchTable.CreateCommandQueueWithProperties(context,
                                                                device,
                                                                propList.data(),
                                                                errcode_ret);
}

cl_sampler CL_API_CALL
Mine_clCreateSamplerWithProperties(cl_context                   context,
                                   const cl_sampler_properties* properties,
                                   cl_int*                      errcode_ret)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.CreateSamplerWithProperties(context,
                                                           properties,
                                                           errcode_ret);
}

void* CL_API_CALL
Mine_clSVMAlloc(cl_context       context,
                cl_svm_mem_flags flags,
                size_t           size,
                cl_uint          alignment)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.SVMAlloc(context,
                                        flags,
                                        size,
                                        alignment);
}

void CL_API_CALL
Mine_clSVMFree(cl_context context,
               void*      svm_pointer)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.SVMFree(context,
                                       svm_pointer);
}

cl_int CL_API_CALL
Mine_clSetKernelArgSVMPointer(cl_kernel   kernel,
                              cl_uint     arg_index,
                              const void* arg_value)
{
    SeqIDGenerator::Instance()->GenerateID();

    g_Profiler.AddKernelArgSVMPointer(kernel, arg_index);

    return g_nextDispatchTable.SetKernelArgSVMPointer(kernel,
                                                      arg_index,
                                                      arg_value);
}

cl_int CL_API_CALL
Mine_clSetKernelExecInfo(cl_kernel           kernel,
                         cl_kernel_exec_info param_name,
                         size_t              param_value_size,
                         const void*         param_value)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.SetKernelExecInfo(kernel,
                                                 param_name,
                                                 param_value_size,
                                                 param_value);
}

cl_int CL_API_CALL
Mine_clEnqueueSVMFree(cl_command_queue command_queue,
                      cl_uint          num_svm_pointers,
                      void*            svm_pointers[],
                      void (CL_CALLBACK* pfn_free_func)(cl_command_queue, cl_uint, void* [], void*),
                      void*            user_data,
                      cl_uint          num_events_in_wait_list,
                      const cl_event*  event_wait_list,
                      cl_event*        event)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.EnqueueSVMFree(command_queue,
                                              num_svm_pointers,
                                              svm_pointers,
                                              pfn_free_func,
                                              user_data,
                                              num_events_in_wait_list,
                                              event_wait_list,
                                              event);
}

cl_int CL_API_CALL
Mine_clEnqueueSVMMemcpy(cl_command_queue command_queue,
                        cl_bool          blocking_copy,
                        void*            dst_ptr,
                        const void*      src_ptr,
                        size_t           size,
                        cl_uint          num_events_in_wait_list,
                        const cl_event*  event_wait_list,
                        cl_event*        event)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.EnqueueSVMMemcpy(command_queue,
                                                blocking_copy,
                                                dst_ptr,
                                                src_ptr,
                                                size,
                                                num_events_in_wait_list,
                                                event_wait_list,
                                                event);
}

cl_int CL_API_CALL
Mine_clEnqueueSVMMemFill(cl_command_queue command_queue,
                         void*            svm_ptr,
                         const void*      pattern,
                         size_t           pattern_size,
                         size_t           size,
                         cl_uint          num_events_in_wait_list,
                         const cl_event*  event_wait_list,
                         cl_event*        event)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.EnqueueSVMMemFill(command_queue,
                                                 svm_ptr,
                                                 pattern,
                                                 pattern_size,
                                                 size,
                                                 num_events_in_wait_list,
                                                 event_wait_list,
                                                 event);
}

cl_int CL_API_CALL
Mine_clEnqueueSVMMap(cl_command_queue command_queue,
                     cl_bool          blocking_map,
                     cl_map_flags     flags,
                     void*            svm_ptr,
                     size_t           size,
                     cl_uint          num_events_in_wait_list,
                     const cl_event*  event_wait_list,
                     cl_event*        event)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.EnqueueSVMMap(command_queue,
                                             blocking_map,
                                             flags,
                                             svm_ptr,
                                             size,
                                             num_events_in_wait_list,
                                             event_wait_list,
                                             event);
}

cl_int CL_API_CALL
Mine_clEnqueueSVMUnmap(cl_command_queue command_queue,
                       void*            svm_ptr,
                       cl_uint          num_events_in_wait_list,
                       const cl_event*  event_wait_list,
                       cl_event*        event)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.EnqueueSVMUnmap(command_queue,
                                               svm_ptr,
                                               num_events_in_wait_list,
                                               event_wait_list,
                                               event);
}

cl_mem CL_API_CALL
Mine_clCreatePipe(cl_context                context,
                  cl_mem_flags              flags,
                  cl_uint                   pipe_packet_size,
                  cl_uint                   pipe_max_packets,
                  const cl_pipe_properties* properties,
                  cl_int*                   errcode_ret)
{
    SeqIDGenerator::Instance()->GenerateID();
    cl_mem mem = g_nextDispatchTable.CreatePipe(context,
                                                flags,
                                                pipe_packet_size,
                                                pipe_max_packets,
                                                properties,
                                                errcode_ret);

    if (NULL != mem)
    {
        g_Profiler.AddPipe(context, mem);
    }

    return mem;

}

cl_int CL_API_CALL
Mine_clGetPipeInfo(cl_mem        pipe,
                   cl_pipe_info  param_name,
                   size_t        param_value_size,
                   void*         param_value,
                   size_t*       param_value_size_ret)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.GetPipeInfo(pipe,
                                           param_name,
                                           param_value_size,
                                           param_value,
                                           param_value_size_ret);
}

void* CL_API_CALL
Mine_clSVMAllocAMD(cl_context       context,
                   cl_svm_mem_flags flags,
                   size_t           size,
                   cl_uint          alignment)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_realExtensionFunctionTable.SVMAllocAMD(context,
                                                    flags,
                                                    size,
                                                    alignment);
}

void CL_API_CALL
Mine_clSVMFreeAMD(cl_context context,
                  void*      svm_pointer)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_realExtensionFunctionTable.SVMFreeAMD(context,
                                                   svm_pointer);
}

cl_int CL_API_CALL
Mine_clSetKernelArgSVMPointerAMD(cl_kernel   kernel,
                                 cl_uint     arg_index,
                                 const void* arg_value)
{
    SeqIDGenerator::Instance()->GenerateID();

    g_Profiler.AddKernelArgSVMPointer(kernel, arg_index);

    return g_realExtensionFunctionTable.SetKernelArgSVMPointerAMD(kernel,
                                                                  arg_index,
                                                                  arg_value);
}

cl_int CL_API_CALL
Mine_clSetKernelExecInfoAMD(cl_kernel           kernel,
                            cl_kernel_exec_info param_name,
                            size_t              param_value_size,
                            const void*         param_value)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_realExtensionFunctionTable.SetKernelExecInfoAMD(kernel,
                                                             param_name,
                                                             param_value_size,
                                                             param_value);
}

cl_int CL_API_CALL
Mine_clEnqueueSVMFreeAMD(cl_command_queue command_queue,
                         cl_uint          num_svm_pointers,
                         void*            svm_pointers[],
                         void (CL_CALLBACK* pfn_free_func)(cl_command_queue, cl_uint, void* [], void*),
                         void*            user_data,
                         cl_uint          num_events_in_wait_list,
                         const cl_event*  event_wait_list,
                         cl_event*        event)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_realExtensionFunctionTable.EnqueueSVMFreeAMD(command_queue,
                                                          num_svm_pointers,
                                                          svm_pointers,
                                                          pfn_free_func,
                                                          user_data,
                                                          num_events_in_wait_list,
                                                          event_wait_list,
                                                          event);
}

cl_int CL_API_CALL
Mine_clEnqueueSVMMemcpyAMD(cl_command_queue command_queue,
                           cl_bool          blocking_copy,
                           void*            dst_ptr,
                           const void*      src_ptr,
                           size_t           size,
                           cl_uint          num_events_in_wait_list,
                           const cl_event*  event_wait_list,
                           cl_event*        event)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_realExtensionFunctionTable.EnqueueSVMMemcpyAMD(command_queue,
                                                            blocking_copy,
                                                            dst_ptr,
                                                            src_ptr,
                                                            size,
                                                            num_events_in_wait_list,
                                                            event_wait_list,
                                                            event);
}

cl_int CL_API_CALL
Mine_clEnqueueSVMMemFillAMD(cl_command_queue command_queue,
                            void*            svm_ptr,
                            const void*      pattern,
                            size_t           pattern_size,
                            size_t           size,
                            cl_uint          num_events_in_wait_list,
                            const cl_event*  event_wait_list,
                            cl_event*        event)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_realExtensionFunctionTable.EnqueueSVMMemFillAMD(command_queue,
                                                             svm_ptr,
                                                             pattern,
                                                             pattern_size,
                                                             size,
                                                             num_events_in_wait_list,
                                                             event_wait_list,
                                                             event);
}

cl_int CL_API_CALL
Mine_clEnqueueSVMMapAMD(cl_command_queue command_queue,
                        cl_bool          blocking_map,
                        cl_map_flags     flags,
                        void*            svm_ptr,
                        size_t           size,
                        cl_uint          num_events_in_wait_list,
                        const cl_event*  event_wait_list,
                        cl_event*        event)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_realExtensionFunctionTable.EnqueueSVMMapAMD(command_queue,
                                                         blocking_map,
                                                         flags,
                                                         svm_ptr,
                                                         size,
                                                         num_events_in_wait_list,
                                                         event_wait_list,
                                                         event);
}

cl_int CL_API_CALL
Mine_clEnqueueSVMUnmapAMD(cl_command_queue command_queue,
                          void*            svm_ptr,
                          cl_uint          num_events_in_wait_list,
                          const cl_event*  event_wait_list,
                          cl_event*        event)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_realExtensionFunctionTable.EnqueueSVMUnmapAMD(command_queue,
                                                           svm_ptr,
                                                           num_events_in_wait_list,
                                                           event_wait_list,
                                                           event);
}


//******End of Seq track*********************//

void CreateMineDispatchTable(cl_icd_dispatch_table& dispatchTable)
{
    dispatchTable.GetPlatformIDs = Mine_clGetPlatformIDs;
    dispatchTable.GetPlatformInfo = Mine_clGetPlatformInfo;
    dispatchTable.GetDeviceIDs = Mine_clGetDeviceIDs;
    dispatchTable.GetDeviceInfo = Mine_clGetDeviceInfo;
    dispatchTable.CreateContext = Mine_clCreateContext;
    dispatchTable.CreateContextFromType = Mine_clCreateContextFromType;
    dispatchTable.RetainContext = Mine_clRetainContext;
    dispatchTable.ReleaseContext = Mine_clReleaseContext;
    dispatchTable.GetContextInfo = Mine_clGetContextInfo;
    dispatchTable.CreateCommandQueue = Mine_clCreateCommandQueue;
    dispatchTable.RetainCommandQueue = Mine_clRetainCommandQueue;
    dispatchTable.ReleaseCommandQueue = Mine_clReleaseCommandQueue;
    dispatchTable.GetCommandQueueInfo = Mine_clGetCommandQueueInfo;
    dispatchTable.SetCommandQueueProperty = Mine_clSetCommandQueueProperty;
    dispatchTable.CreateBuffer = Mine_clCreateBuffer;
    dispatchTable.CreateSubBuffer = Mine_clCreateSubBuffer;
    dispatchTable.CreateImage2D = Mine_clCreateImage2D;
    dispatchTable.CreateImage3D = Mine_clCreateImage3D;
    dispatchTable.RetainMemObject = Mine_clRetainMemObject;
    dispatchTable.ReleaseMemObject = Mine_clReleaseMemObject;
    dispatchTable.GetSupportedImageFormats = Mine_clGetSupportedImageFormats;
    dispatchTable.GetMemObjectInfo = Mine_clGetMemObjectInfo;
    dispatchTable.GetImageInfo = Mine_clGetImageInfo;
    dispatchTable.SetMemObjectDestructorCallback = Mine_clSetMemObjectDestructorCallback;
    dispatchTable.CreateSampler = Mine_clCreateSampler;
    dispatchTable.RetainSampler = Mine_clRetainSampler;
    dispatchTable.ReleaseSampler = Mine_clReleaseSampler;
    dispatchTable.GetSamplerInfo = Mine_clGetSamplerInfo;
    dispatchTable.CreateProgramWithSource = Mine_clCreateProgramWithSource;
    dispatchTable.CreateProgramWithBinary = Mine_clCreateProgramWithBinary;
    dispatchTable.RetainProgram = Mine_clRetainProgram;
    dispatchTable.ReleaseProgram = Mine_clReleaseProgram;
    dispatchTable.BuildProgram = Mine_clBuildProgram;
    dispatchTable.UnloadCompiler = Mine_clUnloadCompiler;
    dispatchTable.GetProgramInfo = Mine_clGetProgramInfo;
    dispatchTable.GetProgramBuildInfo = Mine_clGetProgramBuildInfo;
    dispatchTable.CreateKernel = Mine_clCreateKernel;
    dispatchTable.CreateKernelsInProgram = Mine_clCreateKernelsInProgram;
    dispatchTable.RetainKernel = Mine_clRetainKernel;
    dispatchTable.ReleaseKernel = Mine_clReleaseKernel;
    dispatchTable.SetKernelArg = Mine_clSetKernelArg;
    dispatchTable.GetKernelInfo = Mine_clGetKernelInfo;
    dispatchTable.GetKernelWorkGroupInfo = Mine_clGetKernelWorkGroupInfo;
    dispatchTable.WaitForEvents = Mine_clWaitForEvents;
    dispatchTable.GetEventInfo = Mine_clGetEventInfo;
    dispatchTable.CreateUserEvent = Mine_clCreateUserEvent;
    dispatchTable.RetainEvent = Mine_clRetainEvent;
    dispatchTable.ReleaseEvent = Mine_clReleaseEvent;
    dispatchTable.SetUserEventStatus = Mine_clSetUserEventStatus;
    dispatchTable.SetEventCallback = Mine_clSetEventCallback;
    dispatchTable.GetEventProfilingInfo = Mine_clGetEventProfilingInfo;
    dispatchTable.Flush = Mine_clFlush;
    dispatchTable.Finish = Mine_clFinish;
    dispatchTable.EnqueueReadBuffer = Mine_clEnqueueReadBuffer;
    dispatchTable.EnqueueReadBufferRect = Mine_clEnqueueReadBufferRect;
    dispatchTable.EnqueueWriteBuffer = Mine_clEnqueueWriteBuffer;
    dispatchTable.EnqueueWriteBufferRect = Mine_clEnqueueWriteBufferRect;
    dispatchTable.EnqueueCopyBuffer = Mine_clEnqueueCopyBuffer;
    dispatchTable.EnqueueCopyBufferRect = Mine_clEnqueueCopyBufferRect;
    dispatchTable.EnqueueReadImage = Mine_clEnqueueReadImage;
    dispatchTable.EnqueueWriteImage = Mine_clEnqueueWriteImage;
    dispatchTable.EnqueueCopyImage = Mine_clEnqueueCopyImage;
    dispatchTable.EnqueueCopyImageToBuffer = Mine_clEnqueueCopyImageToBuffer;
    dispatchTable.EnqueueCopyBufferToImage = Mine_clEnqueueCopyBufferToImage;
    dispatchTable.EnqueueMapBuffer = Mine_clEnqueueMapBuffer;
    dispatchTable.EnqueueMapImage = Mine_clEnqueueMapImage;
    dispatchTable.EnqueueUnmapMemObject = Mine_clEnqueueUnmapMemObject;
    dispatchTable.EnqueueNDRangeKernel = Mine_clEnqueueNDRangeKernel;
    dispatchTable.EnqueueTask = Mine_clEnqueueTask;
    dispatchTable.EnqueueNativeKernel = Mine_clEnqueueNativeKernel;
    dispatchTable.EnqueueMarker = Mine_clEnqueueMarker;
    dispatchTable.EnqueueWaitForEvents = Mine_clEnqueueWaitForEvents;
    dispatchTable.EnqueueBarrier = Mine_clEnqueueBarrier;

    // CL/GL Interop
    dispatchTable.CreateFromGLBuffer = Mine_clCreateFromGLBuffer;
    dispatchTable.CreateFromGLTexture2D = Mine_clCreateFromGLTexture2D;
    dispatchTable.CreateFromGLTexture3D = Mine_clCreateFromGLTexture3D;
    dispatchTable.CreateFromGLRenderbuffer = Mine_clCreateFromGLRenderbuffer;
    dispatchTable.GetGLObjectInfo = Mine_clGetGLObjectInfo;
    dispatchTable.GetGLTextureInfo = Mine_clGetGLTextureInfo;
    dispatchTable.EnqueueAcquireGLObjects = Mine_clEnqueueAcquireGLObjects;
    dispatchTable.EnqueueReleaseGLObjects = Mine_clEnqueueReleaseGLObjects;
    dispatchTable.GetGLContextInfoKHR = Mine_clGetGLContextInfoKHR;
    dispatchTable.CreateEventFromGLsyncKHR = Mine_clCreateEventFromGLsyncKHR;

    // OpenCL 1.2
    dispatchTable.CreateSubDevices = Mine_clCreateSubDevices;
    dispatchTable.RetainDevice = Mine_clRetainDevice;
    dispatchTable.ReleaseDevice = Mine_clReleaseDevice;
    dispatchTable.CreateImage = Mine_clCreateImage;
    dispatchTable.CreateProgramWithBuiltInKernels = Mine_clCreateProgramWithBuiltInKernels;
    dispatchTable.CompileProgram = Mine_clCompileProgram;
    dispatchTable.LinkProgram = Mine_clLinkProgram;
    dispatchTable.UnloadPlatformCompiler = Mine_clUnloadPlatformCompiler;
    dispatchTable.GetKernelArgInfo = Mine_clGetKernelArgInfo;
    dispatchTable.EnqueueFillBuffer = Mine_clEnqueueFillBuffer;
    dispatchTable.EnqueueFillImage = Mine_clEnqueueFillImage;
    dispatchTable.EnqueueMigrateMemObjects = Mine_clEnqueueMigrateMemObjects;
    dispatchTable.EnqueueMarkerWithWaitList = Mine_clEnqueueMarkerWithWaitList;
    dispatchTable.EnqueueBarrierWithWaitList = Mine_clEnqueueBarrierWithWaitList;
    dispatchTable.GetExtensionFunctionAddressForPlatform = Mine_clGetExtensionFunctionAddressForPlatform;
    dispatchTable.CreateFromGLTexture = Mine_clCreateFromGLTexture;
    dispatchTable.GetExtensionFunctionAddress = Mine_clGetExtensionFunctionAddress;

#ifdef OPENCL_FISSION_EXT_SUPPORT
    dispatchTable._reservedForDeviceFissionEXT[0] = (void*)Mine_clCreateSubDevicesEXT;
    dispatchTable._reservedForDeviceFissionEXT[1] = (void*)Mine_clRetainDeviceEXT;
    dispatchTable._reservedForDeviceFissionEXT[2] = (void*)Mine_clReleaseDeviceEXT;
#endif

#ifdef _WIN32
    dispatchTable._reservedForD3D10KHR[0] = (void*)Mine_clGetDeviceIDsFromD3D10KHR;
    dispatchTable._reservedForD3D10KHR[1] = (void*)Mine_clCreateFromD3D10BufferKHR;
    dispatchTable._reservedForD3D10KHR[2] = (void*)Mine_clCreateFromD3D10Texture2DKHR;
    dispatchTable._reservedForD3D10KHR[3] = (void*)Mine_clCreateFromD3D10Texture3DKHR;
    dispatchTable._reservedForD3D10KHR[4] = (void*)Mine_clEnqueueAcquireD3D10ObjectsKHR;
    dispatchTable._reservedForD3D10KHR[5] = (void*)Mine_clEnqueueReleaseD3D10ObjectsKHR;
#endif

    // OpenCL 2.0
    dispatchTable.CreateCommandQueueWithProperties = Mine_clCreateCommandQueueWithProperties;
    dispatchTable.CreatePipe = Mine_clCreatePipe;
    dispatchTable.GetPipeInfo = Mine_clGetPipeInfo;
    dispatchTable.SVMAlloc = Mine_clSVMAlloc;
    dispatchTable.SVMFree = Mine_clSVMFree;
    dispatchTable.EnqueueSVMFree = Mine_clEnqueueSVMFree;
    dispatchTable.EnqueueSVMMemcpy = Mine_clEnqueueSVMMemcpy;
    dispatchTable.EnqueueSVMMemFill = Mine_clEnqueueSVMMemFill;
    dispatchTable.EnqueueSVMMap = Mine_clEnqueueSVMMap;
    dispatchTable.EnqueueSVMUnmap = Mine_clEnqueueSVMUnmap;
    dispatchTable.CreateSamplerWithProperties = Mine_clCreateSamplerWithProperties;
    dispatchTable.SetKernelArgSVMPointer = Mine_clSetKernelArgSVMPointer;
    dispatchTable.SetKernelExecInfo = Mine_clSetKernelExecInfo;
}

void* AssignExtensionFunctionPointer(const char* pFuncName, void* pRealFuncPtr)
{
    void* pRetVal = pRealFuncPtr;

    if (pRealFuncPtr != NULL)
    {
        CL_FUNC_TYPE funcType = InitExtensionFunction(pFuncName, pRetVal);

        switch (funcType)
        {
            case CL_FUNC_TYPE_clSVMAllocAMD:
                pRetVal = (void*)Mine_clSVMAllocAMD;
                break;

            case CL_FUNC_TYPE_clSVMFreeAMD:
                pRetVal = (void*)Mine_clSVMFreeAMD;
                break;

            case CL_FUNC_TYPE_clEnqueueSVMFreeAMD:
                pRetVal = (void*)Mine_clEnqueueSVMFreeAMD;
                break;

            case CL_FUNC_TYPE_clEnqueueSVMMemcpyAMD:
                pRetVal = (void*)Mine_clEnqueueSVMMemcpyAMD;
                break;

            case CL_FUNC_TYPE_clEnqueueSVMMemFillAMD:
                pRetVal = (void*)Mine_clEnqueueSVMMemFillAMD;
                break;

            case CL_FUNC_TYPE_clEnqueueSVMMapAMD:
                pRetVal = (void*)Mine_clEnqueueSVMMapAMD;
                break;

            case CL_FUNC_TYPE_clEnqueueSVMUnmapAMD:
                pRetVal = (void*)Mine_clEnqueueSVMUnmapAMD;
                break;

            case CL_FUNC_TYPE_clSetKernelArgSVMPointerAMD:
                pRetVal = (void*)Mine_clSetKernelArgSVMPointerAMD;
                break;

            case CL_FUNC_TYPE_clSetKernelExecInfoAMD:
                pRetVal = (void*)Mine_clSetKernelExecInfoAMD;
                break;

            default:
                break;

        }
    }

    return pRetVal;
};
