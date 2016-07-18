//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnPowerBackendAdapter.cpp
///
//==================================================================================

// C++.
#include <sstream>

// Local.
#include <AMDTRemoteAgent/dmnPowerBackendAdapter.h>
#include <AMDTRemoteAgent/dmnSessionThread.h>
#include <AMDTRemoteAgent/dmnUtils.h>

// Infra.
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osThread.h>

// ************************************************************************************************
// Static synchronization mechanism to prevent from multiple threads to access the backend - START.

static bool gs_isSessionInProgress = false;
static osCriticalSection gs_threadGuard;

static bool IsRemoteSessionInProgress()
{
    osCriticalSectionLocker locker(gs_threadGuard);
    return gs_isSessionInProgress;
}

static void SetRemoteSessionInProgress(bool isInProgress)
{
    osCriticalSectionLocker locker(gs_threadGuard);
    gs_isSessionInProgress = isInProgress;
}

// Static synchronization mechanism to prevent from multiple threads to access the backend - END.
// ************************************************************************************************

dmnPowerBackendAdapter::dmnPowerBackendAdapter(dmnSessionThread* pOwner,
                                               osTCPSocketServerConnectionHandler* pConnHandler) : m_pSessionThread(pOwner),
    m_pConnHandler(pConnHandler), m_procId(0), m_pAppWatcherThread(nullptr), m_isTerminationSafe(false),
    m_isStopProfile(true)
{
    GT_ASSERT(m_pConnHandler != NULL);
}


dmnPowerBackendAdapter::~dmnPowerBackendAdapter()
{
    // Terminate the watcher thread and release it memory.
    delete m_pAppWatcherThread;
    m_pAppWatcherThread = nullptr;

    // If the target application is still running, terminate it.
    if (m_procId != 0)
    {
        osTerminateProcess(m_procId);
    }
}

bool dmnPowerBackendAdapter::handlePowerSessionInitRequest()
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pConnHandler != NULL)
    {
        // First, check if the driver is not taken.
        (*m_pConnHandler) << IsRemoteSessionInProgress();

        if (!IsRemoteSessionInProgress())
        {
            // We are now starting a session.
            SetRemoteSessionInProgress(true);

            // Extract the profile mode.
            gtInt32 profileModeAsInt = -1;
            (*m_pConnHandler) >> profileModeAsInt;
            GT_IF_WITH_ASSERT(profileModeAsInt > -1)
            {
                // Initialize the power profiling backend.
                AMDTPwrProfileMode profileMode = static_cast<AMDTPwrProfileMode>(profileModeAsInt);
                AMDTResult rc = AMDTPwrProfileInitialize(profileMode);
                gtUInt32 retAsUnsignedInt = static_cast<gtUInt32>(rc);
                (*m_pConnHandler) << retAsUnsignedInt;
                ret = true;
            }
        }
    }
    return ret;
}


bool dmnPowerBackendAdapter::handlePowerSessionConfigRequest()
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pConnHandler != NULL)
    {
        // First, extract the option.
        gtInt32 optionAsGtInt = 0;
        (*m_pConnHandler) >> optionAsGtInt;
        AMDTSampleValueOption valueOption = static_cast<AMDTSampleValueOption>(optionAsGtInt);

        // Set the config option.
        AMDTResult rc = AMDTPwrSetSampleValueOption(valueOption);

        // Transfer the return code.
        gtUInt32 retAsUnsignedInt = static_cast<gtUInt32>(rc);
        (*m_pConnHandler) << retAsUnsignedInt;
        ret = (rc == AMDT_STATUS_OK);
    }
    return ret;
}


bool dmnPowerBackendAdapter::handleSetSamplingIntervalMsRequest()
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pConnHandler != NULL)
    {
        // First, extract the sampling period.
        gtUInt32 samplingIntervalAsGtUInt = 0;
        (*m_pConnHandler) >> samplingIntervalAsGtUInt;
        GT_IF_WITH_ASSERT(samplingIntervalAsGtUInt > 0)
        {
            // Set the sampling interval.
            AMDTUInt32 samplingInterval = static_cast<AMDTPwrProfileMode>(samplingIntervalAsGtUInt);
            AMDTResult rc = AMDTPwrSetTimerSamplingPeriod(samplingInterval);

            // Transfer the return code.
            gtUInt32 retAsUnsignedInt = static_cast<gtUInt32>(rc);
            (*m_pConnHandler) << retAsUnsignedInt;
            ret = (rc == AMDT_STATUS_OK);
        }
    }
    return ret;
}

