//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apMonitoredFunctionBreakPoint.h
///
//==================================================================================

//------------------------------ apMonitoredFunctionBreakPoint.h ------------------------------

#ifndef __APMONITOREDFUNCTIONBREAKPOINT
#define __APMONITOREDFUNCTIONBREAKPOINT

// Local:
#include <AMDTAPIClasses/Include/apBreakPoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>


// ----------------------------------------------------------------------------------
// Class Name:           IP_API apMonitoredFunctionBreakPoint
// General Description:
//   Represents a monitored function breakpoint. When the debugged process calls
//   this function its execution stops, and a breakpoint event is triggered.
// Author:  AMD Developer Tools Team
// Creation Date:        8/4/2004
// ----------------------------------------------------------------------------------
class AP_API apMonitoredFunctionBreakPoint : public apBreakPoint
{
public:
    apMonitoredFunctionBreakPoint(apMonitoredFunctionId monitoredFunctionId = apMonitoredFunctionsAmount)
        : _monitoredFunctionId(monitoredFunctionId) {};
    virtual ~apMonitoredFunctionBreakPoint() {};

    // Overrides apBreakPoint:
    virtual bool compareToOther(const apBreakPoint& otherBreakpoint) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    void setMonitoredFunctionId(apMonitoredFunctionId functionId) {_monitoredFunctionId = functionId;};
    apMonitoredFunctionId monitoredFunctionId() const {return _monitoredFunctionId;};

private:
    // The monitored function id:
    apMonitoredFunctionId _monitoredFunctionId;
};


#endif  // __APMONITOREDFUNCTIONBREAKPOINT
