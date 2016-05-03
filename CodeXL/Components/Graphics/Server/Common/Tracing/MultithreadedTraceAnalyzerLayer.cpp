//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The baseclass for the Multithreaded Trace Analyzer.
//==============================================================================

#include "MultithreadedTraceAnalyzerLayer.h"
#include "../SharedGlobal.h"
#include "../ModernAPILayerManager.h"
#include "../ModernAPIFrameDebuggerLayer.h"
#include "../ModernAPIFrameProfilerLayer.h"
#include "../TraceMetadata.h"
#include "../FrameInfo.h"
#include "../OSWrappers.h"
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include "../misc.h"
#include "../SessionManager.h"
#include "../TypeToString.h"
#include "APIEntry.h"
#include "ThreadTraceData.h"

//--------------------------------------------------------------------------
/// MultithreadedTraceAnalyzerLayer's default constructor, which initializes CommandResponses.
//--------------------------------------------------------------------------
MultithreadedTraceAnalyzerLayer::MultithreadedTraceAnalyzerLayer()
    : IModernAPILayer()
    , mbGPUTraceAlreadyCollected(false)
    , mbCollectingApiTrace(false)
    , mbCollectingGPUTrace(false)
    , mbWaitingForAutocaptureClient(false)
    , mbLinkedTraceForCapture(false)
    , mLastTracedFrameIndex(-1)
    , mbCollectApiTrace(false)
{
    // Command that collects an API trace for a newly-rendered frame.
    AddCommand(CONTENT_TEXT, "TXTLog", "API Trace TXT", "Log.txt", DISPLAY, INCLUDE, m_apiTraceTXT);

    // Command that collects a GPU Trace for a newly-rendered frame.
    AddCommand(CONTENT_TEXT, "GPUTrace", "GPUTrace", "GPUTrace.txt", DISPLAY, INCLUDE, m_cmdGPUTrace);

    // Command that collects a CPU and GPU trace from the same frame.
    AddCommand(CONTENT_TEXT, "LinkedTrace", "LinkedTrace", "LinkedTrace.txt", DISPLAY, INCLUDE, mCmdLinkedTrace);

    // Command used to automatically trace a target frame in an instrumented application.
    AddCommand(CONTENT_TEXT, "AutoTrace", "AutoTrace", "AutoTrace.txt", DISPLAY, INCLUDE, mCmdAutoCaptureCachedTrace);
}

//--------------------------------------------------------------------------
/// MultithreadedTraceAnalyzerLayer's destructor, in which all trace buffers are destroyed.
//--------------------------------------------------------------------------
MultithreadedTraceAnalyzerLayer::~MultithreadedTraceAnalyzerLayer()
{
    // Destroy all of the buffered trace data.
    ClearCPUThreadTraceData();
}

//--------------------------------------------------------------------------
/// Cleanup that needs to happen after a GPU Trace should be implemented here.
//--------------------------------------------------------------------------
void MultithreadedTraceAnalyzerLayer::AfterAPITrace()
{
    bool bVerifiedTimestamps = VerifyUniqueTimestamps();

    if (!bVerifiedTimestamps)
    {
        Log(logERROR, "API Trace Timestamps failed verification. Refer to logged errors for specific details.\n");
    }
}

//--------------------------------------------------------------------------
/// BeginFrame is called when a new frame is started, and should setup the
/// collection process to log each API call issued during the frame.
/// \return Nothing.
//--------------------------------------------------------------------------
void MultithreadedTraceAnalyzerLayer::BeginFrame()
{
    bool bFrameCaptureWithSaveActive = GetParentLayerManager()->mCmdFrameCaptureWithSave.IsActive();

    // Check if automatic tracing is enabled for a specific frame. Determine which trace type by examining the result.
    int autotraceFlags = GetTraceTypeFlags();

    // Pick up new requests that are active and flip the switch to begin tracing calls.
    bool bLinkedTraceRequested = (mCmdLinkedTrace.IsActive() || bFrameCaptureWithSaveActive) || (autotraceFlags == kTraceType_Linked) || mbLinkedTraceForCapture;

    // If a linked trace is required, turn on both trace switches.
    bool bAPITraceNeeded = OnlyAPITraceRequested() || bLinkedTraceRequested || (autotraceFlags & kTraceType_API);
    bool bGPUTraceNeeded = OnlyGPUTraceRequested() || bLinkedTraceRequested || (autotraceFlags & kTraceType_GPU);

    mFramestartTime = mFramestartTimer.GetRaw();

    if (bAPITraceNeeded || bGPUTraceNeeded)
    {
        // Set the flag indicating that the frame is being traced. We'll be rendering the next frame when this flag is checked.
        mLastTracedFrameIndex = GetParentLayerManager()->GetFrameCount();

        // Clear out the previous trace data before tracing the new frame.
        Clear();

        // We need to enable tracing no matter what so that we go into the PreCall/PostCall.
        SetCollectTrace(true);

        if (bAPITraceNeeded)
        {
            // Enable global trace collection so API and GPU trace can happen.
            BeforeAPITrace();
            mbCollectingApiTrace = true;
        }

        if (bGPUTraceNeeded)
        {
            BeforeGPUTrace();

            // Enable GPU time collection
            ModernAPIFrameProfilerLayer* frameProfiler = GetParentLayerManager()->GetFrameProfilerLayer();
            frameProfiler->SetProfilingEnabled(true);
            mbCollectingGPUTrace = true;
        }
    }
}

