//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file WindowsWindow.cpp
/// \brief An empty window used for replaying API Frame Capture files.
//==============================================================================

#include <X11/Xlib.h>
#include "X11Window.h"

#include "Logger.h"

const char* kWindowTitle = "Capture Player - [PLACEHOLDER].ACR"; ///< Window title

/// Constructor.
/// \param inWidth The width of the player window
/// \param inHeight The height of the player window
X11Window::X11Window(UINT windowWidth, UINT windowHeight)
    : WindowBase(windowWidth, windowHeight)
    , mWindowHandle(0)
    , mDisplay(NULL)
{
}

/// Destructor
X11Window::~X11Window()
{
}

/// Create a new window and prepare it for use.
/// \returns True if initialization is successful.
bool X11Window::Initialize()
{
    mDisplay = XOpenDisplay(NULL);
    if (!mDisplay)
    {
        Log(logERROR, "Error: couldn't open display.\n");
        return false;
    }

    // find the screen and the root window
    int screen = DefaultScreen(mDisplay);
    Window root = RootWindow(mDisplay, screen);
    Visual* visual = DefaultVisual(mDisplay, screen);
    int depth = DefaultDepth(mDisplay, screen);

    // window attributes
    XSetWindowAttributes attr;
    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap(mDisplay, root, visual, AllocNone);
    attr.event_mask =
        ExposureMask |
        FocusChangeMask |
        StructureNotifyMask;

    unsigned long mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    mWindowHandle = XCreateWindow(mDisplay, root, 0, 0, m_windowWidth, m_windowHeight,
        0, depth, InputOutput, visual, mask, &attr);

    // set hints and properties
    {
        XSizeHints sizehints;
        sizehints.x = 0;
        sizehints.y = 0;
        sizehints.width  = m_windowWidth;
        sizehints.height = m_windowHeight;
        sizehints.flags = USSize | USPosition;
        XSetNormalHints(mDisplay, mWindowHandle, &sizehints);
        XSetStandardProperties(mDisplay, mWindowHandle, kWindowTitle, kWindowTitle,
                                None, (char **)NULL, 0, &sizehints);
    }

    return true;
}


/// Shut down and clean up resources associated with a ReplayWindow instance.
/// \returns True if cleanup and shutdown was successful.
bool X11Window::Shutdown()
{
    XDestroyWindow(mDisplay, mWindowHandle);

    XCloseDisplay(mDisplay);

    return true;
}

/// Open an initialized window in the system UI.
/// \param inNCmdShow Controls how the window is to be shown.
/// \return True if success, false if fail.
bool X11Window::OpenAndUpdate(int inNCmdShow)
{
    UNREFERENCED_PARAMETER(inNCmdShow);

    // make the window visible
    XMapWindow(mDisplay, mWindowHandle);

    return true;
}

/// Update the window. This is the OS-dependent message loop
/// implementation so should be called periodically.
/// \return false if the message loop is to be terminated, true
/// otherwise.
bool X11Window::Update()
{
    bool result = true;
    Display* xdisplay = mDisplay;
    Window xwin = mWindowHandle;
    XEvent evt;
    int w;
    int h;

    while (XEventsQueued(xdisplay, QueuedAfterFlush))
    {
        XNextEvent(xdisplay, &evt);
        switch (evt.type)
        {

        case ClientMessage:
            // close window
            if (evt.xmotion.window == xwin)
            {
                long wmdel;
                wmdel = (long)XInternAtom(xdisplay, "WM_DELETE_WINDOW", True);
                if (evt.xclient.data.l[0] == wmdel)
                {
                    result = false;
                }
            }
            break;

        case ConfigureNotify:
            // assume it's a window resize
            if (evt.xconfigure.window == xwin)
            {
                w = evt.xconfigure.width;
                h = evt.xconfigure.height;
            }
            break;
        }
    }
    return result;
}
