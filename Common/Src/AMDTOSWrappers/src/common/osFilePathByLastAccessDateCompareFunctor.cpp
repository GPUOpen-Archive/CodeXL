//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osFilePathByLastAccessDateCompareFunctor.cpp
///
//=====================================================================

//------------------------------ osFilePathByLastModifiedDateCompareFunctor.cpp ------------------------------

// Standard C:
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osFilePathByLastAccessDateCompareFunctor.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// ---------------------------------------------------------------------------
// Name:        bool osFilePathByLastModifiedDateCompareFunctor::operator ()(const osFilePath& file1, const osFilePath& file2)
// Description: Compares files by the last modification date
// Arguments:
//              file1, file2 - the two filePaths to compare
//Return Value:
//              Returns true iff the last access date of file1 is more recent than that of file2
// Author:      AMD Developer Tools Team
// Date:        2007/12/24
// ---------------------------------------------------------------------------
bool osFilePathByLastModifiedDateCompareFunctor::operator()(const osFilePath& file1, const osFilePath& file2)
{
    // The struct that hold the files information
    osStatStructure file1Properties, file2Properties;

    // Get the filenames
    gtString file1Name = file1.asString();
    gtString file2Name = file2.asString();

    // Get the files status
    int rc1 = osWStat(file1Name, file1Properties);
    int rc2 = osWStat(file2Name, file2Properties);

    GT_ASSERT(rc1 == 0 && rc2 == 0);

    // Extract the last modified date from the properties struct
    // lastModifiedFile1Time and lastModifiedFile2Time hold the number of seconds that passed since
    // January 1, 1970 until the last modification moment.
    // Notice: Do not use access time, since on Vista, access time is hardly ever changed.
    time_t lastModifiedFile1Time = file1Properties.st_mtime;
    time_t lastModifiedFile2Time = file2Properties.st_mtime;

    // If the number of seconds since January 1st, 1970 to the last access moment is bigger,
    // then the last access date is newer
    return (lastModifiedFile1Time > lastModifiedFile2Time);
}

