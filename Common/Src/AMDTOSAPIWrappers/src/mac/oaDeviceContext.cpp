//------------------------------ oaDeviceContext.cpp ------------------------------

// Mac OS X
#ifndef _GR_IPHONE_BUILD
    #include <ApplicationServices/ApplicationServices.h>
#endif

// GRBaseTools:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <inc/mac/oaPlatformSpecificFunctionPointers.h>
#include <AMDTOSAPIWrappers/Include/oaPixelFormat.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLRenderContext.h>
#include <AMDTOSAPIWrappers/Include/oaDeviceContext.h>

// This member is as we assume that there is only one oaOGLRC-oaDC-oaHW combination
// active at any time:
oaPixelFormatId oaDeviceContextHelperPixelFormat = OA_NO_PIXEL_FORMAT_ID;


// ---------------------------------------------------------------------------
// Name:        oaGetoaDeviceContextHelperPixelFormatId
// Description: Gets the oaPixelFormatId which we use for OSWrappers-created
//              OGL objects (or creates it if needed)
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
oaPixelFormatId oaGetoaDeviceContextHelperPixelFormatId()
{
    oaPixelFormatId retVal = OA_NO_PIXEL_FORMAT_ID;

    if (oaDeviceContextHelperPixelFormat == OA_NO_PIXEL_FORMAT_ID)
    {
        // If we are NOT on iPhone platforms:
#ifndef _GR_IPHONE_BUILD
        {
            // Make sure that CGL function pointers are loaded:
            bool rcOCLFuncs = oaLoadCGLFunctionPointers();
            GT_IF_WITH_ASSERT(rcOCLFuncs)
            {
                // Create the CGL pixel format:
                CGLPixelFormatAttribute glDisplaysMask = (CGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay); // We require the main display to be available

                // First try with the "best" attributes
                static CGLPixelFormatAttribute osPixelFormatAttribs1[] =
                {
                    kCGLPFAAccelerated,
                    kCGLPFANoRecovery,
                    kCGLPFADisplayMask,
                    glDisplaysMask,
                    (CGLPixelFormatAttribute)NULL
                };

                long numberOfFormats = -1;
                CGLError errorCode = pOACGLChoosePixelFormat(osPixelFormatAttribs1, &oaDeviceContextHelperPixelFormat, &numberOfFormats);

                if ((errorCode == kCGLNoError) && (numberOfFormats > 0))
                {
                    // Set the return value:
                    retVal = oaDeviceContextHelperPixelFormat;
                }
                else
                {
                    // The first try failed, try with "lower" attributes (mainly, do not require hardware acceleration):
                    static CGLPixelFormatAttribute osPixelFormatAttribs2[] =
                    {
                        kCGLPFAOffScreen,
                        kCGLPFADisplayMask,
                        glDisplaysMask,
                        (CGLPixelFormatAttribute)NULL
                    };

                    errorCode = pOACGLChoosePixelFormat(osPixelFormatAttribs2, &oaDeviceContextHelperPixelFormat, &numberOfFormats);

                    if ((errorCode == kCGLNoError) || (numberOfFormats > 0))
                    {
                        // Set the return value:
                        retVal = oaDeviceContextHelperPixelFormat;
                    }
                    else
                    {
                        // We have a problem:
                        GT_ASSERT_EX(false, L"Could not find a suitable pixel format!");
                        oaDeviceContextHelperPixelFormat = OA_NO_PIXEL_FORMAT_ID;
                    }
                }
            }
        }
#else
        {
            // TO_DO iPhone: create this as an EAGL pixel format
        }
#endif
    }
    else
    {
        // Return the existing pixel format:
        retVal = oaDeviceContextHelperPixelFormat;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaDestroyoaDeviceContextHelperPixelFormatId
// Description: Destroys the helper pixel format
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
void oaDestroyoaDeviceContextHelperPixelFormatId()
{
    if (oaDeviceContextHelperPixelFormat != OA_NO_PIXEL_FORMAT_ID)
    {
        // If we are NOT on iPhone platforms:
#ifndef _GR_IPHONE_BUILD
        {
            // Make sure that CGL function pointers are loaded:
            bool rcOCLFuncs = oaLoadCGLFunctionPointers();
            GT_IF_WITH_ASSERT(rcOCLFuncs)
            {
                CGLError errorCode = pOACGLDestroyPixelFormat(oaDeviceContextHelperPixelFormat);
                GT_ASSERT(errorCode == kCGLNoError);
                oaDeviceContextHelperPixelFormat = OA_NO_PIXEL_FORMAT_ID;
            }
        }
#else
        {
            // TO_DO iPhone
        }
#endif
    }
}


// ---------------------------------------------------------------------------
// Name:        oaDeviceContext::oaDeviceContext
// Description: Constructor
// Arguments:   This class is fairly fictitious, see implementation notes in
//              the header file
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
oaDeviceContext::oaDeviceContext(oaWindowHandle hWnd, oaHiddenWindow* pRelatedWindow)
    : _hWnd(hWnd), _hDC(NULL), _isOSDCOwner(false), _pOGLRenderContext(NULL), _pRelatedWindow(pRelatedWindow)
{

}

// ---------------------------------------------------------------------------
// Name:        oaDeviceContext::oaDeviceContext
// Description: Constructor
// Arguments:   This class is fairly fictitious, see implementation notes in
//              the header file
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
oaDeviceContext::oaDeviceContext(oaWindowHandle hWnd, oaDeviceContextHandle hDC, oaHiddenWindow* pRelatedWindow)
    : _hWnd(hWnd), _hDC(hDC), _isOSDCOwner(false), _pOGLRenderContext(NULL), _pRelatedWindow(pRelatedWindow)
{
    GT_ASSERT_EX(hDC == NULL, "Unexpected Device Context Handle");
}

// ---------------------------------------------------------------------------
// Name:        oaDeviceContext::~oaDeviceContext
// Description: Destructor.
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
oaDeviceContext::~oaDeviceContext()
{
    // If a render context was created:
    if (_pOGLRenderContext != NULL)
    {
        // Release it:
        delete _pOGLRenderContext;
    }

    // Release the pixel format ID if it exists:
    oaDestroyoaDeviceContextHelperPixelFormatId();
}

// ---------------------------------------------------------------------------
// Name:        GrtPixelFormatSelector::amountOfAvailablePixelFormats
// Description: Return the amount of pixel formats available in this graphic
//              device context.
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
int oaDeviceContext::amountOfAvailablePixelFormats() const
{
    int retVal = 1;

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
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
bool oaDeviceContext::getPixelFormat(oaPixelFormatId pixelFormatIndex, gtAutoPtr<oaPixelFormat>& aptrPixelFormat) const
{
    bool retVal = false;

    // Create a wrapper class for the requested pixel format:
    oaPixelFormat* pPixelFormat = new oaPixelFormat(NULL, pixelFormatIndex);
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
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
oaPixelFormatId oaDeviceContext::getOpenGLPixelFormatId() const
{
    oaPixelFormatId retVal = oaGetoaDeviceContextHelperPixelFormatId();

    // Since there is no way in CGL to obtain a pixel format from a render context
    // (CGL pixel formats are temporary objects that are destroyed after usage), we hold
    // a single pixel format for all queries.

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaDeviceContext::getHardwareAcceleratedOpenGLPixelFormatId
// Description: This function is irrelevant since Linux does not differentiate pixel formats
//              for different Displays
//
// Return Val: see getOpenGLPixelFormatId
//
// Author:      Uri Shomroni
// Date:        11/12/2008
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
// Author:      Uri Shomroni
// Date:        11/12/2008
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
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
oaPixelFormatId oaDeviceContext::getPixelFormatIDFromIndex(int pixelFormatIndex) const
{
    oaPixelFormatId retVal = oaGetoaDeviceContextHelperPixelFormatId();

    return retVal;
}
