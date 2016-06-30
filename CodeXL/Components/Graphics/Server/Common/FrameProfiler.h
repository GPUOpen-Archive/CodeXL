//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Contains the declaration of the FrameProfiler class which should
///         be utilized by all plugins.
//==============================================================================

#ifndef FRAMEPROFILER__H
#define FRAMEPROFILER__H
#include "ILayer.h"
#include "GPUPerfAPIUtils/GPUPerfAPILoader.h"
#include "CommandProcessor.h"
#include "HTTPLogger.h"
#include "IDrawCall.h"
#include "timer.h"
#include <vector>

#if defined GDT_INTERNAL && defined _WIN32
    #include "../InternalCommonSource/ThreadTracer.h"
#endif // GDT_INTERNAL

/// Structure to hold draw call pairs
typedef struct
{
    unsigned int startCall; ///< min marker for the range
    unsigned int endCall;   ///< max marker for the range
} DrawCallPair;

//=============================================================================
/// Collects draw call, api trace, profiler, and frame debugger information.
//=============================================================================
class FrameProfiler: public ILayer, public CommandProcessor
{
public:

    //-----------------------------------------------------------------------------
    /// Default constructor
    //-----------------------------------------------------------------------------
    FrameProfiler();

    //-----------------------------------------------------------------------------
    /// virtual destructor
    //-----------------------------------------------------------------------------
    virtual ~FrameProfiler() {};

    //-----------------------------------------------------------------------------
    /// A profiler should call this function instead of the applications Draw Call
    //-----------------------------------------------------------------------------
    virtual void OnDrawCall(IDrawCall& rDrawCall);

    //-----------------------------------------------------------------------------
    /// Signals the frame analyzer that a frame is beginning and it should
    /// initialize based on any received commands
    //-----------------------------------------------------------------------------
    virtual void BeginFrame();

    //-----------------------------------------------------------------------------
    /// Signals the frame analyzer that the frame is ended and any collected data should be sent
    //-----------------------------------------------------------------------------
    virtual void EndFrame();

    /// Adds a captured call to the stream
    /// \param rRequest the command response to send back the data
    /// \param pcsMessage the message to send back
    /// \param nSize the size of messages stored
    void AddProfiledCall(CommandResponse& rRequest, const char* pcsMessage, int nSize);

    //-----------------------------------------------------------------------------
    /// Similar to OnDrawCall, but only starts the profiling. Used primarily for
    /// profiling a block of draw calls.
    //-----------------------------------------------------------------------------
    void OnDrawCallBegin();

    //-----------------------------------------------------------------------------
    /// Similar to OnDrawCall, but ends the profiling. Used primarily for
    /// profiling a block of draw calls.
    /// \param rDrawCall the drawcall to capture XML description of or profile
    //-----------------------------------------------------------------------------
    void OnDrawCallEnd(IDrawCall& rDrawCall);

protected:

    /// number of profiled counters that need to happen for the server to ping the client
    ULongCommandResponse m_dwProfilerUpdateFrequency;

    //-----------------------------------------------------------------------------
    /// Encapsulates loading the external profiler ".dll" and the "Internal.dll" version
    //-----------------------------------------------------------------------------
    bool LoadProfilerDLL(GPA_API_Type api);

    //-----------------------------------------------------------------------------
    /// Must be implemented by a derived class and provides a pointer to an object
    /// which the profiler can use to identify a particular context or device
    //-----------------------------------------------------------------------------
    virtual void* GetProfilerDevicePtr() = 0;

    //-----------------------------------------------------------------------------
    /// Must be implemented by a derived class and provides a label which is used
    /// by the profiler to identify the counter definitions that should be used
    //-----------------------------------------------------------------------------
    virtual gtASCIIString GetProfilerConfigLabel() = 0;

    //-----------------------------------------------------------------------------
    /// Provides a means for a derived class to add additional information to the
    /// returned statistics
    //-----------------------------------------------------------------------------
    virtual gtASCIIString OnGetStatsXML() { return ""; };

    //-----------------------------------------------------------------------------
    /// Indicates that the FrameProfiler is profiling
    //-----------------------------------------------------------------------------
    bool IsProfiling();

    /// Gets the current progress of the profiler as a percentage
    /// \return -1 if profiling is not active; otherwise returns a percentage of profiler passes that have completed
    int GetProgress()
    {
        if (m_requiredPassCount == 0)
        {
            return -1;
        }

        return (int)((m_currentPass * 100) / m_requiredPassCount);
    }

private:

    //-----------------------------------------------------------------------------
    /// Returns Counter Select result
    //-----------------------------------------------------------------------------
    void DoCounterSelect(TextCommandResponse& rSelectionRequest);

