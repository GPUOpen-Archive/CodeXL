//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file ModernAPILayerManager.cpp
/// \brief A LayerManager implementation for use with Modern rendering APIs.
//==============================================================================

#include "ModernAPILayerManager.h"
#include "SharedGlobal.h"
#include "Tracing/MultithreadedTraceAnalyzerLayer.h"
#include "ObjectDatabaseProcessor.h"
#include "../Common/SessionManager.h"
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osKeyboardListener.h>

//--------------------------------------------------------------------------
/// A switch used to enable/disable the "Trace on Keypress" functionality.
//--------------------------------------------------------------------------
#define ENABLE_TRACE_ON_KEYPRESS                0

//--------------------------------------------------------------------------
/// A switch used to determine if the graphics server is able to block client requests.
//--------------------------------------------------------------------------
#define ENABLE_CLIENT_LOCKOUT_ON_KEYPRESS       0

//--------------------------------------------------------------------------
/// The Virtual-Key code for the "PrintScreen" key, used to trigger a capture on keypress.
//--------------------------------------------------------------------------
static const UINT kPrintScreenVirtualKeyCode = 0x2C;

//--------------------------------------------------------------------------
/// Default constructor for NextGenLayerManager.
//--------------------------------------------------------------------------
ModernAPILayerManager::ModernAPILayerManager() :
    mbInCapturePlayer(false),
    mbTraceTriggeredFromKeypress(false),
    m_captureType(3),
    m_captureCount(1)
{
    // Command that collects a CPU and GPU trace from the same frame.
    AddCommand(CONTENT_TEXT, "FrameCaptureWithSave", "FrameCaptureWithSave", "FrameCaptureWithSave", NO_DISPLAY, NO_INCLUDE, mCmdFrameCaptureWithSave);

    // Command to set the session name
    AddCommand(CONTENT_TEXT, "SetSessionName", "SetSessionName", "SetSessionName.txt", DISPLAY, INCLUDE, mCmdSetSessionName);
    mCmdSetSessionName.SetEditableContentAutoReply(false);

    // Command to set the project name
    AddCommand(CONTENT_TEXT, "SetProjectName", "SetProjectName", "SetProjectName.txt", DISPLAY, INCLUDE, mCmdSetProjectName);
    mCmdSetProjectName.SetEditableContentAutoReply(false);

#if ENABLE_TRACE_ON_KEYPRESS
    // Bind the keyboard's keypress callback to our virtual "OnKeyPressed" function. We can override if wherever we need to use it.
    std::function<void(int, WPARAM, LPARAM)> onKeyPressedHandler =
        std::bind(&ModernAPILayerManager::OnKeyPressed, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    osKeyboardListener::Instance().SetOnKbPressedCallback(onKeyPressedHandler);
#endif // ENABLE_TRACE_ON_KEYPRESS
}

//--------------------------------------------------------------------------
/// Default destructor for NextGenLayerManager.
//--------------------------------------------------------------------------
ModernAPILayerManager::~ModernAPILayerManager()
{

}

//--------------------------------------------------------------------------
/// The layer must create its resources here, it may hook some functions if really needed
/// \param type the creation object type.
/// \param pPtr Pointer to the object that was just created.
/// \return True if success, False if fail.
//--------------------------------------------------------------------------
bool ModernAPILayerManager::OnCreate(CREATION_TYPE type, void* pPtr)
{
    bool bReturn = true;

    for (UINT32 i = 0; i < m_AvailableLayers.size(); i++)
    {
        if (m_AvailableLayers[i]->OnCreate(type, pPtr) == false)
        {
            Log(logERROR, "Layer with index '%u' failed in OnCreate call.\n", i);
            bReturn = false;
        }
    }

    static bool bInitialized = false;

    if (!bInitialized)
    {
        osModuleArchitecture moduleArchitecture;
        osRuntimePlatform currentPlatform;
        gtString executablePathString;
        gtString commandLine;
        gtString workingDirectory;

        if (osGetProcessLaunchInfo(osGetCurrentProcessId(), moduleArchitecture, currentPlatform, executablePathString, commandLine, workingDirectory) == true)
        {
            osFilePath executablePath;
            executablePath.setFullPathFromString(executablePathString);

            gtString appName;

            if (executablePath.getFileName(appName) == true)
            {
                gtString capturePlayerExecutableName;
#ifdef CODEXL_GRAPHICS
                capturePlayerExecutableName.fromASCIIString("CXLGraphicsServerPlayer");
#else // CODEXL_GRAPHICS
                capturePlayerExecutableName.fromASCIIString("CapturePlayer");
#endif // CODEXL_GRAPHICS

                if (appName.startsWith(capturePlayerExecutableName))
                {
                    mbInCapturePlayer = true;

                    ParseCommandLine(commandLine.asASCIICharArray());
                }
            }
            else
            {
                Log(logWARNING, "Failed to parse target application filepath.\n");
            }
        }
        else
        {
            Log(logERROR, "Failed to retrieve process launch information.\n");
        }

        bInitialized = true;
    }

    return bReturn;
}

//--------------------------------------------------------------------------
/// Destroy all layers tracked by this LayerManager.
/// \param type The device type (GR_DEVICE) that's being tracked.
/// \param pPtr A pointer to the device that's being tracked my this server plugin.
/// \return True if destruction was successful.
//--------------------------------------------------------------------------
bool ModernAPILayerManager::OnDestroy(CREATION_TYPE type, void* pPtr)
{
    bool bReturn = false;

    // tell each layer that the context was destroyed; Leave the last layer TimeControl layer destroyed later
    for (UINT32 i = 0; i < m_AvailableLayers.size() - 1; i++)
    {
        ILayer* pL = m_AvailableLayers[m_AvailableLayers.size() - i - 1];
        bool bDestructionSuccess = pL->OnDestroy(type, pPtr);

        if (!bDestructionSuccess)
        {
            // Something went wrong when we tried to destroy the layer- report it here.
            Log(logERROR, "Layer with index '%u' failed in OnDestroy call.\n", i);
        }

        bReturn |= bDestructionSuccess;
    }

    return bReturn;
}

//--------------------------------------------------------------------------
/// Begin the frame.
//--------------------------------------------------------------------------
void ModernAPILayerManager::BeginFrame()
{
    GetPendingRequests();

    if (mCmdSetSessionName.IsActive())
    {
        gtASCIIString sessionName = mCmdSetSessionName.GetValue();

        if (SessionManager::Instance()->SetSessionName(sessionName) == false)
        {
            mCmdSetSessionName.Send("Failed");
        }
        else
        {
            mCmdSetSessionName.Send("OK");
        }
    }

    if (mCmdSetProjectName.IsActive())
    {
        gtASCIIString projectName = mCmdSetProjectName.GetValue();

        if (SessionManager::Instance()->SetProjectName(projectName) == false)
        {
            mCmdSetProjectName.Send("Failed");
        }
        else
        {
            mCmdSetProjectName.Send("OK");
        }
    }

    // Check if we need to collect a trace for this currently-rendering frame.
    if (mbTraceTriggeredFromKeypress)
    {
        Log(logMESSAGE, "Keypress capture starting.\n");

        MultithreadedTraceAnalyzerLayer* traceAnalyzer = GetTraceAnalyzerLayer();

        if (traceAnalyzer != nullptr)
        {
            // Examine the layer stack to check for any enabled layers. If there are,
            // we won't be able to collect a trace successfully.
            if (m_EnabledLayers.empty())
            {
#if ENABLE_CLIENT_LOCKOUT_ON_KEYPRESS
                // Tell the GPUPerfServer to reject all incoming requests (it will send the stalled status return to all messages)
                SG_SET_BOOL(ForceRenderStallState, true);
#endif // ENABLE_CLIENT_LOCKOUT_ON_KEYPRESS

                // Since there aren't any layers enabled in the stack, we can proceed with pushing the Logger.
                if (!traceAnalyzer->IsEnabled())
                {
                    // Need to add the TraceAnalyzer layer to the stack.
                    PushLayer(*traceAnalyzer, &m_pushLayer);
                }

                // Enable trace collection for the next rendered frame.
                traceAnalyzer->EnableLinkedTraceCollection();
            }
            else
            {
                // The LayerManager's "enabled layer stack" wasn't empty, meaning the client was in the middle of an operation.
                // Dump a message to the log, and forget that a keypress trace was requested.
                Log(logMESSAGE, "Layer stack is non-empty. Not going to push the Logger for Keypress Capture.\n");
                mbTraceTriggeredFromKeypress = false;
            }
        }
    }

    if (mCmdFrameCaptureWithSave.IsActive())
    {
        // Extract the capture mode argument from the capture mode string
        m_captureType = mCmdFrameCaptureWithSave.GetCaptureType();
        m_captureCount = mCmdFrameCaptureWithSave.GetCaptureCount();

        if (m_captureCount == 0)
        {
            Log(logERROR, "ModernAPILayerManager::BeginFrame - m_captureCount is 0, forcing it to 1.\n");
            m_captureCount = 1;
        }

        if (m_captureType == 3)
        {
            // Now I need to make sure that the MultithreadedTraceAnalyzerLayer is on the stack.
            MultithreadedTraceAnalyzerLayer* traceAnalyzer = GetTraceAnalyzerLayer();
            traceAnalyzer->EnableLinkedTraceCollection();

            // enable objectInspector writing to disk
            ObjectDatabaseProcessor* objectDatabase = GetObjectDatabaseProcessor();
            objectDatabase->EnableObjectDatabaseCollection();
        }
    }

    // Call into the base class to deal with basic layer management.
    LayerManager::BeginFrame();
}

#include "../Common/StreamLog.h"

//--------------------------------------------------------------------------
/// End the frame.
//--------------------------------------------------------------------------
void ModernAPILayerManager::EndFrame()
{
    // Check if we need to collect a trace for this currently-rendering frame.
    MultithreadedTraceAnalyzerLayer* pTraceAnalyzer = GetTraceAnalyzerLayer();

    bool bDisableObjectDataBase = false;

    if (mCmdFrameCaptureWithSave.IsActive())
    {
        // Extract the capture mode argument from the
        m_captureType = mCmdFrameCaptureWithSave.GetCaptureType();
        m_captureCount = mCmdFrameCaptureWithSave.GetCaptureCount();

        if (m_captureType == 3)
        {
            // Now need to make sure that the MultithreadedTraceAnalyzerLayer is on the stack.
            if (pTraceAnalyzer != nullptr)
            {
                pTraceAnalyzer->DisableLinkedTraceCollection();
            }

            // stop objectInspector from writing to disk
            bDisableObjectDataBase = true;
        }
    }

    LayerManager::EndFrame();

    // If the TraceAnalyzer layer is active, and a trace was triggered by keypress, disable the tracing layer.
    if (pTraceAnalyzer != nullptr)
    {
        // Check if the previous frame had been traced. If so, we'll need to clean up the TraceAnalyzer from the stack.
        bool bTracedLastFrame = (pTraceAnalyzer->GetLastTracedFrameIndex() == static_cast<int>(GetFrameCount()));

        if (bTracedLastFrame && mbTraceTriggeredFromKeypress)
        {
            Log(logMESSAGE, "Keypress capture ending.\n");

            // Enable trace collection for the next rendered frame.
            pTraceAnalyzer->DisableLinkedTraceCollection();

            if (pTraceAnalyzer->IsEnabled())
            {
                // Remove the TraceAnalyzerLayer that we pushed earlier.
                PopEnabledLayer();
            }

#if ENABLE_CLIENT_LOCKOUT_ON_KEYPRESS
            // Tell the GPUPerfServer to accept all incoming commands
            SG_SET_BOOL(ForceRenderStallState, false);
#endif // ENABLE_CLIENT_LOCKOUT_ON_KEYPRESS

            // Disable the keypress flag- we've finished tracing the frame.
            mbTraceTriggeredFromKeypress = false;
        }
    }

    // need to disable ObjectDataBase after because mCmdFrameCaptureWithSave is cleared before object EndFrame
    if (bDisableObjectDataBase)
    {
        // stop objectInspector from writing to disk
        ObjectDatabaseProcessor* objectDatabase = GetObjectDatabaseProcessor();
        objectDatabase->DisableObjectDatabaseCollection();
    }
}

//--------------------------------------------------------------------------
/// A handler invoked when Autocapture mode has been triggered.
//--------------------------------------------------------------------------
void ModernAPILayerManager::AutocaptureTriggered()
{
    // Only enable AutoCapture mode with a valid trace type.
    bool bValidTraceMode = SG_GET_INT(OptionTraceType) != kTraceType_None;

    if (bValidTraceMode)
    {
        m_AutoCapture = true;
    }
}

//--------------------------------------------------------------------------
/// A handler invoked when a key is pressed within the instrumented application.
/// \param inCode The code for the keypress event.
/// \param wParam The WPARAM for the keypress event. Contains the virtual key code for the pressed key.
/// \param lParam The LPARAM for the keypress event. Contains flags for the key press event.
//--------------------------------------------------------------------------
void ModernAPILayerManager::OnKeyPressed(int inCode, WPARAM wParam, LPARAM lParam)
{
    PS_UNREFERENCED_PARAMETER(inCode);
    PS_UNREFERENCED_PARAMETER(lParam);

    // If the capture key was pressed, we need to trigger a capture.
    if (wParam == kPrintScreenVirtualKeyCode)
    {
        mbTraceTriggeredFromKeypress = true;
    }
}

//--------------------------------------------------------------------------
/// Process a new request received by the server plugin.
/// \param inRequestId The Id associated with the new incoming request.
/// \return True if the request was handled successfully.
//--------------------------------------------------------------------------
bool ModernAPILayerManager::ProcessRequestFromCommId(CommunicationID inRequestId)
{
    CommandObject command(inRequestId, (char*)GetRequestText(inRequestId));
    return Process(command);
}

//--------------------------------------------------------------------------
/// Parse the commandline arguments string passed to the target application.
/// \param inApplicationCommandLine The commandline arguments string.
//--------------------------------------------------------------------------
void ModernAPILayerManager::ParseCommandLine(const std::string& inApplicationCommandLine)
{
    // The capture player has only one command line arg - the location of the metadata file.
    mPathToTargetMetadataFile = inApplicationCommandLine.c_str();
}
