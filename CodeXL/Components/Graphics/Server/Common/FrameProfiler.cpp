//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Contains the implementation of the FrameProfiler class which should
///         be utilized by all plugins.
//==============================================================================

#include "FrameProfiler.h"
#include "parser.h"
#include "xml.h"
#include "CommandStrings.h"
#include "misc.h"
#include "SharedGlobal.h"
#include "GPUPerfAPIUtils/GPUPerfAPIUtil.h"
#include "Compressor.h"
#include <AMDTBaseTools/Include/gtASCIIString.h>
#ifdef _WIN32
    #include "ADLUtil.h"
#endif

#include <sstream>

#define USE_POWER_STATE

// Comment in the line below to calculate and log the old weighted averaging
// method and compare values to the new method
//#define COMPARE_AVERAGE

// Comment in the line below to use the original weighted average method
//#define USE_WEIGHTED_AVERAGE

std::string FrameProfiler::m_sLastGPAError;

//=============================================================================
/// FrameProfiler
///
/// Collects draw call, api trace, profiler, and frame debugger information
/// in response to processing associated commands. The data is sent back when
/// the EndFrame method is called
//=============================================================================
FrameProfiler::FrameProfiler()
    : m_ulDrawCallCounter(0),
      m_ulDrawCallsInPrevFrame(0),
      m_pActiveProfileCommand(NULL),
      m_currentSessionID(1),
      m_currentPass(0),
      m_requiredPassCount(0),
      m_lastBeginDrawCallCount(1),
      m_passStarted(false),
      m_profileRetries(0)
{

#if defined GDT_INTERNAL && defined _WIN32
    AddProcessor("ThreadTrace", "Thread Trace", "TT", "", DISPLAY, m_threadTracer);
#endif

    AddCommand(CONTENT_TEXT,  "Profiler",         "Profiler",         "Profiler.xml",         DISPLAY, INCLUDE, m_profilerData);
    AddCommand(CONTENT_TEXT,  "ProfilerUpdateFrequency",  "ProfilerUpdateFrequency", "ProfilerUpdateFrequency",  NO_DISPLAY, INCLUDE, m_dwProfilerUpdateFrequency);
    AddCommand(CONTENT_XML,   "DrawcallProfiler", "DrawcallProfiler", "DrawcallProfiler.xml", DISPLAY, INCLUDE, m_drawcallProfilerData);

    AddCommand(CONTENT_XML,   "ProfileRange", "ProfileRange", "ProfileRange.xml", DISPLAY, INCLUDE, m_ProfileRangeData);

    AddCommand(CONTENT_XML,   "CounterInfo",      "Counter Info",     "CounterInfo.xml",      DISPLAY, INCLUDE, m_counterInfo);
    AddCommand(CONTENT_TEXT,  "CounterSelect",    "Counter Select",   "CounterSelect.txt",    DISPLAY, INCLUDE, m_CounterSelectResponse);
    AddCommand(CONTENT_XML,   "Statistics",       "Statistics",       "Stats.xml",            DISPLAY, INCLUDE, m_Stats);

    //monitor params: time in ms, timing/counter
    AddCommand(CONTENT_XML,   "Monitor",          "Monitor",          "Monitor.xml",          DISPLAY, INCLUDE, m_monitorResponse);

    //incremental params: # of samples
    AddCommand(CONTENT_XML,   "Incremental",      "Incremental",      "Incremental.xml",      DISPLAY, INCLUDE, m_incrementalResponse);

    //draw filter params: t/f
    AddCommand(CONTENT_XML,   "DrawFilter",       "DrawFilter",       "DrawFilter.xml",       DISPLAY, INCLUDE, m_drawFilterResponse);

    m_drawcallProfilerData.SetEditableContentAutoReply(false);
    m_ProfileRangeData.SetEditableContentAutoReply(false);
    m_CounterSelectResponse.SetEditableContentAutoReply(false);
    m_monitorResponse.SetEditableContentAutoReply(false);
    m_incrementalResponse.SetEditableContentAutoReply(false);
    m_drawFilterResponse.SetEditableContentAutoReply(false);

    m_profilerDrawCalls = "";
    m_executingMonitor = false;
    m_executingIncremental = false;

    m_measureStartEndCPUTime = false;
    m_measureStartEndGPUTime = false;

    m_drawFilter = false;
    m_monitorTiming = false;

    m_sampleStarted = false;
    m_gpuBusyAvailable = false;
    m_dwProfilerUpdateFrequency = 100;

    SetLayerName("FrameProfiler");
}

//-----------------------------------------------------------------------------
/// Counts the number of draw calls, collects DrawCall information, and profiles
/// the draw call as necessary
/// \param rDrawCall the drawcall to capture XML description of, profile, and/or execute
//-----------------------------------------------------------------------------
void FrameProfiler::OnDrawCall(IDrawCall& rDrawCall)
{
    // This counter resets after reached max number of drawcalls.
    m_ulDrawCallCounter++;

    if (m_drawFilter || (m_executingIncremental && (m_ulDrawCallCounter > m_currentIncDrawIndex)))
    {
        return;
    }

#if defined GDT_INTERNAL && defined _WIN32
    m_threadTracer.MidTrace(m_ulDrawCallCounter);
#endif

    if (m_profilerData.IsActive())
    {
        OnDrawCall_Profile(rDrawCall);
    }
    else if (m_drawcallProfilerData.IsActive())
    {
        unsigned long singleDrawcallIndex = m_drawcallProfilerData.GetValue();

        // should this draw call be profiled?
        if (m_ulDrawCallCounter == singleDrawcallIndex)
        {
            OnDrawCall_Profile(rDrawCall);
        }
        else
        {
            // Execute the call without any sampling
            rDrawCall.Execute();
        }
    }
    else if (m_ProfileRangeData.IsActive() && (m_ProfileRangeDrawCallPairs.empty() == false))
    {
        OnDrawCall_ProfileRange(rDrawCall);
    }
    else
    {
        // Execute the call without any sampling
        rDrawCall.Execute();
    }
}

//--------------------------------------------------------------------------------------
/// This function is called by OnDrawCall(IDrawCall& rDrawCall) for full frame profile
/// and single draw call profile.
/// \param rDrawCall the drawcall to capture XML description of, profile, and/or execute
//--------------------------------------------------------------------------------------
void FrameProfiler::OnDrawCall_Profile(IDrawCall& rDrawCall)
{
    m_SampledDrawCalls.push_back(m_ulDrawCallCounter);

    BeginProfilerSample();

    rDrawCall.Execute();

    EndProfilerSample();

    if (m_currentPass == 1)
    {
        gtASCIIString xmlString = rDrawCall.GetXML();
#ifdef GDT_INTERNAL
        GetShaderCRC(rDrawCall, xmlString);
#endif //GDT_INTERNAL
        xmlString += XML("hash", rDrawCall.GetHash().asCharArray());
        m_profilerDrawCalls += GetDrawCallXML(m_ulDrawCallCounter, xmlString.asCharArray()).asCharArray();
    }
}

//--------------------------------------------------------------------------------------
/// This function is called by OnDrawCall(IDrawCall& rDrawCall) for range profile.
/// \param rDrawCall the drawcall to capture XML description of, profile, and/or execute
//--------------------------------------------------------------------------------------
void FrameProfiler::OnDrawCall_ProfileRange(IDrawCall& rDrawCall)
{
    if (m_ProfileRangeDrawCallPairs.empty() == false)
    {
        // Top most draw call pair
        DrawCallPair pair = *(m_ProfileRangeDrawCallPairs.begin());

        // Begin range
        if (m_ulDrawCallCounter == pair.startCall)
        {
            m_SampledDrawCalls.push_back(m_ulDrawCallCounter);
            BeginProfilerSample();
        }

        rDrawCall.Execute();

        // End Range
        if (m_ulDrawCallCounter == pair.endCall)
        {
            EndProfilerSample();

            // now get the drawcall xml that corresponds to this profiled data
            // only get the draw call info on the first pass, because all the subsequent passes must have the same draw call info.
            if (m_currentPass == 1)
            {
                DrawCallPair pair1 = *(m_ProfileRangeDrawCallPairs.begin());
                gtASCIIString xmlString = rDrawCall.GetXML();
#ifdef GDT_INTERNAL
                GetShaderCRC(rDrawCall, xmlString);
#endif //GDT_INTERNAL
                xmlString += XML("hash", rDrawCall.GetHash().asCharArray());
                m_profilerDrawCalls += GetDrawCallXML(pair1.startCall, xmlString.asCharArray()).asCharArray();
            }

            // reached the end of the range, remove this range from the list
            m_ProfileRangeDrawCallPairs.pop_front();
        }
    }
    else
    {
        rDrawCall.Execute();
    }
}

//-----------------------------------------------------------------------------
/// Similar to OnDrawCall, but only starts the profiling. Used primarily for
/// profiling a block of draw calls.
//-----------------------------------------------------------------------------
void FrameProfiler::OnDrawCallBegin()
{
    // This counter resets after reached max number of drawcalls.
    m_ulDrawCallCounter++;

    if (m_profilerData.IsActive())
    {
        // start profiling this individual draw call
        m_SampledDrawCalls.push_back(m_ulDrawCallCounter);
        BeginProfilerSample();
    }
    else if (m_drawcallProfilerData.IsActive())
    {
        unsigned long singleDrawcallIndex = m_drawcallProfilerData.GetValue();

        // should this draw call be profiled?
        if (m_ulDrawCallCounter == singleDrawcallIndex)
        {
            m_SampledDrawCalls.push_back(m_ulDrawCallCounter);
            BeginProfilerSample();
        }
    }
    else if (m_ProfileRangeData.IsActive() && (m_ProfileRangeDrawCallPairs.empty() == false))
    {
        // Top most draw call pair
        DrawCallPair pair = *(m_ProfileRangeDrawCallPairs.begin());

        // Begin range
        if (m_ulDrawCallCounter == pair.startCall)
        {
            // Start profiling this range of drawcalls
            m_SampledDrawCalls.push_back(m_ulDrawCallCounter);
            BeginProfilerSample();
        }
    }
}

