//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOutputDebugStringEvent.h
///
//==================================================================================

//------------------------------ apOutputDebugStringEvent.h ------------------------------

#ifndef __APOUTPUTDEBUGSTRINGEVENT
#define __APOUTPUTDEBUGSTRINGEVENT

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apOutputDebugStringEvent
// General Description:
//   Represents the event of a debug string printed by the debugged process.
//   (Usually using the OutputDebugString Win32 API function).
// Author:  AMD Developer Tools Team
// Creation Date:       30/3/2004
// ----------------------------------------------------------------------------------
class AP_API apOutputDebugStringEvent : public apEvent
{
public:
    enum OutputWindowType
    {
        AP_EVENT_VIEW = 0,
        AP_GENERAL_OUTPUT_VIEW
    };

    apOutputDebugStringEvent(osThreadId triggeringThreadId, const gtString& debugString, const int targetView = AP_EVENT_VIEW);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    // Self functions:
    const gtString& debugString() const { return _debugString; };
    int targetView() const { return m_targetView; };

private:
    friend class osTransferableObjectCreator<apOutputDebugStringEvent>;

    // Do not allow the use of the default constructor:
    apOutputDebugStringEvent();

private:
    // The outputted debug string:
    gtString _debugString;

    // target output window
    int m_targetView;
};


#endif  // __APOUTPUTDEBUGSTRINGEVENT
