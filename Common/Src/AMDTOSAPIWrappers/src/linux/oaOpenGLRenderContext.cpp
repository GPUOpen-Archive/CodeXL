//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaOpenGLRenderContext.cpp
///
//=====================================================================

//------------------------------ oaOpenGLRenderContext.cpp ------------------------------

// GRBaseTools:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <linux/oaPlatformSpecificFunctionPointers.h>
#include <AMDTOSAPIWrappers/Include/oaHiddenWindow.h>
#include <AMDTOSAPIWrappers/Include/oaDeviceContext.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLRenderContext.h>

// Static members initializations:
oaOpenGLRenderContext* oaOpenGLRenderContext::_pDefaultRenderContext = NULL;

// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::oaOpenGLRenderContext
// Description: Constructor - Creates an OpenGL render context out of a
//                            graphic device context.
// Author:      AMD Developer Tools Team
// Date:        20/5/2008
// ---------------------------------------------------------------------------
oaOpenGLRenderContext::oaOpenGLRenderContext(const oaDeviceContext& deviceContext)
    : _myDeviceContext(deviceContext), _hOpenGLRC(NULL), _vendorType(OA_VENDOR_UNKNOWN), _vendorString(L"")
{
    // Get the native Display handle:
    oaDeviceContextHandle hDC = deviceContext.nativeDeviceContextHandle();

    // Get the pixel format from the window:
    oaPixelFormatHandle hPF = OA_NO_PIXEL_FORMAT_HANDLE;
    XVisualInfo* pCreatedVisualInfo = NULL;
    const oaHiddenWindow* pWindow = deviceContext.getRelatedWindow();
    GT_IF_WITH_ASSERT(pWindow != NULL)
    {
        hPF = pWindow->getPixelFormatHandle();
    }

    // If we couldn't get the pixel format, try a fallback:
    if (hPF == OA_NO_PIXEL_FORMAT_HANDLE)
    {
        // This shouldn't happen:
        GT_ASSERT(hPF == OA_NO_PIXEL_FORMAT_HANDLE)

        // Get the device context's default visual, it might work:
        pCreatedVisualInfo = new XVisualInfo;

#if defined(__cplusplus) || defined(c_plusplus)
        int c_class = DefaultVisual(hDC, DefaultScreen(hDC))->c_class;
#else
        int c_class = DefaultVisual(hDC, DefaultScreen(hDC))->class;
#endif
        int depth = DefaultDepth(hDC, DefaultScreen(hDC));

        XMatchVisualInfo(hDC, DefaultScreen(hDC), depth, c_class, pCreatedVisualInfo);

        hPF = pCreatedVisualInfo;
    }

    // Make sure we have the GLX function pointers:
    bool rcProc = oaLoadGLXFunctionPointers();
    GT_IF_WITH_ASSERT(rcProc)
    {
        // Create a render context:
        _hOpenGLRC = pOAglXCreateContext(hDC, hPF, NULL, GL_TRUE);  // NULL = No sharing of display lists GL_TRUE = Direct rendering if possible

        // Sanity test:
        GT_ASSERT_EX(NULL != _hOpenGLRC, L"Could not create OpenGL context on the default display. This is probably since the application is being run in command-line mode.");
    }

    // If we created a visual info, delete it:
    delete pCreatedVisualInfo;
}


// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::~oaOpenGLRenderContext
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        20/5/2008
// ---------------------------------------------------------------------------
oaOpenGLRenderContext::~oaOpenGLRenderContext()
{
    oaDeviceContextHandle hDC = _myDeviceContext.nativeDeviceContextHandle();

    if (hDC != NULL)
    {
        // Make sure we have the GLX function pointers:
        bool rcProc = oaLoadGLXFunctionPointers();
        GT_IF_WITH_ASSERT(rcProc)
        {
            bool rc = (pOAglXMakeCurrent(hDC, None, NULL) == Success);
            GT_IF_WITH_ASSERT(rc && (_hOpenGLRC != NULL))
            {
                pOAglXDestroyContext(hDC, _hOpenGLRC);
                _hOpenGLRC = NULL;
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::makeCurrent
// Description: Makes this OpenGL rendering context the calling thread's current rendering
//              context. (All OpenGL calls made by this thread will be performed on
//              this render context).
// Author:      AMD Developer Tools Team
// Date:        20/5/2008
// ---------------------------------------------------------------------------
bool oaOpenGLRenderContext::makeCurrent()
{
    bool retVal = false;

    // Get the native Device context handle:
    oaDeviceContextHandle hDC = _myDeviceContext.nativeDeviceContextHandle();
    oaWindowHandle hWnd = _myDeviceContext.nativeWindowHandle();
    GT_IF_WITH_ASSERT(hDC != NULL)
    {
        // Make sure we have the GLX function pointers:
        bool rcProc = oaLoadGLXFunctionPointers();
        GT_IF_WITH_ASSERT(rcProc)
        {
            // Make this render context the "current context" of the calling thread:
            Bool rc = pOAglXMakeCurrent(hDC, hWnd, _hOpenGLRC);
            GT_IF_WITH_ASSERT(rc != False)
            {
                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::doneCurrent
// Description: Signal that the context should no longer be current
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        13/9/2012
// ---------------------------------------------------------------------------
bool oaOpenGLRenderContext::doneCurrent()
{
    bool retVal = false;

    // Get the native Device context handle:
    oaDeviceContextHandle hDC = _myDeviceContext.nativeDeviceContextHandle();
    GT_IF_WITH_ASSERT(hDC != NULL)
    {
        // Make sure we have the GLX function pointers:
        bool rcProc = oaLoadGLXFunctionPointers();
        GT_IF_WITH_ASSERT(rcProc)
        {
            // Make this render context the "current context" of the calling thread:
            Bool rc = pOAglXMakeCurrent(hDC, None, NULL);
            GT_IF_WITH_ASSERT(rc != False)
            {
                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::platformSpecificExtensionsString
// Description: Gets the supported extensions string by using glXQueryExtensionsString
// Arguments: extensions - the list goes here
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        23/3/2009
// ---------------------------------------------------------------------------
bool oaOpenGLRenderContext::platformSpecificExtensionsString(gtString& extensions)
{
    bool retVal = false;

    if (_platformSpecificExtensionsString.isEmpty())
    {
        // Make sure we have the GLX function pointers:
        bool rcProc = oaLoadGLXFunctionPointers();
        GT_IF_WITH_ASSERT(rcProc)
        {
            // Query GLX extensions list:
            // -------------------------
            oaDeviceContextHandle dpy = _myDeviceContext.nativeDeviceContextHandle();
            _platformSpecificExtensionsString.fromASCIIString(pOAglXQueryExtensionsString(dpy, DefaultScreen(dpy)));
            retVal = (!_platformSpecificExtensionsString.isEmpty());
        }
    }
    else
    {
        retVal = true;
    }

    if (retVal)
    {
        extensions = _platformSpecificExtensionsString;
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
// Author:      AMD Developer Tools Team
// Date:        20/5/2008
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
                bool rc = pDisplay->createOpenGLRenderContext();
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
// Description: Wrapper for glXGetProcAddress.
// Author:      AMD Developer Tools Team
// Date:        22/2/2010
// ---------------------------------------------------------------------------
void* oaOpenGLRenderContext::getOpenGLProcAddress(const char* pFuncName)
{
    void* retVal = NULL;

    // Make sure we have the GLX function pointers:
    bool rcProc = oaLoadGLXFunctionPointers();
    GT_IF_WITH_ASSERT(rcProc)
    {
        retVal = (void*)pOAglXGetProcAddress((const GLubyte*)pFuncName);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::getGPUInformation
// Description: GPU affinity is currently not supported on Linux
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        23/3/2009
// ---------------------------------------------------------------------------
bool oaOpenGLRenderContext::getGPUInformation(unsigned long gpuIndex, gtString& gpuName, intptr_t& hGPU)
{
    (void)(gpuIndex); // unused
    (void)(gpuName); // unused
    (void)(hGPU); // unused
    GT_ASSERT(false);
    return false;
}


