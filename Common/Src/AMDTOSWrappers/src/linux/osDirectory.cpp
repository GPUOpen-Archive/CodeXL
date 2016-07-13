//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osDirectory.cpp
///
//=====================================================================

//------------------------------ osDirectory.cpp ------------------------------

// POSIX:
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <dirent.h>
#include <fnmatch.h>
#include <stdio.h>
#include <fcntl.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osSystemError.h>


// ---------------------------------------------------------------------------
// Name:        osDirectory::exists
// Description: Returns true iff this directory exists on disk.
// Author:      AMD Developer Tools Team
// Date:        15/5/2004
// ---------------------------------------------------------------------------
bool osDirectory::exists() const
{
    bool retVal = false;

    // Convert directory to UTF8
    std::string utf8Path;
    _directoryPath.asString().asUtf8(utf8Path);

    // Try to get the file status:
    struct stat fileStatus;
    int rc = ::stat(utf8Path.c_str(), &fileStatus);

    if (rc == 0)
    {
        // If this is a directory:
        if (S_ISDIR(fileStatus.st_mode))
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
// Date:        2/11/2006
// ---------------------------------------------------------------------------
bool osDirectory::create()
{
    bool retVal = false;

    // If directory exists no need to create it:
    if (exists())
    {
        retVal = true;
    }
    else
    {
        gtString directoryAsStr = _directoryPath.asString();
        mode_t directoryPremissions = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;

        // Start creating the sub directories if they don't exist skipping the initial root dir:
        int rc = 0;

        // Do not look at the first slash of the root directory:
        int slashPosition = 1;

        while (0 == rc && -1 != slashPosition)
        {
            // Find the next sub directory:
            slashPosition = directoryAsStr.find('/', slashPosition);

            // Get the sub directory string (take full name if there is not next subsection):
            gtString subDirectoryStr = directoryAsStr;

            if (slashPosition != -1)
            {
                directoryAsStr.getSubString(0, slashPosition - 1, subDirectoryStr);
            }

            osFilePath subDirectoryPath(subDirectoryStr);
            osDirectory subDirectory(subDirectoryPath);

            // If sub directory does not exist create it:
            if (!subDirectory.exists())
            {
                std::string utf8Path;
                subDirectoryStr.asUtf8(utf8Path);
                rc = ::mkdir(utf8Path.c_str(), directoryPremissions);
            }

            // Move the slash Position after the slash:
            if (slashPosition != -1)
            {
                slashPosition++;
            }
        }

        if (0 == rc)
        {
            retVal = true;
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
// Date:        2/11/2006
// ---------------------------------------------------------------------------
bool osDirectory::deleteFile(const gtString& fileName)
{
    bool retVal = false;

    // Get the full path of the file:
    gtString filePath = _directoryPath.asString();
    filePath += osFilePath::osPathSeparator;
    filePath += fileName;

    std::string utf8Path;
    filePath.asUtf8(utf8Path);

    int rc = ::unlink(utf8Path.c_str());

    if (rc == 0)
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
// Date:        2/11/2006
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
        const osFilePath& currFilePath = *iter;
        osFile currFile(currFilePath);
        bool rc3 = currFile.deleteFile();

        if (rc3 == false)
        {
            GT_ASSERT(false);
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
            GT_ASSERT(false);
            retVal = false;
        }

        ++iter;
    }

    // Step D - delete this directory:
    // -------------------------------
    std::string utf8Path;
    _directoryPath.asString().asUtf8(utf8Path);

    int rc5 = ::rmdir(utf8Path.c_str());

    if (rc5 != 0)
    {
        GT_ASSERT(rc5 == 0);
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

    // Open the input directory:
    std::string utf8DirectoryPath;
    _directoryPath.asString().asUtf8(utf8DirectoryPath);
    DIR* pDirectoryStream = ::opendir(utf8DirectoryPath.c_str());
    GT_IF_WITH_ASSERT(pDirectoryStream != NULL)
    {
        retVal = true;

        // Iterate the directory files:
        struct dirent* pCurrDirFile = ::readdir(pDirectoryStream);

        while (pCurrDirFile != NULL)
        {
            // Get the current file name:
            gtString currFileName;
            currFileName.fromUtf8String(pCurrDirFile->d_name);

            // Ignore this directory, the "." and ".." directories:
            if ((!currFileName.isEmpty() && (currFileName != L".") && (currFileName != L"..")))
            {
                // Build the current file full path:
                gtString currFilePathAsStr = _directoryPath.asString();
                currFilePathAsStr += osFilePath::osPathSeparator;
                currFilePathAsStr += currFileName;
                osFilePath currFilePath = currFilePathAsStr;

                // If the current file is a directory:
                if (currFilePath.isDirectory())
                {
                    // Add the current file to the output files list:
                    subDirectoriesPaths.push_back(currFilePath);
                }
            }

            // Next directory file:
            pCurrDirFile = ::readdir(pDirectoryStream);
        }

        // Close the directory:
        int rc1 = ::closedir(pDirectoryStream);
        GT_ASSERT(rc1 == 0);
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

    // Open the input directory:
    std::string utf8DirectoryPath;
    _directoryPath.asString().asUtf8(utf8DirectoryPath);
    DIR* pDirectoryStream = ::opendir(utf8DirectoryPath.c_str());
    GT_IF_WITH_ASSERT(pDirectoryStream != NULL)
    {
        retVal = true;

        // Iterate the directory files:
        struct dirent* pCurrDirFile = ::readdir(pDirectoryStream);

        while (pCurrDirFile != NULL)
        {
            // Get the current file name:
            gtString currFileName;
            currFileName.fromUtf8String(pCurrDirFile->d_name);

            // Ignore this directory, the "." and ".." directories:
            if ((!currFileName.isEmpty() && (currFileName != L".") && (currFileName != L"..")))
            {
                // Build the current file full path:
                gtString currFilePathAsStr = _directoryPath.asString();
                currFilePathAsStr += osFilePath::osPathSeparator;
                currFilePathAsStr += currFileName;
                osFilePath currFilePath = currFilePathAsStr;

                // If the current file is a regular file:
                if (currFilePath.isRegularFile())
                {
                    // If the current file match the input search string:
                    std::string utf8fileNameSearchString, utf8currFileName;
                    fileNameSearchString.asUtf8(utf8fileNameSearchString);
                    currFileName.asUtf8(utf8currFileName);
                    int rc1 = ::fnmatch(utf8fileNameSearchString.c_str(), utf8currFileName.c_str(), 0);

                    if (rc1 == 0)
                    {
                        // Add the current file to the output files list:
                        filePaths.push_back(currFilePath);
                    }
                }
            }

            // Next directory file:
            pCurrDirFile = ::readdir(pDirectoryStream);
        }

        // Close the directory:
        int rc2 = ::closedir(pDirectoryStream);
        GT_ASSERT(rc2 == 0);
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
        std::string utf8OldPath, utf8NewPath;
        _directoryPath.fileDirectoryAsString().asUtf8(utf8OldPath);
        newDirectoryPath.fileDirectoryAsString().asUtf8(utf8NewPath);

        retVal = (0 == ::rename(utf8OldPath.c_str(), utf8NewPath.c_str()));

        if (retVal)
        {
            _directoryPath = newDirectoryPath;
        }
    }

    return retVal;
}

