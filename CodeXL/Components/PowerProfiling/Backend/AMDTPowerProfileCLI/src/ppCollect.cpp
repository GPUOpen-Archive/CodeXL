//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppCollect.cpp
///
//==================================================================================

// Project
#include <ppCollect.h>
#include <ppCliUtils.h>

#include <iostream>
#include <algorithm>
// TODO: this not required only linux pcore stuff is moved to API layer
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <sstream>
    #include <fstream>
#endif

#define MAX_PROFILE_COUNTER (100)
//
//  Public member functions
//

AMDTResult ppCollect::Initialize()
{
    // Validate the profile options
    ValidateProfileOptions();

    // Enable Profiling and get the list of supported counters
    EnableProfiling();

    return m_error;
} // Initialize


AMDTResult ppCollect::Configure()
{
    // Validate Counters
    ValidateCounters();

    // Configure Profile
    ConfigureProfile();

    // FIXME - doing it here just for reporting...
    m_profStartTime = GetTimeStr();

    return m_error;
} // Configure

AMDTResult ppCollect::EnableProcessProfiling()
{
    if (AMDT_STATUS_OK != (m_error = AMDTEnableProcessProfiling()))
    {
        ReportError(true, "WARNING:Error in configuring process profiling. (error code 0x%lx).\n", m_error);

        // Cosider this as warning
        if (AMDT_WARN_PROCESS_PROFILE_NOT_SUPPORTED == m_error)
        {
            m_error = AMDT_STATUS_OK;
        }
    }

    return m_error;
}

AMDTResult ppCollect::StartProfiling()
{
    if (isOK() && isProfileStateReady())
    {
        // FIXME.. though this is the correct place, setting the start time in Configure
        // m_profStartTime = GetTimeStr();

        m_error = AMDTPwrStartProfiling();

        if (AMDT_ERROR_ACCESSDENIED == m_error)
        {
            ReportError(false, "A power profiling session is already in progress. Please make sure that no other\n"
                        "instance of CodeXL graphic or command line tool is performing a power profiling\n"
                        "session and then start command line power profiling again.\n");
        }
        else if (AMDT_STATUS_OK != m_error)
        {
            ReportError(false, "The driver failed to start profiling. (error code 0x%lx).\n", m_error);
        }
    }

    return m_error;
} // StartProfiling


AMDTResult ppCollect::StopProfiling()
{
    if (isOK() && isProfileStateRunning())
    {
        m_profEndTime = GetTimeStr();

        // stop profile
        if (AMDT_STATUS_OK != (m_error = AMDTPwrStopProfiling()))
        {
            ReportError(true, "The driver failed to stop profiling. (error code 0x%lx).\n", m_error);
        }
    }

    return m_error;
} // StopProfiling


AMDTResult ppCollect::ResumeProfiling()
{
    if (isOK() && isProfileStatePaused())
    {
        if (AMDT_STATUS_OK != (m_error = AMDTPwrResumeProfiling()))
        {
            ReportError(false, "The driver failed to resume profiling. (error code 0x%lx).\n", m_error);
        }
    }

    return m_error;
} // ResumeProfiling


AMDTResult ppCollect::PauseProfiling()
{
    if (isOK() && isProfileStateRunning())
    {
        // pause profile
        if (AMDT_STATUS_OK != (m_error = AMDTPwrPauseProfiling()))
        {
            ReportError(true, "The driver failed to pause profiling. (error code 0x%lx).\n", m_error);
        }
    }

    return m_error;
} // PauseProfiling


AMDTResult ppCollect::GetSampleData(AMDTUInt32* pNbrSamples, AMDTPwrSample** ppSampleData)
{
    AMDTResult ret = AMDT_ERROR_FAIL;

    if (isOK() && isProfileStateRunning())
    {
        m_nbrSamples = 0;
        m_pSampleData = NULL;

        ret = AMDTPwrReadAllEnabledCounters(&m_nbrSamples, &m_pSampleData);

        if ((AMDT_STATUS_OK == m_error) && (m_nbrSamples > 0) && (NULL != m_pSampleData))
        {
            m_totalNbrSamples += m_nbrSamples;
            m_totalElapsedTime = m_pSampleData[m_nbrSamples - 1].m_elapsedTimeMs;

            if (NULL != pNbrSamples)
            {
                *pNbrSamples = m_nbrSamples;
            }

            if (NULL != ppSampleData)
            {
                *ppSampleData = m_pSampleData;
            }
        }
        else
        {
            m_NbrOfFailedReads++;
        }
    }

    return ret;
} // GetSampleData


