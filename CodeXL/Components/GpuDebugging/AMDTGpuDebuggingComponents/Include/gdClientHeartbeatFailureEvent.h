//------------------------------ gdClientHeartbeatFailureEvent.h ------------------------------

#ifndef __GDCLIENTHEARTBEATFAILUREEVENT_H
#define __GDCLIENTHEARTBEATFAILUREEVENT_H

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdCodeXLAppDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdClientHeartbeatFailureEvent : public wxEvent
// General Description:
//   A wxWindows event that is fired when the client heartbeat (to the license server)
//   fails.
// Author:               Yaki Tebeka
// Creation Date:        30/10/2006
// ----------------------------------------------------------------------------------
class GD_API gdClientHeartbeatFailureEvent : public wxEvent
{
public:
    gdClientHeartbeatFailureEvent(int hearbeatFailureCount);
    int hearbeatFailureCount() const { return _hearbeatFailureCount; };

    // Overrides wxEvent:
    virtual wxEvent* Clone() const;

private:
    // Do not allow the use of my default constructor:
    gdClientHeartbeatFailureEvent();

private:
    // The amount of successive heartbeat failures:
    int _hearbeatFailureCount;
};

// This event type:
GD_API extern wxEventType gdEVT_CLIENT_HEARTBEAT_FAILURE_EVENT;

// Event handler method definition:
typedef void (wxEvtHandler::*gdClientHeartbeatFailureEventHandler)(gdClientHeartbeatFailureEvent&);

// Macro for creating wxWindows event table entry:
#define EVT_GD_CLIENT_HEARTBEAT_FAILURE_EVENT(func) wxEventTableEntry(gdEVT_CLIENT_HEARTBEAT_FAILURE_EVENT, -1, -1, wxNewEventTableFunctor(gdEVT_CLIENT_HEARTBEAT_FAILURE_EVENT, (wxObjectEventFunction)(wxEventFunction)(gdClientHeartbeatFailureEventHandler)& func), (wxObject*) NULL ),




#endif //__GDCLIENTHEARTBEATFAILUREEVENT_H
