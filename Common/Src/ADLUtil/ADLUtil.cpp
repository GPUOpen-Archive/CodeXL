//==============================================================================
// Copyright (c) 2011-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Interface from Developer Tools to ADL.
//==============================================================================

// TODO: Use DynamicLibraryModule for the dll/so interactions.
// TODO: Consider moving ADLUtil into the DynamicLibraryModule directory.

#include <cstdlib>
#include <cstring>
#include <sstream>

#include "ADLUtil.h"
#ifdef _WIN32
    #include <windows.h>
#elif defined(_LINUX)
    #include <dlfcn.h>
#endif

// Callback so that ADL can allocate memory
void* __stdcall ADL_Main_Memory_Alloc(int iSize)
{
    void* lpBuffer = malloc(iSize);
    return lpBuffer;
}

// Optional ADL Memory de-allocation function
void __stdcall ADL_Main_Memory_Free(void** lpBuffer)
{
    if (nullptr != *lpBuffer)
    {
        free(*lpBuffer);
        *lpBuffer = nullptr;
    }
}

// converts a single hex character to its decimal equivalent
char xtod(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }

    if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }

    if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }

    return c = 0;      // not Hex digit
}

// converts a string of hex characters to a decimal equivalent,
// adding the decimal value to the supplied "initialValue"
int HextoDec(const char* hex, int initalValue)
{
    if (*hex == 0)
    {
        return (initalValue);
    }

    return HextoDec(hex + 1, initalValue * 16 + xtod(*hex));
}

// Converts a hex string to its decimal equivalent
int xtoi(const char* hex)
{
    return HextoDec(hex, 0);
}


//-----------------------------------------------------------------------------
ADLUtil_Result ADLUtil_GetASICInfo(AsicInfoList& asicInfoList)
{
    return AMDTADLUtils::Instance()->GetAsicInfoList(asicInfoList);
}


ADLUtil_Result
ADLUtil_GetVersionsInfo(
    struct ADLVersionsInfo& info)
{
    return AMDTADLUtils::Instance()->GetADLVersionsInfo(info);
}

AMDTADLUtils::AMDTADLUtils() :
    m_libHandle(nullptr),
    m_adlContext(nullptr),
    m_asicInfoListRetVal(ADL_RESULT_NONE),
    m_versionRetVal(ADL_RESULT_NONE)
{
#define X(SYM) m_##SYM = nullptr;
    ADL_INTERFACE_TABLE
#undef X
}

AMDTADLUtils::~AMDTADLUtils()
{
    Unload();
}

ADLUtil_Result AMDTADLUtils::LoadAndInit()
{
    ADLUtil_Result result = ADL_SUCCESS;

    if (nullptr == m_libHandle)
    {
#ifdef _WIN32
        m_libHandle = LoadLibraryA("atiadlxx.dll");

        if (nullptr == m_libHandle)
        {
            // A 32 bit calling application on 64 bit OS will fail to LoadLIbrary.
            // Try to load the 32 bit library (atiadlxy.dll) instead
            m_libHandle = LoadLibraryA("atiadlxy.dll");
        }

#else
        m_libHandle = dlopen("libatiadlxx.so", RTLD_LAZY | RTLD_GLOBAL);
#endif

        if (nullptr == m_libHandle)
        {
            result = ADL_NOT_FOUND;
        }

#ifdef _WIN32
#define LOCAL_GET_PROC_ADDRESS ::GetProcAddress
#else
#define LOCAL_GET_PROC_ADDRESS dlsym
#endif
#define X(SYM) m_##SYM = (SYM##_fn)LOCAL_GET_PROC_ADDRESS(m_libHandle, #SYM); \
    if (nullptr == m_##SYM) \
    { \
        Unload(); \
        result = ADL_MISSING_ENTRYPOINTS; \
    }
        ADL_INTERFACE_TABLE;
#undef X
#undef LOCAL_GET_PROC_ADDRESS

        if (ADL_SUCCESS == result)
        {
            int adlResult;

            // Initialize ADL. The second parameter is 1, which means:
            // retrieve adapter information only for adapters that are physically present and enabled in the system
            if (nullptr != m_ADL2_Main_Control_Create)
            {
                adlResult = m_ADL2_Main_Control_Create(ADL_Main_Memory_Alloc, 1, &m_adlContext);
            }
            else
            {
                adlResult = m_ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1);
            }

            if (ADL_OK != adlResult && ADL_OK_WARNING != adlResult)
            {
                Unload();
                return ADL_INITIALIZATION_FAILED;
            }
        }
    }

    return result;
}

