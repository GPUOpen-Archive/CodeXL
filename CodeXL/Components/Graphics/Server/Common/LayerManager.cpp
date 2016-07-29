//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Layer manager base class. Contains common functionality required
///         by all derived layer managers
//==============================================================================

#if defined (_WIN32)
    #include <windows.h>
#endif
#include "LayerManager.h"
#include "IServerPlugin.h"
#include "SharedGlobal.h"
#include "xml.h"
#include "timer.h"
#include "CaptureLayer.h"
#include "FrameStatsLogger.h"

FrameStatsLogger* LayerManager::mFrameStatsLogger = NULL;

LayerManager::LayerManager()
    : m_LayerList(NULL),
      m_LayerListSize(0),
      m_NumGPUs(1),
      m_FrameCaptured(false),
      m_frameCount(0),
      m_instantCaptureState(INSTANT_CAPTURE_ALLOW)
{
    m_DebugFlag = SG_GET_UINT(OptionLayerFlag);

    AddCommand(CONTENT_XML, "ShowStack",  "Show Stack",   "ShowStack", NO_DISPLAY, INCLUDE, m_showStack);
    AddCommand(CONTENT_HTML, "PopLayer",  "Pop Layer",   "PopLayer", NO_DISPLAY, INCLUDE, m_popLayer);
    AddCommand(CONTENT_HTML, "PushLayer",  "Push Layer",   "PushLayer", NO_DISPLAY, INCLUDE, m_pushLayer);
    AddCommand(CONTENT_TEXT, "StepFrame", "Step Frame", "StepFrame", NO_DISPLAY, INCLUDE, m_stepFrame);

    // These are queried by the client at connect time
    // TODO: move these as ServerState XML?
    AddCommand(CONTENT_TEXT, "AutoCapture", "AutoCapture", "AutoCapture", NO_DISPLAY, INCLUDE, m_AutoCapture);
    AddCommand(CONTENT_TEXT, "FlattenCommandLists", "FlattenCommandLists", "FlattenCommandLists", NO_DISPLAY, INCLUDE, m_FlattenCommandLists);
    AddCommand(CONTENT_TEXT, "NumGPUs",     "NumGPUs",     "NumGPUs",     NO_DISPLAY, INCLUDE, m_NumGPUs);
    AddCommand(CONTENT_TEXT, "FrameCaptured", "FrameCaptured", "FrameCaptured", NO_DISPLAY, INCLUDE, m_FrameCaptured);

    m_captureFrame = SG_GET_INT(OptionCaptureFrame);

    m_pushLayer.SetEditableContentAutoReply(false);
    m_AutoCapture = false;

    m_stepFrame.SetEditableContentAutoReply(false);
    m_stepFrame.Reset();

    SetLayerName("LayerManager");

    if (SG_GET_BOOL(OptionCollectFrameStats))
    {
        if (mFrameStatsLogger == NULL)
        {
            mFrameStatsLogger = new FrameStatsLogger;
            mFrameStatsLogger->Initialize();
        }
    }

    // Set the values in the frame timings array
    for (int i = 0; i < FRAMES_TO_MEASURE; i++)
    {
        mFrameDurationsMilliseconds[i] = 0.0;
    }

    mAverageFramesPerSecond = 1.0;
    mLastFrameDurationMilliseconds = 0.0;
    mPreviousFrameDurationMilliseconds = 0.0;

    // Reset the timer to initialize the start time.
    mFrameTimer.ResetTimer();
}

/// if the layer can be enabled, then it will be pushed onto the enabled layers stack
/// \param rLayer the layer to push on the stack
/// \param pRequest the request to enable that layer
/// \return true if the layer could be enabled; false otherwise
bool LayerManager::PushLayer(ILayer& rLayer, CommandResponse* pRequest)
{
    if (rLayer.EnableLayer(true, pRequest))
    {
        m_EnabledLayers.push_back(&rLayer);
        return true;
    }
    else
    {
        return false;
    }
}

/// if the layer can be disabled, then it will be popped off of the enabled layers stack
/// \return true if the layer can be disabled; false otherwise
bool LayerManager::PopEnabledLayer()
{
    if (m_EnabledLayers.begin() == m_EnabledLayers.end())
    {
        // if there are no objects, then there's nothing to Pop
        return false;
    }

    if (m_EnabledLayers.back()->EnableLayer(false, &m_popLayer))
    {
        m_EnabledLayers.pop_back();
        return true;
    }
    else
    {
        return false;
    }
}

