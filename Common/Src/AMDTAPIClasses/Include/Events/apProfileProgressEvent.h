//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apProfileProgressEvent.h
///
//==================================================================================
#ifndef _APPROFILEPROGRESSEVENT_H
#define _APPROFILEPROGRESSEVENT_H

//infrastructure
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apProfileProgressEvent
// General Description: This event is sent when the profiling is handling long time
//                      operations and wants to update for progress
// Author:  AMD Developer Tools Team
// Creation Date:       5/29/2012
// ----------------------------------------------------------------------------------
class AP_API apProfileProgressEvent : public apEvent
{
public:
    apProfileProgressEvent(const gtString& profilerName, const gtString& progress, const int value);
    ~apProfileProgressEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    const gtString& profileName() const {return m_profilerName;};
    void setProfileName(const gtString& profilerName) {m_profilerName = profilerName;}

    const gtString& progress() const {return m_progress;};
    void setProgress(const gtString& progress) {m_progress = progress;}

    int value() const {return m_value;};
    void setValue(const int value) {m_value = value;}

    //If aborted(), the progress string is the abort message
    bool aborted() const {return m_aborted;};
    void setAborted(const bool aborted) {m_aborted = aborted;}

    bool increment() const { return m_increment; };
    void setIncrement(const bool increment) { m_increment = increment; }

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    gtString m_profilerName;
    gtString m_progress;
    int m_value;
    bool m_aborted;
    bool m_increment;
};

#endif //_APPROFILEPROGRESSEVENT_H