ADLUtil_Result AMDTADLUtils::Unload()
{
    ADLUtil_Result result = ADL_SUCCESS;

    if (nullptr != m_libHandle)
    {
        if (nullptr != m_ADL2_Main_Control_Destroy)
        {
            if (nullptr != m_adlContext)
            {
                m_ADL2_Main_Control_Destroy(m_adlContext);
                m_adlContext = nullptr;
            }
        }
        else if (nullptr != m_ADL_Main_Control_Destroy)
        {
            m_ADL_Main_Control_Destroy();
        }

#ifdef _WIN32
        FreeLibrary(m_libHandle);
#else
        dlclose(m_libHandle);
#endif
        m_libHandle = nullptr;

#define X(SYM) m_##SYM = nullptr;
        ADL_INTERFACE_TABLE
#undef X
    }

    Reset();

    /// attempt to restore the default power mode, in case an app calls UseHighPowerMode() without a corresponding ResumeNormalPowerMode() call
    if (0 != m_highPowerGpuSpeedSet.size())
    {
        ResumeNormalPowerMode();
    }

    return result;
}


ADLUtil_Result AMDTADLUtils::GetAsicInfoList(AsicInfoList& asicInfoList)
{
    AMDTScopeLock lock(m_asicInfoMutex);

    if (ADL_RESULT_NONE == m_asicInfoListRetVal)
    {
        m_asicInfoListRetVal = LoadAndInit();

        if (ADL_SUCCESS == m_asicInfoListRetVal)
        {
            int adlResult = ADL_OK;

            int numAdapter = 0;

            // Obtain the number of adapters for the system
            if (nullptr != m_ADL2_Adapter_NumberOfAdapters_Get)
            {
                adlResult = m_ADL2_Adapter_NumberOfAdapters_Get(m_adlContext, &numAdapter);
            }
            else
            {
                adlResult = m_ADL_Adapter_NumberOfAdapters_Get(&numAdapter);
            }

            if (ADL_OK != adlResult)
            {
                m_asicInfoListRetVal = ADL_GET_ADAPTER_COUNT_FAILED;
            }
            else
            {
                if (0 < numAdapter)
                {
                    LPAdapterInfo lpAdapterInfo = (LPAdapterInfo)malloc(sizeof(AdapterInfo) * numAdapter);
                    memset(lpAdapterInfo, '\0', sizeof(AdapterInfo) * numAdapter);

                    // Get the AdapterInfo structure for all adapters in the system
                    if (nullptr != m_ADL2_Adapter_AdapterInfo_Get)
                    {
                        adlResult = m_ADL2_Adapter_AdapterInfo_Get(m_adlContext, lpAdapterInfo, sizeof(AdapterInfo) * numAdapter);
                    }
                    else
                    {
                        adlResult = m_ADL_Adapter_AdapterInfo_Get(lpAdapterInfo, sizeof(AdapterInfo) * numAdapter);
                    }

                    if (ADL_OK == adlResult)
                    {
                        for (int i = 0; i < numAdapter; ++i)
                        {
                            std::string adapterName = lpAdapterInfo[i].strAdapterName;
                            std::string adapterInfo = lpAdapterInfo[i].strUDID;

                            // trim trailing whitespace
                            size_t lastNonSpace = adapterName.length() - 1;

                            while (adapterName[lastNonSpace] == ' ')
                            {
                                --lastNonSpace;
                            }

                            ADLUtil_ASICInfo asicInfo;
                            asicInfo.adapterName = adapterName.substr(0, lastNonSpace + 1);
                            // TODO: find a way to get the correct gpu index in the system
                            asicInfo.gpuIndex = 0;
#ifdef _WIN32
                            size_t vendorIndex = adapterInfo.find("PCI_VEN_") + strlen("PCI_VEN_");
                            size_t devIndex = adapterInfo.find("&DEV_") + strlen("&DEV_");
                            size_t revIndex = adapterInfo.find("&REV_") + strlen("&REV_");

                            if (vendorIndex != std::string::npos)
                            {
                                std::string vendorIDString = adapterInfo.substr(vendorIndex, 4);
                                asicInfo.vendorID = xtoi(vendorIDString.c_str());
                            }
                            else
                            {
                                asicInfo.vendorID = 0;
                            }

                            if (devIndex != std::string::npos)
                            {
                                asicInfo.deviceIDString = adapterInfo.substr(devIndex, 4);
                                asicInfo.deviceID = xtoi(asicInfo.deviceIDString.c_str());
                            }
                            else
                            {
                                asicInfo.deviceIDString.clear();
                                asicInfo.deviceID = 0;
                            }

                            if (revIndex != std::string::npos)
                            {
                                std::string revIDString = adapterInfo.substr(revIndex, 2);
                                asicInfo.revID = xtoi(revIDString.c_str());
                            }
                            else
                            {
                                asicInfo.revID = 0;
                            }

                            asicInfo.registryPath = lpAdapterInfo[i].strDriverPath;
                            asicInfo.registryPathExt = lpAdapterInfo[i].strDriverPathExt;
#elif defined(_LINUX)
                            asicInfo.vendorID = lpAdapterInfo[i].iVendorID;
                            size_t devIndex = adapterInfo.find(":") + 1;

                            if (devIndex != std::string::npos)
                            {
                                asicInfo.deviceIDString = adapterInfo.substr(devIndex, std::string::npos);
                                size_t colonPos = asicInfo.deviceIDString.find(":");

                                if (colonPos != std::string::npos)
                                {
                                    asicInfo.deviceIDString = asicInfo.deviceIDString.substr(0, colonPos);
                                    asicInfo.deviceID = atoi(asicInfo.deviceIDString.c_str());
                                }
                                else
                                {
                                    asicInfo.deviceID = 0;
                                }
                            }
                            else
                            {
                                asicInfo.deviceID = 0;
                            }

                            // TODO: see if we can get revision id on Linux
                            asicInfo.revID = 0;
#endif

                            m_asicInfoList.push_back(asicInfo);
                        }

                        ADL_Main_Memory_Free((void**)&lpAdapterInfo);
                    }
                    else
                    {
                        m_asicInfoListRetVal = ADL_GET_ADAPTER_INFO_FAILED;
                    }
                }
            }
        }
    }

    asicInfoList = m_asicInfoList;
    return m_asicInfoListRetVal;
}

