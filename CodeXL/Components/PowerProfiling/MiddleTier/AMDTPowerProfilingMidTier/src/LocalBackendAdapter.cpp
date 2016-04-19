//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file LocalBackendAdapter.cpp
///
//==================================================================================

// Local.
#include <AMDTPowerProfilingMidTier/include/LocalBackendAdapter.h>
#include <AMDTPowerProfilingMidTier/include/BackendDataConvertor.h>

// Infra.
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// The PP backend.
#include <AMDTPowerProfileAPI/inc/AMDTPowerProfileApi.h>
#include <AMDTPowerProfileAPI/inc/AMDTPowerProfileDataTypes.h>

// Common DB related structures
#include <AMDTCommonHeaders/AMDTCommonProfileDataTypes.h>

// C++.
#include <algorithm>

// For the app launch details - this mechanism will be refactored.
#include <AMDTRemoteClient/Include/CXLDaemonClient.h>

#ifdef MEASURE_TIME_BETWEEN_CALLS_ON_WINDOWS
LocalBackendAdapter::LocalBackendAdapter() : m_endStopwatch(0), m_startStopwatch(0), m_stopwatchFrequency(0)
{

    QueryPerformanceFrequency((LARGE_INTEGER*)&m_stopwatchFrequency);
}
#else
LocalBackendAdapter::LocalBackendAdapter()
{
}
#endif


LocalBackendAdapter::~LocalBackendAdapter()
{
}

PPResult LocalBackendAdapter::InitializeBackend()
{
    // For now, we are always in Online mode.
    const AMDTPwrProfileMode profileMode = AMDT_PWR_PROFILE_MODE_ONLINE;

    PPResult ret = PPR_UNKNOWN_FAILURE;
    AMDTResult rc = AMDTPwrProfileInitialize(profileMode);

    // Log the API initialization:
    gtString message;
    message.appendFormattedString(L"Initialized power profile backend. Result: %d", rc);
    OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_INFO);

    if (rc == AMDT_ERROR_DRIVER_ALREADY_INITIALIZED)
    {
        ret = PPR_NO_ERROR;
    }
    else if (rc == AMDT_STATUS_OK || rc == AMDT_WARN_IGPU_DISABLED || rc == AMDT_WARN_SMU_DISABLED)
    {
        AMDTResult rcInner = AMDTPwrSetSampleValueOption(AMDT_PWR_SAMPLE_VALUE_LIST);
        GT_IF_WITH_ASSERT(rcInner == AMDT_STATUS_OK)
        {
            ret = AMDT_WARN_SMU_DISABLED == rc ? PPR_WARNING_SMU_DISABLED : PPR_NO_ERROR;
        }
    }
    else if (AMDT_ERROR_PLATFORM_NOT_SUPPORTED == rc)
    {
        OS_OUTPUT_DEBUG_LOG(L"Power Profiling not supported on platform. Back-end initialization aborted.", OS_DEBUG_LOG_INFO);
        AMDTPwrProfileClose();
        ret = PPR_NOT_SUPPORTED;
    }
    else if (AMDT_DRIVER_VERSION_MISMATCH == rc)
    {
        OS_OUTPUT_DEBUG_LOG(L"Power Profiling driver mismatch. Backend initialization failed.", OS_DEBUG_LOG_INFO);
        AMDTPwrProfileClose();
        ret = PPR_DRIVER_VERSION_MISMATCH;
    }
    else if (AMDT_ERROR_HYPERVISOR_NOT_SUPPORTED == rc)
    {
        OS_OUTPUT_DEBUG_LOG(L"Power Profiling on hypervisor not supported. Backend initialization failed.", OS_DEBUG_LOG_INFO);
        AMDTPwrProfileClose();
        ret = PPR_HYPERVISOR_NOT_SUPPORTED;
    }
    else if (AMDT_ERROR_COUNTERS_NOT_ENABLED == rc)
    {
        OS_OUTPUT_DEBUG_LOG(L"No counters are enabled for collecting profile data. Backend initialization failed.", OS_DEBUG_LOG_INFO);
        AMDTPwrProfileClose();
        ret = PPR_COUNTERS_NOT_ENABLED;
    }

    else
    {
        OS_OUTPUT_DEBUG_LOG(L"Error initializing PP backend", OS_DEBUG_LOG_ERROR);
        AMDTPwrProfileClose();
    }

    return ret;
}

