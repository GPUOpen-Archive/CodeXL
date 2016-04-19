//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csEventsMonitor.h
///
//==================================================================================

//------------------------------ csEventsMonitor.h ------------------------------

#ifndef __CSEVENTSMONITOR_H
#define __CSEVENTSMONITOR_H

// Forward declarations:
class apCLEnqueuedCommand;
class csContextMonitor;

// Infra:
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/apCLEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:         csEventsMonitor
// General Description: Manages the monitoring and operation of OpenCL event objects
//                      (cl_event), created by command queues or the user.
// Author:             Uri Shomroni
// Creation Date:      22/8/2013
// ----------------------------------------------------------------------------------
class csEventsMonitor
{
public:
    csEventsMonitor(int controllingContext);
    ~csEventsMonitor();

    // Event creation and destruction:
    void onEventCreated(oaCLEventHandle hEvent);
    void onEventCreated(oaCLEventHandle hEvent, oaCLCommandQueueHandle hQueue, gtAutoPtr<apCLEnqueuedCommand>& aptrEnqueuedCommand);
    void onEventMarkedForDeletion(oaCLEventHandle hEvent);

    // Reference count checking:
    void checkForReleasedEvents();

    // Event info queries:
    int amountOfEvents() const {return (int)m_events.size();};
    const apCLEvent* eventDetails(oaCLEventHandle hEvent) const;
    const apCLEvent* eventDetailsByIndex(int eventIndex) const;

private:
    int m_context;

    // The events monitored:
    gtPtrVector<apCLEvent*> m_events;
    osCriticalSection m_eventsVectorAccessCS;
};



#endif //__CSEVENTSMONITOR_H

