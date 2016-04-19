//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vsdDLLInit.cpp
///
//==================================================================================

//------------------------------ vsdDLLInit.cpp ------------------------------

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Local:
#include <CodeXLVSPackageDebugger/Include/vsdPackageDLLBuild.h>
// #include <CodeXLVSPackageDebugger/Include/vsdInitFunc.h>

// Forward Declarations:
VSD_API bool vsdPackageCodeInitFunc();

BOOL APIENTRY DllMain(HMODULE module,
                      DWORD reason,
                      LPVOID reserved)
{
    GT_UNREFERENCED_PARAMETER(module);
    GT_UNREFERENCED_PARAMETER(reserved);

    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
        {
            if (!vsdPackageCodeInitFunc())
            {
                return FALSE;
            }
        }
        break;

        case DLL_THREAD_ATTACH: break;

        case DLL_THREAD_DETACH: break;

        case DLL_PROCESS_DETACH: break;
    }

    return TRUE;
}

bool vsdPackageCodeInitFunc()
{
    // Initialize the DLL:
    bool retVal = true;

    return retVal;
}
