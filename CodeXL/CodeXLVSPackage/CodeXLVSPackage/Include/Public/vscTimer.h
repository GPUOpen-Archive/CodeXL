//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscTimer.h
///
//==================================================================================

#ifndef vscTimer_h__
#define vscTimer_h__
#include "CodeXLVSPackageCoreDefs.h"
#include <Include/Public/CoreInterfaces/IVscTimerOwner.h>

void* vscTimer_CreateInstance();
void vscTimer_DestroyInstance(void* pVscTimer);
void vscTimer_SetOwner(void* pVscTimer, IVscTimerOwner* pOwner);
void vscTimer_OnClockTick(void* pVscTimer);

#endif // vscTimer_h__
