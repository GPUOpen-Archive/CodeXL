//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaPlatformSpecificFunctionPointers.h
///
//=====================================================================

//------------------------------ oaPlatformSpecificFunctionPointers.h ------------------------------

#ifndef __OAPLATFORMSPECIFICFUNCTIONPOINTERS_H
#define __OAPLATFORMSPECIFICFUNCTIONPOINTERS_H

// Local:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// GLX function types:
typedef  GLXContext(*PFNGLXCREATECONTEXT)(Display* dpy, XVisualInfo* vis, GLXContext share_list, Bool direct);
typedef void (*PFNGLXDESTROYCONTEXT)(Display* dpy, GLXContext ctx);
typedef Bool(*PFNGLXMAKECURRENT)(Display* dpy, GLXDrawable drawable, GLXContext ctx);
typedef const char* (*PFNGLXQUERYEXTENSIONSSTRING)(Display* dpy, int screen);
typedef void (*(*PFNGLXGETPROCADDRESS)(const GLubyte* procname))(void);
typedef int (*PFNGLXGETCONFIG)(Display* dpy, XVisualInfo* vis, int attrib, int* value);
typedef XVisualInfo* (*PFNGLXCHOOSEVISUAL)(Display* dpy, int screen, int* attrib_list);

// GLX function pointers:
extern PFNGLXCREATECONTEXT pOAglXCreateContext;
extern PFNGLXDESTROYCONTEXT pOAglXDestroyContext;
extern PFNGLXMAKECURRENT pOAglXMakeCurrent;
extern PFNGLXQUERYEXTENSIONSSTRING pOAglXQueryExtensionsString;
extern PFNGLXGETPROCADDRESS pOAglXGetProcAddress;
extern PFNGLXGETCONFIG pOAglXGetConfig;
extern PFNGLXCHOOSEVISUAL pOAglXChooseVisual;

// Aid functions:
bool oaLoadGLXFunctionPointers();


#endif //__OAPLATFORMSPECIFICFUNCTIONPOINTERS_H