/// Gets called at the beginning of the frame
void LayerManager::BeginFrame()
{
    // increase the frame count if not currently captured
    if (m_instantCaptureState != INSTANT_CAPTURE_CAPTURED)
    {
        m_frameCount++;
    }

    //handle PopLayer command
    if (m_popLayer.IsActive())
    {
        if (m_Processors.empty() == false)
        {
            m_popLayer.Send(m_Processors[m_Processors.size() - 1]->GetTagName());

            m_Processors.pop_back();
            PopEnabledLayer();
        }
        else
        {
            m_popLayer.Send("<empty>");
        }
    }

    //handle PushLayer command
    if (m_pushLayer.IsActive())
    {
        // iterate through each layer
        for (DWORD i = 0; i < m_LayerListSize; i++)
        {
            // compare the requested layer to each layer's name
            LAYERDESC layer = m_LayerList[i];

            if (strcmp(m_pushLayer.GetValue(), layer.tagName) == 0)
            {
                // found matching layer, try to enable it
                // supply the m_pushLayer command so that errors can be sent to the client.
                if (PushLayer(*(layer.Layer), &m_pushLayer))
                {
                    AddProcessor(layer.tagName, layer.displayName, layer.pID, "", layer.eDisplayMode, *(layer.CmdProcessor));
                }

                break;
            }
        }

        if (m_pushLayer.IsActive())
        {
            m_pushLayer.SendError("Layer not found");
        }
    }

    //handle ShowStack command
    if (m_showStack.IsActive())
    {
        std::string str = "";

        for (DWORD i = 0; i < m_Processors.size(); i++)
        {
            str += XML("Layer", m_Processors[i]->GetTagName()).asCharArray();
        }

        str = XML("LayerStack" , str.c_str()).asCharArray();

        m_showStack.Send(str.c_str());
    }

    // Check for instant capture if required. For now, a frame of -1 means disabled
    if (m_captureFrame >= 0)
    {
        // grab global frame capture index, compare, call instant capture
        if (m_frameCount >= m_captureFrame && m_instantCaptureState == INSTANT_CAPTURE_ALLOW)
        {
            AutocaptureTriggered();
        }
    }

    if (m_stepFrame.IsActive())
    {
        m_stepFrame.Send("OK");
    }

    std::vector<ILayer*>::const_iterator it;

    for (it = m_EnabledLayers.begin(); it != m_EnabledLayers.end(); ++it)
    {
        (*it)->BeginFrame();
    }
}

/// Gets called at the end of the frame
void LayerManager::EndFrame()
{
    // The frame stats logger is non-null only when it has been enabled through the config options.
    if (mFrameStatsLogger != NULL)
    {
        // Update the stats before doing any other layer management for the end of the frame.
        mFrameStatsLogger->UpdateStats();
    }

    UpdateFrameTimingInfo();

    // make a temp vector to use to iterate, since the GL server can cause re-entry and that can cause
    // the m_EnabledLayers vector to change, then when you play the app again, the C runtime recognizes
    // that the vector had changed, and crashes the app.
    std::vector<ILayer*> tmpVec = m_EnabledLayers;

    std::vector<ILayer*>::const_iterator it;

    for (it = tmpVec.begin() ; it != tmpVec.end(); ++it)
    {
        (*it)->EndFrame();
    }
}

void LayerManager::UpdateFrameTimingInfo()
{
    double currentTime = mFrameTimer.GetAbsoluteMicroseconds();
    static double previousTime = currentTime;

    mPreviousFrameDurationMilliseconds = mLastFrameDurationMilliseconds;
    mLastFrameDurationMilliseconds = currentTime - previousTime;
    mPreviousAverageFramesPerSecond = mAverageFramesPerSecond;

    //Log(logERROR, "previousTime: %lf, currentTime: %lf,  mLastFrameDurationMilliseconds: %lf\n", currentTime, previousTime, mLastFrameDurationMilliseconds);

    // Update the shared memory with our current rendering time.
    SG_SET_DOUBLE(LastPresentTime, currentTime);

    // This catches the first usage where the time delta is zero
    if (mLastFrameDurationMilliseconds == 0)
    {
        previousTime = currentTime;
        mPreviousAverageFramesPerSecond = mAverageFramesPerSecond = 1.0;
        return;
    }

    mAverageFramesPerSecond = 1000000.0 / mLastFrameDurationMilliseconds;

    previousTime = currentTime;
}


/// A handler invoked when Autocapture mode has been triggered.
void LayerManager::AutocaptureTriggered()
{
    DoInstantCapture();

    m_instantCaptureState = INSTANT_CAPTURE_CAPTURED;
}

/// Send a single client command to the server.
/// \param layerString String corresponding to the layer this command is sent to
/// \param commandString the command to send
/// \return true if the command is sent, false otherwise
bool LayerManager::SendServerCommand(const char* layerString, const char* commandString)
{
    bool bRtnProcess = false;

    for (DWORD i = 0; i < m_LayerListSize; i++)
    {
        // compare the requested layer to each layer's name
        LAYERDESC layer = m_LayerList[i];

        if ((strcmp(layerString, layer.tagName) == 0) && (true == layer.Layer->IsEnabled()))
        {
            // found matching enabled layer
            CommandObject objCommand(0, (char*)commandString);
            bRtnProcess = layer.CmdProcessor->Process(objCommand);
        }
    }

    return bRtnProcess;
}

