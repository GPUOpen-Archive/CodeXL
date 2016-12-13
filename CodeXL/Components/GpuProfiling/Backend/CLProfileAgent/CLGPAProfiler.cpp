//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief The implementation for the GPA helper class.
//==============================================================================

#include <set>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "CLGPAProfiler.h"
#include "CLFunctionDefs.h"
#include "CLUtils.h"
#include "../Common/SeqIDGenerator.h"
#include "../Common/StringUtils.h"
#include "../Common/FileUtils.h"
#include "../Common/GlobalSettings.h"
#include "../Common/Version.h"
#include "../CLCommon/CLPlatformInfo.h"


#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osProcess.h>

using namespace std;
using namespace CLUtils;

extern CLGPAProfiler g_Profiler;

CLGPAProfiler::CLGPAProfiler() :
    m_isGPAOpened(false),
    m_uiCurKernelCount(0),
    m_uiMaxKernelCount(DEFAULT_MAX_KERNELS),
    m_uiOutputLineCount(0),
    m_bIsProfilingEnabled(true),
    m_bGPU(false),
    m_bDelayStartEnabled(false),
    m_bProfilerDurationEnabled(false),
    m_delayInMilliseconds(0ul),
    m_durationInMilliseconds(0ul)
{
    m_strOutputFile.clear();
    m_delayTimer = nullptr;
    m_durationTimer = nullptr;
}


void CLGPAProfilerTimerEndResponse(ProfilerTimerType timerType)
{
    switch (timerType)
    {
        case PROFILEDELAYTIMER:
            g_Profiler.EnableProfiling(true);
            unsigned long profilerDuration;

            if (g_Profiler.IsProfilerDurationEnabled(profilerDuration))
            {
                g_Profiler.CreateTimer(PROFILEDURATIONTIMER, profilerDuration);
                g_Profiler.SetTimerFinishHandler(PROFILEDURATIONTIMER, CLGPAProfilerTimerEndResponse);
                g_Profiler.StartTimer(PROFILEDURATIONTIMER);
            }

            break;

        case PROFILEDURATIONTIMER:
            g_Profiler.EnableProfiling(false);
            break;

        default:
            break;
    }
}

CLGPAProfiler::~CLGPAProfiler()
{
    // delete unreferenced user event, deferred kernel
    // If a program is well-written, both list should be empty
    for (UserEventList::iterator it = m_userEventList.begin(); it != m_userEventList.end(); it++)
    {
        delete(*it);
    }

    if (m_delayTimer != nullptr)
    {
        m_delayTimer->stopTimer();
        delete m_delayTimer;
    }

    if (m_durationTimer != nullptr)
    {
        m_durationTimer->stopTimer();
        delete m_durationTimer;
    }
}

bool CLGPAProfiler::Open(cl_command_queue commandQueue)
{
    SpAssertRet(!m_isGPAOpened) false;

    SeqIDGenerator::Instance()->EnableGenerator(false);
    m_isGPAOpened = m_GPAUtils.Open((void*)commandQueue);
    return m_isGPAOpened;
}

bool CLGPAProfiler::Close()
{
    bool retVal = true;
    SeqIDGenerator::Instance()->EnableGenerator(true);

    if (m_isGPAOpened)
    {
        retVal = m_GPAUtils.Close();
        m_isGPAOpened = !retVal;
    }

    return retVal;
}

bool CLGPAProfiler::AddContext(const cl_context context)
{
    return m_contextManager.AddContext(context);
}

bool CLGPAProfiler::RemoveContext(const cl_context context)
{
    return m_contextManager.RemoveContext(context);
}

bool CLGPAProfiler::AddKernel(const cl_kernel kernel)
{
    cl_context context;
    cl_int status = g_realDispatchTable.GetKernelInfo(kernel, CL_KERNEL_CONTEXT, sizeof(cl_context), &context, NULL);

    if (CL_SUCCESS != status)
    {
        return false;
    }

    return m_contextManager.AddKernelToContext(context, kernel);
}

bool CLGPAProfiler::RemoveMemObject(const cl_mem& mem)
{
    cl_context ctx;
    cl_int nRet = g_realDispatchTable.GetMemObjectInfo(mem, CL_MEM_CONTEXT, sizeof(cl_context), &ctx, NULL);

    if (nRet != CL_SUCCESS)
    {
        return false;
    }

    return m_contextManager.RemoveBuffer(ctx, mem);
}

bool CLGPAProfiler::RemoveKernel(const cl_kernel kernel)
{
    cl_context context;
    cl_int status = g_realDispatchTable.GetKernelInfo(kernel, CL_KERNEL_CONTEXT, sizeof(cl_context), &context, NULL);

    if (CL_SUCCESS != status)
    {
        return false;
    }

    return m_contextManager.RemoveKernelFromContext(context, kernel);
}

bool CLGPAProfiler::AddBuffer(const cl_context& context,
                              const cl_mem&      buffer,
                              cl_mem_flags      flags,
                              size_t            bufferSize,
                              void*             pHost)
{
    return m_contextManager.AddBufferToContext(context, buffer, flags, bufferSize, pHost);
}

bool CLGPAProfiler::AddSubBuffer(const cl_mem& parentBuffer,
                                 const cl_mem& subBuffer,
                                 cl_mem_flags  flags,
                                 size_t        bufferSize)
{
    return m_contextManager.AddSubBuffer(parentBuffer, subBuffer, flags, bufferSize);
}

