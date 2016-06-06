//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osApplication.cpp
///
//=====================================================================

//------------------------------ osApplication.cpp ------------------------------

#include <wctype.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <VersionInfo/VersionInfo.h>

osFilePath* os_stat_applicationDllsPath = NULL;

int osGetRedirectionFileName(const gtString& commandLine, int startingPos, gtString& fileName);

// ---------------------------------------------------------------------------
// Name:        osGetCurrentApplicationName
// Description: Returns the current application name (the exe file name).
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        2/1/2006
// ---------------------------------------------------------------------------
bool osGetCurrentApplicationName(gtString& applicationName)
{
    bool retVal = false;

    // Get the current application path:
    osFilePath currApplicationPath;
    bool rc1 = osGetCurrentApplicationPath(currApplicationPath);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Get its file name:
        bool rc2 = currApplicationPath.getFileName(applicationName);
        GT_IF_WITH_ASSERT(rc2)
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osSetCurrentApplicationDllsPath
// Description: Sets the path to a directory where the CodeXL dlls are located
// Author:      AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
void osSetCurrentApplicationDllsPath(const osFilePath& dllsPath)
{
    if (os_stat_applicationDllsPath == NULL)
    {
        os_stat_applicationDllsPath = new osFilePath(dllsPath);
    }
    else // os_stat_applicationDllsPath != NULL
    {
        *os_stat_applicationDllsPath = dllsPath;
    }
}

// ---------------------------------------------------------------------------
// Name:        osGetCurrentApplicationDllsPath
// Description: Gets the path to a directory where the CodeXL dlls are located
// Retrieve the path to the DLLs that this app is using, if it has been set.
// This is used when running as a Visual Studio extension, in which case the
// CodeXL DLLs are not located in the same folder as the application executable.
// \param osModuleArchitecture specifies if the retrieved path should be of DLLs with
//                             a specific architecture. DLLs specific to 64-bit architecture
//                             reside in <DLLs path>/x64 while 32-bit specific DLLs
//                             reside in <DLLs path>/x86
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
bool osGetCurrentApplicationDllsPath(osFilePath& dllsPath, osModuleArchitecture specificArchitecture /* = OS_UNKNOWN_ARCHITECTURE */)
{
    bool retVal = (os_stat_applicationDllsPath != NULL);

    if (retVal)
    {
        dllsPath = *os_stat_applicationDllsPath;

        if (OS_I386_ARCHITECTURE == specificArchitecture)
        {
            dllsPath.appendSubDirectory(OS_STR_32BitDirectoryName);
        }
        else if (OS_X86_64_ARCHITECTURE == specificArchitecture)
        {
            dllsPath.appendSubDirectory(OS_STR_64BitDirectoryName);
        }

    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osCheckForOutputRedirection
// Description: find if redirection for output exists and open file in the correct format
// Arguments:   const gtString& commandLine
// Author:      AMD Developer Tools Team
// Date:        13/6/2013
// ---------------------------------------------------------------------------
bool osCheckForOutputRedirection(gtString& commandLine, gtString& fileName, bool& appendMode)
{
    int nameEnd = -1;
    bool nameLooked = false;
    appendMode = false;

    // Look for the redirection directives:
    int nameStart = commandLine.find(L">>");

    if (nameStart != -1)
    {
        // Look for the name and mark that a directive was found:
        nameEnd = osGetRedirectionFileName(commandLine, nameStart + 2, fileName);
        nameLooked = true;
        appendMode = true;
    }
    else
    {
        nameStart = commandLine.find('>');

        if (nameStart != -1)
        {
            // Look for the name and mark that an append directive was found:
            nameEnd = osGetRedirectionFileName(commandLine, nameStart + 1, fileName);
            nameLooked = true;
        }
    }

    bool retVal = false;

    // if a directive was found:
    if (nameLooked)
    {
        if (nameEnd != -1)
        {
            // remove the redirection section:
            commandLine.extruct(nameStart, nameEnd);

            retVal = true;
        }
        else
        {
            // if no name was found just log it:
            gtString errorStr(OS_STR_Redirection_File_missing);
            errorStr.append(commandLine);
            OS_OUTPUT_DEBUG_LOG(commandLine.asCharArray(), OS_DEBUG_LOG_DEBUG);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osCheckForInputRedirection
// Description: find if redirection for input exists and open file in the correct format
// Arguments:   const gtString& commandLine
// Author:      AMD Developer Tools Team
// Date:        13/6/2013
// ---------------------------------------------------------------------------
bool osCheckForInputRedirection(gtString& commandLine, gtString& fileName)
{
    int nameEnd = -1;
    bool nameLooked = false;

    // Look for the redirection directives:
    int nameStart = commandLine.find('<');

    if (nameStart != -1)
    {
        // Look for the name and mark that a directive was found:
        nameEnd = osGetRedirectionFileName(commandLine, nameStart + 1, fileName);
        nameLooked = true;
    }

    bool retVal = false;

    // if a directive was found:
    if (nameLooked)
    {
        if (nameEnd != -1)
        {
            // remove the redirection section:
            commandLine.extruct(nameStart, nameEnd);
            retVal = true;
        }
        else
        {
            // if no name was found just log it:
            gtString errorStr(OS_STR_Redirection_File_missing);
            errorStr.append(commandLine);
            OS_OUTPUT_DEBUG_LOG(commandLine.asCharArray(), OS_DEBUG_LOG_DEBUG);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afWin32RedirectionManager::getRedirectionFileName
// Description: find the name of the file name and return the end position
// Return Val:  int
// Author:      AMD Developer Tools Team
// Date:        19/6/2013
// ---------------------------------------------------------------------------
int osGetRedirectionFileName(const gtString& commandLine, int startingPos, gtString& fileName)
{
    int retVal = -1;
    int currentPos = startingPos;

    // Find first char that is not space:
    while (commandLine[currentPos++] != ' ' && currentPos < commandLine.length());

    if (currentPos < commandLine.length())
    {
        int startName = currentPos;
        bool inQuota = false;

        // if the name is in quote special case:
        if ('\"' == commandLine[currentPos])
        {
            // if there is a second ", retVal will not be -1 and the getSubString will create a file name correctly
            retVal = commandLine.find('\"', currentPos + 1);

            // if there is one move to the next pos for right name termination:
            if (retVal != -1)
            {
                retVal++;
            }

            inQuota = true;
        }
        else
        {
            // look until none char is reached or end of line:
            while ((iswalnum(commandLine[currentPos]) || (wcschr(L".\\/:", commandLine[currentPos]) != NULL)) && currentPos < commandLine.length())
            {
                currentPos++;
            }

            retVal = currentPos;
        }

        // create the name of the file if it was found correctly
        if (retVal != -1)
        {
            commandLine.getSubString(startName, retVal, fileName);

            // in quota trip the " from the start and end
            if (inQuota)
            {
                fileName.removeChar('"');
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osApplication::osGetApplicationVersionFromMacros
// Description: Retrieves the CodeXL application version from as indicated by the
//              macros found in VersionInfo.h.
// Author:      AMD Developer Tools Team
// Date:        11/6/2014
// ---------------------------------------------------------------------------
OS_API void osGetApplicationVersionFromMacros(osProductVersion& applicationVersion)
{
    struct TempStructType
    {
        int _majorVersion;
        int _minorVersion;
        int _patchNumber;
        int _revisionNumber;
    };

    TempStructType temp = { 0, 0, 0, 0 };

    TempStructType local = { PRODUCTVER };
    temp = local;

    // Output the product version:
    applicationVersion._majorVersion = temp._majorVersion;
    applicationVersion._minorVersion = temp._minorVersion;
    applicationVersion._patchNumber = temp._patchNumber;
    applicationVersion._revisionNumber = temp._revisionNumber;
}