static void TransferDevice(AMDTPwrDevice* pDevice, osTCPSocketServerConnectionHandler* pConnHandler)
{
    // Note: this is an internally-linked utility sub-routine which assumes that the
    //        pConnHandler is not NULL. Therefore, there is no NULL check in this function.

    // Transfer the data members of this device.
    (*pConnHandler) << pDevice->m_deviceID;
    pConnHandler->writeString(pDevice->m_pDescription);
    pConnHandler->writeString(pDevice->m_pName);
    (*pConnHandler) << static_cast<gtInt32>(pDevice->m_type);

    // Check if this device has sub devices.
    gtUInt32 numOfSubDevices = 0;

    if (pDevice->m_pFirstChild != NULL)
    {
        numOfSubDevices = 1;
    }

    // Tell the client if there are sub devices.
    (*pConnHandler) << numOfSubDevices;

    if (numOfSubDevices > 0)
    {
        // Transfer the sub-devices.
        TransferDevice(pDevice->m_pFirstChild, pConnHandler);
    }

    // Check if this device has succeeding devices.
    gtUInt32 numOfSucceedingDevices = 0;

    if (pDevice->m_pNextDevice != NULL)
    {
        numOfSucceedingDevices = 1;
    }

    // Tell the client if there are succeeding devices.
    (*pConnHandler) << numOfSucceedingDevices;

    if (numOfSucceedingDevices > 0)
    {
        // Transfer the next devices.
        TransferDevice(pDevice->m_pNextDevice, pConnHandler);
    }


}

bool dmnPowerBackendAdapter::handleGetSystemTopologyRequest()
{
    bool ret = false;
    AMDTPwrDevice* pRootDevice = NULL;
    AMDTResult rc = AMDTPwrGetSystemTopology(&pRootDevice);

    // First return the error code.
    gtInt32 rcAsInt = static_cast<gtInt32>(rc);
    (*m_pConnHandler) << rcAsInt;

    GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
    {
        // Now transfer the whole tree structure.
        TransferDevice(pRootDevice, m_pConnHandler);
        ret = true;
    }
    return ret;
}

bool dmnPowerBackendAdapter::handleGetSMinSamplingIntervalMsRequest()
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pConnHandler != NULL)
    {
        // Get the min sampling interval.
        AMDTUInt32 samplingInterval = 0;
        AMDTResult rc = AMDTPwrGetMinimalTimerSamplingPeriod(&samplingInterval);

        // Transfer the return code.
        gtUInt32 retAsUnsignedInt = static_cast<gtUInt32>(rc);
        (*m_pConnHandler) << retAsUnsignedInt;

        // Transfer the min sampling interval.
        gtUInt32 samplingIntervalAsGtUInt32 = static_cast<gtUInt32>(samplingInterval);
        (*m_pConnHandler) << samplingIntervalAsGtUInt32;

        ret = (rc == AMDT_STATUS_OK);
    }
    return ret;
}

bool dmnPowerBackendAdapter::handleGetSCurrentSamplingIntervalMsRequest()
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pConnHandler != NULL)
    {
        // Get the sampling interval.
        AMDTUInt32 samplingInterval = 0;
        AMDTResult rc = AMDTPwrGetTimerSamplingPeriod(&samplingInterval);

        // Transfer the return code.
        gtUInt32 retAsUnsignedInt = static_cast<gtUInt32>(rc);
        (*m_pConnHandler) << retAsUnsignedInt;

        // Transfer the sampling interval.
        gtUInt32 samplingIntervalAsGtUInt32 = static_cast<gtUInt32>(samplingInterval);
        (*m_pConnHandler) << samplingIntervalAsGtUInt32;

        ret = (rc == AMDT_STATUS_OK);
    }
    return ret;
}