PPResult LocalBackendAdapter::GetSystemTopology(gtList<PPDevice*>& systemDevices)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    AMDTPwrDevice* pSysTopology = NULL;
    AMDTResult rc = AMDTPwrGetSystemTopology(&pSysTopology);
    GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
    {
        GT_IF_WITH_ASSERT(pSysTopology != NULL)
        {
            AMDTPwrDevice* pCurrDevice = pSysTopology;

            // If multiple System roots is not expected to be supported, then the hierarchy
            // should be switched to PPDevice*& instead of a list, and this while loop should
            // become a one iteration pass on its body.
            while (pCurrDevice != NULL)
            {
                systemDevices.push_back(BackendDataConvertor::CreatePPDevice(*pCurrDevice, this));
                pCurrDevice = pCurrDevice->m_pNextDevice;
            }

            ret = PPR_NO_ERROR;
        }
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(L"Error retrieving system topology from PP backend", OS_DEBUG_LOG_ERROR);
    }

    return ret;
}

PPResult LocalBackendAdapter::GetDeviceCounters(int deviceID, gtList<AMDTPwrCounterDesc*>& supportedCounters)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;

    AMDTUInt32 numOfSupportedCounters = 0;
    AMDTPwrCounterDesc* pBeSupportedCounters;
    AMDTResult rc = AMDTPwrGetDeviceCounters(deviceID, &numOfSupportedCounters, &pBeSupportedCounters);

    if (rc == AMDT_STATUS_OK)
    {
        AMDTPwrCounterDesc* pCurrentCounter = pBeSupportedCounters;

        for (size_t i = 0; (i < numOfSupportedCounters) && (pCurrentCounter != NULL); ++i)
        {
            if (BackendDataConvertor::IsCounterRequired(*pCurrentCounter))
            {
                // Create a copy of the BE's counter.
                AMDTPwrCounterDesc* pCounter =
                    BackendDataConvertor::CopyPwrCounterDesc(*pCurrentCounter);

                // Add that new counter to our supported counters list.
                supportedCounters.push_back(pCounter);
            }

            // Advance the pointer.
            ++pCurrentCounter;
        }

        ret = PPR_NO_ERROR;
    }
    else if (rc == AMDT_ERROR_INVALID_DEVICEID)
    {
        // This error should be allowed as the backend API returns it for the System device,
        // which is a valid device. The backend API should be updated to return no error in that
        // case.
        ret = PPR_NO_ERROR;
    }

    return ret;
}

PPResult LocalBackendAdapter::GetMinTimerSamplingPeriodMS(unsigned int& samplingPeriodBufferMs)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    samplingPeriodBufferMs = 0;
    AMDTUInt32 beSamplingIntervalMs = 0;
    AMDTResult rc = AMDTPwrGetMinimalTimerSamplingPeriod(&beSamplingIntervalMs);
    GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
    {
        samplingPeriodBufferMs = beSamplingIntervalMs;
        ret = PPR_NO_ERROR;
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(L"Error retrieving min sampling from PP backend", OS_DEBUG_LOG_ERROR);
    }

    return ret;
}

PPResult LocalBackendAdapter::GetCurrentSamplingInterval(unsigned int& samplingIntervalMs)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    samplingIntervalMs = 0;
    AMDTUInt32 beSamplingIntervalMs = 0;
    AMDTResult rc = AMDTPwrGetTimerSamplingPeriod(&beSamplingIntervalMs);
    GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
    {
        samplingIntervalMs = beSamplingIntervalMs;
        ret = PPR_NO_ERROR;
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(L"Error retrieving current sampling from PP backend", OS_DEBUG_LOG_ERROR);
    }

    return ret;
}

PPResult LocalBackendAdapter::GetProfilingState(AMDTPwrProfileState& stateBuffer)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;

    GT_UNREFERENCED_PARAMETER(stateBuffer);
    GT_ASSERT_EX(false, L"The method or operation is not implemented.");

    return ret;
}