//-----------------------------------------------------------------------------
/// Similar to OnDrawCall, but ends the profiling. Used primarily for
/// profiling a block of draw calls.
/// \param rDrawCall the drawcall to capture XML description of or profile
//-----------------------------------------------------------------------------
void FrameProfiler::OnDrawCallEnd(IDrawCall& rDrawCall)
{
    if (m_profilerData.IsActive())
    {
        // stop profiling this individual draw call
        EndProfilerSample();

        if (m_currentPass == 1)
        {
            gtASCIIString xmlString = rDrawCall.GetXML();
#ifdef GDT_INTERNAL
            GetShaderCRC(rDrawCall, xmlString);
#endif //GDT_INTERNAL
            xmlString += XML("hash", rDrawCall.GetHash().asCharArray());
            m_profilerDrawCalls += GetDrawCallXML(m_ulDrawCallCounter, xmlString.asCharArray()).asCharArray();
        }
    }
    else if (m_drawcallProfilerData.IsActive())
    {
        unsigned long singleDrawcallIndex = m_drawcallProfilerData.GetValue();

        // should this draw call be profiled?
        if (m_ulDrawCallCounter == singleDrawcallIndex)
        {
            EndProfilerSample();

            if (m_currentPass == 1)
            {
                gtASCIIString xmlString = rDrawCall.GetXML();
#ifdef GDT_INTERNAL
                GetShaderCRC(rDrawCall, xmlString);
#endif //GDT_INTERNAL
                xmlString += XML("hash", rDrawCall.GetHash().asCharArray());
                m_profilerDrawCalls += GetDrawCallXML(m_ulDrawCallCounter, xmlString.asCharArray()).asCharArray();
            }
        }
    }
    else if (m_ProfileRangeData.IsActive() && (m_ProfileRangeDrawCallPairs.empty() == false))
    {
        // End Range
        DrawCallPair pair = *(m_ProfileRangeDrawCallPairs.begin());

        if (m_ulDrawCallCounter == pair.endCall)
        {
            // Stop profiling this range of drawcalls
            EndProfilerSample();

            // now get the drawcall xml that corresponds to this profiled data
            // only get the draw call info on the first pass, because all the subsequent passes must have the same draw call info.
            if (m_currentPass == 1)
            {
                DrawCallPair pair1 = *(m_ProfileRangeDrawCallPairs.begin());
                gtASCIIString xmlString = rDrawCall.GetXML();
#ifdef GDT_INTERNAL
                GetShaderCRC(rDrawCall, xmlString);
#endif //GDT_INTERNAL
                xmlString += XML("hash", rDrawCall.GetHash().asCharArray());
                m_profilerDrawCalls += GetDrawCallXML(pair1.startCall, xmlString.asCharArray()).asCharArray();
            }

            // reached the end of the range, remove this range from the list
            m_ProfileRangeDrawCallPairs.pop_front();
        }
    }
}

//-----------------------------------------------------------------------------
/// Attempts to load the supplied DLLname with a .dll extension and then again
/// with "Internal.dll" appended.
/// \param api the name of the GPA dll to load without
///      the ".dll" extension
/// \return true if either the external or internal dll could be loaded; false otherwise
//-----------------------------------------------------------------------------
bool FrameProfiler::LoadProfilerDLL(GPA_API_Type api)
{
    PsAssert(api < GPA_API__LAST);
    Log(logMESSAGE, "Loading GPA for API: %d\n", api);

    const char* errorMessage = NULL;
    bool GPALoaded = m_GPALoader.Load(SG_GET_PATH(GPUPerfAPIPath), api, &errorMessage);

    if (GPALoaded == false)
    {
        Log(logERROR, "Failed to load GPA. Load error: %s", errorMessage);
        return false;
    }

    if (m_GPALoader.GPA_RegisterLoggingCallback(GPA_LOGGING_ERROR_AND_MESSAGE, (GPA_LoggingCallbackPtrType)&FrameProfiler::GPALoggingCallback) != GPA_STATUS_OK)
    {
        Log(logERROR, "Failed to register profiler logging callback.");
    }

#if defined GDT_INTERNAL && defined _WIN32

    if (m_GPALoader.GPA_RegisterLoggingDebugCallback(GPA_LOG_DEBUG_MESSAGE, (GPA_LoggingDebugCallbackPtrType)&FrameProfiler::GPALoggingDebugCallback) != GPA_STATUS_OK)
    {
        Log(logERROR, "Failed to register profiler debug logging callback.");
    }

#endif // GDT_INTERNAL

    return true;
}

//-----------------------------------------------------------------------------
/// Selects a specified set of counters
/// \param rSelectionRequest a text command response with a comma-separated
///        list of counter names to endable
//-----------------------------------------------------------------------------
void FrameProfiler::DoCounterSelect(TextCommandResponse& rSelectionRequest)
{
    if (m_GPALoader.Loaded() == false)
    {
        rSelectionRequest.SendError("Unable to select counters: profiler DLL is not available.");
        return;
    }

    //GPA is opened here to permit validation of the enabled counters
    // It is closed after the counters have been checked
    Log(logMESSAGE, "FrameProfiler::DoCounterSelect: Device being used: %x\n", GetProfilerDevicePtr());

    GPA_Status openStatus = m_GPALoader.GPA_OpenContext(GetProfilerDevicePtr()) ;

    if (openStatus != GPA_STATUS_OK)
    {
        if (openStatus == GPA_STATUS_ERROR_DRIVER_NOT_SUPPORTED)
        {
            rSelectionRequest.SendError("AMD Radeon Software version is unsupported. Please switch to a different version and try again.");
            return;
        }

        if (openStatus != GPA_STATUS_ERROR_COUNTERS_ALREADY_OPEN)
        {
            rSelectionRequest.SendError("Unable to enable selected counters: %s\n", GetStatusString(openStatus).c_str());
            return;
        }

        return;
    }

    // turn the string into a gtASCIIString for easier parsing
    gtASCIIString input = rSelectionRequest.GetValue();

    m_enabledCounters.clear();

    if (input.length() == 0)
    {
        // enabling no counters
        m_GPALoader.GPA_CloseContext();
        rSelectionRequest.Send("OK,0");
        return;
    }

    if ("All" == input)
    {
        GPA_Status status = m_GPALoader.GPA_EnableAllCounters();

        if (status != GPA_STATUS_OK)
        {
            rSelectionRequest.SendError("Unable to enable all counters. Please contact the AMD GPU Developer Tools team.");
            return;
        }

        // all counters were enabled, now get the # of passes to return to the client
        gpa_uint32 numPasses = 0;
        m_GPALoader.GPA_GetPassCount(&numPasses);

        rSelectionRequest.Send(FormatText("OK,%u", numPasses).asCharArray());
        return;
    }

    // split the list of counter indexes by commas
    // try to enable each index individually
    std::list< gtASCIIString > inputSplit;
    input.Split(",", true, inputSplit);
    int paramCount = (int)inputSplit.size();

    for (int i = 0 ; i < paramCount ; i++)
    {
        int32 index;
        bool intOk = inputSplit.begin()->toIntNumber(index);
        inputSplit.pop_front();

        if (intOk)
        {
            GPA_Status gpaStatus = m_GPALoader.GPA_EnableCounter(index);

            if (gpaStatus != GPA_STATUS_OK)
            {
                const char* pCounterName = NULL;
                m_GPALoader.GPA_GetCounterName(index, &pCounterName);
                Log(logERROR, "Unable to enable counter index %d (%s)\n", index, pCounterName);
                m_enabledCounters.clear();
                m_GPALoader.GPA_CloseContext();
                rSelectionRequest.SendError("Unable to enable counter index %d. Please contact the AMD GPU Developer Tools team.", index);
                return;
            }

            m_enabledCounters.push_back(index);
        }
        else
        {
            Log(logERROR, "Invalid counter index in GPA_Profiler::CounterSelect.\n");
            m_enabledCounters.clear();
            m_GPALoader.GPA_CloseContext();
            rSelectionRequest.SendError("Invalid counter index (expected int value): %s", inputSplit.begin()->asCharArray());
            return;
        }
    }

    // all counters were enabled, now get the # of passes to return to the client
    gpa_uint32 numPasses = 0;
    m_GPALoader.GPA_GetPassCount(&numPasses);

    GPA_Status closeCountersStatus = m_GPALoader.GPA_CloseContext();

    if (closeCountersStatus != GPA_STATUS_OK)
    {
        Log(logERROR, "GPA_CloseContext failed: %s.\n", GetStatusString(closeCountersStatus).c_str());
        rSelectionRequest.SendError("Failed to close context: %s", GetStatusString(closeCountersStatus).c_str());
        return;
    }

    rSelectionRequest.Send(FormatText("OK,%u", numPasses).asCharArray());
    return;
}


//-----------------------------------------------------------------------------
/// \return true if the profiler is profiling; false otherwise
//-----------------------------------------------------------------------------
bool FrameProfiler::IsProfiling()
{
    return (m_profilerData.IsActive() || m_drawcallProfilerData.IsActive() || m_ProfileRangeData.IsActive());
}

