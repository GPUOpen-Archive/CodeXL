//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscOwnerRegistrationManager.cpp
///
//==================================================================================

#include "stdafx.h"
#include <Include\Public\vscOwnerRegistrationManager.h>
#include <src\vscDebugEngine.h>
#include <src\vscBreakpointsManager.h>
#include <src\vspEventObserver.h>
#include <src\vspWindowsManager.h>
#include <src\vspProgressBarWrapper.h>

void vscSetVscDebugEngineOwner(IVscDebugEngineOwner* pOwner)
{
    vspCDebugEngine::setOwner(pOwner);
}

void vscSetVscBreakpoinstManagerOwner(IVscBreakpointsManagerOwner* pOwner)
{
    vscBreakpointsManager::setOwner(pOwner);
}

void vscSetVscEventsObserverOwner(IVscEventObserverOwner* pOwner)
{
    vspEventObserver::setOwner(pOwner);
}

void vscSetVscWindowsManagerOwner(IVscWindowsManagerOwner* pOwner)
{
    vspWindowsManager::setOwner(pOwner);
}

void vscSetVscProgressBarWrapperOwner(IProgressBarEventHandler* pOwner)
{
    vscProgressBarWrapper::setOwner(pOwner);
}