//--------------------------------------------------------------------------
/// EndFrame is used to signal the end of a rendered frame and to stop/send
/// the captured results that were logged during the frame render.
/// \return Nothing.
//--------------------------------------------------------------------------
void MultithreadedTraceAnalyzerLayer::EndFrame()
{
    // Check again which trace type is active at the end of the frame. Need to match how it was started.
    int autotraceFlags = GetTraceTypeFlags();

    // Will we be required to dump the trace response to a file on disk?
    bool bSaveResponseToFile = GetParentLayerManager()->mCmdFrameCaptureWithSave.IsActive() || mbLinkedTraceForCapture;

    // If the linked trace was requested, return all of the results through a single response.
    bool bLinkedTraceRequested = bSaveResponseToFile || mCmdLinkedTrace.IsActive() || (autotraceFlags == kTraceType_Linked);

    // If we want a linked trace, we'll need responses from all trace types.
    bool bAPITraceResponseNeeded = OnlyAPITraceRequested() || bLinkedTraceRequested || (autotraceFlags & kTraceType_API);
    bool bGPUTraceResponseNeeded = OnlyGPUTraceRequested() || bLinkedTraceRequested || (autotraceFlags & kTraceType_GPU);

    if (bAPITraceResponseNeeded || bGPUTraceResponseNeeded)
    {
        // We're done collecting, so turn off trace collection.
        SetCollectTrace(false);

        // Also disable frame profiling.
        ModernAPIFrameProfilerLayer* frameProfiler = GetParentLayerManager()->GetFrameProfilerLayer();
        frameProfiler->SetProfilingEnabled(false);

        AfterAPITrace();
        AfterGPUTrace();

        std::string apiTraceResponseString, gpuTraceResponseString;

        if (bAPITraceResponseNeeded)
        {
            mbCollectingApiTrace = false;
            apiTraceResponseString.assign(GetAPITraceTXT().c_str());
        }

        if (bGPUTraceResponseNeeded)
        {
            mbGPUTraceAlreadyCollected = false;
            gpuTraceResponseString.assign(GetGPUTraceTXT().c_str());
        }

        gtASCIIString fullResponseString;

        if (bAPITraceResponseNeeded)
        {
            fullResponseString += apiTraceResponseString.c_str();
            fullResponseString += "\n";
        }

        if (bGPUTraceResponseNeeded)
        {
            fullResponseString += gpuTraceResponseString.c_str();
            fullResponseString += "\n";
        }

        // If the autotrace flags are anything besides "None," we'll just store the trace log internally so the client can pick it up later.
        bool bShouldCacheForAutotrace = (autotraceFlags != kTraceType_None);

        if (bShouldCacheForAutotrace)
        {
            // Don't send the response back through a command yet.
            // The client will know to pick it up through a special AutoCapture command.
            mCachedTraceResponse.assign(fullResponseString.asCharArray());
            mbWaitingForAutocaptureClient = true;
        }
        else
        {
            // Send the response string back to the server through a specific request command.
            if (bLinkedTraceRequested)
            {
                HandleLinkedTraceResponse(fullResponseString, bSaveResponseToFile);

                // If tracing was active, disable it after handling the response.
                if (mbLinkedTraceForCapture)
                {
                    DisableLinkedTraceCollection();
                }
            }
            else
            {
                if (bAPITraceResponseNeeded)
                {
                    m_apiTraceTXT.Send(apiTraceResponseString.c_str());
                }
                else if (bGPUTraceResponseNeeded)
                {
                    m_cmdGPUTrace.Send(gpuTraceResponseString.c_str());
                }
            }
        }

    }

    // When AutoCapture is enabled, we need to delay rendering so the user has time to retrieve the cached response.
    if (mbWaitingForAutocaptureClient)
    {
        // For AutoCapture the client will send the separate request below to collect the cached trace response.
        if (mCmdAutoCaptureCachedTrace.IsActive())
        {
            // We're done waiting to send the cached response to the client. Move on with playback.
            mbWaitingForAutocaptureClient = false;

            mCmdAutoCaptureCachedTrace.Send(mCachedTraceResponse.c_str());
            mCachedTraceResponse.clear();
        }
        else
        {
            // Sleep to give the user a chance to connect during playback.
            osSleep(500);
        }
    }
}

