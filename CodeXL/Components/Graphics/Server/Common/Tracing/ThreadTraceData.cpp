//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file ThreadTraceData.cpp
/// \brief The ThreadTraceData type is used in recording every API call that
/// occurs in a single thread of execution within an instrumented application.
//==============================================================================

#include "ThreadTraceData.h"
#include "APIEntry.h"

//--------------------------------------------------------------------------
/// Default constructor.
//--------------------------------------------------------------------------
ThreadTraceData::ThreadTraceData()
{
    // Initialize this to known garbage so we can check it later. It should *always* be overwritten by real data.
#ifdef _WIN32
    m_startTime.QuadPart = s_DummyTimestampValue;
#else
    m_startTime = s_DummyTimestampValue;
#endif

    // We should expect a lot of entries within this structure, so make it large enough up front.
    // @PERFORMANCE @TODO: Do we really need to reserve space for APIEntries?
    mLoggedCallVector.reserve(2048);
}

//--------------------------------------------------------------------------
/// Destructor clears any remaining buffered data before the instance dies.
//--------------------------------------------------------------------------
ThreadTraceData::~ThreadTraceData()
{
    Clear();
}

//--------------------------------------------------------------------------
/// Insert the latest API call information into our list of traced calls.
/// \param inStartTime The timestamp collected directly before the traced API call.
/// \param inNewEntry An APIEntry instance containing the details of the traced call.
//--------------------------------------------------------------------------
void ThreadTraceData::AddAPIEntry(GPS_TIMESTAMP inStartTime, APIEntry* inNewEntry)
{
    // Insert a new logged duration into the call timer.
    mAPICallTimer.Add(inNewEntry->mThreadId, inStartTime);

    // Now insert the APIEntry into the list of traced API calls for this thread.
    mLoggedCallVector.push_back(inNewEntry);
}

//--------------------------------------------------------------------------
/// Clear all logged data in the thread's collection buffer.
//--------------------------------------------------------------------------
void ThreadTraceData::Clear()
{
    // Invalidate all entries from the previous run.
    for (size_t callIndex = 0; callIndex < mLoggedCallVector.size(); ++callIndex)
    {
        // This instance isn't needed anymore, so destroy it and reclaim pool memory.
        APIEntry* thisEntry = mLoggedCallVector[callIndex];
        SAFE_DELETE(thisEntry);
    }

    mLoggedCallVector.clear();
    mAPICallTimer.Clear();
}