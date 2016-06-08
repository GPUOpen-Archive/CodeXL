//=====================================================================
// Copyright 2004-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file egltypes.h
/// This file contains platform dependent definitions required by EGL
/// (OpenGL ES and OpenVG interface to the native platform window system)
///
/// It defines:
///  - EGL types and resources
///  - Native types
///  - EGL and native handle values
///
//=====================================================================

//------------------------------ egltypes.h ------------------------------

#ifndef __EGLTYPES_H
#define __EGLTYPES_H

/* Win32 */
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN 1
    #include <windows.h>
#endif // _WIN32

/* API export definition */
#define GLAPI_EXT

/* Types */
typedef int EGLBoolean;
#define EGL_FALSE 0
#define EGL_TRUE  1

/* An integer of at least 32 bits */
typedef int EGLint;

/* Resources */
typedef void* EGLDisplay;
typedef void* EGLConfig;
typedef void* EGLSurface;
typedef void* EGLContext;

#ifdef _WIN32
    /* Windowing system: */
    typedef HWND  NativeWindowType;
    typedef HDC   NativeDisplayType;
#else
    typedef EGLint NativeWindowType;
    typedef EGLint NativeDisplayType;

    #ifndef APIENTRY
        #define APIENTRY
    #endif

#endif // _WIN32
typedef void* NativePixmapType;

/* EGL and native handle values */
#define EGL_DEFAULT_DISPLAY ((NativeDisplayType)0)
#define EGL_NO_CONTEXT ((EGLContext)0)
#define EGL_NO_DISPLAY ((EGLDisplay)0)
#define EGL_NO_SURFACE ((EGLSurface)0)


#endif //__EGLTYPES_H
