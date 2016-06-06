//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessDetectedErrorEvent.h
///
//==================================================================================

//------------------------------ apDebuggedProcessDetectedErrorEvent.h ------------------------------

#ifndef __APDEBUGGEDPROCESSDETECTEDERROREVENT_H
#define __APDEBUGGEDPROCESSDETECTEDERROREVENT_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Infra:
#include <AMDTAPIClasses/Include/apErrorCode.h>
#include <AMDTAPIClasses/Include/apDetectedErrorParameters.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           PD_API apDebuggedProcessDetectedErrorEvent : public apEvent
// General Description:
//   Is triggered when the debugged process generates an error.
// Author:  AMD Developer Tools Team
// Creation Date:        21/3/2005
// ----------------------------------------------------------------------------------
class AP_API apDebuggedProcessDetectedErrorEvent : public apEvent
{
public:
    apDebuggedProcessDetectedErrorEvent(osThreadId triggeringThreadId, const apDetectedErrorParameters& detectedErrorParameters, bool wasGeneratedByBreak);
    virtual ~apDebuggedProcessDetectedErrorEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    const apDetectedErrorParameters& detectedErrorParameters() const { return _detectedErrorParameters; };
    bool wasGeneratedByBreak() const {return _wasGeneratedByBreak;};

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    friend class osTransferableObjectCreator<apDebuggedProcessDetectedErrorEvent>;

    // Do not allow the use of the default constructor:
    apDebuggedProcessDetectedErrorEvent();

private:
    // Contains the detected error parameters:
    apDetectedErrorParameters _detectedErrorParameters;

    // Was this received as a breakpoint or as a report?
    bool _wasGeneratedByBreak;
};


#endif //__APDEBUGGEDPROCESSDETECTEDERROREVENT_H

