//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCallStackFrameSelectedEvent.h
///
//==================================================================================

//------------------------------ apCallStackFrameSelectedEvent.h ------------------------------

#ifndef __APCALLSTACKFRAMESELECTEDEVENT_H
#define __APCALLSTACKFRAMESELECTEDEVENT_H


// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCallStackFrameSelectedEvent : public apEvent
// General Description: The class is used when CodeXL needs to output a message for
//                      the user in the output window
// Author:  AMD Developer Tools Team
// Creation Date:       7/19/2012
// ----------------------------------------------------------------------------------
class AP_API apCallStackFrameSelectedEvent : public apEvent
{
public:

    apCallStackFrameSelectedEvent(int frameIndex);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    // Self functions:
    int frameIndex() const { return m_frameIndex; };

private:

    friend class osTransferableObjectCreator<apCallStackFrameSelectedEvent>;

    // Do not allow the use of the default constructor:
    apCallStackFrameSelectedEvent();

private:
    // The outputted warning string:
    int m_frameIndex;
};


#endif //__APCALLSTACKFRAMESELECTEDEVENT_H