AMDTResult ppCollect::GetHistogramCounters(AMDTUInt32* pNumOfCounters, AMDTPwrHistogram* pHist)
{
    AMDTPwrHistogram* pHistData = NULL;
    AMDTUInt32 cnt = 0;
    AMDTUInt32 i = 0;
    AMDTResult ret = AMDT_STATUS_OK;

    if (NULL != pHist)
    {
        if (isOK() && !m_histogramCounterIdVec.empty())
        {
            for (; i < m_histogramCounterIdVec.size(); i++)
            {
                AMDTUInt32 cntr = m_histogramCounterIdVec[i];
                ret = AMDTPwrReadCounterHistogram(cntr, &cnt, &pHistData);

                if (AMDT_STATUS_OK == ret)
                {
                    memcpy(&pHist[i], pHistData, sizeof(AMDTPwrHistogram));

                    pHist[i].m_pBins = (AMDTFloat32*) malloc(pHistData->m_numOfBins * sizeof(AMDTFloat32));

                    if (nullptr != pHist[i].m_pBins)
                    {
                        memcpy(pHist[i].m_pBins, pHistData->m_pBins, pHistData->m_numOfBins * sizeof(AMDTFloat32));
                    }

                    pHist[i].m_pRange = (AMDTFloat32*) malloc((pHistData->m_numOfBins + 1) * sizeof(AMDTFloat32));

                    if (nullptr != pHist[i].m_pRange)
                    {
                        memcpy(pHist[i].m_pRange, pHistData->m_pRange, (pHistData->m_numOfBins + 1) * sizeof(AMDTFloat32));
                    }
                }
                else
                {
                    ReportError(false, "Error while reading histogram data for counter %u . (error code 0x%lx).\n", cntr, ret);
                }
            }
        }
    }

    *pNumOfCounters = i;
    return ret;
}

AMDTResult ppCollect::GetCumulativeCounters(AMDTUInt32* pNumOfCounters, AMDTFloat32* pHist, AMDTUInt32* pCounterId)
{
    AMDTFloat32* pHistData = NULL;
    AMDTUInt32 cnt = 0;
    AMDTUInt32 i = 0;
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTUInt32 nbrOfCountersRead = 0;

    if (NULL != pHist)
    {
        if (isOK() && !m_cumulativeCounterIdVec.empty())
        {
            for (; i < m_cumulativeCounterIdVec.size(); i++)
            {
                AMDTUInt32 cntr = m_cumulativeCounterIdVec[i];
                ret = AMDTPwrReadCumulativeCounter(cntr, &cnt, &pHistData);
                pCounterId[i] = cntr;

                if (AMDT_STATUS_OK == ret)
                {
                    memcpy(&pHist[nbrOfCountersRead] , pHistData, sizeof(AMDTFloat32));
                    nbrOfCountersRead++;
                }
                else
                {
                    ReportError(true, "Error while reading histogram data for counter %u . (error code 0x%lx).\n", cntr, ret);
                }
            }
        }
    }

    *pNumOfCounters = nbrOfCountersRead;
    return ret;
}

//
//  Private member functions
//

void ppCollect::ValidateProfileOptions()
{
    m_error = AMDT_ERROR_FAIL;
    osCpuid cpuInfo;

    // If APIC is not available, power profiling cannot be supported
    if (!cpuInfo.hasLocalApic())
    {
        ReportError(false, "Local APIC support is not available, hence power profiling cannot be done.\n");
        return;
    }

    // Validate the core-affinity-mask of the launched process
    // TODO: Currently this supports only upto 64 cores
    if (IsValidCoreMask(m_args.GetCoreAffinityMask()))
    {
        m_args.SetCoreAffinityMask(m_args.GetCoreAffinityMask() & m_coreMask);
    }
    else
    {
        return;
    }

    // Validate the core-mask for 'core' counters specified with -P
    // TODO: Currently this supports only upto 64 cores
    if (IsValidCoreMask(m_args.GetCoreMask()))
    {
        m_args.SetCoreMask(m_args.GetCoreMask() & m_coreMask);
    }
    else
    {
        return;
    }

    // validate the output file path
    osFilePath filePath;

    if (!m_args.GetReportFilePath(filePath))
    {
        return;
    }

    // Check for valid working dir
    if (!m_args.GetWorkingDir().isEmpty())
    {
        osFilePath workDir(m_args.GetWorkingDir());

        if (!workDir.exists())
        {
            ReportError(false, "Working directory (%s) does not exist.\n", m_args.GetWorkingDir().asASCIICharArray());
            return;
        }
    }

    // Terminate app -b ?
    if (m_args.IsTerminateApp())
    {
        // terminate (-b option) app only for launched app case
        if (m_args.GetLaunchApp().isEmpty())
        {
            ReportError(false, "Option -b can be used only if Launch application is specified.\n");
            return;
        }

        // -b without -d is invalid
        if (0 == m_args.GetProfileDuration())
        {
            ReportError(false, "Option to terminate launch application (-b) is invalid without Profile duration option (-d).\n");
            return;
        }
    }

    if (m_args.IsProfileCounters())
    {
        if ((m_args.GetProfileDuration() <= 0) && (m_args.GetLaunchApp().isEmpty()))
        {
            ReportError(false, "Profile duration(-d) is missing.\n");
            return;
        }

        if (m_args.GetProfileDuration()
            && (m_args.GetSamplingInterval() >= (m_args.GetProfileDuration() * 1000)))
        {
            ReportError(false, "Sampling interval(%d) should not be greater than profile duration(%d).\n",
                        m_args.GetSamplingInterval(), m_args.GetProfileDuration());
            return;
        }
    }

    m_isDbExportEnabled = !m_args.GetDbFileOutDir().isEmpty();

    m_error = S_OK;
    return;
} // ValidateProfileOptions


