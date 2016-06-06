//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osGeneralFunctions.cpp
///
//=====================================================================

//------------------------------ osGeneralFunctions.cpp ------------------------------

// POSIX:
#include <sys/utsname.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osLinuxProcFileSystemReader.h>


// ---------------------------------------------------------------------------
// Name:        osGetOSShortDescriptionString
// Description: Retrieves a short string describing the Operating System on which
//              this program runs.
// Author:      AMD Developer Tools Team
// Return Val: bool  - Success / failure.
// Date:        11/7/2007
// ---------------------------------------------------------------------------
bool osGetOSShortDescriptionString(gtString& osDescriptionString)
{
    // Get the Linux variant name:
    gtString linuxVariantName;
    bool retVal = osGetLinuxVariantName(linuxVariantName);

    // Get the OS address space:
    gtString osAddressSpaceString;
    osGetOSAddressSpaceString(osAddressSpaceString);

    // Build the returned string:
    osDescriptionString = L"Linux";
    osDescriptionString += linuxVariantName;
    osDescriptionString += L"-";
    osDescriptionString += osAddressSpaceString;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetOperatingSystemVersionNumber
// Description: gets the operating system's version number as three ints
// Arguments: majorVersion - will contain the kernel major version
//            minorVersion - will contain the kernel minor version
//            buildNumber = will contain the kernel revision number
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        15/5/2008
// ---------------------------------------------------------------------------
bool osGetOperatingSystemVersionNumber(int& majorVersion, int& minorVersion, int& buildNumber)
{
    osLinuxProcFileSystemReader procReader;
    bool retVal = procReader.getKernelVersion(majorVersion, minorVersion, buildNumber);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetOperatingSystemVersionString
// Description: Get the Operating System Version as String
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        22/5/2008
// ---------------------------------------------------------------------------
bool osGetOperatingSystemVersionString(gtString& osVersionName)
{
    osLinuxProcFileSystemReader procReader;
    bool retVal = procReader.getVersionString(osVersionName);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetOSAddressSpaceString
// Description:
//   Retrieves a string describing the address space type of the
//   Operating System on which we run.
// Author:      AMD Developer Tools Team
// Date:        4/2/2009
// ---------------------------------------------------------------------------
void osGetOSAddressSpaceString(gtString& addressSpaceString)
{
    addressSpaceString = L"Unknown";

    // Get information about the OS kernel on which we run:
    struct utsname unameData;
    int rc1 = uname(&unameData);
    GT_IF_WITH_ASSERT(rc1 == 0)
    {
        gtString machineType;
        machineType.fromASCIIString(unameData.machine);

        if (machineType.compareNoCase(L"x86_64") == 0)
        {
            addressSpaceString = OS_64_BIT_ADDRESS_SPACE_AS_STR;
        }
        else if (machineType.compareNoCase(L"ia64") == 0)
        {
            addressSpaceString = OS_ITANIUM_64_BIT_ADDRESS_SPACE_AS_STR;
        }
        else if ((machineType.compareNoCase(L"i386") == 0) || (machineType.compareNoCase(L"i686") == 0))
        {
            addressSpaceString = OS_32_BIT_ADDRESS_SPACE_AS_STR;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        osGetOSAddressSpace
// Description: Return address space
// Author:      AMD Developer Tools Team
// Date:        06/09/2012
// ---------------------------------------------------------------------------
bool osGetOSAddressSpace(int& addressSpace)
{
    bool retVal = false;

    // Get information about the OS kernel on which we run:
    struct utsname unameData;
    int rc1 = uname(&unameData);
    GT_IF_WITH_ASSERT(rc1 == 0)
    {
        gtString machineType;
        machineType.fromASCIIString(unameData.machine);

        if ((machineType.compareNoCase(L"x86_64") == 0) || (machineType.compareNoCase(L"ia64") == 0))
        {
            addressSpace = AMDT_64_BIT_ADDRESS_SPACE;
        }
        else if ((machineType.compareNoCase(L"i386") == 0) || (machineType.compareNoCase(L"i686") == 0))
        {
            addressSpace = AMDT_32_BIT_ADDRESS_SPACE;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetLinuxVariantName
// Description:  Outputs the Linux variant name.
// Arguments: Will get the Linux variant name or "Unknown" in case of failure.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        8/2/2009
// Implementation notes:
//   We identify the Linux variant name according to the variant release name file.
//   A list of release name files can be found under: http://linuxmafia.com/faq/Admin/release-files.html
// ---------------------------------------------------------------------------
bool osGetLinuxVariantName(gtString& linuxVariantName)
{
    bool retVal = false;

    linuxVariantName = L"Unknown";

    // If this is a SUSE Linux:
    osFilePath suseReleaseFilePath(L"/etc/SuSE-release");
    bool isSUSELinux = suseReleaseFilePath.isRegularFile();

    if (isSUSELinux)
    {
        linuxVariantName = L"SUSE";
        retVal = true;
    }
    else
    {
        // If this is a RedHat Linux:
        osFilePath redHatReleaseFilePath(L"/etc/redhat-release");
        bool isRedHatLinux = redHatReleaseFilePath.isRegularFile();

        if (isRedHatLinux)
        {
            linuxVariantName = L"RedHat";
            retVal = true;
        }
        else
        {
            // If this is a Ubuntu Linux:
            osFilePath ubuntuReleaseFilePath(L"/etc/lsb-release");
            bool isUbuntuLinux = ubuntuReleaseFilePath.isRegularFile();

            if (isUbuntuLinux)
            {
                linuxVariantName = OS_STR_linuxVariantUbuntu;
                retVal = true;
            }
            else
            {
                // If this is a Debian Linux:
                osFilePath debianReleaseFilePath(L"/etc/debian_version");
                bool isDebianLinux = debianReleaseFilePath.isRegularFile();

                if (isDebianLinux)
                {
                    linuxVariantName = L"Debian";
                    retVal = true;
                }
                else
                {
                    // If this is a Gentoo Linux:
                    osFilePath gentooReleaseFilePath(L"/etc/gentoo-release");
                    bool isGentooLinux = gentooReleaseFilePath.isRegularFile();

                    if (isGentooLinux)
                    {
                        linuxVariantName = L"Gentoo";
                        retVal = true;
                    }
                }
            }
        }
    }

    return retVal;
}

