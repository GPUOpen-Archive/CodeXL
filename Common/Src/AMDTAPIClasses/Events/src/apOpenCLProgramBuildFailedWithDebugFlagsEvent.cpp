//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLProgramBuildFailedWithDebugFlagsEvent.cpp
///
//==================================================================================

//------------------------------ apOpenCLProgramBuildFailedWithDebugFlagsEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramBuildFailedWithDebugFlagsEvent.h>

// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramBuildFailedWithDebugFlagsEvent::apOpenCLProgramBuildFailedWithDebugFlagsEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        14/11/2011
// ---------------------------------------------------------------------------
apOpenCLProgramBuildFailedWithDebugFlagsEvent::apOpenCLProgramBuildFailedWithDebugFlagsEvent(osThreadId triggeringThreadId, int contextIndex, int programIndex, cl_int buildErrorCode)
    : apEvent(triggeringThreadId), _contextIndex(contextIndex), _programIndex(programIndex), _buildErrorCode(buildErrorCode)
{
}

// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramBuildFailedWithDebugFlagsEvent::apOpenCLProgramBuildFailedWithDebugFlagsEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        14/11/2011
// ---------------------------------------------------------------------------
apOpenCLProgramBuildFailedWithDebugFlagsEvent::apOpenCLProgramBuildFailedWithDebugFlagsEvent()
    : _contextIndex(-1), _programIndex(-1), _buildErrorCode(CL_SUCCESS)
{
}

// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramBuildFailedWithDebugFlagsEvent::~apOpenCLProgramBuildFailedWithDebugFlagsEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        14/11/2011
// ---------------------------------------------------------------------------
apOpenCLProgramBuildFailedWithDebugFlagsEvent::~apOpenCLProgramBuildFailedWithDebugFlagsEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramBuildFailedWithDebugFlagsEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        14/11/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apOpenCLProgramBuildFailedWithDebugFlagsEvent::type() const
{
    return OS_TOBJ_ID_CL_PROGRAM_BUILD_FAILED_WITH_DEBUG_FLAGS_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramBuildFailedWithDebugFlagsEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/11/2011
// ---------------------------------------------------------------------------
bool apOpenCLProgramBuildFailedWithDebugFlagsEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the OpenCL Server context id:
    ipcChannel << (gtInt32)_contextIndex;

    // Write the OpenCL Server program index:
    ipcChannel << (gtInt32)_programIndex;

    // Write the build ended flag:
    ipcChannel << (gtInt64)_buildErrorCode;

    // Call my parent class's version of this function:
    retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramBuildFailedWithDebugFlagsEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/11/2011
// ---------------------------------------------------------------------------
bool apOpenCLProgramBuildFailedWithDebugFlagsEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the OpenCL Server context id:
    gtInt32 contextIndexAsInt32 = 0;
    ipcChannel >> contextIndexAsInt32;
    _contextIndex = (int)contextIndexAsInt32;

    // Read the OpenCL program index:
    gtInt32 programIndexAsInt32 = 0;
    ipcChannel >> programIndexAsInt32;
    _programIndex = (int)programIndexAsInt32;

    // Write the build ended flag:
    gtInt64 buildErrorCodeAsInt64 = 0;
    ipcChannel >> buildErrorCodeAsInt64;
    _buildErrorCode = (cl_int)buildErrorCodeAsInt64;

    // Call my parent class's version of this function:
    retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramBuildFailedWithDebugFlagsEvent::eventType
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        14/11/2011
// ---------------------------------------------------------------------------
apEvent::EventType apOpenCLProgramBuildFailedWithDebugFlagsEvent::eventType() const
{
    return apEvent::AP_OPENCL_PROGRAM_BUILD_FAILED_WITH_DEBUG_FLAGS_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramBuildFailedWithDebugFlagsEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        14/11/2011
// ---------------------------------------------------------------------------
apEvent* apOpenCLProgramBuildFailedWithDebugFlagsEvent::clone() const
{
    apOpenCLProgramBuildFailedWithDebugFlagsEvent* pEventCopy = new apOpenCLProgramBuildFailedWithDebugFlagsEvent(triggeringThreadId(), _contextIndex, _programIndex, _buildErrorCode);
    return pEventCopy;
}