bool dmnPowerBackendAdapter::handleEnableCounterRequest()
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pConnHandler != NULL)
    {
        // First get the counter id.
        AMDTUInt32 cid = 0;
        (*m_pConnHandler) >> cid;

        // Enable the counter.
        AMDTResult rc = AMDTPwrEnableCounter(cid);

        // Transfer the return code.
        gtUInt32 retAsUnsignedInt = static_cast<gtUInt32>(rc);
        (*m_pConnHandler) << retAsUnsignedInt;

        ret = (rc == AMDT_STATUS_OK);
    }
    return ret;
}

bool dmnPowerBackendAdapter::handleIsCounterEnabledRequest()
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pConnHandler != NULL)
    {
        // First get the counter id.
        AMDTUInt32 cid = 0;
        (*m_pConnHandler) >> cid;

        // Check if the counter is enabled.
        AMDTResult rc = AMDTPwrIsCounterEnabled(cid);

        // Transfer the return code.
        gtUInt32 retAsUnsignedInt = static_cast<gtUInt32>(rc);
        (*m_pConnHandler) << retAsUnsignedInt;

        ret = (rc == AMDT_STATUS_OK);
    }
    return ret;
}

static bool ExtractTargetAppInfo(osChannel* pConnHandler, osFilePath& targetAppFilePath, osFilePath& targetWorkingDir,
                                 gtString& cmdLineArgs, std::vector<osEnvironmentVariable>& envVars)
{
    // This is a limit for security reasons.
    const gtUInt32 MAX_ENV_VARS_COUNT = 1024;

    bool ret = false;
    GT_IF_WITH_ASSERT(pConnHandler != NULL)
    {
        // Accept the path of the application to be launched.
        gtString targetAppFullPath;
        pConnHandler->readString(targetAppFullPath);

        // Check if the target application exists.
        targetAppFilePath = osFilePath(targetAppFullPath);
        bool isTargetAppExists = targetAppFilePath.exists();
        (*pConnHandler) << isTargetAppExists;

        if (isTargetAppExists)
        {
            // Accept the working directory.
            gtString targetWorkingDirFullPath;
            pConnHandler->readString(targetWorkingDirFullPath);

            // Check if the target working dir exists and respond.
            targetWorkingDir = osFilePath(targetWorkingDirFullPath);
            bool isWorkingDirExists = targetWorkingDir.exists();
            (*pConnHandler) << isWorkingDirExists;

            if (isWorkingDirExists)
            {
                // Accept the command line arguments for the target app.
                pConnHandler->readString(cmdLineArgs);

                // Accept the environment variables.
                gtUInt32 numOfEnvVars = 0;
                (*pConnHandler) >> numOfEnvVars;

                if (numOfEnvVars < MAX_ENV_VARS_COUNT)
                {
                    ret = true;

                    while (ret && numOfEnvVars-- > 0)
                    {
                        gtString _key, _value;
                        ret = pConnHandler->readString(_key);
                        ret = ret && pConnHandler->readString(_value);
                        GT_ASSERT(ret);
                        envVars.push_back(osEnvironmentVariable(_key, _value));
                    }
                }
            }
        }
    }
    return ret;
}

