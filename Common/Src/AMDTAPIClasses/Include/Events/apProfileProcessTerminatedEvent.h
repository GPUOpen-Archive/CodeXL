//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apProfileProcessTerminatedEvent.h
///
//==================================================================================

//------------------------------ apProfileProcessTerminatedEvent.h ------------------------------
#ifndef _APPROFILEPROCESSTERMINATEDEVENT_H
#define _APPROFILEPROCESSTERMINATEDEVENT_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Infra:
#include <AMDTOSWrappers/Include/osTime.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apProfileProcessTerminatedEvent
// General Description:
//   Represents the event of the profiled process termination.
// Author:  AMD Developer Tools Team
// Creation Date:        5/23/2012
// ----------------------------------------------------------------------------------
class AP_API apProfileProcessTerminatedEvent : public apEvent
{
public:
    /// Constructor
    /// \param profilerName the name of the profiler plugin
    /// \param the process exit code
    /// \param profileType the type of the profile process or -1 if not used
    apProfileProcessTerminatedEvent(const gtString& profilerName, long exitCode, int profileType = -1);
    virtual ~apProfileProcessTerminatedEvent();

    /// Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    /// Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    /// Self functions:
    const gtString& profilerName() const { return m_profilerName; };
    long processExitCode() const { return m_exitCode; };
    const osTime& processTerminationTime() const { return m_processTerminationTime; };
    int ProfileType() const { return m_profileType; }

private:
    friend class osTransferableObjectCreator<apProfileProcessTerminatedEvent>;

    /// Do not allow the use of the default constructor:
    apProfileProcessTerminatedEvent();

private:
    /// The profiler that owns the process being terminated:
    gtString m_profilerName;

    /// The process exit code:
    long m_exitCode;

    /// The process exit time:
    osTime m_processTerminationTime;

    /// En enumeration used for the profile type by the plugin:
    int m_profileType;
};

#endif //_APPROFILEPROCESSTERMINATEDEVENT_H
