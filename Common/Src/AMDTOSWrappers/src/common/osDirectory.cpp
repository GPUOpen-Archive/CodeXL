//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osDirectory.cpp
///
//=====================================================================

//------------------------------ osDirectory.cpp ------------------------------

// Standard C:
#include <sys/types.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osFilePathByLastAccessDateCompareFunctor.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFile.h>

// ---------------------------------------------------------------------------
// Name:        osDirectory::osDirectory
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        15/5/2004
// ---------------------------------------------------------------------------
osDirectory::osDirectory()
{
}


// ---------------------------------------------------------------------------
// Name:        osDirectory::osDirectory
// Description: Constructor - inputs a directory full path.
// Author:      AMD Developer Tools Team
// Date:        15/5/2004
// ---------------------------------------------------------------------------
osDirectory::osDirectory(const osFilePath& directoryPath)
    : _directoryPath(directoryPath)
{
    _directoryPath.reinterpretAsDirectory();
}


osDirectory::osDirectory(const osDirectory& other) : _directoryPath(other._directoryPath)
{
}


osDirectory& osDirectory::operator=(const osDirectory& other)
{
    _directoryPath = other._directoryPath;
    return *this;
}


#if AMDT_HAS_CPP0X

osDirectory::osDirectory(osFilePath&& directoryPath) : _directoryPath(std::forward<osFilePath>(directoryPath))
{
}


osDirectory::osDirectory(osDirectory&& other) : _directoryPath(std::move(other._directoryPath))
{
}


osDirectory& osDirectory::operator=(osFilePath&& directoryPath)
{
    _directoryPath = std::forward<osFilePath>(directoryPath);
    return *this;
}


osDirectory& osDirectory::operator=(osDirectory&& other)
{
    _directoryPath = std::move(other._directoryPath);
    return *this;
}

#endif


