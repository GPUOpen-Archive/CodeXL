//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Records the timing values for the API trace.
//==============================================================================

#ifndef _GPS_TIMING_LOG_H_
#define _GPS_TIMING_LOG_H_

#include <vector>
#include <sstream>
#include "misc.h"
#include "timer.h"

#if defined (_LINUX)
    #include <sys/time.h>
#endif // _LINUX

/// Strores the threadID and atart and end times of an API call.
struct CallsTiming
{
    UINT32 m_ThreadID; ///< Thread ID
    GPS_TIMESTAMP m_startTime; ///< Start time
    GPS_TIMESTAMP m_endTime; ///< End time
};

/// Records the timing values for the API trace.
class TimingLog
{

private:

    std::vector<CallsTiming> m_TimingLog; ///< Stores the API entries

    Timer m_cpuTimer; ///< Timer used to set the timings in the before and after data stores

public:

    //--------------------------------------------------------------------------
    /// Retrieve a CallsTiming instance by index in the TimingLog.
    /// \param inIndex The index to use when looking up a CallsTiming instance.
    /// \returns A CallsTiming structure containing timing information for a call.
    //--------------------------------------------------------------------------
    const CallsTiming& GetTimingByIndex(size_t inIndex) const
    {
        PsAssert(inIndex < m_TimingLog.size());
        return m_TimingLog[inIndex];
    }

    /// Gets the number of API entries in the store.
    /// \return The API count
    UINT32 Size()
    {
        return (UINT32)m_TimingLog.size();
    }

    /// Get the raw timing value from the internal CPU timer.
    /// \return The current timestamp value.
    GPS_TIMESTAMP GetRaw()
    {
        return m_cpuTimer.GetRaw();
    }

    /// Add a new itme to the log.
    void Add(UINT32 thread, GPS_TIMESTAMP startTime)
    {
        CallsTiming ct;

        ct.m_ThreadID = thread;
        ct.m_startTime = startTime;
        ct.m_endTime = m_cpuTimer.GetRaw();

        m_TimingLog.push_back(ct);
    }

    //--------------------------------------------------------------------------
    /// Helper to make the secret conversion formula for converting timestamps to nanosecond values public.
    /// This is necessary where the TimingLog is used, but the resultant string is not.
    /// \param inStart The GPS_TIMESTAMP collected before the call was executed.
    /// \param inEnd The GPS_TIMESTAMP collected after the call was executed.
    /// \param outDeltaStartTime The start timestamp converted to nanoseconds.
    /// \param outDeltaEndTime The end timestamp converted to nanoseconds.
    /// \param inFrameStart The start timestamp
    /// \param inTimeFrequency An optional parameter containing the counter frequency. Only necessary and not-NULL under Windows.
    //--------------------------------------------------------------------------
    bool ConvertTimestampToDoubles(const GPS_TIMESTAMP& inStart, const GPS_TIMESTAMP& inEnd,
                                   double& outDeltaStartTime, double& outDeltaEndTime,
                                   const GPS_TIMESTAMP& inFrameStart, GPS_TIMESTAMP* inTimeFrequency = NULL) const
    {
#if defined (_WIN32)

        if (inTimeFrequency == NULL)
        {
            // If the incoming frequency is null, we're going to have show-stopping problems. Just return false.
            return false;
        }

        double dTimeFrequency = (double)inTimeFrequency->QuadPart;
        outDeltaStartTime = (double)((inStart.QuadPart - inFrameStart.QuadPart) * 1000000000.0) / dTimeFrequency;
        outDeltaEndTime = (double)((inEnd.QuadPart - inFrameStart.QuadPart) * 1000000000.0) / dTimeFrequency;
#else
        /// Convert timings to nanoseconds. Timing functions on Linux use clock_gettime which has nanosecond precision.
        PS_UNREFERENCED_PARAMETER(inTimeFrequency);
        outDeltaStartTime = (double)(inStart - inFrameStart);
        outDeltaEndTime = (double)(inEnd - inFrameStart);
#endif // _WIN32

        /*
        // @TIMINGLOGCHANGE - This is included here to check if the values are way out of the expected range of valid values.
        if ((outDeltaStartTime < 0.0) || (outDeltaEndTime < 0.0f))
        {
           printf("stop");
        }

        if ((outDeltaStartTime > 80000000.0) || (outDeltaEndTime > 80000000.0))
        {
           printf("stop");
        }
        */

        return true;
    }

    //--------------------------------------------------------------------------
    /// Get a GPS_TIMESTAMP view of the time frequency for this performance counter.
    /// \return The time ferquency.
    //--------------------------------------------------------------------------
    GPS_TIMESTAMP GetTimeFrequency() const
    {
#if defined (_WIN32)
        GPS_TIMESTAMP TimeFrequency;
        QueryPerformanceFrequency(&TimeFrequency);
        return TimeFrequency;
#elif defined (_LINUX)
        // the Linux equivalent of QueryPerformanceCounter used in PerfStudio is
        // clock_getTime(), which has a fixed frequency of 1 nanosecond
        return 1000000000;
#else
#error IMPLEMENT ME!
#endif // _WIN32
    }

    /// Convert the log timing information as a string. The string is space-delimited
    /// and consists of 3 values; a thread ID (signed int) and a start and end time.
    /// Both timing values are double-precision floats, in nanoseconds
    /// \return The log as one giant string.
    std::string GetLogAsString()
    {
        std::stringstream timing;

        GPS_TIMESTAMP* pTimeFrequency = NULL;
#if defined (_WIN32)
        GPS_TIMESTAMP timeFrequency = GetTimeFrequency();
        pTimeFrequency = &timeFrequency;
#elif defined (_LINUX)
        // nothing to do here.
#else
#error IMPLEMENT ME!
#endif // _WIN32

        if (m_TimingLog.empty() == false)
        {
            GPS_TIMESTAMP frameStart = m_TimingLog[0].m_startTime;

            for (size_t i = 1; i < m_TimingLog.size(); i++)
            {
                GPS_TIMESTAMP startTime = m_TimingLog[i].m_startTime;
                GPS_TIMESTAMP endTime = m_TimingLog[i].m_endTime;

                double deltaStartTime, deltaEndTime;
                ConvertTimestampToDoubles(startTime, endTime, deltaStartTime, deltaEndTime, frameStart, pTimeFrequency);

                timing << m_TimingLog[i].m_ThreadID << " " << deltaStartTime << " " << deltaEndTime << std::endl;
            }
        }

        return timing.str();
    }

    /// Removes all items from the store.
    void Clear()
    {
        m_TimingLog.clear();
    }

};

#endif //_GPS_TIMING_LOG_H_
