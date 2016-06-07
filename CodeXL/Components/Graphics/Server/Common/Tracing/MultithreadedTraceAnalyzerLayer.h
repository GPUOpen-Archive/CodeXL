//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The baseclass for the Multithreaded Trace Analyzer.
//==============================================================================

#ifndef MULTITHREADEDTRACEANALYZERLAYER_H
#define MULTITHREADEDTRACEANALYZERLAYER_H

#include "../../Common/ModernAPILayerManager.h"
#include "../../Common/IModernAPILayer.h"
#include "../../Common/OSwrappers.h"
#include "../../Common/CommandProcessor.h"
#include "../../Common/TimingLog.h"
#include "../../Common/Timer.h"
#include <unordered_map>

//
/// Forward declare these, since the definition exists in multiple places.
//
enum FuncId : int;
    class TraceMetadata;
    class ThreadTraceData;

    //--------------------------------------------------------------------------
    /// Associate a FuncId with a string containing the name of the function.
    //--------------------------------------------------------------------------
    typedef std::map<FuncId, std::string> FuncIdToNamestringMap;

    //--------------------------------------------------------------------------
    /// A map used to associate a ThreadId with the structure that all the thread's results are stored in.
    //--------------------------------------------------------------------------
    typedef std::unordered_map<DWORD, ThreadTraceData*> ThreadIdToTraceData;

    //--------------------------------------------------------------------------
    /// Collects API Trace in a multi-threaded manner by mapping each submission
    /// thread to its own buffer that it can dump logged calls to.
    //--------------------------------------------------------------------------
    class MultithreadedTraceAnalyzerLayer : public IModernAPILayer, public CommandProcessor
{
public:
    //--------------------------------------------------------------------------
    /// MultithreadedTraceAnalyzerLayer destructor. Will clean up trace data.
    //--------------------------------------------------------------------------
    virtual ~MultithreadedTraceAnalyzerLayer();

    //--------------------------------------------------------------------------
    /// Retrieve a string containing the current API.
    /// \returns A string containing the current API.
    //--------------------------------------------------------------------------
    virtual char const* GetAPIString() = 0;

    //--------------------------------------------------------------------------
    /// Begin the frame.
    //--------------------------------------------------------------------------
    virtual void BeginFrame();

    //--------------------------------------------------------------------------
    /// End the frame.
    //--------------------------------------------------------------------------
    virtual void EndFrame();

    //-----------------------------------------------------------------------------
    /// Overridden implementation that concatenates all thread trace logs together
    /// and returns them through a single combined string.
    /// \return A line-delimited, ASCII-encoded, version of the API Trace data.
    //-----------------------------------------------------------------------------
    virtual std::string GetAPITraceTXT();

    //-----------------------------------------------------------------------------
    /// Return GPU-time in text format, to be parsed by the Client and displayed as its own timeline.
    /// \return A line-delimited, ASCII-encoded, version of the GPU Trace data.
    //-----------------------------------------------------------------------------
    virtual std::string GetGPUTraceTXT();

    //-----------------------------------------------------------------------------
    /// Return the device used in the target application.
    //-----------------------------------------------------------------------------
    virtual void* GetActiveDevice() { return NULL; }

    //--------------------------------------------------------------------------
    /// Call this before invoking the actual implementation of an API call.
    //--------------------------------------------------------------------------
    virtual void BeforeAPICall();

    //--------------------------------------------------------------------------
    /// Invoked when the MultithreadedTraceAnalyzerLayer is created.
    /// \param inType The incoming type of interface being created.
    /// \param pInPtr An interface pointer being created.
    /// \returns True if creation was successful.
    //--------------------------------------------------------------------------
    virtual bool OnCreate(CREATION_TYPE inType, void* pInPtr);

    //--------------------------------------------------------------------------
    /// Invoked when the MultithreadedTraceAnalyzerLayer is finished and about to be destroyed.
    /// \param inType The incoming type of instance being destroyed.
    /// \param pInPtr An instance pointer being destroyed.
    /// \returns True if destruction was successful.
    //--------------------------------------------------------------------------
    virtual bool OnDestroy(CREATION_TYPE inType, void* pInPtr);

