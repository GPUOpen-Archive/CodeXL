//=============================================================
// (c) 2017 Advanced Micro Devices, Inc.
//
/// \author CodeXL Developer Tools
/// \brief  Example program using the AMDTPowerProfile APIs.
//
//=============================================================

// This sample shows the code for:
// - Initializing the AMDTPwrProfile API in online mode
// - Get the number of available counters and enable all the counters
// - Start the profiling
// - Periodically read the counter values and report till the user has requested to stop

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include <AMDTPowerProfileApi.h>

void GetTimeStampString(AMDTPwrSystemTime& sampleTime, AMDTUInt64 elapsedMs, char* pTimeStr)
{
#define WINDOWS_TICK_PER_SEC     10000000
#define MICROSEC_IN_SECOND       1000000

#if defined ( WIN32 )
    ULARGE_INTEGER time;

    // Convert sample time to 100-nanosec
    time.QuadPart = (sampleTime.m_second * WINDOWS_TICK_PER_SEC) + (sampleTime.m_microSecond * 10);

    // adjust the absolute profile start TS with elapsed time (in ms)
    time.QuadPart += elapsedMs * 10000;

    FILETIME fileTime;
    fileTime.dwHighDateTime = (DWORD)(time.HighPart);
    fileTime.dwLowDateTime = (DWORD)(time.LowPart);

    SYSTEMTIME sysTime;

    if (FileTimeToSystemTime(&fileTime, &sysTime))
    {
        sprintf(pTimeStr, "%d:%d:%d:%03d", sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
    }

#else
    struct timeval ts;
    struct tm time;
    AMDTUInt64 tmp = 0;

    ts.tv_sec = sampleTime.m_second;
    ts.tv_usec = sampleTime.m_microSecond;

    tmp = ts.tv_usec + (elapsedMs * 1000);
    // when tmp > 1000000 usec add to seconds
    ts.tv_sec += tmp / MICROSEC_IN_SECOND;
    ts.tv_usec = tmp % MICROSEC_IN_SECOND;
    tzset();
    localtime_r(&(ts.tv_sec), &time);

    sprintf(pTimeStr, "%d:%d:%d:%03lu", time.tm_hour, time.tm_min, time.tm_sec, ts.tv_usec / (1000));
#endif
}

void CollectAllCounters()
{
    AMDTResult hResult = AMDT_STATUS_OK;

    // Initialize online mode
    hResult = AMDTPwrProfileInitialize(AMDT_PWR_PROFILE_MODE_ONLINE);
    // --- Handle the error

    // Configure the profile run
    //   1. Get the supported counters
    //   2. Enable all the counter
    //   3. Set the timer configuration

    // 1. Get the supported counter details
    AMDTUInt32 nbrCounters = 0;
    AMDTPwrCounterDesc* pCounters = NULL;
    AMDTPwrDeviceId deviceId = AMDT_PWR_ALL_DEVICES;

    hResult = AMDTPwrGetDeviceCounters(deviceId, &nbrCounters, &pCounters);
    assert(AMDT_STATUS_OK == hResult);

    // Enable all the counters
    hResult = AMDTPwrEnableAllCounters();
    assert(AMDT_STATUS_OK == hResult);

    // Set the timer configuration
    AMDTUInt32 samplingInterval = 100;      // in milliseconds
    AMDTUInt32 profilingDuration = 10;      // in seconds

    hResult = AMDTPwrSetTimerSamplingPeriod(samplingInterval);
    assert(AMDT_STATUS_OK == hResult);

    // Start the Profile Run
    hResult = AMDTPwrStartProfiling();
    assert(AMDT_STATUS_OK == hResult);

    // Collect and report the counter values periodically
    //   1. Take the snapshot of the counter values
    //   2. Read the counter values
    //   3. Report the counter values

    volatile bool isProfiling = true;
    bool stopProfiling = false;
    AMDTUInt32 nbrSamples = 0;

    while (isProfiling)
    {
        // sleep for refresh duration - at least equivalent to the sampling interval specified
#if defined ( WIN32 )
        // Windows
        Sleep(samplingInterval);
#else
        // Linux
        usleep(samplingInterval * 1000);
#endif

        // read all the counter values
        AMDTPwrSample* pSampleData = nullptr;

        hResult = AMDTPwrReadAllEnabledCounters(&nbrSamples, &pSampleData);

        if (AMDT_STATUS_OK != hResult)
        {
            continue;
        }

        if (nullptr != pSampleData)
        {
            // iterate over all the samples and report the sampled counter values
            for (AMDTUInt32 idx = 0; idx < nbrSamples; idx++)
            {
                // Iterate over the sampled counter values and print
                for (unsigned int i = 0; i < pSampleData[idx].m_numOfValues; i++)
                {
                    if (nullptr != pSampleData[idx].m_counterValues)
                    {
                        // Get the counter descriptor to print the counter name
                        AMDTPwrCounterDesc counterDesc;
                        AMDTPwrGetCounterDesc(pSampleData[idx].m_counterValues->m_counterID, &counterDesc);

                        fprintf(stdout, "%s : %f ", counterDesc.m_name, pSampleData[idx].m_counterValues->m_counterValue);

                        pSampleData[idx].m_counterValues++;
                    }
                } // iterate over the sampled counters

                fprintf(stdout, "\n");
            } // iterate over all the samples collected

            // check if we exceeded the profile duration
            if ((profilingDuration > 0)
                && (pSampleData->m_elapsedTimeMs >= (profilingDuration * 1000)))
            {
                stopProfiling = true;
            }

            if (stopProfiling)
            {
                // stop the profiling
                hResult = AMDTPwrStopProfiling();
                assert(AMDT_STATUS_OK == hResult);
                isProfiling = false;
            }
        }
    }

    // Close the profiler
    hResult = AMDTPwrProfileClose();
    assert(AMDT_STATUS_OK == hResult);
}

int main()
{
    AMDTResult hResult = AMDT_STATUS_OK;
    CollectAllCounters();
    return hResult;
}