//--------------------------------------------------------------------------
/// Load a trace file from disk when given a valid path.
/// \param inTraceFilepath The full filepath to a trace file.
/// \param outTraceFileContents The full contents of the loaded trace file.
/// \returns True if the trace file was loaded correctly.
//--------------------------------------------------------------------------
bool MultithreadedTraceAnalyzerLayer::LoadTraceFile(const std::string& inTraceFilepath, gtASCIIString& outTraceFileContents)
{
    bool bReadSuccessful = false;
    gtString traceFilepath;
    traceFilepath.fromASCIIString(inTraceFilepath.c_str());
    osFile traceFile(traceFilepath);
    bool bTraceFileOpened = traceFile.open(osChannel::OS_ASCII_TEXT_CHANNEL);

    if (bTraceFileOpened)
    {
        // Read the entire file and return the contents through the output string.
        if (traceFile.readIntoString(outTraceFileContents))
        {
            bReadSuccessful = true;
        }
        else
        {
            Log(logERROR, "Failed to read trace file at path '%s'.", inTraceFilepath.c_str());
        }
    }

    return bReadSuccessful;
}

//--------------------------------------------------------------------------
/// Clear and destroy all of the traced thread data.
//--------------------------------------------------------------------------
void MultithreadedTraceAnalyzerLayer::ClearCPUThreadTraceData()
{
    if (!mThreadTraces.empty())
    {
        ScopeLock threadTraceLock(&mTraceMutex);

        // Kill all thread trace buffers so we can start over on the next frame, or just to shut the layer down.
        std::unordered_map<DWORD, ThreadTraceData*>::iterator threadIter;

        for (threadIter = mThreadTraces.begin(); threadIter != mThreadTraces.end(); ++threadIter)
        {
            ThreadTraceData* traceData = threadIter->second;
            SAFE_DELETE(traceData);
        }

        mThreadTraces.clear();
    }
}

//--------------------------------------------------------------------------
/// Enable collection of a linked trace before a new frame is started.
//--------------------------------------------------------------------------
void MultithreadedTraceAnalyzerLayer::EnableLinkedTraceCollection()
{
    mbLinkedTraceForCapture = true;
}

//--------------------------------------------------------------------------
/// Disable collection of a linked trace after a new frame has finished.
//--------------------------------------------------------------------------
void MultithreadedTraceAnalyzerLayer::DisableLinkedTraceCollection()
{
    mbLinkedTraceForCapture = false;
}

//--------------------------------------------------------------------------
/// Generate a trace info block that can be appended to the top of the trace. Included application and system information.
/// \param outHeaderString A string containing the generated block of header text.
/// \returns True if the header was generated successfully. False if it failed.
//--------------------------------------------------------------------------
bool MultithreadedTraceAnalyzerLayer::GenerateLinkedTraceHeader(gtASCIIString& outHeaderString)
{
    bool bHeaderGenerated = false;

    // The response should include a header when connected to CodeXL Graphics.
    outHeaderString.appendFormattedString("//CodeXL Frame Trace\n");

    osModuleArchitecture moduleArchitecture;
    osRuntimePlatform currentPlatform;
    gtString executablePath;
    gtString commandLine;
    gtString workingDirectory;

    if (osGetProcessLaunchInfo(osGetCurrentProcessId(), moduleArchitecture, currentPlatform, executablePath, commandLine, workingDirectory) == true)
    {
        outHeaderString.appendFormattedString("//ProcessExe=%s\n", executablePath.asASCIICharArray());

        // Build a timestamp.
        osTime currentTime;
        currentTime.setFromCurrentTime();
        tm timeStruct;
        currentTime.timeAsTmStruct(timeStruct, osTime::LOCAL);

        // Need to add 1900, since tm contains "years since 1900".
        int year = timeStruct.tm_year + 1900;

        // Need to add 1, since tm contains "months since January".
        int month = timeStruct.tm_mon + 1;

        int day = timeStruct.tm_mday;
        int hour = timeStruct.tm_hour;
        int minute = timeStruct.tm_min;
        int second = timeStruct.tm_sec;

        gtASCIIString timestampBuilder;
        timestampBuilder.appendFormattedString("%d/%d/%d %d:%d:%d", month, day, year, hour, minute, second);
        outHeaderString.appendFormattedString("//TraceDateTime=%s\n", timestampBuilder.asCharArray());

        outHeaderString.appendFormattedString("//TraceFileVersion=%d\n", 1);
        outHeaderString.appendFormattedString("//ApplicationArgs=%s\n", commandLine.asASCIICharArray());
        outHeaderString.appendFormattedString("//WorkingDirectory=%s\n", workingDirectory.asASCIICharArray());

        // Build a system information header string.
        std::string systemInfo;
        OSWrappers::WriteSystemInfoString(systemInfo);
        outHeaderString.appendFormattedString("\n%s\n", systemInfo.c_str());

        bHeaderGenerated = true;
    }
    else
    {
        Log(logERROR, "Failed to retrieve process info when building response header.\n");
    }

    return bHeaderGenerated;
}

