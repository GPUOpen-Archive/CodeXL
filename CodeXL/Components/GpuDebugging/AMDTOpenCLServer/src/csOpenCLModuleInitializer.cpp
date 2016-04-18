//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csOpenCLModuleInitializer.cpp
///
//==================================================================================

//------------------------------ csOpenCLModuleTerminator.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osModule.h>

// Local:
#include <src/csOpenCLServerInitialization.h>

#if AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
// ---------------------------------------------------------------------------
// Name:        csInitializeOpenCLModule
// Description: Initializes the OpenGL module (shared library).
//              This function is called when the OpenGL module is loaded by
//              the OS dynamic loader during ::dlopen() operation (or LD_PRELOAD).
// Author:      Uri Shomroni
// Date:        14/12/2010
// ---------------------------------------------------------------------------
static void OS_MODULE_CONSTRUCTOR csInitializeOpenCLModule()
{
    // Perform initialization operations
    csInitializeOpenCLServer();
}
#endif

// ---------------------------------------------------------------------------
// Name:        csTerminateOpenCLModule
// Description: Terminates the OpenCL module (shared library).
//              This function is called when the OpenCL module is unloaded by
//              the OS dynamic loader during ::dlclose() operation.
// Author:      Uri Shomroni
// Date:        29/11/2009
// ---------------------------------------------------------------------------
static void OS_MODULE_DESTRUCTOR csTerminateOpenCLModule()
{
    // Report, to my debugger, that the debugged process is about to terminate:
    // TO_DO: We might need to move this to Spies Utilities:
    // csReportDebuggedProcessTermination(true);

    // Terminate the spy:
    (void) csTerminateOpenCLServer();
}
