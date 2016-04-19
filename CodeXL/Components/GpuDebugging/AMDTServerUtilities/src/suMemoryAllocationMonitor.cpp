//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suMemoryAllocationMonitor.cpp
///
//==================================================================================

//------------------------------ suMemoryAllocationMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osCallsStackReader.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/Events/apMemoryAllocationFailureEvent.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// Local:
#include <AMDTServerUtilities/Include/suMemoryAllocationMonitor.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>

// Static members initializations:
suMemoryAllocationMonitor* suMemoryAllocationMonitor::m_spMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        suMemoryAllocationMonitor::instance
// Description: Returns the single instance of the suMemoryAllocationMonitor class.
//              Creates it the first time this function is called.
// Author:      Gilad Yarnitzky
// Date:        6/1/2015
// ---------------------------------------------------------------------------
suMemoryAllocationMonitor& suMemoryAllocationMonitor::instance()
{
    if (m_spMySingleInstance == NULL)
    {
        m_spMySingleInstance = new suMemoryAllocationMonitor;
        std::set_new_handler(HandlerMemoryAllocationFailure);
    }

    return *m_spMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        suMemoryAllocationMonitor::suMemoryAllocationMonitor
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        6/1/2015
// ---------------------------------------------------------------------------
suMemoryAllocationMonitor::suMemoryAllocationMonitor()
{

}

// ---------------------------------------------------------------------------
// Name:        suMemoryAllocationMonitor::~suMemoryAllocationMonitor
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        6/1/2015
// ---------------------------------------------------------------------------
suMemoryAllocationMonitor::~suMemoryAllocationMonitor()
{

}

// ---------------------------------------------------------------------------
void suMemoryAllocationMonitor::HandlerMemoryAllocationFailure()
{
    gtFreeReservedMemory();

    OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_DebuggedApplicationRanOutOfMemory, OS_DEBUG_LOG_INFO);

    osCallsStackReader csReader;
    osCallStack csBuffer;
    bool isOk = csReader.getCurrentCallsStack(csBuffer, true, true);
    GT_ASSERT(isOk);

    // Report that process has run out of memory:
    apMemoryAllocationFailureEvent memoryAllocationFailedEvent(csBuffer);
    bool rcEve = suForwardEventToClient(memoryAllocationFailedEvent);
    GT_ASSERT(rcEve);

    // Exit.
    osExitCurrentProcess(0xFFFFFFFFU);
}