AMDTResult ppCollect::EnableProfiling()
{
    AMDTUInt32 minSamplingRate = 0;

    if (isOK() && isProfileStateUninitialized())
    {
        // FIXME: This should be in API (Linux ONLY)
#if 0
        if (AMDT_STATUS_OK != (m_error = LoadPcoreModule()))
        {
            ReportError(true, "Failed to load the driver. (error code 0x%lx).\n", m_error);
        }

#endif

        if (isOK() && (AMDT_STATUS_OK != (m_error = AMDTPwrProfileInitialize(m_args.GetProfileMode()))))
        {
            if (AMDT_WARN_IGPU_DISABLED == m_error)
            {
                ReportError(true, "WARNING: IGPU disabled. SMU functionalities will not be available (error code 0x%lx).\n", m_error);

                // Profile should still work for non smu related counters
                m_error = AMDT_STATUS_OK;
            }
            else if (AMDT_DRIVER_VERSION_MISMATCH == m_error)
            {
                ReportError(true, "Driver version mismatch. (error code 0x%lx).\n", m_error);
            }
            else if (AMDT_ERROR_PLATFORM_NOT_SUPPORTED == m_error)
            {
                ReportError(true, "Power Profiler is not supported on the current platform (error code 0x%lx).\n", m_error);
            }
            else if (AMDT_WARN_SMU_DISABLED == m_error)
            {
                ReportError(true, "WARNING: One of the SMU is not accessible (error code 0x%lx).\n", m_error);
                // This is a warning. One of the SMU counters are not available
                // But rest of the counter shouls be available.
                m_error = AMDT_STATUS_OK;
            }
            else if (AMDT_ERROR_DRIVER_UNAVAILABLE == m_error)
            {
                ReportError(true, "ERROR: Driver not available (error code 0x%lx).\n", m_error);
            }
            else
            {
                ReportError(true, "Failed to initialize the driver. (error code 0x%lx).\n", m_error);
            }
        }

        AMDTPwrGetMinimalTimerSamplingPeriod(&minSamplingRate);

        if (m_args.GetSamplingInterval() < (int)minSamplingRate)
        {
            ReportError(false, "Sampling interval(%d) is lesser than minimum sampling interval(%d).\n",
                        m_args.GetSamplingInterval(), minSamplingRate);
            m_error = AMDT_ERROR_FAIL;
        }

        AMDTPwrSetSampleValueOption(AMDT_PWR_SAMPLE_VALUE_LIST);
    }

    if (isOK() && isProfileStateReady())
    {
        if (AMDT_STATUS_OK != (m_error = AMDTPwrGetSystemTopology(&m_pDeviceTopology)))
        {
            ReportError(true, "Failed to retrieve system topology. (error code 0x%lx).\n", m_error);
        }

        // TODO: Internalize the device details

        //AMDTPwrDeviceNameDeescMap m_deviceNameDescMap;   // Device Name - Device Desc Map
        //AMDTPwrDeviceIdDescVec    m_deviceIdDescVec;     // Device ID-Desc vector
        //AMDTPwrDeviceIdNameVec    m_deviceIdNameVec;     // Device ID-Name vector

        AMDTPwrDevice* pDeviceNode = m_pDeviceTopology;

        if (NULL != pDeviceNode)
        {
            m_deviceIdDescVec.resize(64); // FIXME - max 64 devices
            m_deviceIdNameVec.resize(64); // FIXME - max 64 devices

            gtList<AMDTPwrDevice*> deviceList;
            deviceList.push_back(pDeviceNode);

            while (deviceList.size() != 0)
            {
                pDeviceNode = deviceList.front();
                deviceList.pop_front();

                m_deviceIdDescVec[pDeviceNode->m_deviceID] = pDeviceNode;

                // Add this device to core mask
                if (AMDT_PWR_DEVICE_CPU_CORE == pDeviceNode->m_type)
                {
                    m_coreMask <<= 1;
                    m_coreMask |= 1;
                }

                // convert the name into lower-case and replace spaces with -
                wchar_t deviceNameW[OS_MAX_PATH];
                memset(deviceNameW, 0, sizeof(deviceNameW));

                if (NULL != pDeviceNode->m_pName)
                {
                    mbstowcs(deviceNameW, pDeviceNode->m_pName, OS_MAX_PATH - 1);
                }

                gtString deviceName(deviceNameW);
                deviceName.replace(L" ", L"-");

                // Name-Desc map
                m_deviceNameDescMap.insert(AMDTPwrDeviceNameDescMap::value_type(deviceName.toLowerCase(),
                                                                                pDeviceNode));

                // ID-Name Vec
                m_deviceIdNameVec[pDeviceNode->m_deviceID] = deviceName.toLowerCase();

                if (NULL != pDeviceNode->m_pFirstChild)
                {
                    deviceList.push_back(pDeviceNode->m_pFirstChild);
                }

                // For DB
                if (m_isDbExportEnabled)
                {
                    m_dbDeviceVec.push_back(ppCliUtils::ConvertToProfileDevice(*pDeviceNode));
                }

                if (NULL != pDeviceNode->m_pNextDevice)
                {
                    deviceList.push_back(pDeviceNode->m_pNextDevice);
                }
            }
        }

        m_nbrSupportedCounters = 0;

        if (isOK() &&
            (AMDT_STATUS_OK != (m_error = AMDTPwrGetDeviceCounters(AMDT_PWR_ALL_DEVICES, &m_nbrSupportedCounters, &m_pSupportedCountersDesc))))
        {
            ReportError(true, "Failed to retrieve counter information. (error code 0x%lx).\n", m_error);
        }

        // FIXME - can i assume the max counterID <= m_nbrSupportedCounters
        if (m_nbrSupportedCounters > 0)
        {
            m_supportedCounterIdDescVec.resize(MAX_PROFILE_COUNTER);
            m_supportedCounterIdNameVec.resize(MAX_PROFILE_COUNTER);
        }

        for (AMDTUInt32 i = 0; i < m_nbrSupportedCounters; i++)
        {
            // ID-Desc vector (counter-id is used as index)
            m_supportedCounterIdDescVec[m_pSupportedCountersDesc[i].m_counterID] = &m_pSupportedCountersDesc[i];

            // counter name used in reporting
            wchar_t counterNameW[OS_MAX_PATH];
            memset(counterNameW, 0, sizeof(counterNameW));

            if (NULL != m_pSupportedCountersDesc[i].m_name)
            {
                mbstowcs(counterNameW, m_pSupportedCountersDesc[i].m_name, OS_MAX_PATH - 1);
            }

            gtString counterName(counterNameW);
            counterName.replace(L" - ", L"-");
            counterName.replace(L" ", L"-");

            // Name-Desc map
            m_supportedCounterNameDescMap.insert(AMDTPwrCounterNameDescMap::value_type(counterName.toLowerCase(),
                                                 &m_pSupportedCountersDesc[i]));

            // ID-Name Vec
            m_supportedCounterIdNameVec[m_pSupportedCountersDesc[i].m_counterID] = counterName.toLowerCase();

            // Mullins does not support Temperature related countes
            if (AMDT_PWR_CATEGORY_TEMPERATURE == m_pSupportedCountersDesc[i].m_category)
            {
                m_isTemperatureSupported = true;
            }

            // For DB
            if (m_isDbExportEnabled)
            {
                m_dbCounterDescVec.push_back(ppCliUtils::ConvertToProfileCounterDesc(m_pSupportedCountersDesc[i]));
            }
        }
    }

    // FIXME
    // m_supportedCounterIdDescVec.shrink_to_fit();
    // m_supportedCounterIdNameVec.shrink_to_fit();

    m_isProfilingEnabled = (AMDT_STATUS_OK == m_error) ? true : false;

    return m_error;
} // EnableProfiling


