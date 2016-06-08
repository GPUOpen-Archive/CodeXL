//------------------------------ osFile.cpp ------------------------------

// POSIX:
#include <sys/stat.h>
#include <unistd.h>
#include <copyfile.h>

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
// Author:      Uri Shomroni
// Date:        2/5/2010
// ---------------------------------------------------------------------------
bool osCopyFile(const osFilePath& sourcePath, const osFilePath& destinationPath, bool allowOverwrite)
{
    bool retVal = false;

    // If the source file exists (and is not a directory, etc):
    if (sourcePath.isRegularFile())
    {
        // Copy using the Unix copyfile API:
        copyfile_flags_t copyFlags = COPYFILE_ALL;

        if (!allowOverwrite)
        {
            copyFlags |= COPYFILE_EXCL;
        }

        int retCode = ::copyfile(sourcePath.asString().asCharArray(), destinationPath.asString().asCharArray(), NULL, copyFlags);

        if (retCode == 0)
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
// Name:        osFile::deleteFile
// Description: Deletes the file from disk.
// Author:      Yaki Tebeka
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
