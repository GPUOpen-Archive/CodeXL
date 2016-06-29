//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file WindowsWindow.h
/// \brief An empty Windows window used for replaying API Frame Capture files.
//==============================================================================

#ifndef X11_WINDOW_H
#define X11_WINDOW_H

#include <WinDefs.h>
#include "WindowBase.h"

/// An empty window used for replaying API Frame Capture files.
class X11Window : public WindowBase
{
public:

    /// Constructor
    X11Window(UINT width, UINT height);

    /// Destructor
    virtual ~X11Window();

    /// Create a new window and prepare it for use.
    /// \returns True if initialization is successful.
    virtual bool Initialize();

    /// Shut down and clean up resources associated with a ReplayWindow instance.
    /// \returns True if cleanup and shutdown was successful.
    virtual bool Shutdown();

    /// Retrieve the OS window handle for the ReplayWindow.
    /// \return The OS window handle for the ReplayWindow.
    virtual NativeWindowType GetWindowHandle() const
    {
        return mWindowHandle;
    }

    /// Retrieve the application instance.
    /// \returns The OS application instance.
    virtual NativeInstanceType GetInstance() const
    {
        return mDisplay;
    }

    /// Open an initialized window in the system UI.
    /// \param inNCmdShow Controls how the window is to be shown.
    /// \return True if success, false if fail.
    virtual bool OpenAndUpdate(int inNCmdShow);

    /// Update the window. This is the OS-dependent message loop
    /// implementation so should be called periodically.
    /// \return false if the message loop is to be terminated, true
    /// otherwise.
    virtual bool Update();

private:

    /// A handle to the window that was opened to replay a capture.
    NativeWindowType mWindowHandle;

    /// The HINSTANCE of the player application.
    NativeInstanceType mDisplay;
};

#endif // X11_WINDOW_H
