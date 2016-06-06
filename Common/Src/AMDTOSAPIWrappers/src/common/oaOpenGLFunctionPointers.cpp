//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaOpenGLFunctionPointers.cpp
///
//=====================================================================

//------------------------------ oaOpenGLFunctionPointers.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <common/oaOpenGLFunctionPointers.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osModule.h>

// The System's OpenGL module:
static osModuleHandle stat_systemOpenGLModuleHandle = NULL;

// OpenGL function pointers:
PFNGLGETSTRING pOAglGetString = NULL;


// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::loadSystemOpenGLModule
// Description: Loads the system's OpenGL module and stores its handle in stat_systemOpenGLModuleHandle.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        8/4/2010
// ---------------------------------------------------------------------------
bool oaLoadSystemOpenGLModule()
{
    bool retVal = (stat_systemOpenGLModuleHandle != OS_NO_MODULE_HANDLE);

    // If the module was not yet loaded:
    if (!retVal)
    {
        // Get the system's OpenGL module path:
        gtVector<osFilePath> systemOGLModulePath;
        osGetSystemOpenGLModulePath(systemOGLModulePath);

        int numberOfGLPaths = (int)systemOGLModulePath.size();

        gtString errMsg;

        for (int i = 0; (i < numberOfGLPaths) && (!retVal); i++)
        {
            // Translate the OpenGL module path to string:
            gtString systemOGLModulePathAsStr = systemOGLModulePath[i].asString();

            // Load the system OpenGL module:
            retVal = osLoadModule(systemOGLModulePathAsStr, stat_systemOpenGLModuleHandle, &errMsg, false);

            if (!retVal)
            {
                OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaUnLoadSystemOpenGLModule
// Description: Loads the system's OpenGL module and stores its handle in stat_systemOpenGLModuleHandle.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        8/4/2010
// ---------------------------------------------------------------------------
bool oaUnLoadSystemOpenGLModule()
{
    bool retVal = false;

    // If the system's OpenGL module was not loaded by this file:
    if (stat_systemOpenGLModuleHandle == OS_NO_MODULE_HANDLE)
    {
        retVal = true;
    }
    else
    {
        // Unload the system OpenGL module:
        retVal = osReleaseModule(stat_systemOpenGLModuleHandle);
        stat_systemOpenGLModuleHandle = NULL;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaSystemOpenGLModuleHandle
// Description:
//  Returns the system's OpenGL Module handle.
//  This handle is loaded by oaLoadSystemOpenGLModule and released by oaUnLoadSystemOpenGLModule.
// Author:      AMD Developer Tools Team
// Date:        8/4/2010
// ---------------------------------------------------------------------------
osModuleHandle oaSystemOpenGLModuleHandle()
{
    return stat_systemOpenGLModuleHandle;
}


// ---------------------------------------------------------------------------
// Name:        oaLoadOpenGLFunctionPointers
// Description: Loads all OpenGL functions needed by the GROSWrappers module.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        8/4/2010
// ---------------------------------------------------------------------------
bool oaLoadOpenGLFunctionPointers()
{
    // This function could fail if any function was not yet acquired:
    bool retVal = (pOAglGetString != NULL);

    // Make sure we have all function pointers:
    if (!retVal)
    {
        // Make sure the system OpenGL module is loaded:
        bool rcMod = oaLoadSystemOpenGLModule();
        GT_IF_WITH_ASSERT(rcMod)
        {
            // Get the function pointers:
            osProcedureAddress pglGetString = NULL;
            bool rc1 = osGetProcedureAddress(stat_systemOpenGLModuleHandle, "glGetString", pglGetString);
            GT_IF_WITH_ASSERT(rc1 && (pglGetString != NULL))
            {
                // Copy the pointers to the variables:
                pOAglGetString = (PFNGLGETSTRING)pglGetString;

                retVal = true;
            }
        }
    }

    return retVal;
}

