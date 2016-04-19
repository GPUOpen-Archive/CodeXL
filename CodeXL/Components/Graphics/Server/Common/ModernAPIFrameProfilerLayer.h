//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file ModernAPIFrameProfilerLayer.h
/// \brief The baseclass Frame Profiler layer implementation for modern APIs.
//==============================================================================

#ifndef MODERNAPIFRAMEPROFILERLAYER_H
#define MODERNAPIFRAMEPROFILERLAYER_H

#include "IModernAPILayer.h"
#include "CommandProcessor.h"
#include "CommonTypes.h"
#include <GPUPerfAPI.h>
#include <GPUPerfAPILoader.h>
#include <unordered_set>
#include <unordered_map>

//--------------------------------------------------------------------------
/// The baseclass Frame Profiler layer implementation for modern APIs.
//--------------------------------------------------------------------------
class ModernAPIFrameProfilerLayer : public IModernAPILayer, public CommandProcessor
{
public:
    //--------------------------------------------------------------------------
    /// Default constructor for ModernAPIFrameProfilerLayer.
    //--------------------------------------------------------------------------
    ModernAPIFrameProfilerLayer();

    //--------------------------------------------------------------------------
    /// Default destructor for ModernAPIFrameProfilerLayer.
    //--------------------------------------------------------------------------
    virtual ~ModernAPIFrameProfilerLayer();

    //--------------------------------------------------------------------------
    /// Allows derived classes to add additional settings.
    //--------------------------------------------------------------------------
    virtual std::string GetDerivedSettings() { return ""; }

    //--------------------------------------------------------------------------
    /// Clear out all of the ProfilerResults collected by the DX12CmdListProfiler.
    //--------------------------------------------------------------------------
    virtual void ClearProfilingResults() = 0;

    //--------------------------------------------------------------------------
    /// Lock GPA access mutex
    //--------------------------------------------------------------------------
    void LockGPA();

    //--------------------------------------------------------------------------
    /// unlock GPA access mutex
    //--------------------------------------------------------------------------
    void UnlockGPA();

    //--------------------------------------------------------------------------
    /// A GPA logging callback used to output GPA messages to the GPS log.
    /// \param messageType The type of message being logged.
    /// \param message The message string being logged.
    //--------------------------------------------------------------------------
    static void GPALoggingCallback(GPA_Logging_Type messageType, const char* message);

    //--------------------------------------------------------------------------
    /// Retrieve a reference to the GPA loader wrapper.
    /// \returns A reference to the GPA loader wrapper.
    //--------------------------------------------------------------------------
    GPUPerfAPILoader& GetGPALoader() { return mGPALoader; }

    //--------------------------------------------------------------------------
    /// Initialize GPA.
    /// \param inAPI The API being initialized.
    /// \returns True if GPA initialization was successful.
    //--------------------------------------------------------------------------
    bool InitializeGPA(GPA_API_Type inAPI);

    //--------------------------------------------------------------------------
    /// Shutdown GPA.
    /// \returns True if shutdown was successful.
    //--------------------------------------------------------------------------
    bool ShutdownGPA();

    //--------------------------------------------------------------------------
    /// Reset the SampleId unique globally unique number generator.
    //--------------------------------------------------------------------------
    void ResetSampleIdCounter();

    //--------------------------------------------------------------------------
    /// A function used to check if GPU time is being collected while building the frame.
    /// \returns True if GPU Time collection is active. False if it is disabled.
    //--------------------------------------------------------------------------
    inline bool ShouldCollectGPUTime() const { return mbProfilerEnabled; }

    //--------------------------------------------------------------------------
    /// Set the internal flag that determines if GPU command profiling is enabled.
    /// \param inbProfilingEnabled The flag used to enable or disable profiling.
    //--------------------------------------------------------------------------
    virtual void SetProfilingEnabled(bool inbProfilingEnabled) { mbProfilerEnabled = inbProfilingEnabled; }

protected:
    //--------------------------------------------------------------------------
    /// Retrieve the current profiling session's Id.
    /// \returns The current profiling session's Id.
    //--------------------------------------------------------------------------
    gpa_uint32 GetGPASessionId() const { return mCurrentProfilingSessionId; }

    //--------------------------------------------------------------------------
    /// Set the current GPA session Id after starting a new session.
    /// \param inSessionId The new SessionId, as returned from GPA_BeginSession.
    //--------------------------------------------------------------------------
    void SetGPASessionId(gpa_uint32 inSessionId) { mCurrentProfilingSessionId = inSessionId; }