    //--------------------------------------------------------------------------
    /// Return the stringified function name based on input enum.
    /// \param inFunctionId An enumeration representing the function being invoked.
    /// \returns A string containing the function name.
    //--------------------------------------------------------------------------
    virtual const char* GetFunctionNameFromId(FuncId inFunctionId) = 0;

    //--------------------------------------------------------------------------
    /// Step through all of the timestamps collected when tracing an application.
    /// Verify that they are all unique within a thread, as expected.
    /// \returns True if all timestamps were verified as being unique. False if non-zero timestamps match exactly.
    //--------------------------------------------------------------------------
    virtual bool VerifyUniqueTimestamps() const;

    //--------------------------------------------------------------------------
    /// Clear all logged data. Overridden because this TraceAnalyzer collects multiple
    /// instances of the same data, so it all has to be deallocated.
    //--------------------------------------------------------------------------
    void Clear();

    //--------------------------------------------------------------------------
    /// Check if the newly-started frame should be automatically traced at a specific frame.
    /// \returns An eTraceType value, corresponding to the value set in the server config.
    //--------------------------------------------------------------------------
    int GetTraceTypeFlags();

    //--------------------------------------------------------------------------
    /// Retrieve the number of API calls invoked within a traced frame.
    /// \returns The number of API calls invoked within a traced frame.
    //--------------------------------------------------------------------------
    uint32 GetNumTracedAPICalls();

    //--------------------------------------------------------------------------
    /// Retrieve the number of Draw calls that occurred within a traced frame.
    /// \returns The number of Draw calls that occurred within a traced frame.
    //--------------------------------------------------------------------------
    uint32 GetNumTracedDrawCalls();

    //--------------------------------------------------------------------------
    /// Enable collection of a linked trace before a new frame is started.
    //--------------------------------------------------------------------------
    void EnableLinkedTraceCollection();

    //--------------------------------------------------------------------------
    /// Disable collection of a linked trace after a new frame has finished.
    //--------------------------------------------------------------------------
    void DisableLinkedTraceCollection();

    //--------------------------------------------------------------------------
    /// Set the internal flag that determines if API Trace collection is enabled.
    /// \param inbCollectTrace The flag used to enable or disable API tracing.
    //--------------------------------------------------------------------------
    inline void SetCollectTrace(bool inbCollectTrace) { mbCollectApiTrace = inbCollectTrace; }

    //--------------------------------------------------------------------------
    /// A function used to check if a function should be logged in the API Trace.
    /// \returns True if the function should be logged in the API Trace.
    //--------------------------------------------------------------------------
    inline bool ShouldCollectTrace() const { return mbCollectApiTrace; }

    //--------------------------------------------------------------------------
    /// Check if the API Trace request is available to handle.
    /// \returns True when an API trace has been requested through the APITraceTXT command.
    //--------------------------------------------------------------------------
    inline bool OnlyAPITraceRequested() { return m_apiTraceTXT.IsActive(); }

    //--------------------------------------------------------------------------
    /// Check if the GPU Trace request is available to handle.
    /// \returns True when a GPU trace has been requested through the GPUTrace command.
    //--------------------------------------------------------------------------
    inline bool OnlyGPUTraceRequested() { return m_cmdGPUTrace.IsActive(); }

    //--------------------------------------------------------------------------
    /// Accessor for mFramestartTime
    //--------------------------------------------------------------------------
    inline GPS_TIMESTAMP GetFrameStartTime() const { return mFramestartTime; }

    //--------------------------------------------------------------------------
    /// A variable containing the index of the last traced frame.
    /// \returns The index of the last traced frame.
    //--------------------------------------------------------------------------
    inline int GetLastTracedFrameIndex() const { return mLastTracedFrameIndex; }

protected:
    //--------------------------------------------------------------------------
    /// A protected constructor, because this will be used as the baseclass for a singleton.
    //--------------------------------------------------------------------------
    MultithreadedTraceAnalyzerLayer();

    //--------------------------------------------------------------------------
    /// Create a new instance of an API-specific ThreadTraceData object.
    /// \returns A new instance of a ThreadTraceData object.
    //--------------------------------------------------------------------------
    virtual ThreadTraceData* CreateThreadTraceDataInstance() = 0;

