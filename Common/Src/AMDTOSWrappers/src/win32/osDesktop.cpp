//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osDesktop.cpp
///
//=====================================================================

//------------------------------ osDesktop.cpp ------------------------------

// Windows
#include <Shlobj.h>

// Standard C:
#include <stdlib.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osDesktop.h>



// ---------------------------------------------------------------------------
// Name:        osGetDesktopName
// Description:
//              Retrieves the type of the desktop on which this
//              application runs.
//              Currently not supported on Windows.
//
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        20/2/2007
// ---------------------------------------------------------------------------
bool osGetDesktopType(osDesktopType& desktopType)
{
    (void)(desktopType); // unused
    GT_ASSERT(false);
    return false;
}


// ---------------------------------------------------------------------------
// Name:        osDesktopTypeToString
// Description: Inputs a desktop type and returns its name as a string.
//              Currently not supported on Windows.
// Arguments: desktopType - The input desktop type.
//            desktoptTypeAsString - The output string.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        7/5/2008
// ---------------------------------------------------------------------------
bool osDesktopTypeToString(const osDesktopType& desktopType, gtString& desktoptTypeAsString)
{
    (void)(desktopType); // unused
    (void)(desktoptTypeAsString); // unused
    GT_ASSERT(false);
    return false;
}

// ---------------------------------------------------------------------------
// Name:        osAddToRecentDocs
// Description: Notifies the operating system that an item has been accessed,
//              for the purposes of tracking those items used most recently and
//              most frequently. On Windows these items are added to the
//              Start Menu's list of recent documents and to the application's
//              jump-list on the task bar.
//              Currently implemented as a no-op for Linux.
// Arguments:   docPath [INPUT] - The path to the document that will be added to the recent docs lists
// Return Val:  none.
// Author:      AMD Developer Tools Team
// Date:        Dec-25, 2013
// ---------------------------------------------------------------------------
OS_API void osAddToRecentDocs(const gtString& docPath)
{
    // Pass the complete path of the document file in unicode format to the Windows API
    SHAddToRecentDocs(SHARD_PATHW, docPath.asCharArray());
}