ADLUtil_Result AMDTADLUtils::GetADLVersionsInfo(ADLVersionsInfo& adlVersionInfo)
{
    AMDTScopeLock lock(m_adlVersionsMutex);

    if (ADL_RESULT_NONE == m_versionRetVal)
    {
        m_versionRetVal = LoadAndInit();

        if (ADL_SUCCESS == m_versionRetVal)
        {
            int adlResult = ADL_OK;

            // Obtain the number of adapters for the system
            if (nullptr != m_ADL2_Graphics_Versions_Get)
            {
                adlResult = m_ADL2_Graphics_Versions_Get(m_adlContext, &m_adlVersionsInfo);
            }
            else
            {
                adlResult = m_ADL_Graphics_Versions_Get(&m_adlVersionsInfo);
            }

            if (ADL_OK != adlResult)
            {
                if (ADL_OK_WARNING == adlResult)
                {
                    m_versionRetVal = ADL_WARNING;
                }
                else // ADL_OK_WARNING != adlResult
                {
                    m_versionRetVal = ADL_GRAPHICS_VERSIONS_GET_FAILED;
                }
            }
        }
    }

    adlVersionInfo = m_adlVersionsInfo;
    return m_versionRetVal;
}

ADLUtil_Result AMDTADLUtils::GetDriverVersion(unsigned int& majorVer, unsigned int& minorVer, unsigned int& subMinorVer)
{
    majorVer = 0;
    minorVer = 0;
    subMinorVer = 0;

    ADLVersionsInfo driverVerInfo;
    ADLUtil_Result adlResult = AMDTADLUtils::Instance()->GetADLVersionsInfo(driverVerInfo);

    if (adlResult == ADL_SUCCESS || adlResult == ADL_WARNING)
    {
        std::string strDriverVersion(driverVerInfo.strDriverVer);

        // driver version looks like:  13.35.1005-140131a-167669E-ATI or 14.10-140115n-021649E-ATI, etc...
        // truncate at the first dash
        strDriverVersion = strDriverVersion.substr(0, strDriverVersion.find("-", 0));

        size_t pos = 0;
        std::string strToken;
        std::string strDelimiter = ".";
        std::stringstream ss;

        // parse the major driver version
        pos = strDriverVersion.find(strDelimiter);

        if (pos != std::string::npos)
        {
            strToken = strDriverVersion.substr(0, pos);
            ss.str(strToken);

            if ((ss >> majorVer).fail())
            {
                majorVer = 0;
            }
            else
            {
                adlResult = ADL_SUCCESS;
                strDriverVersion.erase(0, pos + strDelimiter.length());
            }

            // parse the minor driver version
            bool subMinorAvailable = false;

            pos = strDriverVersion.find(strDelimiter);

            if (pos != std::string::npos)
            {
                strToken = strDriverVersion.substr(0, pos);
                strDriverVersion.erase(0, pos + strDelimiter.length());
                subMinorAvailable = true;
            }
            else
            {
                strToken = strDriverVersion;
            }

            ss.clear();
            ss.str(strToken);

            if ((ss >> minorVer).fail())
            {
                minorVer = 0;
            }

            // parse the sub-minor driver version
            if (subMinorAvailable)
            {
                pos = strDriverVersion.find(strDelimiter);

                if (pos != std::string::npos)
                {
                    strToken = strDriverVersion.substr(0, pos);
                    strDriverVersion.erase(0, pos + strDelimiter.length());
                }
                else
                {
                    strToken = strDriverVersion;
                }

                ss.clear();
                ss.str(strToken);

                if ((ss >> subMinorVer).fail())
                {
                    subMinorVer = 0;
                }
            }
        }
    }

    return adlResult;
}