AMDTResult ppCollect::ValidateCounters()
{
    // -e is supported with counter-id/name
    // -D is supported with device-id
    // -P is supported with predefined counter group names

    // TODO: support for device name

    if (isOK() && isProfileStateReady())
    {
        // check if any counter group is specified
        AddCounterGroups();

        // If any device id is specified
        gtList<AMDTUInt32> deviceIdList = m_args.GetDeviceIDList();

        for (gtList<AMDTUInt32>::iterator iter = deviceIdList.begin(); iter != deviceIdList.end(); iter++)
        {
            AMDTUInt32 deviceId = (*iter);
            bool foundDevice = false;

            for (AMDTUInt32 i = 0; i < m_nbrSupportedCounters; i++)
            {
                if (((m_pSupportedCountersDesc + i)->m_deviceId == deviceId)
                    || (AMDT_PWR_ALL_DEVICES == deviceId)
                    || (0 == deviceId))
                {
                    // Add only the simple counter
                    if (AMDTUInt32(AMDT_PWR_VALUE_SINGLE == (m_pSupportedCountersDesc + i)->m_aggregation))
                    {
                        AddValidCounterId((m_pSupportedCountersDesc + i)->m_counterID);
                    }

                    foundDevice = true;
                }
            }

            if (false == foundDevice)
            {
                ReportError(false, "Invalid device(%u).\n", deviceId);
                m_error = AMDT_ERROR_FAIL;
            }
        }

        gtList<AMDTUInt32> counterIdList = m_args.GetCounterIDList();

        for (gtList<AMDTUInt32>::iterator iter = counterIdList.begin(); iter != counterIdList.end(); iter++)
        {
            if (!AddValidCounterId(*iter))
            {
                ReportError(false, "Invalid counter(%u).\n", (*iter));
                m_error = AMDT_ERROR_FAIL;
            }
        }

        gtList<gtString> counterNameList = m_args.GetCounterNameList();

        for (gtList<gtString>::iterator iter = counterNameList.begin(); iter != counterNameList.end(); iter++)
        {
            AMDTUInt32 counterID = static_cast<AMDTUInt32>(-1);

            if (!(*iter).compareNoCase(PP_PROFILE_ALL_COUNTERS))
            {
                // Add all the supported counters
                for (AMDTUInt32 i = 0; i < m_nbrSupportedCounters; i++)
                {
                    // For a all option configure and collect only the Simple Counters
                    if (AMDTUInt32(AMDT_PWR_VALUE_SINGLE == (m_pSupportedCountersDesc + i)->m_aggregation))
                    {
                        AddValidCounterId((m_pSupportedCountersDesc + i)->m_counterID);
                    }
                }
            }
            else
            {
                m_error = GetCounterId(*iter, counterID);

                bool validCounter = (AMDT_STATUS_OK == m_error) ? true : false;

                if (validCounter)
                {
                    validCounter = AddValidCounterId(counterID);
                }

                if (!validCounter)
                {
                    ReportError(false, "Invalid counter(%s).\n", (*iter).asASCIICharArray());
                    m_error = AMDT_ERROR_FAIL;
                }
            }
        }
    }

    return m_error;
} // ValidateCounters


