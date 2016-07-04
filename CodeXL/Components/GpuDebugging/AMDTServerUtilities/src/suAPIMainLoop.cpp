//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suAPIMainLoop.cpp
///
//==================================================================================

//------------------------------ suAPIMainLoop.cpp ------------------------------

// POSIX:
#include <limits.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>

// Local:
#include <src/suSpyToAPIConnector.h>
#include <AMDTServerUtilities/Include/suAPIMainLoop.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// The size of gtInt32:
static const int stat_int32Size = sizeof(gtInt32);


// ---------------------------------------------------------------------------
// Name:        suAPIMainLoop
// Description:
//   This is the API main loop. It is executed by the API calls handling thread,
//   listens to the API socket and responds to received API requests.
//
// Arguments: apiSocket - The API connecting socket.
//
// Author:      Yaki Tebeka
// Date:        30/4/2007
// ---------------------------------------------------------------------------
void suAPIMainLoop(osSocket& apiSocket)
{
    // True iff we should continue listening to API requests:
    bool goOn = true;

    do
    {
        // Wait infinitely until we have an input:
        apiSocket.setReadOperationTimeOut(OS_CHANNEL_INFINITE_TIME_OUT);

        // If needed, wait for a direct function execution to end:
        suWaitForDirectFunctionExecutionEnd();

        // Wait for an API function call and handle it:
        gtInt32 executedFunctionId = 0;
        suHandleSingleAPICall(apiSocket, executedFunctionId);

        // Handle calls that terminate the API loop:
        suHandleAPILoopTerminatingCalls((apAPIFunctionId)executedFunctionId, goOn);

        // If we are during debugged process termination:
        bool isDuringTermination = suIsInProcessTerminationAPILoop();

        if (isDuringTermination)
        {
            // Handle the API loop behavior during this phase:
            suHandleAPILoopBehaviourDuringDebuggedProcessTermination((apAPIFunctionId)executedFunctionId, goOn);
        }

    }
    while (goOn);
}


// ---------------------------------------------------------------------------
// Name:        suProcessSingleAPICall
// Description:
//   Is called by the debugger directly to process a single API function call
//   who's arguments already reside in the API socket.
//
// Return Val: int - 1 - success.
//                 - 0 - failure.
//
// Author:      Yaki Tebeka
// Date:        30/4/2007
// ---------------------------------------------------------------------------
int suProcessSingleAPICall()
{
    int retVal = 0;

    // Get the API socket:
    suSpyToAPIConnector& theSpyToAPIConnector = suSpyToAPIConnector::instance();
    osSocket* pAPISocket = theSpyToAPIConnector.apiSocket();
    GT_IF_WITH_ASSERT(pAPISocket != NULL)
    {
        // Process the API function that resides in the socket:
        gtInt32 executedFunctionId = 0;
        retVal = suHandleSingleAPICall(*pAPISocket, executedFunctionId);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suHandleSingleAPICall
// Description:
//   Handles a single API function call who's arguments already reside in the API socket.
//
// Arguments: apiSocket - The API socket.
//            executedFunctionId - The executed API function id.
//
// Return Val: int - 1 - success.
//                 - 0 - failure.
//
// Author:      Yaki Tebeka
// Date:        30/4/2007
// ---------------------------------------------------------------------------
int suHandleSingleAPICall(osSocket& apiSocket, gtInt32& executedFunctionId)
{
    int retVal = 0;

    // Read the called API function id:
    executedFunctionId = 0;
    bool rc = apiSocket.read((gtByte*)&executedFunctionId, stat_int32Size);

    if (rc)
    {
        // Call the called API function stub:
        suExecuteAPIFunctionStub((apAPIFunctionId)executedFunctionId, apiSocket);

        retVal = 1;
    }

    return retVal;
}