PPResult LocalBackendAdapter::IsCounterEnabled(unsigned int counterId, bool& isEnabled)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;

    // Check if the counter is enabled.
    AMDTResult rc = AMDTPwrIsCounterEnabled(counterId);
    isEnabled = (rc == AMDT_STATUS_OK);

    // By getting one of these values, we know that the function completed succesfully.
    if (rc == AMDT_STATUS_OK || rc == AMDT_ERROR_COUNTER_NOT_ENABLED)
    {
        ret = PPR_NO_ERROR;
    }
    else
    {
        gtString errMsg(L"Error checking if a PP counter is enabled, counterId = ");
        errMsg << counterId << L", BE error code = " << static_cast<int>(rc);
        OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
    }

    return ret;
}

PPResult LocalBackendAdapter::GetNumOfEnabledCounters(int& numOfAvailableCounters)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;

    GT_UNREFERENCED_PARAMETER(numOfAvailableCounters);
    GT_ASSERT_EX(false, L"The method or operation is not implemented.");

    return ret;
}

PPResult LocalBackendAdapter::StartProfiling()
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    AMDTResult rc = AMDTPwrStartProfiling();
    GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
    {
        ret = PPR_NO_ERROR;
    }
    else
    {
        if (rc == AMDT_ERROR_ACCESSDENIED)
        {
            ret = PPR_DRIVER_ALREADY_IN_USE;
        }

        if (rc == AMDT_WARN_SMU_DISABLED)
        {
            ret = PPR_WARNING_SMU_DISABLED;
        }

        if (rc == AMDT_WARN_IGPU_DISABLED)
        {
            ret = PPR_WARNING_IGPU_DISABLED;
        }

        if (rc == AMDT_ERROR_HYPERVISOR_NOT_SUPPORTED)
        {
            ret = PPR_HYPERVISOR_NOT_SUPPORTED;
        }

        if (rc == AMDT_ERROR_COUNTERS_NOT_ENABLED)
        {
            ret = PPR_COUNTERS_NOT_ENABLED;
        }

        // Add log printout with the API return value:
        gtString message;
        message.appendFormattedString(L"Error or warning starting power profile API. API return value: 0x%08x", rc);
        OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_ERROR);
    }

    return ret;
}

PPResult LocalBackendAdapter::StopProfiling()
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    AMDTResult rc = AMDTPwrStopProfiling();
    GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
    {
        ret = PPR_NO_ERROR;
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(L"Error stopping PP.", OS_DEBUG_LOG_ERROR);
    }

    return ret;
}

PPResult LocalBackendAdapter::PauseProfiling()
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    AMDTResult rc = AMDTPwrPauseProfiling();
    GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
    {
        ret = PPR_NO_ERROR;
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(L"Error pausing PP.", OS_DEBUG_LOG_ERROR);
    }

    return ret;
}

PPResult LocalBackendAdapter::ResumeProfiling()
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    AMDTResult rc = AMDTPwrResumeProfiling();
    GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
    {
        ret = PPR_NO_ERROR;
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(L"Error resuming PP.", OS_DEBUG_LOG_ERROR);
    }

    return ret;
}

PPResult LocalBackendAdapter::EnableCounter(int counterId)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    AMDTResult rc = AMDTPwrEnableCounter(counterId);
    GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK || rc == AMDT_ERROR_COUNTER_ALREADY_ENABLED)
    {
        ret = PPR_NO_ERROR;
    }
    else
    {
        gtString errMsg(L"Error enabling PP counter backend, counter id = ");
        errMsg << counterId;
        OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
    }

    return ret;
}

PPResult LocalBackendAdapter::SetTimerSamplingInterval(unsigned int interval)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    AMDTResult rc = AMDTPwrSetTimerSamplingPeriod(interval);
    GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
    {
        ret = PPR_NO_ERROR;
    }
    else
    {
        gtString errMsg(L"Error setting PP sampling interval, interval = ");
        errMsg << interval;
        OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
    }

    return ret;
}