bool CLGPAProfiler::AddPipe(const cl_context context, const cl_mem pipe)
{
    return m_contextManager.AddPipeToContext(context, pipe);
}

bool CLGPAProfiler::AddKernelArg(const cl_kernel kernel, cl_uint argIdx, const void* pArgValue)
{
    cl_context context;
    cl_int status = g_realDispatchTable.GetKernelInfo(kernel, CL_KERNEL_CONTEXT, sizeof(cl_context), &context, NULL);

    if (CL_SUCCESS != status)
    {
        return false;
    }

    return m_contextManager.AddKernelArg(context, kernel, argIdx, pArgValue);
}

bool CLGPAProfiler::AddKernelArgSVMPointer(const cl_kernel kernel, cl_uint argIdx)
{
    bool retVal = false;

    cl_context context;
    cl_int status = g_realDispatchTable.GetKernelInfo(kernel, CL_KERNEL_CONTEXT, sizeof(cl_context), &context, NULL);

    if (CL_SUCCESS == status)
    {
        retVal = m_contextManager.AddKernelArgSVMPointer(context, kernel, argIdx);
    }

    return retVal;
}

bool CLGPAProfiler::HasKernelArgSVMPointer(const cl_kernel kernel)
{
    bool retVal = false;

    cl_context context;
    cl_int status = g_realDispatchTable.GetKernelInfo(kernel, CL_KERNEL_CONTEXT, sizeof(cl_context), &context, NULL);

    if (CL_SUCCESS == status)
    {
        retVal = m_contextManager.HasKernelArgSVMPointer(context, kernel);
    }

    return retVal;
}

bool CLGPAProfiler::AddKernelArgPipe(const cl_kernel kernel, cl_uint argIdx)
{
    bool retVal = false;

    cl_context context;
    cl_int status = g_realDispatchTable.GetKernelInfo(kernel, CL_KERNEL_CONTEXT, sizeof(cl_context), &context, NULL);

    if (CL_SUCCESS == status)
    {
        retVal = m_contextManager.AddKernelArgPipe(context, kernel, argIdx);
    }

    return retVal;
}

bool CLGPAProfiler::HasKernelArgPipe(const cl_kernel kernel)
{
    bool retVal = false;

    cl_context context;
    cl_int status = g_realDispatchTable.GetKernelInfo(kernel, CL_KERNEL_CONTEXT, sizeof(cl_context), &context, NULL);

    if (CL_SUCCESS == status)
    {
        retVal = m_contextManager.HasKernelArgPipe(context, kernel);
    }

    return retVal;
}

bool CLGPAProfiler::GetDeviceName(char* pszDeviceName, cl_command_queue commandQueue) const
{
    if (NULL == pszDeviceName)
    {
        assert(!"GetDeviceName: pszDeviceName is NULL\n");
        return false;
    }

    cl_device_id device;

    // get device from the command queue
    if (CL_SUCCESS != g_realDispatchTable.GetCommandQueueInfo(commandQueue, CL_QUEUE_DEVICE, sizeof(cl_device_id), &device, NULL))
    {
        return false;
    }

    // get the device type
    cl_device_type deviceType;

    if (CL_SUCCESS != g_realDispatchTable.GetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type), &deviceType, NULL) ||
        CL_DEVICE_TYPE_GPU != deviceType)
    {
        return false;
    }

    // get the GPU name
    if (CL_SUCCESS != g_realDispatchTable.GetDeviceInfo(device, CL_DEVICE_NAME, SP_MAX_PATH, pszDeviceName, NULL))
    {
        return false;
    }

    return true;
}

