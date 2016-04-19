//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csWrappersCommon.h
///
//==================================================================================

//------------------------------ csWrappersCommon.h ------------------------------

#ifndef __CSWRAPPERSCOMMON_H
#define __CSWRAPPERSCOMMON_H

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>


// --------------------------------------------------------
//             Public functions
// --------------------------------------------------------

bool csInitializeWrapperFunctions();
bool csTerminateWrapperFunctions();
osProcedureAddress csGetSystemsOCLModuleProcAddress(const char* procname);
osProcedureAddress csGetSystemsOCLModuleProcAddress(oaCLPlatformID platformId, const char* procname);



#endif //__CSWRAPPERSCOMMON_H

