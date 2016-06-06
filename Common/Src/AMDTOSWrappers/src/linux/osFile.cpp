//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osFile.cpp
///
//=====================================================================

//------------------------------ osFile.cpp ------------------------------

// POSIX:
#include <sys/stat.h>
#include <unistd.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osChannel.h>


// ---------------------------------------------------------------------------
// Name:        osFile::getSize
// Description: Returns the file size.
// Arguments:   fileSize - Output file size
// Return Val:  bool - Success / Failure
// Author:      AMD Developer Tools Team
// Date:        6/11/2006
// ---------------------------------------------------------------------------
bool osFile::getSize(unsigned long& fileSize) const
{
    bool retVal = false;
    fileSize = 0;

    // Try to get the file status:
    struct stat fileStatus;
    std::string utf8FilePath;
    _filePath.asString().asUtf8(utf8FilePath);
    int rc = ::stat(utf8FilePath.c_str(), &fileStatus);

    if (rc == 0)
    {
        // Get the file size:
        fileSize = fileStatus.st_size;

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osFile::hide (Windows only)
// Description: Makes the file invisible to the user in the file system explorer
// Return Val:  bool - Success / Failure
// Author:      Amit Ben-Moshe
// Date:        5/1/2016
// ---------------------------------------------------------------------------
bool osFile::hide()
{
    // Not implemented on Linux.
    return false;
}


// ---------------------------------------------------------------------------
// Name:        osCopyFile
// Description: Copies a file from sourcePath to destinationPath, overwriting if
//              allowOverwrite is true. Only the file being copied is considered
//              success (i.e. allowOverwrite = false with an existing
//              destinationPath is considered a failure).
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        8/7/2012
// ---------------------------------------------------------------------------
bool osCopyFile(const osFilePath& sourcePath, const osFilePath& destinationPath, bool allowOverwrite)
{
    bool retVal = false;

    // If the source file exists (and is not a directory, etc):
    if (!sourcePath.isRegularFile())
    {
        return retVal;
    }

    // do a check on a none const version of the file paths
    osFilePath copySrc(sourcePath);
    osFilePath copyDest(destinationPath);

    copySrc.resolveToAbsolutePath();
    copyDest.resolveToAbsolutePath();
    if (copySrc == copyDest)
    {
        retVal = true;
        return retVal;
    }

    // Check if exist and writable
    if (destinationPath.isRegularFile())
    {
        if (!allowOverwrite)
        {
            return retVal;
        }
        else
        {
            osFile dstFile(destinationPath);
            {
                if (!dstFile.deleteFile())
                {
                    return retVal;
                }
            }
        }
    }

    // Copy file
    gtSize_t read = 0;
    gtSize_t totRead = 0;
    gtSize_t bufSize = 1024;
    gtByte buffer[bufSize];
    osFile srcFile(sourcePath);
    unsigned long srcSize(0);
    osFile dstFile(destinationPath);

    bool bSrc = srcFile.open(osChannel::OS_BINARY_CHANNEL,
                             osFile::OS_OPEN_TO_READ);
    bool bDst = dstFile.open(osChannel::OS_BINARY_CHANNEL,
                             osFile::OS_OPEN_TO_WRITE);

    if (bSrc && bDst)
    {
        srcFile.getSize(srcSize);

        while (srcFile.readAvailableData((gtByte*)&buffer, bufSize, read)
               && read != 0)
        {
            totRead += read;
            dstFile.write(buffer, read);
        }
    }

    if (bSrc)
    {
        srcFile.close();
    }

    if (bDst)
    {
        dstFile.close();
    }

    // Verify the output file, allowing 0 size files to be copied
    if (((totRead != 0) || (0 == srcSize)) && destinationPath.isRegularFile())
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFile::deleteFile
// Description: Deletes the file from disk.
// Author:      AMD Developer Tools Team
// Date:        20/6/2011
// ---------------------------------------------------------------------------
bool osFile::deleteFile()
{
    bool retVal = false;

    // Delete the file from disk:
    std::string utf8FilePath;
    _filePath.asString().asUtf8(utf8FilePath);
    int rc = unlink(utf8FilePath.c_str());

    if (rc == 0)
    {
        retVal = true;
    }

    return retVal;
}


bool osFile::IsExecutable() const
{
    bool result = 0 == access(_filePath.asString().asASCIICharArray(), X_OK);
    return result;
}