bool CLGPAProfiler::Init(const Parameters& params, string& strErrorOut)
{
    // Set kernel files
    m_KernelAssembly.SetOutputIL(params.m_bOutputIL);
    m_KernelAssembly.SetOutputISA(params.m_bOutputISA);
    m_KernelAssembly.SetOutputCL(params.m_bOutputCL);
    m_KernelAssembly.SetOutputHSAIL(params.m_bOutputHSAIL);

    // Set output file
    SetOutputFile(params.m_strOutputFile);

    // Set list separator
    KernelProfileResultManager::Instance()->SetListSeparator(params.m_cOutputSeparator);

    // Get platform info
    if (!CLUtils::GetPlatformInfo(m_platformList))
    {
        m_platformList.clear();
    }

    m_uiMaxKernelCount = params.m_uiMaxKernels;
    m_bForceSinglePass = params.m_bForceSinglePassPMC;
    m_bCollectGPUTime = m_bForceSinglePass ? false : params.m_bGPUTimePMC;
    size_t nMaxPass = m_bForceSinglePass ? 1 : GPA_INFINITE_PASS;

    if (!params.m_bStartDisabled)
    {
        m_bDelayStartEnabled = params.m_bDelayStartEnabled;
        m_bProfilerDurationEnabled = params.m_bProfilerDurationEnabled;
        m_delayInMilliseconds = params.m_delayInMilliseconds;
        m_durationInMilliseconds = params.m_durationInMilliseconds;
        m_bIsProfilingEnabled = m_delayInMilliseconds > 0 ? false : true;

        if (m_bDelayStartEnabled)
        {
            CreateTimer(PROFILEDELAYTIMER, m_delayInMilliseconds);
            m_delayTimer->SetTimerFinishHandler(CLGPAProfilerTimerEndResponse);
            m_delayTimer->startTimer(true);
        }
        else if (m_bProfilerDurationEnabled)
        {
            CreateTimer(PROFILEDURATIONTIMER, m_durationInMilliseconds);
            m_durationTimer->SetTimerFinishHandler(CLGPAProfilerTimerEndResponse);
            m_durationTimer->startTimer(true);
        }
    }
    else
    {
        m_bIsProfilingEnabled = !params.m_bStartDisabled;
    }


    gtString strDllPath = params.m_strDLLPath;

    if (strDllPath.isEmpty())
    {
        gtString strAgentPath;
        osGetCurrentProcessEnvVariableValue(L"CL_AGENT", strAgentPath);

        if (!strAgentPath.isEmpty())
        {
            osFilePath agentPath(strAgentPath);
            strDllPath = agentPath.fileDirectoryAsString();
            strDllPath.append(osFilePath::osPathSeparator);
        }
    }

    CounterList enabledCounters;
    m_GPAUtils.InitGPA(GPA_API_OPENCL,
                       strDllPath,
                       strErrorOut,
                       params.m_strCounterFile.empty() ? NULL : params.m_strCounterFile.c_str(),
                       &enabledCounters,
                       nMaxPass);

    // Enable all counters if no counter file is specified or counter file is empty.
    if (enabledCounters.empty())
    {
#ifdef AMDT_INTERNAL
        // Internal mode must have a counter file specified.
        cout << "Please specify a counter file using -c. No counter is enabled." << endl;
        return false;
#else

        // get the device ids for all installed adapters
        AsicInfoList asicInfoList;
        AMDTADLUtils::Instance()->GetAsicInfoList(asicInfoList);

        set<string> counterSet;
        vector<string> tempCounters;

        // Enable all counters
        for (CLPlatformSet::iterator idxPlatform = m_platformList.begin(); idxPlatform != m_platformList.end(); idxPlatform++)
        {
            GDT_HW_GENERATION gen;
            vector<string> counterNames;

            string translatedDeviceName = AMDTDeviceInfoUtils::Instance()->TranslateDeviceName(idxPlatform->strDeviceName.c_str());

            int deviceId;
            int revisionId;
            bool retVal = GetAvailableDeviceIdFromDeviceNameAndAsicInfoList(translatedDeviceName.c_str(), asicInfoList, deviceId, revisionId);

            if (retVal)
            {
                m_GPAUtils.GetAvailableCountersForDevice(deviceId, revisionId, nMaxPass, counterNames);
            }
            else
            {
                if (AMDTDeviceInfoUtils::Instance()->GetHardwareGeneration(translatedDeviceName.c_str(), gen))
                {
                    switch (gen)
                    {
                        case GDT_HW_GENERATION_SOUTHERNISLAND:
                        {
                            m_GPAUtils.GetAvailableCounters(GPA_HW_GENERATION_SOUTHERNISLAND, counterNames);
                            break;
                        }

                        case GDT_HW_GENERATION_SEAISLAND:
                        {
                            m_GPAUtils.GetAvailableCounters(GPA_HW_GENERATION_SEAISLAND, counterNames);
                            break;
                        }

                        case GDT_HW_GENERATION_VOLCANICISLAND:
                        {
                            m_GPAUtils.GetAvailableCounters(GPA_HW_GENERATION_VOLCANICISLAND, counterNames);
                            break;
                        }

                        default:
                            break;
                    }
                }
            }

            tempCounters.insert(tempCounters.end(), counterNames.begin(), counterNames.end());
        }

        // remove duplicated counter
        for (vector<string>::iterator it = tempCounters.begin(); it != tempCounters.end(); ++it)
        {
            if (counterSet.find(*it) == counterSet.end())
            {
                counterSet.insert(*it);
                enabledCounters.push_back(*it);
            }
        }

        m_GPAUtils.SetEnabledCounters(enabledCounters);

#endif //AMDT_INTERNAL
    }

    // Init CSV file header and column row
    InitHeader();

    for (vector<string>::iterator it = enabledCounters.begin(); it != enabledCounters.end(); ++it)
    {
        KernelProfileResultManager::Instance()->AddProfileResultItem(*it);
    }

    return true;
}

bool CLGPAProfiler::EnableCounters(cl_command_queue commandQueue)
{
    bool retVal = m_isGPAOpened;

    if (m_isGPAOpened)
    {
        if (m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_SelectContext(commandQueue)) != GPA_STATUS_OK)
        {
            // make sure that the current context has been opened by GPA
            retVal = false;
        }
        else
        {
            retVal = m_GPAUtils.EnableCounters();
        }
    }

    return retVal;
}