    //-----------------------------------------------------------------------------
    /// Describes the current statistics for the profiler as XML
    //-----------------------------------------------------------------------------
    gtASCIIString GetStatsXML();

#ifdef GDT_INTERNAL
    //-----------------------------------------------------------------------------
    /// Get the shader CRC values as XML.
    /// \param rDrawCall the drawcall to capture XML description of, profile, and/or execute
    /// \param ioXMLString string to append shader CRC XML text to
    //-----------------------------------------------------------------------------
    void GetShaderCRC(IDrawCall& rDrawCall, gtASCIIString& ioXMLString);
#endif

    /// Generates an XML string of the available counters
    void SendCounterInformationString(CommandResponse& rRequest);

    // Note: the GPA back end may allocate resources on the device
    /// Initializes the profiler when a profile request is received
    bool HandleProfilerRequest(void* deviceToMonitor, CommandResponse& rProfilerXML, const char* configSectionLabel);

    /// Updates the profiler to confirm that it is started at the first draw call
    /// \param totalDrawCallCount the number of the current draw call
    /// \param timePasses indicates whether or not the passes are being timed
    void UpdateProfiler(uint32 totalDrawCallCount, bool timePasses);

    /// This is called by the OnDrawCall(...) function for regular profiles
    /// \param rDrawCall the draw call being executed currently
    void OnDrawCall_Profile(IDrawCall& rDrawCall);

    /// This is called by the OnDrawCall(...) function for range profiles
    /// \param rDrawCall the draw call being executed currently
    void OnDrawCall_ProfileRange(IDrawCall& rDrawCall);

    //-----------------------------------------------------------------------------
    /// Simplifies accessing the HandleProfilerRequest method of the profiler. This
    /// method hides some of the additional parameters that would be needed for the
    /// HandleProfilerRequest method.
    /// \param pDevice a pointer to the device or context that is being profiled
    /// \param configSectionLabel a string identifying which section of the counter
    ///    file should be used for this pass of profiling
    /// \return true if the pass could be started and all counters initialized; false otherwise
    //-----------------------------------------------------------------------------
    bool BeginProfilerPass(void* pDevice, const char* configSectionLabel);

    //-----------------------------------------------------------------------------
    /// Simplifies accessing the EndProfilerPass method of the profiler.
    //-----------------------------------------------------------------------------
    void EndProfilerPass();

    //-----------------------------------------------------------------------------
    /// Simplifies accessing the BeginProfilerSample method of the profiler.
    //-----------------------------------------------------------------------------
    void BeginProfilerSample();

    //-----------------------------------------------------------------------------
    /// Simplifies accessing the EndProfilerSample method of the profiler.
    //-----------------------------------------------------------------------------
    void EndProfilerSample();

    /// Generates an XML string of the profiled counter data
    void SendCounterResultString(CommandResponse& rRequest, const char* pcszDrawCalls);

    //-----------------------------------------------------------------------------
    // No additional settings
    //-----------------------------------------------------------------------------
    virtual string GetDerivedSettings() { return ""; }

    //-----------------------------------------------------------------------------
    /// Profiler monitor command fail.
    /// report to client and cleanup.
    //-----------------------------------------------------------------------------
    void CounterMeasureFail(const char* error);

    /// Converts the status into a string that can be used in error messages
    /// \param status the status to convert
    /// \return a string version of the status
    std::string GetStatusString(GPA_Status status);

    /// Asserts if the supplied status indicates an error
    /// \param status the status to check
    /// \return the same status that was supplied as a parameter
    GPA_Status StatusCheck(GPA_Status status);

    /// Callback function for GPA Logging.
    /// Routes GPA log messages to the PerfStudio log.
    /// \param messageType the type of logging message
    /// \param message the message to log
    static void GPALoggingCallback(GPA_Logging_Type messageType, const char* message);

#if defined GDT_INTERNAL && defined _WIN32
    /// Callback function for GPA debug Logging.
    /// Routes GPA log messages to the PerfStudio log.
    /// \param messageType the type of logging message
    /// \param message the message to log
    static void GPALoggingDebugCallback(GPA_Log_Debug_Type messageType, const char* message);
#endif // GDT_INTERNAL

private:

#if defined GDT_INTERNAL && defined _WIN32
    /// Handles all thread trace commands.
    ThreadTracer m_threadTracer;
#endif

    /// Number of Draw Calls, this is incremented every frame and is used for breakpoints
    unsigned long m_ulDrawCallCounter;

    /// stores the number of drawcalls in the previous frame
    unsigned long m_ulDrawCallsInPrevFrame;

    /// The active profile command (points to either the m_profilerData or m_drawcallProfilerData member).
    /// Should be NULL if there is no active profile command.
    CommandResponse* m_pActiveProfileCommand;

