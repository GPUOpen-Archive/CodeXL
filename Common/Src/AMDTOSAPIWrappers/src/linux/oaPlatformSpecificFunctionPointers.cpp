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

// Local:
#include <linux/oaPlatformSpecificFunctionPointers.h>
#include <common/oaOpenGLFunctionPointers.h>
#include <AMDTOSWrappers/Include/osModule.h>

// GLX function pointers:
PFNGLXCREATECONTEXT pOAglXCreateContext = NULL;
PFNGLXDESTROYCONTEXT pOAglXDestroyContext = NULL;
PFNGLXMAKECURRENT pOAglXMakeCurrent = NULL;
PFNGLXQUERYEXTENSIONSSTRING pOAglXQueryExtensionsString = NULL;
PFNGLXGETPROCADDRESS pOAglXGetProcAddress = NULL;
PFNGLXGETCONFIG pOAglXGetConfig = NULL;
PFNGLXCHOOSEVISUAL pOAglXChooseVisual = NULL;


// ---------------------------------------------------------------------------
// Name:        oaLoadGLXFunctionPointers
// Description: Loads all GLX functions needed by the GROSWrappers module.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        8/4/2010
// ---------------------------------------------------------------------------
bool oaLoadGLXFunctionPointers()
{
    // This function could fail if any function was not yet acquired:
    bool retVal = !((pOAglXCreateContext == NULL) || (pOAglXDestroyContext == NULL) || (pOAglXMakeCurrent == NULL) ||
                    (pOAglXQueryExtensionsString == NULL) || (pOAglXGetProcAddress == NULL) || (pOAglXGetConfig == NULL) ||
                    (pOAglXChooseVisual == NULL));

    // Make sure we have all function pointers:
    if (!retVal)
    {
        // We are supposed to have all functions, or none at all:
        GT_ASSERT((pOAglXCreateContext == NULL) && (pOAglXDestroyContext == NULL) && (pOAglXMakeCurrent == NULL) &&
                  (pOAglXQueryExtensionsString == NULL) && (pOAglXGetProcAddress == NULL) && (pOAglXGetConfig == NULL) &&
                  (pOAglXChooseVisual == NULL));

        // Make sure the system OpenGL module is loaded:
        bool rcMod = oaLoadSystemOpenGLModule();
        GT_IF_WITH_ASSERT(rcMod)
        {
            // Get the OpenGL module handle:
            osModuleHandle systemOpenGLModuleHandle = oaSystemOpenGLModuleHandle();
            GT_IF_WITH_ASSERT(systemOpenGLModuleHandle != OS_NO_MODULE_HANDLE)
            {
                // Get the function pointers:
                osProcedureAddress pglXCreateContext = NULL;
                bool rc1 = osGetProcedureAddress(systemOpenGLModuleHandle, "glXCreateContext", pglXCreateContext);
                osProcedureAddress pglXDestroyContext = NULL;
                bool rc2 = osGetProcedureAddress(systemOpenGLModuleHandle, "glXDestroyContext", pglXDestroyContext);
                osProcedureAddress pglXMakeCurrent = NULL;
                bool rc3 = osGetProcedureAddress(systemOpenGLModuleHandle, "glXMakeCurrent", pglXMakeCurrent);
                osProcedureAddress pglXQueryExtensionsString = NULL;
                bool rc4 = osGetProcedureAddress(systemOpenGLModuleHandle, "glXQueryExtensionsString", pglXQueryExtensionsString);
                osProcedureAddress pglXGetProcAddress = NULL;
                bool rc5 = osGetProcedureAddress(systemOpenGLModuleHandle, "glXGetProcAddress", pglXGetProcAddress);
                osProcedureAddress pglXGetConfig = NULL;
                bool rc6 = osGetProcedureAddress(systemOpenGLModuleHandle, "glXGetConfig", pglXGetConfig);
                osProcedureAddress pglXChooseVisual = NULL;
                bool rc7 = osGetProcedureAddress(systemOpenGLModuleHandle, "glXChooseVisual", pglXChooseVisual);
                GT_IF_WITH_ASSERT(rc1 && (pglXCreateContext != NULL) && rc2 && (pglXDestroyContext != NULL) &&
                                  rc3 && (pglXMakeCurrent != NULL) && rc4 && (pglXQueryExtensionsString  != NULL) &&
                                  rc5 && (pglXGetProcAddress  != NULL) && rc6 && (pglXGetConfig != NULL) &&
                                  rc7 && (pglXChooseVisual != NULL))
                {
                    // Copy the pointers to the variables:
                    pOAglXCreateContext = (PFNGLXCREATECONTEXT)pglXCreateContext;
                    pOAglXDestroyContext = (PFNGLXDESTROYCONTEXT)pglXDestroyContext;
                    pOAglXMakeCurrent = (PFNGLXMAKECURRENT)pglXMakeCurrent;
                    pOAglXQueryExtensionsString = (PFNGLXQUERYEXTENSIONSSTRING)pglXQueryExtensionsString;
                    pOAglXGetProcAddress = (PFNGLXGETPROCADDRESS)pglXGetProcAddress;
                    pOAglXGetConfig = (PFNGLXGETCONFIG)pglXGetConfig;
                    pOAglXChooseVisual = (PFNGLXCHOOSEVISUAL)pglXChooseVisual;

                    retVal = true;
                }
            }
        }
    }

    return retVal;
}