ADLUtil_Result AMDTADLUtils::UseHighPowerMode(unsigned int gpuIndex)
{
    return ForceGPUClock(true, gpuIndex);
}

ADLUtil_Result AMDTADLUtils::ResumeNormalPowerMode(unsigned int gpuIndex)
{
    return ForceGPUClock(false, gpuIndex);
}

ADLUtil_Result AMDTADLUtils::ForceGPUClock(bool forceHigh, unsigned int gpuIndex)
{
    AsicInfoList asicInfoList;
    ADLUtil_Result result = GetAsicInfoList(asicInfoList);
    static const int AMD_VENDOR_ID = 0x1002;

    if (ADL_SUCCESS == result)
    {
        AMDTScopeLock lock(m_powerTableMutex);

        int adlResult = ADL_OK;

        for (AsicInfoList::iterator it = asicInfoList.begin(); it != asicInfoList.end(); ++it)
        {
            unsigned int thisGpuIndex = it->gpuIndex;
            const int invalidIndex    = -1;

            if ((static_cast<unsigned int>(invalidIndex) != gpuIndex && gpuIndex != thisGpuIndex) || AMD_VENDOR_ID != it->vendorID)
            {
                continue;
            }

            if (forceHigh)
            {
                if (m_highPowerGpuSpeedSet.find(thisGpuIndex) != m_highPowerGpuSpeedSet.end())
                {
                    continue; // this adapter has already been set to run in high power mode
                }

                ADLODParameters odParameters;

                if (nullptr != m_ADL2_Overdrive5_ODParameters_Get)
                {
                    adlResult = m_ADL2_Overdrive5_ODParameters_Get(m_adlContext, thisGpuIndex, &odParameters);
                }
                else
                {
                    adlResult = m_ADL_Overdrive5_ODParameters_Get(thisGpuIndex, &odParameters);
                }

                if (ADL_OK > adlResult)
                {
                    // adl Error, skip this adapter
                    continue;
                }

                int perfLevelSize = sizeof(ADLODPerformanceLevels) + sizeof(ADLODPerformanceLevel) * (odParameters.iNumberOfPerformanceLevels - 1);

                if (0 < perfLevelSize)
                {
                    ADLODPerformanceLevels* odPerformanceLevels = (ADLODPerformanceLevels*)malloc(perfLevelSize);

                    odPerformanceLevels->iSize = perfLevelSize;

                    if (nullptr != m_ADL2_Overdrive5_ODPerformanceLevels_Get)
                    {
                        adlResult = m_ADL2_Overdrive5_ODPerformanceLevels_Get(m_adlContext, thisGpuIndex, 0, odPerformanceLevels);
                    }
                    else
                    {
                        adlResult = m_ADL_Overdrive5_ODPerformanceLevels_Get(thisGpuIndex, 0, odPerformanceLevels);
                    }

                    if (ADL_OK <= adlResult)
                    {
                        ADLODPerformanceLevels* origODPerformanceLevels = (ADLODPerformanceLevels*)malloc(perfLevelSize);
                        memcpy(origODPerformanceLevels, odPerformanceLevels, odPerformanceLevels->iSize);
                        m_origODPerformanceLevels[thisGpuIndex] = origODPerformanceLevels;

                        int maxEngineClock = 0;
                        int maxMemoryClock = 0;

                        for (int i = 0; i < odParameters.iNumberOfPerformanceLevels; i++)
                        {
                            if (odPerformanceLevels->aLevels[i].iEngineClock > maxEngineClock)
                            {
                                maxEngineClock = odPerformanceLevels->aLevels[i].iEngineClock;
                                maxMemoryClock = odPerformanceLevels->aLevels[i].iMemoryClock;
                            }
                        }

                        for (int i = 0; i < odParameters.iNumberOfPerformanceLevels; i++)
                        {
                            odPerformanceLevels->aLevels[i].iEngineClock = maxEngineClock;
                            odPerformanceLevels->aLevels[i].iMemoryClock = maxMemoryClock;
                        }

                        if (nullptr != m_ADL2_Overdrive5_ODPerformanceLevels_Set)
                        {
                            adlResult = m_ADL2_Overdrive5_ODPerformanceLevels_Set(m_adlContext, thisGpuIndex, odPerformanceLevels);
                        }
                        else
                        {
                            adlResult = m_ADL_Overdrive5_ODPerformanceLevels_Set(thisGpuIndex, odPerformanceLevels);
                        }

                        if (ADL_OK <= adlResult)
                        {
                            m_highPowerGpuSpeedSet.insert(thisGpuIndex);
                        }
                    }

                    free(odPerformanceLevels);
                }
            }
            else // reset to default power mode
            {
                if (m_highPowerGpuSpeedSet.find(thisGpuIndex) == m_highPowerGpuSpeedSet.end())
                {
                    continue; // this adapter already been set to run in default power mode
                }

                m_highPowerGpuSpeedSet.erase(thisGpuIndex);

                if (nullptr != m_ADL2_Overdrive5_ODPerformanceLevels_Set)
                {
                    adlResult = m_ADL2_Overdrive5_ODPerformanceLevels_Set(m_adlContext, thisGpuIndex, m_origODPerformanceLevels[thisGpuIndex]);
                }
                else
                {
                    adlResult = m_ADL_Overdrive5_ODPerformanceLevels_Set(thisGpuIndex, m_origODPerformanceLevels[thisGpuIndex]);
                }

                free(m_origODPerformanceLevels[thisGpuIndex]);
                m_origODPerformanceLevels.erase(thisGpuIndex);
            }
        }

        if (ADL_OK > adlResult)
        {
            result = ADL_CHANGING_POWER_MODE_FAILED;
        }
    }

    return result;
}

void AMDTADLUtils::Reset()
{
    m_asicInfoList.clear();
    m_asicInfoListRetVal = ADL_RESULT_NONE;
    m_versionRetVal = ADL_RESULT_NONE;
}