    //--------------------------------------------------------------------------
    /// Reset the session Id used for profiling with GPA.
    //--------------------------------------------------------------------------
    void ResetGPASession();

    //--------------------------------------------------------------------------
    /// A small helper structure to track Begin/End Sample calls per thread.
    //--------------------------------------------------------------------------
    struct SampleInfo
    {
        //--------------------------------------------------------------------------
        /// The SampleId assigned to the GPA sample.
        //--------------------------------------------------------------------------
        UINT64 mSampleId;

        //--------------------------------------------------------------------------
        /// Was BeginSample successful? If so, we can call EndSample.
        //--------------------------------------------------------------------------
        bool mbBeginSampleSuccessful;
    };

    //--------------------------------------------------------------------------
    /// A map used to associate a ThreadId with a SampleInfo structure.
    //--------------------------------------------------------------------------
    typedef unordered_map<DWORD, SampleInfo*> ThreadIdToSampleIdMap;

    //--------------------------------------------------------------------------
    /// An association of CPU ThreadId to a SampleInfo structure.
    //--------------------------------------------------------------------------
    ThreadIdToSampleIdMap mSampleIdMap;

    //--------------------------------------------------------------------------
    /// Set the active profiler SampleId for the given thread ID.
    /// \param inSampleInfo The SampleInfo structure holding the sampling info for a specific traced thread.
    /// \returns A new SampleId.
    //--------------------------------------------------------------------------
    UINT64 SetNextSampleId(SampleInfo* inSampleInfo);

    //--------------------------------------------------------------------------
    /// Retrieve an existing SampleInfo instance, or create a new one, for the given ThreadId.
    /// \param inThreadId The current thread requesting a SampleInfo structure.
    /// \returns A SampleInfo instance to be used with the given ThreadId.
    //--------------------------------------------------------------------------
    SampleInfo* GetSampleInfoForThread(DWORD inThreadId);

    //--------------------------------------------------------------------------
    /// Check if a given pointer was initialized for use as a GPA context.
    /// \param inPossibleContext A pointer to an API object that may have been used to open a GPA context.
    /// \returns True if 'inPossibleContext' was previously used to open a GPA context.
    //--------------------------------------------------------------------------
    bool IsGPAContext(void* inPossibleContext) const { return (mOpenGPAContexts.find(inPossibleContext) != mOpenGPAContexts.end()); }

    //--------------------------------------------------------------------------
    /// Track a pointer used as a Context for profiling with GPA.
    /// \param inContext A pointer to the Context used for profiling with GPA.
    //--------------------------------------------------------------------------
    void TrackGPAContext(void* inContext) { mOpenGPAContexts.insert(inContext); }

    //--------------------------------------------------------------------------
    /// Remove the given pointer from the set of objects being used as a GPA context.
    /// \param inContext A pointer to the object previously used as a GPA context.
    //--------------------------------------------------------------------------
    void EraseGPAContext(void* inContext) { mOpenGPAContexts.erase(inContext); }

    //--------------------------------------------------------------------------
    /// A set of pointers actively being used as a GPA profiling context.
    //--------------------------------------------------------------------------
    std::unordered_set<void*> mOpenGPAContexts;

private:
    //--------------------------------------------------------------------------
    /// Generate the next globally-unique SampleId.
    /// \returns A new SampleId that can be used to profile a GPU command.
    //--------------------------------------------------------------------------
    UINT64 GetNextSampleId();

    //--------------------------------------------------------------------------
    /// A globally unique SampleId counter.
    //--------------------------------------------------------------------------
    UINT64 mSampleIndex;

    //--------------------------------------------------------------------------
    /// A mutex used to lock the SampleId generator.
    //--------------------------------------------------------------------------
    mutex mUniqueSampleIdMutex;

    //--------------------------------------------------------------------------
    /// A mutex used to lock usage of GPA when switching contexts between threads.
    //--------------------------------------------------------------------------
    mutex mGPAPrePostMatcherMutex;

    //--------------------------------------------------------------------------
    /// A GPA loader used as a convenience wrapper for GPA calls.
    //--------------------------------------------------------------------------
    GPUPerfAPILoader mGPALoader;

    //--------------------------------------------------------------------------
    /// The session ID returned from GPA_BeginSession. There's only ever one session active.
    //--------------------------------------------------------------------------
    gpa_uint32 mCurrentProfilingSessionId;

    //--------------------------------------------------------------------------
    /// A flag used to track if a command should be profiled when building the frame.
    //--------------------------------------------------------------------------
    bool mbProfilerEnabled;
};

#endif // MODERNAPIFRAMEPROFILERLAYER_H