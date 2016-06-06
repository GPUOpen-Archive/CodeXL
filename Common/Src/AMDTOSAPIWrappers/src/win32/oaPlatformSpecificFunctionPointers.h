//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaPlatformSpecificFunctionPointers.h
///
//=====================================================================

//------------------------------ oaPlatformSpecificFunctionPointers.h ------------------------------

#ifndef __OAPLATFORMSPECIFICFUNCTIONPOINTERS_WIN_H
#define __OAPLATFORMSPECIFICFUNCTIONPOINTERS_WIN_H

// Local:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// WGL function types:
typedef WINGDIAPI HGLRC(WINAPI* PFNWGLCREATECONTEXTPROC)(HDC dc);
typedef WINGDIAPI BOOL (WINAPI* PFNWGLDELETECONTEXTPROC)(HGLRC ctx);
typedef WINGDIAPI BOOL (WINAPI* PFNWGLMAKECURRENTPROC)(HDC dc, HGLRC ctx);
typedef WINGDIAPI PROC(WINAPI* PFNWGLGETPROCADDRESSPROC)(LPCSTR a);

// WGL function pointers:
extern PFNWGLCREATECONTEXTPROC pOAwglCreateContext;
extern PFNWGLDELETECONTEXTPROC pOAwglDeleteContext;
extern PFNWGLMAKECURRENTPROC pOAwglMakeCurrent;
extern PFNWGLGETPROCADDRESSPROC pOAwglGetProcAddress;

// Aid functions:
bool oaLoadWGLFunctionPointers();


#endif //__OAPLATFORMSPECIFICFUNCTIONPOINTERS_WIN_H