    /// Profiler data response
    HTTPLogger   m_profilerData;

    /// Drawcall profiler data input
    IntCommandResponse m_drawcallProfilerData;

    /// Range Profiler data response
    TextCommandResponse   m_ProfileRangeData;

    /// Drawcall profiler data response
    string m_drawcallProfilerXML;

    /// Stores the most recent GPA error string so that GPS2 can give the user better feedback when a counter fails.
    static string m_sLastGPAError;

    /// Profiler draw call list
    string m_profilerDrawCalls;

    /// Counter Info response
    CommandResponse   m_counterInfo;

    /// Statistics response
    CommandResponse   m_Stats;

    /// Counter Selection response
    TextCommandResponse   m_CounterSelectResponse;

    /// Monitor response
    TextCommandResponse   m_monitorResponse;

    /// Incremental response
    TextCommandResponse   m_incrementalResponse;

    /// Incremental response
    TextCommandResponse   m_drawFilterResponse;

    /// Records when a profile is taking place.
    bool m_executingMonitor;

    /// Timer used in sending data back to the client.
    Timer m_monitorTime;

    /// In milliseconds.
    unsigned int m_timeToMonitor;

    /// Used to store the CPU time.
    Timer m_CPUTime;

    /// Used to manage the recording of time for GPU.
    bool m_measureStartEndGPUTime;

    /// Used to manage the recording of time for CPU.
    bool m_measureStartEndCPUTime;

    /// Used in incremental timing.
    bool m_executingIncremental;

    /// Used to track the current draw call.
    unsigned int m_currentIncDrawIndex;

    /// Used to store the average GPU time.
    uint64 m_avgGPUTime;

    /// Used to store the min GPU time.
    uint64 m_minGPUTime;

    /// Used to store the max GPU time.
    uint64 m_maxGPUTime;

    /// Used to store the average CPU time.
    double m_avgCPUTime;

    /// Used to store the min CPU time.
    double m_minCPUTime;

    /// Used to store the max CPU time.
    double m_maxCPUTime;

    /// Record the frame count.
    unsigned long m_monitorFrameCount;

    /// Record th current session ID.
    uint32 m_currentWaitSessionID;

    /// Record the current outstanding sessions count.
    unsigned int m_outstandingSessions;

    /// Record the average draws.
    unsigned int m_aveDraws;

    /// Record the min draws.
    unsigned int m_minDraws;

    /// Record the max draws.
    unsigned int m_maxDraws;

    /// Used to count number of frames while GPUTime is calculated so the average can be taken
    unsigned long m_gpuTimeFrameCount;

    /// Used to count number of frames while GPUBusy is calculated so the average can be taken
    unsigned long m_gpuBusyFrameCount;

    /// Used to count number of frames while CPUTime/inDraws is calculated so the average can be taken
    unsigned long m_cpuFrameCount;

    /// Vector to record the GPU timings.
    std::vector<uint64> m_gpuPassTiming;

    /// Vector to record the CPU timings.
    std::vector<uint64> m_cpuPassTiming;

    /// Flag to control how draws are filtered.
    bool m_drawFilter;

    /// Flag to control the monitoring of timing.
    bool m_monitorTiming;

    /// Used to calculate an average GPU busy time.
    float64 m_gpuBusy;

    /// Record the minimum GPU busy time.
    float64 m_gpuBusyMin;

    /// Record the maximum GPU busy time.
    float64 m_gpuBusyMax;

    /// Indicates when gpuBusy counter is available
    bool m_gpuBusyAvailable;

    /// Used to record when a sample has started.
    bool m_sampleStarted;

    GPUPerfAPILoader m_GPALoader;                           ///< the loader that is used to load GPA dlls
    gpa_uint32 m_currentSessionID;                          ///< the current session ID of the profiler
    gpa_uint32 m_currentPass;                               ///< the current pass in multi-pass profiling
    gpa_uint32 m_requiredPassCount;                         ///< the number of required passes to gather all data
    unsigned int m_lastBeginDrawCallCount;                  ///< the number of draw calls in the last frame - used to confirm that BeginSample is called correctly
    bool m_passStarted;                                     ///< indicates that a pass has been started
    unsigned int m_profileRetries;                          ///< the number of times the profiler will retry to get data
    std::vector<int32> m_enabledCounters;                   ///< array of enabled counters
    std::vector<unsigned int> m_SampledDrawCalls;           ///< array of draw cals for which the BeginSamples() was called

    // Data structures for range profiling
    std::list<DrawCallPair> m_ProfileRangeDrawCallPairs;    ///< array of draw call pairs which specify the range for profiling
};

#endif //FRAMEPROFILER__H
