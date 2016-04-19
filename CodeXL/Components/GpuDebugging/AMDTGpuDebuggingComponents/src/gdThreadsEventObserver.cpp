//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdThreadsEventObserver.cpp
///
//==================================================================================

//------------------------------ gdThreadsEventObserver.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunSuspendedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdThreadsEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>


// Static members initializations:
unsigned int gdThreadsEventObserver::_amountOfDebuggedApplicationThreads = 0;
gtMap<osThreadId, unsigned int> gdThreadsEventObserver::_OSThreadIdToInternalID;
gdThreadsEventObserver* gdThreadsEventObserver::m_spMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        gdThreadsEventObserver::gdThreadsEventObserver
// Description: Constructor
// Author:      Uri Shomroni
// Date:        12/5/2008
// ---------------------------------------------------------------------------
gdThreadsEventObserver::gdThreadsEventObserver() :
    _debuggedProcessExists(false), _isDebuggedProcessSuspended(false)
{
    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
}


// ---------------------------------------------------------------------------
// Name:        gdThreadsEventObserver::~gdThreadsEventObserver
// Description: Destructor
// Author:      Uri Shomroni
// Date:        12/5/2008
// ---------------------------------------------------------------------------
gdThreadsEventObserver::~gdThreadsEventObserver()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}


// ---------------------------------------------------------------------------
// Name:        gdPropertiesEventObserver::instance
// Description: Returns the singleton instance of this class
// Return Val:  gdPropertiesEventObserver&
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
gdThreadsEventObserver& gdThreadsEventObserver::instance()
{
    if (NULL == m_spMySingleInstance)
    {
        m_spMySingleInstance = new gdThreadsEventObserver;

    }

    return *m_spMySingleInstance;
}



// ---------------------------------------------------------------------------
// Name:        gdThreadsEventObserver::onEvent
// Description: Is called when a debugged process event occurs.
// Arguments:   eve - A class representing the event.
// Author:      Uri Shomroni
// Date:        12/5/2008
// ---------------------------------------------------------------------------
void gdThreadsEventObserver::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent);  // unused
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    switch (eventType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        {
            _debuggedProcessExists = true;
            _isDebuggedProcessSuspended = false;
            clearThreadIDsVector();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
        {
            _debuggedProcessExists = false;
            _isDebuggedProcessSuspended = false;
            clearThreadIDsVector();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        {
            _isDebuggedProcessSuspended = false;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            _isDebuggedProcessSuspended = true;
        }
        break;

        default:
        {
            // Unhandled events.
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdThreadsEventObserver::getThreadInternalID
// Description: Gets the thread's internal ID or gives it the next available one
//              if it's not yet registered
// Arguments: threadId - the normal (external) thread ID / handle
// Return Val: unsigned int - the internal ID (or the new internal ID assigned)
// Author:      Uri Shomroni
// Date:        11/5/2008
// ---------------------------------------------------------------------------
unsigned int gdThreadsEventObserver::getThreadInternalID(osThreadId threadId)
{
    unsigned int retVal;
    gtMap<osThreadId, unsigned int>::const_iterator iter = _OSThreadIdToInternalID.find(threadId);

    if (iter == _OSThreadIdToInternalID.end())
    {
        _OSThreadIdToInternalID[threadId] = _amountOfDebuggedApplicationThreads;
        retVal = _amountOfDebuggedApplicationThreads;
        _amountOfDebuggedApplicationThreads++;
    }
    else
    {
        retVal = (*iter).second;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdThreadsEventObserver::getThreadIDFromInternalID
// Description: Gets the thread's ID from its internal ID.
// Arguments: internalID - the thread's internal ID (0 should be the main thread)
// Return Val: osThreadId - the thread's ID on the OS.
// Author:      Uri Shomroni
// Date:        11/5/2008
// ---------------------------------------------------------------------------
osThreadId gdThreadsEventObserver::getThreadIDFromInternalID(unsigned int internalID)
{
    osThreadId retVal = OS_NO_THREAD_ID;

    // Sanity check:
    GT_IF_WITH_ASSERT(internalID < _amountOfDebuggedApplicationThreads)
    {
        bool goOn = true;
        unsigned int i = 0;

        while (goOn)
        {
            gtMap<osThreadId, unsigned int>::const_iterator iter = _OSThreadIdToInternalID.lower_bound(i);

            if (iter != _OSThreadIdToInternalID.end())
            {
                if ((*iter).second == internalID)
                {
                    retVal = (*iter).first;
                    goOn = false;
                }

                i = (*iter).first + 1;
            }
            else
            {
                goOn = false;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdThreadsEventObserver::clearThreadIDsVector
// Description: Clears the Internal IDs table.
// Author:      Uri Shomroni
// Date:        11/5/2008
// ---------------------------------------------------------------------------
void gdThreadsEventObserver::clearThreadIDsVector()
{
    _OSThreadIdToInternalID.clear();
    _amountOfDebuggedApplicationThreads = 0;
}