AMDTResult ppCollect::ConfigureProfile()
{
    // Check for valid launch app
    if (isOK())
    {
        if (!m_args.GetLaunchApp().isEmpty())
        {
            osFilePath exePath(m_args.GetLaunchApp());

            if (!exePath.exists())
            {
                ReportError(false, "Launch application(%s) does not exist.\n", m_args.GetLaunchApp().asASCIICharArray());
                m_error = AMDT_ERROR_FAIL;
            }

            if (isOK() && (!exePath.isExecutable()))
            {
                ReportError(false, "Launch application(%s) is not an executable binary.\n", m_args.GetLaunchApp().asASCIICharArray());
                m_error = AMDT_ERROR_FAIL;
            }
        }
    }

    if (isOK() && isProfileStateReady())
    {
        if (isOK())
        {
            // Set the sampling period
            m_error = AMDTPwrSetTimerSamplingPeriod(m_args.GetSamplingInterval());
        }

        // Enable Simple Counters
        if (isOK())
        {
            for (AMDTUInt32 i = 0; i < m_simpleCounterIdVec.size(); i++)
            {
                m_error = AMDTPwrEnableCounter(m_simpleCounterIdVec[i]);

                // If the counter is already enabled, do not treat it as an error
                m_error = (AMDT_ERROR_COUNTER_ALREADY_ENABLED == m_error) ? AMDT_STATUS_OK : m_error;

                // To write into db, add the simple enabled counters
                if (AMDT_STATUS_OK == m_error)
                {
                    m_dbEnabledCountersVec.push_back(m_simpleCounterIdVec[i]);
                }
            }
        }

        // Enable cumulative counters
        if (isOK())
        {
            for (AMDTUInt32 i = 0; i < m_cumulativeCounterIdVec.size(); i++)
            {
                m_error = AMDTPwrEnableCounter(m_cumulativeCounterIdVec[i]);

                // If the counter is already enabled, do not treat it as an error
                m_error = (AMDT_ERROR_COUNTER_ALREADY_ENABLED == m_error) ? AMDT_STATUS_OK : m_error;
            }
        }

        // Enable histogram counters
        if (isOK())
        {
            for (AMDTUInt32 i = 0; i < m_histogramCounterIdVec.size(); i++)
            {
                m_error = AMDTPwrEnableCounter(m_histogramCounterIdVec[i]);

                // If the counter is already enabled, do not treat it as an error
                m_error = (AMDT_ERROR_COUNTER_ALREADY_ENABLED == m_error) ? AMDT_STATUS_OK : m_error;
            }
        }

#ifdef  AMDT_PWR_PROFILE_MODE_OFFLINE

        // Set the output file path for OFFLINE profile mode
        if (isOK() && m_args.IsProfileOffline())
        {
            osFilePath filePath;
            m_args.GetOutputFilePath(filePath);
            filePath.setFileExtension(PP_RAWFILE_EXTENSION);

            m_error = AMDTPwrSetProfileDataFile(filePath.asString().asASCIICharArray(), filePath.asString().lengthInBytes());
        }

#endif
    }

    return m_error;
} // ConfigureProfile


bool ppCollect::IsValidCounterId(AMDTUInt32 counterId)
{
    if (NULL != m_pSupportedCountersDesc)
    {
        for (AMDTUInt32 i = 0; i < m_nbrSupportedCounters; i++)
        {
            if (counterId == (m_pSupportedCountersDesc + i)->m_counterID)
            {
                return true;
            }
        }
    }

    return false;
} // IsValidCounterId


