//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaPlatformSpecificFunctionPointers.cpp
///
//=====================================================================

//------------------------------ oaPlatformSpecificFunctionPointers.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osModule.h>

// Local:
#include <win32/oaPlatformSpecificFunctionPointers.h>
#include <common/oaOpenGLFunctionPointers.h>

// WGL function pointers:
PFNWGLCREATECONTEXTPROC pOAwglCreateContext = NULL;
PFNWGLDELETECONTEXTPROC pOAwglDeleteContext = NULL;
PFNWGLMAKECURRENTPROC pOAwglMakeCurrent = NULL;
PFNWGLGETPROCADDRESSPROC pOAwglGetProcAddress = NULL;


// ---------------------------------------------------------------------------
// Name:        oaLoadWGLFunctionPointers
// Description: Loads all GLX functions needed by the GROSWrappers module.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        8/4/2010
// ---------------------------------------------------------------------------
bool oaLoadWGLFunctionPointers()
{
    // This function could fail if any function was not yet acquired:
    bool retVal = !((pOAwglCreateContext == NULL) || (pOAwglDeleteContext == NULL) || (pOAwglMakeCurrent == NULL) || (pOAwglGetProcAddress == NULL));

    // Make sure we have all function pointers:
    if (!retVal)
    {
        // We are supposed to have all functions, or none at all:
        GT_ASSERT((pOAwglCreateContext == NULL) && (pOAwglDeleteContext == NULL) && (pOAwglMakeCurrent == NULL) && (pOAwglGetProcAddress == NULL));

        // Make sure the system OpenGL module is loaded:
        bool rcMod = oaLoadSystemOpenGLModule();
        GT_IF_WITH_ASSERT(rcMod)
        {
            osModuleHandle oglModuleHandle = oaSystemOpenGLModuleHandle();
            GT_IF_WITH_ASSERT(oglModuleHandle != OS_NO_MODULE_HANDLE)
            {
                // Get the function pointers:
                osProcedureAddress pwglCreateContext = NULL;
                bool rc1 = osGetProcedureAddress(oglModuleHandle, "wglCreateContext", pwglCreateContext);
                osProcedureAddress pwglDeleteContext = NULL;
                bool rc2 = osGetProcedureAddress(oglModuleHandle, "wglDeleteContext", pwglDeleteContext);
                osProcedureAddress pwglMakeCurrent = NULL;
                bool rc3 = osGetProcedureAddress(oglModuleHandle, "wglMakeCurrent", pwglMakeCurrent);
                osProcedureAddress pwglGetProcAddress = NULL;
                bool rc4 = osGetProcedureAddress(oglModuleHandle, "wglGetProcAddress", pwglGetProcAddress);
                GT_IF_WITH_ASSERT(rc1 && (pwglCreateContext != NULL) && rc2 && (pwglDeleteContext != NULL) && rc3 && (pwglMakeCurrent != NULL) && rc4 && (pwglGetProcAddress != NULL))
                {
                    // Copy the pointers to the variables:
                    pOAwglCreateContext = (PFNWGLCREATECONTEXTPROC)pwglCreateContext;
                    pOAwglDeleteContext = (PFNWGLDELETECONTEXTPROC)pwglDeleteContext;
                    pOAwglMakeCurrent = (PFNWGLMAKECURRENTPROC)pwglMakeCurrent;
                    pOAwglGetProcAddress = (PFNWGLGETPROCADDRESSPROC)pwglGetProcAddress;

                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