bool dmnPowerBackendAdapter::handleStartPowerProfilingRequest()
{
    bool ret = false;
    osFilePath targetAppFilePath;
    osFilePath targetWorkingDir;
    gtString cmdLineArgs;
    std::vector<osEnvironmentVariable> envVars;

    // Check if we need to launch an application.
    bool shouldLaunchApp = false;
    (*m_pConnHandler) >> shouldLaunchApp;

    if (shouldLaunchApp)
    {
        // Extract the details which are required for the application launch.
        shouldLaunchApp = ExtractTargetAppInfo(m_pConnHandler, targetAppFilePath,
                                               targetWorkingDir, cmdLineArgs, envVars);
    }

    osEnvVarScope environmentVariablesSetter(envVars);

    // Start the profiling session.
    AMDTResult rc = AMDTPwrStartProfiling();

    // Transfer the return code.
    gtUInt32 retAsUnsignedInt = static_cast<gtUInt32>(rc);
    (*m_pConnHandler) << retAsUnsignedInt;
    ret = (rc == AMDT_STATUS_OK);

    if (!ret)
    {
        // Log the error.
        gtString errMsg;
        errMsg << L"AMDTPwrStartProfiling failed with the following error code: " << rc;
        OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
    }

    if (ret && shouldLaunchApp)
    {
        // Launch the target application.
        osProcessHandle procHandle;
        osThreadHandle procThreadHandle;
        ret = osLaunchSuspendedProcess(targetAppFilePath, cmdLineArgs, targetWorkingDir, m_procId, procHandle, procThreadHandle);

        if (ret)
        {
            // Resume the target application.
            ret = osResumeSuspendedProcess(m_procId, procHandle, procThreadHandle, true);

            if (ret)
            {
                // If there is already a watcher thread running, terminate it.
                if (m_pAppWatcherThread != nullptr)
                {
                    delete m_pAppWatcherThread;
                    m_pAppWatcherThread = nullptr;
                }

                // Create the watcher thread. This thread will
                // signal us in case that the target app has stopped.
                m_pAppWatcherThread = new dmnAppWatcherThread(L"DMN LAUNCHED APP WATCHER THREAD", m_procId);

                // Register as an observer of the watcher thread.
                m_pAppWatcherThread->registerToEventNotification(this);

                // Launch the watcher thread.
                bool isWatcherThreadLaunched = m_pAppWatcherThread->execute();
                GT_ASSERT(isWatcherThreadLaunched);
            }
        }
        else
        {
            // Log the error.
            std::wstringstream stream;
            stream << L"Unable to resume a process: " << targetAppFilePath.asString().asCharArray() << std::endl;
            dmnUtils::LogMessage(stream.str(), OS_DEBUG_LOG_ERROR);
        }
    }

    return ret;
}

bool dmnPowerBackendAdapter::handleStopPowerProfilingRequest()
{
    bool ret = false;

    GT_IF_WITH_ASSERT(m_pConnHandler != NULL)
    {
        if (0 != m_procId)
        {
            m_isStopProfile = false;
            m_isTerminationSafe = true;
            // Kill the launched application if required.
            osTerminateProcess(m_procId);
            m_procId = 0;
        }

        // Stop the profiling session.
        gtUInt32 rc = StopPowerProfiling();

        // No session is currently in progress.
        SetRemoteSessionInProgress(false);

        // Transfer the return code.
        (*m_pConnHandler) << rc;
    }

    return ret;
}

bool dmnPowerBackendAdapter::handlePausePowerProfilingRequest()
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pConnHandler != NULL)
    {
        // Pause the profiling session.
        AMDTResult rc = AMDTPwrPauseProfiling();

        // Transfer the return code.
        gtUInt32 retAsUnsignedInt = static_cast<gtUInt32>(rc);
        (*m_pConnHandler) << retAsUnsignedInt;
        ret = (rc == AMDT_STATUS_OK);
    }
    return ret;
}

bool dmnPowerBackendAdapter::handleResumePowerProfilingRequest()
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pConnHandler != NULL)
    {
        // Resume the profiling session.
        AMDTResult rc = AMDTPwrResumeProfiling();

        // Transfer the return code.
        gtUInt32 retAsUnsignedInt = static_cast<gtUInt32>(rc);
        (*m_pConnHandler) << retAsUnsignedInt;
        ret = (rc == AMDT_STATUS_OK);
    }
    return ret;
}

bool dmnPowerBackendAdapter::handleDisableCounterRequest()
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pConnHandler != NULL)
    {
        // First get the counter id.
        AMDTUInt32 cid = 0;
        (*m_pConnHandler) >> cid;

        // Disable the counter.
        AMDTResult rc = AMDTPwrDisableCounter(cid);

        // Transfer the return code.
        gtUInt32 retAsUnsignedInt = static_cast<gtUInt32>(rc);
        (*m_pConnHandler) << retAsUnsignedInt;

        ret = (rc == AMDT_STATUS_OK);
    }
    return ret;
}

