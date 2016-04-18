//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsOpenGLModuleInitializer.cpp
///
//==================================================================================

//------------------------------ gsOpenGLModuleTerminator.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osModule.h>

// Local:
#include <src/gsOpenGLSpyInitFuncs.h>

#if AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
// ---------------------------------------------------------------------------
// Name:        gsInitializeOpenGLModule
// Description: Initializes the OpenGL module (shared library).
//              This function is called when the OpenGL module is loaded by
//              the OS dynamic loader during ::dlopen() operation (or LD_PRELOAD).
// Author:      Uri Shomroni
// Date:        14/12/2010
// ---------------------------------------------------------------------------
static void OS_MODULE_CONSTRUCTOR gsInitializeOpenGLModule()
{
    // Perform initialization operations
    gsOpenGLSpyInit();
}
#endif

// ---------------------------------------------------------------------------
// Name:        gsTerminateOpenGLModule
// Description: Terminates the OpenGL module (shared library).
//              This function is called when the OpenGL module is unloaded by
//              the OS dynamic loader during ::dlclose() operation.
// Author:      Uri Shomroni
// Date:        29/11/2009
// ---------------------------------------------------------------------------
static void OS_MODULE_DESTRUCTOR gsTerminateOpenGLModule()
{
    // Report, to my debugger, that the debugged process is about to terminate:
    // TO_DO: We might need to move this to Spies Utilities:
    // gsReportDebuggedProcessTermination(true);

    // Terminate the OpenGL Server:
    (void) gsOpenGLSpyTerminate();
}
