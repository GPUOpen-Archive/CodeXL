//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaDriver.h
///
//=====================================================================

//------------------------------ oaDriver.h ------------------------------

#ifndef __OADRIVER
#define __OADRIVER

// Forward declarations:
class osFilePath;

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaOSAPIWrappersDLLBuild.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
enum oaDriverError
{
    OA_DRIVER_UNKNOWN = -1,
    OA_DRIVER_OK = 0,
    OA_DRIVER_NOT_FOUND,
    OA_DRIVER_VERSION_EMPTY,
};

OA_API bool oaGetCalVersion(gtString& calVersion);
OA_API gtString oaGetDriverVersion(int&  driverError);
OA_API oaDriverError oaGetDriverVersionfromADLModule(osModuleHandle adlModule, gtString& driverVersion);
OA_API bool oaIsHSADriver();
OA_API bool oaGetHSADeviceIds(gtVector<gtUInt32>& deviceIDs);

// Module constructor and destructor functions markers:
// (Currently supported by Linux and Mac only)
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define OA_MODULE_CONSTRUCTOR __attribute__((constructor))
    #define OA_MODULE_DESTRUCTOR __attribute__((destructor))
#endif

#endif  // __OADRIVER
