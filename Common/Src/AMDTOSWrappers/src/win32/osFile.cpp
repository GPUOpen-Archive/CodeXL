//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osFile.cpp
///
//=====================================================================

//------------------------------ osFile.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osFile.h>

// ---------------------------------------------------------------------------
// Name:        osCopyFile
// Description: Copies a file from sourcePath to destinationPath, overwriting if
//              allowOverwrite is true. Only the file being copied is considered
//              success (i.e. allowOverwrite = false with an existing
//              destinationPath is considered a failure).
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        2/5/2010
// ---------------------------------------------------------------------------
bool osCopyFile(const osFilePath& sourcePath, const osFilePath& destinationPath, bool allowOverwrite)
{
    bool retVal = false;

    // If the source file exists (and is not a directory, etc):
    if (sourcePath.isRegularFile())
    {
        // Copy using the Windows API:
        BOOL failIfExists = allowOverwrite ? FALSE : TRUE;
        const wchar_t* source = sourcePath.asString().asCharArray();
        const wchar_t* dest = destinationPath.asString().asCharArray();
        BOOL retCode = ::CopyFile(source, dest, failIfExists);

        if (retCode == TRUE)
        {
            // Make sure the file now exists:
            GT_IF_WITH_ASSERT(destinationPath.isRegularFile())
            {
                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFile::hide
// Description: Makes the file invisible to the user in the file system explorer
// Return Val:  bool - Success / Failure
// Author:      Amit Ben-Moshe
// Date:        5/1/2016
// ---------------------------------------------------------------------------
bool osFile::hide()
{
    bool ret = false;

    if (_filePath.exists())
    {
        BOOL rc = ::SetFileAttributes(_filePath.asString().asCharArray(), FILE_ATTRIBUTE_HIDDEN);
        ret = (rc == TRUE);
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        osFile::getSize
// Description: Returns the file size.
// Arguments:   fileSize - Output file size
// Return Val:  bool - Success / Failure
// Author:      AMD Developer Tools Team
// Date:        31/8/2005
// ---------------------------------------------------------------------------
bool osFile::getSize(unsigned long& fileSize) const
{
    bool retVal = false;
    fileSize = 0;

    // Verify that the file exists on disk:
    if (_filePath.isRegularFile())
    {
        // Open the file for reading:
        DWORD minimalDesiredAccessRights = 0;
        HANDLE hFile = ::CreateFile(_filePath.asString().asCharArray(),
                                    minimalDesiredAccessRights,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL, OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL, NULL);

        // If we managed to open the file:
        if (hFile != INVALID_HANDLE_VALUE)
        {
            // Get the file size:
            fileSize = ::GetFileSize(hFile, NULL);

            // Close the opened file:
            BOOL rc = ::CloseHandle(hFile);

            if (rc == 0)
            {
                GT_ASSERT_EX(0, L"Failed to close the file handle");
            }
            else
            {
                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osFile::deleteFile
// Description: Deletes the file from disk.
// Author:      AMD Developer Tools Team
// Date:        04/11/2006
// ---------------------------------------------------------------------------
bool osFile::deleteFile()
{
    bool retVal = false;

    // Delete the file from disk:
    int rc = _wunlink(_filePath.asString().asCharArray());

    if (rc == 0)
    {
        retVal = true;
    }

    return retVal;
}

bool osFile::IsExecutable() const
{
    DWORD binaryType = DWORD(-1);
    bool result = FALSE!= GetBinaryType(_filePath.asString().asCharArray(), &binaryType);
    return result;

}