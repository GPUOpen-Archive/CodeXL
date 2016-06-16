//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osDirectory.h
///
//=====================================================================

//------------------------------ osDirectory.h ------------------------------

#ifndef __OSDIRECTORY
#define __OSDIRECTORY

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtList.h>

// Local:
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>

// A search string that brings all files:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define OS_ALL_CONTAINED_FILES_SEARCH_STR L"*.*"
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define OS_ALL_CONTAINED_FILES_SEARCH_STR L"*"
#else
    #error Unknown build target!
#endif


// ----------------------------------------------------------------------------------
// Class Name:           osDirectory
// General Description:
//   Represents a Directory - A folder containing files on a disk / other storage device.
//
// Author:      AMD Developer Tools Team
// Creation Date:        6/12/2003
// ----------------------------------------------------------------------------------
class OS_API osDirectory : public osTransferableObject
{
public:
    enum SortMethod
    {
        SORT_BY_NAME_ASCENDING,
        SORT_BY_NAME_DESCENDING,
        SORT_BY_DATE_ASCENDING,
        SORT_BY_DATE_DESCENDING
    };

    osDirectory();
    osDirectory(const osFilePath& directoryPath);
    osDirectory(const osDirectory& other);
    osDirectory& operator=(const osDirectory& other);

#if AMDT_HAS_CPP0X
    osDirectory(osFilePath&& directoryPath);
    osDirectory(osDirectory&& other);
    osDirectory& operator=(osFilePath&& directoryPath);
    osDirectory& operator=(osDirectory&& other);
#endif

    const gtString& asString(bool appendSeparatorToDir = false) const;
    const osFilePath& asFilePath() const { return _directoryPath; }

    /// Is the directory empty?
    /// \param return true iff the directory doesn't exist on disk, or contain no files and sub-directories
    bool IsEmpty() const ;

    void setDirectoryPath(const osFilePath& path) { _directoryPath = path;};
    void setDirectoryFullPathFromString(const gtString& path) { _directoryPath.setFileDirectory(path);};
    const osFilePath& directoryPath() const { return _directoryPath; };
    bool exists() const;
    bool create();
    bool deleteFile(const gtString& fileName);
    bool deleteRecursively();
    bool isWriteAccessible();
    bool getContainedFilePaths(const gtString& fileNameSearchString, SortMethod sortMethod,
                               gtList<osFilePath>& filePaths, bool clearOutVal = true) const;
    bool getSubDirectoriesPaths(SortMethod sortMethod, gtList<osFilePath>& subDirectoriesPaths) const;
    osDirectory& upOneLevel();
    osDirectory getParentDirectory() const;

    bool rename(const gtString& newPathName);
    bool copyFilesToDirectory(const gtString& destinationPathString, gtList<gtString>& filenameFilter);

    /// finds file by name + extension in this directory or it's sub directories
    /// \param requestedFile is the searched file + extension
    /// \returns the full path + name of the found file, or empty string if not found
    gtString FindFile(const gtString& requestedFile);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& channel) const;
    virtual bool readSelfFromChannel(osChannel& channel);

private:
    bool getContainedFilePaths(const gtString& fileNameSearchString, gtList<osFilePath>& filePaths, bool clearOutVal = true) const;
    static bool sortFilePathsListFromAscendingNameOrder(gtList<osFilePath>& filePaths, SortMethod sortMethod);

    // Do not allow the use of this function, since the automatic conversion from gtString to
    // osFilePath is not what the user wants:
    void setDirectoryPath(const gtString& path);

private:
    // The directory file path:
    osFilePath _directoryPath;
};


#endif  // __OSDIRECTORY
