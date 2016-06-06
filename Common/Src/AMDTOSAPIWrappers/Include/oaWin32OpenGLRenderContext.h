//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osWin32OpenGLRenderContext.h ------------------------------

#ifndef __OSWIN32OPENGLRENDERCONTEXT
#define __OSWIN32OPENGLRENDERCONTEXT

// Pre-declarations:
class osWin32GraphicDeviceContext;

// Win 32:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Local:
#include <GROSWrappers\osOpenGLRenderContext.h>


// ----------------------------------------------------------------------------------
// Class Name:           osWin32GraphicDeviceContext
// General Description:
//   Win32 implementation of an OpenGL render context.
//
// Author:      AMD Developer Tools Team
// Creation Date:        6/12/2003
// ----------------------------------------------------------------------------------
class OS_API osWin32OpenGLRenderContext : public osOpenGLRenderContext
{
public:
    osWin32OpenGLRenderContext(osWin32GraphicDeviceContext& deviceContext);
    virtual ~osWin32OpenGLRenderContext();

    // Overrides osGraphicDeviceContext:
    virtual bool makeCurrent();

private:
    // Do not allow the use of my default constructor:
    osWin32OpenGLRenderContext();

private:
    // Handle to the Win32 window that contains this device context:
    HWND _hWnd;

    // Handle to the Win32 device context:
    HDC _hDC;

    // Handle to the Win32 OpenGL render context:
    HGLRC _hglRC;
};


#endif  // __OSWIN32OPENGLRENDERCONTEXT
