//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspDLLInit.cpp
///
//==================================================================================

//------------------------------ vsDLLInit.cpp ------------------------------

#include "stdafx.h"
// Windows:

// VSX:
#include <vsshell.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osModule.h>

// Local:
#include <src/vspQApplicationWrapper.h>

#include <CodeXLVSPackageCoreDefs.h>

// ---------------------------------------------------------------------------
// Name:        vspDllMain
// Description: This function is called from the package DLL main. It initializes WX application.
// Arguments:   DWORD reason
//              LPVOID reserved
// Return Val:  BOOL APIENTRY
// Author:      Sigal Algranaty
// Date:        5/9/2010
// ---------------------------------------------------------------------------
BOOL APIENTRY vspDllMain(DWORD reason, LPVOID reserved, HINSTANCE hInstance)
{
    GT_UNREFERENCED_PARAMETER(reserved);

    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
        {
            // Only run this code one time:
            static bool stat_runOneTime = true;

            if (stat_runOneTime)
            {
                stat_runOneTime = false;

                // Initialize the wxWidgets application object:


                // Register this DLL in the registry to add the debug engine CLSID there:
                // Uri, 19/12/10: Doing this in the code requires VS to be run in administrator mode. Instead we currently do this
                // manually / at the installation.
                // HRESULT hr = DllRegisterServer();
                // GT_ASSERT(SUCCEEDED(hr));

                // This is the main DLL for the VS package, and thus, its directory is the dll binaries directory:
                osFilePath dllsDir;
                bool rcPth = osGetCurrentApplicationDllsPath(dllsDir);

                if (!rcPth)
                {
                    rcPth = osGetLoadedModulePath(hInstance, dllsDir);
                    GT_IF_WITH_ASSERT(rcPth)
                    {
                        // Set it as the application dlls path:
                        dllsDir.clearFileName().clearFileExtension();
                        osSetCurrentApplicationDllsPath(dllsDir);
                    }
                }
            }
        }
        break;

        case DLL_THREAD_ATTACH: break;

        case DLL_THREAD_DETACH: break;

        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}

void vscSetApplicationBinariesFolder(const char* pathStr)
{
    GT_IF_WITH_ASSERT(nullptr != pathStr)
    {
        gtString pathGTStr;
        pathGTStr.fromASCIIString(pathStr);
        osFilePath binariesPath(pathGTStr);
        binariesPath.reinterpretAsDirectory();
        osSetCurrentApplicationDllsPath(binariesPath);
    }
}