//--------------------------------------------------------------------------
/// Handle what happens when a Linked Trace is requested. We can either:
/// 1. Return the trace response as normal.
/// 2. Cache the response to disk, and generate a "trace metadata" file used to retrieve the trace later.
/// \param inFullResponseString The response string built by tracing the application.
/// \param inbSaveResponseToFile A switch used to determine which response method to use.
//--------------------------------------------------------------------------
void MultithreadedTraceAnalyzerLayer::HandleLinkedTraceResponse(gtASCIIString& inFullResponseString, bool inbSaveResponseToFile)
{
    ModernAPILayerManager* parentLayerManager = GetParentLayerManager();

    if (parentLayerManager->InCapturePlayer())
    {
        const std::string& metadataFile = parentLayerManager->GetPathToTargetMetadataFile();

        if (metadataFile.length() > 0)
        {
            // Read the metadata file and store the contents in a structure.
            TraceMetadata traceMetadata;
            traceMetadata.mFrameInfo = new FrameInfo;

            bool bReadMetadataFileSuccessfully = ReadMetadataFile(metadataFile, &traceMetadata);

            if (bReadMetadataFileSuccessfully)
            {
                gtASCIIString traceContents;
                bool bReadTraceSuccessfully = LoadTraceFile(traceMetadata.mPathToTraceFile, traceContents);

                if (bReadTraceSuccessfully)
                {
                    // At this point the full trace response text should be loaded into our string and ready to be sent back to the client.
                    mCmdLinkedTrace.Send(traceContents.asCharArray());
                }
                else
                {
                    Log(logERROR, "Failed to read trace file at '%s'.", traceMetadata.mPathToTraceFile.c_str());
                }
            }
            else
            {
                Log(logERROR, "Failed to read metadata file at '%s'.", metadataFile.c_str());
            }

            // Destroy the FrameInfo instance that was created above.
            SAFE_DELETE(traceMetadata.mFrameInfo);
        }
        else
        {
            Log(logERROR, "Failed to locate valid path to trace metadata file.");
        }
    }
    else
    {
        gtASCIIString traceHeaderBlock;
        bool bBuiltHeaderSuccessfully = GenerateLinkedTraceHeader(traceHeaderBlock);

        if (bBuiltHeaderSuccessfully)
        {
            bool bKeypressTrigger = parentLayerManager->IsTraceTriggeredByKeypress();

            // Collect a trace and generate the trace metadata string. Write the trace and metadata files to disk.
            std::string metadataXMLString;
            bool bWriteMetadataSuccessful = WriteTraceAndMetadataFiles(traceHeaderBlock, inFullResponseString, metadataXMLString);

            // If the trace wasn't triggered by a keypress, we'll need to send a response back through either of the following commands.
            CommandResponse& frameCaptureWithSaveResponse = (inbSaveResponseToFile == true) ? parentLayerManager->mCmdFrameCaptureWithSave : mCmdLinkedTrace;

            if (bWriteMetadataSuccessful)
            {
                // We only need to send the response back through a request if the client triggered collection.
                if (!bKeypressTrigger)
                {
                    // Check if we want to cache the response to disk, or return it as-is.
                    if (inbSaveResponseToFile)
                    {
                        if (bWriteMetadataSuccessful)
                        {
                            // Send a response back to the client indicating which trace metadata file was written to disk.
                            frameCaptureWithSaveResponse.Send(metadataXMLString.c_str());
                        }
                        else
                        {
                            Log(logERROR, "Failed to write trace metadata XML.\n");
                            frameCaptureWithSaveResponse.Send("Failed");
                        }
                    }
                    else
                    {
                        // Send a response containing the API and GPU trace text.
                        frameCaptureWithSaveResponse.Send(inFullResponseString.asCharArray());
                    }
                }
                else
                {
                    Log(logMESSAGE, "Successfully traced frame %d.\n", parentLayerManager->GetFrameCount());
                }
            }
            else
            {
                Log(logERROR, "Failed to write trace metadata XML.\n");

                // If a failed trace collection was triggered by a command, we need to respond with an error message.
                if (!bKeypressTrigger)
                {
                    frameCaptureWithSaveResponse.Send("Failed");
                }
            }
        }
    }
}