    //--------------------------------------------------------------------------
    /// Initialize any state before an API trace is performed.
    //--------------------------------------------------------------------------
    virtual void BeforeAPITrace() { }

    //--------------------------------------------------------------------------
    /// Cleanup that needs to happen after an API Trace should be implemented here.
    //--------------------------------------------------------------------------
    virtual void AfterAPITrace();

    //--------------------------------------------------------------------------
    /// A chance to initialize states before a GPU trace is performed.
    //--------------------------------------------------------------------------
    virtual void BeforeGPUTrace() { }

    //--------------------------------------------------------------------------
    /// Cleanup that needs to happen after a GPU Trace should be implemented here.
    //--------------------------------------------------------------------------
    virtual void AfterGPUTrace() { }

    //--------------------------------------------------------------------------
    /// No additional settings.
    /// \returns A string containing additional derived TraceAnalyzer settings.
    //--------------------------------------------------------------------------
    virtual std::string GetDerivedSettings() { return ""; }

    //--------------------------------------------------------------------------
    /// Clear and destroy all ThreadTraceData instances.
    //--------------------------------------------------------------------------
    void ClearCPUThreadTraceData();

    //--------------------------------------------------------------------------
    /// Generate a trace info block that can be appended to the top of the trace. Included application and system information.
    /// \param outHeaderString A string containing the generated block of header text.
    /// \returns True if the header was generated successfully. False if it failed.
    //--------------------------------------------------------------------------
    bool GenerateLinkedTraceHeader(gtASCIIString& outHeaderString);

    //--------------------------------------------------------------------------
    /// Handle what happens when a Linked Trace is requested. We can either:
    /// 1. Return the trace response as normal.
    /// 2. Cache the response to disk, and generate a "trace metadata" file used
    /// to retrieve the trace later.
    /// \param inFullResponseString The response string built by tracing the application.
    /// \param inbSaveResponseToFile A switch used to determine which response method to use.
    //--------------------------------------------------------------------------
    void HandleLinkedTraceResponse(gtASCIIString& inFullResponseString, bool inbSaveResponseToFile);

    //--------------------------------------------------------------------------
    /// Handle what happens when a API Trace is requested. We can either:
    /// 1. Return the cached trace file
    /// 2. Send live generated data back to the client
    /// \param inFullResponseString Data to send to command response
    //--------------------------------------------------------------------------
    void HandleAPITraceResponse(std::string& inFullResponseString);

    //--------------------------------------------------------------------------
    /// Handle what happens when a GPU Trace is requested. We can either:
    /// 1. Return the cached trace file
    /// 2. Send live generated data back to the client
    /// \param inFullResponseString Data to send to command response
    //--------------------------------------------------------------------------
    void HandleGPUTraceResponse(std::string& inFullResponseString);

    //--------------------------------------------------------------------------
    /// Send a cached trace file to a specific command response
    /// \param m_cmdResponse Command Response to send the cached trace data to
    //--------------------------------------------------------------------------
    void SendTraceFile(CommandResponse& m_cmdGPUTrace);

    //--------------------------------------------------------------------------
    /// Find thread-private trace data to dump logged calls into.
    /// \param inThreadId A ThreadId used to lookup or create a corresponding ThreadTraceData instance.
    /// \returns A new or existing ThreadTraceData instance for use with a specific thread.
    //--------------------------------------------------------------------------
    ThreadTraceData* FindOrCreateThreadData(DWORD inThreadId);

    //--------------------------------------------------------------------------
    /// Write a trace's metadata file and return the contenst through the out-param.
    /// \param inHeaderString The full response string for a collected linked trace request.
    /// \param inResponseString Response string
    /// \param outMetadataXML The XML metadata string to return to the client.
    /// \returns True if writing the metadata file was succesful.
    //--------------------------------------------------------------------------
    bool WriteTraceAndMetadataFiles(const gtASCIIString& inHeaderString, const gtASCIIString& inResponseString, std::string& outMetadataXML);

    //--------------------------------------------------------------------------
    /// Load a trace file from disk when given a valid path.
    /// \param inTraceFilepath The full filepath to a trace file.
    /// \param outTraceFileContents The full contents of the loaded trace file.
    /// \returns True if the trace file was loaded correctly.
    //--------------------------------------------------------------------------
    bool LoadTraceFile(const std::string& inTraceFilepath, gtASCIIString& outTraceFileContents);

