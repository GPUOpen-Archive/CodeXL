//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suSpiesUtilitiesModuleTerminator.cpp
///
//==================================================================================

//------------------------------ suSpiesUtilitiesModuleTerminator.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osModule.h>

// Local:
#include <src/suSpiesUtilitiesDLLInitializationFunctions.h>


// ---------------------------------------------------------------------------
// Name:        suSpiesUtilitiesModuleTerminator
// Description: Terminates the Spies Utilities module (shared library).
//              This function is called when the Spies Utilities module is unloaded
//              by the OS dynamic loader during ::dlclose() operation.
// Author:      Uri Shomroni
// Date:        29/11/2009
// ---------------------------------------------------------------------------
void OS_MODULE_DESTRUCTOR suSpiesUtilitiesModuleTerminator()
{
    // Report, to my debugger, that the debugged process is about to terminate:
    suReportDebuggedProcessTermination();

    // Terminate the spy:
    bool rc1 = suTerminateSpiesUtilitiesModule();

    GT_ASSERT(rc1);
}
