//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaPixelFormat.cpp
///
//=====================================================================

//------------------------------ oaPixelFormat.cpp ------------------------------

// GRBaseTools:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <linux/oaPlatformSpecificFunctionPointers.h>
#include <common/oaOpenGLFunctionPointers.h>
#include <AMDTOSAPIWrappers/Include/oaPixelFormat.h>


// ---------------------------------------------------------------------------
// Name:        oaPixelFormat::oaPixelFormat
// Description: Constructor - Buld this representation out of a given XVisualInfo.
// Arguments:   hDC - A handle to the device context in which this pixel format resides.
//              pixelFormatIndex - The index of the pixel format in this context.
// Author:      AMD Developer Tools Team
// Date:        2/6/2003
// Implementation Notes:
//
// ---------------------------------------------------------------------------
oaPixelFormat::oaPixelFormat(oaDeviceContextHandle hDC, oaPixelFormatId pixelFormatIndex)
    : _hDC(hDC), _nativeId(pixelFormatIndex), _isInitialized(false),
      _supportsOpenGL(false), _supportsNativeRendering(false), _isDoubleBuffered(false),
      _isStereoscopic(false), _hardwareSupport(NO_HARDWARE_ACCELERATION), _pixelType(RGBA),
      _amountOfColorBits(0), _amountOfRedBits(0), _amountOfGreenBits(0), _amountOfBlueBits(0),
      _amountOfAlphaBits(0), _amountOfZBufferBits(0), _amountOfAccumulationBufferBits(0),
      _amountOfStencilBufferBits(0)
{
}

// ---------------------------------------------------------------------------
// Name:        oaPixelFormat::~oaPixelFormat
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        2/1/2006
// ---------------------------------------------------------------------------
oaPixelFormat::~oaPixelFormat()
{
}

