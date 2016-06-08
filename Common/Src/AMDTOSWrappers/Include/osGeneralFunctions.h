//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osGeneralFunctions.h
///
//=====================================================================

//------------------------------ osGeneralFunctions.h ------------------------------

#ifndef __OSGENERALFUNCTIONS_H
#define __OSGENERALFUNCTIONS_H
//std
#include <vector>
// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>
// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// All Operating systems:
OS_API bool osGetOSShortDescriptionString(gtString& osDescriptionString);
OS_API bool osGetOperatingSystemVersionNumber(int& majorVersion, int& minorVersion, int& buildNumber);
OS_API bool osGetOperatingSystemVersionString(gtString& osVersionName);
OS_API void osGetOSAddressSpaceString(gtString& addressSpaceString);
OS_API bool osGetOSAddressSpace(int& addressSpace);
OS_API void osGetBinariesAddressSpaceString(gtString& addressSpaceString);
OS_API void osProcedureAddressToString(osProcedureAddress64 pointer, bool is64BitAddress, bool inUppercase, gtString& outputString);

OS_API bool osIsWindowsSystemModule(const gtString& absolutePath);
OS_API bool osIsLinuxSystemModule(const gtString& absolutePath);
OS_API bool osIsSystemModule(const gtString& absolutePath);
OS_API bool osIsLocalPortAvaiable(const unsigned short port);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

// A Windows OS version:
enum osWindowsVersion
{
    OS_WIN_98,
    OS_WIN_ME,
    OS_WIN_2000,
    OS_WIN_XP,
    OS_WINDOWS_XP_PRO_X64,
    OS_WINDOWS_HOME_SERVER,
    OS_WIN_SERVER_2003,
    OS_WIN_SERVER_2003_R2,
    OS_WIN_VISTA,
    OS_WIN_SERVER_2008,
    OS_WIN_7,
    OS_WIN_SERVER_2008_R2,
    OS_WIN_8,
    OS_WIN_SERVER_2012,
    OS_WIN_8_1,
    OS_WIN_SERVER_2012_R2,
    OS_WIN_10,
};

OS_API bool osGetWindowsVersion(osWindowsVersion& windowsVersion);
OS_API bool osWindowsVersionToString(const osWindowsVersion& windowsVersion, gtString& windowsVersionStr);
OS_API bool osWindowsVersionToShortString(const osWindowsVersion& windowsVersion, gtString& windowsVersionStr);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
// Mac specific:
// GUI elements are not relevant to the iPhone:
#ifndef _AMDT_IPHONE_BUILD
    // We get build problems when compiling these functions under 64 bit (where they are not needed):
    #if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
        typedef struct OpaqueControlRef* ControlRef;
        OS_API bool osGetMacListControlCurrentSortOrder(ControlRef hListCtrl);
        OS_API void osSetMacListControlSortOrderAndColumn(ControlRef hListCtrl, int column = 0, bool ascending = true);
        OS_API bool osSetMacAlwaysOnTopStatusOfWindow(/* WindowRef */ void* rWindow, bool alwaysOnTop);
    #endif
#endif
OS_API bool osGetStringPropertyValueFromPListFile(const gtString& fileFullPath, const gtString& propertyName, gtString& propertyValue);
OS_API gtString osGetExecutableFromMacApplicationBundle(const gtString& bundlePath);
OS_API bool osDoesProcessExist(const gtString& processName);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
// Generic Linux variants:
OS_API bool osGetLinuxVariantName(gtString& linuxVariantName);

#endif // AMDT_BUILD_TARGET


#endif //__OSGENERALFUNCTIONS_H

