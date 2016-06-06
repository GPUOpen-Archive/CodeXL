//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osDesktop.cpp
///
//=====================================================================

//------------------------------ osDesktop.cpp ------------------------------

// Standard C:
#include <stdlib.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDesktop.h>
#include <AMDTOSWrappers/Include/osProcess.h>


// ---------------------------------------------------------------------------
// Name:        osGetDesktopName
// Description:
//  Retrieves the type of the desktop on which this
//  application runs.
//
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        20/2/2007
// ---------------------------------------------------------------------------
bool osGetDesktopType(osDesktopType& desktopType)
{
    bool retVal = false;

    gtString envVariableName(L"GDMSESSION");
    gtString envVarValue;
    bool rc1 = osGetCurrentProcessEnvVariableValue(envVariableName, envVarValue);

    GT_IF_WITH_ASSERT(rc1)
    {
        gtString vendorLowerCase = envVarValue;
        vendorLowerCase.toLowerCase();

        if (vendorLowerCase.find(L"gnome") != -1)
        {
            // This is a GNOME desktop:
            desktopType = OS_GNOME_DESKTOP;
            retVal = true;
        }
        else if (vendorLowerCase.find(L"kde") != -1)
        {
            // This is a KDE desktop:
            desktopType = OS_KDE_DESKTOP;
            retVal = true;
        }
        else
        {
            // Report the name of the vendor which we do not support:
            gtString errString = envVarValue;
            errString.prepend(L"Unknown Linux session manager: ");
            OS_OUTPUT_DEBUG_LOG(errString.asCharArray(), OS_DEBUG_LOG_DEBUG);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osDesktopTypeToString
// Description: Inputs a desktop type and returns its name as a string.
// Arguments: desktopType - The input desktop type.
//            desktoptTypeAsString - The output string.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        7/5/2008
// ---------------------------------------------------------------------------
bool osDesktopTypeToString(const osDesktopType& desktopType, gtString& desktoptTypeAsString)
{
    bool retVal = true;
    desktoptTypeAsString.makeEmpty();

    switch (desktopType)
    {
        case OS_KDE_DESKTOP:
            desktoptTypeAsString = L"KDE";
            break;

        case OS_GNOME_DESKTOP:
            desktoptTypeAsString = L"GNOME";
            break;

        default:
        {
            // Unknown desktop type:
            GT_ASSERT(false);
            retVal = false;
        }
        break;
    }

    return retVal;
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
    // Currently implemented as a no-op for Linux.
    (void)(docPath);  // unused
}