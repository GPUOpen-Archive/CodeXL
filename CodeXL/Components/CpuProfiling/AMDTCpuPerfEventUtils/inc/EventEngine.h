//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file EventEngine.h
/// \brief Interface for the EventEngine class.
///
//==================================================================================

#ifndef _EVENTENGINE_H_
#define _EVENTENGINE_H_

#include <AMDTOSWrappers/Include/osDirectory.h>

#include "EventsFile.h"
#include "IbsEvents.h"

// PMC
#define PMC_BASE 0
#define PMC_MAX 0xFFE

// L2I
#define L2I_BASE 0xD000
#define L2I_MAX 0xDFFF

// CodeXL defines timer event as 0xFFF.
#define TIMER_CXL_PRD 0xFFF

// PERF SW events for timer is encoded as 0xE100.
// Therefore we have two number which can represent timer, when importing
// We are unifying CXL with the PERF timer event.
#define TIMER_PERF 0xE100


class CP_EVENT_API EventEngine
{
public:
    EventEngine() = default;
    virtual ~EventEngine() = default;

    bool Initialize(const osDirectory& eventsDirectory);
    osFilePath GetEventFilePath(gtUInt32 cpuFamily, gtUInt32 cpuModel);
    EventsFile* GetEventFile(gtUInt32 cpuFamily, gtUInt32 cpuModel);

private:
    // Directory where the event files reside
    osDirectory m_eventFileDirectory;
};

//Is the event value the arbitrary timer event
inline bool IsTimerEvent(gtUInt16 event) { return event == TIMER_CXL_PRD || event == TIMER_PERF; }
//Is the event value a hardware pmc event
inline bool IsPmcEvent(gtUInt16 event) { return /* event >= PMC_BASE && */ event <= PMC_MAX; }
//Is the event value a hardware L2I event
inline bool IsL2IEvent(gtUInt16 event) { return event >= L2I_BASE && event <= L2I_MAX; }
//Is the event value any of the IBS fetch derived events
inline bool IsIbsFetchEvent(gtUInt16 event) { return event >= IBS_FETCH_BASE && event <= IBS_FETCH_MAX; }
//Is the event value any of the IBS ops derived events
inline bool IsIbsOpEvent(gtUInt16 event) { return event >= IBS_OP_BASE && event <= IBS_OP_MAX; }
//Is the event value any of the IBS CLU events
inline bool IsIbsCluEvent(gtUInt16 event) { return event >= IBS_CLU_BASE && event <= IBS_CLU_MAX; }
//Get the arbitrary timer event value
inline gtUInt16 GetTimerEvent() { return TIMER_PERF; }
//Get the first IBS fetch event value
inline gtUInt16 GetIbsFetchEvent() { return IBS_FETCH_BASE; }
//Get the first IBS ops event value
inline gtUInt16 GetIbsOpEvent() { return IBS_OP_BASE; }
//Get the first IBS CLU event value
inline gtUInt16 GetIbsCluEvent() { return IBS_CLU_BASE; }

#endif // _EVENTENGINE_H_
