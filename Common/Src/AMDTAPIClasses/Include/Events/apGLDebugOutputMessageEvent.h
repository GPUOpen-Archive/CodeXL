//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLDebugOutputMessageEvent.h
///
//==================================================================================

//------------------------------ apGLDebugOutputMessageEvent.h ------------------------------

#ifndef __APGLDEBUGOUTPUTMESSAGEEVENT_H
#define __APGLDEBUGOUTPUTMESSAGEEVENT_H

// Infra:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTBaseTools/Include/gtString.h>

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apGLDebugOutputMessageEvent
// General Description: Is thrown when a debug output message is received
// Author:  AMD Developer Tools Team
// Creation Date:       7/6/2010
// ----------------------------------------------------------------------------------
class AP_API apGLDebugOutputMessageEvent : public apEvent
{
public:
    apGLDebugOutputMessageEvent(osThreadId triggeringThreadId, const gtString& debugOutputSource, const gtString& debugOutputType, const gtString& debugOutputSeverity, const gtString& debugOutputMessageContent, int contextId, int messageId);
    virtual ~apGLDebugOutputMessageEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    const gtString& debugOutputSource() const { return m_debugOutputSource; };
    const gtString& debugOutputType() const { return m_debugOutputType; };
    const gtString& debugOutputSeverity() const { return m_debugOutputSeverity; };
    const gtString& debugOutputMessageContent() const { return m_debugOutputMessageContent; };
    int contextID() const { return m_contextId; };
    int messageID() const { return m_messageId; };

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    // Only the transferable object creator should be able to call my default constructor:
    friend class osTransferableObjectCreator<apGLDebugOutputMessageEvent>;
    apGLDebugOutputMessageEvent();

private:
    int m_contextId;
    int m_messageId;
    gtString m_debugOutputSource;
    gtString m_debugOutputType;
    gtString m_debugOutputSeverity;
    gtString m_debugOutputMessageContent;
};


#endif //__APGLDEBUGOUTPUTMESSAGEEVENT_H