// ---------------------------------------------------------------------------
// Name:        osDirectory::getContainedFilePaths
// Description: Returns a list of paths of the files that are contained in this directory.
//              (Direct children directories only - non recursive)
// Arguments:
//   fileNameSearchString - A search string for the file name.
//                          The search string can contain wildcard characters (* and ?).
//                          Example: "*.txt".
//   sortMethod - The order in which the output file paths will be sorted.
//   filePaths - The output file paths list.
//   clearOutVal - true if we clear filePaths before filling it up
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        15/5/2004
// ---------------------------------------------------------------------------
bool osDirectory::getContainedFilePaths(const gtString& fileNameSearchString,
                                        SortMethod sortMethod,
                                        gtList<osFilePath>& filePaths, bool clearOutVal/* = true*/) const
{
    // First - get the file names (in alphabetic order):
    bool retVal = getContainedFilePaths(fileNameSearchString, filePaths, clearOutVal);

    sortFilePathsListFromAscendingNameOrder(filePaths, sortMethod);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osDirectory::sortFilePathsListFromAscendingNameOrder
// Description: Takes a list of file paths sorted by name in ascending order
//              and sorts it according to sortMethod.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/5/2010
// ---------------------------------------------------------------------------
bool osDirectory::sortFilePathsListFromAscendingNameOrder(gtList<osFilePath>& filePaths, SortMethod sortMethod)
{
    bool retVal = true;

    if (sortMethod == SORT_BY_NAME_ASCENDING)
    {
        // The list we got is already sorted by name in ascending order, do nothing more.
    }
    else if (sortMethod == SORT_BY_NAME_DESCENDING)
    {
        // The list we got is sorted by name in ascending order, reverse it:
        filePaths.reverse();
    }
    else if ((sortMethod == SORT_BY_DATE_ASCENDING) || (sortMethod == SORT_BY_DATE_DESCENDING))
    {
        // Convert the List to Vector in order to use gtSort - this is because we need RandomAccessIterator
        // in order to use gtSort
        gtVector<osFilePath> filePathsVector;
        gtList<osFilePath>::iterator listcopyStartIterator = filePaths.begin();
        gtList<osFilePath>::iterator listcopyEndIterator = filePaths.end();

        while (listcopyStartIterator != listcopyEndIterator)
        {
            filePathsVector.push_back(*listcopyStartIterator);
            ++listcopyStartIterator;
        }

        // Sort the filePath vector according to the last edit date:
        osFilePathByLastModifiedDateCompareFunctor compareFunctor;
        gtVector<osFilePath>::iterator firstIter = filePathsVector.begin();
        gtVector<osFilePath>::iterator endIter = filePathsVector.end();
        gtSort(firstIter, endIter, compareFunctor);

        // Update the list - it should be sorted in the exact ordering as the vector
        filePaths.clear();
        firstIter = filePathsVector.begin();
        endIter = filePathsVector.end();

        while (firstIter != endIter)
        {
            filePaths.push_back(*firstIter);
            ++firstIter;
        }

        if (sortMethod == SORT_BY_DATE_ASCENDING)
        {
            filePaths.reverse();
        }
    }
    else // sortMethod
    {
        // Unknown sort method:
        GT_ASSERT(false);
        retVal = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osDirectory::upOneLevel
// Description: Goes one level up, myDir.upOneLevel() is equivalent to calling
//              myDir = myDir.getParentDirectory()
// Author:      AMD Developer Tools Team
// Date:        4/1/2009
// ---------------------------------------------------------------------------
osDirectory& osDirectory::upOneLevel()
{
    gtString dirPath = _directoryPath.asString();

    // If there is no more than 1 path separator, this is a root directory
    // (eg "C:\" on Windows, "/" on Linux or Mac), so we do nothing.
    if (dirPath.count(osFilePath::osPathSeparator) > 1)
    {
        int lastSeparatorPosition = dirPath.reverseFind(osFilePath::osPathSeparator);

        // If the last character is the last slash, find the next one:
        if (lastSeparatorPosition == dirPath.length() - 1)
        {
            lastSeparatorPosition = dirPath.reverseFind(osFilePath::osPathSeparator, lastSeparatorPosition - 1);
        }

        dirPath.truncate(0, (lastSeparatorPosition - 1));
        _directoryPath.clear();
        _directoryPath.setFileDirectory(dirPath);
    }

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        osDirectory::getParentDirectory
// Description: Returns the parent directory of this directory. If this directory
//              is the [drive] root, returns itsself.
// Author:      AMD Developer Tools Team
// Date:        4/1/2009
// ---------------------------------------------------------------------------
osDirectory osDirectory::getParentDirectory() const
{
    osDirectory retVal = *this;
    retVal.upOneLevel();
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osDirectory::copyFilesToDirectory
// Description: Recursively copies the filtered files from this diectory to the destination directory
//              If the files already exist, they will be overwritten
//              If the destination directory does not exist, it will be created
// Return Val:  bool - Success / failure.
// Arguments:
//   destinationPathString - The path to the destination directory
//   filenameFilter - Each search string can contain wildcard characters (* and ?).
//                          Example: "*.txt".
//
// Author:      AMD Developer Tools Team
// Date:        4/24/2012
// ---------------------------------------------------------------------------
bool osDirectory::copyFilesToDirectory(const gtString& destinationPathString, gtList<gtString>& filenameFilter)
{
    bool retVal(true);
    osDirectory destDir;
    gtList<osFilePath> fileList;

    //verify the destination directory exists
    destDir.setDirectoryFullPathFromString(destinationPathString);

    if (!destDir.exists())
    {
        retVal = destDir.create();
    }

    retVal = destDir.exists();

    //Step 1, get list of files to copy
    if (retVal)
    {
        if (filenameFilter.empty())
        {
            gtString allOfThem(OS_ALL_CONTAINED_FILES_SEARCH_STR);
            // get all the file names (in alphabetic order):
            retVal = getContainedFilePaths(allOfThem, fileList);
        }
        else
        {
            gtList<gtString>::const_iterator iter = filenameFilter.begin();
            gtList<gtString>::const_iterator endIter = filenameFilter.end();

            while ((retVal) && (iter != endIter))
            {
                // get all the file names for each filter
                retVal = getContainedFilePaths((*iter), fileList, false);
                ++iter;
            }
        }
    }

    //Step 2, copy each file in the list
    if (retVal)
    {
        gtList<osFilePath>::const_iterator fileIter = fileList.begin();
        gtList<osFilePath>::const_iterator fileEndIter = fileList.end();

        while ((retVal) && (fileIter != fileEndIter))
        {
            gtString newFilePath;
            (*fileIter).getFileNameAndExtension(newFilePath);
            newFilePath.prepend(L"/");
            newFilePath.prepend(destinationPathString);

            retVal = osCopyFile(*fileIter, newFilePath, true);
            ++fileIter;
        }
    }

    //Step 3, recurse for all sub directories
    if (retVal)
    {
        gtList<osFilePath> subDirectoriesPaths;
        retVal = getSubDirectoriesPaths(SORT_BY_NAME_ASCENDING, subDirectoriesPaths);
        gtList<osFilePath>::iterator iter = subDirectoriesPaths.begin();
        gtList<osFilePath>::iterator endIter = subDirectoriesPaths.end();

        while ((retVal) && (iter != endIter))
        {
            osDirectory subDir(*iter);
            gtString subDirPath;

            iter->getFileNameAndExtension(subDirPath);
            subDirPath.prepend(L"/");
            subDirPath.prepend(destinationPathString);

            retVal = subDir.copyFilesToDirectory(subDirPath, filenameFilter);
            ++iter;
        }
    }

    return retVal;
}

bool osDirectory::IsEmpty() const
{
    bool retVal = !exists();

    if (!retVal)
    {
        gtList<osFilePath> listOfFiles;
        bool rc = getContainedFilePaths(OS_ALL_CONTAINED_FILES_SEARCH_STR, listOfFiles);
        GT_IF_WITH_ASSERT(rc)
        {
            retVal = (listOfFiles.size() == 0);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osDirectory::isWriteAccessible
// Description: Verifies that the current user has write permissions to the
//              folder by trying to create a temporary subdir and immediately deleting it.
// Arguments:
// Return Val:  bool - have write permissions / don't have write permissions.
// Author:      AMD Developer Tools Team
// Date:        Oct-15,2013
// ---------------------------------------------------------------------------
bool osDirectory::isWriteAccessible()
{
    // Verify that the directory can be accessed by creating a temporary subdirectory and immediately deleting it
    gtString tempSubDir = _directoryPath.fileDirectoryAsString();
    tempSubDir += osFilePath::osPathSeparator;
    tempSubDir += L"access_permission_trial";

    osDirectory accessPermissionTrialDir(tempSubDir);
    bool isPathAccessible = accessPermissionTrialDir.create();

    if (isPathAccessible)
    {
        // Write permisssions verified. Delete the temp directory
        accessPermissionTrialDir.deleteRecursively();
    }

    return isPathAccessible;
}

// ---------------------------------------------------------------------------
gtString osDirectory::FindFile(const gtString& requestedFile)
{
    gtString foundPath;
    gtList<osFilePath> filePaths;

    // search for fir is current directory
    getContainedFilePaths(requestedFile, osDirectory::SORT_BY_NAME_ASCENDING, filePaths);

    // if no file found
    if (filePaths.size() == 0)
    {
        // get sub directories
        getSubDirectoriesPaths(osDirectory::SORT_BY_NAME_ASCENDING, filePaths);

        // search for the file in every sub dir
        gtList<osFilePath>::iterator iter = filePaths.begin();

        for (; iter != filePaths.end(); ++iter)
        {
            osDirectory* subDir = new osDirectory(*iter);
            foundPath = subDir->FindFile(requestedFile);

            if (!foundPath.isEmpty())
            {
                break;
            }
        }
    }
    else
    {
        // if file found return it's full path, name and extension
        osFilePath filePath = *(filePaths.begin());
        foundPath = filePath.asString();
    }

    return foundPath;
}


// ---------------------------------------------------------------------------
// Name:        osDirectory::type
// Description: Returns my transferable object type.
// Arguments:
// Author:      AMD Developer Tools Team
// Date:        Feb-12,2015
// ---------------------------------------------------------------------------
osTransferableObjectType osDirectory::type() const
{
    return OS_TOBJ_ID_DIRECTORY;
}

// ---------------------------------------------------------------------------
// Name:        osDirectory::writeSelfIntoChannel
// Description: Writes self into a channel.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        Feb-12,2015
// ---------------------------------------------------------------------------
bool osDirectory::writeSelfIntoChannel(osChannel& channel) const
{
    channel << _directoryPath;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        osDirectory::readSelfFromChannel
// Description: Read self from a channel.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        Feb-12,2015
// ---------------------------------------------------------------------------
bool osDirectory::readSelfFromChannel(osChannel& channel)
{
    // Read directory:
    gtAutoPtr<osFilePath> aptrDir;
    bool rc = osReadTransferableObjectFromChannel<osFilePath>(channel, aptrDir);

    if (rc)
    {
        _directoryPath = *aptrDir;
    }

    return rc;
}

// ---------------------------------------------------------------------------
// Name:        osDirectory::asString
// Description: Returns the file path as string.
// Author:      AMD Developer Tools Team
// Date:        Feb-12,2015
// ---------------------------------------------------------------------------
const gtString& osDirectory::asString(bool appendSeparatorToDir) const
{
    return _directoryPath.asString(appendSeparatorToDir);
}