bool dmnPowerBackendAdapter::handleClosePowerProfilingSessionRequest()
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pConnHandler != NULL)
    {
        // Perform the close operation..
        AMDTResult rc = AMDTPwrProfileClose();

        // Transfer the return code.
        gtUInt32 retAsUnsignedInt = static_cast<gtUInt32>(rc);
        (*m_pConnHandler) << retAsUnsignedInt;

        ret = (rc == AMDT_STATUS_OK);
    }
    return ret;
}

static void TransferSample(AMDTPwrSample& sample, osTCPSocketServerConnectionHandler* pConnHandler)
{
    // Note: this is an internal function, which is called right
    // after the osTCPSocketServerConnectionHandler pointer has been checked
    // for NULL value, therefore there is NO check for NULL value over here.

    gtUInt64 sysTimeSeconds         = sample.m_systemTime.m_second;
    gtUInt64 sysTimeMicroSeconds    = sample.m_systemTime.m_microSecond;
    gtUInt64 elapsedTimeMs          = sample.m_elapsedTimeMs;
    gtUInt64 recordId               = sample.m_recordId;
    gtUInt32 numOfVals              = sample.m_numOfValues;

    // Transfer the system time in seconds.
    (*pConnHandler) << sysTimeSeconds;

    // Transfer the system time in micro seconds.
    (*pConnHandler) << sysTimeMicroSeconds;

    // Transfer the elapsed time in milliseconds.
    (*pConnHandler) << elapsedTimeMs;

    // Transfer the record id.
    (*pConnHandler) << recordId;

    // Transfer the number of values.
    (*pConnHandler) << numOfVals;

    // Transfer the values.
    AMDTPwrCounterValue* pCurrCounterValue = sample.m_counterValues;

    while (numOfVals-- > 0)
    {
        if (pCurrCounterValue != NULL)
        {
            gtUInt32  cid = pCurrCounterValue->m_counterID;
            gtFloat32 val = pCurrCounterValue->m_counterValue;

            // Transfer the counter id.
            (*pConnHandler) << cid;

            // Transfer the counter value.
            (*pConnHandler) << val;

            // Move to the next element.
            ++pCurrCounterValue;
        }
    }
}

bool dmnPowerBackendAdapter::handleReadAllEnabledCountersRequest()
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pConnHandler != NULL)
    {
        if (IsRemoteSessionInProgress())
        {
            // Read the counters.
            AMDTUInt32 numOfSamples = 0;
            AMDTPwrSample* pSamples = NULL;
            AMDTResult rc = AMDTPwrReadAllEnabledCounters(&numOfSamples, &pSamples);

            // Transfer the return code.
            gtUInt32 retAsUnsignedInt = static_cast<gtUInt32>(rc);
            (*m_pConnHandler) << retAsUnsignedInt;

            if (rc == AMDT_STATUS_OK)
            {
                // Transfer the amount of samples.
                (*m_pConnHandler) << numOfSamples;

                // Transfer the samples.
                AMDTPwrSample* pCurrSample = pSamples;

                while (numOfSamples-- > 0)
                {
                    if (pCurrSample != NULL)
                    {
                        TransferSample(*pCurrSample, m_pConnHandler);
                    }

                    // Advance the pointer.
                    ++pCurrSample;
                }

                ret = true;
            }
            else
            {
                // Log the error.
                gtString errMsg;
                errMsg << L"AMDTPwrReadAllEnabledCounters failed with the following error code: " << rc;
                OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
            }
        }
        else
        {
            // Transfer the code that signals that the session is about to terminate itself.
            gtUInt32 retAsUnsignedInt = static_cast<gtUInt32>(DMN_SELF_TERMINATION_CODE);
            (*m_pConnHandler) << retAsUnsignedInt;

            // Signal that it is safe to terminate this session.
            m_isTerminationSafe = true;
        }
    }
    return ret;
}