//-----------------------------------------------------------------------------
// Send the text information for the counters which are valid on the particular
// hardware that the app is currently running on. This will appear as the list
// of counters displayed in the Frame Profiler on the client.
//
//    format:
//
//<counterset>
//    <counter>
//       <name>
//       <\name>
//       <description>
//       <\description>
//       <datatype>
//       <\datatype>
//       <usage>
//       <\usage>
//       <type>
//       <\type>
//    <\counter>
//    ...
//<\counterset>
//<hwinfo>
// <deviceid>
// <devicedesc>
//<\hwinfo>
//-----------------------------------------------------------------------------
void FrameProfiler::SendCounterInformationString(CommandResponse& rRequest)
{
    if (m_GPALoader.Loaded() == false)
    {
        rRequest.SendError("Cannot generate counter information string: Profiler dll was not loaded.");
        return;
    }

    std::stringstream countersString;

    Log(logMESSAGE, "FrameProfiler::SendCounterInformationString: Device being used: %x\n", GetProfilerDevicePtr());

    GPA_Status openStatus = m_GPALoader.GPA_OpenContext(GetProfilerDevicePtr());

    if (openStatus != GPA_STATUS_OK)
    {
        if (openStatus == GPA_STATUS_ERROR_DRIVER_NOT_SUPPORTED)
        {
            rRequest.SendError("AMD Radeon Software version is unsupported. Please switch to a different version and try again.");
            return;
        }

        if (openStatus != GPA_STATUS_ERROR_COUNTERS_ALREADY_OPEN)
        {
            rRequest.SendError("Unable to enable selected counters: %s\n", GetStatusString(openStatus).c_str());
            return;
        }
    }

    gpa_uint32 count = 0;
    m_GPALoader.GPA_GetNumCounters(&count);

    for (gpa_uint32 i = 0 ; i < count ; i++)
    {

        // get counter name
        const char* counterName = NULL;
        GPA_Status s = m_GPALoader.GPA_GetCounterName(i, &counterName);

        if (s != GPA_STATUS_OK)
        {
            rRequest.SendError("Unable to access counter name for index %u: %s", i, GetStatusString(s).c_str());
            m_GPALoader.GPA_CloseContext();
            return;
        }

        // get counter description
        const char* counterDesc = NULL;
        s = m_GPALoader.GPA_GetCounterDescription(i, &counterDesc);

        if (s != GPA_STATUS_OK)
        {
            rRequest.SendError("Unable to access counter description for '%s': %s", counterName, GetStatusString(s).c_str());
            m_GPALoader.GPA_CloseContext();
            return;
        }

        // Get counter data type
        GPA_Type counterDataType;
        s = m_GPALoader.GPA_GetCounterDataType(i, &counterDataType);

        if (s != GPA_STATUS_OK)
        {
            rRequest.SendError("Unable to access counter data type for '%s': %s", counterName, GetStatusString(s).c_str());
            m_GPALoader.GPA_CloseContext();
            return;
        }

        const char* typeStr = NULL;

        m_GPALoader.GPA_GetDataTypeAsStr(counterDataType, &typeStr);

        // Get Counter Usage Type
        GPA_Usage_Type counterUsageType = GPA_USAGE_TYPE__LAST;

        s = m_GPALoader.GPA_GetCounterUsageType(i, &counterUsageType);

        if (s != GPA_STATUS_OK)
        {
            rRequest.SendError("Unable to access counter usage type for '%s': %s", counterName, GetStatusString(s).c_str());
            m_GPALoader.GPA_CloseContext();
            return;
        }

        const char* usageTypeStr = NULL;

        s = m_GPALoader.GPA_GetUsageTypeAsStr(counterUsageType, &usageTypeStr);

        if (s != GPA_STATUS_OK)
        {
            rRequest.SendError("Unable to access counter usage string for '%s' type %u: %s", counterName, counterUsageType, GetStatusString(s).c_str());
            m_GPALoader.GPA_CloseContext();
            return;
        }

        // convert data to XML
        countersString << "<counter>";

        countersString << "<name>" << XMLEscape(counterName).asCharArray() << "</name>";

        countersString << "<description>" << XMLEscape(counterDesc).asCharArray() << "</description>";

        countersString << "<datatype>" << typeStr << "</datatype>";

        countersString << "<usage>" << usageTypeStr << "</usage>";

        countersString << "</counter>";
    }

    // get device ID
    gtASCIIString hwInfoString;
    gpa_uint32 deviceID = 0;
    GPA_Status gotOk = m_GPALoader.GPA_GetDeviceID(&deviceID);

    if (gotOk == GPA_STATUS_OK)
    {
        gtASCIIString devIDStr;
        devIDStr.appendFormattedString("%u", deviceID);
        hwInfoString += XML("deviceid", devIDStr.asCharArray());
    }

    // get device description (name)
    const char* desc = NULL;
    gotOk = m_GPALoader.GPA_GetDeviceDesc(&desc);

    if (gotOk == GPA_STATUS_OK)
    {
        hwInfoString += XML("devicedesc", desc);
    }

    // close counters
    GPA_Status closeCountersStatus = m_GPALoader.GPA_CloseContext();

    if (closeCountersStatus != GPA_STATUS_OK)
    {
        rRequest.SendError("Unable to close counters: %s", GetStatusString(closeCountersStatus).c_str());
        return;
    }

    // send successful response to client
    gtASCIIString xmlString = XML("counterset", countersString.str().c_str());
    xmlString += XML("hwinfo", hwInfoString.asCharArray());
    rRequest.Send(xmlString.asCharArray());
    return;
}


