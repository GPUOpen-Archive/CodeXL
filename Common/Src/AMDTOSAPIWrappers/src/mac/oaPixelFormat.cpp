//------------------------------ oaPixelFormat.cpp ------------------------------

// GRBaseTools:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <mac/oaPlatformSpecificFunctionPointers.h>
#include <AMDTOSAPIWrappers/Include/oaPixelFormat.h>

// Mac OS X
#ifndef _GR_IPHONE_BUILD
    #include <OpenGL/OpenGL.h>
#else
    // TO_DO iPhone: EAGL includes
#endif


// ---------------------------------------------------------------------------
// Name:        oaPixelFormat::oaPixelFormat
// Description: Constructor - Build this representation out of a given CGLPixelFormatObj.
// Arguments:   hDC - A handle to the device context in which this pixel format resides.
//              pixelFormatIndex - The index of the pixel format in this context.
// Author:      Uri Shomroni
// Date:        3/12/2008
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
// Author:      Uri Shomroni
// Date:        3/12/2008
// ---------------------------------------------------------------------------
oaPixelFormat::~oaPixelFormat()
{
}

// ---------------------------------------------------------------------------
// Name:        oaPixelFormat::initialize
// Description: Initializes oaPixelFormat attributes from the corresponding
//              pixel format id.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        3/12/2008
// ---------------------------------------------------------------------------
bool oaPixelFormat::initialize()
{
    //////////////////////////////////////////////////////////////////////////
    // Uri, 10/12/08 - Looking at CGL (and AGL) documentation, there doesn't
    //  seem to be any sort of unique identifier to a pixel format in mac
    //  (meaning that two identical calls to "choose pixel format" functions
    //  can theoretically result in two different pixel formats. This was
    //  confirmed by Apple (according to them there is no way to enumerate
    //  CGL pixel formats. As a result, we can currently only use oaPixelFormat
    //  in mac to determine whether a render context is hardware accelerated
    //  or not (we disable the "list all pixel formats" option), so we simply
    //  initialize the oaPixelFormat to be the current one. To do this, we
    //  outwardsly refer to the pixel format handle (CGLPixelFormatObj) as if
    //  it were an ID. Note that this might obviously cause complications if
    //  trying to generate a pixel format from a numeric ID.
    //
    // http://developer.apple.com/documentation/GraphicsImaging/Reference/CGL_OpenGL/Reference/reference.html
    //////////////////////////////////////////////////////////////////////////

    GT_IF_WITH_ASSERT(_nativeId != OA_NO_PIXEL_FORMAT_ID)
    {
#ifdef _GR_IPHONE_BUILD
        {
            // On the iPhone (EAGL), our pixel format objects need data from OpenGL ES to get initialized, so we cannot initialize them here:
            if (!_isInitialized)
            {
                GT_ASSERT(false);
            }
        }
#else // ndef _GR_IPHONE_BUILD
        // If we are NOT on iPhone platforms:
        {
            // Make sure that CGL function pointers are loaded:
            bool rcOCLFuncs = oaLoadCGLFunctionPointers();
            GT_IF_WITH_ASSERT(rcOCLFuncs)
            {
                // Fill the CGL pixel format attributes:

                GLint paramValue = -1;

                CGLError cglError = pOACGLDescribePixelFormat(_nativeId, 0, kCGLPFAAccelerated, &paramValue);

                if ((cglError != kCGLNoError) || (paramValue == 0))
                {
                    _hardwareSupport = NO_HARDWARE_ACCELERATION;
                    GT_ASSERT(cglError == kCGLNoError);
                }
                else
                {
                    _hardwareSupport = FULL_HARDWARE_ACCELERATION;
                }

                // Uri, 28/1/09: according to an email we recieved from apple on 9/12/08, all OS X pixel formats
                // support OpenGL and native rendering.
                _supportsOpenGL = true;
                _supportsNativeRendering = true;

                cglError = pOACGLDescribePixelFormat(_nativeId, 0, kCGLPFADoubleBuffer, &paramValue);
                GT_IF_WITH_ASSERT(cglError == kCGLNoError)
                {
                    _isDoubleBuffered = (paramValue != 0);
                }
                cglError = pOACGLDescribePixelFormat(_nativeId, 0, kCGLPFAStereo, &paramValue);
                GT_IF_WITH_ASSERT(cglError == kCGLNoError)
                {
                    _isStereoscopic = (paramValue != 0);
                }

                // CGL does not support indexed color rendering (and thus pixel formats can never be indexed)
                _pixelType = RGBA;

                cglError = pOACGLDescribePixelFormat(_nativeId, 0, kCGLPFAColorSize, &paramValue);
                GT_IF_WITH_ASSERT(cglError == kCGLNoError)
                {
                    _amountOfColorBits = paramValue;
                }
                cglError = pOACGLDescribePixelFormat(_nativeId, 0, kCGLPFAAlphaSize, &paramValue);
                GT_IF_WITH_ASSERT(cglError == kCGLNoError)
                {
                    _amountOfAlphaBits = paramValue;
                }

                // Uri, 3/12/08 - It seems that CGL does not specify each color channel's
                //  size separately - not sure if each channel gets exactly 1/3 of the color
                //  buffer size - hence we can assume _amountOfColorBits is divisble by 3.
                //  According to an email from Applie on 9/12/08, if there was ever a pixel format
                //  which had different sizes to the channels, this fact could only be verified
                //  by querying a render context created with the pixel format.
                //
                // Since this information is presently only used by the Pixel Formats tab in the
                // system information dialog (this tab is non-existant in Mac), we fill it with the
                // above-mentioned educated guess, until the time when we might need it. The following
                // commented out code is if we ever find a way to get it from CGL.
                if (cglError == kCGLNoError)
                {
                    // Note the above condition is true iff we succeeded in getting the number of alpha bits:
                    int amountOfRGBBits = _amountOfColorBits - _amountOfAlphaBits;

                    // Sanity Check:
                    GT_IF_WITH_ASSERT(amountOfRGBBits >= 0)
                    {
                        // We give any extra bits to the red channel, just so there
                        // won't be any "missing" bits:
                        _amountOfRedBits = (amountOfRGBBits / 3) + (amountOfRGBBits % 3);
                        _amountOfGreenBits = (amountOfRGBBits / 3);
                        _amountOfBlueBits = (amountOfRGBBits / 3);
                    }
                }

                /*
                cglError = pOACGLDescribePixelFormat(_nativeId, 0, ???, &paramValue);
                GT_IF_WITH_ASSERT(cglError == kCGLNoError)
                {
                    _amountOfRedBits = paramValue;
                }
                cglError = pOACGLDescribePixelFormat(_nativeId, 0, ???, &paramValue);
                GT_IF_WITH_ASSERT(cglError == kCGLNoError)
                {
                    _amountOfGreenBits = paramValue;
                }
                cglError = pOACGLDescribePixelFormat(_nativeId, 0, ???, &paramValue);
                GT_IF_WITH_ASSERT(cglError == kCGLNoError)
                {
                    _amountOfBlueBits = paramValue;
                }
                */

                cglError = pOACGLDescribePixelFormat(_nativeId, 0, kCGLPFADepthSize, &paramValue);
                GT_IF_WITH_ASSERT(cglError == kCGLNoError)
                {
                    _amountOfZBufferBits = paramValue;
                }
                cglError = pOACGLDescribePixelFormat(_nativeId, 0, kCGLPFAAccumSize, &paramValue);
                GT_IF_WITH_ASSERT(cglError == kCGLNoError)
                {
                    _amountOfAccumulationBufferBits = paramValue;
                }
                cglError = pOACGLDescribePixelFormat(_nativeId, 0, kCGLPFAStencilSize, &paramValue);
                GT_IF_WITH_ASSERT(cglError == kCGLNoError)
                {
                    _amountOfStencilBufferBits = paramValue;
                }
            }
        }
#endif // _GR_IPHONE_BUILD

        _isInitialized = true;
    }

    return _isInitialized;
}


