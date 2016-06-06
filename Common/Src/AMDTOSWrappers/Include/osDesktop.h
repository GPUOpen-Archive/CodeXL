//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osDesktop.h
///
//=====================================================================

//------------------------------ osDesktop.h ------------------------------

#ifndef __OSDESKTOP_H
#define __OSDESKTOP_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>

// Lists supported desktop types:
enum osDesktopType
{
    OS_KDE_DESKTOP,
    OS_GNOME_DESKTOP
};


OS_API bool osGetDesktopType(osDesktopType& desktopType);
OS_API bool osDesktopTypeToString(const osDesktopType& desktopType, gtString& desktoptTypeAsString);

// Notifies the operating system that an item has been accessed, for the purposes of tracking those items used
// most recently and most frequently.
// On Windows these items are added to the Start Menu's list of recent documents and to the application's jump-
// list on the task bar.
// Currently implemented as a no-op for Linux.
OS_API void osAddToRecentDocs(const gtString& docPath);


#endif  // __OSDESKTOP_H
