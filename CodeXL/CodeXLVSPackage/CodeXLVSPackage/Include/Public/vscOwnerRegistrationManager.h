//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscOwnerRegistrationManager.h
///
//==================================================================================

#ifndef vscOwnerRegistrationManager_h__
#define vscOwnerRegistrationManager_h__

#include "src\CodeXLVSPackageCoreDefs.h"
#include <Include/Public/CoreInterfaces/IVscApplicationCommandsOwner.h>
#include <Include/Public/CoreInterfaces/IVscBreakpointsManagerOwner.h>
#include <Include/Public/CoreInterfaces/IVscDebugEngineOwner.h>
#include <Include/Public/CoreInterfaces/IVscEventObserverOwner.h>
#include <Include/Public/CoreInterfaces/IVscSourceCodeViewerOwner.h>
#include <Include/Public/CoreInterfaces/IVscWindowsManagerOwner.h>
#include <Include/Public/CoreInterfaces/IProgressBarDataProvider.h>

void vscSetVscDebugEngineOwner(IVscDebugEngineOwner* pVscDebugEngineOwner);

void vscSetVscBreakpoinstManagerOwner(IVscBreakpointsManagerOwner* pVscDebugEngineOwner);

void vscSetVscEventsObserverOwner(IVscEventObserverOwner* pVscDebugEngineOwner);

void vscSetVscWindowsManagerOwner(IVscWindowsManagerOwner* pOwner);

void vscSetVscProgressBarWrapperOwner(IProgressBarEventHandler* pOwner);

#endif // vscOwnerRegistrationManager_h__