//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file RemoteBackendAdapter.cpp
///
//==================================================================================

// Local.
#include <AMDTPowerProfilingMidTier/include/BackendDataConvertor.h>
#include <AMDTPowerProfilingMidTier/include/RemoteBackendAdapter.h>

// Power profiling backend definitions.
#include <AMDTPowerProfileAPI/inc/AMDTPowerProfileDataTypes.h>

// Infra.
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Network.
#include <AMDTOSWrappers/Include/osPortAddress.h>

const long READ_TIMEOUT_MS = 1500;


RemoteBackendAdapter::~RemoteBackendAdapter()
{
    // Close the connection.
    CXLDaemonClient::Close();
}

PPResult RemoteBackendAdapter::InitializeBackend()
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    m_lastErrorMsg.makeEmpty();

    // Initialize the remote client.
    osPortAddress remoteTargetAddr(m_remoteHostName, m_remoteTargetPort);
    bool rcClient = CXLDaemonClient::IsInitialized(remoteTargetAddr);

    if (rcClient == false)
    {
        rcClient = CXLDaemonClient::Init(remoteTargetAddr, READ_TIMEOUT_MS);
        GT_ASSERT_EX(rcClient, L"ERROR: Setting the remote power profiling target address");
    }

    if (rcClient)
    {
        auto pRemoteClient = CXL_DAEMON_CLIENT;
        GT_IF_WITH_ASSERT(pRemoteClient != nullptr)
        {
            osPortAddress clientAddr;
            rcClient = pRemoteClient->ConnectToDaemon(clientAddr);// function takes an output parameter in which it returns the address and port number of the remote end after the connection was established.
            GT_ASSERT_EX(rcClient, L"ERROR: Connecting to the remote agent.");

            if (rcClient)
            {
                // Perform the handshake.
                bool isHandshakeOk = false;
                gtString handshakeErr;
                pRemoteClient->PerformHandshake(isHandshakeOk, handshakeErr);

                if (isHandshakeOk)
                {
                    gtUInt32 rc = static_cast<gtUInt32>(-1);
                    rcClient = pRemoteClient->InitPowerProfilingSession(rc);
                    GT_ASSERT(rcClient);
                    GT_ASSERT(AMDT_ERROR_PLATFORM_NOT_SUPPORTED == rc ||
                              AMDT_STATUS_OK == rc || AMDT_ERROR_DRIVER_UNAVAILABLE == rc || AMDT_DRIVER_VERSION_MISMATCH == rc);

                    if (rc == AMDT_STATUS_OK)
                    {
                        rcClient = pRemoteClient->SetPowerProfilingSamplingOption(AMDT_PWR_SAMPLE_VALUE_LIST, rc);
                        GT_IF_WITH_ASSERT(rcClient && rc == AMDT_STATUS_OK)
                        {
                            ret = PPR_NO_ERROR;
                        }
                        else
                        {
                            ret = PPR_REMOTE_SESSION_CONFIGURATION_ERROR;
                        }
                    }
                    else
                    {
                        if (AMDT_ERROR_PLATFORM_NOT_SUPPORTED == rc)
                        {
                            OS_OUTPUT_DEBUG_LOG(L"REMOTE: Power Profiling not supported. Back-end initialization aborted.", OS_DEBUG_LOG_INFO);
                            ret = PPR_NOT_SUPPORTED;
                        }
                        else if (AMDT_DRIVER_VERSION_MISMATCH == rc)
                        {
                            OS_OUTPUT_DEBUG_LOG(L"REMOTE: Power Profiling driver mismatch on the remote machine. Backend initialization failed.", OS_DEBUG_LOG_INFO);
                            ret = PPR_DRIVER_VERSION_MISMATCH;
                        }
                        else if (AMDT_ERROR_DRIVER_UNAVAILABLE == rc)
                        {
                            OS_OUTPUT_DEBUG_LOG(L"REMOTE: Power Profiling driver already in use on the remote machine. Backend initialization failed.", OS_DEBUG_LOG_INFO);
                            ret = PPR_DRIVER_ALREADY_IN_USE;
                        }
                        else if (AMDT_WARN_SMU_DISABLED == rc)
                        {
                            OS_OUTPUT_DEBUG_LOG(L"REMOTE: Power Profiling driver returned a warning that the SMU is not accessible. SMU counter will be disabled.", OS_DEBUG_LOG_INFO);
                            ret = PPR_WARNING_SMU_DISABLED;
                        }
                        else if (AMDT_WARN_IGPU_DISABLED == rc)
                        {
                            OS_OUTPUT_DEBUG_LOG(L"REMOTE: Power Profiling driver returned a warning that the iGPU is not accessible. iGPU counter will be disabled.", OS_DEBUG_LOG_INFO);
                            ret = PPR_WARNING_IGPU_DISABLED;
                        }
                        else
                        {
                            OS_OUTPUT_DEBUG_LOG(L"Error initializing PP backend", OS_DEBUG_LOG_ERROR);
                        }

                        // Disconnect from the remote agent.
                        pRemoteClient->DisconnectWithoutClosing();

                        // Log the API initialization:
                        gtString message;
                        message.appendFormattedString(L"Initialized remote power profile backend. Result: %d", ret);
                        OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_INFO);
                    }
                }
                else
                {
                    // The handshake failed.
                    m_lastErrorMsg = handshakeErr;
                    ret = PPR_REMOTE_HANDSHAKE_FAILURE;
                }
            }
            else
            {
                ret = PPR_REMOTE_CONNECTION_ERROR;
            }
        }
        else
        {
            ret = PPR_REMOTE_CONNECTION_ERROR;
        }
    }

    return ret;
}

