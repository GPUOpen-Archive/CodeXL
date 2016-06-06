//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apExecutionModeChangedEvent.h
///
//==================================================================================

//------------------------------ apExecutionModeChangedEvent.h ------------------------------

#ifndef __APEXECUTIONMODECHANGEDEVENT_H
#define __APEXECUTIONMODECHANGEDEVENT_H


// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apExecutionModeChangedEvent : public apEvent
// General Description: The class is used when CodeXL needs to output a message for
//                      the user in the output window
// Author:  AMD Developer Tools Team
// Creation Date:       30/10/2011
// ----------------------------------------------------------------------------------
class AP_API apExecutionModeChangedEvent : public apEvent
{
public:

    apExecutionModeChangedEvent(const gtString& modeName, const int sessionTypeIndex, bool updateOnlySessionTypeIndex = false);
    apExecutionModeChangedEvent(const gtString& modeName, const gtString& sessionTypeName);
    apExecutionModeChangedEvent(const gtString& modeName, const gtString& sessionTypeName, const int sessionTypeIndex);
    apExecutionModeChangedEvent(const gtString& modeName, const gtString& sessionTypeName, const int sessionTypeIndex, bool updateOnlySessionTypeIndex);

    virtual ~apExecutionModeChangedEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    // Self functions:
    const gtString& modeType() const { return m_modeName; };
    const gtString& sessionTypeName() const { return m_sessionTypeName; };
    int sessionTypeIndex() const { return m_sessionTypeIndex; };
    bool onlySessionTypeIndex() const { return m_updateOnlySessionTypeIndex; };

private:

    friend class osTransferableObjectCreator<apExecutionModeChangedEvent>;

    // Do not allow the use of the default constructor:
    apExecutionModeChangedEvent();

private:
    // The outputted warning string:
    gtString m_modeName;
    gtString m_sessionTypeName;
    int m_sessionTypeIndex;
    bool m_updateOnlySessionTypeIndex;
};


#endif //__apExecutionModeChangedEvent_H

