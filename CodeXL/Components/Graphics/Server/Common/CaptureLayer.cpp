//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Base class for all derived API specific capture layers
//==============================================================================

//=============================================================================
// Constructor
//=============================================================================

#include "CaptureLayer.h"

#include "TimeControlLayer.h"

#if defined _WIN32
LRESULT CALLBACK Capture_StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
#elif defined _LINUX
// nothing to do here
#else
#warning IMPLEMENT ME!
#endif

/// Initializes variables
CaptureLayer::CaptureLayer() : m_pMdoManager(nullptr)
{
    AddCommand(CONTENT_TEXT, "Capture", "Capture", "Capture", NO_DISPLAY, INCLUDE, m_Capture);
    AddCommand(CONTENT_TEXT,  "CaptureUpdateFrequency",  "CaptureUpdateFrequency", "CaptureUpdateFrequency",  NO_DISPLAY, INCLUDE, m_dwCaptureUpdateFrequency);
    m_dwCaptureUpdateFrequency = 100;

    AddCommand(CONTENT_HTML, "Release", "Release", "Release", NO_DISPLAY, INCLUDE, m_Release);
    m_Release.SetEditableContentAutoReply(false);

    AddCommand(CONTENT_TEXT, "CaptureLog", "CaptureLog", "CaptureLog.txt", NO_DISPLAY, INCLUDE, m_CaptureLog);
    AddCommand(CONTENT_TEXT, "TimingLog", "TimingLog", "TimingLog.txt", NO_DISPLAY, INCLUDE, m_TimingLog);
    AddCommand(CONTENT_TEXT, "Res", "Res", "Resources.txt", NO_DISPLAY, INCLUDE, m_Resources);
    AddCommand(CONTENT_TEXT, "ClearInRed", "ClearInRed", "ClearInRed", NO_DISPLAY, INCLUDE, m_ClearInRed);
    AddCommand(CONTENT_TEXT, "EnableSkipPoint", "EnableSkipPoint", "EnableSkipPoint", NO_DISPLAY, INCLUDE, m_EnableSkipPoint);
    AddCommand(CONTENT_TEXT,  "SkipPoint",  "SkipPoint", "SkipPoint",  NO_DISPLAY, INCLUDE, m_dwSkipPoint);
    m_dwSkipPoint = DWORD(-1);

    AddCommand(CONTENT_TEXT,  "LogCallsAsItCaptures",  "LogCallsAsItCaptures", "LogCallsAsItCaptures",  NO_DISPLAY, INCLUDE, m_LogCallsAsItCaptures);
    AddCommand(CONTENT_TEXT,  "LogCallsAsItReplays",  "LogCallsAsItReplays", "LogCallsAsItReplays",  NO_DISPLAY, INCLUDE, m_LogCallsAsItReplays);

    AddCommand(CONTENT_TEXT, "HandleMapsOnCPU", "HandleMapsOnCPU", "HandleMapsOnCPU", NO_DISPLAY, INCLUDE, m_HandleMapsOnCPU);
    AddCommand(CONTENT_TEXT, "FlattenCommandLists", "FlattenCommandLists", "FlattenCommandLists", NO_DISPLAY, INCLUDE, m_FlattenCommandLists);

    AddCommand(CONTENT_TEXT, "CaptureBufferLocks", "CaptureBufferLocks", "CaptureBufferLocks", NO_DISPLAY, INCLUDE, m_CaptureBufferLocks);
    m_CaptureBufferLocks = true;

    AddCommand(CONTENT_TEXT, "CaptureAndPlay", "CaptureAndPlay", "CaptureAndPlay", NO_DISPLAY, INCLUDE, m_CaptureAndPlay);
    m_CaptureAndPlay = true;

    AddCommand(CONTENT_TEXT, "ReplayFrame", "ReplayFrame", "ReplayFrame", NO_DISPLAY, INCLUDE, m_ReplayFrame);
    m_ReplayFrame = true;

    AddCommand(CONTENT_TEXT, "ReplayLoop", "ReplayLoop", "ReplayLoop", NO_DISPLAY, INCLUDE, m_ReplayLoop);
    m_ReplayLoop = false;

    AddCommand(CONTENT_TEXT, "FilterDrawCalls", "FilterDrawCalls", "FilterDrawCalls", NO_DISPLAY, INCLUDE, m_filterDrawcallMask);
    m_filterDrawcallMask = 0;

    // Capture stream to use the same mutex as the FC in order to be thread safe
    m_CapturedAPICalls.SetMutex(&m_CaptureLayerMutex);

    SetLayerName("CaptureLayer");
}