PPResult RemoteBackendAdapter::GetSystemTopology(gtList<PPDevice*>& systemDevices)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    auto pRemoteClient = CXL_DAEMON_CLIENT;
    GT_IF_WITH_ASSERT(pRemoteClient != nullptr)
    {
        AMDTPwrDevice* pSysTopology = nullptr;
        gtUInt32 rcAsInt = 0;
        pRemoteClient->GetSystemTopology(pSysTopology, rcAsInt);
        AMDTResult rc = static_cast<AMDTResult>(rcAsInt);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            GT_IF_WITH_ASSERT(pSysTopology != nullptr)
            {
                AMDTPwrDevice* pCurrDevice = pSysTopology;

                // If multiple System roots is not expected to be supported, then the hierarchy
                // should be switched to PPDevice*& instead of a list, and this while loop should
                // become a one iteration pass on its body.
                while (pCurrDevice != nullptr)
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
    }
    return ret;
}

PPResult RemoteBackendAdapter::GetDeviceCounters(int deviceID, gtList<AMDTPwrCounterDesc*>& supportedCounters)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    auto pRemoteClient = CXL_DAEMON_CLIENT;
    GT_IF_WITH_ASSERT(pRemoteClient != nullptr)
    {
        AMDTUInt32 numOfSupportedCounters = 0;
        AMDTPwrCounterDesc* pBeSupportedCounters;
        AMDTResult rc = AMDT_STATUS_OK;
        pRemoteClient->GetDeviceSupportedCounters(deviceID, rc, numOfSupportedCounters, pBeSupportedCounters);

        if (rc == AMDT_STATUS_OK)
        {
            AMDTPwrCounterDesc* pCurrentCounter = pBeSupportedCounters;

            for (size_t i = 0; (i < numOfSupportedCounters) && (pCurrentCounter != nullptr); ++i)
            {
                if (BackendDataConvertor::IsCounterRequired(*pCurrentCounter))
                {
                    // Create a copy of the BE's counter.
                    AMDTPwrCounterDesc* pCounter =  BackendDataConvertor::CopyPwrCounterDesc(*pCurrentCounter);

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
    }


    return ret;
}

PPResult RemoteBackendAdapter::GetMinTimerSamplingPeriodMS(unsigned int& samplingPeriodBufferMs)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    auto pRemoteClient = CXL_DAEMON_CLIENT;
    GT_IF_WITH_ASSERT(pRemoteClient != nullptr)
    {
        samplingPeriodBufferMs = 0;
        AMDTUInt32 beSamplingIntervalMs = 0;
        gtUInt32 rcAsInt = 0;
        pRemoteClient->GetMinSamplingIntervalMs(beSamplingIntervalMs, rcAsInt);
        AMDTResult rc = static_cast<AMDTResult>(rcAsInt);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            samplingPeriodBufferMs = beSamplingIntervalMs;
            ret = PPR_NO_ERROR;
        }
        else
        {
            OS_OUTPUT_DEBUG_LOG(L"Error retrieving min sampling from PP backend", OS_DEBUG_LOG_ERROR);
        }
    }
    return ret;
}

PPResult RemoteBackendAdapter::GetProfilingState(AMDTPwrProfileState& stateBuffer)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;

    GT_UNREFERENCED_PARAMETER(stateBuffer);
    GT_ASSERT_EX(false, L"The method or operation is not implemented.");

    return ret;
}

PPResult RemoteBackendAdapter::GetCurrentSamplingInterval(unsigned int& samplingIntervalMs)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    auto pRemoteClient = CXL_DAEMON_CLIENT;
    GT_IF_WITH_ASSERT(pRemoteClient != nullptr)
    {
        samplingIntervalMs = 0;
        gtUInt32 beSamplingIntervalMs = 0;
        gtUInt32 rcAsInt = 0;
        pRemoteClient->GetCurrentSamplingIntervalMs(beSamplingIntervalMs, rcAsInt);
        AMDTResult rc = static_cast<AMDTResult>(rcAsInt);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            samplingIntervalMs = beSamplingIntervalMs;
            ret = PPR_NO_ERROR;
        }
        else
        {
            OS_OUTPUT_DEBUG_LOG(L"Error retrieving current sampling from PP backend", OS_DEBUG_LOG_ERROR);
        }
    }
    return ret;
}

