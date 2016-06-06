//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaDisplay.cpp
///
//=====================================================================

// X11
#include <X11/Xlib.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osMachine.h>

// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineAmountOfMonitors
// Description: returns the number of monitors on the current machine
// Return Val: int
// Author:      AMD Developer Tools Team
// Date:        21/5/2008
// ---------------------------------------------------------------------------
int oaGetLocalMachineAmountOfMonitors()
{
    Display* dpy = XOpenDisplay(NULL);
    bool retVal = ScreenCount(dpy);
    GT_ASSERT(XCloseDisplay(dpy) == Success);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetDisplayMonitorInfo
// Description: Get the Primary display device name
// Arguments:   deviceName - the device name goes here
//              monitorName - the monitor name goes here
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/5/2008
// ---------------------------------------------------------------------------
bool oaGetDisplayMonitorInfo(gtString& deviceName, gtString& monitorName)
{
    Display* dpy = XOpenDisplay(NULL);
    bool retVal = (dpy != NULL);

    if (retVal)
    {
        deviceName.makeEmpty();
        deviceName.appendFormattedString(L"%d", DefaultScreen(dpy));
        monitorName.fromASCIIString(DisplayString(dpy));
        retVal = (XCloseDisplay(dpy) == Success);
        GT_ASSERT(retVal);
    }

    // TO_DO: not sure this is correct
    return retVal;
}

