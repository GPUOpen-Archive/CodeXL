//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLProgramDeletedEvent.h
///
//==================================================================================

//------------------------------ apOpenCLProgramDeletedEvent.h ------------------------------

#ifndef __APOPENCLPROGRAMDELETEDEVENT_H
#define __APOPENCLPROGRAMDELETEDEVENT_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>
#include <AMDTOSWrappers/Include/osFilePath.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apOpenCLProgramDeletedEvent
// General Description:
//   Is thrown when an OpenCL program is deleted.
// Author:  AMD Developer Tools Team
// Creation Date:        1/5/2010
// ----------------------------------------------------------------------------------
class AP_API apOpenCLProgramDeletedEvent : public apEvent
{
public:
    apOpenCLProgramDeletedEvent(osThreadId triggeringThreadId, int contextID, int programIndex, const osFilePath& programSourceFilePath);
    virtual ~apOpenCLProgramDeletedEvent();

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
    friend class osTransferableObjectCreator<apOpenCLProgramDeletedEvent>;
    apOpenCLProgramDeletedEvent();

private:
    // The OpenCL context id:
    int _contextID;

    // The OpenCL Program index:
    int _programIndex;

    // The program source file path:
    osFilePath _programSourceFilePath;

};


#endif //__APOPENCLPROGRAMDELETEDEVENT_H

