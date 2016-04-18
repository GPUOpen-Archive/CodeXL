//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DllMain.cpp
/// \brief  Defines the entry point for the DLL application
///
//==================================================================================

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    GT_UNREFERENCED_PARAMETER(hModule);
    GT_UNREFERENCED_PARAMETER(ul_reason_for_call);
    GT_UNREFERENCED_PARAMETER(lpReserved);
    return TRUE;
}
