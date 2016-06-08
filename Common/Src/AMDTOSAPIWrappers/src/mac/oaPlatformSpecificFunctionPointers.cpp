//------------------------------ oaPlatformSpecificFunctionPointers.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osModule.h>

// Local:
#include <mac/oaPlatformSpecificFunctionPointers.h>
#include <common/oaOpenGLFunctionPointers.h>

// If we are NOT on the iPhone:
// TO_DO iPhone - implement this for the iPhone platforms.
#ifndef _GR_IPHONE_BUILD

// CGL function pointers:
PFNCGLDESCRIBEPIXELFORMAT pOACGLDescribePixelFormat = NULL;
PFNCGLCHOOSEPIXELFORMAT pOACGLChoosePixelFormat = NULL;
PFNCGLDESTROYPIXELFORMAT pOACGLDestroyPixelFormat = NULL;
PFNCGLCREATECONTEXT pOACGLCreateContext = NULL;
PFNCGLDESTROYCONTEXT pOACGLDestroyContext = NULL;
PFNCGLSETCURRENTCONTEXT pOACGLSetCurrentContext = NULL;
PFNCGLSETOFFSCREEN pOACGLSetOffScreen = NULL;
PFNCGLGETOFFSCREEN pOACGLGetOffScreen = NULL;
PFNCGLCLEARDRAWABLE pOACGLClearDrawable = NULL;


// ---------------------------------------------------------------------------
// Name:        oaLoadCGLFunctionPointers
// Description: Loads all GLX functions needed by the GROSWrappers module.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/4/2010
// ---------------------------------------------------------------------------
bool oaLoadCGLFunctionPointers()
{
    // This function could fail if any function was not yet acquired:
    bool retVal = !((pOACGLDescribePixelFormat == NULL) || (pOACGLChoosePixelFormat == NULL) || (pOACGLDestroyPixelFormat == NULL) ||
                    (pOACGLCreateContext == NULL) || (pOACGLDestroyContext == NULL) || (pOACGLSetCurrentContext == NULL) ||
                    (pOACGLSetOffScreen == NULL) || (pOACGLGetOffScreen == NULL) && (pOACGLClearDrawable == NULL));

    // Make sure we have all function pointers:
    if (!retVal)
    {
        // We are supposed to have all functions, or none at all:
        GT_ASSERT((pOACGLDescribePixelFormat == NULL) && (pOACGLChoosePixelFormat == NULL) && (pOACGLDestroyPixelFormat == NULL) &&
                  (pOACGLCreateContext == NULL) && (pOACGLDestroyContext == NULL) && (pOACGLSetCurrentContext == NULL) &&
                  (pOACGLSetOffScreen == NULL) && (pOACGLGetOffScreen == NULL) && (pOACGLClearDrawable == NULL));

        // Make sure the system OpenGL module is loaded:
        bool rcMod = oaLoadSystemOpenGLModule();
        GT_IF_WITH_ASSERT(rcMod)
        {
            // Get the OpenGL module handle:
            osModuleHandle systemOpenGLModuleHandle = oaSystemOpenGLModuleHandle();
            GT_IF_WITH_ASSERT(systemOpenGLModuleHandle != OS_NO_MODULE_HANDLE)
            {
                // Get the function pointers:
                osProcedureAddress pCGLDescribePixelFormat = NULL;
                bool rc1 = osGetProcedureAddress(systemOpenGLModuleHandle, "CGLDescribePixelFormat", pCGLDescribePixelFormat);
                osProcedureAddress pCGLChoosePixelFormat = NULL;
                bool rc2 = osGetProcedureAddress(systemOpenGLModuleHandle, "CGLChoosePixelFormat", pCGLChoosePixelFormat);
                osProcedureAddress pCGLDestroyPixelFormat = NULL;
                bool rc3 = osGetProcedureAddress(systemOpenGLModuleHandle, "CGLDestroyPixelFormat", pCGLDestroyPixelFormat);
                osProcedureAddress pCGLCreateContext = NULL;
                bool rc4 = osGetProcedureAddress(systemOpenGLModuleHandle, "CGLCreateContext", pCGLCreateContext);
                osProcedureAddress pCGLDestroyContext = NULL;
                bool rc5 = osGetProcedureAddress(systemOpenGLModuleHandle, "CGLDestroyContext", pCGLDestroyContext);
                osProcedureAddress pCGLSetCurrentContext = NULL;
                bool rc6 = osGetProcedureAddress(systemOpenGLModuleHandle, "CGLSetCurrentContext", pCGLSetCurrentContext);
                osProcedureAddress pCGLSetOffScreen = NULL;
                bool rc7 = osGetProcedureAddress(systemOpenGLModuleHandle, "CGLSetOffScreen", pCGLSetOffScreen);
                osProcedureAddress pCGLGetOffScreen = NULL;
                bool rc8 = osGetProcedureAddress(systemOpenGLModuleHandle, "CGLGetOffScreen", pCGLGetOffScreen);
                osProcedureAddress pCGLClearDrawable = NULL;
                bool rc9 = osGetProcedureAddress(systemOpenGLModuleHandle, "CGLClearDrawable", pCGLClearDrawable);

                GT_IF_WITH_ASSERT(rc1 && (pCGLDescribePixelFormat != NULL) && rc2 && (pCGLChoosePixelFormat != NULL) &&
                                  rc3 && (pCGLDestroyPixelFormat != NULL) && rc4 && (pCGLCreateContext  != NULL) &&
                                  rc5 && (pCGLDestroyContext  != NULL) && rc6 && (pCGLSetCurrentContext != NULL) &&
                                  rc7 && (pCGLSetOffScreen != NULL) && rc8 && (pCGLGetOffScreen != NULL) &&
                                  rc9 && (pCGLClearDrawable != NULL))
                {
                    // Copy the pointers to the variables:
                    pOACGLDescribePixelFormat = (PFNCGLDESCRIBEPIXELFORMAT)pCGLDescribePixelFormat;
                    pOACGLChoosePixelFormat = (PFNCGLCHOOSEPIXELFORMAT)pCGLChoosePixelFormat;
                    pOACGLDestroyPixelFormat = (PFNCGLDESTROYPIXELFORMAT)pCGLDestroyPixelFormat;
                    pOACGLCreateContext = (PFNCGLCREATECONTEXT)pCGLCreateContext;
                    pOACGLDestroyContext = (PFNCGLDESTROYCONTEXT)pCGLDestroyContext;
                    pOACGLSetCurrentContext = (PFNCGLSETCURRENTCONTEXT)pCGLSetCurrentContext;
                    pOACGLSetOffScreen = (PFNCGLSETOFFSCREEN)pCGLSetOffScreen;
                    pOACGLGetOffScreen = (PFNCGLGETOFFSCREEN)pCGLGetOffScreen;
                    pOACGLClearDrawable = (PFNCGLCLEARDRAWABLE)pCGLClearDrawable;

                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

#endif // _GR_IPHONE_BUILD



