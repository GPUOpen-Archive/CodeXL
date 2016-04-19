//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#include "DCGPAProfiler.h"

#include <unordered_set>

#include "DCDetourID3D11DeviceContext.h"
#include "DCFuncDefs.h"
#include "..\Common\StringUtils.h"
#include "DCUtils.h"
#include "../Common/FileUtils.h"
#include "../Common/GlobalSettings.h"
#include "../Common/KernelProfileResultManager.h"
#include "../Common/LocaleSetting.h"

using std::vector;
using std::string;

static const std::string GPU_TIME_COUNTER_NAME("GPUTime");
static const std::string CS_COUNTER_NAME_PREFIX("CS");

DCGPAProfiler::DCGPAProfiler(void)
    :
    m_uiCurDispatchCount(0),
    m_uiMaxDispatchCount(DEFAULT_MAX_KERNELS),
    m_profilerParams(),
    m_uiSequenceNum(0),
    m_pID3D11Device(NULL),
    m_bDoneHeadings(false),
    m_enabledCounters(),
    m_deferLoadCounters(false)
{
    // Note: m_enabledCounters is lazy initialized in the dump header method
}

DCGPAProfiler::~DCGPAProfiler(void)
{
}

bool DCGPAProfiler::Open()
{
    return m_GPAUtils.Open(m_pID3D11Device);
}

bool DCGPAProfiler::Close()
{
    // check original resources and UAVs ref counter, if equal to 1,
    // it means client has released the original resources or UAVs, we need to
    // remove the backup ones we created
    m_DCContextMgr.Cleanup();
    return m_GPAUtils.Close();
}

bool DCGPAProfiler::EnableCounters()
{
    bool retVal = true;

    if (m_deferLoadCounters)
    {
        CounterList counterList;

        retVal = GetAllComputeCounters(counterList);

        if (retVal)
        {
            retVal = m_GPAUtils.SetEnabledCounters(counterList);
        }

        m_deferLoadCounters = false;
    }

    if (retVal)
    {
        retVal = m_GPAUtils.EnableCounters();
    }

    return retVal;
}

void DCGPAProfiler::MapCounterNames(CounterList& selectedCounters)
{
    for (CounterList::iterator it = selectedCounters.begin(); it != selectedCounters.end(); ++it)
    {
        string str = *it;

        if (str.compare("FetchUnitBusy") == 0)
        {
            *it = "TexUnitBusy";
        }
        else if (str.compare("FetchUnitStalled") == 0)
        {
            *it = "TexUnitStalled";
        }
        else if (str.compare("WriteUnitStalled") == 0)
        {
            *it = "PSExportStalls";
        }
        else if (str.compare("FetchSize") == 0)
        {
            *it = "TexMemBytesRead";
        }
        else
        {
            *it = CS_COUNTER_NAME_PREFIX + *it;
        }
    }
}

