//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaDriver.cpp
///
//=====================================================================

//------------------------------ oaDriver.cpp ------------------------------

// Local - OSDriver.
#include <AMDTOSAPIWrappers/Include/oaDriver.h>

// Forward declarations.
typedef unsigned int (CL_API_CALL* CALGETVERSION_PROC)(unsigned int* major, unsigned int* minor, unsigned int* imp);

// Windows.
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// C++.
#include <xlocbuf>
#include <codecvt>

// Infra.
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osModule.h>

// Local.
#include <AMDTOSAPIWrappers/Include/oaStringConstants.h>

// For the driver version extraction.
#include <ADLUtil.h>

// For Catalyst version extraction.
#define CATALYST_REG_KEY_PARENT           L"SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\"
#define DRIVER_REG_KEY_VALUE_NAME         L"DriverVersion"
#define DRIVER_REG_KEY_PROVIDER_NAME      L"ProviderName"
#define AMD_PROVIDER_NAME                 L"Advanced Micro Devices, Inc."

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
    bool retVal = false;

    calVersion.makeEmpty();

    // Get the cal dll module:
    osFilePath calModulePath(osFilePath::OS_SYSTEM_DIRECTORY, OS_ATI_CALRT_DRIVER_DLL_NAME, OS_MODULE_EXTENSION);
    osModuleHandle calModuleHandle = nullptr;
    bool rc = osLoadModule(calModulePath, calModuleHandle);
    GT_IF_WITH_ASSERT(rc && (calModuleHandle != nullptr))
    {
        // Get the procedure for the cal function:
        osProcedureAddress calVersionFunction = nullptr;
        rc = osGetProcedureAddress(calModuleHandle, "calGetVersion", calVersionFunction);
        GT_IF_WITH_ASSERT(rc && (calVersionFunction != nullptr))
        {
            // Check the version:
            CALGETVERSION_PROC calGetVersionWrapper = (CALGETVERSION_PROC)calVersionFunction;
            unsigned int majorVersion, minorVersion, impVersion;
            unsigned int result = calGetVersionWrapper(&majorVersion, &minorVersion, &impVersion);

            GT_IF_WITH_ASSERT(result == 0)
            {
                calVersion.appendFormattedString(L"%d.%d.%d", majorVersion, minorVersion, impVersion);
                retVal = true;
            }
        }

        osReleaseModule(calModuleHandle);
    }

    return retVal;
}

// ------------------------------------------------------------------------------------------
// Name:        oaGetDriverVersionFromRegistryKeyHelper
// Description: A helper sub-routine for extracting the driver version from the registry key.
//              Note: this function was relocated from AMDTAnalysisBackend's beBackend.cpp.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        5/4/2015
// ------------------------------------------------------------------------------------------
static bool oaGetDriverVersionFromRegistryKeyHelper(const std::wstring& registryKey, gtString& driverVersion, bool& branchExist)
{
    bool ret = false;
    branchExist = false;
    HKEY hKey;
    LONG resultValue = RegOpenKeyEx(HKEY_LOCAL_MACHINE, registryKey.c_str(), 0, KEY_READ, &hKey);

    if (ERROR_SUCCESS == resultValue)
    {
        branchExist = true;
        wchar_t  stringBuffer[256];
        DWORD dwBufferSize = sizeof(stringBuffer);

        // First read the provider name to make sure the driver detailed in this registry path is the AMD driver
        resultValue = RegQueryValueEx(hKey, DRIVER_REG_KEY_PROVIDER_NAME, 0, nullptr, (LPBYTE)&stringBuffer, &dwBufferSize);

        if (ERROR_SUCCESS == resultValue)
        {
            // Is this the AMD driver?
            if (0 == wcsncmp(stringBuffer, AMD_PROVIDER_NAME, 256))
            {
                resultValue = RegQueryValueEx(hKey, DRIVER_REG_KEY_VALUE_NAME, 0, nullptr, (LPBYTE)&stringBuffer, &dwBufferSize);

                if (ERROR_SUCCESS == resultValue)
                {
                    // We got the key, now copy it to the output parameter
                    driverVersion = stringBuffer;
                    ret = true;
                }
            }
        }
    }

    return ret;
}

