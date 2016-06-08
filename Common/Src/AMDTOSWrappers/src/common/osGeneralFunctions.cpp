//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osGeneralFunctions.cpp
///
//=====================================================================

//------------------------------ osGeneralFunctions.cpp ------------------------------
// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osTCPSocketServer.h>

// ---------------------------------------------------------------------------
// Name:        osGetBinariesAddressSpaceString
// Description: Retrieves a string describing the address space type of the
//              compiled binaries in which this function appears.
//
// Author:      AMD Developer Tools Team
// Date:        11/7/2007
// ---------------------------------------------------------------------------
void osGetBinariesAddressSpaceString(gtString& addressSpaceString)
{
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
    addressSpaceString = OS_32_BIT_ADDRESS_SPACE_AS_STR;
#elif  AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    addressSpaceString = OS_64_BIT_ADDRESS_SPACE_AS_STR;
#else
#error Error Unknown address space type!
#endif
}


// ---------------------------------------------------------------------------
// Name:        osProcedureAddressToString
// Description: Translates a pointer address into a string.
// Arguments:   pointer - The pointer address.
//              is64BitAddress - true for 64 bit address, false for 32 bit address.
//              inUppercase - Should the output string be uppercase.
//              outputString - Will get the output string.
// Author:      AMD Developer Tools Team
// Date:        7/7/2010
// ---------------------------------------------------------------------------
void osProcedureAddressToString(osProcedureAddress64 pointer, bool is64BitAddress, bool inUppercase, gtString& outputString)
{
    outputString.makeEmpty();

    if (is64BitAddress)
    {
        if (inUppercase)
        {
            outputString.appendFormattedString(GT_64_BIT_POINTER_FORMAT_UPPERCASE, pointer);
        }
        else
        {
            outputString.appendFormattedString(GT_64_BIT_POINTER_FORMAT_LOWERCASE, pointer);
        }
    }
    else
    {
        if (inUppercase)
        {
            outputString.appendFormattedString(GT_32_BIT_POINTER_FORMAT_UPPERCASE, (gtUInt32)pointer);
        }
        else
        {
            outputString.appendFormattedString(GT_32_BIT_POINTER_FORMAT_LOWERCASE, (gtUInt32)pointer);
        }
    }
}


// This function tries to tell whether a given module name is a Windows system library.
//
// We look for ':' (C:...) or name ending with ".dll", ".sys" or ".exe".
// If so, look at the path to see whether it includes "\\Windows\\" or contains "\\Sys*" or "\\winsxs\\".
//
bool osIsWindowsSystemModule(const gtString& absolutePath)
{
    bool ret = false;

    if (absolutePath.length() > 4 && (absolutePath.endsWith(L".dll") ||
                                      absolutePath.endsWith(L".sys") ||
                                      absolutePath.endsWith(L".exe")))
    {
        // 21 is the minimum of: "\\windows\\system\\*.***"
        if (absolutePath.length() >= 21)
        {
            gtString lowerAbsolutePath = absolutePath;

            lowerAbsolutePath.replace(L'/', L'\\');
            lowerAbsolutePath.toLowerCase();

            int rootPos = lowerAbsolutePath.find(L"\\windows\\");

            if (-1 != rootPos)
            {
                // 9 is the length of "\\windows\\"
                rootPos += 9;

                if (lowerAbsolutePath.compare(rootPos, 3, L"sys") == 0)
                {
                    rootPos += 3;

                    if (lowerAbsolutePath.compare(rootPos, 4, L"tem\\") == 0 || // "\\windows\\system\\"
                        lowerAbsolutePath.compare(rootPos, 6, L"tem32\\") == 0 || // "\\windows\\system32\\"
                        lowerAbsolutePath.compare(rootPos, 6, L"wow64\\") == 0)   // "\\windows\\syswow64\\"
                    {
                        ret = true;
                    }
                }
                else
                {
                    if (lowerAbsolutePath.compare(rootPos, 7, L"winsxs\\") == 0)
                    {
                        ret = true;
                    }
                }
            }
        }
    }

    return ret;
}

// This function tries to tell whether a given module name is a Linux system library.
//
// The special name "[kernel.kallsyms]" is the module name for samples within the kernel.
// Then, if the path does not start with '/' we assume it's not a system library.
// The name must then start with "lib" and have ".so" within it.
// If so, we consider these files to be system libraries if they are from:
//          /lib*
//          /usr/lib*
//          /usr/local/lib*
//          /usr/share/gdb*
//
bool osIsLinuxSystemModule(const gtString& absolutePath)
{
    bool ret = false;

    // Kernel samples
    if (absolutePath.find(L"[kernel.kallsyms]") != -1)
    {
        ret = true;
    }
    else
    {
        // has ".so" within it
        if ((absolutePath.find(L".so") != -1))
        {
            // starts with '/'
            if (L'/' == absolutePath[0])
            {
                // starts with '/lib'
                if (absolutePath.compare(1, 3, L"lib") == 0)
                {
                    ret = true;
                }
                // starts with '/usr/'
                else if (absolutePath.compare(1, 4, L"usr/") == 0)
                {
                    // starts with '/usr/lib' or '/usr/local/lib' or '/usr/share/gdb'
                    if (absolutePath.compare(5, 3, L"lib") ||
                        absolutePath.compare(5, 9, L"local/lib") ||
                        absolutePath.compare(5, 9, L"share/gdb") == 0)
                    {
                        ret = true;
                    }
                }
            }
        }
    }

    return ret;
}

bool osIsSystemModule(const gtString& absolutePath)
{
    bool ret;

    if (absolutePath.length() > 4 && (absolutePath.endsWith(L".dll") ||
                                      absolutePath.endsWith(L".sys") ||
                                      absolutePath.endsWith(L".exe")))
    {
        ret = osIsWindowsSystemModule(absolutePath);
    }
    else
    {
        ret = osIsLinuxSystemModule(absolutePath);
    }

    return ret;
}

OS_API bool osIsLocalPortAvaiable(const unsigned short port)
{
    osPortAddress portAddress("0.0.0.0", port);
    osTCPSocketServer tcpServer;
    bool result = tcpServer.open() && tcpServer.bind(portAddress);
    return result;
}
