//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaOpenGLRenderContext.cpp
///
//=====================================================================

//------------------------------ oaOpenGLRenderContext.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osModule.h>

// Local:
#include <win32/oaPlatformSpecificFunctionPointers.h>
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
// Date:        27/12/2003
// ---------------------------------------------------------------------------
oaOpenGLRenderContext::oaOpenGLRenderContext(const oaDeviceContext& deviceContext)
    : _myDeviceContext(deviceContext), _hOpenGLRC(NULL), _vendorType(OA_VENDOR_UNKNOWN), _vendorString()
{
    // Make sure we have the WGL function pointers:
    bool rcProc = oaLoadWGLFunctionPointers();

    // If we got the context:
    GT_IF_WITH_ASSERT(rcProc && (pOAwglCreateContext != NULL))
    {
        // Get the native Device context handle:
        oaDeviceContextHandle _hDC = deviceContext.nativeDeviceContextHandle();

        // Create a render context:
        _hOpenGLRC = pOAwglCreateContext(_hDC);

        DWORD err = GetLastError();

        // Sanity test:
        if (err != 0)
        {
            GT_ASSERT(_hOpenGLRC != NULL);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::~oaOpenGLRenderContext
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        27/12/2003
// ---------------------------------------------------------------------------
oaOpenGLRenderContext::~oaOpenGLRenderContext()
{
    if (_hOpenGLRC)
    {
        // Yaki - 8/5/2007 - TO_DO:
        // On Vista, the ATI driver has a crash when deleting the render context.
        // Until they fix the crash, the below code should remain commented out.

        // BOOL rc = pOAwglDeleteContext(_hOpenGLRC);
        // Delete the win32 render context:
        // GT_ASSERT(rc == TRUE);

        _hOpenGLRC = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::makeCurrent
// Description: Makes this OpenGL rendering context the calling thread's current rendering
//              context. (All OpenGL calls made by this thread will be performed on
//              this render context).
// Author:      AMD Developer Tools Team
// Date:        27/12/2003
// ---------------------------------------------------------------------------
bool oaOpenGLRenderContext::makeCurrent()
{
    bool retVal = false;

    // Make sure we have the WGL function pointers:
    bool rcProc = oaLoadWGLFunctionPointers();

    // If we got the context:
    GT_IF_WITH_ASSERT(rcProc && (pOAwglMakeCurrent != NULL))
    {
        // Get the native Device context handle:
        oaDeviceContextHandle _hDC = _myDeviceContext.nativeDeviceContextHandle();
        GT_IF_WITH_ASSERT(_hDC != NULL)
        {
            // Make this render context the "current context" of the calling thread:
            BOOL rc = pOAwglMakeCurrent(_hDC, _hOpenGLRC);
            GT_IF_WITH_ASSERT(rc != FALSE)
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

    // Make sure we have the WGL function pointers:
    bool rcProc = oaLoadWGLFunctionPointers();

    // If we got the context:
    GT_IF_WITH_ASSERT(rcProc && (pOAwglMakeCurrent != NULL))
    {
        // Make this render context the "current context" of the calling thread:
        BOOL rc = pOAwglMakeCurrent(NULL, NULL);
        GT_IF_WITH_ASSERT(rc != FALSE)
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::platformSpecificExtensionsString
// Description: Gets the supported extensions string by using wglGetExtensionsStringARB
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
        // Make sure we have the WGL function pointers:
        bool rcProc = oaLoadWGLFunctionPointers();

        // If we got the context:
        GT_IF_WITH_ASSERT(rcProc && (pOAwglGetProcAddress != NULL))
        {
            PFNWGLGETEXTENSIONSSTRINGARBPROC pFunc = (PFNWGLGETEXTENSIONSSTRINGARBPROC)pOAwglGetProcAddress("wglGetExtensionsStringARB");

            if (pFunc != NULL)
            {
                // Query WGL extensions list:
                // ----------------------------
                const char* pExtensionsString = pFunc(_myDeviceContext.nativeDeviceContextHandle());
                _platformSpecificExtensionsString.fromASCIIString(pExtensionsString);
                retVal = (!_platformSpecificExtensionsString.isEmpty());
            }
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
// Date:        13/7/2006
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
            oaDeviceContext* pWinDeviceContext = _pDefaultRenderContextWindow->deviceContext();
            GT_IF_WITH_ASSERT(pWinDeviceContext != NULL)
            {
                // Load the WGL functions, to properly initialize WGL on the Windows GDI renderer:
                bool rcWGL = oaLoadWGLFunctionPointers();
                GT_ASSERT(rcWGL);

                // Get the id of an OpenGL hardware accelerated pixel format:
                oaPixelFormatId pixelFormatId = pWinDeviceContext->getOpenGLPixelFormatId();
                GT_IF_WITH_ASSERT(pixelFormatId != OA_NO_PIXEL_FORMAT_ID)
                {
                    // Set this pixel format as the active pixel format:
                    bool rc1 = _pDefaultRenderContextWindow->setActivePixelFormat(pixelFormatId);
                    GT_IF_WITH_ASSERT(rc1)
                    {
                        // Create an OpenGL render context:
                        bool rc2 = pWinDeviceContext->createOpenGLRenderContext();
                        GT_IF_WITH_ASSERT(rc2)
                        {
                            // Get the created render context:
                            _pDefaultRenderContext = pWinDeviceContext->openGLRenderContext();
                            GT_IF_WITH_ASSERT(_pDefaultRenderContext != NULL)
                            {
                                // Extract OpenGL vendor:
                                bool rc3 = _pDefaultRenderContext->extractVendorType();
                                GT_ASSERT(rc3);

                                retVal = _pDefaultRenderContext;
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::getOpenGLProcAddress
// Description: Wrapper for wglGetProcAddress.
// Author:      AMD Developer Tools Team
// Date:        22/2/2010
// ---------------------------------------------------------------------------
void* oaOpenGLRenderContext::getOpenGLProcAddress(const char* pFuncName)
{
    void* retVal = NULL;

    // Make sure we have the WGL function pointers:
    bool rcProc = oaLoadWGLFunctionPointers();

    // If we got the context:
    GT_IF_WITH_ASSERT(rcProc && (pOAwglGetProcAddress != NULL))
    {
        retVal = (void*)pOAwglGetProcAddress((LPCSTR)pFuncName);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::getGPUInformation
// Description: Gets the Name and handle of a GPU from its index:
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        23/3/2009
// ---------------------------------------------------------------------------
bool oaOpenGLRenderContext::getGPUInformation(unsigned long gpuIndex, gtString& gpuName, intptr_t& hGPU)
{
    bool retVal = false;

    // Make sure we have the WGL function pointers:
    bool rcProc = oaLoadWGLFunctionPointers();

    // If we got the context:
    GT_IF_WITH_ASSERT(rcProc && (pOAwglGetProcAddress != NULL))
    {
        // Get the pointers to wglEnumGpusNV and wglEnumGpuDevicesNV
        PFNWGLENUMGPUSNVPROC wglEnumGpusNV = (PFNWGLENUMGPUSNVPROC)pOAwglGetProcAddress("wglEnumGpusNV");
        PFNWGLENUMGPUDEVICESNVPROC wglEnumGpuDevicesNV = (PFNWGLENUMGPUDEVICESNVPROC)pOAwglGetProcAddress("wglEnumGpuDevicesNV");

        if ((wglEnumGpusNV != NULL) && (wglEnumGpuDevicesNV != NULL))
        {
            // Get the GPU handle (we expect this to fail if the index is too high)
            HGPUNV gpuHandle = NULL;
            BOOL resultCode = wglEnumGpusNV((UINT)gpuIndex, &gpuHandle);

            if (resultCode == TRUE)
            {
                // Get the description of the first device on the GPU (we will use its name as the GPU name)
                GPU_DEVICE deviceInfo;
                deviceInfo.cb = sizeof(GPU_DEVICE);
                resultCode = wglEnumGpuDevicesNV(gpuHandle, 0, &deviceInfo);

                if (resultCode == TRUE)
                {
                    // Set the return values:
                    gpuName.fromASCIIString(deviceInfo.DeviceString);
                    hGPU = (intptr_t)gpuHandle;
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}



