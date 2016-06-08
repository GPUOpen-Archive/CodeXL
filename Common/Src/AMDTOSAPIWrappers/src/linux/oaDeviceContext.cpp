//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaDeviceContext.cpp
///
//=====================================================================

//------------------------------ oaDeviceContext.cpp ------------------------------

// GRBaseTools:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaPixelFormat.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLRenderContext.h>
#include <AMDTOSAPIWrappers/Include/oaDeviceContext.h>


// ---------------------------------------------------------------------------
// Name:        oaDeviceContext::oaDeviceContext
// Description: Constructor
// Arguments:   hWnd - Handle to the X11 Window that holds this device context.
//              Note that as in Linux we need a device context to create a window,
//              This can be NULL (and will be later copied by using the other constructor).
// Author:      AMD Developer Tools Team
// Date:        20/5/2008
// ---------------------------------------------------------------------------
oaDeviceContext::oaDeviceContext(oaWindowHandle hWnd, oaHiddenWindow* pRelatedWindow)
    : _hWnd(hWnd), _hDC(NULL), _isOSDCOwner(false), _pOGLRenderContext(NULL), _pRelatedWindow(pRelatedWindow)
{
    // Create the device context:
    _hDC = XOpenDisplay(NULL);

    GT_IF_WITH_ASSERT(_hDC != NULL)
    {
        // Mark that this class is the owner of the OS device context handle:
        _isOSDCOwner = true;
    }
}

// ---------------------------------------------------------------------------
// Name:        oaDeviceContext::oaDeviceContext
// Description: Constructor
// Arguments:   hWnd - Handle to the Win32 window that contains this device context.
//              hDC - Handle to the Win32 device context.
// Author:      AMD Developer Tools Team
// Date:        20/5/2008
// ---------------------------------------------------------------------------
oaDeviceContext::oaDeviceContext(oaWindowHandle hWnd, oaDeviceContextHandle hDC, oaHiddenWindow* pRelatedWindow)
    : _hWnd(hWnd), _hDC(hDC), _isOSDCOwner(false), _pOGLRenderContext(NULL), _pRelatedWindow(pRelatedWindow)
{
}

// ---------------------------------------------------------------------------
// Name:        oaDeviceContext::~oaDeviceContext
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        20/5/2008
// ---------------------------------------------------------------------------
oaDeviceContext::~oaDeviceContext()
{
    // If a render context was created:
    if (_pOGLRenderContext != NULL)
    {
        // Release it:
        delete _pOGLRenderContext;
    }
}

