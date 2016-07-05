//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsWrappersCommon.h
///
//==================================================================================

//------------------------------ gsWrappersCommon.h ------------------------------

#ifndef __GSWRAPPERSCOMMON
#define __GSWRAPPERSCOMMON

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTServerUtilities/Include/suInterceptionMacros.h>

// Local:
#include <src/gsGlobalVariables.h>

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    #include <AMDTServerUtilities/Include/suMacOSXInterception.h>
#endif

// --------------------------------------------------------
//             Public functions
// --------------------------------------------------------

bool gsInitializeWrapperFunctions();
bool gsTerminateWrapperFunctions();
osProcedureAddress gsGetSystemsOGLModuleProcAddress(const char* procname);
const char* gsTextureCoordinateString(GLenum coord);
void gsEnableInitializationFunctionsLogging(bool isLoggingEnabled);
bool gsAreInitializationFunctionsLogged();
void gsUpdateTLSVariableValues();


#endif  // __GSWRAPPERSCOMMON
