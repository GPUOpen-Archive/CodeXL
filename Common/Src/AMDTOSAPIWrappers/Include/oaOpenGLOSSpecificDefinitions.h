//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaOpenGLOSSpecificDefinitions.h
///
//=====================================================================

//------------------------------ osOpenGLWin32Definitions.h ------------------------------

#ifndef __OAOPENGLWIN32DEFINITIONS_H
#define __OAOPENGLWIN32DEFINITIONS_H

/*
    This file contains "Per OS" OpenGL definitions.

    Functions standard declerations are as following:

    - WINGDIAPI void    APIENTRY    openGLFunc();
    - GLAPI     void    APIENTRY    openGLExtensionFunc();
    - extern    void    WINAPI      wglFunc();
    - GL_API    void    GL_APIENTRY openGLESFunc();
    - GLAPI     void    APIENTRY    eglFunc();

    Our wrapper functions uses the same calling convention as the appropriate
    standard function decelerations:
    -           void    APIENTRY    openGLFuncWrapper();
    -           void    APIENTRY    openGLExtensionFuncWrapper();
    -           void    WINAPI      wglFuncWrapper();
    -           void    WINAPI      wglExtensionFuncWrapper();
    -           void    GL_APIENTRY openGLESFuncWrapper();
    -           void    APIENTRY    eglFuncWrapper();

*/

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>


// If we are building on Windows 32 bit:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    // Add Win32 definitions:
    #define WIN32_LEAN_AND_MEAN 1
    #include <Windows.h>

    // Windows.h defines WINGDIAPI as nothing.
    // We need to define it as __declspec(dllexport) or __declspec(dllimport):
    #ifdef WINGDIAPI
        #undef WINGDIAPI
    #endif

    // If we are building a spy dll:
    // (we immitate a Win32 dll, so we declare _GDI32_ at the spy project file)
    #ifdef _GDI32_

        // OpenGL export deceleration:
        #define WINGDIAPI __declspec(dllexport)

    #else // Building a "non" spy dll: (_GDI32_ is not defined):

        // OpenGL import deceleration:
        #define WINGDIAPI __declspec(dllimport)

    #endif // _GDI32_


#endif  // AMDT_BUILD_TARGET


#endif //__OSOPENGLWIN32DEFINITIONS_H


