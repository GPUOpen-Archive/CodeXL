//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSystemError.h
///
//=====================================================================

//------------------------------ osSystemError.h ------------------------------

#ifndef __OSSYSTEMERROR_H
#define __OSSYSTEMERROR_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>


OS_API osSystemErrorCode osGetLastSystemError();
OS_API void osGetLastSystemErrorAsString(gtString& systemErrorAsString);
OS_API void osGetSystemErrorAsString(osSystemErrorCode systemError, gtString& systemErrorAsString);


#endif //__OSSYSTEMERROR_H

