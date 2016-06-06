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
// Arguments:   hWnd - Handle to the Wind32 window that contains this device context.
// Author:      AMD Developer Tools Team
// Date:        6/12/2003
// ---------------------------------------------------------------------------
oaDeviceContext::oaDeviceContext(oaWindowHandle hWnd, oaHiddenWindow* pRelatedWindow)
    : _hWnd(hWnd), _hDC(NULL), _isOSDCOwner(false), _pOGLRenderContext(NULL), _pRelatedWindow(pRelatedWindow)
{
    // Get the window device context:
    _hDC = ::GetDC(_hWnd);
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
// Date:        8/5/2007
// ---------------------------------------------------------------------------
oaDeviceContext::oaDeviceContext(oaWindowHandle hWnd, oaDeviceContextHandle hDC, oaHiddenWindow* pRelatedWindow)
    : _hWnd(hWnd), _hDC(hDC), _isOSDCOwner(false), _pOGLRenderContext(NULL), _pRelatedWindow(pRelatedWindow)
{
}


// ---------------------------------------------------------------------------
// Name:        oaDeviceContext::~oaDeviceContext
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        6/12/2003
// ---------------------------------------------------------------------------
oaDeviceContext::~oaDeviceContext()
{
    // If a render context was created:
    if (_pOGLRenderContext != NULL)
    {
        // Release it:
        delete _pOGLRenderContext;
    }

    // If this class is the OS device context handle owner, release it:
    if (_isOSDCOwner && (_hDC != NULL))
    {
        ::ReleaseDC(_hWnd, _hDC);
    }
}


// ---------------------------------------------------------------------------
// Name:        GrtPixelFormatSelector::amountOfAvailablePixelFormats
// Description: Return the amount of pixel formats available in this graphic
//              device context.
// Author:      AMD Developer Tools Team
// Date:        6/12/2003
// ---------------------------------------------------------------------------
int oaDeviceContext::amountOfAvailablePixelFormats() const
{
    int retVal = 0;

    // If succeeded, DescribePixelFormat returns the amount of available pixel formats for
    // the given device context. If it fails, it returns 0:
    PIXELFORMATDESCRIPTOR pixelFormatDescriptior;
    retVal = ::DescribePixelFormat(_hDC, 1, sizeof(PIXELFORMATDESCRIPTOR), &pixelFormatDescriptior);

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
// Date:        6/12/2003
// ---------------------------------------------------------------------------
bool oaDeviceContext::getPixelFormat(oaPixelFormatId pixelFormatIndex, gtAutoPtr<oaPixelFormat>& aptrPixelFormat) const
{
    bool retVal = false;

    // Create a wrapper class for the requested pixel format:
    oaPixelFormat* pPixelFormat = new oaPixelFormat(_hDC, pixelFormatIndex);
    GT_IF_WITH_ASSERT(pPixelFormat != NULL)
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
// Description:
//   Search for a pixel format that supports OpenGL.
//   a. Sarch for a fully hardware accelerated OGL pixel format.
//   b. If such a pixel format is not found, search for a partially hardware
//      accelerated pixel format.
//   c. If such a pixel format is not found, search for a pixel format that supports
//      OpenGL.
//
// Return Val: int - Will get the index of a pixel format that supports OpenGL.
//                   or -1 if such a pixel format does not exist in this device context.
//
// Author:      AMD Developer Tools Team
// Date:        13/7/2006
// ---------------------------------------------------------------------------
oaPixelFormatId oaDeviceContext::getOpenGLPixelFormatId() const
{
    oaPixelFormatId retVal = OA_NO_PIXEL_FORMAT_ID;

    // Search for a hardware accelerated pixel format index:
    oaPixelFormatId hardwarePixelFormat = getHardwareAcceleratedOpenGLPixelFormatId();

    // If we found a hardware accelerated pixel format:
    if (hardwarePixelFormat != OA_NO_PIXEL_FORMAT_ID)
    {
        retVal = hardwarePixelFormat;
    }
    else
    {
        // Iterate the available pixel formats:
        int amountOfPixelFormats = amountOfAvailablePixelFormats();

        for (int i = 1; i <= amountOfPixelFormats; i++)
        {
            // Get the current pixel format:
            gtAutoPtr<oaPixelFormat> aptrCurrentPixelFormat;
            bool rc = getPixelFormat(i, aptrCurrentPixelFormat);
            GT_IF_WITH_ASSERT(rc)
            {
                // Does it support OpenGL:
                bool supportsOpenGL = aptrCurrentPixelFormat->supportsOpenGL();

                if (supportsOpenGL)
                {
                    retVal = aptrCurrentPixelFormat->nativeId();

                    if (retVal != OA_NO_PIXEL_FORMAT_ID)
                    {
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaDeviceContext::getHardwareAcceleratedOpenGLPixelFormatId
// Description:
//   Search for a hardware accelerated OpenGL pixel format in this device context.
//   First - search for a fully hardware accelerated OGL pixel format.
//   If such a pixel format is not found, search for a partially hardware
//   accelerated pixel format.
//
// Return Val: int - Will get the index of a hardware accelerated OpenGL pixel format,
//                   or -1 if such a pixel format does not exist in this device context.
//
// Author:      AMD Developer Tools Team
// Date:        12/7/2007
// ---------------------------------------------------------------------------
oaPixelFormatId oaDeviceContext::getHardwareAcceleratedOpenGLPixelFormatId() const
{
    oaPixelFormatId retVal = OA_NO_PIXEL_FORMAT_ID;

    oaPixelFormatId partiallyHardwareAcceleratedPixelFormatIndex = OA_NO_PIXEL_FORMAT_ID;

    // Iterate the available pixel formats:
    int amountOfPixelFormats = amountOfAvailablePixelFormats();

    for (int i = 1; i <= amountOfPixelFormats; i++)
    {
        // Get the current pixel format:
        gtAutoPtr<oaPixelFormat> aptrCurrentPixelFormat;
        bool rc = getPixelFormat(i, aptrCurrentPixelFormat);
        GT_IF_WITH_ASSERT(rc)
        {
            // Does it support OpenGL:
            bool supportsOpenGL = aptrCurrentPixelFormat->supportsOpenGL();

            if (supportsOpenGL)
            {
                // If it is fully hardware accelerated:
                oaPixelFormat::HardwareSupport hardwareSupport = aptrCurrentPixelFormat->hardwareSupport();

                if (hardwareSupport == oaPixelFormat::FULL_HARDWARE_ACCELERATION)
                {
                    // We found the pixel format we looked for:
                    retVal = i;
                    break;
                }
                else
                {
                    // If it is partially hardware accelerated pixel format:
                    if (hardwareSupport == oaPixelFormat::PARTIAL_HARDWARE_ACCELERATION)
                    {
                        // If its the first partially hardware accelerated pixel format:
                        if (partiallyHardwareAcceleratedPixelFormatIndex == -1)
                        {
                            partiallyHardwareAcceleratedPixelFormatIndex = i;
                        }
                    }
                }
            }
        }
    }

    // If a fully hardware accelerated OGL pixel format was not found:
    if (retVal == OA_NO_PIXEL_FORMAT_ID)
    {
        // If there is a partially hardware accelerated pixel format:
        if (partiallyHardwareAcceleratedPixelFormatIndex != OA_NO_PIXEL_FORMAT_ID)
        {
            retVal = partiallyHardwareAcceleratedPixelFormatIndex;
        }
    }

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
// Date:        27/12/2003
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
// Description: Returns the pixel format's ID from an index in the list. This is
//              trivial in windows, since the pixel formats are kept in an indexed
//              list anyway.
// Author:      AMD Developer Tools Team
// Date:        25/5/2008
// ---------------------------------------------------------------------------
oaPixelFormatId oaDeviceContext::getPixelFormatIDFromIndex(int pixelFormatIndex) const
{
    oaPixelFormatId retVal = OA_NO_PIXEL_FORMAT_ID;
    GT_IF_WITH_ASSERT((pixelFormatIndex > 0) && (pixelFormatIndex <= amountOfAvailablePixelFormats()))
    {
        gtAutoPtr<oaPixelFormat> aptrCurrentPixelFormat;
        bool rc = getPixelFormat(pixelFormatIndex, aptrCurrentPixelFormat);
        GT_IF_WITH_ASSERT(rc)
        {
            retVal = aptrCurrentPixelFormat->nativeId();
        }
    }
    return retVal;
}