bool CLGPAProfiler::FullProfile(
    cl_command_queue commandQueue,
    cl_kernel        kernel,
    cl_uint          uWorkDim,
    const size_t*    pGlobalWorkOffset,
    const size_t*    pGlobalWorkSize,
    const size_t*    pLocalWorkSize,
    cl_uint          uEventWaitList,
    const cl_event*  pEventWaitList,
    cl_event*        pEvent,
    cl_int&          nResultOut,
    gpa_uint32&      uSessionIDOut,
    double&          dKernelTimeOut)
{
    if (!m_GPAUtils.Loaded())
    {
        return false;
    }

    ++m_uiCurKernelCount;

    // get context from the command queue
    cl_context context = NULL;
    g_realDispatchTable.GetCommandQueueInfo(commandQueue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);

    bool kernelAlreadyDispatched = false;

    if (!m_bForceSinglePass)
    {
        // save related memory buffers (that are used for both read and write) before
        // we dispatch the kernel so we can load these buffers prior to starting
        // the next pass of the kernel
        // only need to save if kernel has r/w buffers and we are doing multi-pass
        m_contextManager.SaveArena(context, commandQueue, kernel);
    }

    if (m_bCollectGPUTime)
    {
        cl_event* pTmpEvent = pEvent;
        cl_event event1;

        if (pTmpEvent == NULL)
        {
            pTmpEvent = &event1;
        }

        // run the kernel once to workaround the high cost of the first kernel run.  NOTE: GPU Time is also collected for this run
        // call the entry in g_nextDispatchTable to pass the call down the agent chain to the runtime
        nResultOut = g_nextDispatchTable.EnqueueNDRangeKernel(commandQueue,
                                                              kernel,
                                                              uWorkDim,
                                                              pGlobalWorkOffset,
                                                              pGlobalWorkSize,
                                                              pLocalWorkSize,
                                                              uEventWaitList,
                                                              pEventWaitList,
                                                              pTmpEvent);

        if (CL_SUCCESS != nResultOut)
        {
            // there is a cl error, so we don't need to continue
            return false;
        }

        kernelAlreadyDispatched = true;

        // get the kernel time
        CLUtils::GetElapsedTimeFromEvent(pTmpEvent, dKernelTimeOut);

        if (NULL == pEvent)
        {
            // release the temporary event created otherwise buffer used won't get released
            g_realDispatchTable.ReleaseEvent(*pTmpEvent);
        }
    }
    else
    {
        dKernelTimeOut = 0;
    }

    if (m_isGPAOpened)
    {
        if (GPA_STATUS_OK == m_GPAUtils.GetGPALoader().GPA_BeginSession(&uSessionIDOut))
        {
            gpa_uint32 GPA_pCountTemp;
            m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_GetPassCount(&GPA_pCountTemp));

            for (gpa_uint32 GPA_loopTemp = 0; GPA_loopTemp < GPA_pCountTemp; GPA_loopTemp++)
            {
                if (!m_bForceSinglePass && kernelAlreadyDispatched)
                {
                    // load related memory buffers before starting the next kernel dispatch
                    // so that we can guarantee correctness of the multiple passes of the same kernel
                    m_contextManager.LoadArena(context, commandQueue, kernel);

                    if (NULL != pEvent)
                    {
                        // release the created event otherwise buffer used won't get released
                        g_realDispatchTable.ReleaseEvent(*pEvent);
                    }
                }

                m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_BeginPass());
                m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_BeginSample(0));

                // call the entry in g_realDispatchTable to prevent other agents from seeing the replayed kernels
                cl_int clRetVal = g_realDispatchTable.EnqueueNDRangeKernel(commandQueue,
                                                                           kernel,
                                                                           uWorkDim,
                                                                           pGlobalWorkOffset,
                                                                           pGlobalWorkSize,
                                                                           pLocalWorkSize,
                                                                           uEventWaitList,
                                                                           pEventWaitList,
                                                                           pEvent);

                if (!kernelAlreadyDispatched)
                {
                    nResultOut = clRetVal;
                }

                kernelAlreadyDispatched = true;

                m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_EndSample());
                m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_EndPass());

                if (nResultOut != CL_SUCCESS)
                {
                    break;
                }

                if (NULL != pEvent)
                {
                    g_realDispatchTable.WaitForEvents(1, pEvent);
                }
            }

            m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_EndSession());

            if (!m_bForceSinglePass)
            {
                m_contextManager.ClearArena(context, kernel);
            }

            return true;
        }
    }

    if (!kernelAlreadyDispatched)
    {
        // make sure we dispatch the kernel at least once see BUG450137
        nResultOut = g_nextDispatchTable.EnqueueNDRangeKernel(commandQueue,
                                                              kernel,
                                                              uWorkDim,
                                                              pGlobalWorkOffset,
                                                              pGlobalWorkSize,
                                                              pLocalWorkSize,
                                                              uEventWaitList,
                                                              pEventWaitList,
                                                              pEvent);
    }

    return false;
}

bool CLGPAProfiler::GenerateKernelAssembly(const cl_command_queue& commandQueue,
                                           const cl_kernel&        kernel,
                                           const std::string&      strKernelName,
                                           const std::string&      strKernelHandle)
{
    std::string strOutputDir;

    if (!FileUtils::GetWorkingDirectory(m_strOutputFile, strOutputDir))
    {
        return false;
    }

    return m_KernelAssembly.Generate(commandQueue,
                                     kernel,
                                     strKernelName,
                                     strKernelHandle,
                                     strOutputDir);

}

const KernelInfo& CLGPAProfiler::GetKernelInfoFromKernelAssembly(std::string& strKernelName) const
{
    return m_KernelAssembly.GetKernelInfo(strKernelName);
}

