//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file ModernAPILayerManager.h
/// \brief A LayerManager implementation for use with Modern rendering APIs.
//==============================================================================

#ifndef MODERNAPILAYERMANAGER_H
#define MODERNAPILAYERMANAGER_H

#include "LayerManager.h"
#include "CommonTypes.h"

class MultithreadedTraceAnalyzerLayer;
class ModernAPIFrameDebuggerLayer;
class ModernAPIFrameProfilerLayer;
class ObjectDatabaseProcessor;

//--------------------------------------------------------------------------
/// The ModernAPILayerManager is intended to be used with Modern rendering
/// APIs. Shared functionality can be found implemented here.
//--------------------------------------------------------------------------
class ModernAPILayerManager : public LayerManager
{
public:
    //--------------------------------------------------------------------------
    /// Default constructor for ModernAPILayerManager.
    //--------------------------------------------------------------------------
    ModernAPILayerManager();

    //--------------------------------------------------------------------------
    /// Default destructor for ModernAPILayerManager.
    //--------------------------------------------------------------------------
    virtual ~ModernAPILayerManager();

    //--------------------------------------------------------------------------
    /// The layer must create its resources here, it may hook some functions if really needed
    /// \param type the creation object type.
    /// \param pPtr Pointer to the object that was just created.
    /// \return True if success, False if fail.
    //--------------------------------------------------------------------------
    virtual bool OnCreate(CREATION_TYPE type, void* pPtr);

    //--------------------------------------------------------------------------
    /// Destroy all layers tracked by this LayerManager.
    /// \param type The object type that's being tracked.
    /// \param pPtr A pointer to the device that's being tracked my this server plugin.
    /// \return True always.
    //--------------------------------------------------------------------------
    virtual bool OnDestroy(CREATION_TYPE type, void* pPtr);

    //--------------------------------------------------------------------------
    /// Begin the frame.
    //--------------------------------------------------------------------------
    virtual void BeginFrame();

    //--------------------------------------------------------------------------
    /// End the frame.
    //--------------------------------------------------------------------------
    virtual void EndFrame();

    //--------------------------------------------------------------------------
    /// A handler invoked when Autocapture mode has been triggered.
    //--------------------------------------------------------------------------
    virtual void AutocaptureTriggered();

    /// A handler invoked when a key is pressed within the instrumented application.
    /// \param inCode The code for the keypress event.
    /// \param wParam The WPARAM for the keypress event. Contains the virtual key code for the pressed key.
    /// \param lParam The LPARAM for the keypress event. Contains flags for the key press event.
    virtual void OnKeyPressed(int inCode, WPARAM wParam, LPARAM lParam);

    //--------------------------------------------------------------------------
    /// Retrieve a pointer to the TraceAnalyzer layer.
    /// \returns A pointer to the TraceAnalyzer layer.
    //--------------------------------------------------------------------------
    virtual MultithreadedTraceAnalyzerLayer* GetTraceAnalyzerLayer() { return NULL; }

    //--------------------------------------------------------------------------
    /// Retrieve a pointer to the FrameDebugger layer.
    /// \returns A pointer to the FrameDebugger layer.
    //--------------------------------------------------------------------------
    virtual ModernAPIFrameDebuggerLayer* GetFrameDebuggerLayer() { return NULL; }

    //--------------------------------------------------------------------------
    /// Retrieve a pointer to the FrameProfiler layer.
    /// \returns A pointer to the FrameProfiler layer.
    //--------------------------------------------------------------------------
    virtual ModernAPIFrameProfilerLayer* GetFrameProfilerLayer() { return NULL; }

    //--------------------------------------------------------------------------
    /// Retrieve a pointer to the ObjectDatabaseProcessor layer.
    /// \returns A pointer to the ObjectDatabaseProcessor layer.
    //--------------------------------------------------------------------------
    virtual ObjectDatabaseProcessor* GetObjectDatabaseProcessor() { return NULL; }

    //--------------------------------------------------------------------------
    /// Create a new CommandObject to be processed by the LayerManager.
    //--------------------------------------------------------------------------
    bool ProcessRequestFromCommId(CommunicationID inRequestId);

    //--------------------------------------------------------------------------
    /// Check if the server plugin is executing within the CapturePlayer executable.
    /// \returns True if the server plugin is executing within the CapturePlayer executable.
    //--------------------------------------------------------------------------
    bool InCapturePlayer() const { return mbInCapturePlayer; }

    //--------------------------------------------------------------------------
    /// Check if a trace has been triggered for collection by a keypress.
    //--------------------------------------------------------------------------
    bool IsTraceTriggeredByKeypress() const { return mbTraceTriggeredFromKeypress; }

    //--------------------------------------------------------------------------
    /// A CommandResponse that can collect a frame capture or trace, and will save
    /// the response to disk. Metadata XML describing the operation is returned.
    //--------------------------------------------------------------------------
    ModernCaptureCommandResponse mCmdFrameCaptureWithSave;

    //--------------------------------------------------------------------------
    /// A CommandResponse that sets the session name to be used when capturing
    //--------------------------------------------------------------------------
    TextCommandResponse mCmdSetSessionName;

    //--------------------------------------------------------------------------
    /// A CommandResponse that sets the project name to be used when capturing
    //--------------------------------------------------------------------------
    TextCommandResponse mCmdSetProjectName;

    //--------------------------------------------------------------------------
    /// Retrieve the path to the metadata file, if one has been provided.
    //--------------------------------------------------------------------------
    const std::string& GetPathToTargetMetadataFile() const { return mPathToTargetMetadataFile; }

    /// Get method for the number frames to capture
    /// \return The number of frames to capture
    int GetCaptureCount()
    {
        return m_captureCount;
    }

    /// Get method for the capture type
    /// \return The capture type
    int GetCaptureType()
    {
        return m_captureType;
    }

private:
    //--------------------------------------------------------------------------
    /// Parse the commandline arguments string passed to the target application.
    /// \param inApplicationCommandLine The commandline arguments string.
    //--------------------------------------------------------------------------
    void ParseCommandLine(const std::string& inApplicationCommandLine);

    //--------------------------------------------------------------------------
    /// A full path to the metadata file provided when starting the CapturePlayer.
    //--------------------------------------------------------------------------
    std::string mPathToTargetMetadataFile;

    //--------------------------------------------------------------------------
    /// A flag that indicates if the server plugin is running inside the
    /// CapturePlayer executable.
    //--------------------------------------------------------------------------
    bool mbInCapturePlayer;

    //--------------------------------------------------------------------------
    /// A flag that indicates that the server should collect a frame triggered by a keypress.
    //--------------------------------------------------------------------------
    bool mbTraceTriggeredFromKeypress;

private:

    int m_captureType;  ///< Capture type
    int m_captureCount; ///< The number of frames to capture
};

#endif // MODERNAPILAYERMANAGER_H