bool DCGPAProfiler::FullProfile(
    ID3D11DeviceContext* pObj,
    UINT ThreadGroupCountX,
    UINT ThreadGroupCountY,
    UINT ThreadGroupCountZ,
    gpa_uint32& uSessionIDOut)
{
    if (m_uiCurDispatchCount >= m_uiMaxDispatchCount)
    {
        // we've reached the limit for the number of dispatches to profile, so we just dispatch and return
        Real_ID3D11DeviceContext_Dispatch(GetID3D11DeviceContextVTableManager(), pObj, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
        return false;
    }

    if (GPA_STATUS_OK == m_GPAUtils.GetGPALoader().GPA_BeginSession(&uSessionIDOut))
    {
        //update backup UAVs
        if (!m_DCContextMgr.RestoreContext())
        {
            SpAssert(!"Failed to RestoreContext\n");
            m_GPAUtils.GetGPALoader().GPA_EndSession();
            //run dispatch
            Real_ID3D11DeviceContext_Dispatch(GetID3D11DeviceContextVTableManager(), pObj, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
            return false;
        }

        //call CSSetUnorderedAccessViews() again to set binded UAVs to the backup UAVs which created by DCContextMgr
        //Because we don't want to mess up client's original data
        if (!m_DCContextMgr.PushContext())
        {
            SpAssert(!"Failed to PushContext\n");
            m_GPAUtils.GetGPALoader().GPA_EndSession();
            //run dispatch
            Real_ID3D11DeviceContext_Dispatch(GetID3D11DeviceContextVTableManager(), pObj, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
            return false;
        }

        gpa_uint32 GPA_pCountTemp;
        m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_GetPassCount(&GPA_pCountTemp));

        for (gpa_uint32 GPA_loopTemp = 0 ; GPA_loopTemp < GPA_pCountTemp ; GPA_loopTemp++)
        {
            GPA_Status status = m_GPAUtils.GetGPALoader().GPA_BeginPass();
            m_GPAUtils.StatusCheck(status);

            if (GPA_STATUS_OK == status)
            {
                status = m_GPAUtils.GetGPALoader().GPA_BeginSample(0);
                m_GPAUtils.StatusCheck(status);

                if (GPA_STATUS_OK == status)
                {
                    Real_ID3D11DeviceContext_Dispatch(GetID3D11DeviceContextVTableManager(), pObj, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
                    status = m_GPAUtils.GetGPALoader().GPA_EndSample();
                    m_GPAUtils.StatusCheck(status);
                }
                else
                {
                    Real_ID3D11DeviceContext_Dispatch(GetID3D11DeviceContextVTableManager(), pObj, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
                }

                status = m_GPAUtils.GetGPALoader().GPA_EndPass();
                m_GPAUtils.StatusCheck(status);
            }
            else
            {
                Real_ID3D11DeviceContext_Dispatch(GetID3D11DeviceContextVTableManager(), pObj, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
            }

            //Restore backup UAVs (copy UAVs in the acti ve UAV set which is always the original UAVs to backup UAV)
            //so that it appears to be that Dispatch has never been called
            if (!m_DCContextMgr.RestoreContext())
            {
                SpAssert(!"Failed to RestoreContext\n");
                break;
            }
        }

        m_GPAUtils.StatusCheck(m_GPAUtils.GetGPALoader().GPA_EndSession());

        // change active UAVs set to the real UAVs created by the client
        if (!m_DCContextMgr.PopContext())
        {
            SpAssert(!"Failed to PopContext\n");
            // We can't recover from this error.
            return false;
        }

        //run dispatch
        Real_ID3D11DeviceContext_Dispatch(GetID3D11DeviceContextVTableManager(), pObj, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
        return true;
    }
    else
    {
        Log(logERROR, "beginSession return != STATUS_OK, can't profile, it happens when no counter is enabled, call real dispatch\n");
        Real_ID3D11DeviceContext_Dispatch(GetID3D11DeviceContextVTableManager(), pObj, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
        return false;
    }
}

bool DCGPAProfiler::DumpSession(gpa_uint32 uSessionID, UINT threadGroupX, UINT threadGroupY, UINT threadGroupZ)
{
    if (!m_bDoneHeadings)
    {
        DumpHeader();
    }

    ID3DBlob* pBlob = NULL;
    string kernelName;
    UINT groupSizeX, groupSizeY, groupSizeZ;
    m_KernelAssembly.GetKernelInformation(m_DCContextMgr.GetCurrentComputeShader(), &pBlob, kernelName, groupSizeX, groupSizeY, groupSizeZ);

    if (!pBlob)
    {
        kernelName = "session";
    }

    vector<string> counterValues;
    UINT sampleCount;
    UINT numCounter;
    m_GPAUtils.GetCounterValues(uSessionID, counterValues, sampleCount, numCounter);

    std::stringstream strStream;
    std::string counterName;

    for (UINT sci = 0; sci < sampleCount; ++sci)
    {
        KernelProfileResultManager::Instance()->BeginKernelInfo();

        KernelProfileResultManager::Instance()->WriteKernelInfo("Identifier", kernelName.c_str());
        KernelProfileResultManager::Instance()->WriteKernelInfo("ExecutionOrder", m_uiSequenceNum);
        static const size_t valueStrSize = 64;
        strStream.str().reserve(valueStrSize);
        strStream << "{" << threadGroupX << " " << threadGroupY << " " << threadGroupZ << "}";
        KernelProfileResultManager::Instance()->WriteKernelInfo("ThreadGroup", strStream.str().c_str());

        if (pBlob)
        {
            strStream.str("");
            strStream << "{" << groupSizeX << " " << groupSizeY << " " << groupSizeZ << "}";
            KernelProfileResultManager::Instance()->WriteKernelInfo("WorkGroupSize", strStream.str().c_str());
        }
        else
        {
            KernelProfileResultManager::Instance()->WriteKernelInfo("WorkGroupSize", "NA");
        }

        //write counters
        for (UINT ci = 0; ci < numCounter; ++ci)
        {
            counterName = m_enabledCounters[ci];
            UnmapCounterName(CS_COUNTER_NAME_PREFIX, counterName);
            KernelProfileResultManager::Instance()->WriteKernelInfo(counterName, counterValues[(sci * numCounter) + ci].c_str());
        }

        KernelProfileResultManager::Instance()->EndKernelInfo();
    }

    m_uiSequenceNum++;

    return true;
}

void DCGPAProfiler::DumpMemoryStats(ID3D11Resource* pResource, float elapsedTime, D3D11_MAP MapType, UINT MapFlags)
{
    if (MapFlags == D3D11_MAP_FLAG_DO_NOT_WAIT)
    {
        return;//unable to measure the time, non-blocking
    }
    else
    {
        bool dumpMapData = false;
        string mapName;

        switch (MapType)
        {
            case D3D11_MAP_READ:
                mapName = "D3D11_MAP_READ";
                dumpMapData = true;
                break;

            case D3D11_MAP_WRITE:
                mapName = "D3D11_MAP_WRITE";
                dumpMapData = true;
                break;

            case D3D11_MAP_READ_WRITE:
                mapName = "D3D11_MAP_READ_WRITE";
                dumpMapData = true;
                break;

            case D3D11_MAP_WRITE_DISCARD: // Commonly used in updating constant buffer
            case D3D11_MAP_WRITE_NO_OVERWRITE: // Commonly used in updating constant buffer
                break;

            default:
                break;
        }

        if (dumpMapData)
        {
            if (!m_bDoneHeadings)
            {
                DumpHeader();
            }

            KernelProfileResultManager::Instance()->BeginKernelInfo();
            KernelProfileResultManager::Instance()->WriteKernelInfo("Identifier", mapName.c_str());
            KernelProfileResultManager::Instance()->WriteKernelInfo("ExecutionOrder", m_uiSequenceNum);

            string strMapDataSize;
            UINT mapSize = DCUtils::GetResourceSize(pResource);
            float mapSizeInKilobytes = (float)mapSize / 1024.0f;
            strMapDataSize = StringUtils::ToString(mapSizeInKilobytes);
            KernelProfileResultManager::Instance()->WriteKernelInfo("DataTransferSize", strMapDataSize.c_str());

            if (IsGpuTimeEnabled())
            {
                std::string strElapsedTime = StringUtils::ToString(elapsedTime);
                std::string counterName = GPU_TIME_COUNTER_NAME;
                UnmapCounterName(CS_COUNTER_NAME_PREFIX, counterName);
                KernelProfileResultManager::Instance()->WriteKernelInfo(counterName, strElapsedTime.c_str());
                KernelProfileResultManager::Instance()->EndKernelInfo();
                m_uiSequenceNum++;
            }
        }
    }
}

KernelAssembly& DCGPAProfiler::GetKernelAssemblyManager()
{
    return m_KernelAssembly;
}

void DCGPAProfiler::UnmapCounterName(const string& prefix, string& str)
{
    size_t pos = str.find(prefix);

    if (pos != string::npos)
    {
        str = str.substr(pos + prefix.length());
        return;
    }
    else if (str.compare("TexUnitBusy") == 0)
    {
        str = "FetchUnitBusy";
        return;
    }
    else if (str.compare("TexUnitStalled") == 0)
    {
        str = "FetchUnitStalled";
        return;
    }
    else if (str.compare("PSExportStalls") == 0)
    {
        str = "WriteUnitStalled";
        return;
    }
    else if (str.compare("GPUTime") == 0)
    {
        str = "Time";
        return;
    }
    else if (str.compare("TexMemBytesRead") == 0)
    {
        str = "FetchSize";
        return;
    }
}

void DCGPAProfiler::DumpHeader()
{
    // Header should only be dumped once
    if (m_bDoneHeadings)
    {
        return;
    }

    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("ProfileFileVersion=%d.%d", GPUPROFILER_BACKEND_MAJOR_VERSION, GPUPROFILER_BACKEND_MINOR_VERSION));
    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("ProfilerVersion=%d.%d.%d", GPUPROFILER_BACKEND_MAJOR_VERSION, GPUPROFILER_BACKEND_MINOR_VERSION, GPUPROFILER_BACKEND_BUILD_NUMBER));
    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("API=DirectCompute"));
    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("Application=%s", FileUtils::GetExeFullPath().c_str()));
    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("ApplicationArgs=%s", m_profilerParams.m_strCmdArgs.asUTF8CharArray()));
    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("WorkingDirectory=%s", m_profilerParams.m_strWorkingDir.asUTF8CharArray()));
    EnvVarMap envVarMap = GlobalSettings::GetInstance()->m_params.m_mapEnvVars;

    if (!envVarMap.empty())
    {
        KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("FullEnvironment=%d", GlobalSettings::GetInstance()->m_params.m_bFullEnvBlock));

        for (EnvVarMap::const_iterator it = envVarMap.begin(); it != envVarMap.end(); ++it)
        {
            KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("EnvVar=%s=%s", it->first.asUTF8CharArray(), it->second.asUTF8CharArray()));
        }
    }

    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("OS Version=%s", OSUtils::Instance()->GetOSInfo().c_str()));
    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("DisplayName=%s", GlobalSettings::GetInstance()->m_params.m_strSessionName.c_str()));
    KernelProfileResultManager::Instance()->AddHeader(StringUtils::FormatString("ListSeparator=%c", m_profilerParams.m_cOutputSeparator));

    SP_TODO("Currently, DumpHeader only works if counters are enabled in GPA, consider switching to offline mode like CL/HSA Profiler");
    CounterList selectedCounters;
    m_GPAUtils.GetEnabledCounterNames(selectedCounters);

    KernelProfileResultManager::Instance()->AddProfileResultItem("Identifier");
    KernelProfileResultManager::Instance()->AddProfileResultItem("ExecutionOrder");
    KernelProfileResultManager::Instance()->AddProfileResultItem("ThreadGroup");
    KernelProfileResultManager::Instance()->AddProfileResultItem("WorkGroupSize");

    // 2014/06/18
    // Comment out data transfer size column to make DirectCokmpute output compatible with OpenCL output.
    // Leaving commented out to make it easy to revert this change.
    //KernelProfileResultManager::Instance()->AddProfileResultItem("DataTransferSize");

    // lazy initialization of the enabled counter list
    std::string counterName;

    if (IsGpuTimeEnabled()) // Always add the GPU time unless it's disabled
    {
        m_enabledCounters.push_back(GPU_TIME_COUNTER_NAME);
        counterName = GPU_TIME_COUNTER_NAME;
        UnmapCounterName(CS_COUNTER_NAME_PREFIX, counterName);
        KernelProfileResultManager::Instance()->AddProfileResultItem(counterName);
    }

    for (CounterList::iterator counterIter = selectedCounters.begin(); counterIter != selectedCounters.end(); ++counterIter)
    {
        counterName = *counterIter;

        if (GPU_TIME_COUNTER_NAME != counterName) // time is handled seperately above
        {
            m_enabledCounters.push_back(counterName);
            UnmapCounterName(CS_COUNTER_NAME_PREFIX, counterName);
            KernelProfileResultManager::Instance()->AddProfileResultItem(counterName);
        }
    }

    m_GPAUtils.SetEnabledCounters(m_enabledCounters);
    m_bDoneHeadings = true;
}

