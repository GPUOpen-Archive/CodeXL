//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscWindowsManager.h
///
//==================================================================================

#ifndef vscWindowsManager_h__
#define vscWindowsManager_h__
#include "CodeXLVSPackageCoreDefs.h"
#include <vsshell.h>

void vscWindowsManager_SetIVsUiShell(IVsUIShell* pUiShell);
IVsUIShell* vscWindowsManager_GetIVsUiShell();


#endif // vscWindowsManager_h__