bool ppCollect::AddValidCounterId(AMDTUInt32 counterId)
{
    AMDTPwrAggregation aggType;
    bool retVal = false;

    if (IsValidCounterId(counterId) && GetCounterAggregationType(counterId, aggType))
    {
        retVal = true;

        switch (aggType)
        {
            case AMDT_PWR_VALUE_SINGLE:
                if (std::find(m_profiledCounterIdVec.begin(), m_profiledCounterIdVec.end(), counterId) == m_profiledCounterIdVec.end())
                {
                    m_simpleCounterIdVec.push_back(counterId);
                    m_profiledCounterIdVec.push_back(counterId);
                }

                break;

            case AMDT_PWR_VALUE_CUMULATIVE:
                if (std::find(m_profiledCounterIdVec.begin(), m_profiledCounterIdVec.end(), counterId) == m_profiledCounterIdVec.end())
                {
                    m_cumulativeCounterIdVec.push_back(counterId);
                    m_profiledCounterIdVec.push_back(counterId);
                }

                break;

            case AMDT_PWR_VALUE_HISTOGRAM:
                if (std::find(m_profiledCounterIdVec.begin(), m_profiledCounterIdVec.end(), counterId) == m_profiledCounterIdVec.end())
                {
                    m_histogramCounterIdVec.push_back(counterId);
                    m_profiledCounterIdVec.push_back(counterId);
                }

                break;

            default:
                retVal = false;
                break;
        }
    }

    return retVal;
} // AddValidCounterId


bool ppCollect::AddCounterGroups()
{
    bool temperatureCounter = false;
    gtList<gtString> counterGroupList = m_args.GetCounterGroupNameList();

    for (gtList<gtString>::iterator iter = counterGroupList.begin(); iter != counterGroupList.end(); iter++)
    {
        bool validCounterGroup = true;
        AMDTDeviceType deviceType = AMDT_PWR_DEVICE_CNT;
        AMDTPwrCategory category = AMDT_PWR_CATEGORY_CNT;

        if (!(*iter).compareNoCase(PP_COUNTER_GROUP_POWER))
        {
            deviceType = AMDT_PWR_DEVICE_CNT;  // all devices
            category = AMDT_PWR_CATEGORY_POWER;
        }
        else if (!(*iter).compareNoCase(PP_COUNTER_GROUP_TEMPERATURE))
        {
            if (m_isTemperatureSupported)
            {
                deviceType = AMDT_PWR_DEVICE_CNT;
                category = AMDT_PWR_CATEGORY_TEMPERATURE;
            }
            else
            {
                temperatureCounter = true;
                validCounterGroup = false;
            }
        }
        else if (!(*iter).compareNoCase(PP_COUNTER_GROUP_FREQUENCY))
        {
            deviceType = AMDT_PWR_DEVICE_CNT;
            category = AMDT_PWR_CATEGORY_FREQUENCY;
        }
        else if (!(*iter).compareNoCase(PP_COUNTER_GROUP_CU_POWER))
        {
            deviceType = AMDT_PWR_DEVICE_CPU_COMPUTE_UNIT;
            category = AMDT_PWR_CATEGORY_POWER;
        }
        else if (!(*iter).compareNoCase(PP_COUNTER_GROUP_CU_TEMPERATURE))
        {
            if (m_isTemperatureSupported)
            {
                deviceType = AMDT_PWR_DEVICE_CPU_COMPUTE_UNIT;
                category = AMDT_PWR_CATEGORY_TEMPERATURE;
            }
            else
            {
                temperatureCounter = true;
                validCounterGroup = false;
            }
        }
        else if (!(*iter).compareNoCase(PP_COUNTER_GROUP_GPU_POWER))
        {
            // FIXME what about external gpu
            deviceType = AMDT_PWR_DEVICE_INTERNAL_GPU;
            category = AMDT_PWR_CATEGORY_POWER;
        }
        else if (!(*iter).compareNoCase(PP_COUNTER_GROUP_GPU_TEMPERATURE))
        {
            // FIXME: what about external gpu
            if (m_isTemperatureSupported)
            {
                deviceType = AMDT_PWR_DEVICE_INTERNAL_GPU;
                category = AMDT_PWR_CATEGORY_TEMPERATURE;
            }
            else
            {
                temperatureCounter = true;
                validCounterGroup = false;
            }
        }
        else if (!(*iter).compareNoCase(PP_COUNTER_GROUP_CORE))
        {
            deviceType = AMDT_PWR_DEVICE_CPU_CORE;
            category = AMDT_PWR_CATEGORY_CNT;  // All categories
        }

#ifdef SVI2_COUNTERS_SUPPORTED
        else if (!(*iter).compareNoCase(PP_COUNTER_GROUP_SVI2))
        {
            deviceType = AMDT_PWR_DEVICE_SVI2;
            category = AMDT_PWR_CATEGORY_CNT;
        }

#endif
        else if (!(*iter).compareNoCase(PP_COUNTER_GROUP_DVFS))
        {
            deviceType = AMDT_PWR_DEVICE_CNT;
            category = AMDT_PWR_CATEGORY_DVFS;
        }
        else if (!(*iter).compareNoCase(PP_COUNTER_GROUP_ALL))
        {
            deviceType = AMDT_PWR_DEVICE_CNT;
            category = AMDT_PWR_CATEGORY_CNT;
        }
        else
        {
            validCounterGroup = false;
        }

        if (validCounterGroup)
        {
            if (false == AddCounters(deviceType, category))
            {
                ReportError(false, "Counter group (%s) sepecified with option -P doesn't have a valid counter.\n", (*iter).asASCIICharArray());
                m_error = AMDT_ERROR_FAIL;
            }
        }
        else
        {
            if (temperatureCounter)
            {
                ReportError(false, "Counter group (%s) sepecified with option -P is not supported for this platform.\n", (*iter).asASCIICharArray());
            }
            else
            {
                ReportError(false, "Invalid counter group (%s) is sepecified with option -P.\n", (*iter).asASCIICharArray());
            }

            m_error = AMDT_ERROR_FAIL;
        }
    }

    return true;
} // AddCounterGroups

