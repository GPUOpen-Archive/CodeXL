//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file WindowsWindow.h
/// \brief An empty Windows window used for replaying API Frame Capture files.
//==============================================================================

#ifndef WINDOWS_WINDOW_H
#define WINDOWS_WINDOW_H

#include "WindowBase.h"

/// An empty window used for replaying API Frame Capture files.
class WindowsWindow : public WindowBase
{
public:

    /// Constructor
    WindowsWindow(UINT windowWidth, UINT windowHeight, HINSTANCE hInstance, WNDPROC inWndProc);

    /// Destructor
    virtual ~WindowsWindow();

    /// Create a new window and prepare it for use.
    /// \param inHinstance The HINSTANCE for the running application.
    /// \returns True if initialization is successful.
    bool Initialize();

    /// Shut down and clean up resources associated with a ReplayWindow instance.
    /// \returns True if cleanup and shutdown was successful.
    bool Shutdown();

    /// Retrieve the OS window handle for the ReplayWindow.
    /// \return The OS window handle for the ReplayWindow.
    const NativeWindowType GetWindowHandle() const
    {
        return mWindowHandle;
    }

    /// Retrieve the application instance.
    /// \returns The OS application instance.
    const NativeInstanceType GetInstance() const
    {
        return mhInstance;
    }

    /// Open an initialized window in the system UI.
    /// \param inNCmdShow Controls how the window is to be shown.
    /// \return True if success, false if fail.
    bool OpenAndUpdate(int inNCmdShow);

    /// Update the window. This is the OS-dependent message loop
    /// implementation so should be called periodically.
    /// \return false if the message loop is to be terminated, true
    /// otherwise.
    bool Update();

private:

    /// A handle to the window that was opened to replay a capture.
    NativeWindowType mWindowHandle;

    /// The window class that must be registered in order to open a replay window.
    WNDCLASSEX mWindowClass;

    /// The HINSTANCE of the player application.
    NativeInstanceType mhInstance;

    /// The callback
    WNDPROC mWndProc;
};

#endif // WINDOWS_WINDOW_H