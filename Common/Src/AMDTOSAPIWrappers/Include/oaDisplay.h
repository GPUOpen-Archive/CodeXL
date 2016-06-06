//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaDisplay.h
///
//=====================================================================

//------------------------------ oaDisplay.h ------------------------------

#ifndef __OADISPLAY
#define __OADISPLAY

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIWrappersDLLBuild.h>

//
// Functions for getting details of the display connected to the local machine
//
OA_API int oaGetLocalMachineAmountOfMonitors();
OA_API bool oaGetDisplayMonitorInfo(gtString& deviceName, gtString& monitorName);


#endif // __OADISPLAY
