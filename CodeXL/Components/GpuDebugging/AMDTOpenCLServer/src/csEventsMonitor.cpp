//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csEventsMonitor.cpp
///
//==================================================================================

//------------------------------ csEventsMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTAPIClasses/Include/apCLEvent.h>

// Local:
#include <src/csCommandQueueMonitor.h>
#include <src/csCommandQueuesMonitor.h>
#include <src/csEventsMonitor.h>
#include <src/csGlobalVariables.h>
#include <src/csOpenCLHandleMonitor.h>
#include <src/csOpenCLMonitor.h>


// ---------------------------------------------------------------------------
// Name:        csEventsMonitor::csEventsMonitor
// Description: Constructor
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
csEventsMonitor::csEventsMonitor(int controllingContextId)
    : m_context(controllingContextId)
{

}

// ---------------------------------------------------------------------------
// Name:        csEventsMonitor::~csEventsMonitor
// Description: Destrcutor
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
csEventsMonitor::~csEventsMonitor()
{
    osCriticalSectionLocker vectorAccessCSLocker(m_eventsVectorAccessCS);
    m_events.deleteElementsAndClear();
}

// ---------------------------------------------------------------------------
// Name:        csEventsMonitor::onEventCreated
// Description: Called when an event is created without a queue. Creates an apCLEvent representation
//              for it.
// Arguments: oaCLEventHandle hEvent - the event handle
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
void csEventsMonitor::onEventCreated(oaCLEventHandle hEvent)
{
    gtAutoPtr<apCLEnqueuedCommand> aptrDummyCmd;
    onEventCreated(hEvent, OA_CL_NULL_HANDLE, aptrDummyCmd);
}

// ---------------------------------------------------------------------------
// Name:        csEventsMonitor::onEventCreated
// Description: Called when an event is created. Creates an apCLEvent representation
//              for it.
// Arguments: oaCLEventHandle hEvent - the event handle
//            oaCLCommandQueueHandle hQueue - the queue handle, or OA_CL_NULL_HANDLE for user events
//            apCLEnqueuedCommand* pEnqueuedCommand - optional - the apCLEnqueuedCommand (events manager takes ownership)
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
void csEventsMonitor::onEventCreated(oaCLEventHandle hEvent, oaCLCommandQueueHandle hQueue, gtAutoPtr<apCLEnqueuedCommand>& aptrEnqueuedCommand)
{
    osCriticalSectionLocker vectorAccessCSLocker(m_eventsVectorAccessCS);

    // Event overflow parameters:
    bool wasEventsOverflow = false;
    gtString eventOverflowError;
    int eventOverflowContext = -1;

    // Sanity check:
    GT_IF_WITH_ASSERT(OA_CL_NULL_HANDLE != hEvent)
    {
        // Refresh the vector:
        checkForReleasedEvents();

        // Make sure the event does not yet exist:
        const apCLEvent* pExistingEvent = eventDetails(hEvent);
        GT_IF_WITH_ASSERT(NULL == pExistingEvent)
        {
            // Create the event object:
            apCLEvent* pNewEvent = new apCLEvent(hEvent, hQueue, false);

            // Get its index:
            int newEventIndex = (int)m_events.size();

            // Add it to the vector:
            m_events.push_back(pNewEvent);

            // Register the event handle:
            csOpenCLHandleMonitor& handlesMonitor = cs_stat_openCLMonitorInstance.openCLHandleMonitor();
            int queueId = -1;

            if (OA_CL_NULL_HANDLE != hQueue)
            {
                apCLObjectID* pQueueHandleInfo = handlesMonitor.getCLHandleObjectDetails(hQueue);
                GT_IF_WITH_ASSERT(NULL != pQueueHandleInfo)
                {
                    GT_IF_WITH_ASSERT((m_context == pQueueHandleInfo->_contextId) && (OS_TOBJ_ID_CL_COMMAND_QUEUE == pQueueHandleInfo->_objectType))
                    {
                        queueId = pQueueHandleInfo->_objectId;

                        // Get the context to get the queue monitor:
                        csContextMonitor* pContext = cs_stat_openCLMonitorInstance.clContextMonitor(m_context);
                        GT_IF_WITH_ASSERT(NULL != pContext)
                        {
                            // Add the event to the queue:
                            csCommandQueueMonitor* pQueue = pContext->commandQueuesMonitor().commandQueueMonitor(queueId);
                            GT_IF_WITH_ASSERT(NULL != pQueue)
                            {
                                pQueue->addEventToQueue(hEvent, false, false, wasEventsOverflow, eventOverflowError, eventOverflowContext);
                            }
                        }
                    }
                }
            }

            handlesMonitor.registerOpenCLHandle((oaCLHandle)hEvent, m_context, newEventIndex, OS_TOBJ_ID_CL_EVENT, queueId, newEventIndex + 1);
        }
    }

    // If we have an enqueued command object:
    if (NULL != aptrEnqueuedCommand.pointedObject())
    {
        // We currently do not use profiling information in the debugger - so we can destroy the command object.
        aptrEnqueuedCommand = NULL;
    }

    // If we got an event overflow, we must exit the critical section before reporting it, since it might trigger a breakpoint:
    if (wasEventsOverflow)
    {
        // Release the CS:
        vectorAccessCSLocker.leaveCriticalSection();

        // Report the error:
        cs_stat_openCLMonitorInstance.reportDetectedError(eventOverflowContext, AP_QUEUE_EVENTS_OVERFLOW, eventOverflowError);
    }
}

