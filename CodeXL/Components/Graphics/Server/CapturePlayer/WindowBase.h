//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file WindowBase.h
/// \brief Base class for window used for replaying API Frame Capture files.
//==============================================================================

#ifndef WINDOW_BASE_H
#define WINDOW_BASE_H

#ifdef WIN32
#include <Windows.h>
typedef HWND NativeWindowType;
typedef HINSTANCE NativeInstanceType;
#else
#include <X11/Xutil.h>
typedef Window NativeWindowType;
typedef Display* NativeInstanceType;
#endif

/// An empty window used for replaying API Frame Capture files.
class WindowBase
{
public:

    /// Constructor
    WindowBase() {}

    virtual ~WindowBase() {}

    virtual bool Initialize() = 0;

    virtual bool Shutdown() = 0;

    /// Retrieve the OS window handle for the ReplayWindow.
    /// \return The OS window handle for the ReplayWindow.
    virtual const NativeWindowType GetWindowHandle() const = 0;

    /// Retrieve the application instance.
    /// \returns The OS application instance.
    virtual const NativeInstanceType GetInstance() const = 0;

    virtual bool OpenAndUpdate(int inNCmdShow) = 0;
};

#endif // WINDOW_BASE_H