//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscWindowsManager.cpp
///
//==================================================================================

#include <Include/Public/vscWindowsManager.h>
#include <src/vspWindowsManager.h>

//class vscWindowsManager
//{
//public:
//    vscWindowsManager(){}
//    ~vscWindowsManager(){}
//};

void vscWindowsManager_SetIVsUiShell(IVsUIShell* pUiShell)
{
    vspWindowsManager::instance().setUIShell(pUiShell);
}

IVsUIShell* vscWindowsManager_GetIVsUiShell()
{
    return vspWindowsManager::instance().getUIShell();
}