// ------------------------------------------------------------------------------------------
// Name:        oaGetDriverVersionFromRegistryKeyHelper
// Description: A helper function for extracting the driver version from the registry key.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        5/4/2015
// ------------------------------------------------------------------------------------------
static bool oaGetDriverVersionInfoViaRegistry(gtString& driverVersionWide)
{
    bool isKeyReadSuccess = false;

    // Assume the first branch exists.
    bool branchExist = true;

    // First, try to use the ADL Util library to find the driver version in the registry.
    AsicInfoList asicInfoList;
    ADLUtil_Result result = AMDTADLUtils::Ref().GetAsicInfoList(asicInfoList);

    if (result == ADL_SUCCESS)
    {
        for (size_t i = 0; i < asicInfoList.size() && isKeyReadSuccess == false; ++i)
        {
            std::string& narrow = asicInfoList[i].registryPath;

            // Remove the redundant prefix from the registry path returned by ADL.
            std::string redundantPrefix("\\Registry\\Machine\\");

            if (0 == narrow.find(redundantPrefix))
            {
                narrow.erase(0, redundantPrefix.size());
            }

            // Convert from single byte characters to wide characters.
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring adlRegistryPath = converter.from_bytes(narrow);

            // Read the driver version from the registry.
            isKeyReadSuccess = oaGetDriverVersionFromRegistryKeyHelper(adlRegistryPath, driverVersionWide, branchExist);
        }
    }

    // Fallback:
    // If we did not manage to get the driver version from the registry path provided by the ADL Util,
    // then we try to independently iterate through relevant registry paths and look for the driver version.
    if (!isKeyReadSuccess)
    {
        // Iterate over the potential key branches in the registry and look for the driver version key in each path:
        // "SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000"
        // "SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0001"
        // "SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0002"
        // etc.
        int keyNumber = 0;

        while (!isKeyReadSuccess && branchExist)
        {
            // Generate the numeric part of the branch path.
            wchar_t keyNumString[5];
            wsprintf(keyNumString, L"%0.4d", keyNumber);

            // Combine numeric and textual part into the complete branch path.
            std::wstring keyPath = CATALYST_REG_KEY_PARENT;
            keyPath += keyNumString;
            isKeyReadSuccess = oaGetDriverVersionFromRegistryKeyHelper(keyPath, driverVersionWide, branchExist);

            if (!isKeyReadSuccess)
            {
                // Advance the key number in case.
                keyNumber++;
            }
        }
    }

    return isKeyReadSuccess;
}


// ---------------------------------------------------------------------------
// Name:        oaGetDriverVersion
// Description: Dynamically load atiadlxx.dll module and using ADL_Graphics_Versions_Get
//              get the catalyst version
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
    osModuleHandle driverModuleHandle = nullptr;

    bool rcLoadedModule = false;

    // If this is a win64, cal xy driver file should exists, otherwise, look for cal xx driver file:
    if (driverModulePath.exists())
    {
        // Load the module:
        rcLoadedModule = osLoadModule(driverModulePath, driverModuleHandle);
    }
    else
    {
        // Try to get the ATI cal XX driver dll file:
        driverModulePath.setFileName(OS_ATI_CALXX_DRIVER_DLL_NAME);

        if (driverModulePath.exists())
        {
            // Load the module:
            rcLoadedModule = osLoadModule(driverModulePath, driverModuleHandle);
        }
    }

    GT_IF_WITH_ASSERT(rcLoadedModule && (driverModuleHandle != nullptr))
    {
        driverError = oaGetDriverVersionfromADLModule(driverModuleHandle, driverVersion);

        osReleaseModule(driverModuleHandle);
    }
    else
    {
        driverError = OA_DRIVER_NOT_FOUND;
    }

    // Fallback: try to extract the version via the registry.
    if (driverError != OA_DRIVER_OK)
    {
        driverVersion.makeEmpty();
        bool isExtractedFromReg = oaGetDriverVersionInfoViaRegistry(driverVersion);

        if (isExtractedFromReg && !driverVersion.isEmpty())
        {
            driverError = OA_DRIVER_OK;
        }
    }

    return driverVersion;
}

OA_API bool oaIsHSADriver()
{
    // HSA currently not supported on Windows
    return false;
}


OA_API bool oaGetHSADeviceIds(gtVector<gtUInt32>& deviceIDs)
{
    GT_UNREFERENCED_PARAMETER(deviceIDs);

    // HSA currently not supported on Windows
    return false;
}
