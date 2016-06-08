//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaSingeltonsDelete.cpp
///
//=====================================================================

//------------------------------ oaSingeltonsDelete.cpp ------------------------------

// Local:
#include <common/oaSingeltonsDelete.h>
#include <AMDTOSAPIWrappers/Include/oaHiddenWindow.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLRenderContext.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // ATI perf counters currently only used on Windows:
    #include <AMDTOSAPIWrappers/Include/oaATIFunctionWrapper.h>
#endif

// A static instance of the singleton deleter class. Its destructor will delete all
// the singletons instances.
static oaSingeltonsDelete singeltonDeleter;


// ---------------------------------------------------------------------------
// Name:        oaSingeltonsDelete::~oaSingeltonsDelete
// Description: Destructor - deletes all the singleton instances.
// Author:      AMD Developer Tools Team
// Date:        24/4/2004
// ---------------------------------------------------------------------------
oaSingeltonsDelete::~oaSingeltonsDelete()
{
    // Delete oaOpenGLRenderContext static members:
    // (deleting the render context destroys both its window and device context)
    delete oaOpenGLRenderContext::_pDefaultRenderContextWindow;
    oaOpenGLRenderContext::_pDefaultRenderContextWindow = NULL;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

#ifdef OA_DEBUGGER_USE_AMD_GPA
    // ATI perf counters currently only used on Windows:
    // Delete the ATI function wrapper:
    delete oaATIFunctionWrapper::_pMySingleGLInstance;
    oaATIFunctionWrapper::_pMySingleGLInstance = NULL;

    delete oaATIFunctionWrapper::_pMySingleCLInstance;
    oaATIFunctionWrapper::_pMySingleCLInstance = NULL;

#endif // OA_DEBUGGER_USE_AMD_GPA

#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
}