//--------------------------------------------------------------------------
/// Return a string with all of the logged API call data in line-delimited
/// text format. This is used within the Timeline view in the client.
/// \return A string of all of the logged API calls captured during frame render.
//--------------------------------------------------------------------------
std::string MultithreadedTraceAnalyzerLayer::GetAPITraceTXT()
{
    // A switch to determine at the last moment whether or not we should send our generated response back to the client.
    bool bWriteResponseString = false;
    const GPS_TIMESTAMP frameStartTime = mFramestartTime;

    // Concatenate all of the logged call lines into a single string that we can send to the client.
    gtASCIIString appendString = "";

    std::unordered_map<DWORD, ThreadTraceData*>::iterator traceIter;

    for (traceIter = mThreadTraces.begin(); traceIter != mThreadTraces.end(); ++traceIter)
    {
        ThreadTraceData* currentTrace = traceIter->second;
        const TimingLog& currentTimer = currentTrace->mAPICallTimer;
        GPS_TIMESTAMP timeFrequency = currentTimer.GetTimeFrequency();
        size_t numEntries = currentTrace->mLoggedCallVector.size();

        // When using the updated trace format, include a preamble section for each traced thread.

        // Write the trace type, API, ThreadID, and count of APIs traced.
        appendString += "//==API Trace==";
        appendString += "\n";

        appendString += "//API=";
        appendString += GetAPIString();
        appendString += "\n";

        appendString += "//ThreadID=";
        appendString += DWORDToString(traceIter->first);
        appendString += "\n";

        appendString += "//ThreadAPICount=";
        appendString += UINT64ToString((UINT64)numEntries);
        appendString += "\n";

        for (size_t entryIndex = 0; entryIndex < numEntries; ++entryIndex)
        {
            // Get each logged call by index.
            const CallsTiming& callTiming = currentTimer.GetTimingByIndex(entryIndex);

            // Divide by the clock frequency and scale to milliseconds.
            double dTimeFrequency = (double)timeFrequency.QuadPart;
            double deltaStartTime = (double)((callTiming.m_startTime.QuadPart - frameStartTime.QuadPart) * 1000.0) / dTimeFrequency;
            double deltaEndTime = (double)((callTiming.m_endTime.QuadPart - frameStartTime.QuadPart) * 1000.0) / dTimeFrequency;

            const APIEntry* callEntry = currentTrace->mLoggedCallVector[entryIndex];

            // This exists as a sanity check. If a duration stretches past this point, we can be pretty sure something is messed up.
            // This signal value is basically random, with the goal of it being large enough to catch any obvious duration errors.
            if (deltaEndTime > 8000000000.0f)
            {
                const char* functionName = callEntry->GetAPIName();
                Log(logWARNING, "The duration for APIEntry '%s' with index '%d' is suspicious. Tracing the application may have hung, producing inflated results.\n", functionName, entryIndex);
            }

            callEntry->AppendAPITraceLine(appendString, deltaStartTime, deltaEndTime);
        }

        bWriteResponseString = true;
    }

    // If for some reason we failed to write a valid response string, reply with a known failure signal so the client handles it properly.
    if (!bWriteResponseString)
    {
        appendString += "NODATA";
    }

    return appendString.asCharArray();

    //return traceString.str();
}

//-----------------------------------------------------------------------------
/// Return GPU-time in text format, to be parsed by the Client and displayed as its own timeline.
/// \return A line-delimited, ASCII-encoded, version of the GPU Trace data.
//-----------------------------------------------------------------------------
std::string MultithreadedTraceAnalyzerLayer::GetGPUTraceTXT()
{
    return "NODATA";
}

//--------------------------------------------------------------------------
/// Check if the newly-started frame should be automatically traced at a specific frame.
/// \returns An eTraceType value, corresponding to the value set in the server config.
//--------------------------------------------------------------------------
int MultithreadedTraceAnalyzerLayer::GetTraceTypeFlags()
{
    int traceTypeFlags = kTraceType_None;

    // It's time to capture a frame. Check the trace type that was requested.
    if (GetParentLayerManager()->IsAutocaptureFrame())
    {
        traceTypeFlags = SG_GET_INT(OptionTraceType);
    }

    return traceTypeFlags;
}

