//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osConsole.cpp
///
//=====================================================================
#include <AMDTOSWrappers/Include/osConsole.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <conio.h>

// ---------------------------------------------------------------------------
// Name:        osWaitForKeyPress
// Description: Blocks until the user hits a keyboard key
// Arguments:   None
// Author:      AMD Developer Tools Team
// Date:        2/2/2014
// ---------------------------------------------------------------------------
void OS_API osWaitForKeyPress()
{
    const unsigned SLEEP_INTERVAL_MS = 50;

    while (!kbhit())
    {
        osSleep(SLEEP_INTERVAL_MS);
    }
}