void CLGPAProfiler::InitHeader()
{
    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("ProfileFileVersion=%d.%d", GPUPROFILER_BACKEND_MAJOR_VERSION, GPUPROFILER_BACKEND_MINOR_VERSION));
    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("ProfilerVersion=%d.%d.%d", GPUPROFILER_BACKEND_MAJOR_VERSION, GPUPROFILER_BACKEND_MINOR_VERSION, GPUPROFILER_BACKEND_BUILD_NUMBER));
    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("API=OpenCL"));
    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("Application=%s", FileUtils::GetExeFullPath().c_str()));
    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("ApplicationArgs=%s", GlobalSettings::GetInstance()->m_params.m_strCmdArgs.asUTF8CharArray()));
    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("WorkingDirectory=%s", GlobalSettings::GetInstance()->m_params.m_strWorkingDir.asUTF8CharArray()));

    EnvVarMap envVarMap = GlobalSettings::GetInstance()->m_params.m_mapEnvVars;

    if (!envVarMap.empty())
    {
        KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("FullEnvironment=%d", GlobalSettings::GetInstance()->m_params.m_bFullEnvBlock));

        for (EnvVarMap::const_iterator it = envVarMap.begin(); it != envVarMap.end(); ++it)
        {
            KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("EnvVar=%s=%s", it->first.asUTF8CharArray(), it->second.asUTF8CharArray()));
        }
    }

    for (CLPlatformSet::iterator idxPlatform = m_platformList.begin(); idxPlatform != m_platformList.end(); idxPlatform++)
    {
        KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("Device %s Platform Vendor=%s", idxPlatform->strDeviceName.c_str(), idxPlatform->strPlatformVendor.c_str()));
        KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("Device %s Platform Name=%s", idxPlatform->strDeviceName.c_str(), idxPlatform->strPlatformName.c_str()));
        KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("Device %s Platform Version=%s", idxPlatform->strDeviceName.c_str(), idxPlatform->strPlatformVersion.c_str()));
        KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("Device %s CLDriver Version=%s", idxPlatform->strDeviceName.c_str(), idxPlatform->strDriverVersion.c_str()));
        KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("Device %s CLRuntime Version=%s", idxPlatform->strDeviceName.c_str(), idxPlatform->strCLRuntime.c_str()));
        KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("Device %s NumberAppAddressBits=%d", idxPlatform->strDeviceName.c_str(), idxPlatform->uiNbrAddressBits));
    }

    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("OS Version=%s", OSUtils::Instance()->GetOSInfo().c_str()));
    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("DisplayName=%s", GlobalSettings::GetInstance()->m_params.m_strSessionName.c_str()));
    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("ListSeparator=%c", GlobalSettings::GetInstance()->m_params.m_cOutputSeparator));
    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("ForceSinglePass=%s", GlobalSettings::GetInstance()->m_params.m_bForceSinglePassPMC ? "True" : "False"));

    if (GlobalSettings::GetInstance()->m_params.m_kernelFilterList.size() > 0)
    {
        std::string strKernelList;

        for (auto kernelName : GlobalSettings::GetInstance()->m_params.m_kernelFilterList)
        {
            if (!strKernelList.empty())
            {
                strKernelList.append(",");
            }

            strKernelList.append(kernelName);
        }

        KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("KernelsProfiled=%s", strKernelList.c_str()));
    }
    else if (!GlobalSettings::GetInstance()->m_params.m_strKernelFile.empty())
    {
        KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("KernelsProfiled=A kernel list file %s was specified, but no kernels were read from it. All kernels have been profiled", GlobalSettings::GetInstance()->m_params.m_strKernelFile.c_str()));
    }

    if (m_uiMaxKernelCount != DEFAULT_MAX_KERNELS)
    {
        KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("MaxNumberOfKernelsToProfile=%d", m_uiMaxKernelCount));
    }

    KernelProfileResultManager::Instance()->AddProfileResultItem("Method");
    KernelProfileResultManager::Instance()->AddProfileResultItem("ExecutionOrder");
    KernelProfileResultManager::Instance()->AddProfileResultItem("ThreadID");
    KernelProfileResultManager::Instance()->AddProfileResultItem("CallIndex");
    KernelProfileResultManager::Instance()->AddProfileResultItem("GlobalWorkSize");
    KernelProfileResultManager::Instance()->AddProfileResultItem("WorkGroupSize");

    if (m_bCollectGPUTime)
    {
        KernelProfileResultManager::Instance()->AddProfileResultItem("Time");
    }

    KernelProfileResultManager::Instance()->AddProfileResultItem("LocalMemSize");
    KernelProfileResultManager::Instance()->AddProfileResultItem("VGPRs");
    KernelProfileResultManager::Instance()->AddProfileResultItem("SGPRs");
    KernelProfileResultManager::Instance()->AddProfileResultItem("ScratchRegs");
}

