//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This worker thread is responsible for detecting rendering stalls and
/// returning the stalled return code to the client
//==============================================================================

#include "RenderStallThread.h"
#include "../Common/GraphicsServerState.h"
#include "../Common/SharedGlobal.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Checks to see if the app has stopped rendering (or ever rendered at all).
/// The result is stored in g_bRenderLoopStalled and it is used when incoming messages are processed.
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderStallThread::CheckForRenderStall()
{
    static double lastPresentTime = 0.0f;

    for (;;)    // loop forever
    {
        //int stallCount = 0;
        int delay = 0;

        if (GetServerShutdownState() == true)
        {
            return;
        }

        // Get the last value that the server wrote into the slot. It may never have written to the slot.
        double checkTime = SG_GET_DOUBLE(LastPresentTime);
        bool bForceStall = SG_GET_BOOL(ForceRenderStallState);

        // Until the server actually renders lastPresentTime will always be 0
        if (lastPresentTime == 0.0f)
        {
            lastPresentTime = checkTime;
        }

        // Get the diff between renders
        double diffTime = checkTime - lastPresentTime;

        // record the last time so that our diffs are correct next time around
        lastPresentTime = checkTime;

        if (bForceStall == true)
        {
            SetServerStalledState(true);
            Log(logMESSAGE, "Keypress capture is blocking client commands.\n");
        }
        else
        {
            // If there is no diff then there was no rendering so we have stalled.
            if (diffTime == 0.0f)
            {
                // The server has stopped rendering or has never rendered
                //Log(logERROR, "Render Loop Stalled diffTime: %lf\n", diffTime);
                if (delay > GRAPHICS_SERVER_STATUS_STALL_THRESHOLD_TIME)
                {
                    SetServerStalledState(true);
                }

                delay += GRAPHICS_SERVER_STATUS_STALL_LOOP_SLEEP_TIME;
            }
            else
            {
                SetServerStalledState(false);
                delay = 0;
            }
        }

        // Wait a reasonable time. 10ms is too short as it is below a games 16ms typical update and will lead to diffs of zero (a stall condition).
        osSleep(GRAPHICS_SERVER_STATUS_STALL_LOOP_SLEEP_TIME);
    }
}