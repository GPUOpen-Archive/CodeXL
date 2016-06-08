//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLProgramCreatedEvent.h
///
//==================================================================================

//------------------------------ apOpenCLProgramCreatedEvent.h ------------------------------

#ifndef __APOPENCLPROGRAMCREATEDEVENT_H
#define __APOPENCLPROGRAMCREATEDEVENT_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apOpenCLProgramCreatedEvent
// General Description:
//   Is thrown when an OpenCL program is deleted.
// Author:  AMD Developer Tools Team
// Creation Date:        1/5/2010
// ----------------------------------------------------------------------------------
class AP_API apOpenCLProgramCreatedEvent : public apEvent
{
public:
    apOpenCLProgramCreatedEvent(osThreadId triggeringThreadId, int contextID, int programIndex, const osFilePath& programSourceFilePath);
    virtual ~apOpenCLProgramCreatedEvent();

    int contextID() const {return _contextID;};
    int programIndex() const {return _programIndex;};
    const osFilePath& programSourceFilePath() const {return _programSourceFilePath;};

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    // Only the transferable object creator should be able to call my default constructor:
    friend class osTransferableObjectCreator<apOpenCLProgramCreatedEvent>;
    apOpenCLProgramCreatedEvent();

private:

    // The OpenCL context id:
    int _contextID;

    // The OpenCL program index:
    int _programIndex;

    // The program source file path:
    osFilePath _programSourceFilePath;
};


#endif //__APOPENCLPROGRAMCREATEDEVENT_H