void CLGPAProfiler::DumpKernelStats(const KernelStats& kernelStats)
{
    ++m_uiOutputLineCount;

    KernelProfileResultManager::Instance()->WriteKernelInfo("Method", kernelStats.m_strName);
    KernelProfileResultManager::Instance()->WriteKernelInfo("ExecutionOrder", m_uiOutputLineCount);
    KernelProfileResultManager::Instance()->WriteKernelInfo("ThreadID", kernelStats.m_threadId);
    KernelProfileResultManager::Instance()->WriteKernelInfo("CallIndex", kernelStats.m_uSequenceId);

    if (0 == kernelStats.m_globalWorkSize[0] &&
        0 == kernelStats.m_globalWorkSize[1] &&
        0 == kernelStats.m_globalWorkSize[2])
    {
        KernelProfileResultManager::Instance()->WriteKernelInfo("GlobalWorkSize", "NULL");
    }
    else
    {
        KernelProfileResultManager::Instance()->WriteKernelInfo("GlobalWorkSize", StringUtils::FormatString("{%7lu %7lu %7lu}",
                                                                (unsigned long) kernelStats.m_globalWorkSize[0],
                                                                (unsigned long) kernelStats.m_globalWorkSize[1],
                                                                (unsigned long) kernelStats.m_globalWorkSize[2]));
    }

    if (0 == kernelStats.m_workGroupSize[0] &&
        0 == kernelStats.m_workGroupSize[1] &&
        0 == kernelStats.m_workGroupSize[2])
    {
        KernelProfileResultManager::Instance()->WriteKernelInfo("WorkGroupSize", "NULL");
    }
    else
    {
        KernelProfileResultManager::Instance()->WriteKernelInfo("WorkGroupSize", StringUtils::FormatString("{%5lu %5lu %5lu}",
                                                                (unsigned long) kernelStats.m_workGroupSize[0],
                                                                (unsigned long) kernelStats.m_workGroupSize[1],
                                                                (unsigned long) kernelStats.m_workGroupSize[2]));
    }

    if (m_bCollectGPUTime)
    {
        KernelProfileResultManager::Instance()->WriteKernelInfo("Time", StringUtils::FormatString("%15.5lf", kernelStats.m_dTime));
    }

    KernelProfileResultManager::Instance()->WriteKernelInfo("LocalMemSize", kernelStats.m_kernelInfo.m_nUsedLDSSize == KERNELINFO_NONE ? "NA" : StringUtils::ToString(kernelStats.m_kernelInfo.m_nUsedLDSSize));
    KernelProfileResultManager::Instance()->WriteKernelInfo("VGPRs", kernelStats.m_kernelInfo.m_nUsedGPRs == KERNELINFO_NONE ? "NA" : StringUtils::ToString(kernelStats.m_kernelInfo.m_nUsedGPRs));
    KernelProfileResultManager::Instance()->WriteKernelInfo("SGPRs", kernelStats.m_kernelInfo.m_nUsedScalarGPRs == KERNELINFO_NONE ? "NA" : StringUtils::ToString(kernelStats.m_kernelInfo.m_nUsedScalarGPRs));
    KernelProfileResultManager::Instance()->WriteKernelInfo("ScratchRegs", kernelStats.m_kernelInfo.m_nScratchReg == KERNELINFO_NONE ? "NA" : StringUtils::ToString(kernelStats.m_kernelInfo.m_nScratchReg));
}

bool CLGPAProfiler::DumpSession(gpa_uint32 uSessionID, const KernelStats& kernelStats)
{
    if (!m_GPAUtils.Loaded())
    {
        return false;
    }

    KernelProfileResultManager::Instance()->BeginKernelInfo();

    bool bReadyResult = false;
    GPA_Status sessionStatus = GPA_STATUS_ERROR_FAILED;

    if (m_isGPAOpened)
    {
        sessionStatus = m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_IsSessionReady(&bReadyResult, uSessionID));
    }

    // check whether the session is ready and the result has returned from GPU
    if (!bReadyResult || GPA_STATUS_OK != sessionStatus)
    {
        // just CPU profiling (time)
        DumpKernelStats(kernelStats);
        KernelProfileResultManager::Instance()->EndKernelInfo();
        return false;
    }

    if (m_isGPAOpened)
    {
        gpa_uint32 sampleCount;
        m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_GetSampleCount(uSessionID, &sampleCount));

        for (gpa_uint32 sample = 0; sample < sampleCount; sample++)
        {
            DumpKernelStats(kernelStats);

            gpa_uint32 nEnabledCounters = 0;
            m_GPAUtils.GetGPALoader().GPA_GetEnabledCount(&nEnabledCounters);

            for (gpa_uint32 counter = 0; counter < nEnabledCounters; counter++)
            {
                gpa_uint32 enabledCounterIndex;
                m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_GetEnabledIndex(counter, &enabledCounterIndex));
                GPA_Type type;
                m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_GetCounterDataType(enabledCounterIndex, &type));

                const char* pName = NULL;
                m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_GetCounterName(enabledCounterIndex, &pName));
                string strName;

                if (pName != NULL)
                {
                    strName = pName;
                }
                else
                {
                    SpBreak("Failed to retrieve counter name.");
                    continue;
                }

                if (type == GPA_TYPE_UINT32)
                {
                    gpa_uint32 value;
                    m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_GetSampleUInt32(uSessionID, sample, enabledCounterIndex, &value));
                    KernelProfileResultManager::Instance()->WriteKernelInfo(strName, StringUtils::FormatString("%8u", value));
                }
                else if (type == GPA_TYPE_UINT64)
                {
                    gpa_uint64 value;
                    m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_GetSampleUInt64(uSessionID, sample, enabledCounterIndex, &value));

#ifdef _WIN32
                    KernelProfileResultManager::Instance()->WriteKernelInfo(strName, StringUtils::FormatString("%8I64u", value));
