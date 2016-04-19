//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file ModernAPIFrameProfilerLayer.cpp
/// \brief The baseclass Frame Profiler layer implementation for modern APIs.
//==============================================================================

#include "ModernAPIFrameProfilerLayer.h"
#include "SharedGlobal.h"

/// The SampleId to start at when profiling API calls.
static const UINT64 FIRST_GPA_SAMPLE_ID = 0;

/// A magic number that indicates a "session not started" state for GPA.
static const gpa_uint32 INVALID_GPA_SESSION_ID = 0x1337;

//--------------------------------------------------------------------------
/// Default constructor for ModernAPIFrameProfilerLayer.
//--------------------------------------------------------------------------
ModernAPIFrameProfilerLayer::ModernAPIFrameProfilerLayer()
    : mCurrentProfilingSessionId(INVALID_GPA_SESSION_ID)
    , mSampleIndex(FIRST_GPA_SAMPLE_ID)
    , mbProfilerEnabled(false)
{
}

//--------------------------------------------------------------------------
/// Default destructor for ModernAPIFrameProfilerLayer.
//--------------------------------------------------------------------------
ModernAPIFrameProfilerLayer::~ModernAPIFrameProfilerLayer()
{
}

//--------------------------------------------------------------------------
/// A logging callback for use with GPA. When GPA logs a message, it is sent here.
/// \param messageType The type of message being logged.
/// \param message The message string to be logged.
//--------------------------------------------------------------------------
void ModernAPIFrameProfilerLayer::GPALoggingCallback(GPA_Logging_Type messageType, const char* message)
{
    // Dump a message with the correct GPS log type to match GPA. Fall back to logRAW if necessary.
    LogType logType = (messageType == GPA_LOGGING_ERROR) ? logERROR : ((messageType == GPA_LOGGING_MESSAGE) ? logMESSAGE : ((messageType == GPA_LOGGING_TRACE) ? logTRACE : logRAW));
    Log(logType, "GPA: %s\n", message);
}

//--------------------------------------------------------------------------
/// Lock GPA access mutex
//--------------------------------------------------------------------------
void ModernAPIFrameProfilerLayer::LockGPA()
{
    mGPAPrePostMatcherMutex.Lock();
}

//--------------------------------------------------------------------------
/// Unlock GPA access mutex
//--------------------------------------------------------------------------
void ModernAPIFrameProfilerLayer::UnlockGPA()
{
    mGPAPrePostMatcherMutex.Unlock();
}

//--------------------------------------------------------------------------
/// Initialize GPA.
/// \param inAPI The API being initialized.
/// \returns True if GPA initialization was successful.
//--------------------------------------------------------------------------
bool ModernAPIFrameProfilerLayer::InitializeGPA(GPA_API_Type inAPI)
{
    const char* errorMessage = NULL;
    bool bLoadSuccessful = mGPALoader.Load(SG_GET_PATH(GPUPerfAPIPath), inAPI, &errorMessage);

    if (!bLoadSuccessful)
    {
        Log(logERROR, "Failed to load GPA. Load error: %s\n", errorMessage);
    }
    else
    {
        if (mGPALoader.GPA_RegisterLoggingCallback(GPA_LOGGING_ERROR_AND_MESSAGE, (GPA_LoggingCallbackPtrType)&ModernAPIFrameProfilerLayer::GPALoggingCallback) != GPA_STATUS_OK)
        {
            Log(logERROR, "Failed to register profiler logging callback.\n");
        }
    }

    return bLoadSuccessful;
}

//--------------------------------------------------------------------------
/// Shutdown GPA.
/// \returns True if shutdown was successful.
//--------------------------------------------------------------------------
bool ModernAPIFrameProfilerLayer::ShutdownGPA()
{
    if (mGPALoader.Loaded())
    {
        mGPALoader.Unload();
    }
    else
    {
        Log(logERROR, "Attempted to shutdown GPA, but it hadn't already been loaded successfully.\n");
    }

    return true;
}

//--------------------------------------------------------------------------
/// Reset the session Id used for profiling with GPA.
//--------------------------------------------------------------------------
void ModernAPIFrameProfilerLayer::ResetGPASession()
{
    mCurrentProfilingSessionId = INVALID_GPA_SESSION_ID;
}

//--------------------------------------------------------------------------
/// Reset the Sample Id for the next frame.
//--------------------------------------------------------------------------
void ModernAPIFrameProfilerLayer::ResetSampleIdCounter()
{
    // Probably don't need to lock, but doesn't hurt.
    ScopeLock sampleLock(&mUniqueSampleIdMutex);
    mSampleIndex = FIRST_GPA_SAMPLE_ID;

    // Destroy all SampleInfo instances and clear the map.
    ThreadIdToSampleIdMap::iterator sampleInfoIter;

    for (sampleInfoIter = mSampleIdMap.begin(); sampleInfoIter != mSampleIdMap.end(); ++sampleInfoIter)
    {
        SampleInfo* sampleInfo = sampleInfoIter->second;
        SAFE_DELETE(sampleInfo);
    }

    mSampleIdMap.clear();
}

//--------------------------------------------------------------------------
/// Given a sample info, reserve the next available SampleId for the call that's about to happen in the thread.
/// \param inSampleInfo The Id of the thread where the profiled function is about to be invoked.
/// \returns A new unique Sample Id to be used in profiling with GPA.
//--------------------------------------------------------------------------
UINT64 ModernAPIFrameProfilerLayer::SetNextSampleId(SampleInfo* inSampleInfo)
{
    UINT64 nextId = GetNextSampleId();

    if (inSampleInfo != nullptr)
    {
        inSampleInfo->mSampleId = nextId;
        inSampleInfo->mbBeginSampleSuccessful = false;
    }

    return nextId;
}

//--------------------------------------------------------------------------
/// Retrieve an existing SampleInfo instance, or create a new one, for the given ThreadId.
/// \param inThreadId The current thread requesting a SampleInfo structure.
/// \returns A SampleInfo instance to be used with the given ThreadId.
//--------------------------------------------------------------------------
ModernAPIFrameProfilerLayer::SampleInfo* ModernAPIFrameProfilerLayer::GetSampleInfoForThread(DWORD inThreadId)
{
    SampleInfo* pResult = nullptr;

    if (mSampleIdMap.find(inThreadId) != mSampleIdMap.end())
    {
        // mSampleIdMap already has a key for the incoming thread. Update the current sampleInfo.
        pResult = mSampleIdMap[inThreadId];
    }
    else
    {
        // We need to insert a new key into the mSampleIdMap. Need to lock it first.
        ScopeLock sampleLock(&mUniqueSampleIdMutex);

        pResult = new SampleInfo;

        mSampleIdMap[inThreadId] = pResult;
    }

    PsAssert(pResult != nullptr);

    return pResult;
}

//--------------------------------------------------------------------------
/// Retrieve the next available unique Sample Id.
/// \returns A unique Sample Id to be used with GPA.
//--------------------------------------------------------------------------
UINT64 ModernAPIFrameProfilerLayer::GetNextSampleId()
{
    ScopeLock sampleLock(&mUniqueSampleIdMutex);
    mSampleIndex++;
    return mSampleIndex;
}