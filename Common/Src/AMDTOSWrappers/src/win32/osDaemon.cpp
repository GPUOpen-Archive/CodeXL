//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osDaemon.cpp
///
//=====================================================================

//------------------------------ osDaemon.cpp ------------------------------

// Local:
#include <AMDTOSWrappers/Include/osDaemon.h>


// ---------------------------------------------------------------------------
// Name:        osMakeThisProcessDaemon
// Description: Performs actions required to make this process a "well behaved"
//              daemon (background server).
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        13/4/2008
// ---------------------------------------------------------------------------
bool osMakeThisProcessDaemon()
{
    // This function is not yet implemented on Windows.
    return true;
}

