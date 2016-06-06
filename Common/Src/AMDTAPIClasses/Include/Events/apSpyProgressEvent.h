//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apSpyProgressEvent.h
///
//==================================================================================

//------------------------------ apSpyProgressEvent.h ------------------------------

#ifndef __APSPYPROGRESSEVENT_H
#define __APSPYPROGRESSEVENT_H

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           apUpdateContextSteps
// General Description:  Is used to update the 'gaUpdateContextDataSnapshot' progress
// Author:  AMD Developer Tools Team
// Creation Date:        12/8/2010
// ----------------------------------------------------------------------------------
enum apUpdateContextSteps
{
    AP_BEFORE_CONTEXT_MAKE_CONTEXT = 0,
    AP_AFTER_CONTEXT_MAKE_CONTEXT = 1,
    AP_AFTER_RENDER_CONTEXT_UPDATE = 2,
    AP_AFTER_TEXTURE_UNIT_MONITORS_UPDATE = 3,
    AP_AFTER_RENDER_BUFFERS_MONITOR_UPDATE = 4,
    AP_AFTER_STATIC_BUFFERS_MONITOR_UPDATE = 5,
    AP_AFTER_STATE_VARIABLES_UPDATE = 6,
    AP_AFTER_PROGRAM_AND_SHADERS_UPDATE = 7,
    AP_BEFORE_MAKE_NULL_CONTEXT_CURRENT = 8,
    AP_AFTER_MAKE_NULL_CONTEXT_CURRENT = 9,
    AP_LAST_UPDATE_CONTEXT_STEP = AP_AFTER_MAKE_NULL_CONTEXT_CURRENT
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apSpyProgressEvent
// General Description: This event is sent when the spy is handling long time operation
//                      and wants to update for progress
// Author:  AMD Developer Tools Team
// Creation Date:       29/7/2010
// ----------------------------------------------------------------------------------
class AP_API apSpyProgressEvent : public apEvent
{
public:
    apSpyProgressEvent(int progress = 1);
    ~apSpyProgressEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    int progress() const {return _progress;};
    void setProgress(int progress) {_progress = progress;}

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    int _progress;

};

#endif //__apSpyProgressEvent_H

