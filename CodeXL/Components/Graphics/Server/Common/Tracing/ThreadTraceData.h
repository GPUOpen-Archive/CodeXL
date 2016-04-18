//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file ThreadTraceData.h
/// \brief The ThreadTraceData type is used in recording every API call that
/// occurs in a single thread of execution within an instrumented application.
//==============================================================================

#ifndef THREADTRACEDATA_H
#define THREADTRACEDATA_H

#include "../CommonTypes.h"
#include "../TimingLog.h"
#include <vector>

enum FuncId : int;
    class APIEntry;

    //--------------------------------------------------------------------------
    /// A magic number that we use to initialize timestamps. If we find this in
    /// a timestamp variable, we failed to populate it correctly.
    //--------------------------------------------------------------------------
    static const uint64 s_DummyTimestampValue = 666;

    //--------------------------------------------------------------------------
    /// A buffer that can be used on a per-thread basis to log function calls without
    /// having to deal with locking a single buffer and serializing the timing.
    //--------------------------------------------------------------------------
    class ThreadTraceData
{
public:
    //--------------------------------------------------------------------------
    /// Default constructor.
    //--------------------------------------------------------------------------
    ThreadTraceData();

    //--------------------------------------------------------------------------
    /// Destructor clears any remaining buffered data before the instance dies.
    //--------------------------------------------------------------------------
    virtual ~ThreadTraceData();

    //--------------------------------------------------------------------------
    /// Insert the latest API call information into our list of traced calls.
    /// \param inStartTime The timestamp collected directly before the traced API call.
    /// \param inNewEntry An APIEntry instance containing the details of the traced call.
    //--------------------------------------------------------------------------
    void AddAPIEntry(GPS_TIMESTAMP inStartTime, APIEntry* inNewEntry);

    //--------------------------------------------------------------------------
    /// Clear all logged data in the thread's collection buffer.
    //--------------------------------------------------------------------------
    void Clear();

    //--------------------------------------------------------------------------
    /// The start time used in computing the total duration of a logged call.
    //--------------------------------------------------------------------------
    GPS_TIMESTAMP m_startTime;

    //--------------------------------------------------------------------------
    /// An array of logged timings that can be linked to a logged call.
    //--------------------------------------------------------------------------
    TimingLog mAPICallTimer;

    //--------------------------------------------------------------------------
    /// Keep a list of InvocationData structure instances to keep track of CPU calls.
    //--------------------------------------------------------------------------
    std::vector<APIEntry*> mLoggedCallVector;
};

#endif // THREADTRACEDATA_H