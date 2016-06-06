//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osFileLauncher.cpp
///
//=====================================================================

//------------------------------ osFileLauncher.cpp ------------------------------
// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTOSWrappers/Include/osFileLauncher.h>


// ---------------------------------------------------------------------------
// Name:        osFileLauncher::osFileLauncher
// Description: Constructor.
// Arguments: fileToBeLaunched - The file path / URL to be launched.
//            launchFileInDifferentThread - If true - the file will be launched
//                                          in a different thread, not causing the current
//                                          thread to wait for the file "Open" application
//                                          to be launched. This option is ignored on Linux.
// Author:      AMD Developer Tools Team
// Date:        29/7/2004
// ---------------------------------------------------------------------------
osFileLauncher::osFileLauncher(const gtString& fileToBeLaunched, bool launchFileInDifferentThread)
    : _fileToBeLaunched(fileToBeLaunched), _launchFileInDifferentThread(launchFileInDifferentThread)
{
    // Yaki - 15/8/2007:
    // On Linux and Mac, launching a browser in a thread different than the main GUI thread
    // sometimes causes a crash. Therefore, we always launch the browser in the current
    // thread.
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    _launchFileInDifferentThread = false;
#endif
}

// ---------------------------------------------------------------------------
// Name:        osFileLauncher::osFileLauncher
// Description: Constructor.
// Arguments: fileToBeLaunched - The file path / URL to be launched.
//              commandLineParameters - command line arguments
//            launchFileInDifferentThread - If true - the file will be launched
//                                          in a different thread, not causing the current
//                                          thread to wait for the file "Open" application
//                                          to be launched. This option is ignored on Linux.
// Author:      AMD Developer Tools Team
// Date:        29/7/2004
// ---------------------------------------------------------------------------
osFileLauncher::osFileLauncher(const gtString& fileToBeLaunched, const gtString& commandLineParameters, bool launchFileInDifferentThread)
    : _fileToBeLaunched(fileToBeLaunched), _commandLineParameters(commandLineParameters), _launchFileInDifferentThread(launchFileInDifferentThread)
{
    // Yaki - 15/8/2007:
    // On Linux and Mac, launching a browser in a thread different than the main GUI thread
    // sometimes causes a crash. Therefore, we always launch the browser in the current
    // thread.
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    _launchFileInDifferentThread = false;
#endif
}

