//------------------------------ oaOpenGLRenderContext.cpp ------------------------------

// GRBaseTools:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <mac/oaPlatformSpecificFunctionPointers.h>
#include <AMDTOSAPIWrappers/Include/oaPixelFormat.h>
#include <AMDTOSAPIWrappers/Include/oaHiddenWindow.h>
#include <AMDTOSAPIWrappers/Include/oaDeviceContext.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLRenderContext.h>

// Mac OS X
#ifndef _GR_IPHONE_BUILD
    #include <OpenGL/OpenGL.h>
#else
    // TO_DO iPhone: EAGL includes
#endif

// Static members initializations:
oaOpenGLRenderContext* oaOpenGLRenderContext::_pDefaultRenderContext = NULL;
oaHiddenWindow* oaOpenGLRenderContext::_pDefaultRenderContextWindow = NULL;

gtByte* oaOffscreenRenderingBuffer = NULL;


// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::oaOpenGLRenderContext
// Description: Constructor - Creates an OpenGL render context out of a
//                            graphic device context.
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
oaOpenGLRenderContext::oaOpenGLRenderContext(const oaDeviceContext& deviceContext)
    : _myDeviceContext(deviceContext), _hOpenGLRC(NULL), _vendorType(OS_VENDOR_UNKNOWN), _vendorString("")
{
    oaPixelFormatId pix = _myDeviceContext.getHardwareAcceleratedOpenGLPixelFormatId();

    // If we are NOT on iPhone platforms:
#ifndef _GR_IPHONE_BUILD
    {
        // Make sure that CGL function pointers are loaded:
        bool rcOCLFuncs = oaLoadCGLFunctionPointers();
        GT_IF_WITH_ASSERT(rcOCLFuncs)
        {
            CGLError errorCode = pOACGLCreateContext(pix, (CGLContextObj)NULL, &_hOpenGLRC);
            GT_ASSERT(errorCode == kCGLNoError);
        }
    }
#else
    {
        // TO_DO iPhone: create EAGL context
    }
#endif
}

// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::~oaOpenGLRenderContext
// Description: Destructor.
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
oaOpenGLRenderContext::~oaOpenGLRenderContext()
{
    // If we are NOT on iPhone platforms:
#ifndef _GR_IPHONE_BUILD
    {
        // Make sure that CGL function pointers are loaded:
        bool rcOCLFuncs = oaLoadCGLFunctionPointers();
        GT_IF_WITH_ASSERT(rcOCLFuncs)
        {
            GLsizei height = 0, width = 0, bytesPerRow = 0;
            void* pOffscreenBuffer = NULL;
            CGLError errorCode = pOACGLGetOffScreen(_hOpenGLRC, &width, &height, &bytesPerRow, &pOffscreenBuffer);
            GT_IF_WITH_ASSERT(errorCode == kCGLNoError)
            {
                // Make sure everything's as we expect it:
                GT_ASSERT((width == 2) && (height == 2) && (pOffscreenBuffer == (void*) oaOffscreenRenderingBuffer));
            }

            // Release the context from the drawable:
            errorCode = pOACGLClearDrawable(_hOpenGLRC);
            GT_ASSERT_EX((errorCode == kCGLNoError), "Could not release context");

            delete[] oaOffscreenRenderingBuffer;

            if (_hOpenGLRC != NULL)
            {
                errorCode = pOACGLDestroyContext(_hOpenGLRC);
                GT_ASSERT_EX((errorCode == kCGLNoError), "Could not destroy context");
                _hOpenGLRC = NULL;
            }
        }
    }
#else
    {
        // TO_DO iPhone: EAGL cleanup
    }
#endif
}


// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::makeCurrent
// Description: Makes this OpenGL rendering context the calling thread's current rendering
//              context. (All OpenGL calls made by this thread will be performed on
//              this render context). The context is made current to an offscreen
//              buffer, 2x2 pixels in size
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
bool oaOpenGLRenderContext::makeCurrent()
{
    bool retVal = false;
    oaPixelFormatId pix = _myDeviceContext.getHardwareAcceleratedOpenGLPixelFormatId();

    // Calculate the number of bytes per pixel in this pixel format:
    unsigned int bytesPerPixel = 0;

    oaPixelFormat pixForm((oaDeviceContextHandle)NULL, pix);
    pixForm.initialize();
    bytesPerPixel += pixForm.amountOfColorBits();
    //  bytesPerPixel += pixForm.amountOfAlphaBits();
    bytesPerPixel += pixForm.amountOfAccumulationBufferBits();
    bytesPerPixel += pixForm.amountOfZBufferBits();

    GT_IF_WITH_ASSERT(bytesPerPixel > 0)
    {
        // If we are NOT on iPhone platforms:
#ifndef _GR_IPHONE_BUILD
        {
            // Make sure that CGL function pointers are loaded:
            bool rcOCLFuncs = oaLoadCGLFunctionPointers();
            GT_IF_WITH_ASSERT(rcOCLFuncs)
            {
                CGLError errorCode = pOACGLSetCurrentContext(_hOpenGLRC);
                GT_IF_WITH_ASSERT(errorCode == kCGLNoError)
                {
                    if (pixForm.hardwareSupport() == oaPixelFormat::FULL_HARDWARE_ACCELERATION)
                    {
                        // No need to set a drawable for the queries we will perform
                        retVal = true;
                    }
                    else
                    {
                        // This is a software (offscreen) renderer, make an offscreen buffer for it
                        // and set it as the drawable for the context
                        if (oaOffscreenRenderingBuffer == NULL)
                        {
                            oaOffscreenRenderingBuffer = new gtByte[2 * 2 * bytesPerPixel];
                        }

                        GT_IF_WITH_ASSERT(oaOffscreenRenderingBuffer != NULL)
                        {
                            // Make this render context the "current context" of the calling thread, but set
                            // set it to an offscreen buffer (since "CGLSetCurrentContext" renders to the whole screen):
                            errorCode = pOACGLSetOffScreen(_hOpenGLRC,
                                                           2,                                   // Width
                                                           2,                                   // Height
                                                           (2 * bytesPerPixel),                 // Bytes per row
                                                           (void*) oaOffscreenRenderingBuffer); // Target
                            retVal = (errorCode == kCGLNoError);
                        }
                    }
                }
            }
        }
#else // _GR_IPHONE_BUILD
        {
            // TO_DO iPhone: Make EAGL content current
        }
#endif // _GR_IPHONE_BUILD
    }

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::platformSpecificExtensionsString
// Description: There are no extensions to CGL!
// Arguments: extensions - the list goes here
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        23/3/2009
// ---------------------------------------------------------------------------
bool oaOpenGLRenderContext::platformSpecificExtensionsString(gtString& extensions)
{
    bool retVal = false;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::InitHiddenWindow
// Description:
//   Create the hidden window that is used for creating the context
//   This function is public to allow the main thread to create the window before any other thread attempts to access
//   the render context. This is because on Windows only the thread that created a window is allowed to destroy it.
//
// Return Val: true if creation succeeded, false otherwise
//
// Author:      Doron Ofek
// Date:        Oct-22, 2015
// ---------------------------------------------------------------------------
bool oaOpenGLRenderContext::InitHiddenWindow()
{
    bool retVal = true;

    if (nullptr == _pDefaultRenderContextWindow)
    {
        // Create a window class:
        _pDefaultRenderContextWindow = new oaHiddenWindow;

        // Create a native hidden window (2X2 pixels):
        retVal = _pDefaultRenderContextWindow->create(L"oaOpenGLRenderContext::getDefaultRenderContext hidden window", 0, 0, 2, 2);
        GT_ASSERT(retVal)
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::getDefaultRenderContext
// Description:
//   Returns the "default" OpenGL render context.
//   The "default" OpenGL render context is created at the first call to
//   this function. Applications can use it to query data that can be accessed
//   only when there is an active render context.
//
// Return Val: oaOpenGLRenderContext* - The "default" OpenGL render context
//                                      or NULL if its createion failed.
//
// Author:      Uri Shomroni
// Date:        11/12/2008
// ---------------------------------------------------------------------------
oaOpenGLRenderContext* oaOpenGLRenderContext::getDefaultRenderContext()
{
    oaOpenGLRenderContext* retVal = NULL;

    // If the "default" render context was already created:
    if (_pDefaultRenderContext != NULL)
    {
        // Simply return its static instance:
        retVal = _pDefaultRenderContext;
    }
    else
    {
        if (InitHiddenWindow())
        {
            // Get the window device context:
            oaDeviceContext* pDisplay = _pDefaultRenderContextWindow->deviceContext();
            GT_IF_WITH_ASSERT(pDisplay)
            {
                rc = pDisplay->createOpenGLRenderContext();
                GT_IF_WITH_ASSERT(rc)
                {
                    // Get the created render context:
                    _pDefaultRenderContext = pDisplay->openGLRenderContext();
                    GT_IF_WITH_ASSERT(_pDefaultRenderContext)
                    {
                        // Extract OpenGL vendor:
                        rc = _pDefaultRenderContext->extractVendorType();
                        GT_ASSERT(rc);

                        retVal = _pDefaultRenderContext;
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::getOpenGLProcAddress
// Description: Wrapper for wglGetProcAddress / glXGetProcAddress. Should not
//              be used in Mac.
// Author:      Uri Shomroni
// Date:        22/2/2010
// ---------------------------------------------------------------------------
void* oaOpenGLRenderContext::getOpenGLProcAddress(const char* pFuncName)
{
    void* retVal = NULL;

    GT_ASSERT(false);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::getGPUInformation
// Description: GPU affinity is currently not supported on Mac
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        23/3/2009
// ---------------------------------------------------------------------------
bool oaOpenGLRenderContext::getGPUInformation(unsigned long gpuIndex, gtString& gpuName, intptr_t& hGPU)
{
    GT_ASSERT(false);
    return false;
}

