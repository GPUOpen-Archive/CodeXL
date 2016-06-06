//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLProgramBuildEvent.cpp
///
//==================================================================================

//------------------------------ apOpenCLProgramBuildEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramBuildEvent.h>

// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramBuildEvent::apOpenCLProgramBuildEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        13/9/2011
// ---------------------------------------------------------------------------
apOpenCLProgramBuildEvent::apOpenCLProgramBuildEvent(int contextID, int programIndex, gtString programName, const gtVector<apCLProgram::programBuildData>& programBuildDatas, bool buildEnded)
    : apEvent(0), _contextID(contextID), _programIndex(programIndex), _programName(programName), _wasBuildEnded(buildEnded)
{
    // Copy the program build data:
    for (int i = 0 ; i < (int)programBuildDatas.size(); i++)
    {
        // Get the current build data:
        apCLProgram::programBuildData buildData = programBuildDatas[i];

        // Copy the build data:
        apCLProgram::programBuildData copiedBuildData(buildData._buildStatus, buildData._buildLog, buildData._buildOptions, buildData.m_buildGlobalVariablesTotalSize);
        _programDevicesBuildData.push_back(copiedBuildData);
    }
}



// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramBuildEvent::~apOpenCLProgramBuildEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        3/7/2011
// ---------------------------------------------------------------------------
apOpenCLProgramBuildEvent::~apOpenCLProgramBuildEvent()
{
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramBuildEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        3/7/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apOpenCLProgramBuildEvent::type() const
{
    return OS_TOBJ_ID_CL_PROGRAM_BUILD_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramBuildEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        3/7/2011
// ---------------------------------------------------------------------------
bool apOpenCLProgramBuildEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;

    // Write the OpenCL Server context id:
    ipcChannel << (gtInt32)_contextID;

    // Write the OpenCL program index:
    ipcChannel << (gtInt32)_programIndex;

    // Write the OpenCL Server program index:
    ipcChannel << _programName;

    // Write the build ended flag:
    ipcChannel << _wasBuildEnded;

    // Write the program devices:
    gtUInt32 amountOfDevices = (gtUInt32)_programDevicesBuildData.size();
    ipcChannel << amountOfDevices;

    for (gtUInt32 i = 0; i < amountOfDevices; i++)
    {
        const apCLProgram::programBuildData& currentBuildData = _programDevicesBuildData[i];
        ipcChannel << currentBuildData._buildLog;
        ipcChannel << currentBuildData._buildOptions;
        ipcChannel << (gtInt32)currentBuildData._buildStatus;
        ipcChannel << currentBuildData.m_buildGlobalVariablesTotalSize;
    }

    // Call my parent class's version of this function:
    retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramBuildEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        3/7/2011
// ---------------------------------------------------------------------------
bool apOpenCLProgramBuildEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    // Read the OpenCL program index:
    gtInt32 varAsInt32 = 0;
    ipcChannel >> varAsInt32;
    _contextID = (int)varAsInt32;

    ipcChannel >> varAsInt32;
    _programIndex = (int)varAsInt32;

    // Read the OpenCL program name:
    ipcChannel >> _programName;

    // Write the build ended flag:
    ipcChannel >> _wasBuildEnded;

    // Read the program devices and the build data for each:
    gtUInt32 amountOfDevices = 0;
    ipcChannel >> amountOfDevices;

    for (gtUInt32 i = 0; i < amountOfDevices; i++)
    {
        apCLProgram::programBuildData currentBuildData;
        ipcChannel >> currentBuildData._buildLog;
        ipcChannel >> currentBuildData._buildOptions;
        gtInt32 currentBuildStatusAsInt32 = (gtInt32)CL_BUILD_NONE;
        ipcChannel >> currentBuildStatusAsInt32;
        currentBuildData._buildStatus = (cl_build_status)currentBuildStatusAsInt32;
        ipcChannel >> currentBuildData.m_buildGlobalVariablesTotalSize;
        _programDevicesBuildData.push_back(currentBuildData);
    }

    // Call my parent class's version of this function:
    retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramBuildEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        3/7/2011
// ---------------------------------------------------------------------------
apEvent::EventType apOpenCLProgramBuildEvent::eventType() const
{
    return apEvent::AP_OPENCL_PROGRAM_BUILD_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramBuildEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        3/7/2011
// ---------------------------------------------------------------------------
apEvent* apOpenCLProgramBuildEvent::clone() const
{
    apOpenCLProgramBuildEvent* pEventCopy = new apOpenCLProgramBuildEvent(_contextID, _programIndex, _programName, _programDevicesBuildData, _wasBuildEnded);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramBuildEvent::apOpenCLProgramBuildEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        3/7/2011
// ---------------------------------------------------------------------------
apOpenCLProgramBuildEvent::apOpenCLProgramBuildEvent()
    : _contextID(0), _programName(L"")
{
}

