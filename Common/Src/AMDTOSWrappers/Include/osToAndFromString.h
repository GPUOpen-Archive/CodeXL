//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osToAndFromString.h
///
//=====================================================================

//------------------------------ osToAndFromString.h ------------------------------

#ifndef __OSTOANDFROMSTRING_H
#define __OSTOANDFROMSTRING_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

OS_API bool osProcessIdToString(osProcessId processId, gtString& outString);
OS_API bool osProcessIdToString(osProcessId processId, gtASCIIString& outString);
OS_API bool osProcessIdFromString(const gtString& string, osProcessId& processId);


#endif //__OSTOANDFROMSTRING_H