    /// This function alters the two input bools based on the inpout capture type. It is used to switch the
    //  two capture types on or off based on the capture type.
    /// \param captureType Input capture type.
    /// \param APITraceFlag
    /// \param GPUTraceFlag
    void FilterTraceTypes(CaptureType captureType, bool &APITraceFlag, bool &GPUTraceFlag);

    //--------------------------------------------------------------------------
    /// The LinkedTrace CommandResponse can do two things:
    /// 1. Used to retrieve a linked trace where API/GPU traces are combined.
    /// 2. Used to retrieve a cached trace that has previously been saved to disk.
    //--------------------------------------------------------------------------
    CommandResponse mCmdLinkedTrace;

    /// Command to collect API calls in text and send back to the client.
    CommandResponse m_apiTraceTXT;

    /// Command to request the GPUTrace data to be sent back to the client.
    CommandResponse m_cmdGPUTrace;

    //--------------------------------------------------------------------------
    /// A CommandResponse that accepts a path to a trace metadata file, and will
    /// handle loading/reading trace response text, and return to client.
    //--------------------------------------------------------------------------
    TextCommandResponse mRetrieveCachedTraceResponse;

    //--------------------------------------------------------------------------
    /// A CommandResponse used to retrieve the cached trace collected when AutoCapture is enabled.
    //--------------------------------------------------------------------------
    CommandResponse mCmdAutoCaptureCachedTrace;

    //--------------------------------------------------------------------------
    /// A flag to indicate if the GPU Trace has already been collected.
    //--------------------------------------------------------------------------
    bool mbGPUTraceAlreadyCollected;

    //--------------------------------------------------------------------------
    /// A flag to indicate if API calls are actively being traced during the frame.
    //--------------------------------------------------------------------------
    bool mbCollectingApiTrace;

    //--------------------------------------------------------------------------
    /// A flag to indicate if API calls are actively being profiled during the frame.
    //--------------------------------------------------------------------------
    bool mbCollectingGPUTrace;

    //--------------------------------------------------------------------------
    /// Only collect a FuncId while tracing. Build a client response using lookups in this maps.
    //--------------------------------------------------------------------------
    FuncIdToNamestringMap mFunctionIndexToNameString;

    //--------------------------------------------------------------------------
    /// A map of ThreadID -> TraceData, used to buffer logged API calls for each thread.
    //--------------------------------------------------------------------------
    ThreadIdToTraceData mThreadTraces;

    //--------------------------------------------------------------------------
    /// Mutex used in cases where the mThreadTraces map can potentially change.
    //--------------------------------------------------------------------------
    mutex mTraceMutex;

    //--------------------------------------------------------------------------
    /// A timer used to figure out what time the frame started at.
    //--------------------------------------------------------------------------
    GPS_TIMESTAMP mFramestartTime;

    //--------------------------------------------------------------------------
    /// A timer used to indicate exactly when the frame rendering begins.
    //--------------------------------------------------------------------------
    Timer mFramestartTimer;

    //--------------------------------------------------------------------------
    /// Store the last trace response here. This is necessary for AutoCapture so
    /// a client can connect after a trace completes and still collect and load the data.
    //--------------------------------------------------------------------------
    std::string mCachedTraceResponse;

    //--------------------------------------------------------------------------
    /// A flag that gets set when autocapture has occurred, but the client hasn't yet picked up the response.
    //--------------------------------------------------------------------------
    bool mbWaitingForAutocaptureClient;

    //--------------------------------------------------------------------------
    /// A flag to indicate that a linked trace is collected for capture to disk mode.
    //--------------------------------------------------------------------------
    bool mbLinkedTraceForCapture;

    //--------------------------------------------------------------------------
    /// A flag used to track if an API trace should be collected when building the frame.
    //--------------------------------------------------------------------------
    bool mbCollectApiTrace;

    //--------------------------------------------------------------------------
    /// Track the index of the last frame that was traced. Start at -1 to track that it hasn't happened yet.
    //--------------------------------------------------------------------------
    int mLastTracedFrameIndex;
};

#endif // MULTITHREADEDTRACEANALYZERLAYER_H
