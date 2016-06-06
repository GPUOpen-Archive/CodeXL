//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaDriver.cpp
///
//=====================================================================

//------------------------------ oaDriver.cpp ------------------------------

// Local - OSDriver:
#include <AMDTOSAPIWrappers/Include/oaDriver.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// Definitions:
typedef unsigned int (CL_API_CALL* CALGETVERSION_PROC)(unsigned int* major, unsigned int* minor, unsigned int* imp);

// POSIX:
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Linux-only headers:
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
    #include <elf.h>
#endif

// Local:
#include <AMDTOSAPIWrappers/Include/oaStringConstants.h>

// We don't need all of ADLUtil.cpp on Linux, just this function:
void* ADL_Main_Memory_Alloc(int iSize)
{
    void* lpBuffer = malloc(iSize);
    return lpBuffer;
}

// ---------------------------------------------------------------------------
// Name:        oaGetCalVersion
// Description: Dynamically load aticalrt.dll module and using calGetVersion
//              get the cal version
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/5/2011
// ---------------------------------------------------------------------------
OA_API bool oaGetCalVersion(gtString& calVersion)
{
    // TO_DO: not implemented
    calVersion.makeEmpty();

    return false;
}

// ---------------------------------------------------------------------------
// Name:        oaGetDriverVersion
// Description: Dynamically load atiadlxx.dll module and using ADL_Graphics_Versions_Get
//              driverVersion - return value with the catalyst version
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/5/2011
// ---------------------------------------------------------------------------
OA_API gtString oaGetDriverVersion(int& driverError)
{
    driverError = OA_DRIVER_UNKNOWN;
    gtString driverVersion;

    // Get the cal dll module:
    osFilePath driverModulePath(osFilePath::OS_SYSTEM_DIRECTORY, OS_ATI_CALXY_DRIVER_DLL_NAME, OS_MODULE_EXTENSION);
    osModuleHandle driverModuleHandle = NULL;

    bool rcLoadedModule = osLoadModule(driverModulePath, driverModuleHandle, NULL, false);

    if (!rcLoadedModule)
    {
        // Try to get the ATI cal XX driver dll file:
        driverModulePath.setFileName(OS_ATI_CALXX_DRIVER_DLL_NAME);

        // Load the module:
        rcLoadedModule = osLoadModule(driverModulePath, driverModuleHandle, NULL, false);
    }

    // if module not found so far use the catalyst directory (usually in Ubuntu it is needed)
    if (!rcLoadedModule)
    {
        gtString systemDir = driverModulePath.fileDirectoryAsString();
        systemDir.append(L"/");
        systemDir.append(OS_AMD_DRIVER_DIRECTORY);
        driverModulePath.setFileDirectory(systemDir);

        // Load the module:
        rcLoadedModule = osLoadModule(driverModulePath, driverModuleHandle, NULL, false);

        if (!rcLoadedModule)
        {
            driverModulePath.setFileName(OS_ATI_CALXY_DRIVER_DLL_NAME);

            // Load the module:
            rcLoadedModule = osLoadModule(driverModulePath, driverModuleHandle, NULL, false);
        }
    }

    GT_IF_WITH_ASSERT(rcLoadedModule && (driverModuleHandle != NULL))
    {
        driverError = oaGetDriverVersionfromADLModule(driverModuleHandle, driverVersion);

        osReleaseModule(driverModuleHandle);
    }
    else
    {
        driverError = OA_DRIVER_NOT_FOUND;
    }

    // Currently, staging driver does not report the catalyst version properly.
    // This workaround retrieves the Catalyst version via the shell, rather than via the driver API.
    if (driverVersion.isEmpty())
    {
        const char* COMMAND_STR = "dmesg|grep module|grep fglrx";
        gtString outStr;
        gtString versionFromFglrx;
        osExecAndGrabOutput(COMMAND_STR, false, outStr);

        if (!outStr.isEmpty())
        {
            int fglrxPos = outStr.reverseFind(L" - fglrx ");

            if (fglrxPos != -1)
            {
                int verPos = outStr.find(L" [", fglrxPos);

                if (verPos != -1)
                {
                    outStr.getSubString(fglrxPos, verPos, versionFromFglrx);
                    versionFromFglrx = versionFromFglrx.trim();

                    if (!versionFromFglrx.isEmpty())
                    {
                        verPos = versionFromFglrx.reverseFind(L" ");
                        versionFromFglrx.getSubString(verPos + 1, versionFromFglrx.length() - 1, versionFromFglrx);

                        if (!versionFromFglrx.isEmpty())
                        {
                            driverVersion = versionFromFglrx;
                            driverError = OA_DRIVER_OK;
                        }
                    }
                }
            }
        }

        if (driverError != OA_DRIVER_OK)
        {
            // Brahma case.
            const char* COMMAND_STR = "lsmod";
            gtString outStr;
            gtString versionFromFglrx;
            osExecAndGrabOutput(COMMAND_STR, false, outStr);

            if (!outStr.isEmpty())
            {
                int brahmaPos = outStr.find(L"amdgpu");

                if (brahmaPos != -1)
                {
                    // Until we have a method for extracting the Brahma driver version,
                    // just use "Brahma" as the version.
                    driverVersion = L"Brahma driver";
                    driverError = OA_DRIVER_OK;
                }
            }
        }
    }

    return driverVersion;
}

#if CODEXL_HSA_SUPPORT
// HSA only supported on 64-bit configurations:
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
OA_API bool oaIsHSADriver()
{
    return false;
}

OA_API bool oaGetHSADeviceIds(gtVector<gtUInt32>& deviceIDs)
{
    GT_UNREFERENCED_PARAMETER(deviceIDs);

    return false;
}

#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE

// This file causes a compilation error if included in a 32-bit build:
#include <HSAUtils.h>

OA_API bool oaIsHSADriver()
{
    static bool once = true;
    static bool hsaSupported = false;

    if (once)
    {
        once = false;
        gtVector<gtUInt32> hsaDevices;
        bool rcHSA = oaGetHSADeviceIds(hsaDevices);
        hsaSupported = rcHSA && (!hsaDevices.empty());
    }

    return hsaSupported;
}

OA_API bool oaGetHSADeviceIds(gtVector<gtUInt32>& deviceIDs)
{
    bool retVal = false;

    // Get the devices:
    HSADeviceIdList hsaDevices;
    HSAUtils::Instance()->GetHSADeviceIds(hsaDevices);

    if (!hsaDevices.empty())
    {
        // Copy to output vector:
        size_t deviceCount = hsaDevices.size();
        deviceIDs.resize(deviceCount);

        for (size_t i = 0; deviceCount > i; ++i)
        {
            deviceIDs[i] = hsaDevices[i];
        }

        retVal = true;
    }

    return retVal;
}

#else // AMDT_ADDRESS_SPACE_TYPE
#error Unknown Address space type!
#endif // AMDT_ADDRESS_SPACE_TYPE
#else // CODEXL_HSA_SUPPORT
OA_API bool oaIsHSADriver()
{
    return false;
}

OA_API bool oaGetHSADeviceIds(gtVector<gtUInt32>& deviceIDs)
{
    GT_UNREFERENCED_PARAMETER(deviceIDs);

    return false;
}
#endif
