//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaOpenGLFunctionPointers.h
///
//=====================================================================

//------------------------------ oaOpenGLFunctionPointers.h ------------------------------

#ifndef __OAOPENGLFUNCTIONPOINTERS_H
#define __OAOPENGLFUNCTIONPOINTERS_H

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// OpenGL function types:
typedef const GLubyte* (APIENTRY* PFNGLGETSTRING)(GLenum name);

// OpenGL function pointers:
extern PFNGLGETSTRING pOAglGetString;

// Aid functions:
bool oaLoadSystemOpenGLModule();
bool oaUnLoadSystemOpenGLModule();
osModuleHandle oaSystemOpenGLModuleHandle();
bool oaLoadOpenGLFunctionPointers();

#endif //__OAOPENGLFUNCTIONPOINTERS_H