bool ppCollect::AddCounters(AMDTDeviceType deviceType, AMDTPwrCategory category)
{
    bool retVal = false;

    for (AMDTUInt32 i = 0; i < m_nbrSupportedCounters; i++)
    {
        AMDTPwrCounterDesc* pDesc = m_pSupportedCountersDesc + i;

        if (
            ((AMDT_PWR_DEVICE_CNT == deviceType) || (m_deviceIdDescVec[pDesc->m_deviceId]->m_type == deviceType))
            &&
            ((AMDT_PWR_CATEGORY_CNT == category) || (pDesc->m_category == category))
            &&
            (AMDT_PWR_VALUE_SINGLE == pDesc->m_aggregation)
        )
        {
            retVal = AddValidCounterId(pDesc->m_counterID);
        }
    }

    return retVal;
} // AddCounters


bool ppCollect::GetCounterAggregationType(AMDTUInt32 counterId, AMDTPwrAggregation& aggregationType)
{
    if (counterId < m_supportedCounterIdDescVec.max_size())
    {
        aggregationType = m_supportedCounterIdDescVec[counterId]->m_aggregation;
        return true;
    }

    return false;
} // GetCounterAggregationType


bool ppCollect::IsValidCoreMask(gtUInt64 coreMask)
{
    bool retVal = true;

    if (0 == m_coreMask)
    {
        int nbrCores;
        osGetAmountOfLocalMachineCPUs(nbrCores);

        gtUInt64 tmpAffinity = 0;

        for (int core = 0; core < nbrCores; core++)
        {
            tmpAffinity <<= 1;
            tmpAffinity |= 1;
        }

        // TBD: should i set here or use the device topology to construct core mask?
        m_coreMask = tmpAffinity;
    }

    if (static_cast<gtUInt64>(-1) != coreMask)
    {
        if (m_coreMask < coreMask)
        {
            retVal = false;
            ReportError(false, "Invalid core affinity mask (0x%lx) specified with option(-c).\n", coreMask);
        }
    }

    return retVal;
} // IsValidCoreMask


AMDTResult ppCollect::LoadPcoreModule()
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    return AMDT_STATUS_OK;

#else
    // Check if the pcore module is loaded
    AMDTResult res = AMDT_STATUS_OK;
    FILE* fd = popen("lsmod | grep pcore", "r");

    char buf[256];

    if (fread(buf, 1, sizeof(buf), fd) > 0)   // if there is some result the module must be loaded
    {
        printf("module  pcore is loaded\n");
    }
    else
    {
        fprintf(stderr, "\n Error!!! Module pcore is not loaded\n");
        fprintf(stderr, "\n Please run the CodeXLPwrProfDriver.sh to install the pcore driver. \n");
        return AMDT_ERROR_FAIL;
    }

    // Check to see if the device file exists.
    // If not create one
    if (access("/dev/pcore", F_OK) == -1)
    {
        // file doesn't exist
        // On sucessful module load, the driver will create a proc file.
        // use that to get the major number of our device file.
        int nsize = 5;
        char somedata[5];
        ifstream myfile;
        myfile.open("/proc/pcore-device");
        myfile.read(somedata, nsize);
        myfile.close();

        int major;
        stringstream convert(somedata); // stringstream used for the conversion initialized with the contents of Text

        if (!(convert >> major))  //give the value to Result using the characters in the string
        {
            major = 0;//if that fails set Result to 0
        }

        dev_t dev;
        dev = makedev(major, 0);

        int rv = mknod("/dev/pcore", S_IFCHR | S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH, dev);

        if (rv < 0)
        {
            perror(" error creating device file \n");
            return AMDT_ERROR_FAIL;
        }
    }

    return res;
#endif
} // LoadPcoreModule


void ppCollect::RemovePcoreModule()
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    return;

#else

    //Wait for a few seconds for the process to finish
    sleep(5);
    // Remove the device file
    unlink("/dev/pcore");
    // Remove the module
    sleep(5);

    return;
#endif // AMDT_BUILD_TARGET == AMDT_LINUX_OS
} // RemovePcoreModule