bool dmnPowerBackendAdapter::handleGetDeviceCountersRequest()
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pConnHandler != NULL)
    {
        // First get the device id.
        gtUInt32 deviceIdAsGtUint = 0;
        (*m_pConnHandler) >> deviceIdAsGtUint;
        AMDTPwrDeviceId deviceId = static_cast<AMDTPwrDeviceId>(deviceIdAsGtUint);

        // Get the device's counters.
        AMDTUInt32 numOfSupportedCounters = 0;
        AMDTPwrCounterDesc* pSupportedCounters = NULL;
        AMDTResult rc = AMDTPwrGetDeviceCounters(deviceId, &numOfSupportedCounters, &pSupportedCounters);

        // Transfer the return code.
        gtUInt32 retAsUnsignedInt = static_cast<gtUInt32>(rc);
        (*m_pConnHandler) << retAsUnsignedInt;

        if (rc == AMDT_STATUS_OK)
        {
            // Transfer the number of supported counters.
            gtUInt32 numOfSupportedCountersAsGtUint = static_cast<gtUInt32>(numOfSupportedCounters);
            (*m_pConnHandler) << numOfSupportedCountersAsGtUint;

            // Transfer the counters.
            AMDTPwrCounterDesc* pCurrCounter = pSupportedCounters;

            while (numOfSupportedCountersAsGtUint-- > 0)
            {
                // Note: the data validity should have already been checked in the backend.
                //       Therefore, there is no check for NULL value before dereferencing the pointer.

                // Transfer the counter id.
                gtUInt32 counterId = pCurrCounter->m_counterID;
                (*m_pConnHandler) << counterId;

                // Transfer the device id.
                gtUInt32 counterDeviceId = pCurrCounter->m_deviceId;
                (*m_pConnHandler) << counterDeviceId;

                // Transfer the counter name.
                m_pConnHandler->writeString(pCurrCounter->m_name);

                // Transfer the counter description.
                m_pConnHandler->writeString(pCurrCounter->m_description);

                // Transfer the counter category.
                gtInt32 counterCategoryAsGtInt = static_cast<gtInt32>(pCurrCounter->m_category);
                (*m_pConnHandler) << counterCategoryAsGtInt;

                // Transfer the counter aggregation.
                gtInt32 counterAggregationAsGtInt = static_cast<gtInt32>(pCurrCounter->m_aggregation);
                (*m_pConnHandler) << counterAggregationAsGtInt;

                // Transfer the counter units.
                gtInt32 counterUnitsAsGtInt = static_cast<gtInt32>(pCurrCounter->m_units);
                (*m_pConnHandler) << counterUnitsAsGtInt;

                // Advance the pointer.
                ++pCurrCounter;
            }

            ret = true;
        }
        else
        {
            // Log the error.
            gtString errMsg;
            errMsg << L"AMDTPwrGetDeviceCounters failed with the following error code: " << rc <<
                   L", Device ID is: " << deviceId;
            OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
        }
    }
    return ret;
}

void dmnPowerBackendAdapter::onAppTerminated()
{
    // No session is currently in progress.
    SetRemoteSessionInProgress(false);

    // Wait until the client has been notified.
    const unsigned MAX_NUM_OF_ATTEMPTS = 25;
    unsigned numOfAttempts = 0;

    while (!m_isTerminationSafe && (numOfAttempts++ < MAX_NUM_OF_ATTEMPTS))
    {
        // Mitigate the busy-waiting period.
        const unsigned SLEEP_INTERVAL = 200;
        osSleep(SLEEP_INTERVAL);
    }

    if (m_isStopProfile)
    {
        // Since the launched app is terminated, stop the profiling
        StopPowerProfiling();
    }

    // Reset the process ID member to avoid
    // trying to kill the process which is already dead.
    m_procId = 0;
}

gtUInt32 dmnPowerBackendAdapter::StopPowerProfiling()
{
    AMDTResult rc = AMDTPwrStopProfiling();

    if (AMDT_STATUS_OK != rc)
    {
        // Log the error.
        gtString errMsg;
        errMsg << L"AMDTPwrStopProfiling failed with the following error code: " << rc;
        OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
    }

    // Close the session.
    AMDTPwrProfileClose();

    return static_cast<gtUInt32>(rc);
}
