//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLProgramBuildEvent.h
///
//==================================================================================

//------------------------------ apOpenCLProgramBuildEvent.h ------------------------------

#ifndef __APOPENCLPROGRAMBUILDEVENT_H
#define __APOPENCLPROGRAMBUILDEVENT_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apOpenCLProgramBuildEvent
// General Description:  Is thrown when an OpenCL program is build.
// Author:  AMD Developer Tools Team
// Date:                 3/7/2011
// ----------------------------------------------------------------------------------
class AP_API apOpenCLProgramBuildEvent : public apEvent
{
public:
    apOpenCLProgramBuildEvent(int contextID, int programIndex, gtString programName, const gtVector<apCLProgram::programBuildData>& programBuildDatas, bool buildEnded);
    virtual ~apOpenCLProgramBuildEvent();

    int contextID() const {return _contextID;};
    int programIndex() const {return _programIndex;}
    gtString programName() const {return _programName;};
    bool wasBuildEnded() const {return _wasBuildEnded;};

    gtVector<apCLProgram::programBuildData>& devicesBuildData() {return _programDevicesBuildData;};
    const gtVector<apCLProgram::programBuildData>& devicesBuildData() const {return _programDevicesBuildData;};

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    // Only the transferable object creator should be able to call my default constructor:
    friend class osTransferableObjectCreator<apOpenCLProgramBuildEvent>;
    apOpenCLProgramBuildEvent();
private:
    // The OpenCL context id:
    int _contextID;

    // The OpenCL program index:
    int _programIndex;

    // The OpenCL program name:
    gtString _programName;

    // Was the build ended (we throw this event twice - before and after the build):
    bool _wasBuildEnded;

    // The build information for each device (this vector must be the same size and order
    // as the _programDevices vector):
    gtVector<apCLProgram::programBuildData> _programDevicesBuildData;

};


#endif //__APOPENCLPROGRAMBUILDEVENT_H