//--------------------------------------------------------------------------
/// Retrieve the number of API calls invoked within a traced frame.
/// \returns The number of API calls invoked within a traced frame.
//--------------------------------------------------------------------------
uint32 MultithreadedTraceAnalyzerLayer::GetNumTracedAPICalls()
{
    uint32 totalAPICalls = 0;

    // Step through each ThreadTraceData and add up the total number of API calls.
    ThreadIdToTraceData::iterator threadDataIter;

    for (threadDataIter = mThreadTraces.begin(); threadDataIter != mThreadTraces.end(); ++threadDataIter)
    {
        ThreadTraceData* traceData = threadDataIter->second;
        totalAPICalls += static_cast<uint32>(traceData->mLoggedCallVector.size());
    }

    return totalAPICalls;
}

//--------------------------------------------------------------------------
/// Retrieve the number of Draw calls that occurred within a traced frame.
/// \returns The number of Draw calls that occurred within a traced frame.
//--------------------------------------------------------------------------
uint32 MultithreadedTraceAnalyzerLayer::GetNumTracedDrawCalls()
{
    uint32 totalDrawCalls = 0;

    // Step through each ThreadTraceData and add up the total number of API calls.
    ThreadIdToTraceData::iterator threadDataIter;

    for (threadDataIter = mThreadTraces.begin(); threadDataIter != mThreadTraces.end(); ++threadDataIter)
    {
        ThreadTraceData* traceData = threadDataIter->second;

        size_t numCalls = traceData->mLoggedCallVector.size();

        for (size_t callIndex = 0; callIndex < numCalls; ++callIndex)
        {
            APIEntry* currentEntry = traceData->mLoggedCallVector[callIndex];

            if (currentEntry->IsDrawCall())
            {
                totalDrawCalls++;
            }
        }
    }

    return totalDrawCalls;
}

//--------------------------------------------------------------------------
/// This is called before the target function call, and will setup a new ThreadTraceData
/// structure if necessary, but will also track the start time for a function call.
/// \return Nothing.
//--------------------------------------------------------------------------
void MultithreadedTraceAnalyzerLayer::BeforeAPICall()
{
    // Find the correct ThreadTraceData instance and inject the precall time.
    // A single thread will only ever deal with tracing one function at a time, so we can
    // leave "this" traced function's start time in the per-thread data.
    DWORD threadId = osGetCurrentThreadId();
    ThreadTraceData* currentThreadData = FindOrCreateThreadData(threadId);
    currentThreadData->m_startTime = currentThreadData->mAPICallTimer.GetRaw();
}

//--------------------------------------------------------------------------
/// Invoked when the MultithreadedTraceAnalyzerLayer is created.
/// \param inType The incoming type of interface being created.
/// \param pInPtr An interface pointer being created.
/// \returns True if creation was successful.
//--------------------------------------------------------------------------
bool MultithreadedTraceAnalyzerLayer::OnCreate(CREATION_TYPE inType, void* pInPtr)
{
    PS_UNREFERENCED_PARAMETER(inType);
    PS_UNREFERENCED_PARAMETER(pInPtr);

    return true;
}

//--------------------------------------------------------------------------
/// Invoked when the MultithreadedTraceAnalyzerLayer is finished and about to be destroyed.
/// \param inType The incoming type of instance being destroyed.
/// \param pInPtr An instance pointer being destroyed.
/// \returns True if destruction was successful.
//--------------------------------------------------------------------------
bool MultithreadedTraceAnalyzerLayer::OnDestroy(CREATION_TYPE inType, void* pInPtr)
{
    PS_UNREFERENCED_PARAMETER(inType);
    PS_UNREFERENCED_PARAMETER(pInPtr);

    Clear();

    return true;
}

//--------------------------------------------------------------------------
/// Step through all of the timestamps collected when tracing an application.
/// Verify that they are all unique within a thread, as expected.
/// \returns True if all timestamps were verified as being unique. False if non-zero timestamps match exactly.
//--------------------------------------------------------------------------
bool MultithreadedTraceAnalyzerLayer::VerifyUniqueTimestamps() const
{
    bool bAllUniqueTimestamps = true;

    // Step through all results and confirm that all timestamps are unique.
    ThreadIdToTraceData::const_iterator tracedThreadIter;

    for (tracedThreadIter = mThreadTraces.begin(); tracedThreadIter != mThreadTraces.end(); ++tracedThreadIter)
    {
        DWORD threadId = tracedThreadIter->first;
        ThreadTraceData* tracedThreadData = tracedThreadIter->second;

        if (bAllUniqueTimestamps)
        {
            std::vector<APIEntry*>& apiList = tracedThreadData->mLoggedCallVector;

            // We can only verify if timestamps are unique if there are two or more calls in the traced thread's call vector.
            if (apiList.size() > 1)
            {
                for (size_t apiIndex = 0; apiIndex < (apiList.size() - 1); ++apiIndex)
                {
                    const CallsTiming& currentTiming = tracedThreadData->mAPICallTimer.GetTimingByIndex(apiIndex);
                    const CallsTiming& nextTiming = tracedThreadData->mAPICallTimer.GetTimingByIndex(apiIndex + 1);

                    const bool startMatches = currentTiming.m_startTime.QuadPart == nextTiming.m_startTime.QuadPart;
                    const bool endMatches = currentTiming.m_endTime.QuadPart == nextTiming.m_endTime.QuadPart;

                    if (startMatches || endMatches)
                    {
                        bAllUniqueTimestamps = false;

                        if (!bAllUniqueTimestamps)
                        {
                            Log(logERROR, "Duplicate profiling timestamps detected on thread with Id '%d' at API index '%d'.\n", threadId, (apiIndex - 1));
                        }

                        break;
                    }
                }
            }
            else
            {
                Log(logMESSAGE, "Cannot verify unique timestamps in \"apiList\". Need two or more logged calls per thread.\n");
            }
        }
    }

    return bAllUniqueTimestamps;
}

