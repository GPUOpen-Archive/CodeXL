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
    WindowsWindow(HINSTANCE hInstance, WNDPROC inWndProc);

    virtual ~WindowsWindow();

    bool Initialize();

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


    bool OpenAndUpdate(int inNCmdShow);

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