// ---------------------------------------------------------------------------
// Name:        GrtPixelFormatSelector::amountOfAvailablePixelFormats
// Description: Return the amount of pixel formats available in this graphic
//              device context.
// Author:      AMD Developer Tools Team
// Date:        20/5/2008
// ---------------------------------------------------------------------------
int oaDeviceContext::amountOfAvailablePixelFormats() const
{
    int retVal = 0;
    int returnedNumber;
    // If succeeded, DescribePixelFormat returns the amount of available pixel formats for
    // the given device context. If it fails, it returns 0:
    XVisualInfo* pVisualInfo = XGetVisualInfo(_hDC, 0, NULL, &returnedNumber);
    GT_IF_WITH_ASSERT(pVisualInfo != NULL)
    {
        retVal = returnedNumber;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaDeviceContext::getPixelFormat
// Description:
//  Inputs a pixel format index and returns a class the represents it.
//
// Arguments:   pixelFormatIndex - The requested pixel format index.
//                                 Notice: On Windows the first pixel format index is 1 !!!
//              aptrPixelFormat - Will get a class the represents the pixel format.
//
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        20/5/2008
// ---------------------------------------------------------------------------
bool oaDeviceContext::getPixelFormat(oaPixelFormatId pixelFormatIndex, gtAutoPtr<oaPixelFormat>& aptrPixelFormat) const
{
    bool retVal = false;

    // Create a wrapper class for the requested pixel format:
    oaPixelFormat* pPixelFormat = new oaPixelFormat(_hDC, pixelFormatIndex);
    GT_IF_WITH_ASSERT(pPixelFormat)
    {
        // Initialize its data:
        bool rc = pPixelFormat->initialize();
        GT_IF_WITH_ASSERT(rc)
        {
            aptrPixelFormat = pPixelFormat;
            retVal = true;
        }
        else
        {
            // We failed to initialize the pixel format:
            delete pPixelFormat;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaDeviceContext::getOpenGLPixelFormatId
// Description: Search for a pixel format that supports OpenGL.
//              If we know what this Display's Window is, use its pixel format.
//              If we don't (or there isn't one), give the Display's default pixel format
//
// Return Val: int - Will get the index of the visual, or -1 if such a pixel format
//             does not exist in this device context.
//
// Author:      AMD Developer Tools Team
// Date:        20/5/2008
// ---------------------------------------------------------------------------
oaPixelFormatId oaDeviceContext::getOpenGLPixelFormatId() const
{
    oaPixelFormatId retVal = OA_NO_PIXEL_FORMAT_ID;

    Visual* pVisual = XDefaultVisual(_hDC, DefaultScreen(_hDC));

    if (pVisual != NULL)
    {
        retVal = XVisualIDFromVisual(pVisual);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaDeviceContext::getHardwareAcceleratedOpenGLPixelFormatId
// Description: This function is irrelevant since Linux does not differentiate pixel formats
//              for different Displays
//
// Return Val: see getOpenGLPixelFormatId
//
// Author:      AMD Developer Tools Team
// Date:        20/5/2008
// ---------------------------------------------------------------------------
oaPixelFormatId oaDeviceContext::getHardwareAcceleratedOpenGLPixelFormatId() const
{
    oaPixelFormatId retVal = getOpenGLPixelFormatId();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaDeviceContext::createOpenGLRenderContext
// Description: Creates an OpenGL render context that will draw into this
//              graphic device context.
//
// Return Val:  oaOpenGLRenderContext* - The creates OpenGL render context.
//                                       (NULL in case of failure)
// Author:      AMD Developer Tools Team
// Date:        20/5/2008
// ---------------------------------------------------------------------------
bool oaDeviceContext::createOpenGLRenderContext()
{
    bool retVal = false;

    // Create an OpenGL render context:
    _pOGLRenderContext = new oaOpenGLRenderContext(*this);
    GT_IF_WITH_ASSERT(_pOGLRenderContext != NULL)
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaDeviceContext::getPixelFormatIDFromIndex
// Description: Returns the ID of the pixel format which is number pixelFormatIndex
//              in the list of available visual infos for the default screen.
//              pixelFormatIndex - the index in the list (1-based)
// Author:      AMD Developer Tools Team
// Date:        25/5/2008
// ---------------------------------------------------------------------------
oaPixelFormatId oaDeviceContext::getPixelFormatIDFromIndex(int pixelFormatIndex) const
{
    oaPixelFormatId retVal = OA_NO_PIXEL_FORMAT_ID;

    XVisualInfo screenOnlyXVisInfo;
    screenOnlyXVisInfo.screen = DefaultScreen(_hDC);
    int amountOfVisuals = 0;
    XVisualInfo* visInfosList = XGetVisualInfo(_hDC, VisualScreenMask, &screenOnlyXVisInfo, &amountOfVisuals);
    GT_IF_WITH_ASSERT((visInfosList != NULL) && (amountOfVisuals != 0))
    {
        retVal = visInfosList[pixelFormatIndex - 1].visualid;

        // 0 is "all" device context, so we return the default one:
        if (retVal == 0)
        {
            retVal = XVisualIDFromVisual(DefaultVisual(_hDC, DefaultScreen(_hDC)));
        }
    }

    return retVal;
}
