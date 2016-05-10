//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file WindowsWindow.h
/// \brief An empty Windows window used for replaying API Frame Capture files.
//==============================================================================

#ifndef WINDOWS_WINDOW_H
#define WINDOWS_WINDOW_H

#include "../Common/misc.h"

/// An empty window used for replaying API Frame Capture files.
class WindowsWindow
{

public:

    /// Constructor
    WindowsWindow(UINT32 inWidth, UINT32 inHeight, WNDPROC inWndProc);

    virtual ~WindowsWindow();

    bool Initialize(HINSTANCE inHinstance);

    bool Shutdown();

    /// Retrieve the OS window handle for the ReplayWindow.
    /// \return The OS window handle for the ReplayWindow.
    const HWND GetWindowHandle() const
    {
        return mWindowHandle;
    }

    /// Retrieve the application instance.
    /// \returns The OS application instance.
    const HINSTANCE GetHINSTANCE() const
    {
        return mhInstance;
    }


    bool OpenAndUpdate(int inNCmdShow);

private:

    /// The width of the window that was opened for replay.
    UINT32 mWidth;

    /// The height of the window that was opened for replay.
    UINT32 mHeight;

    /// A handle to the window that was opened to replay a capture.
    HWND mWindowHandle;

    /// The window class that must be registered in order to open a replay window.
    WNDCLASSEX mWindowClass;

    /// The HINSTANCE of the player application.
    HINSTANCE mhInstance;

    /// The callback
    WNDPROC mWndProc;
};

#endif // WINDOWS_WINDOW_H