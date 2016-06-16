//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osDirectory.cpp
///
//=====================================================================

//------------------------------ osDirectory.cpp ------------------------------

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>


// ---------------------------------------------------------------------------
// Name:        osDirectory::exists
// Description: Returns true iff this directory exists on disk.
// Author:      AMD Developer Tools Team
// Date:        15/5/2004
// ---------------------------------------------------------------------------
bool osDirectory::exists() const
{
    bool retVal = false;

    // Try to get the file attributes of this directory:
    DWORD fileAttributes = GetFileAttributes(_directoryPath.asString().asCharArray());

    // If we managed to get the file attributes:
    if (fileAttributes != INVALID_FILE_ATTRIBUTES)
    {
        // Verify that this is a directory:
        if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osDirectory::create
// Description: Create the directory on the disk. Return true when succeed.
// Author:      AMD Developer Tools Team
// Date:        17/6/2004
// ---------------------------------------------------------------------------
bool osDirectory::create()
{
    bool retVal = false;


    // Make sure that the requested folder is not empty:
    GT_IF_WITH_ASSERT(!_directoryPath.asString().isEmpty())
    {
        int rcCreate = CreateDirectory(_directoryPath.asString().asCharArray(), NULL);
        retVal = rcCreate != 0;

        if (!retVal)
        {
            // Get the error code for the directory creation failure:
            DWORD lastErrorCode = GetLastError();

            if (lastErrorCode == ERROR_ALREADY_EXISTS)
            {
                retVal = true;
            }
            else if (lastErrorCode == ERROR_PATH_NOT_FOUND)
            {
                // The folder parent does not exist:
                bool directoryExist = false;
                int amountOfUpLevel = 0;
                osDirectory tempFolderPath(*this);

                bool noMoreUp = false;

                while (!directoryExist && !noMoreUp)
                {
                    // Go up one level:
                    osDirectory beforeDirectory = tempFolderPath;
                    tempFolderPath.upOneLevel();

                    if (beforeDirectory.directoryPath().asString() == tempFolderPath.directoryPath().asString())
                    {
                        noMoreUp = true;
                    }
                    else
                    {
                        // Mark that we went up one level more:
                        amountOfUpLevel ++ ;

                        // Check if the current level exists:
                        directoryExist = tempFolderPath.exists();
                    }
                }

                if (directoryExist)
                {
                    retVal = true;

                    bool rcCreatedPreviousLevel = true;

                    // Go through each of the levels and create them:
                    for (int i = amountOfUpLevel ; i >= 0 && rcCreatedPreviousLevel; i--)
                    {
                        // Setup the first directory path to this:
                        tempFolderPath = *this;

                        // Go up i levels:
                        for (int j = 0; j < i; j++)
                        {
                            tempFolderPath.upOneLevel();
                        }

                        // Create the folder:
                        rcCreatedPreviousLevel = tempFolderPath.create();
                        GT_ASSERT(rcCreatedPreviousLevel);

                        retVal = retVal && rcCreatedPreviousLevel;
                    }

                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osDirectory::deleteFile
// Description: Deletes a file that is contained in this directory.
// Arguments:   fileName - The contained file name (including extension)
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/4/2005
// ---------------------------------------------------------------------------
bool osDirectory::deleteFile(const gtString& fileName)
{
    bool retVal = false;

    // Get the full path of the file:
    gtString filePath = _directoryPath.asString();
    filePath += osFilePath::osPathSeparator;
    filePath += fileName;

    BOOL rc2 = DeleteFile(filePath.asCharArray());

    if (rc2 != 0)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osDirectory::deleteRecursively
// Description: Deletes this directory, its sub directories and contained files
//              (recurse into sub directories)
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        24/1/2005
// ---------------------------------------------------------------------------
bool osDirectory::deleteRecursively()
{
    bool retVal = true;

    // Step A - Get this directory contained files and sub directories:
    // ---------------------------------------------------------------

    // Get the sub-directories paths:
    gtList<osFilePath> subDirectoriesPaths;
    bool rc1 = getSubDirectoriesPaths(SORT_BY_NAME_ASCENDING, subDirectoriesPaths);

    // Get the contained files paths:
    gtList<osFilePath> containedFilesPaths;
    bool rc2 = getContainedFilePaths(OS_ALL_CONTAINED_FILES_SEARCH_STR, containedFilesPaths);

    if (!rc1 || !rc2)
    {
        retVal = false;
    }

    // Step B - delete contained files:
    // -------------------------------
    gtList<osFilePath>::const_iterator iter = containedFilesPaths.begin();
    gtList<osFilePath>::const_iterator endIter = containedFilesPaths.end();

    while (iter != endIter)
    {
        const wchar_t* pCurrentFilePath = (*iter).asString().asCharArray();
        BOOL rc3 = DeleteFile(pCurrentFilePath);

        if (rc3 == 0)
        {
            retVal = false;
        }

        ++iter;
    }

    // Step C - delete contained sub directories recursively:
    // -----------------------------------------------------
    iter = subDirectoriesPaths.begin();
    endIter = subDirectoriesPaths.end();

    while (iter != endIter)
    {
        const osFilePath currentSubDirPath = (*iter);
        osDirectory currentSubDir(currentSubDirPath);
        bool rc4 = currentSubDir.deleteRecursively();

        if (!rc4)
        {
            retVal = false;
        }

        ++iter;
    }

    // Step D - delete this directory:
    // -------------------------------
    const wchar_t* pThisDirectoryPath = _directoryPath.asString().asCharArray();
    BOOL rc5 = RemoveDirectory(pThisDirectoryPath);

    if (rc5 == 0)
    {
        retVal = false;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osDirectory::getSubDirectoriesPaths
// Description: Returns a list of the paths of this directory child directories.
//              (Direct children directories only - non recursive)
// Arguments:
//   sortMethod - The order in which the output sub directory paths will be sorted.
//   subDirectoriesPaths - The output sub directories path list.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        15/5/2004
// ---------------------------------------------------------------------------
bool osDirectory::getSubDirectoriesPaths(SortMethod sortMethod,
                                         gtList<osFilePath>& subDirectoriesPaths) const
{
    bool retVal = false;

    // Make the output list empty:
    subDirectoriesPaths.clear();

    // Build the search string:
    gtString searchString = _directoryPath.asString();
    searchString += L"\\";
    searchString += OS_ALL_CONTAINED_FILES_SEARCH_STR;

    // Look for the first directory file:
    WIN32_FIND_DATA fileData;
    HANDLE searchHandle = FindFirstFile(searchString.asCharArray(), &fileData);

    // If we failed to find the first file:
    if (searchHandle == INVALID_HANDLE_VALUE)
    {
        // If it was because there are no such files:
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            retVal = true;
        }
    }
    else // searchHandle != INVALID_HANDLE_VALUE
    {
        // Iterate the contained files:
        bool goOn = true;

        while (goOn)
        {
            // If the current file is a directory:
            if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // Get the current file name:
                gtString currentFilePathAsString = _directoryPath.asString();
                currentFilePathAsString += '\\';
                currentFilePathAsString += fileData.cFileName;

                osFilePath currentFilePath = currentFilePathAsString;

                /*
                dwAttrs = GetFileAttributes(FileData.cFileName);
                    if (!(dwAttrs & FILE_ATTRIBUTE_READONLY))
                */

                // Get the current file name:
                gtString currentFileName;
                bool rc = currentFilePath.getFileName(currentFileName);

                if (rc)
                {
                    // Ignore this directory, the "." and ".." directories:
                    if ((!currentFileName.isEmpty() && (currentFileName != L".") && (currentFileName != L"..")))
                    {
                        // Add it to the output sub directories list:
                        subDirectoriesPaths.push_back(currentFilePath);
                    }
                }
            }

            // Find the next file that match the search string:
            if (FindNextFile(searchHandle, &fileData) == FALSE)
            {
                goOn = false;

                // If we failed because there are no more files that match the search
                // string:
                if (GetLastError() == ERROR_NO_MORE_FILES)
                {
                    retVal = true;
                }
            }
        }

        // Close the search handle.
        FindClose(searchHandle);
    }

    // Apply the sorting:
    sortFilePathsListFromAscendingNameOrder(subDirectoriesPaths, sortMethod);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osDirectory::getContainedFilePaths
// Description: Returns a list of paths of the files that are contained in this directory.
//              (Direct children directories only - non recursive).
//              On NTFS - the files are returned in alphabetic order.
// Arguments:
//   fileNameSearchString - A search string for the file name.
//                          The search string can contain wildcard characters (* and ?).
//                          Example: "*.txt".
//   filePaths - The output file paths list.
//   clearOutVal - true if we clear filePaths before filling it up
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        15/5/2004
// ---------------------------------------------------------------------------
bool osDirectory::getContainedFilePaths(const gtString& fileNameSearchString,
                                        gtList<osFilePath>& filePaths, bool clearOutVal/* = true*/) const
{
    bool retVal = false;

    // Make the output list empty:
    if (clearOutVal)
    {
        filePaths.clear();
    }

    // Build the search string:
    gtString searchString = _directoryPath.asString();
    searchString += L"\\";
    searchString += fileNameSearchString;

    // Look for the first file that match the search string:
    WIN32_FIND_DATA fileData;
    HANDLE searchHandle = FindFirstFile(searchString.asCharArray(), &fileData);

    // If we failed to find the first file:
    if (searchHandle == INVALID_HANDLE_VALUE)
    {
        // If it was because there are no such files:
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            retVal = true;
        }
    }
    else
    {
        // Iterate the files that match the search string:
        bool goOn = true;

        while (goOn)
        {
            // Verify that this is not a directory:
            if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                // Get the current file name:
                gtString currentFilePathAsString = _directoryPath.asString();
                currentFilePathAsString += '\\';
                currentFilePathAsString += fileData.cFileName;

                osFilePath currentFilePath = currentFilePathAsString;

                /*
                dwAttrs = GetFileAttributes(FileData.cFileName);
                 if (!(dwAttrs & FILE_ATTRIBUTE_READONLY))

                */

                // Add it to the output files list:
                filePaths.push_back(currentFilePath);
            }

            // Find the next file that match the search string:
            if (FindNextFile(searchHandle, &fileData) == FALSE)
            {
                goOn = false;

                // If we failed because there are no more files that match the search
                // string:
                if (GetLastError() == ERROR_NO_MORE_FILES)
                {
                    retVal = true;
                }
            }
        }

        // Close the search handle.
        FindClose(searchHandle);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osDirectory::rename
// Description: Renames this directory.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/24/2012
// ---------------------------------------------------------------------------
bool osDirectory::rename(const gtString& newPathName)
{
    bool retVal(true);
    osFilePath newDirectoryPath;

    // if newName is empty
    retVal = !newPathName.isEmpty();

    //Check to see if the new directory exists
    if (retVal)
    {
        newDirectoryPath.setFileDirectory(newPathName);
        retVal = !newDirectoryPath.exists();
    }

    if (retVal)
    {
        gtString sourcePath = _directoryPath.fileDirectoryAsString();

        if (sourcePath[sourcePath.length() - 1] != osFilePath::osPathSeparator)
        {
            sourcePath.append(osFilePath::osPathSeparator);
        }

        gtString targetPath = newPathName;

        if (targetPath[targetPath.length() - 1] != osFilePath::osPathSeparator)
        {
            targetPath.append(osFilePath::osPathSeparator);
        }

        retVal = (TRUE == MoveFile(sourcePath.asCharArray(), targetPath.asCharArray()));

        if (retVal)
        {
            _directoryPath = newDirectoryPath;
        }
        else // fail to move files
        {
            gtString dbgMessage;
            dbgMessage.appendFormattedString(L"Failed to rename the directory path %ls to %ls. (Error code %#x)", _directoryPath.fileDirectoryAsString().asCharArray(), newDirectoryPath.fileDirectoryAsString().asCharArray(), ::GetLastError());
            OS_OUTPUT_DEBUG_LOG(dbgMessage.asCharArray(), OS_DEBUG_LOG_ERROR);
        }
    }

    return retVal;
}



