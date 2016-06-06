//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaDeviceContext.h
///
//=====================================================================

//------------------------------ oaDeviceContext.h ------------------------------

#ifndef __OADEVICECONTEXT_H
#define __OADEVICECONTEXT_H

// Pre-decelerations:
class oaPixelFormat;
class oaOpenGLRenderContext;
class oaHiddenWindow;

// Infra:
#include <AMDTBaseTools/Include/gtAutoPtr.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIWrappersDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           oaDeviceContext
// General Description:
//   Describes an open context to a Graphic device.
//   The context holds an instance for each of the Graphic device attributes variables.
//   Examples
//   - The device view port.
//   - The available pixel formats.
//   - Its background color.
//   - etc
//
//   Serve as a wrapper class for:
//   - Win32 - Device context (HDC).
//   - UNIX - Display
//      * Note: on Mac this class is absolutely meaningless, as we use it to relate
//        between a Render Context and a Pixel Format, but in Mac the Device Context
//        to Pixel Format relation is many-to-many and not one-to-many (ie a PF can
//        support multiple DCs and a DC can be supported by multiple PFs), so we
//        keep it for compatibility reasons only.
//
// Author:      AMD Developer Tools Team
// Creation Date:        6/12/2003
// ----------------------------------------------------------------------------------
class OA_API oaDeviceContext
{
public:
    oaDeviceContext(oaWindowHandle hWnd, oaHiddenWindow* pRelatedWindow = NULL);
    oaDeviceContext(oaWindowHandle hWnd, oaDeviceContextHandle hDC, oaHiddenWindow* pRelatedWindow = NULL);
    virtual ~oaDeviceContext();

    int amountOfAvailablePixelFormats() const;
    bool getPixelFormat(oaPixelFormatId pixelFormatID, gtAutoPtr<oaPixelFormat>& aptrPixelFormat) const;
    oaPixelFormatId getOpenGLPixelFormatId() const;
    oaPixelFormatId getHardwareAcceleratedOpenGLPixelFormatId() const;
    void setWindowHandle(oaWindowHandle hWnd) {_hWnd = hWnd;};

    bool createOpenGLRenderContext();
    oaOpenGLRenderContext* openGLRenderContext() { return _pOGLRenderContext; };

    oaDeviceContextHandle nativeDeviceContextHandle() const { return _hDC; };
    oaWindowHandle nativeWindowHandle() const { return _hWnd; };
    oaHiddenWindow* getRelatedWindow() const {return _pRelatedWindow; };
    oaPixelFormatId getPixelFormatIDFromIndex(int pixelFormatIndex) const;

private:
    // Handle to the native window that contains this device context:
    oaWindowHandle _hWnd;

    // Handle to the native device context that this class wrappers:
    oaDeviceContextHandle _hDC;

    // Contains true iff this class is the OS device context owner:
    bool _isOSDCOwner;

    // An OpenGL render context that renders into this device context:
    oaOpenGLRenderContext* _pOGLRenderContext;

    // The window related to this Device Context. (in Windows it is the parent, in UNIX it is
    // dependant upon this context)
    oaHiddenWindow* _pRelatedWindow;
};

#endif //__OADEVICECONTEXT_H
