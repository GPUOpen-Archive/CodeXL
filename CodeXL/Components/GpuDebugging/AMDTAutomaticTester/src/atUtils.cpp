//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atUtils.cpp
///
//==================================================================================

//------------------------------ atUtils.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// Local:
#include <inc/atUtils.h>

// ---------------------------------------------------------------------------
// Name:        atIsFullPathString
// Description: Helper function used in determining whether a path is relative or not
// Author:      Uri Shomroni
// Date:        29/1/2014
// ---------------------------------------------------------------------------
inline static bool atIsFullPathString(const gtString& pathStr)
{
    bool retVal = false;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    // In windows, full paths start with a drive letter, followed by a colon and a path separator:
    if (2 < pathStr.length())
    {
        if ((':' == pathStr[1]) && (osFilePath::osPathSeparator == pathStr[2]))
        {
            wchar_t firstChar = pathStr[0];

            if ((('a' <= firstChar) && ('z' >= firstChar)) ||
                (('A' <= firstChar) && ('Z' >= firstChar)))
            {
                retVal = true;
            }
        }
    }

#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS

    // In Linux, full paths start with a path separator:
    if (0 < pathStr.length())
    {
        if (osFilePath::osPathSeparator == pathStr[0])
        {
            retVal = true;
        }
    }

#else
#error Unknown build target!
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        atGenerateBaseFilePathList
// Description: Creates a file path list from:
//              - Environment variable, if specified
//              - Current work directory
//              - Executable folder
//              - SDK path, if specified and requested
// Author:      Uri Shomroni
// Date:        29/1/2014
// ---------------------------------------------------------------------------
void atGenerateBaseFilePathList(const gtString& addEnvVarPath, bool addSDKPath, gtVector<gtString>& basePaths)
{
    // If i_addEnvVarPath was specified:
    if (!addEnvVarPath.isEmpty())
    {
        gtString envVarVal;
        bool rcEnv = osGetCurrentProcessEnvVariableValue(addEnvVarPath, envVarVal);

        if (rcEnv && (!envVarVal.isEmpty()))
        {
            envVarVal.removeTrailing(osFilePath::osPathSeparator).append(osFilePath::osPathSeparator);
            basePaths.push_back(envVarVal);
        }
    }

    // Add the current working directory:
    osFilePath currentDir(osFilePath::OS_CURRENT_DIRECTORY);
    currentDir.reinterpretAsDirectory();
    gtString currentDirStr = currentDir.asString();

    if (!currentDirStr.isEmpty())
    {
        currentDirStr.removeTrailing(osFilePath::osPathSeparator).append(osFilePath::osPathSeparator);
        basePaths.push_back(currentDirStr);
    }

    // Add the current executable path:
    osFilePath currentExeFolder;
    bool rcPth = osGetCurrentApplicationPath(currentExeFolder);

    if (rcPth)
    {
        currentExeFolder.clearFileName().clearFileExtension().reinterpretAsDirectory();
        gtString currentExeFolderStr = currentExeFolder.asString();

        if (!currentExeFolderStr.isEmpty())
        {
            currentExeFolderStr.removeTrailing(osFilePath::osPathSeparator).append(osFilePath::osPathSeparator);
            basePaths.push_back(currentExeFolderStr);
        }
    }

    // Add the SDK path if requested
    if (addSDKPath)
    {
        static const gtString appSDKEnvVarName = L"AMDAPPSDKSAMPLESROOT";
        gtString envVarVal;
        bool rcEnv = osGetCurrentProcessEnvVariableValue(appSDKEnvVarName, envVarVal);

        if (rcEnv && (!envVarVal.isEmpty()))
        {
            envVarVal.removeTrailing(osFilePath::osPathSeparator).append(osFilePath::osPathSeparator);
            basePaths.push_back(envVarVal);
        }
    }


}


// ---------------------------------------------------------------------------
// Name:        atMatchFilePathToBasePaths
// Description: If the file path given is absolute, outputs it. Otherwise,
//              checks if it is a path relative to any of the base paths given.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        29/1/2014
// ---------------------------------------------------------------------------
bool atMatchFilePathToBasePaths(const gtString& i_filePath, const gtVector<gtString>& i_basePaths, osFilePath& o_matchedPath)
{
    bool retVal = false;

    if (atIsFullPathString(i_filePath))
    {
        o_matchedPath.setFullPathFromString(i_filePath);

        if (o_matchedPath.exists())
        {
            retVal = true;
        }
    }
    else // !atIsFullPathString(i_filePath)
    {
        int numberOfBasePaths = (int)i_basePaths.size();

        for (int i = 0; numberOfBasePaths > i; i++)
        {
            gtString currentTestedPath = i_basePaths[i];
            currentTestedPath.append(i_filePath);
            o_matchedPath.setFullPathFromString(currentTestedPath);

            if (o_matchedPath.exists())
            {
                retVal = true;
                break;
            }
        }
    }

    if (!retVal)
    {
        o_matchedPath.clear();
    }

    return retVal;
}