// ---------------------------------------------------------------------------
// Name:        csEventsMonitor::onEventMarkedForDeletion
// Description: Called when an event is about to be deleted (its only reference is the
//              1 we added on creation)
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
void csEventsMonitor::onEventMarkedForDeletion(oaCLEventHandle hEvent)
{
    osCriticalSectionLocker vectorAccessCSLocker(m_eventsVectorAccessCS);

    // Find the event in our vector:
    int numberOfEvents = (int)m_events.size();
    csOpenCLHandleMonitor& handlesMonitor = cs_stat_openCLMonitorInstance.openCLHandleMonitor();

    apCLEvent* pFoundEvent = NULL;

    for (int i = 0; i < numberOfEvents; i++)
    {
        if (NULL == pFoundEvent)
        {
            // Get the current event object:
            apCLEvent* pCurrentEvent = m_events[i];
            GT_IF_WITH_ASSERT(NULL != pCurrentEvent)
            {
                // If this is the event we're looking for:
                if (hEvent == pCurrentEvent->eventHandle())
                {
                    // Mark that we found it:
                    pFoundEvent = pCurrentEvent;
                    m_events[i] = NULL;
                }
            }
        }
        else // NULL != pFoundEvent
        {
            // We already found the event, so i must be >= 1. Shift the current
            // pointer one place back (so we can pop_back at the end):
            m_events[i - 1] = m_events[i];

            // Notify the handles monitor of this change:
            oaCLEventHandle movedEventHandle = OA_CL_NULL_HANDLE;
            int eventContextID = -1;
            int queueIndex = -1;
            apCLEvent* pMovedEvent = m_events[i - 1];
            GT_IF_WITH_ASSERT(NULL != pMovedEvent)
            {
                movedEventHandle = pMovedEvent->eventHandle();
                apCLObjectID* pEventHandleDetails = handlesMonitor.getCLHandleObjectDetails(movedEventHandle);
                GT_IF_WITH_ASSERT(NULL != pEventHandleDetails)
                {
                    GT_IF_WITH_ASSERT(OS_TOBJ_ID_CL_EVENT == pEventHandleDetails->_objectType)
                    {
                        eventContextID = pEventHandleDetails->_contextId;
                        queueIndex = pEventHandleDetails->_ownerObjectId;
                    }
                }
            }

            handlesMonitor.registerOpenCLHandle(movedEventHandle, eventContextID, i - 1, OS_TOBJ_ID_CL_EVENT, queueIndex, i);
        }
    }

    GT_IF_WITH_ASSERT(NULL != pFoundEvent)
    {
        // Notify the handles monitor of this change:
        oaCLEventHandle removedEventHandle = OA_CL_NULL_HANDLE;
        int eventContextID = -1;
        int queueIndex = -1;

        removedEventHandle = pFoundEvent->eventHandle();
        GT_ASSERT(hEvent == removedEventHandle);
        apCLObjectID* pEventHandleDetails = handlesMonitor.getCLHandleObjectDetails(removedEventHandle);
        GT_IF_WITH_ASSERT(NULL != pEventHandleDetails)
        {
            GT_IF_WITH_ASSERT(OS_TOBJ_ID_CL_EVENT == pEventHandleDetails->_objectType)
            {
                eventContextID = pEventHandleDetails->_contextId;
                queueIndex = pEventHandleDetails->_ownerObjectId;
            }
        }

        // Delete the event object:
        delete pFoundEvent;

        handlesMonitor.registerOpenCLHandle(removedEventHandle, eventContextID, -1, OS_TOBJ_ID_CL_EVENT, queueIndex);

        // Update the containing queue, if it exists:
        if (-1 < queueIndex)
        {
            // Get the context to get the queue monitor:
            csContextMonitor* pContext = cs_stat_openCLMonitorInstance.clContextMonitor(m_context);
            GT_IF_WITH_ASSERT(NULL != pContext)
            {
                // Add the event to the queue:
                csCommandQueueMonitor* pQueue = pContext->commandQueuesMonitor().commandQueueMonitor(queueIndex);
                GT_IF_WITH_ASSERT(NULL != pQueue)
                {
                    pQueue->removeEventFromQueue(removedEventHandle);
                }
            }
        }

        // The last item in the vector will now be the found event or a duplicate pointer.
        // Remove it either way:
        m_events.pop_back();
    }
}