// ---------------------------------------------------------------------------
// Name:        oaPixelFormat::initializeGLESPixelFormatWithChannelValues
// Description: Initializes the Pixel format objects using default values and values
//              obtained from OpenGL ES. Should only be used by EAGL (iPhone).
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        23/5/2010
// ---------------------------------------------------------------------------
bool oaPixelFormat::initializeGLESPixelFormatWithChannelValues(int amountOfRBits, int amountOfGBits, int amountOfBBits, int amountOfABits, int amountOfDepBits, int amountOfStenBits)
{
#ifdef _GR_IPHONE_BUILD

    if (!_isInitialized)
    {
        // EAGL always supports hardware rendering:
        _hardwareSupport = FULL_HARDWARE_ACCELERATION;

        // EAGL does not have static buffers, and so is not double-buffered:
        _isDoubleBuffered = false;

        // OpenGL ES does not support stereoscopy:
        _isStereoscopic = false;

        // OpenGL ES does not support color index mode:
        _pixelType = RGBA;

        // Set the values obtained from the OpenGL ES implementation:
        _amountOfColorBits = amountOfRBits + amountOfGBits + amountOfBBits;
        _amountOfRedBits = amountOfRBits;
        _amountOfGreenBits = amountOfGBits;
        _amountOfBlueBits = amountOfBBits;
        _amountOfAlphaBits = amountOfABits;
        _amountOfZBufferBits = amountOfDepBits;
        _amountOfStencilBufferBits = amountOfStenBits;

        // OpenGL ES does not support accumulation buffers:
        _amountOfAccumulationBufferBits = 0;

        // Mark that we have initialized properly:
        _isInitialized = true;
    }

#else // ndef _GR_IPHONE_BUILD
    // We should not get here on non-EAGL implementations:
    GT_ASSERT(false);
#endif

    return _isInitialized;
}