PPResult RemoteBackendAdapter::IsCounterEnabled(unsigned int counterId, bool& isEnabled)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    auto pRemoteClient = CXL_DAEMON_CLIENT;
    GT_IF_WITH_ASSERT(pRemoteClient != nullptr)
    {
        // Check if the counter is enabled.
        gtUInt32 rcAsInt = 0;
        pRemoteClient->IsCounterEnabled(counterId, rcAsInt);
        AMDTResult rc = static_cast<AMDTResult>(rcAsInt);
        isEnabled = (rc == AMDT_STATUS_OK);

        // By getting one of these values, we know that the function completed successfully.
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
    }
    return ret;
}

PPResult RemoteBackendAdapter::GetNumOfEnabledCounters(int& numOfAvailableCounters)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;

    GT_UNREFERENCED_PARAMETER(numOfAvailableCounters);
    GT_ASSERT_EX(false, L"The method or operation is not implemented.");

    return ret;
}

PPResult RemoteBackendAdapter::StartProfiling()
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    auto pRemoteClient = CXL_DAEMON_CLIENT;
    GT_IF_WITH_ASSERT(pRemoteClient != nullptr)
    {
        gtUInt32 rcAsInt = 0;
        pRemoteClient->StartPowerProfiling(m_appLaunchDetails, rcAsInt, m_appLaunchStatus);
        AMDTResult rc = static_cast<AMDTResult>(rcAsInt);
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

            // Add log printout with the API return value:
            gtString message;
            message.appendFormattedString(L"Error or warning starting power profile API. API return value: 0x%08x", rc);
            OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_ERROR);
        }
    }
    return ret;
}

PPResult RemoteBackendAdapter::StopProfiling()
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    auto pRemoteClient = CXL_DAEMON_CLIENT;
    GT_IF_WITH_ASSERT(pRemoteClient != nullptr)
    {
        gtUInt32 rcAsInt = 0;
        pRemoteClient->StopPowerProfiling(rcAsInt);
        AMDTResult rc = static_cast<AMDTResult>(rcAsInt);

        if (rc == AMDT_STATUS_OK)
        {
            ret = PPR_NO_ERROR;
        }

        // Close the connection.
        pRemoteClient->Close();
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(L"Error stopping PP.", OS_DEBUG_LOG_ERROR);
    }

    return ret;
}

PPResult RemoteBackendAdapter::PauseProfiling()
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    auto pRemoteClient = CXL_DAEMON_CLIENT;
    GT_IF_WITH_ASSERT(pRemoteClient != nullptr)
    {
        gtUInt32 rcAsInt = 0;
        pRemoteClient->PausePowerProfiling(rcAsInt);
        AMDTResult rc = static_cast<AMDTResult>(rcAsInt);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            ret = PPR_NO_ERROR;
        }
        else
        {
            OS_OUTPUT_DEBUG_LOG(L"Error pausing PP.", OS_DEBUG_LOG_ERROR);
        }
    }
    return ret;
}

PPResult RemoteBackendAdapter::ResumeProfiling()
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    auto pRemoteClient = CXL_DAEMON_CLIENT;
    GT_IF_WITH_ASSERT(pRemoteClient != nullptr)
    {
        gtUInt32 rcAsInt = 0;
        pRemoteClient->ResumePowerProfiling(rcAsInt);
        AMDTResult rc = static_cast<AMDTResult>(rcAsInt);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            ret = PPR_NO_ERROR;
        }
        else
        {
            OS_OUTPUT_DEBUG_LOG(L"Error resuming PP.", OS_DEBUG_LOG_ERROR);
        }
    }
    return ret;
}

PPResult RemoteBackendAdapter::EnableCounter(int counterId)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    auto pRemoteClient = CXL_DAEMON_CLIENT;
    GT_IF_WITH_ASSERT(pRemoteClient != nullptr)
    {
        gtUInt32 rcAsInt = 0;
        pRemoteClient->EnableCounter(counterId, rcAsInt);
        AMDTResult rc = static_cast<AMDTResult>(rcAsInt);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            ret = PPR_NO_ERROR;
        }
        else
        {
            gtString errMsg(L"Error enabling PP counter backend, counter id = ");
            errMsg << counterId;
            OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
        }
    }
    return ret;
}

