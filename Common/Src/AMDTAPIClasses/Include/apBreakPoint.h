//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apBreakPoint.h
///
//==================================================================================

//------------------------------ apBreakPoint.h ------------------------------

#ifndef __APBREAKPOINT
#define __APBREAKPOINT

// Infra:
#include <AMDTOSWrappers/Include/osTransferableObject.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apBreakPoint
// General Description:
//   Represents a breakpoint - A defined function / condition. When the debugged
//   application "hits" a breakpoint its execution stops, and a breakpoint event is
//   triggered.
// Author:  AMD Developer Tools Team
// Creation Date:        8/4/2004
// ----------------------------------------------------------------------------------
class AP_API apBreakPoint : public osTransferableObject
{
public:
    enum State
    {
        BREAKPOINT_STATE_ENABLED,               // Breakpoint is enabled, the debugger will break execution if it reaches it

        BREAKPOINT_STATE_TEMPORARY,             // Breakpoint is temporary, the debugger will break execution if it reaches it
        // and then delete the breakpoint. Used for 'run to cursor'.

        BREAKPOINT_STATE_DISABLED,              // Breakpoint is disabled, the debugger will ignore it

        BREAKPOINT_STATE_TEMPORARILY_DISABLED   // Breakpoint is temporarily disabled, the debugger will ignore it until it
        // completes a step-in operation and then it will re-enable the bp.
        // Used when user requested to step into a kernel and ignore breakpoints until
        // the debugged kernel execution begins.


    };

    apBreakPoint();
    virtual ~apBreakPoint();

    /// Returns true iff otherBreakpoint is an identical breakpoint.
    virtual bool compareToOther(const apBreakPoint& otherBreakpoint) const = 0;

    // Breakpoint state (Enabled / disabled / Temporarily disabled / Temporary:
    bool isEnabled() const { return (m_state == BREAKPOINT_STATE_ENABLED || m_state == BREAKPOINT_STATE_TEMPORARY); };
    bool isTemporarilyDisabled() const { return (m_state == BREAKPOINT_STATE_TEMPORARILY_DISABLED); };
    void setEnableStatus(bool isEnabled) { m_state = isEnabled ? BREAKPOINT_STATE_ENABLED : BREAKPOINT_STATE_DISABLED; };
    State state() const { return m_state; }
    void setState(const State newState) { m_state = newState; }

    // Hit count:
    int hitCount() const { return m_hitCount; };
    void incrementHitCount() { m_hitCount ++ ; };
    void setHitCount(int hitCount) { m_hitCount = hitCount; };

    // Overrides osTransferableObject:
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:

    // The current state of the breakpoint
    State m_state;

    // The breakpoint hit count:
    int m_hitCount;
};


#endif  // __APBREAKPOINT