/// Setup an instant capture. This performs the same process as connecting the
/// client and server before a capture. Since the client isn't used during
/// an instant capture, this setup still needs to be done.
void LayerManager::SetupInstantCapture()
{
    // Commands sent when Connecting the server: Push timeControl layer and get settings.xml
    // /<PID>/<API>/PushLayer=TimeControl
    for (DWORD i = 0; i < m_LayerListSize; i++)
    {
        // compare the requested layer to each layer's name
        LAYERDESC layer = m_LayerList[i];

        if ((strcmp("TimeControl", layer.tagName) == 0) && (false == layer.Layer->IsEnabled()))
        {
            // found matching layer, try to enable it
            if (PushLayer(*(layer.Layer), &m_pushLayer))
            {
                AddProcessor(layer.tagName, layer.displayName, layer.pID, "", layer.eDisplayMode, *(layer.CmdProcessor));
                break;
            }
        }
    }

    // /<PID>/<API>/TC/Settings.xml
    // no need to send this
    //    SendServerCommand("TimeControl", "Settings.xml");

    // /<PID>/<API>/PushLayer=FrameCapture
    // activate FrameCapture layer
    for (DWORD i = 0; i < m_LayerListSize; i++)
    {
        // compare the requested layer to each layer's name
        LAYERDESC layer = m_LayerList[i];

        if ((strcmp("FrameCapture", layer.tagName) == 0) && (false == layer.Layer->IsEnabled()))
        {
            // found matching layer, try to push/enable it
            if (PushLayer(*(layer.Layer), &m_pushLayer))
            {
                AddProcessor(layer.tagName, layer.displayName, layer.pID, "", layer.eDisplayMode, *(layer.CmdProcessor));
                break;
            }
        }
    }
}

/// Do the frame capture. This performs the same process as pressing the pause
/// button from the client.
/// \return true if successful, false if error
bool LayerManager::DoInstantCapture()
{
    //    LogConsole(logMESSAGE, "Instant Capture Frame: Current frame: %d, Required frame: %d\n", m_frameCount, m_captureFrame);
    unsigned int captureFrame = (unsigned int)SG_GET_INT(OptionCaptureFrame);

    int timeOverrideMode;
    int filterDrawCallMask;
    bool copyMappedBuffersUsingCPU;

    // capture this frame
    if (captureFrame == m_captureFrame)
    {
        // if this is an autocapture, read settings from config file
        timeOverrideMode = SG_GET_INT(OptionTimeOverrideMode);
        filterDrawCallMask = SG_GET_INT(OptionFilterDrawCalls);
        copyMappedBuffersUsingCPU = SG_GET_BOOL(OptionCopyMappedBuffersUsingCPU);
        m_FlattenCommandLists = SG_GET_BOOL(OptionFlattenCommandLists);

        // only setup capture on the first time round
        SetupInstantCapture();

        // indicate that this is an autocapture capture so that the client can get settings from the server
        m_AutoCapture = true;
    }
    else
    {
        // otherwise, get data from the step frame command
        timeOverrideMode = m_stepFrame.GetTimeOverrideMode();
        filterDrawCallMask = m_stepFrame.GetFilterDrawCalls();
        copyMappedBuffersUsingCPU = m_stepFrame.GetHandleMapsOnCPU();
        m_FlattenCommandLists = m_stepFrame.GetFlattenCommandLists();
    }

    // Do the actual capture
    bool bRtnProcess = false;

    const char* trueStr = "true";
    const char* falseStr = "false";

    const char* handleMapsStr = copyMappedBuffersUsingCPU == true ? trueStr : falseStr;
    const char* flattenStr = m_FlattenCommandLists == true ? trueStr : falseStr;

    static char message[1024];

    sprintf_s(message, 1024, "Capture?Stream=0&TimeOverrideMode=%d&HandleMapsOnCPU=%s&FlattenCommandLists=%s&FilterDrawCalls=%d&AutoCapture=true",
              timeOverrideMode, handleMapsStr, flattenStr, filterDrawCallMask);

    bRtnProcess = SendServerCommand("FrameCapture", message);

    m_FrameCaptured = true;

    return bRtnProcess;
}

/// Inform the layer manager that a capture has started
/// Set the state to captured so that the frame counter is not incremented
void LayerManager::StartCapture()
{
    m_stepFrame.Reset();
}

/// Inform the layer manager that a capture has ended
/// Set the state back to normal so that the frame counter is incremented
void LayerManager::ReleaseCapture()
{
    m_instantCaptureState = INSTANT_CAPTURE_ALLOW;

    int stepCount = m_stepFrame.GetStepCount();

    if (stepCount > 0)
    {
        // advance m_stepFrame frames from current frame
        m_captureFrame = m_frameCount + stepCount;
    }
    //else
    //{
    //    // disable autocapture for subsequent frames
    //    m_captureFrame = -1;
    //}

    m_FrameCaptured = false;
}
