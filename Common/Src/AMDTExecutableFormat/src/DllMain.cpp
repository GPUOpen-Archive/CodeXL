//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DllMain.cpp
/// \brief  Defines the entry point for the DLL application
///
//==================================================================================

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

static wchar_t g_modulePath[OS_MAX_PATH] = { 0 };

const wchar_t* GetExecutableFormatModulePath()
{
    return g_modulePath;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    GT_UNREFERENCED_PARAMETER(lpReserved);

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            const DWORD maxLen = (sizeof(g_modulePath) / sizeof(*g_modulePath)) - 1;
            DWORD len = GetModuleFileNameW(hModule, g_modulePath, maxLen);

            while (0 != len)
            {
                --len;
                wchar_t wc = g_modulePath[len];

                if (L'\\' == wc || L'/' == wc)
                {
                    g_modulePath[len + 1] = L'\0';
                    break;
                }
            }
        }
        break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}