#else
                    KernelProfileResultManager::Instance()->WriteKernelInfo(strName, StringUtils::FormatString("%lu", value));
#endif

                }
                else if (type == GPA_TYPE_FLOAT32)
                {
                    gpa_float32 value;
                    m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_GetSampleFloat32(uSessionID, sample, enabledCounterIndex, &value));

                    KernelProfileResultManager::Instance()->WriteKernelInfo(strName, StringUtils::FormatString("%12.2f", value));
                }
                else if (type == GPA_TYPE_FLOAT64)
                {
                    gpa_float64 value;
                    m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_GetSampleFloat64(uSessionID, sample, enabledCounterIndex, &value));

                    KernelProfileResultManager::Instance()->WriteKernelInfo(strName, StringUtils::FormatString("%12.2f", value));
                }
                else
                {
                    assert(false);
                    return false;
                }
            }
        }
    }

    KernelProfileResultManager::Instance()->EndKernelInfo();

    return true;
}

std::string CLGPAProfiler::GetStatusString(GPA_Status status) const
{
    std::string result;

    switch (status)
    {
        case GPA_STATUS_ERROR_NULL_POINTER :
            result = "Null pointer";
            break;

        case GPA_STATUS_ERROR_COUNTERS_NOT_OPEN :
            result = "Counters not opened";
            break;

        case GPA_STATUS_ERROR_COUNTERS_ALREADY_OPEN :
            result = "Counters already opened";
            break;

        case GPA_STATUS_ERROR_INDEX_OUT_OF_RANGE :
            result = "Index out of range";
            break;

        case GPA_STATUS_ERROR_NOT_FOUND :
            result = "Not found";
            break;

        case GPA_STATUS_ERROR_ALREADY_ENABLED :
            result = "Already enabled";
            break;

        case GPA_STATUS_ERROR_NO_COUNTERS_ENABLED :
            result = "No counters enabled";
            break;

        case GPA_STATUS_ERROR_NOT_ENABLED :
            result = "Not enabled";
            break;

        case GPA_STATUS_ERROR_SAMPLING_NOT_STARTED :
            result = "Sampling not started";
            break;

        case GPA_STATUS_ERROR_SAMPLING_ALREADY_STARTED :
            result = "Sampling already started";
            break;

        case GPA_STATUS_ERROR_SAMPLING_NOT_ENDED :
            result = "Sampling not ended";
            break;

        case GPA_STATUS_ERROR_NOT_ENOUGH_PASSES :
            result = "Not enough passes";
            break;

        case GPA_STATUS_ERROR_PASS_NOT_ENDED :
            result = "Pass not ended";
            break;

        case GPA_STATUS_ERROR_PASS_NOT_STARTED :
            result = "Pass not started";
            break;

        case GPA_STATUS_ERROR_PASS_ALREADY_STARTED :
            result = "Pass already started";
            break;

        case GPA_STATUS_ERROR_SAMPLE_NOT_STARTED :
            result = "Sample not found";
            break;

        case GPA_STATUS_ERROR_SAMPLE_ALREADY_STARTED :
            result = "Sample already started";
            break;

        case GPA_STATUS_ERROR_SAMPLE_NOT_ENDED :
            result = "sample not ended";
            break;

        case GPA_STATUS_ERROR_CANNOT_CHANGE_COUNTERS_WHEN_SAMPLING :
            result = "Cannot change counters when sampling";
            break;

        case GPA_STATUS_ERROR_SESSION_NOT_FOUND :
            result = "Session not found";
            break;

        case GPA_STATUS_ERROR_SAMPLE_NOT_FOUND :
            result = "Sample not found";
            break;

        case GPA_STATUS_ERROR_SAMPLE_NOT_FOUND_IN_ALL_PASSES :
            result = "Sample not found in all passes";
            break;

        case GPA_STATUS_ERROR_COUNTER_NOT_OF_SPECIFIED_TYPE :
            result = "Counter not of specified type";
            break;

        case GPA_STATUS_ERROR_READING_COUNTER_RESULT :
            result = "Error reading counter result";
            break;

        case GPA_STATUS_ERROR_VARIABLE_NUMBER_OF_SAMPLES_IN_PASSES :
            result = "Variable number of samples in passes";
            break;

        case GPA_STATUS_ERROR_FAILED :
            result = "Failed";
            break;

        case GPA_STATUS_ERROR_HARDWARE_NOT_SUPPORTED :
            result = "Hardware not supported";
            break;

        case GPA_STATUS_ERROR_DRIVER_NOT_SUPPORTED:
            result = "Driver version not supported";
            break;

        default:

            if (status != GPA_STATUS_OK)
            {
                result = "Unknown error";
            }

            break;
    }

    return result;
}

void CLGPAProfiler::AddUserEvent(cl_event userEvent)
{
    CLUserEvent* pUserEventWrapper = new(nothrow) CLUserEvent(userEvent);

    if (pUserEventWrapper != NULL)
    {
        m_userEventList.push_back(pUserEventWrapper);
    }
}

bool CLGPAProfiler::HasUserEvent(const cl_event* wait_list, int num, cl_event* event)
{
    for (UserEventList::iterator it = m_userEventList.begin(); it != m_userEventList.end(); it++)
    {
        for (int i = 0; i < num; i++)
        {
            if ((*it)->CheckDependency(wait_list[i]))
            {
                if (event != NULL)
                {
                    (*it)->AddDependentEvent(*event);
                }

                return true;
            }
        }
    }

    return false;
}

