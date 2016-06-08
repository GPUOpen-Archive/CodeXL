//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaHiddenWindow.h
///
//=====================================================================

//------------------------------ oaHiddenWindow.h ------------------------------

#ifndef __OAWINDOW
#define __OAWINDOW

// Predeclerations:
class oaDeviceContext;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIWrappersDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           oaHiddenWindow
// General Description:
//   Represents a native OS Window
//   - Win32 - A Window (HWND)
//   - UNIX - A Window (Window)
//
// Author:      AMD Developer Tools Team
// Creation Date:        6/12/2003
// ----------------------------------------------------------------------------------
class OA_API oaHiddenWindow
{
public:
    oaHiddenWindow();
    virtual ~oaHiddenWindow();

    bool create(const gtString& title, int originX, int originY, int width, int height);
    bool destroy();

    bool showWindow();
    bool hideWindow();

    const oaDeviceContext* deviceContext() const { return _pDeviceContext; };
    oaDeviceContext* deviceContext() { return _pDeviceContext; };

    oaWindowHandle nativeWindowHandle() const { return _hWindow; };

    bool setActivePixelFormat(oaPixelFormatId pixelFormatIndex);
    bool getActivePixelFormat(oaPixelFormatId& pixelFormatIndex) const;

    oaPixelFormatHandle getPixelFormatHandle() const {return _hPixelFormat;};

private:
    // The native window class name:
    gtString _windowClassName;

    // The native OS window handle:
    oaWindowHandle _hWindow;

    // The window graphic device context:
    oaDeviceContext* _pDeviceContext;

    // The pixel format handle used to create this window:
    oaPixelFormatHandle _hPixelFormat;
};


#endif  // __OAWINDOW