bool DCGPAProfiler::Init(const Parameters& params, string& strErrorOut)
{
    // Set output file
    KernelProfileResultManager::Instance()->SetOutputFile(params.m_strOutputFile);

    // Set list separator
    m_profilerParams = params;
    KernelProfileResultManager::Instance()->SetListSeparator(m_profilerParams.m_cOutputSeparator);

    m_KernelAssembly.SetOutputASM(params.m_bOutputASM);

    m_uiMaxDispatchCount = params.m_uiMaxKernels;

    bool retVal = m_GPAUtils.InitGPA(GPA_API_DIRECTX_11, params.m_strDLLPath, strErrorOut);

    CounterList counterList;

    if (retVal)
    {
        if (!(params.m_strCounterFile.empty()))
        {
            retVal = FileUtils::ReadFile(params.m_strCounterFile.c_str(), counterList, true);
            MapCounterNames(counterList);

            if (retVal)
            {
                retVal = m_GPAUtils.SetEnabledCounters(counterList);
            }
        }
        else
        {
            m_deferLoadCounters = true;
        }
    }

    return retVal;
}

bool DCGPAProfiler::IsGpuTimeEnabled()
{
    return (
               (false == m_profilerParams.m_bForceSinglePassPMC) &&
               (true == m_profilerParams.m_bGPUTimePMC));
}

