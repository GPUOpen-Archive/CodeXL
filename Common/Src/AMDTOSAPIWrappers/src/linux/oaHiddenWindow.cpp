//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaHiddenWindow.cpp
///
//=====================================================================

//------------------------------ oaHiddenWindow.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <linux/oaPlatformSpecificFunctionPointers.h>
#include <AMDTOSAPIWrappers/Include/oaDeviceContext.h>
#include <AMDTOSAPIWrappers/Include/oaHiddenWindow.h>

#define BORDER_WIDTH 0

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::oaHiddenWindow
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        19/5/2008
// ---------------------------------------------------------------------------
oaHiddenWindow::oaHiddenWindow()
    : _hWindow(0), _pDeviceContext(NULL), _hPixelFormat(OA_NO_PIXEL_FORMAT_HANDLE)
{
}

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::~oaHiddenWindow
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        19/5/2008
// ---------------------------------------------------------------------------
oaHiddenWindow::~oaHiddenWindow()
{
    destroy();
}

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::create
//
// Description: Creates the X11 window that this class wraps.
//
// Arguments:   title - The window title.
//              originX - The window origin X coordinate.
//              originY - The window origin Y coordinate.
//              width - The window width.
//              height - The window height.
//
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        19/5/2008
// ---------------------------------------------------------------------------
bool oaHiddenWindow::create(const gtString& title, int originX, int originY, int width, int height)
{
    (void)(title); // unused
    bool retVal = false;

    // Make sure we have a display (device context)
    if (_pDeviceContext == NULL)
    {
        _pDeviceContext = new oaDeviceContext(0, this);

    }

    oaDeviceContextHandle pDisplayNative = _pDeviceContext->nativeDeviceContextHandle();

    // Try and get a double buffered visual info. If we can't, try a single buffered one.
    int attribSingle[] = {GLX_RGBA, None };
    int attribDouble[] = {GLX_RGBA, GLX_DOUBLEBUFFER, None};

    // Make sure that we a pointer to the system's glXChooseVisual function:
    bool rcOGLModule = oaLoadGLXFunctionPointers();
    GT_IF_WITH_ASSERT(rcOGLModule && (pOAglXChooseVisual != NULL))
    {
        _hPixelFormat = pOAglXChooseVisual(pDisplayNative, DefaultScreen(pDisplayNative), attribDouble);

        if (_hPixelFormat == NULL)
        {
            _hPixelFormat = pOAglXChooseVisual(pDisplayNative, DefaultScreen(pDisplayNative), attribSingle);
        }

        GT_IF_WITH_ASSERT(_hPixelFormat != NULL)
        {
            // To use OpenGL on software renderers, we need a color map, as the default one does not suffice:
            XSetWindowAttributes windowAttribs;
            Colormap windowColorMap = XCreateColormap(pDisplayNative, RootWindow(pDisplayNative, _hPixelFormat->screen), _hPixelFormat->visual, AllocNone);
            windowAttribs.colormap = windowColorMap;
            windowAttribs.border_pixel = BORDER_WIDTH;

            // Create the window proper
            _hWindow = XCreateWindow(pDisplayNative,
                                     RootWindow(pDisplayNative, _hPixelFormat->screen),
                                     originX, originY, width, height, BORDER_WIDTH,
                                     _hPixelFormat->depth, InputOutput, _hPixelFormat->visual,
                                     CWBorderPixel | CWColormap, &windowAttribs); // Don't use further attributes

            // If we created a temporary display before, register it as belonging to this window
            if (_pDeviceContext->nativeWindowHandle() == 0)
            {
                _pDeviceContext->setWindowHandle(_hWindow);
            }

            GT_IF_WITH_ASSERT(_hWindow != 0)
            {
                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::destroy
// Description: Destroys the X11 window.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        19/5/2008
// ---------------------------------------------------------------------------
bool oaHiddenWindow::destroy()
{
    bool retVal = true;

    bool releaseWindow = false;
    oaDeviceContextHandle deviceContextHandle;

    if (_pDeviceContext != NULL)
    {
        releaseWindow = true;
        deviceContextHandle = _pDeviceContext->nativeDeviceContextHandle();
    }

    delete _pDeviceContext;
    _pDeviceContext = NULL;

    if (releaseWindow)
    {
        XDestroyWindow(deviceContextHandle, _hWindow);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::showWindow
// Description: Shows the window.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        19/5/2008
// ---------------------------------------------------------------------------
bool oaHiddenWindow::showWindow()
{
    XMapWindow(_pDeviceContext->nativeDeviceContextHandle(), _hWindow);
    return true;
}

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::hideWindow
// Description: Hides the window.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        19/5/2008
// ---------------------------------------------------------------------------
bool oaHiddenWindow::hideWindow()
{
    XUnmapWindow(_pDeviceContext->nativeDeviceContextHandle(), _hWindow);
    return true;
}

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::setActivePixelFormat
// Description: Would try to the active pixel format on this graphic device context.
//              However, X11 does not allow chaging the visual of an existing window,
//              so we just report failure.
// Arguments:   pixelFormatIndex - The index of the pixel format that should become
//                                 the active pixel format.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        20/5/2008
// ---------------------------------------------------------------------------
bool oaHiddenWindow::setActivePixelFormat(oaPixelFormatId pixelFormatIndex)
{
    (void)(pixelFormatIndex); // unused
    bool retVal = false;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::getActivePixelFormat
// Description: Returns the index of the active pixel format.
// Arguments:   pixelFormatIndex - Will get the index of the active pixel format.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        20/5/2008
// ---------------------------------------------------------------------------
bool oaHiddenWindow::getActivePixelFormat(oaPixelFormatId& pixelFormatIndex) const
{
    bool retVal = false;

    if (_pDeviceContext != NULL)
    {
        // Sanity check - make sure we have the right Display:
        if (_pDeviceContext->nativeWindowHandle() == _hWindow)
        {
            oaPixelFormatId pix = _pDeviceContext->getOpenGLPixelFormatId();

            if (pix != OA_NO_PIXEL_FORMAT_ID)
            {
                pixelFormatIndex = pix;
                retVal = true;
            }
        }
    }

    return retVal;
}