CLUserEvent* CLGPAProfiler::HasUserEvent(const cl_event* wait_list, int num)
{
    for (UserEventList::iterator it = m_userEventList.begin(); it != m_userEventList.end(); it++)
    {
        for (int i = 0; i < num; i++)
        {
            if (wait_list[i] == (*it)->m_event)
            {
                return *it;
            }
        }
    }

    return NULL;
}

void CLGPAProfiler::RemoveUserEvent(cl_event event)
{
    UserEventList::iterator found = m_userEventList.end();

    for (UserEventList::iterator it = m_userEventList.begin(); it != m_userEventList.end(); it++)
    {
        if (event == (*it)->m_event)
        {
            found = it;
            break;
        }
    }

    if (found != m_userEventList.end())
    {
        delete(*found);
        m_userEventList.erase(found);
    }
}


bool CLGPAProfiler::IsProfilerDelayEnabled(unsigned long& delayInMilliseconds)
{
    delayInMilliseconds = m_delayInMilliseconds;
    return m_bDelayStartEnabled;
}


bool CLGPAProfiler::IsProfilerDurationEnabled(unsigned long& durationInMilliseconds)
{
    durationInMilliseconds = m_durationInMilliseconds;
    return m_bProfilerDurationEnabled;
}


void CLGPAProfiler::SetTimerFinishHandler(ProfilerTimerType timerType, TimerEndHandler timerEndHandler)
{
    if (m_delayTimer || m_durationTimer)
    {

        switch (timerType)
        {
            case PROFILEDELAYTIMER:
                m_delayTimer->SetTimerFinishHandler(timerEndHandler);
                break;

            case PROFILEDURATIONTIMER:
                m_durationTimer->SetTimerFinishHandler(timerEndHandler);
                break;

            default:
                break;
        }
    }
}

void CLGPAProfiler::CreateTimer(ProfilerTimerType timerType, unsigned long timeIntervalInMilliseconds)
{
    switch (timerType)
    {
        case PROFILEDELAYTIMER:
            if (m_delayTimer == nullptr && timeIntervalInMilliseconds > 0)
            {
                m_delayTimer = new ProfilerTimer(timeIntervalInMilliseconds);
                m_delayTimer->SetTimerType(PROFILEDELAYTIMER);
                m_bDelayStartEnabled = true;
                m_delayInMilliseconds = timeIntervalInMilliseconds;
            }

            break;

        case PROFILEDURATIONTIMER:
            if (m_durationTimer == nullptr && timeIntervalInMilliseconds > 0)
            {
                m_durationTimer = new ProfilerTimer(timeIntervalInMilliseconds);
                m_durationTimer->SetTimerType(PROFILEDURATIONTIMER);
                m_bProfilerDurationEnabled = true;
                m_durationInMilliseconds = timeIntervalInMilliseconds;
            }

            break;

        default:
            break;
    }
}


void CLGPAProfiler::StartTimer(ProfilerTimerType timerType)
{
    switch (timerType)
    {
        case PROFILEDELAYTIMER:
            m_delayTimer->startTimer(true);
            break;

        case PROFILEDURATIONTIMER:
            m_durationTimer->startTimer(true);
            break;

        default:
            break;
    }

}

void CLGPAProfiler::SetOutputFile(const std::string& strOutputFile)
{
    if (strOutputFile.empty())
    {
        // If output file is not set, we use exe name as file name
        m_strOutputFile = FileUtils::GetDefaultOutputPath() + FileUtils::GetExeName() + "." + PERF_COUNTER_EXT;
    }
    else
    {
        std::string strExtension("");

        strExtension = FileUtils::GetFileExtension(strOutputFile);

        if (strExtension != PERF_COUNTER_EXT)
        {
            if ((strExtension == TRACE_EXT) || (strExtension == OCCUPANCY_EXT))
            {
                std::string strBaseFileName = FileUtils::GetBaseFileName(strOutputFile);
                m_strOutputFile = strBaseFileName + "." + PERF_COUNTER_EXT;
            }
            else
            {
                m_strOutputFile = strOutputFile + "." + PERF_COUNTER_EXT;
            }
        }
        else
        {
            m_strOutputFile = strOutputFile;
        }
    }

    KernelProfileResultManager::Instance()->SetOutputFile(m_strOutputFile);
}

bool CLGPAProfiler::GetAvailableDeviceIdFromDeviceNameAndAsicInfoList(const char* pszDeviceName, const AsicInfoList asicInfoList, int& deviceId, int& revisionId)
{
    bool retVal = false;
    deviceId = -1;
    revisionId = -1;

    for (AsicInfoList::const_iterator it = asicInfoList.begin(); it != asicInfoList.end(); ++it)
    {
        GDT_GfxCardInfo cardInfo;
        int curDeviceId = it->deviceID;
        int curRevisionId = it->revID;

        if (AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(curDeviceId, curRevisionId, cardInfo))
        {
            if (strcmp(cardInfo.m_szCALName, pszDeviceName) == 0)
            {
                retVal = true;
                deviceId = curDeviceId;
                revisionId = curRevisionId;
                break;
            }
        }
    }

    return retVal;
}