PPResult LocalBackendAdapter::CloseProfileSession()
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    AMDTResult rc = AMDTPwrProfileClose();
    GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
    {
        ret = PPR_NO_ERROR;
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(L"Error closing PP session", OS_DEBUG_LOG_ERROR);
    }

    return ret;
}

PPResult LocalBackendAdapter::ReadAllEnabledCounters(gtVector<AMDTProfileTimelineSample*>& buffer)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;

    // The number of samples taken.
    AMDTUInt32 numOfSamples = 0;

    // The samples array to be filled by the backend.
    AMDTPwrSample* pSamples;

#ifdef MEASURE_TIME_BETWEEN_CALLS_ON_WINDOWS
    QueryPerformanceCounter((LARGE_INTEGER*)&m_endStopwatch);
    double timeBetweenCalls = double(m_endStopwatch - m_startStopwatch) / double(m_stopwatchFrequency);

    // Print the time between reading samples
    wchar_t msg[1000];
    swprintf(msg, L"time between reading samples = %f\n", timeBetweenCalls);
    OutputDebugString(msg);

#endif // MEASURE_TIME_BETWEEN_CALLS_ON_WINDOWS

    // Read the enabled counters from the backend.
    AMDTResult rc = AMDTPwrReadAllEnabledCounters(&numOfSamples, &pSamples);

#ifdef MEASURE_TIME_BETWEEN_CALLS_ON_WINDOWS
    QueryPerformanceCounter((LARGE_INTEGER*)&m_startStopwatch);
#endif // MEASURE_TIME_BETWEEN_CALLS_ON_WINDOWS

    if (rc == AMDT_STATUS_OK)
    {

#ifdef MEASURE_TIME_BETWEEN_CALLS_ON_WINDOWS
        // Print the time between reading samples
        wchar_t msg2[1000];
        swprintf(msg2, L"BE - SUCCESS IN READING ALL COUNTERS, numOfSamples = %d\n", numOfSamples);
        OutputDebugString(msg2);
#endif

        if (numOfSamples > 0)
        {
            // Go through each backend sample, create a mirror object in our memory
            // and copy the backend sample's values to our object. Then aggregate our
            // object in our container of samples to use an output.
            for (size_t i = 0; i < numOfSamples; ++i)
            {
                AMDTProfileTimelineSample* pCurrSample = new AMDTProfileTimelineSample();
                const AMDTPwrSample* pCurrBeSample = (pSamples + i);
                BackendDataConvertor::AMDTPwrSampleToPPSample(pCurrBeSample, *pCurrSample);
                buffer.push_back(pCurrSample);
            }

            ret = PPR_NO_ERROR;
        }
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(L"Error reading all PP enabled counters", OS_DEBUG_LOG_ERROR);

#ifdef MEASURE_TIME_BETWEEN_CALLS_ON_WINDOWS
        // Print the time between reading samples
        wchar_t msg3[1000];
        swprintf(msg3, L"BE - FAILED TO READ ALL COUNTERS, RC = %u\n", rc);
        OutputDebugString(msg3);
#endif
    }

    return ret;
}

PPResult LocalBackendAdapter::DisableCounter(int counterId)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    AMDTResult rc = AMDTPwrDisableCounter(counterId);

    if (rc == AMDT_STATUS_OK || rc == AMDT_ERROR_COUNTER_NOT_ENABLED)
    {
        ret = PPR_NO_ERROR;
    }
    else
    {
        gtString errMsg(L"Error disabling PP counter backend, counter id = ");
        errMsg << counterId;
        OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
    }

    return ret;
}

PPResult LocalBackendAdapter::SetApplicationLaunchDetails(const ApplicationLaunchDetails& appLaunchDetails)
{
    GT_UNREFERENCED_PARAMETER(appLaunchDetails);
    return PPR_NOT_SUPPORTED;
}

AppLaunchStatus LocalBackendAdapter::GetApplicationLaunchStatus()
{
    // This status is irrelevant for local sessions as higher level components
    // are responsible for launching the target application.
    return rasOk;
}

void LocalBackendAdapter::GetLastErrorMessage(gtString& msg)
{
    // Nothing to return by LocalBackendAdapter for now.
    msg = L"";
}