PPResult RemoteBackendAdapter::DisableCounter(int counterId)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    auto pRemoteClient = CXL_DAEMON_CLIENT;
    GT_IF_WITH_ASSERT(pRemoteClient != nullptr)
    {
        gtUInt32 rcAsInt = 0;
        pRemoteClient->DisableCounter(counterId, rcAsInt);
        AMDTResult rc = static_cast<AMDTResult>(rcAsInt);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            ret = PPR_NO_ERROR;
        }
        else
        {
            gtString errMsg(L"Error disabling PP counter backend, counter id = ");
            errMsg << counterId;
            OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
        }
    }
    return ret;
}

PPResult RemoteBackendAdapter::SetTimerSamplingInterval(unsigned int interval)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    auto pRemoteClient = CXL_DAEMON_CLIENT;
    GT_IF_WITH_ASSERT(pRemoteClient != nullptr)
    {
        gtUInt32 rcAsInt = 0;
        bool isOk = pRemoteClient->SetPowerSamplingInterval(interval, rcAsInt);

        if (isOk)
        {
            AMDTResult rc = static_cast<AMDTResult>(rcAsInt);
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
        }
        else
        {
            ret = PPR_REMOTE_CONNECTION_ERROR;
        }
    }
    return ret;
}

PPResult RemoteBackendAdapter::CloseProfileSession()
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    auto pRemoteClient = CXL_DAEMON_CLIENT;
    GT_IF_WITH_ASSERT(pRemoteClient != nullptr)
    {
        gtUInt32 rcAsInt = 0;
        pRemoteClient->ClosePowerProfiling(rcAsInt);
        AMDTResult rc = static_cast<AMDTResult>(rcAsInt);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            ret = PPR_NO_ERROR;
        }
        else
        {
            OS_OUTPUT_DEBUG_LOG(L"Error closing PP session", OS_DEBUG_LOG_ERROR);
        }
    }
    return ret;
}

PPResult RemoteBackendAdapter::ReadAllEnabledCounters(gtVector<AMDTProfileTimelineSample*>& buffer)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    auto pRemoteClient = CXL_DAEMON_CLIENT;
    GT_IF_WITH_ASSERT(pRemoteClient != nullptr)
    {
        // Backend return code as an int.
        gtUInt32 rcAsInt = 0;

        // The number of samples taken.
        gtUInt32 numOfSamples = 0;

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
        bool isOk = pRemoteClient->ReadAllEnabledCounters(numOfSamples, pSamples, rcAsInt);

        AMDTResult rc = static_cast<AMDTResult>(rcAsInt);

        if (isOk)
        {

#ifdef MEASURE_TIME_BETWEEN_CALLS_ON_WINDOWS
            QueryPerformanceCounter((LARGE_INTEGER*)&m_startStopwatch);
#endif // MEASURE_TIME_BETWEEN_CALLS_ON_WINDOWS

            if (rc == AMDT_STATUS_OK)
            {

#ifdef MEASURE_TIME_BETWEEN_CALLS_ON_WINDOWS
                // Print the time between reading samples
                wchar_t msg[1000];
                swprintf(msg, L"BE - SUCCESS IN READING ALL COUNTERS, numOfSamples = %d\n", numOfSamples);
                OutputDebugString(msg);
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
                wchar_t msg[1000];
                swprintf(msg, L"BE - FAILED TO READ ALL COUNTERS, RC = %u\n", rc);
                OutputDebugString(msg);
#endif
            }
        }
        else
        {
            if (rc == DMN_SELF_TERMINATION_CODE)
            {
                // The target application stopped running on the remote machine.
                ret = PPR_REMOTE_APP_STOPPED;
            }
            else if (rc == rceCommunicationFailure)
            {
                // This is a communication failure.
                ret = PPR_COMMUNICATION_FAILURE;
            }
        }
    }

    return ret;
}

bool RemoteBackendAdapter::SetRemoteTarget(const gtString& remoteTargetHostName, unsigned short remoteTargetPortNumber)
{
    bool ret = false;

    // Set the relevant data members.
    m_remoteHostName = remoteTargetHostName;
    m_remoteTargetPort = remoteTargetPortNumber;

    // Sanity check.
    ret = (!m_remoteHostName.isEmpty() && m_remoteTargetPort > 0);

    return ret;
}

PPResult RemoteBackendAdapter::SetApplicationLaunchDetails(const ApplicationLaunchDetails& appLaunchDetails)
{
    // Assign the application details.
    m_appLaunchDetails = appLaunchDetails;

    // Reset the status.
    m_appLaunchStatus = rasUnknown;

    return PPR_NO_ERROR;
}

AppLaunchStatus RemoteBackendAdapter::GetApplicationLaunchStatus()
{
    return m_appLaunchStatus;
}

void RemoteBackendAdapter::GetLastErrorMessage(gtString& msg)
{
    msg = m_lastErrorMsg;
}