// ---------------------------------------------------------------------------
// Name:        csEventsMonitor::checkForReleasedEvents
// Description: Checks if any of the objects monitored by this class have been released
// Author:      Uri Shomroni
// Date:        26/8/2013
// ---------------------------------------------------------------------------
void csEventsMonitor::checkForReleasedEvents()
{
    // Collect the object handles:
    osCriticalSectionLocker eventsVectorAccessCSLocker(m_eventsVectorAccessCS);
    gtVector<oaCLEventHandle> eventHandles;
    int numberOfEvents = (int)m_events.size();

    for (int i = 0; i < numberOfEvents; i++)
    {
        const apCLEvent* pEvent = m_events[i];
        GT_IF_WITH_ASSERT(NULL != pEvent)
        {
            eventHandles.push_back(pEvent->eventHandle());
        }
    }

    // Check each one. This is done separately, since finding an object
    // that was marked for deletion will release it:
    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
    int eventsFound = (int)eventHandles.size();

    for (int i = 0; i < eventsFound; i++)
    {
        theOpenCLMonitor.checkIfEventWasDeleted((cl_event)eventHandles[i], false);
    }
}

// ---------------------------------------------------------------------------
// Name:        csEventsMonitor::eventDetails
// Description: Gets the apCLEvent handle describing an event
// Arguments: oaCLEventHandle hEvent - the event handle
// Return Val:  const apCLEvent* - the event descriptor, or NULL on failure
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
const apCLEvent* csEventsMonitor::eventDetails(oaCLEventHandle hEvent) const
{
    osCriticalSectionLocker vectorAccessCSLocker((osCriticalSection&)m_eventsVectorAccessCS);

    const apCLEvent* retVal = NULL;

    // Find the event in our vector:
    int numberOfEvents = (int)m_events.size();

    for (int i = 0; i < numberOfEvents; i++)
    {
        // Get the current event object:
        const apCLEvent* pCurrentEvent = m_events[i];
        GT_IF_WITH_ASSERT(NULL != pCurrentEvent)
        {
            // If this is the event we're looking for:
            if (hEvent == pCurrentEvent->eventHandle())
            {
                // Return it:
                retVal = pCurrentEvent;
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csEventsMonitor::eventDetails
// Description: Gets the apCLEvent handle describing an event
// Arguments: int eventIndex - the event index in the vector
// Return Val:  const apCLEvent* - the event descriptor, or NULL on failure
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
const apCLEvent* csEventsMonitor::eventDetailsByIndex(int eventIndex) const
{
    const apCLEvent* retVal = NULL;

    GT_IF_WITH_ASSERT((-1 < eventIndex) && (amountOfEvents() > eventIndex))
    {
        retVal = m_events[eventIndex];
    }

    return retVal;
}