AMDTResult ppCollect::LaunchTargetApp()
{
    bool createWindow = true;
    bool redirectFiles = true;
    bool appLaunched = false;

    m_error = AMDT_STATUS_OK; // FIXME

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    createWindow = false;
#endif // AMDT_BUILD_TARGET

    if (isOK())
    {
        if (!m_args.GetLaunchApp().isEmpty())
        {
            // Launch the target application, if any
            osFilePath exePath(m_args.GetLaunchApp());

            // Set up the arguments for the launched app
            gtString LaunchAppArgs = m_args.GetLaunchAppArgs();

            // Set the working directory for the launched process
            gtString workDir = m_args.GetWorkingDir();

            if (workDir.isEmpty())
            {
                if (!exePath.fileDirectoryAsString().startsWith(L"."))
                {
                    workDir = exePath.fileDirectoryAsString();
                }
                else
                {
                    osFilePath currentWorkDir;
                    currentWorkDir.setPath(osFilePath::OS_CURRENT_DIRECTORY);
                    workDir = currentWorkDir.fileDirectoryAsString();
                }
            }

            appLaunched = osLaunchSuspendedProcess(exePath,
                                                   LaunchAppArgs,
                                                   workDir,
                                                   m_launchedProcessId,
                                                   m_launchedProcessHandle,
                                                   m_launchedProcessThreadHandle,
                                                   createWindow,
                                                   redirectFiles);

            if (appLaunched)
            {
                // Set processor affinity
                osSetProcessAffinityMask(m_launchedProcessId, m_launchedProcessHandle, m_args.GetCoreAffinityMask());
            }
            else
            {
                ReportError(false, "Could not launch the target: %ls", m_args.GetLaunchApp().asASCIICharArray());
                m_error = AMDT_ERROR_FAIL;
            }
        }

        m_isAppLaunched = appLaunched;
    }

    return m_error;
}

AMDTResult ppCollect::GetProcessData(AMDTUInt32* pPIDCount, AMDTPwrProcessInfo** ppData)
{
    AMDTResult ret = AMDT_ERROR_FAIL;
    AMDTUInt32 pidCnt = 0;
    AMDTPwrProcessInfo* pData = nullptr;

    if (isOK())
    {
        m_nbrSamples = 0;
        m_pSampleData = nullptr;

        ret = AMDTGetProcessProfileData(&pidCnt, &pData, AMD_PWR_ALL_PIDS, false);

        if ((AMDT_STATUS_OK == m_error) && (pidCnt > 0) && (nullptr != pData))
        {
            *ppData = pData;
            *pPIDCount = pidCnt;
        }
        else
        {
            ReportError(true, "Reading process data failed with error code(0x%lx)", ret);
        }

    }

    return ret;
}

AMDTResult ppCollect::GetModuleData(AMDTPwrModuleData** ppData, AMDTUInt32* pModuleCnt, AMDTFloat32* pPower)
{
    AMDTResult ret = AMDT_ERROR_FAIL;
    AMDTUInt32 moduleCnt = 0;
    AMDTPwrModuleData* pData = nullptr;
    AMDTFloat32 power = 0;

    if (isOK())
    {
        m_nbrSamples = 0;
        m_pSampleData = nullptr;

        ret = AMDTPwrGetModuleProfileData(&pData, &moduleCnt, &power);

        if ((AMDT_STATUS_OK == m_error) && (moduleCnt > 0) && (nullptr != pData))
        {
            *pPower = power;
            *ppData = pData;
            *pModuleCnt = moduleCnt;
        }
        else
        {
            ReportError(true, "Reading module data failed with error code(0x%lx)", ret);
        }

    }

    return ret;
}

// DB specific functions
bool ppCollect::InitDb(AMDTProfileSessionInfo& sessionInfo)
{
    bool ret = false;

    // Create the DB.
    if (nullptr == m_pDbAdapter)
    {
        m_pDbAdapter = new amdtProfileDbAdapter;

        if (nullptr != m_pDbAdapter)
        {
            // Append the extension
            gtString dbExtension;

            m_pDbAdapter->GetDbFileExtension(dbExtension);
            gtString dbFilePath = sessionInfo.m_sessionDbFullPath;
            dbFilePath.append(dbExtension);

            // create the db
            ret = m_pDbAdapter->CreateDb(dbFilePath, AMDT_PROFILE_MODE_TIMELINE);

            // insert the session info
            if (ret)
            {
                m_pDbAdapter->InsertAllSessionInfo(sessionInfo,
                                                   m_args.GetSamplingInterval(),
                                                   m_dbDeviceVec,
                                                   m_dbCounterDescVec,
                                                   m_dbEnabledCountersVec);
            }
        }
    }

    return ret;
}

bool ppCollect::WriteSamplesIntoDb()
{
    // Check if we need to write these samples into db
    if (m_isDbExportEnabled && (nullptr != m_pDbAdapter))
    {
        // Convert the BE samples into a format with which the DB adapters can work.
        gtVector<AMDTProfileTimelineSample*> samplesBuffer;

        for (size_t i = 0; i < m_nbrSamples; ++i)
        {
            AMDTProfileTimelineSample* pCurrSample = nullptr;
            AMDTPwrSample* pCurrBeSample = (m_pSampleData + i);

            pCurrSample = ppCliUtils::ConvertPwrSampleToTimelineSample(*pCurrBeSample);

            samplesBuffer.push_back(pCurrSample);
        }

        m_pDbAdapter->InsertSamples(samplesBuffer);

        for (auto it : samplesBuffer)
        {
            delete it;
        }
    }

    return true;
}
