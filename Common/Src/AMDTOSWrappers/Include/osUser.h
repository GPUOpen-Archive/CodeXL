//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osUser.h
///
//=====================================================================

//------------------------------ osUser.h ------------------------------

#ifndef __OSUSER_H
#define __OSUSER_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

OS_API bool osGetCurrentUserName(gtString& currUserName);

OS_API bool osGetProcessUserName(osProcessId processId, gtString& userName);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    OS_API bool osIsUserAdministrator();
    OS_API bool osSetPrivilege(void*& tokenHandle, wchar_t* pPrivilege, bool enablePrivilege, bool closeHandle);
#endif


#endif //__OSUSER_H