//--------------------------------------------------------------------------
/// Clear all logged data. Overridden because this TraceAnalyzer collects multiple
/// instances of the same data, so it all has to be deallocated.
//--------------------------------------------------------------------------
void MultithreadedTraceAnalyzerLayer::Clear()
{
    ClearCPUThreadTraceData();
}

//--------------------------------------------------------------------------
/// Find an existing ThreadTraceData instance to drop things into, or create a new one and insert into the map.
/// Each thread will receive its own ThreadTraceData instance to log to, allowing multithreaded collection.
/// \param inThreadId The Id of a thread invoking API calls.
/// \returns A polymorphic ThreadTraceData instance for the thread to log to without any locking.
//--------------------------------------------------------------------------
ThreadTraceData* MultithreadedTraceAnalyzerLayer::FindOrCreateThreadData(DWORD inThreadId)
{
    // We don't need to lock yet. map::find is threadsafe.
    ThreadTraceData* resultTraceData = NULL;
    ThreadIdToTraceData::iterator traceIter = mThreadTraces.find(inThreadId);

    if (traceIter != mThreadTraces.end())
    {
        resultTraceData = traceIter->second;
    }
    else
    {
        // We need to lock here- we're going to insert a new instance into our thread trace map.
        ScopeLock mapInsertionLock(&mTraceMutex);

        // Insert the new ThreadData struct into the map to hold them all.
        resultTraceData = CreateThreadTraceDataInstance();
        mThreadTraces[inThreadId] = resultTraceData;
    }

    return resultTraceData;
}