bool DCGPAProfiler::GetAllComputeCounters(CounterList& counterList)
{
    bool retVal = true;

    // Loading dxgi.dll has a side effect of initializing Real_CreateDXGIFactory1
    LIB_HANDLE hDxgiDll = OSUtils::Instance()->GenericLoadLibrary("dxgi.dll");

    if (NULL == hDxgiDll)
    {
        retVal = false;
    }
    else
    {
        if (NULL == Real_CreateDXGIFactory1)
        {
            retVal = false;
        }
        else
        {
            // Make sure this isn't called from DllMain (or a detoured LoadLibrary call)
            // See this note on MSDN https://msdn.microsoft.com/en-us/library/windows/desktop/bb205075%28v=vs.85%29.aspx#DXGI_Responses_From_DLLMain
            // DXGI Responses from DLLMain
            //    Because a DllMain function can't guarantee the order in which it loads and unloads DLLs, we recommend that your app's DllMain function not call
            //    Direct3D or DXGI functions or methods, including functions or methods that create or release objects.If your app's DllMain function calls into a
            //    particular component, that component might call another DLL that isn't present on the operating system, which causes the operating system to crash.
            //    Direct3D and DXGI might load a set of DLLs, typically a set of drivers, that differs from computer to computer. Therefore, even if your app doesn’t
            //    crash on your development and test computers when its DllMain function calls Direct3D or DXGI functions or methods, it might crash when it runs on
            //    another computer.
            //
            //    To prevent you from creating an app that might cause the operating system to crash, DXGI provides the following responses in the specified situations :
            //       If your app's DllMain function releases its last reference to a DXGI factory, DXGI raises an exception.
            //       If your app's DllMain function creates a DXGI factory, DXGI returns an error code.

            IDXGIFactory* pDxgiFactory = NULL;
            HRESULT hr = Real_CreateDXGIFactory1(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pDxgiFactory));

            if (S_OK != hr)
            {
                retVal = false;
            }
            else
            {
                typedef std::unordered_set<CounterList::value_type> CounterNames;
                CounterNames counterNames;
                CounterList adapterCounters;
                UINT adapterIndex = 0;

                while (retVal && (S_OK == hr))
                {
                    IDXGIAdapter* pDxgiAdapter = NULL;
                    hr = pDxgiFactory->EnumAdapters(adapterIndex, &pDxgiAdapter);

                    if (S_OK == hr)
                    {
                        DXGI_ADAPTER_DESC adapterDesc;
                        hr = pDxgiAdapter->GetDesc(&adapterDesc);

                        if (S_OK == hr)
                        {
                            GDT_HW_GENERATION hwGen;
                            bool hwGenFound = AMDTDeviceInfoUtils::Instance()->GetHardwareGeneration(adapterDesc.DeviceId, hwGen);

                            if (hwGenFound)
                            {
                                adapterCounters.clear();
                                m_GPAUtils.GetAvailableCountersGdt(hwGen, adapterCounters);
                                m_GPAUtils.FilterNonComputeCountersGdt(hwGen, adapterCounters);

                                for (CounterList::iterator aci = adapterCounters.begin() ; adapterCounters.end() != aci; ++aci)
                                {
                                    if (counterNames.end() == counterNames.find(*aci))
                                    {
                                        counterNames.insert(*aci);
                                    }
                                }
                            }
                        }

                        pDxgiAdapter->Release();
                    }

                    ++adapterIndex;
                }

                // DXGI_ERROR_NOT_FOUND marks that EnumAdapters reached the last adapter in the system
                if (DXGI_ERROR_NOT_FOUND != hr)
                {
                    retVal = false;
                }
                else
                {
                    for (CounterNames::iterator cni = counterNames.begin() ; counterNames.end() != cni ; ++cni)
                    {
                        counterList.push_back(*cni);
                    }
                }

                pDxgiFactory->Release();
            }
        }
    }

    return retVal;
}