/// Adds a captured call to the stream
/// \param pCap captured call
void CaptureLayer::AddCapturedCall(Capture* pCap)
{
    m_CapturedAPICalls.Add(pCap);

    if (m_Capture.GetStreamingEnabled() && m_Capture.GetAutoCapture() == false)
    {
        //update the client
        if (m_CapturedAPICalls.Size() % m_dwCaptureUpdateFrequency == 0)
        {
            // send call number
            std::string str = FormatString("%5i", m_CapturedAPICalls.Size());
            //send call string
            str += FormatString(" %s", pCap->Print().c_str());

            m_Capture.Send(str.c_str(), (unsigned int)str.size());
        }
    }
}

/// Replays a whole captured frame
/// \return nothing
void CaptureLayer::ReplayFrame()
{
    m_CapturedAPICalls.m_LogCallsAsItReplays = m_LogCallsAsItReplays;

    if (m_EnableSkipPoint)
    {
        // path when skip-point is enabled

        Capture_Skip CS;
        CS.SetSkipPoint(m_dwSkipPoint);

        m_CapturedAPICalls.PlayOverride(&CS);
    }
    else if (m_TimingLog.IsActive())
    {
        TimingLog timing;

        // add a "frame start" time
        timing.Add(osGetCurrentThreadId(), timing.GetRaw());

        Capture_Timing CT(&timing);

        m_CapturedAPICalls.PlayOverride(&CT);

        std::string data = timing.GetLogAsString();

        m_TimingLog.Send(data.c_str(), (unsigned int)data.size());
    }
    else
    {
        // path to replay a frame with no overrides
        m_CapturedAPICalls.PlayCapture();
    }
}

/// Setup everything needed at the beginning of a frame needed to perform an
/// instant capture. Sets up the time control method and other flags needed
/// before the API calls are captured
/// \param timeOverrideMode equivalent of client Time Override Mode
/// \param filterDrawCallsMask Mask to filter out different types of draw calls
/// \param copyMappedBuffersUsingCPU equivalent of client "Copy mapped buffers using CPU" ("True" or "False")
/// \param frameCount Number of frames to capture.
void CaptureLayer::InstantCaptureBeginFrame(int timeOverrideMode, int filterDrawCallsMask, bool copyMappedBuffersUsingCPU, unsigned int frameCount)
{
    LogConsole(logMESSAGE, "Capturing Frame: %ld\n", frameCount);

    bool speedNeeded = true;

    if (timeOverrideMode == TIME_OVERRIDE_NONE)
    {
        speedNeeded = false;
    }

    bool realPause = false;

    if (timeOverrideMode == TIME_OVERRIDE_FREEZE)
    {
        realPause = true;
    }

    // Commands sent from the client to the server to do a frame capture
    // Send up to 2 commands to the TimeControl layer, depending on the time override mode used:
    // Freeze:
    //    TC?realpause=True
    //    TC?Speed=0
    // Slow motion:
    //    TC?realpause=False
    //    TC?Speed=0
    // None:
    //    TC?realpause=False
    TimeControl::Singleton().SetRealPause(realPause);

    if (speedNeeded)
    {
        TimeControl::Singleton().SetPlaySpeed(0.0f);
    }

    // "Copy mapped buffers using CPU" setting in client
    m_HandleMapsOnCPU = copyMappedBuffersUsingCPU;

    // "FilterDrawCalls" setting in client
    m_filterDrawcallMask = filterDrawCallsMask;
}

/// Setup everything needed at the end of a frame needed to perform an
/// instant capture. Sets up certain flags after the API calls have been
/// captured
/// \param flattenCommandLists equivalent of client "Flatten Commandlists"
void CaptureLayer::InstantCaptureEndFrame(bool flattenCommandLists)
{
    // "Flatten Command Lists" setting in client
    if (flattenCommandLists == true)
    {
        m_FlattenCommandLists = flattenCommandLists;
    }

    // "Frame capture on pause" setting in client
    TimeControlLayer::Instance()->SetPausedWithFrameCapture(true);
}