bool FrameProfiler::HandleProfilerRequest(void* deviceToMonitor, CommandResponse& rProfilerXML, const char* configSectionLabel)
{
    if (m_GPALoader.Loaded())
    {
        gtASCIIString GPAFile = SG_GET_PATH(CounterFile);

        // Note: the GPA back end may allocate resources on the device
        Log(logMESSAGE, "FrameProfiler::HandleProfilerRequest: Device being used: %x\n", GetProfilerDevicePtr());
        GPA_Status openStatus = m_GPALoader.GPA_OpenContext(deviceToMonitor) ;

        if (openStatus != GPA_STATUS_OK)
        {
            if (openStatus == GPA_STATUS_ERROR_DRIVER_NOT_SUPPORTED)
            {
                Log(logERROR, "AMD Radeon Software version is unsupported. Please switch to a different version and try again.");
                return false;
            }

            if (openStatus != GPA_STATUS_ERROR_COUNTERS_ALREADY_OPEN)
            {
                Log(logERROR, "Unable to enable selected counters: %s\n", GetStatusString(openStatus).c_str());
                return false;
            }
        }

        ////////////////////////////////////////////////////////////////////////////////////
        // Removed: Nov 4th 2015. This GPA function is for internal CPU side GPA profiling.
        // GPA profiling
        //m_GPALoader.GPA_InternalProfileStart();
        ////////////////////////////////////////////////////////////////////////////////////

        if (m_enabledCounters.empty() == false)
        {
            for (int i = 0 ; i < (int)m_enabledCounters.size() ; i++)
            {
                GPA_Status enableOk = m_GPALoader.GPA_EnableCounter(m_enabledCounters[i]);

                if (enableOk != GPA_STATUS_OK)
                {
                    Log(logWARNING, "GPA_EnableCounter failed: %s.\n", GetStatusString(enableOk).c_str());
                }
            }
        }
        else
        {
            uint32 countersRead;
            const char* errorTxt;
            GPA_Status enableStatus = GPA_EnableCountersFromFile(&m_GPALoader, GPAFile.asCharArray(), &countersRead, &errorTxt, configSectionLabel);

            if (enableStatus != GPA_STATUS_OK)
            {
                gtASCIIString errorStr = FormatText("Note: No GPA counter enable file loaded. (%s).\n", errorTxt);
                Log(logMESSAGE, "%s", errorStr.asCharArray());
                // error so enable all counters
                m_GPALoader.GPA_EnableAllCounters();
            }
        }

        uint32 enabledCount = 0;

        if ((m_GPALoader.GPA_GetEnabledCount(&enabledCount)) != GPA_STATUS_OK)
        {
            Log(logWARNING, "GPA_GetEnabledCount returned an error\n");
        }

        if (enabledCount > 0)
        {
            if ((m_GPALoader.GPA_GetPassCount(&m_requiredPassCount)) != GPA_STATUS_OK)
            {
                rProfilerXML.SendError((char*) XML("Error", "GPA_GetPassCount returned an error\n").asCharArray());
                return false;
            }
        }
        else
        {
            rProfilerXML.SendError((char*) XML("Error", "No counters were enabled.").asCharArray());
            return false;
        }
    }
    else
    {
        rProfilerXML.SendError((char*) XML("Error", "Profiler not available since unable to load GPUPerfAPI.").asCharArray());
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
/// Simplifies accessing the HandleProfilerRequest method of the profiler. This
/// method hides some of the additional parameters that would be needed for the
/// HandleProfilerRequest method.
/// \param pDevice a pointer to the device or context that is being profiled
/// \param configSectionLabel a string identifying which section of the counter
///    file should be used for this pass of profiling
/// \return true if the pass could be started and all counters initialized; false otherwise
//-----------------------------------------------------------------------------
bool FrameProfiler::BeginProfilerPass(void* pDevice, const char* configSectionLabel)
{
    // Force GPU power state to high
#ifdef _WIN32
#ifdef USE_POWER_STATE

    if (m_currentPass == 0)
    {
        AMDTADLUtils::Instance()->UseHighPowerMode();
    }

#endif
#endif

    // increment pass count
    m_currentPass++;

    // Clear the array that holds the sampled calls before each pass
    m_SampledDrawCalls.clear();

    if (m_ProfileRangeData.IsActive())
    {
        // Clear the array that holds the draw call pairs for range profiles
        m_ProfileRangeDrawCallPairs.clear();

        // Create an gtASCIIString for easier parsing
        gtASCIIString rangeProfilerCommandStr = m_ProfileRangeData.GetValue();
        int32 nPos = rangeProfilerCommandStr.find("?NoHashData=1", 0);

        if (nPos > 0)
        {
            m_ProfileRangeData.NoHashData(true);
            rangeProfilerCommandStr.truncate(0, nPos - 1);
        }

        // split the list of draw call indices by commas
        std::list< gtASCIIString > rangeProfilerCommandSplit;
        rangeProfilerCommandStr.Split(",", true, rangeProfilerCommandSplit);
        int paramCount = (int)rangeProfilerCommandSplit.size();

        // Make sure we have even number of draw call indices
        if ((paramCount > 1) && !(paramCount % 2))
        {
            // Separate out each pair to have the range profiler work with specific ranges
            for (int i = 0; i < paramCount; i += 2)
            {
                DrawCallPair pair;
                int index;
                rangeProfilerCommandSplit.begin()->toIntNumber(index);
                pair.startCall = (unsigned int)index;
                rangeProfilerCommandSplit.pop_front();
                rangeProfilerCommandSplit.begin()->toIntNumber(index);
                pair.endCall = (unsigned int)index;
                rangeProfilerCommandSplit.pop_front();

                // check for validity of this pair
                if (pair.startCall > pair.endCall)
                {
                    // Do not stop the profile, just skip this pair since it is invalid
                    Log(logERROR, "Invalid draw call range pair at index %d with startcall=%d and endcall=%d\n", (i / 2) + 1, pair.startCall, pair.endCall);
                    m_ProfileRangeData.SendError("Invalid draw call range pair. Please contact the AMD GPU Developer Tools team.");
                    return false;
                }

                // check for validity of the order in which pairs are supplied
                if (m_ProfileRangeDrawCallPairs.empty() == false && m_ProfileRangeDrawCallPairs.back().endCall >= pair.startCall)
                {
                    Log(logERROR, "Invalid order of draw calls, pairs must be unique and specified in ascending order");
                    m_ProfileRangeData.SendError("Invalid draw call range order. Please contact the AMD GPU Developer Tools team.");
                    return false;
                }

                m_ProfileRangeDrawCallPairs.push_back(pair);
            }
        }
        else
        {
            Log(logERROR, "Odd number of draw call indices supplied, cannot determine the correct range");
            m_ProfileRangeData.SendError("Invalid profile command format. Please contact the AMD GPU Developer Tools team.");
            return false;
        }
    }

    if (IsProfiling() && m_currentPass == 1)
    {
        // first pass means this is a new profile request
        m_profilerData.makeEmpty();
        m_profilerDrawCalls = "";

        // start profiling
        bool reqResult = HandleProfilerRequest(pDevice, *m_pActiveProfileCommand, configSectionLabel);

        if (!reqResult)
        {
            return false;
        }
    }

    if (IsProfiling())
    {
        // Update the streaming profiler command (if the active request is one) with a status update.
        AddProfiledCall(*m_pActiveProfileCommand, FormatString("                Processing Pass %5d  /%5d", m_currentPass, m_requiredPassCount).c_str(), 0);

        // profiler is performing a multi-pass operation and is incomplete. Do not try to start another profile, just return.
        UpdateProfiler(1, true);
        return true;
    }

    return true;
}

// This function should be called at the start of every measurable element.
// e.g. SetRenderTarget, Clear, Draw
void FrameProfiler::UpdateProfiler(uint32 totalDrawCallCount, bool timePasses)
{
    // first draw call
    if (totalDrawCallCount == 1)
    {
        if (m_GPALoader.Loaded())
        {
            // first pass
            if (m_currentPass == 1)
            {

#ifdef GDT_INTERNAL
                // first pass, need to start sampling
                m_GPALoader.GPA_InternalSetDrawCallCounts(m_ulDrawCallsInPrevFrame);
#endif

                if (StatusCheck(m_GPALoader.GPA_BeginSession(&m_currentSessionID)) != GPA_STATUS_OK)
                {
                    Log(logWARNING, "GPA_BeginSession returned an error\n");
                }
            }

            // start of a new pass
            if (timePasses && StatusCheck(m_GPALoader.GPA_BeginPass()) != GPA_STATUS_OK)
            {
                Log(logWARNING, "GPA_BeginPass returned an error\n");
            }

            m_lastBeginDrawCallCount = 1;
        }
    }
}

//-----------------------------------------------------------------------------
/// Simplifies accessing the EndProfilerPass method of the profiler.
//-----------------------------------------------------------------------------
void FrameProfiler::EndProfilerPass()
{
    if (m_GPALoader.Loaded() && m_currentPass > 0)
    {
        GPA_Status endPassStatus = m_GPALoader.GPA_EndPass();

        if (endPassStatus == GPA_STATUS_ERROR_VARIABLE_NUMBER_OF_SAMPLES_IN_PASSES)
        {
            // bad pass, restart sampling
            m_GPALoader.GPA_EndSession();
            m_currentPass = 0;
            m_profileRetries++;

            if (m_profileRetries == 1)
            {
                ////////////////////////////////////////////////////////////////////////////////////
                // Removed: Nov 4th 2015. This GPA function is for internal CPU side GPA profiling.
                // m_GPALoader.GPA_InternalProfileStop("c:\\PS2GPAProfile.csv");
                ////////////////////////////////////////////////////////////////////////////////////

                GPA_Status closeCountersStatus = m_GPALoader.GPA_CloseContext();
                PsAssert(closeCountersStatus == GPA_STATUS_OK);

                char* errorDescrip = NULL;

                if (closeCountersStatus != GPA_STATUS_OK)
                {
                    errorDescrip = (char*)"GPA_CloseContext() failed";
                }
                else
                {
                    errorDescrip = (char*)"Number of draw calls per pass varies. Unable to profile.";
                }

                CounterMeasureFail(errorDescrip);

                m_profileRetries = 0;
            }

            return;
        }
        else if (endPassStatus != GPA_STATUS_OK)
        {
            PsAssert(endPassStatus == GPA_STATUS_OK);
        }

        //      Log( logMESSAGE, "Profiler pass %u of %u completed.\n", m_currentPass, m_requiredPassCount );
        if (m_currentPass >= m_requiredPassCount)
        {

            // Restore the original power state
#ifdef _WIN32
#ifdef USE_POWER_STATE
            AMDTADLUtils::Instance()->ResumeNormalPowerMode();
#endif
#endif

            // done with the passes
            StatusCheck(m_GPALoader.GPA_EndSession());
            m_profileRetries = 0;
            m_requiredPassCount = 0;
            m_currentPass = 0;

            uint32 samples;
            StatusCheck(m_GPALoader.GPA_GetSampleCount(m_currentSessionID, &samples));

            // this will generate an error string if needed
            assert(m_pActiveProfileCommand != NULL);
            SendCounterResultString(*m_pActiveProfileCommand, m_profilerDrawCalls.c_str());

            ////////////////////////////////////////////////////////////////////////////////////
            // Removed: Nov 4th 2015. This GPA function is for internal CPU side GPA profiling.
            //m_GPALoader.GPA_InternalProfileStop("c:\\PS2GPAProfile.csv");
            ////////////////////////////////////////////////////////////////////////////////////

            GPA_Status closeCountersStatus = m_GPALoader.GPA_CloseContext();

            if (closeCountersStatus != GPA_STATUS_OK)
            {
                Log(logERROR, "GPA CloseContext failed\n");  // %s\n", m_GPALoader.GPA_GetStatusAsStr( closeCountersStatus ) );
            }
        }
    }
}

//-----------------------------------------------------------------------------
/// Simplifies accessing the BeginProfilerSample method of the profiler.
//-----------------------------------------------------------------------------
void FrameProfiler::BeginProfilerSample()
{
    if (m_GPALoader.Loaded() && m_currentPass > 0)
    {
        if (m_profilerData.IsActive())
        {
            // if this is false totalDrawCallCounts were not in sequence. Problem in calling code.
            PsAssert(m_lastBeginDrawCallCount == m_ulDrawCallCounter);
        }
        else if (m_drawcallProfilerData.IsActive())
        {
            // This function is called only once and m_lastBeginDrawCallCount is then reset to 1
            PsAssert(m_lastBeginDrawCallCount == 1);
        }

        // begin the sample
        GPA_Status status = m_GPALoader.GPA_BeginSample(m_ulDrawCallCounter);

        m_lastBeginDrawCallCount++;

        if (StatusCheck(status) != GPA_STATUS_OK)
        {
            if (FrameProfiler::m_sLastGPAError.empty())
            {
                CounterMeasureFail("Unable to begin counter sample. Please contact the AMD GPU Developer Tools team.");
            }
            else
            {
                CounterMeasureFail(FrameProfiler::m_sLastGPAError.c_str());
            }
        }
    }
}

//-----------------------------------------------------------------------------
/// Simplifies accessing the EndProfilerSample method of the profiler.
//-----------------------------------------------------------------------------
void FrameProfiler::EndProfilerSample()
{
    if (m_GPALoader.Loaded() && m_currentPass > 0)
    {
        // end the sample
        GPA_Status status = m_GPALoader.GPA_EndSample();

        if (StatusCheck(status) != GPA_STATUS_OK)
        {
            CounterMeasureFail("Unable to end counter sample. Please contact the AMD GPU Developer Tools team.");
        }
    }
}


//-----------------------------------------------------------------------------
/// Profiler monitor command fail.
/// report to client and cleanup.
/// \param error Error message
//-----------------------------------------------------------------------------
void FrameProfiler::CounterMeasureFail(const char* error)
{
    if (m_monitorResponse.IsActive())
    {
        m_monitorResponse.SendError(error);
    }

    if (m_incrementalResponse.IsActive())
    {
        m_incrementalResponse.SendError(error);
    }

    m_executingMonitor = false;
    m_executingIncremental = false;

    m_measureStartEndGPUTime = false;
    m_measureStartEndCPUTime = false;

    m_currentPass = 0;

    assert(m_pActiveProfileCommand != NULL);

    if (m_pActiveProfileCommand != NULL)
    {
        m_pActiveProfileCommand->SendError(error);

        // close the stream;
        if (m_pActiveProfileCommand->GetStreamingEnabled())
        {
            m_pActiveProfileCommand->Send(NULL, 0);
        }

        m_pActiveProfileCommand = NULL;
    }

    if (m_GPALoader.Loaded())
    {
        m_GPALoader.GPA_EndSample();
        m_GPALoader.GPA_EndPass();
        m_GPALoader.GPA_EndSession();
        m_GPALoader.GPA_CloseContext();
    }
}


//-----------------------------------------------------------------------------
/// Signals the frame analyzer that a frame is beginning and it should
/// initialize based on any received commands
//-----------------------------------------------------------------------------
void FrameProfiler::BeginFrame()
{
#if defined GDT_INTERNAL && defined _WIN32
    m_threadTracer.Initialize(GetProfilerDevicePtr(), GetProfilerConfigLabel().asCharArray());
    m_threadTracer.BeginTrace();
#endif

    bool activePerFrameProfile = m_profilerData.IsActive();
    bool activePerDrawCallProfile = m_drawcallProfilerData.IsActive();
    bool activeRangeProfile = m_ProfileRangeData.IsActive();


    // Should not be both true;
    PsAssert(!(activePerFrameProfile == true && activePerDrawCallProfile == true));

    if (activePerFrameProfile || activePerDrawCallProfile || activeRangeProfile)
    {
        if (activePerFrameProfile)
        {
            m_pActiveProfileCommand = &m_profilerData;
        }
        else if (activePerDrawCallProfile)
        {
            m_pActiveProfileCommand = &m_drawcallProfilerData;
        }
        else if (activeRangeProfile)
        {
            m_pActiveProfileCommand = &m_ProfileRangeData;
        }

        PsAssert(m_pActiveProfileCommand != NULL);

        if (BeginProfilerPass(GetProfilerDevicePtr(), GetProfilerConfigLabel().asCharArray()) == false)
        {
            CounterMeasureFail("Unable to start profiling. Please make sure you are using supported hardware.");
        }
    }

    if (m_drawFilterResponse.IsActive())
    {
        gtASCIIString param = m_drawFilterResponse.GetValue();

        if (param.compareNoCase("t") == 0)
        {
            m_drawFilter = true;
        }
        else if (param.compareNoCase("f") == 0)
        {
            m_drawFilter = false;
        }

        m_drawFilterResponse.Send("Ok");
    }

    bool needToOpenCounters = false;

    if (m_monitorResponse.IsActive() && !m_executingMonitor && !(IsProfiling()))
    {
        // Start the frame monitor

        gtASCIIString input = m_monitorResponse.GetValue();
        std::list< gtASCIIString > inputSplit;
        input.Split(",", true, inputSplit);
        int paramCount = (int)inputSplit.size();

        if (paramCount < 2)
        {
            m_monitorResponse.SendError("Expecting 2 parameters for monitor command.");
            return;
        }

        gtASCIIString timeParam = *inputSplit.begin();
        bool timeOk = timeParam.toUnsignedIntNumber(m_timeToMonitor);

        if (!timeOk)
        {
            m_monitorResponse.SendError("Specified time value not valid.");
            return;
        }

        inputSplit.pop_front();
        gtASCIIString counterParam = *inputSplit.begin();

        if (counterParam.compareNoCase("timing") == 0)
        {
            m_monitorTiming = true;
        }
        else if (counterParam.compareNoCase("counter") == 0)
        {
            m_monitorTiming = false;
        }
        else
        {
            m_monitorResponse.SendError("Unknown mode parameter in monitor command.");
            return;
        }

        //assign initial state
        m_measureStartEndGPUTime = true;
        m_measureStartEndCPUTime = true;
        m_monitorTime.ResetTimer();
        m_executingMonitor = true;
        m_outstandingSessions  = 0;
        m_currentWaitSessionID = 1;   //Since we just opened the counters the first sessionID will always be 1
        m_avgGPUTime = 0;
        m_avgCPUTime = 0;
        m_aveDraws = 0;
        m_monitorFrameCount = 0;
        m_gpuBusy = 0.0;

        m_gpuTimeFrameCount = 0;
        m_gpuBusyFrameCount = 0;
        m_cpuFrameCount = 0;

        needToOpenCounters = true;
    }

    if (m_incrementalResponse.IsActive() && !m_executingIncremental && !m_executingMonitor)
    {
        //do incremental
        m_currentIncDrawIndex = 1;
        m_executingIncremental = true;
        m_measureStartEndGPUTime = true;
        m_measureStartEndCPUTime = true;

        m_outstandingSessions  = 0;
        m_currentWaitSessionID = 1;   //Since we just opened the counters the first sessionID will always be 1
        m_avgGPUTime = 0;
        m_avgCPUTime = 0;

        m_cpuPassTiming.clear();
        m_gpuPassTiming.clear();

        needToOpenCounters = true;
    }


    if (needToOpenCounters)
    {
        if (!m_GPALoader.Loaded())
        {
            CounterMeasureFail("Profiling dll was not loaded.");
            return;
        }

        Log(logMESSAGE, "FrameProfiler::BeginFrame: Device being used: %x\n", GetProfilerDevicePtr());
        GPA_Status openStatus = m_GPALoader.GPA_OpenContext(GetProfilerDevicePtr()) ;

        if (openStatus != GPA_STATUS_OK)
        {
            if (openStatus == GPA_STATUS_ERROR_DRIVER_NOT_SUPPORTED)
            {
                std::string errorMsg = "AMD Radeon Software version unsupported.";
                errorMsg.append(GetStatusString(openStatus).c_str());
                CounterMeasureFail(errorMsg.c_str());
                return;
            }

            if (openStatus != GPA_STATUS_ERROR_COUNTERS_ALREADY_OPEN)
            {
                std::string errorMsg = "Unable to open counters: ";
                errorMsg.append(GetStatusString(openStatus).c_str());
                CounterMeasureFail(errorMsg.c_str());
                return;
            }
        }

        if (m_monitorTiming)
        {
            GPA_Status gpaStatus = m_GPALoader.GPA_EnableCounterStr("GPUTime");

            if (gpaStatus != GPA_STATUS_OK)
            {
                CounterMeasureFail("Unable to enable counter GPUTime.");
                return;
            }
        }
        else
        {
            GPA_Status gpaStatus = m_GPALoader.GPA_EnableCounterStr("GPUBusy");

            if (gpaStatus != GPA_STATUS_OK)
            {
                // just open a timestamp counter instead to allow sampling code to function as usual
                // value will be ignored. saves complicating start/end sampling and value read logic for no gain.
                GPA_Status gpaStatus1 = m_GPALoader.GPA_EnableCounterStr("GPUTime");

                if (gpaStatus1 != GPA_STATUS_OK)
                {
                    CounterMeasureFail("Unable to enable counter GPUTime.");
                    return;
                }

                m_gpuBusyAvailable = false;
            }
            else
            {
                m_gpuBusyAvailable = true;
            }
        }
    }


    if (m_counterInfo.IsActive())
    {
        SendCounterInformationString(m_counterInfo);
    }

    if (m_CounterSelectResponse.IsActive())
    {
        DoCounterSelect(m_CounterSelectResponse);
    }

    if (m_Stats.IsActive())
    {
        m_Stats.Send(GetStatsXML().asCharArray());
    }

    if (m_measureStartEndCPUTime)
    {
        m_CPUTime.ResetTimer();
    }

    // finish a gpu timing sample that already started
    if (m_sampleStarted && m_GPALoader.Loaded())
    {
        if (m_GPALoader.GPA_EndSample() != GPA_STATUS_OK)
        {
            CounterMeasureFail("Unable to end sample.");
            return;
        }

        if (m_GPALoader.GPA_EndPass() != GPA_STATUS_OK)
        {
            CounterMeasureFail("Unable to end pass.");
            return;
        }

        if (m_GPALoader.GPA_EndSession() != GPA_STATUS_OK)
        {
            CounterMeasureFail("Unable to end Session.");
            return;
        }

        m_sampleStarted = false;
        m_outstandingSessions++;
    }

    if (m_measureStartEndGPUTime && m_GPALoader.Loaded())
    {
        // deal with start/end timing, set up counters
        uint32 currentSessionID = 0;

#ifdef GDT_INTERNAL
        m_GPALoader.GPA_InternalSetDrawCallCounts(m_ulDrawCallsInPrevFrame);
#endif

        if (m_GPALoader.GPA_BeginSession(&currentSessionID) != GPA_STATUS_OK)
        {
            CounterMeasureFail("Unable to begin sampling.");
            return;
        }

        if (m_GPALoader.GPA_BeginPass() != GPA_STATUS_OK)
        {
            CounterMeasureFail("Unable to begin pass.");
            return;
        }

        if (m_GPALoader.GPA_BeginSample(0) != GPA_STATUS_OK)
        {
            CounterMeasureFail("Unable to begin sample.");
            return;
        }

        m_sampleStarted = true;
    }
}


//-----------------------------------------------------------------------------
/// Signals the frame analyzer that the frame is ended and any collected data should be sent
//-----------------------------------------------------------------------------
void FrameProfiler::EndFrame()
{
#if defined GDT_INTERNAL && defined _WIN32
    m_threadTracer.EndTrace();
    m_threadTracer.SendResults();
#endif

    // track cpu time (also draw count)
    if (m_measureStartEndCPUTime)
    {
        double timeThisFrame = m_CPUTime.LapDouble();

        if (m_executingIncremental)
        {
            m_cpuPassTiming.push_back((unsigned long)timeThisFrame);
        }

        // increment the number of frames that the CPU count has been active. An average time is needed so rather
        // than use a weighted average, devide the total time by this value
        m_cpuFrameCount++;

#ifdef COMPARE_AVERAGE
        static float64 avgCPUTime = 0.0f; // used to store value using old method of calculation for comparison
        static unsigned int aveDraws = 0; // used to store value using old method of calculation for comparison
#endif

        if (m_avgCPUTime == 0)
        {
#ifdef COMPARE_AVERAGE
            avgCPUTime = timeThisFrame;
#endif
            m_avgCPUTime = timeThisFrame;
            m_minCPUTime = timeThisFrame;
            m_maxCPUTime = timeThisFrame;
        }
        else
        {
            // add the time taken this frame. The average will be calculated later when the value is sent to the
            // client in XML format
            m_avgCPUTime += timeThisFrame;
#ifdef USE_WEIGHTED_AVERAGE
            m_avgCPUTime /= 2;
#endif
#ifdef COMPARE_AVERAGE
            // technically, this is a weighted average, not a true average of the CPU Time.
            // I'm not sure what Derek intended.
            avgCPUTime += timeThisFrame;
            avgCPUTime /= 2;
#endif

            if (timeThisFrame < m_minCPUTime)
            {
                m_minCPUTime = timeThisFrame;
            }

            if (timeThisFrame > m_maxCPUTime)
            {
                m_maxCPUTime = timeThisFrame;
            }
        }

        if (m_aveDraws == 0)
        {
            m_aveDraws = m_ulDrawCallCounter;
            m_minDraws = m_ulDrawCallCounter;
            m_maxDraws = m_ulDrawCallCounter;
#ifdef COMPARE_AVERAGE
            aveDraws = m_ulDrawCallCounter;
#endif
        }
        else
        {
            m_aveDraws += m_ulDrawCallCounter;
#ifdef USE_WEIGHTED_AVERAGE
            m_aveDraws /= 2;
#endif
#ifdef COMPARE_AVERAGE
            // this is also a weighted average
            aveDraws += m_ulDrawCallCounter;
            aveDraws /= 2;
#endif

            if (m_ulDrawCallCounter < m_minDraws)
            {
                m_minDraws = m_ulDrawCallCounter;
            }

            if (m_ulDrawCallCounter > m_maxDraws)
            {
                m_maxDraws = m_ulDrawCallCounter;
            }
        }

#ifdef COMPARE_AVERAGE
        LogConsole(logMESSAGE, "***m_avgCPUTime: %lf (%lf), min: %lf, max: %lf, timeThisFrame: %lf, frames: %ld\n", m_avgCPUTime / m_cpuFrameCount, avgCPUTime, m_minCPUTime, m_maxCPUTime, timeThisFrame, m_cpuFrameCount);
        LogConsole(logMESSAGE, "***m_avgDraws: %ld (%ld), min: %ld, max: %ld, timeThisFrame: %lf, frames: %ld\n", m_aveDraws / m_cpuFrameCount, aveDraws, m_minDraws, m_maxDraws, timeThisFrame, m_cpuFrameCount);
#endif
    }

    // this can be executed after monitoring has finished
    // - takes time to get data from the hw
    if (m_outstandingSessions > 0 && m_GPALoader.Loaded())
    {
        bool readyResult = false;
        GPA_Status sessionStatus;

        while ((sessionStatus = m_GPALoader.GPA_IsSessionReady(&readyResult, m_currentWaitSessionID)) == GPA_STATUS_ERROR_SESSION_NOT_FOUND)
        {
            // skipping a session which got overwritten
            m_currentWaitSessionID++;
            m_outstandingSessions--;

            if (m_outstandingSessions == 0)
            {
                break;
            }
        }

        if (readyResult)
        {
            m_outstandingSessions--;

            if (m_monitorTiming)
            {
                // do monitor frame timestamp update
                float64 fGPUTime = 0;
                uint32 enabledCounterIndex1;

                if (m_GPALoader.GPA_GetEnabledIndex(0, &enabledCounterIndex1) != GPA_STATUS_OK)
                {
                    CounterMeasureFail("Unable to retrieve enabled counters.");
                    return;
                }

                if (m_GPALoader.GPA_GetSampleFloat64(m_currentWaitSessionID, 0, enabledCounterIndex1, &fGPUTime) != GPA_STATUS_OK)
                {
                    CounterMeasureFail("Unable to retrieve counter result.");
                    return;
                }

                m_currentWaitSessionID++;
                uint64 gpuTimeThisFrame = (uint64)fGPUTime;

                if (m_executingIncremental)
                {
                    m_gpuPassTiming.push_back(gpuTimeThisFrame);
                }

#ifdef COMPARE_AVERAGE
                static uint64 avgGPUTime = 0; // used to store value using old method of calculation for comparison
#endif
                m_gpuTimeFrameCount++;

                if (m_avgGPUTime == 0)
                {
                    m_avgGPUTime = gpuTimeThisFrame;
#ifdef COMPARE_AVERAGE
                    avgGPUTime = gpuTimeThisFrame;
#endif
                    m_minGPUTime = gpuTimeThisFrame;
                    m_maxGPUTime = gpuTimeThisFrame;
                }
                else
                {
                    m_avgGPUTime += gpuTimeThisFrame;
#ifdef USE_WEIGHTED_AVERAGE
                    m_avgGPUTime /= 2;
#endif
#ifdef COMPARE_AVERAGE
                    avgGPUTime += gpuTimeThisFrame;
                    avgGPUTime /= 2;
#endif

                    if (gpuTimeThisFrame < m_minGPUTime)
                    {
                        m_minGPUTime = gpuTimeThisFrame;
                    }

                    if (gpuTimeThisFrame > m_maxGPUTime)
                    {
                        m_maxGPUTime = gpuTimeThisFrame;
                    }
                }

#ifdef COMPARE_AVERAGE
                LogConsole(logMESSAGE, "***AvgGPUTime: %ld (%ld), min: %ld, max: %ld, timeThisFrame: %ld, frames: %ld\n", m_avgGPUTime / m_gpuTimeFrameCount, avgGPUTime, m_minGPUTime, m_maxGPUTime, gpuTimeThisFrame, m_gpuTimeFrameCount);
#endif
            }
            else
            {
                // monitor counter collection is active (as opposed to timing)
                float64 gpuBusyTime = 0.0;
                uint32 enabledCounterIndex1 = 0;

                if (m_gpuBusyAvailable)
                {
                    if (m_GPALoader.GPA_GetEnabledIndex(0, &enabledCounterIndex1) != GPA_STATUS_OK)
                    {
                        CounterMeasureFail("Unable to retrieve enabled counters.");
                        return;
                    }

                    if (m_GPALoader.GPA_GetSampleFloat64(m_currentWaitSessionID, 0, enabledCounterIndex1, &gpuBusyTime) != GPA_STATUS_OK)
                    {
                        CounterMeasureFail("Unable to retrieve counter result.");
                        return;
                    }
                }

                m_currentWaitSessionID++;

                // update gpu busy ave,min,max
#ifdef COMPARE_AVERAGE
                static float64 gpuBusy = 0.0f; // used to store value using old method of calculation for comparison
#endif
                m_gpuBusyFrameCount++;

                if (m_gpuBusy == 0.0)
                {
                    m_gpuBusy = gpuBusyTime;
                    m_gpuBusyMin = gpuBusyTime;
                    m_gpuBusyMax = gpuBusyTime;
#ifdef COMPARE_AVERAGE
                    gpuBusy = gpuBusyTime;
#endif
                }
                else
                {
                    m_gpuBusy += gpuBusyTime;
#ifdef USE_WEIGHTED_AVERAGE
                    m_gpuBusy /= 2;
#endif
#ifdef COMPARE_AVERAGE
                    gpuBusy += gpuBusyTime;
                    gpuBusy /= 2;
#endif

                    if (gpuBusyTime < m_gpuBusyMin)
                    {
                        m_gpuBusyMin = gpuBusyTime;
                    }

                    if (gpuBusyTime > m_gpuBusyMax)
                    {
                        m_gpuBusyMax = gpuBusyTime;
                    }
                }

#ifdef COMPARE_AVERAGE
                LogConsole(logMESSAGE, "***AvgGPUBusy: %lf (%lf), min: %lf, max: %lf, valueThisFrame: %lf, frames: %ld\n", m_gpuBusy / m_gpuBusyFrameCount, gpuBusy, m_gpuBusyMin, m_gpuBusyMax, gpuBusyTime, m_gpuBusyFrameCount);
#endif
            }
        }
    }

    if (IsProfiling())
    {
        EndProfilerPass();
    }

    if (m_monitorResponse.IsActive())
    {
        m_monitorFrameCount++;

        unsigned long totalCPUTime = m_monitorTime.Lap();

        //first see if need to stop reading data
        if (totalCPUTime > m_timeToMonitor && m_executingMonitor)
        {
            m_measureStartEndGPUTime = false;
            m_measureStartEndCPUTime = false;
        }

        // next see if completed reading data from HW
        if (m_outstandingSessions == 0 && !m_sampleStarted)
        {
            // finished monitor command. close everything
            m_GPALoader.GPA_CloseContext();

            gtASCIIString out = "";
            //         out += XML( "AvgCPUTime", FormatText( "%lu", m_avgCPUTime ) );
            //         out += XML( "MinCPUTime", FormatText( "%lu", m_minCPUTime ) );
            //         out += XML( "MaxCPUTime", FormatText( "%lu", m_maxCPUTime ) );
#ifndef USE_WEIGHTED_AVERAGE

            if (m_cpuFrameCount > 0)
            {
                m_avgCPUTime /= m_cpuFrameCount;
            }

#endif
#ifdef COMPARE_AVERAGE
            LogConsole(logMESSAGE, "***Sending AvgCPUTime: %lf (frame count %ld)\n", m_avgCPUTime, m_cpuFrameCount);
#endif
            out += XML("AvgCPUTime", FormatText("%lf", m_avgCPUTime).asCharArray());
            out += XML("MinCPUTime", FormatText("%lf", m_minCPUTime).asCharArray());
            out += XML("MaxCPUTime", FormatText("%lf", m_maxCPUTime).asCharArray());

            if (m_monitorTiming)
            {
#ifndef USE_WEIGHTED_AVERAGE

                if (m_gpuTimeFrameCount > 0)
                {
                    m_avgGPUTime /= m_gpuTimeFrameCount;
                }

#endif
#ifdef COMPARE_AVERAGE
                LogConsole(logMESSAGE, "***Sending avgGPUTime: %ld (frame count %ld)\n", avgGPUTime, m_gpuTimeFrameCount);
#endif
                out += XML("AvgGPUTime", FormatText("%I64u", m_avgGPUTime).asCharArray());
                out += XML("MinGPUTime", FormatText("%I64u", m_minGPUTime).asCharArray());
                out += XML("MaxGPUTime", FormatText("%I64u", m_maxGPUTime).asCharArray());
            }
            else if (m_gpuBusyAvailable)
            {
#ifndef USE_WEIGHTED_AVERAGE

                if (m_gpuBusyFrameCount > 0)
                {
                    m_gpuBusy /= m_gpuBusyFrameCount;
                }

#endif
#ifdef COMPARE_AVERAGE
                LogConsole(logMESSAGE, "***Sending AvgGPUBusy: %lf (frame count %ld)\n", m_gpuBusy, m_gpuBusyFrameCount);
#endif
                out += XML("AveGPUBusy", FormatText("%lf", m_gpuBusy).asCharArray());
                out += XML("MinGPUBusy", FormatText("%lf", m_gpuBusyMin).asCharArray());
                out += XML("MaxGPUBusy", FormatText("%lf", m_gpuBusyMax).asCharArray());
            }

#ifndef USE_WEIGHTED_AVERAGE

            if (m_cpuFrameCount > 0)
            {
                m_aveDraws /= m_cpuFrameCount;
            }

#endif
#ifdef COMPARE_AVERAGE
            LogConsole(logMESSAGE, "***Sending AvgDraws: %d (frame count %ld), min: %d, max: %d, framecount: %lu, fps: %f\n", m_aveDraws, m_cpuFrameCount, m_minDraws, m_maxDraws, m_monitorFrameCount, (float)m_monitorFrameCount / ((float)totalCPUTime / 1000.0f));
#endif

            out += XML("FrameCount", FormatText("%lu", m_monitorFrameCount).asCharArray());
            out += XML("Fps", FormatText("%f", (float)m_monitorFrameCount / ((float)totalCPUTime / 1000.0f)).asCharArray());
            out += XML("AvgDraws", FormatText("%u", m_aveDraws).asCharArray());
            out += XML("MinDraws", FormatText("%u", m_minDraws).asCharArray());
            out += XML("MaxDraws", FormatText("%u", m_maxDraws).asCharArray());

            out = XML("Monitor", out.asCharArray());

            m_monitorResponse.Send(out.asCharArray());
            m_executingMonitor = false;
        }
    }

    // do incremental
    if (m_incrementalResponse.IsActive())
    {
        m_currentIncDrawIndex++;

        if (m_currentIncDrawIndex > m_ulDrawCallCounter && m_executingIncremental)
        {
            // done all draw calls
            m_measureStartEndGPUTime = false;
            m_measureStartEndCPUTime = false;
        }

        // next see if completed reading data from HW
        if (m_outstandingSessions == 0 && !m_sampleStarted)
        {
            // finished incremental command. close everything
            m_GPALoader.GPA_CloseContext();

            gtASCIIString out = "";

            if (m_cpuPassTiming.size() != m_gpuPassTiming.size())
            {
                m_incrementalResponse.SendError("CPU timing count does not match GPU timing count");
            }
            else
            {
                for (unsigned int i = 0 ; i < m_cpuPassTiming.size() ; i++)
                {
                    out += XML("CPUTime", FormatText("%I64u", m_cpuPassTiming[i]).asCharArray());
                    out += XML("GPUTime", FormatText("%I64u", m_gpuPassTiming[i]).asCharArray());
                }

                out = XML("Incremental", out.asCharArray());

                m_incrementalResponse.Send(out.asCharArray());
                m_executingIncremental = false;
            }
        }
    }

    // store the number of drawcalls that were in this frame
    // we need the value for comparison with the breakpoint
    m_ulDrawCallsInPrevFrame = m_ulDrawCallCounter;

    // reset the draw call counter for the next frame
    m_ulDrawCallCounter = 0;
}

//-----------------------------------------------------------------------------
/// \return string describing some profiler statistics in XML format
//-----------------------------------------------------------------------------
gtASCIIString FrameProfiler::GetStatsXML()
{
    gtASCIIString out = "";
    out += XML("TotalDrawCalls", FormatText("%lu", m_ulDrawCallsInPrevFrame).asCharArray());
    out += XML("Progress", GetProgress());

    // add derived class' statistics
    out += OnGetStatsXML();

    return XML(STR_STATISTICS, out.asCharArray());
}

//--------------------------------------------------------------------------
// create counter XML output, block to get counter values
//--------------------------------------------------------------------------
void FrameProfiler::SendCounterResultString(CommandResponse& rRequest, const char* pcszDrawCalls)
{
    if (m_GPALoader.Loaded() == false)
    {
        rRequest.SendError("Unable to obtain profile results: profiler dll is not loaded.");
        return;
    }

    uint32 samples = 0;
    GPA_Status status = m_GPALoader.GPA_GetSampleCount(m_currentSessionID, &samples);

    if (StatusCheck(status) != GPA_STATUS_OK)
    {
        rRequest.SendError("Unable to retrieve the number of profiled samples: %s", GetStatusString(status).c_str());
        //result = "Accessing counters failed. Make sure hardware counters are enabled.";
        return;
    }

    bool bSessionReady = false;

    while (bSessionReady == false)
    {
        m_GPALoader.GPA_IsSessionReady(&bSessionReady, m_currentSessionID);
    }

    uint32 enabledCount = 0;
    StatusCheck(m_GPALoader.GPA_GetEnabledCount(&enabledCount));

    std::stringstream result;

    if (m_profilerData.IsActive())
    {
        // add xml header for streamed command
        result << "<?xml version='1.0' encoding='ISO-8859-1'?><XML src='Profiler.xml'>";
    }

    result << "<frame>";

    uint32 size = 0;

    for (uint32 i = 1; i <= samples; i++)
    {
        // Get the draw call index from the list of sampled calls
        uint32 drawcallIndex = m_SampledDrawCalls[i - 1];

        result << "<DrawCall>";
        result << XMLStream("Index", drawcallIndex);

        for (uint32 j = 0 ; j < enabledCount ; j++)
        {
            size++;

            uint32 enabledCounterIndex;
            StatusCheck(m_GPALoader.GPA_GetEnabledIndex(j, &enabledCounterIndex));

            GPA_Type counterDataType;
            StatusCheck(m_GPALoader.GPA_GetCounterDataType(enabledCounterIndex, &counterDataType));

            const char* counterName = NULL;

            StatusCheck(m_GPALoader.GPA_GetCounterName(enabledCounterIndex, &counterName));

            std::stringstream newCounterName;

            if ((counterName[0] >= '0') && (counterName[0] <= '9'))
            {
                newCounterName << '_';
            }

            std::string sCounterName(counterName);

            //replace space and Parenthesis in counterName with '_'
            for (size_t k = 0; k < sCounterName.length(); k++)
            {
                if (sCounterName[k] == ' ' || sCounterName[k] == '(' || sCounterName[k] == ')')
                {
                    sCounterName[k] = '_';
                }
            }

            newCounterName << sCounterName;

            // output opening tag
            result << "<" << newCounterName.str() << ">";

            PsAssert((counterDataType == GPA_TYPE_UINT32)  ||
                     (counterDataType == GPA_TYPE_UINT64)  ||
                     (counterDataType == GPA_TYPE_FLOAT32) ||
                     (counterDataType == GPA_TYPE_FLOAT64)
                    );

            if (counterDataType == GPA_TYPE_UINT32)
            {
                uint32 counterValue = 0;
                StatusCheck(m_GPALoader.GPA_GetSampleUInt32(m_currentSessionID, drawcallIndex, enabledCounterIndex, &counterValue));
                result << counterValue;
            }
            else if (counterDataType == GPA_TYPE_UINT64)
            {
                gpa_uint64 counterValue = 0;
                StatusCheck(m_GPALoader.GPA_GetSampleUInt64(m_currentSessionID, drawcallIndex, enabledCounterIndex, &counterValue));
                result << counterValue;
            }
            else if (counterDataType == GPA_TYPE_FLOAT32)
            {
                float32 counterValue = 0.0f;
                StatusCheck(m_GPALoader.GPA_GetSampleFloat32(m_currentSessionID, drawcallIndex, enabledCounterIndex, &counterValue));
                result << counterValue;
            }
            else if (counterDataType == GPA_TYPE_FLOAT64)
            {
                float64 counterValue = 0.0f;
                GPA_Status status1 = m_GPALoader.GPA_GetSampleFloat64(m_currentSessionID, drawcallIndex, enabledCounterIndex, &counterValue);

                if (StatusCheck(status1))
                {
                    rRequest.SendError("Unable to get profile data for '%s'. Please contact the AMD GPU Developer Tools team.", newCounterName.rdbuf()->str().c_str());
                    return;
                }

                result << counterValue;
            }

            // output closing tag
            result << "</" << newCounterName.str() << ">";
            std::string str = FormatString(" Getting data: drawcall %5d, Counter %s", drawcallIndex, newCounterName.rdbuf()->str().c_str());
            AddProfiledCall(rRequest, str.c_str(), size);

        }

        result << "</DrawCall>";
    }

    result << "</frame>";

    // For the new PerfMarker based UI we do not want to send back the hash data
    // To use this command: "Profiler.xml?Stream=0&NoHashData=1"
    // For use with the PerfMarker profiling we do not use streaming so use: "Profiler.xml?NoHashData=1"
    if (m_pActiveProfileCommand->NoHashData() == false)
    {
        result << XMLStream("drawcalllist", pcszDrawCalls);
    }

    if (m_profilerData.IsActive())
    {
        // add xml pattern for streamed command
        result << "</XML>";
    }

    ////// Exmaple of how to use gzip compression
#ifdef USE_GZIP
    Compress(result.str().c_str());
    unsigned int dataSize = GetCompressedSize();
    unsigned char* pData = GetCompressedData();

    FILE* fPtr = NULL;

    fopen_s(&fPtr, "c:\\temp\\profileSource.xml.gz", "wb");

    if (fPtr != NULL)
    {
        size_t  written = fwrite(pData, 1, dataSize, fPtr);

        if (written == 0)
        {
            Log(logERROR, "Nothing written to compressed XML file.\n");
        }

        fclose(fPtr);
    }

#endif

    // Send data to the client
    rRequest.Send(result.str().c_str());
    AddProfiledCall(rRequest, "Ok", 0);

#ifdef USE_GZIP
    // Clean up the memory
    CleanupCompressedData();
#endif


    // close the stream;
    if (rRequest.GetStreamingEnabled())
    {
        rRequest.Send(NULL, 0);
    }

    return;
}

void FrameProfiler::AddProfiledCall(CommandResponse& rRequest, const char* pcsMessage, int nSize)
{
    if (rRequest.GetStreamingEnabled())
    {
        if (nSize % m_dwProfilerUpdateFrequency == 0)
        {
            // send message
            std::string str = FormatString(" %s", pcsMessage);

            rRequest.Send(pcsMessage, (unsigned int)str.size());
        }
    }
}

std::string FrameProfiler::GetStatusString(GPA_Status status)
{
    std::string result;

    switch (status)
    {
        case GPA_STATUS_ERROR_NULL_POINTER :
            result = "Null pointer";
            break;

        case GPA_STATUS_ERROR_COUNTERS_NOT_OPEN :
            result = "Counters not opened";
            break;

        case GPA_STATUS_ERROR_COUNTERS_ALREADY_OPEN :
            result = "Counters already opened";
            break;

        case GPA_STATUS_ERROR_INDEX_OUT_OF_RANGE :
            result = "Index out of range";
            break;

        case GPA_STATUS_ERROR_NOT_FOUND :
            result = "Not found";
            break;

        case GPA_STATUS_ERROR_ALREADY_ENABLED :
            result = "Already enabled";
            break;

        case GPA_STATUS_ERROR_NO_COUNTERS_ENABLED :
            result = "No counters enabled";
            break;

        case GPA_STATUS_ERROR_NOT_ENABLED :
            result = "Not enabled";
            break;

        case GPA_STATUS_ERROR_SAMPLING_NOT_STARTED :
            result = "Sampling not started";
            break;

        case GPA_STATUS_ERROR_SAMPLING_ALREADY_STARTED :
            result = "Sampling already started";
            break;

        case GPA_STATUS_ERROR_SAMPLING_NOT_ENDED :
            result = "Sampling not ended";
            break;

        case GPA_STATUS_ERROR_NOT_ENOUGH_PASSES :
            result = "Not enough passes";
            break;

        case GPA_STATUS_ERROR_PASS_NOT_ENDED :
            result = "Pass not ended";
            break;

        case GPA_STATUS_ERROR_PASS_NOT_STARTED :
            result = "Pass not started";
            break;

        case GPA_STATUS_ERROR_PASS_ALREADY_STARTED :
            result = "Pass already started";
            break;

        case GPA_STATUS_ERROR_SAMPLE_NOT_STARTED :
            result = "Sample not found";
            break;

        case GPA_STATUS_ERROR_SAMPLE_ALREADY_STARTED :
            result = "Sample already started";
            break;

        case GPA_STATUS_ERROR_SAMPLE_NOT_ENDED :
            result = "sample not ended";
            break;

        case GPA_STATUS_ERROR_CANNOT_CHANGE_COUNTERS_WHEN_SAMPLING :
            result = "Cannot change counters when sampling";
            break;

        case GPA_STATUS_ERROR_SESSION_NOT_FOUND :
            result = "Session not found";
            break;

        case GPA_STATUS_ERROR_SAMPLE_NOT_FOUND :
            result = "Sample not found";
            break;

        case GPA_STATUS_ERROR_SAMPLE_NOT_FOUND_IN_ALL_PASSES :
            result = "Sample not found in all passes";
            break;

        case GPA_STATUS_ERROR_COUNTER_NOT_OF_SPECIFIED_TYPE :
            result = "Counter not of specified type";
            break;

        case GPA_STATUS_ERROR_READING_COUNTER_RESULT :
            result = "Error reading counter result";
            break;

        case GPA_STATUS_ERROR_VARIABLE_NUMBER_OF_SAMPLES_IN_PASSES :
            result = "Variable number of samples in passes";
            break;

        case GPA_STATUS_ERROR_FAILED :
            result = "Failed";
            break;

        case GPA_STATUS_ERROR_HARDWARE_NOT_SUPPORTED :
            result = "Hardware not supported";
            break;

        case GPA_STATUS_ERROR_DRIVER_NOT_SUPPORTED:
            result = "Driver version not supported";
            break;

        default:

            if (status != GPA_STATUS_OK)
            {
                result = "Unknown error";
            }

            break;
    }

    return result;
}

GPA_Status FrameProfiler::StatusCheck(GPA_Status status)
{
    switch (status)
    {
        case GPA_STATUS_ERROR_NULL_POINTER :
            assert(!"Null pointer");
            break;

        case GPA_STATUS_ERROR_COUNTERS_NOT_OPEN :
            assert(!"Counters not opened");
            break;

        case GPA_STATUS_ERROR_COUNTERS_ALREADY_OPEN :
            assert(!"Counters already opened");
            break;

        case GPA_STATUS_ERROR_INDEX_OUT_OF_RANGE :
            assert(!"Index out of range");
            break;

        case GPA_STATUS_ERROR_NOT_FOUND :
            assert(!"Not found");
            break;

        case GPA_STATUS_ERROR_ALREADY_ENABLED :
            assert(!"Already enabled");
            break;

        case GPA_STATUS_ERROR_NO_COUNTERS_ENABLED :
            assert(!"No counters enabled");
            break;

        case GPA_STATUS_ERROR_NOT_ENABLED :
            assert(!"Not enabled");
            break;

        case GPA_STATUS_ERROR_SAMPLING_NOT_STARTED :
            assert(!"Sampling not started");
            break;

        case GPA_STATUS_ERROR_SAMPLING_ALREADY_STARTED :
            assert(!"Sampling already started");
            break;

        case GPA_STATUS_ERROR_SAMPLING_NOT_ENDED :
            assert(!"Sampling not ended");
            break;

        case GPA_STATUS_ERROR_NOT_ENOUGH_PASSES :
            assert(!"Not enough passes");
            break;

        case GPA_STATUS_ERROR_PASS_NOT_ENDED :
            assert(!"Pass not ended");
            break;

        case GPA_STATUS_ERROR_PASS_NOT_STARTED :
            assert(!"Pass not started");
            break;

        case GPA_STATUS_ERROR_PASS_ALREADY_STARTED :
            assert(!"Pass already started");
            break;

        case GPA_STATUS_ERROR_SAMPLE_NOT_STARTED :
            assert(!"Sample not found");
            break;

        case GPA_STATUS_ERROR_SAMPLE_ALREADY_STARTED :
            assert(!"Sample already started");
            break;

        case GPA_STATUS_ERROR_SAMPLE_NOT_ENDED :
            assert(!"sample not ended");
            break;

        case GPA_STATUS_ERROR_CANNOT_CHANGE_COUNTERS_WHEN_SAMPLING :
            assert(!"Cannot change counters when sampling");
            break;

        case GPA_STATUS_ERROR_SESSION_NOT_FOUND :
            assert(!"Session not found");
            break;

        case GPA_STATUS_ERROR_SAMPLE_NOT_FOUND :
            assert(!"Sample not found");
            break;

        case GPA_STATUS_ERROR_SAMPLE_NOT_FOUND_IN_ALL_PASSES :
            assert(!"Sample not found in all passes");
            break;

        case GPA_STATUS_ERROR_COUNTER_NOT_OF_SPECIFIED_TYPE :
            assert(!"Counter not of specified type");
            break;

        case GPA_STATUS_ERROR_READING_COUNTER_RESULT :
            assert(!"Error reading counter result");
            break;

        case GPA_STATUS_ERROR_VARIABLE_NUMBER_OF_SAMPLES_IN_PASSES :
            assert(!"Variable number of samples in passes");
            break;

        case GPA_STATUS_ERROR_FAILED :
            assert(!"Failed");
            break;

        case GPA_STATUS_ERROR_HARDWARE_NOT_SUPPORTED :
            assert(!"Hardware not supported");
            break;

        case GPA_STATUS_ERROR_DRIVER_NOT_SUPPORTED:
            assert(!"Driver version not supported");
            break;

        default:

            if (status != GPA_STATUS_OK)
            {
                assert(!"Unknown error");
            }

            break;
    }

    return status;
}

//-----------------------------------------------------------------------------
void FrameProfiler::GPALoggingCallback(GPA_Logging_Type messageType, const char* message)
{
    if (messageType == GPA_LOGGING_ERROR)
    {
        Log(logERROR, "GPA: %s\n", message);
        FrameProfiler::m_sLastGPAError = message;
    }
    else if (messageType == GPA_LOGGING_MESSAGE)
    {
        Log(logMESSAGE, "GPA: %s\n", message);
    }
    else if (messageType == GPA_LOGGING_TRACE)
    {
        Log(logMESSAGE, "GPA: %s\n", message);
    }
}

//-----------------------------------------------------------------------------
#if defined GDT_INTERNAL && defined _WIN32
void FrameProfiler::GPALoggingDebugCallback(GPA_Log_Debug_Type messageType, const char* message)
{
    if (messageType == GPA_LOG_DEBUG_ERROR)
    {
        Log(logERROR, "GPA: %s\n", message);
        FrameProfiler::m_sLastGPAError = message;
    }
    else if (messageType == GPA_LOG_DEBUG_MESSAGE)
    {
        Log(logMESSAGE, "GPA: %s\n", message);
    }
    else if (messageType == GPA_LOG_DEBUG_TRACE)
    {
        Log(logMESSAGE, "GPA: %s\n", message);
    }
    else
    {
        Log(logMESSAGE, "GPA: %s\n", message);
    }
}
#endif // GDT_INTERNAL

#ifdef GDT_INTERNAL
void FrameProfiler::GetShaderCRC(IDrawCall& rDrawCall, gtASCIIString& ioXMLString)
{
    // get the CRC's for the various shaders
    gtASCIIString hexCode = FormatText("%.16I64x\n", rDrawCall.GetShaderCRC(PIPELINE_STAGE_CS));
    ioXMLString += XML("cscrc", hexCode.asCharArray());

    hexCode = FormatText("%.16I64x\n", rDrawCall.GetShaderCRC(PIPELINE_STAGE_VS));
    ioXMLString += XML("vscrc", hexCode.asCharArray());

    hexCode = FormatText("%.16I64x\n", rDrawCall.GetShaderCRC(PIPELINE_STAGE_HS));
    ioXMLString += XML("hscrc", hexCode.asCharArray());

    hexCode = FormatText("%.16I64x\n", rDrawCall.GetShaderCRC(PIPELINE_STAGE_DS));
    ioXMLString += XML("dscrc", hexCode.asCharArray());

    hexCode = FormatText("%.16I64x\n", rDrawCall.GetShaderCRC(PIPELINE_STAGE_GS));
    ioXMLString += XML("gscrc", hexCode.asCharArray());

    hexCode = FormatText("%.16I64x\n", rDrawCall.GetShaderCRC(PIPELINE_STAGE_PS));
    ioXMLString += XML("pscrc", hexCode.asCharArray());
}

#endif // GDT_INTERNAL