// ---------------------------------------------------------------------------
// Name:        oaPixelFormat::initialize
// Description: Initializes oaPixelFormat attributes from the corresponding
//              VisualID.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        6/12/2003
// ---------------------------------------------------------------------------
bool oaPixelFormat::initialize()
{
    XVisualInfo* visualInfo = NULL;
    XVisualInfo idOnlyVisInf;
    idOnlyVisInf.visualid = _nativeId;
    idOnlyVisInf.screen = XDefaultScreen(_hDC);
    int numOfVisuals = 0;

    visualInfo = XGetVisualInfo(_hDC, VisualIDMask | VisualScreenMask, &idOnlyVisInf, &numOfVisuals);

    if (visualInfo == NULL)
    {
        gtString failedVisInfo;
        failedVisInfo.appendFormattedString(L"Could not find a visual with ID %d, trying default visual instead", _nativeId);
        GT_ASSERT_EX((visualInfo != NULL), failedVisInfo.asCharArray());

        idOnlyVisInf.visualid = XVisualIDFromVisual(XDefaultVisual(_hDC, XDefaultScreen(_hDC)));
        visualInfo = XGetVisualInfo(_hDC, VisualIDMask | VisualScreenMask, &idOnlyVisInf, &numOfVisuals);
    }

    GT_IF_WITH_ASSERT((visualInfo != NULL) && (numOfVisuals != 0))
    {
        // Fill the pixel format attributes:
        int* value = new int;

        // Make sure that we a pointer to the system's glGetString function:
        bool rcOGLModule = oaLoadOpenGLFunctionPointers();
        GT_IF_WITH_ASSERT(rcOGLModule && (pOAglGetString != NULL))
        {
            // Get the OpenGL vendor string:
            gtString openGLRendererString;
            const char* pGLRendererString = (const char*)pOAglGetString(GL_VENDOR);

            if (pGLRendererString != NULL)
            {
                openGLRendererString.fromASCIIString(pGLRendererString);
            }

            // Check if this pixel format is exported by a software renderer:
            static const gtString softwareRendererName = L"Mesa";

            if (!openGLRendererString.isEmpty() && (openGLRendererString.find(softwareRendererName) != -1))
            {
                _hardwareSupport = NO_HARDWARE_ACCELERATION;
            }
            else
            {
                _hardwareSupport = FULL_HARDWARE_ACCELERATION;
            }
        }

        // Make sure that we a pointer to the system's glXGetConfig function:
        bool rcGLXModule = oaLoadGLXFunctionPointers();
        GT_IF_WITH_ASSERT(rcGLXModule && (pOAglXGetConfig != NULL))
        {
            int rcVisualInfo = pOAglXGetConfig(_hDC, visualInfo, GLX_USE_GL, value);
            GT_IF_WITH_ASSERT(rcVisualInfo == Success)
            {
                _supportsOpenGL = (*value == True);
                _supportsNativeRendering = true;
                int rcDoubleBuff = pOAglXGetConfig(_hDC, visualInfo, GLX_DOUBLEBUFFER, value);
                GT_IF_WITH_ASSERT(rcDoubleBuff  == Success)
                {
                    _isDoubleBuffered = (*value == True);
                }

                int rcStereo = pOAglXGetConfig(_hDC, visualInfo, GLX_STEREO, value);
                GT_IF_WITH_ASSERT(rcStereo == Success)
                {
                    _isStereoscopic = (*value == True);
                }

                int rcRGBA = pOAglXGetConfig(_hDC, visualInfo, GLX_RGBA, value);
                GT_IF_WITH_ASSERT(rcRGBA == Success)
                {
                    if (*value == True)
                    {
                        _pixelType = RGBA;
                    }
                    else
                    {
                        _pixelType = COLOR_INDEX;
                    }
                }

                int rcBuffSize = pOAglXGetConfig(_hDC, visualInfo, GLX_BUFFER_SIZE, value);
                GT_IF_WITH_ASSERT(rcBuffSize  == Success)
                {
                    _amountOfColorBits = *value;
                }

                int rcRedSize = pOAglXGetConfig(_hDC, visualInfo, GLX_RED_SIZE, value);
                GT_IF_WITH_ASSERT(rcRedSize == Success)
                {
                    _amountOfRedBits = *value;
                }

                int rcGreenSize = pOAglXGetConfig(_hDC, visualInfo, GLX_GREEN_SIZE, value);
                GT_IF_WITH_ASSERT(rcGreenSize == Success)
                {
                    _amountOfGreenBits = *value;
                }

                int rcBlueSize = pOAglXGetConfig(_hDC, visualInfo, GLX_BLUE_SIZE, value);
                GT_IF_WITH_ASSERT(rcBlueSize == Success)
                {
                    _amountOfBlueBits = *value;
                }

                int rcAlphaSize = pOAglXGetConfig(_hDC, visualInfo, GLX_ALPHA_SIZE, value);
                GT_IF_WITH_ASSERT(rcAlphaSize == Success)
                {
                    _amountOfAlphaBits = *value;
                }

                int rcDepthSize = pOAglXGetConfig(_hDC, visualInfo, GLX_DEPTH_SIZE, value);
                GT_IF_WITH_ASSERT(rcDepthSize == Success)
                {
                    _amountOfZBufferBits = *value;
                }

                int rcAccRed = pOAglXGetConfig(_hDC, visualInfo, GLX_ACCUM_RED_SIZE, value);
                GT_IF_WITH_ASSERT(rcAccRed == Success)
                {
                    _amountOfAccumulationBufferBits = *value;
                }

                int rcAccGreen = pOAglXGetConfig(_hDC, visualInfo, GLX_ACCUM_GREEN_SIZE, value);
                GT_IF_WITH_ASSERT(rcAccGreen == Success)
                {
                    _amountOfAccumulationBufferBits += *value;
                }

                int rcAccBlue = pOAglXGetConfig(_hDC, visualInfo, GLX_ACCUM_BLUE_SIZE, value);
                GT_IF_WITH_ASSERT(rcAccBlue == Success)
                {
                    _amountOfAccumulationBufferBits += *value;
                }

                int rcAccAlpha = pOAglXGetConfig(_hDC, visualInfo, GLX_ACCUM_ALPHA_SIZE, value);
                GT_IF_WITH_ASSERT(rcAccAlpha == Success)
                {
                    _amountOfAccumulationBufferBits += *value;
                }

                int rcStencilSize = pOAglXGetConfig(_hDC, visualInfo, GLX_STENCIL_SIZE, value);
                GT_IF_WITH_ASSERT(rcStencilSize == Success)
                {
                    _amountOfStencilBufferBits = *value;
                }
            }
        }

        delete value;
        _isInitialized = true;
    }

    // delete windowAttribs;
    if (visualInfo != NULL)
    {
        XFree(visualInfo);
    }

    return _isInitialized;
}

// ---------------------------------------------------------------------------
// Name:        oaPixelFormat::initializeGLESPixelFormatWithChannelValues
// Description: Initializes the Pixel format objects using default values and values
//              obtained from OpenGL ES. Should only be used by EAGL (iPhone).
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        23/5/2010
// ---------------------------------------------------------------------------
bool oaPixelFormat::initializeGLESPixelFormatWithChannelValues(int amountOfRBits, int amountOfGBits, int amountOfBBits, int amountOfABits, int amountOfDepBits, int amountOfStenBits)
{
    (void)(amountOfRBits); // unused
    (void)(amountOfGBits); // unused
    (void)(amountOfBBits); // unused
    (void)(amountOfABits); // unused
    (void)(amountOfDepBits); // unused
    (void)(amountOfStenBits); // unused
    // We should not get here on non-EAGL implementations:
    GT_ASSERT(false);

    return _isInitialized;
}
