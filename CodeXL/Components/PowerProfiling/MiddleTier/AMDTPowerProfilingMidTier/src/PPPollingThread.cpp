//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PPPollingThread.cpp
///
//==================================================================================

#include <AMDTPowerProfilingMidTier/include/PPPollingThread.h>

// Infra.
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// C++.
#include <memory>

// Remote.
#include <AMDTRemoteClient/Include/CXLDaemonClient.h>

// The following value should be the same as PP_MAX_SAMPLING_INTERVAL defined in ppAppController.h
#define MAX_POLLING_INTERVAL_MILLISECONDS 2000

PPPollingThread::PPPollingThread(unsigned pollingInterval, PPSamplesDataHandler cb, void* pDataCbParams, PPFatalErrorHandler cbErr, void* pErrorCbParams, IPowerProfilerBackendAdapter* pBeAdapter, amdtProfileDbAdapter* pDataAdapter)
    : osThread(L"PowerProfilingPollingThread")
      // Verify that the polling interval is valid
    , m_pollingInterval(pollingInterval > 0 && pollingInterval <= MAX_POLLING_INTERVAL_MILLISECONDS ? pollingInterval : 100)
    , m_quantizedTime(0)
    , m_dataCb(cb)
    , m_pDataCbParams(pDataCbParams)
    , m_errorCb(cbErr)
    , m_pErrorCbParams(pErrorCbParams)
    , m_pBeAdapter(pBeAdapter)
    , m_pDataAdapter(pDataAdapter)
    , m_sessionStartedStatus(PPR_COMMUNICATION_FAILURE)
    , m_targetAppLaunchStatus(rasUnknown)
    , m_profilingErr(false)
    , m_profResult(PPR_NO_ERROR)
{
    GT_ASSERT_EX((pollingInterval > 0 && pollingInterval <= MAX_POLLING_INTERVAL_MILLISECONDS), L"Invalid PP Polling interval");
    GT_ASSERT_EX((m_dataCb != NULL), L"Invalid PPSamplesDataHandler");
    GT_ASSERT_EX((m_errorCb != NULL), L"Invalid PPFatalErrorHandler");
}

PPPollingThread::~PPPollingThread()
{
}

static void CreateCounterToSampleMap(const AMDTProfileTimelineSample& beSample, gtMap<int, PPSampledValuesBatch>& outMap)
{
    const gtVector<AMDTProfileCounterValue>& beValsVector = beSample.m_sampleValues;
    const AMDTUInt32 elapsedTimeMs = (AMDTUInt32)beSample.m_sampleElapsedTimeMs;

    for (size_t i = 0; i < beValsVector.size(); ++i)
    {
        AMDTUInt32 currCounterId = beValsVector[i].m_counterId;
        auto iter = outMap.find(currCounterId);

        if (iter == outMap.end())
        {
            // If we don't yet have a batch for that counter id, let's create it.
            outMap[currCounterId] = PPSampledValuesBatch(elapsedTimeMs);
            iter = outMap.find(currCounterId);
        }

        // Add the value to the counter's sampled values batch.
        outMap[currCounterId].m_sampleValues.push_back(beValsVector[i].m_counterValue);
    }
}

int PPPollingThread::entryPoint()
{
    GT_IF_WITH_ASSERT((m_pDataAdapter != NULL) && (m_pBeAdapter != NULL) && (m_dataCb != NULL))
    {
        // Start the actual profiling.
        PPResult rc = m_pBeAdapter->StartProfiling();

        // Retrieve the application launch status.
        m_targetAppLaunchStatus = m_pBeAdapter->GetApplicationLaunchStatus();

        // Assign the session start status.
        m_sessionStartedStatus = rc;

        GT_IF_WITH_ASSERT(rc < PPR_FIRST_ERROR)
        {
            // A buffer to hold the samples.
            gtVector<AMDTProfileTimelineSample*> samplesBuffer;

            // Num of ticks.
            unsigned numOfTicks = 1;

            // Calculate when we need to flush the DB data.
            const unsigned DB_REFRESH_TIME_MS = 2000;
            unsigned timeFactor = DB_REFRESH_TIME_MS / m_pollingInterval;

            if (timeFactor == 0)
            {
                timeFactor = 3;
            }
            else
            {
                timeFactor += 30;
            }

            // Poll for new data until we get killed.
            while (m_IsStopped == false)
            {
                // Sleep for the polling interval.
                osSleep(m_pollingInterval);

                // Read the value of the sampled counters.
                samplesBuffer.clear();
                rc = m_pBeAdapter->ReadAllEnabledCounters(samplesBuffer);

                // Time tick.
                m_quantizedTime += m_pollingInterval;
                ++numOfTicks;

                if (rc < PPR_FIRST_ERROR)
                {
                    if (m_pDataAdapter != NULL)
                    {
                        // Insert the data to the DB.
                        m_pDataAdapter->InsertSamples(samplesBuffer);

                        if (numOfTicks % timeFactor == 0)
                        {
                            m_pDataAdapter->FlushDbAsync();
                        }
                    }

                    size_t numOfSamples = samplesBuffer.size();

                    for (size_t i = 0; i < numOfSamples; ++i)
                    {
                        const AMDTProfileTimelineSample* pCurrBeSample = samplesBuffer[i];

                        GT_IF_WITH_ASSERT(pCurrBeSample != NULL)
                        {
                            // Create a map to hold the sample values by counter id.
                            std::shared_ptr<gtMap<int, PPSampledValuesBatch>> pCounterIdToSampleMap(new gtMap<int, PPSampledValuesBatch>());

                            // Fill the counter to sample map.
                            CreateCounterToSampleMap(*pCurrBeSample, *pCounterIdToSampleMap);

                            // Raise the event.
                            m_dataCb(pCounterIdToSampleMap, m_pDataCbParams);
                        }
                    }

                    for (auto it : samplesBuffer)
                    {
                        delete it;
                    }

                }
                else
                {
                    // Log this event.
                    gtString errMsg(L"Error occurred when reading all enabled counters, error code is: ");
                    errMsg << static_cast<unsigned>(rc);
                    OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);

                    if ((m_errorCb != nullptr) &&
                        ((PPR_COMMUNICATION_FAILURE == rc) || (rc == PPR_REMOTE_APP_STOPPED)))
                    {
                        m_profilingErr = true;
                        m_profResult = rc;
                        m_IsStopped = true;
                    }
                }
            }//while (m_IsStopped == false)
        }
    }

    return 0;
}

void PPPollingThread::beforeTermination()
{
    if ((true == m_profilingErr) && (nullptr != m_errorCb))
    {
        if (m_profResult == PPR_COMMUNICATION_FAILURE)
        {
            // This is a communication failure, notify the system.
            m_errorCb(m_profResult, m_pDataCbParams);
        }
        else if (m_profResult == PPR_REMOTE_APP_STOPPED)
        {
            // Log this event.
            gtString errMsg2(L"Remote app stopped.");
            errMsg2 << static_cast<unsigned>(m_profResult);
            OS_OUTPUT_DEBUG_LOG(errMsg2.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);

            // Notify the system.
            m_errorCb(m_profResult, m_pDataCbParams);
        }
    }
}
