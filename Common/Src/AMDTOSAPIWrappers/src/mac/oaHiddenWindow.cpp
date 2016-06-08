//------------------------------ oaHiddenWindow.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaDeviceContext.h>
#include <AMDTOSAPIWrappers/Include/oaHiddenWindow.h>

// Is used to give a different id to each window class:
static int stat_windowClassNameCounter = 0;

//////////////////////////////////////////////////////////////////////////
// Warning: This class is a dummy class in Mac OS X, as CGL supports
// native offscreen rendering - Thus the "window" is in fact a byte buffer
// which we hold in the oaOpenGLRenderContext class.
// This class is only implemented on Mac for compatibility
//////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::oaHiddenWindow
// Description: Constructor
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
oaHiddenWindow::oaHiddenWindow()
    : _hWindow(0), _pDeviceContext(NULL), _hPixelFormat(OA_NO_PIXEL_FORMAT_HANDLE)
{
}

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::~oaHiddenWindow
// Description: Destructor
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
oaHiddenWindow::~oaHiddenWindow()
{
    destroy();
}

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::create
//
// Description: Creates the Device context wrapper for this "window".
//
// Arguments:   title - The window title.
//              originX - The window origin X coordinate.
//              originY - The window origin Y coordinate.
//              width - The window width.
//              height - The window height.
//
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
bool oaHiddenWindow::create(const gtString& title, int originX, int originY, int width, int height)
{
    bool retVal = true;

    // Make sure we have a display (device context)
    if (_pDeviceContext == NULL)
    {
        _pDeviceContext = new oaDeviceContext(NULL, this);

    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::destroy
// Description: Destroys the device context object.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
bool oaHiddenWindow::destroy()
{
    bool retVal = true;

    delete _pDeviceContext;
    _pDeviceContext = NULL;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::showWindow
// Description: Shows the window.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
bool oaHiddenWindow::showWindow()
{
    return false;
}

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::hideWindow
// Description: Hides the window.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
bool oaHiddenWindow::hideWindow()
{
    return false;
}

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::setActivePixelFormat
// Description: Would try to the active pixel format on this graphic device context.
//              However, Mac does not allow chaging the pixel format of an existing
//              window, so we just report failure.
// Arguments:   pixelFormatId - The pix of the pixel format that should become
//                              the active pixel format.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
bool oaHiddenWindow::setActivePixelFormat(oaPixelFormatId pixelFormatId)
{
    bool retVal = false;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::getActivePixelFormat
// Description: Returns the pix of the active pixel format.
// Arguments:   pixelFormatIndex - Will get the pix of the active pixel format.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        11/12/2008
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