//--------------------------------------------------------------------------
/// Write a trace's metadata file and return the contenst through the out-param.
/// \param inHeaderString The full response string for a collected linked trace request.
/// \param inResponseString Response string
/// \param outMetadataXML The XML metadata string to return to the client.
/// \returns True if writing the metadata file was succesful.
//--------------------------------------------------------------------------
bool MultithreadedTraceAnalyzerLayer::WriteTraceAndMetadataFiles(const gtASCIIString& inHeaderString, const gtASCIIString& inResponseString, std::string& outMetadataXML)
{
    bool bWrittenSuccessfully = false;

    // Empty out the incoming path to the metadata file. We'll know the exact path later.
    outMetadataXML.assign("");

    // Use this object to pass data into and out of the GetSessionManagaerData() method.
    SessionManagerData smd;

    smd.frameIndex = GetParentLayerManager()->GetFrameCount();

    bool result = SessionManager::Instance()->GetSessionManagerData(smd);

    if (result == false)
    {
        return result;
    }

    gtASCIIString pathToMetadataFile = smd.pathToDataDirectory; //pathToDataDirectory;
    pathToMetadataFile.appendFormattedString("%s", smd.metadataFilename.asCharArray());

    gtString fullMetadataFilepathAsGTString;
    fullMetadataFilepathAsGTString.fromASCIIString(pathToMetadataFile.asCharArray());

    osFile metadataFile(fullMetadataFilepathAsGTString);
    bool bMetadataFileOpened = metadataFile.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

    // If we've successfully opened the metadata file, we'll also attempt to write the trace file.
    if (bMetadataFileOpened == false)
    {
        Log(logERROR, "Failed to open trace metadata file for writing: '%s'\n", smd.metadataFilename.asCharArray());
        return false;
    }

    osFilePath traceFileDirectory;
    traceFileDirectory.setPath(osFilePath::OS_TEMP_DIRECTORY);

    traceFileDirectory.appendSubDirectory(smd.toolDirectory);

    // Construct a filename for the cached trace response.
    gtASCIIString fullTraceFilename;
    fullTraceFilename.appendFormattedString("LinkedTrace-%s-%d-%d-%d-%d-%d-%d.ltr", smd.appName.asASCIICharArray(), smd.year, smd.month, smd.day, smd.hour, smd.minute, smd.second);

    gtASCIIString fullTraceFilePath = smd.pathToDataDirectory;
    fullTraceFilePath.appendFormattedString("%s", fullTraceFilename.asCharArray());

    gtString fullTraceResponseFilepathGTString;
    fullTraceResponseFilepathGTString.fromASCIIString(fullTraceFilePath.asCharArray());

    // Write the contents of the trace response file.
    osFile traceResponseFile(fullTraceResponseFilepathGTString);
    bool bTraceResponseFileOpened = traceResponseFile.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

    if (bTraceResponseFileOpened)
    {
        traceResponseFile.writeString(inHeaderString);
        traceResponseFile.writeString(inResponseString);
        traceResponseFile.close();
    }
    else
    {
        Log(logERROR, "Failed to write trace response to file: '%s'\n", fullTraceResponseFilepathGTString.asASCIICharArray());
    }

    // Write the filename for the associated trace response that was just collected.
    std::string traceFilepathAsString;
    traceFilepathAsString.assign(fullTraceResponseFilepathGTString.asASCIICharArray());

    TraceMetadata metadataToWrite;

    // Insert the location of the metadata file being written out.
    metadataToWrite.mMetadataFilepath = pathToMetadataFile.asCharArray();

    // Insert the path to the cached trace file.
    metadataToWrite.mPathToTraceFile = fullTraceFilePath.asCharArray();

    // Write object files info. It is currently assumed trace and object data will exists simultaneously.
    metadataToWrite.mPathToObjectTreeFile = smd.pathToDataDirectory.asCharArray();
    metadataToWrite.mPathToObjectTreeFile.append("ObjectTree.xml");

    metadataToWrite.mPathToObjectDatabaseFile = smd.pathToDataDirectory.asCharArray();
    metadataToWrite.mPathToObjectDatabaseFile.append("FullObjectDatabase.xml");

    ModernAPIFrameDebuggerLayer* frameDebugger = GetParentLayerManager()->GetFrameDebuggerLayer();

    unsigned char* pngData = NULL;
    unsigned int numBytes = 0;

    // NOTE: Passing in 0 for the width and height will cause the renderer to render the PNG image at the same resolution as the applications frame buffer (i.e full resolution).
    bool bCapturedSuccessfully = frameDebugger->CaptureFrameBuffer(0, 0, &pngData, &numBytes, true);

    if (bCapturedSuccessfully)
    {
        gtASCIIString fullImageFilename;
        fullImageFilename.appendFormattedString("%s_FrameBuffer%d.png", smd.appName.asASCIICharArray(), smd.frameIndex);

        gtASCIIString imageFilepath = smd.pathToDataDirectory;
        imageFilepath.appendFormattedString("%s", fullImageFilename.asCharArray());

        FILE* frameBufferImageFile = fopen(imageFilepath.asCharArray(), "wb");

        if (frameBufferImageFile != NULL)
        {
            fwrite(pngData, sizeof(unsigned char), numBytes, frameBufferImageFile);
            fclose(frameBufferImageFile);

            // Add the captured image's path into the XML metadata.
            metadataToWrite.mPathToFrameBufferImage = imageFilepath.asCharArray();
        }
        else
        {
            Log(logERROR, "Failed to write frame buffer image file.\n");
        }

        SAFE_DELETE_ARRAY(pngData);
    }
    else
    {
        metadataToWrite.mPathToFrameBufferImage = "ERROR - Failed to capture frame buffer image.";
        Log(logERROR, "Failed to capture frame buffer for captured frame.\n");
    }

    // @TODO: This is placeholder for now to get a prototype working.
    // We should also be able to set this to "Capture".
    metadataToWrite.mTraceType = kTraceType_Linked;

    // @TODO: This is temporary until TraceMetadata generation is moved to the LayerManager.
    FrameInfo frameInfo;
    frameDebugger->GetFrameInfo(&frameInfo);

    // Populate the metadata structure with the values stored in the LayerManager.
    metadataToWrite.mFrameInfo = &frameInfo;
    metadataToWrite.mArchitecture = smd.moduleArchitecture;
    metadataToWrite.mAPICallCount = GetNumTracedAPICalls();
    metadataToWrite.mDrawCallCount = GetNumTracedDrawCalls();

    bool bMetadataWriteSuccessful = WriteMetadataFile(&metadataToWrite, pathToMetadataFile.asCharArray(), outMetadataXML);

    if (bMetadataWriteSuccessful)
    {
        bWrittenSuccessfully = true;
    }

    return bWrittenSuccessfully;
}
