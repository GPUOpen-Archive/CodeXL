//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osConsole.cpp
///
//=====================================================================
#include <AMDTOSWrappers/Include/osConsole.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

// ---------------------------------------------------------------------------
// Name:        osKbhitLinuxImpl
// Description: This is a helper function: implementing kbhit() on Linux
// Arguments:   None
// Author:      AMD Developer Tools Team
// Date:        2/2/2014
// ---------------------------------------------------------------------------
int osKbhitLinuxImpl()
{
    int ret = 0;
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        ret = 1;
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        osWaitForKeyPress
// Description: Blocks the console until the user hits a keyboard key
// Arguments:   None
// Author:      AMD Developer Tools Team
// Date:        2/2/2014
// ---------------------------------------------------------------------------
void osWaitForKeyPress()
{
    const unsigned SLEEP_INTERVAL_MS = 50;

    // If we are running in background mode, there is no console, therefore we
    // should block forever (until the user, who decided to run us in background
    // mode, will kill us). Otherwise, we actually wait for a keyboard hit.
    bool isBackground = (getpgrp() != tcgetpgrp(STDOUT_FILENO));

    while (isBackground || !osKbhitLinuxImpl())
    {
        osSleep(SLEEP_INTERVAL_MS);
    }
}
