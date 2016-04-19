//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file ReplayWindow.h
/// \brief An empty window used for replaying API Frame Capture files.
//==============================================================================

#ifndef REPLAYWINDOW_H
#define REPLAYWINDOW_H

#include "../Common/misc.h"

//--------------------------------------------------------------------------
/// An empty window used for replaying API Frame Capture files.
//--------------------------------------------------------------------------
class ReplayWindow
{
public:
    //--------------------------------------------------------------------------
    /// Default constructor for ReplayWindow.
    //--------------------------------------------------------------------------
    ReplayWindow(UINT inWidth, UINT inHeight);

    //--------------------------------------------------------------------------
    /// Default destructor for ReplayWindow.
    //--------------------------------------------------------------------------
    virtual ~ReplayWindow();

    //--------------------------------------------------------------------------
    /// Create a new window and prepare it for use.
    /// \param inHInstance The HINSTANCE for the running application.
    /// \param innShowCmd The nShowCmd we get from WinMain.
    /// \param inWndProc The window procedure used to handle system messages.
    /// \returns True if initialization is successful.
    //--------------------------------------------------------------------------
    bool Initialize(HINSTANCE inHinstance, int innShowCmd, WNDPROC inWndProc);

    //--------------------------------------------------------------------------
    /// Shut down and clean up resources associated with a ReplayWindow instance.
    /// \returns True if cleanup and shutdown was successful.
    //--------------------------------------------------------------------------
    bool Shutdown();

    //--------------------------------------------------------------------------
    /// Retrieve the OS window handle for the ReplayWindow.
    /// \returns The OS window handle for the ReplayWindow.
    //--------------------------------------------------------------------------
    const HWND GetWindowHandle() const { return mWindowHandle; }

    //--------------------------------------------------------------------------
    /// Open an initialized window in the system UI.
    //--------------------------------------------------------------------------
    bool OpenAndUpdate(int inNCmdShow);

private:
    //--------------------------------------------------------------------------
    /// The width of the window that was opened for replay.
    //--------------------------------------------------------------------------
    UINT32 mWidth;

    //--------------------------------------------------------------------------
    /// The height of the window that was opened for replay.
    //--------------------------------------------------------------------------
    UINT32 mHeight;

    //--------------------------------------------------------------------------
    /// A handle to the window that was opened to replay a capture.
    //--------------------------------------------------------------------------
    HWND mWindowHandle;

    //--------------------------------------------------------------------------
    /// The window class that must be registered in order to open a replay window.
    //--------------------------------------------------------------------------
    WNDCLASSEX mWindowClass;

    //--------------------------------------------------------------------------
    /// The HINSTANCE of the player application.
    //--------------------------------------------------------------------------
    HINSTANCE mhInstance;

};

#endif // REPLAYWINDOW_H