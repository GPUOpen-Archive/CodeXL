//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaSpyConnectionWaiterThread.cpp
///
//==================================================================================

//------------------------------ gaSpyConnectionWaiterThread.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apApiConnectionEstablishedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>

// Local:
#include <src/gaSpyConnectionWaiterThread.h>


// ---------------------------------------------------------------------------
// Name:        gaSpyConnectionWaiterThread::gaSpyConnectionWaiterThread
// Description: Constructor.
// Arguments: APIConnectionType - The type of the API connection to which this thread waits for.
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
gaSpyConnectionWaiterThread::gaSpyConnectionWaiterThread(apAPIConnectionType APIConnectionType)
    : osThread(L"gaSpyConnectionWaiterThread"), _APIConnectionType(APIConnectionType)
{
}


// ---------------------------------------------------------------------------
// Name:        gaSpyConnectionWaiterThread::~gaSpyConnectionWaiterThread
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
gaSpyConnectionWaiterThread::~gaSpyConnectionWaiterThread()
{
    // If the OS thread is still running - terminate it:
    if (isAlive())
    {
        terminate();
    }
}


// ---------------------------------------------------------------------------
// Name:        gaSpyConnectionWaiterThread::triggerAPIConnectionEstablishedEvent
// Description: Triggers an "API connection established" event.
// Author:      Yaki Tebeka
// Date:        25/8/2005
// ---------------------------------------------------------------------------
void gaSpyConnectionWaiterThread::triggerAPIConnectionEstablishedEvent()
{
    // Trigger an "API Connection established" event:
    apApiConnectionEstablishedEvent apiConnectionEstablishedEvent(_APIConnectionType);
    apEventsHandler::instance().registerPendingDebugEvent(apiConnectionEstablishedEvent);
}

