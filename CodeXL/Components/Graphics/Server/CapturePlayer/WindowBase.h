//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file WindowBase.h
/// \brief Base class for window used for replaying API Frame Capture files.
//==============================================================================

#ifndef WINDOW_BASE_H
#define WINDOW_BASE_H

#include <AMDTBaseTools/Include/AMDTDefinitions.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <Windows.h>
    typedef HWND NativeWindowType;
    typedef HINSTANCE NativeInstanceType;
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <xcb/xcb.h>
    #include <X11/Xutil.h>
    #include <vulkan/vulkan.h>
    #if 1  // XCB
        typedef xcb_window_t NativeWindowType;
        typedef xcb_connection_t* NativeInstanceType;
    #else  // X11
        typedef Window NativeWindowType;
        typedef Display* NativeInstanceType;
    #endif
#else
    #error Unknown build target! No valid value for AMDT_BUILD_TARGET.
#endif // AMDT_BUILD_TARGET

/// An empty window used for replaying API Frame Capture files.
class WindowBase
{
public:

    /// Constructor
    explicit WindowBase(UINT windowWidth, UINT windowHeight)
        : m_windowWidth(windowWidth)
        , m_windowHeight(windowHeight)
    {}

    /// Destructor
    virtual ~WindowBase()
    {}

    /// Create a new window and prepare it for use.
    /// \param inHinstance The HINSTANCE for the running application.
    /// \return True if initialization is successful.
    virtual bool Initialize() = 0;

    /// Shut down and clean up resources associated with a ReplayWindow instance.
    /// \return True if cleanup and shutdown was successful.
    virtual bool Shutdown() = 0;

    /// Retrieve the OS window handle for the ReplayWindow.
    /// \return The OS window handle for the ReplayWindow.
    virtual const NativeWindowType GetWindowHandle() const = 0;

    /// Retrieve the application instance.
    /// \return The OS application instance.
    virtual const NativeInstanceType GetInstance() const = 0;

    /// Open an initialized window in the system UI.
    /// \param inNCmdShow Controls how the window is to be shown.
    /// \return True if success, false if fail.
    virtual bool OpenAndUpdate(int inNCmdShow) = 0;

    /// Update the window. This is the OS-dependent message loop
    /// implementation so should be called periodically.
    /// \return false if the message loop is to be terminated, true
    /// otherwise.
    virtual bool Update() = 0;

    /// Retrieve the width of the window
    /// return The width of the window
    inline UINT GetWindowWidth() const {
        return m_windowWidth;
    }

    /// Retrieve the height of the window
    /// return The height of the window
    inline UINT GetWindowHeight() const {
        return m_windowHeight;
    }

protected:
    /// Width of the player window
    UINT m_windowWidth;

    /// Height of the player window
    UINT m_windowHeight;
};

#endif // WINDOW_BASE_H
