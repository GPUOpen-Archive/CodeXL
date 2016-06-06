//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLProgramBuildFailedWithDebugFlagsEvent.h
///
//==================================================================================

//------------------------------ apOpenCLProgramBuildFailedWithDebugFlagsEvent.h ------------------------------

#ifndef __APOPENCLPROGRAMBUILDFAILEDWITHDEBUGFLAGSEVENT_H
#define __APOPENCLPROGRAMBUILDFAILEDWITHDEBUGFLAGSEVENT_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Infra:
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apOpenCLProgramBuildFailedWithDebugFlagsEvent : public apEvent
// General Description: Thrown when a program build fails due to the addition of debug flags
// Author:  AMD Developer Tools Team
// Creation Date:       14/11/2011
// ----------------------------------------------------------------------------------
class AP_API apOpenCLProgramBuildFailedWithDebugFlagsEvent : public apEvent
{
public:
    apOpenCLProgramBuildFailedWithDebugFlagsEvent(osThreadId triggeringThreadId, int contextIndex, int programIndex, cl_int buildErrorCode);
    virtual ~apOpenCLProgramBuildFailedWithDebugFlagsEvent();

    int contextIndex() const {return _contextIndex;};
    int programIndex() const {return _programIndex;};
    cl_int buildErrorCode() const {return _buildErrorCode;};

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    // Only the transferable object creator should be able to call my default constructor:
    friend class osTransferableObjectCreator<apOpenCLProgramBuildFailedWithDebugFlagsEvent>;
    apOpenCLProgramBuildFailedWithDebugFlagsEvent();

private:
    // The OpenCL context id:
    int _contextIndex;

    // The OpenCL program index:
    int _programIndex;

    // The error code we got when trying to build with the flags:
    cl_int _buildErrorCode;
};


#endif //__APOPENCLPROGRAMBUILDFAILEDWITHDEBUGFLAGSEVENT_H

