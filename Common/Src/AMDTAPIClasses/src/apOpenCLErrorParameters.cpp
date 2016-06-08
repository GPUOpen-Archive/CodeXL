//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLErrorParameters.cpp
///
//==================================================================================

//------------------------------ apOpenCLErrorParameters.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>

// Local:
#include <AMDTAPIClasses/Include/apOpenCLErrorParameters.h>


// ---------------------------------------------------------------------------
// Name:        apOpenCLErrorParameters::apOpenCLErrorParameters
// Description: Copy constructor
// Author:  AMD Developer Tools Team
// Date:        22/2/2010
// ---------------------------------------------------------------------------
apOpenCLErrorParameters::apOpenCLErrorParameters(const apOpenCLErrorParameters& other)
{
    operator=(other);
}

// ---------------------------------------------------------------------------
// Name:        apOpenCLErrorParameters::apOpenCLErrorParameters
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        22/2/2010
// ---------------------------------------------------------------------------
apOpenCLErrorParameters::apOpenCLErrorParameters()
{
    clearParameters();
}

// ---------------------------------------------------------------------------
// Name:        apOpenCLErrorParameters::operator=
// Description: Assignment operator
// Author:  AMD Developer Tools Team
// Date:        22/2/2010
// ---------------------------------------------------------------------------
apOpenCLErrorParameters& apOpenCLErrorParameters::operator=(const apOpenCLErrorParameters& other)
{
    if (_aptrBreakedOnFunctionCall.pointedObject() != NULL)
    {
        delete _aptrBreakedOnFunctionCall.pointedObject();
        _aptrBreakedOnFunctionCall = NULL;
    }

    if (other._aptrBreakedOnFunctionCall.pointedObject() != NULL)
    {
        _aptrBreakedOnFunctionCall = (apFunctionCall*)(other._aptrBreakedOnFunctionCall->clone());
    }

    _openCLErrorCode = other._openCLErrorCode;
    return *this;
}

// ---------------------------------------------------------------------------
// Name:        apOpenCLErrorParameters::clearParameters
// Description: Clears the detected error parameters.
// Author:  AMD Developer Tools Team
// Date:        22/2/2010
// ---------------------------------------------------------------------------
void apOpenCLErrorParameters::clearParameters()
{
    _openCLErrorCode = -1;
    _aptrBreakedOnFunctionCall = NULL;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLErrorParameters::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        22/2/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apOpenCLErrorParameters::type() const
{
    return OS_TOBJ_ID_OPENCL_ERROR_PARAMS;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLErrorParameters::writeSelfIntoChannel
// Description: Writes this class content into a channel.
// Arguments: ipcChannel - The channel to which this class content will be written.
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        22/2/2010
// ---------------------------------------------------------------------------
bool apOpenCLErrorParameters::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    bool isFunctionCallPresent = (_aptrBreakedOnFunctionCall.pointedObject() != NULL);

    ipcChannel << isFunctionCallPresent;

    if (isFunctionCallPresent)
    {
        // Write the broken-on function:
        retVal = _aptrBreakedOnFunctionCall->writeSelfIntoChannel(ipcChannel);
    }

    // Write the OpenCL error code:
    ipcChannel << (gtUInt32)_openCLErrorCode;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLErrorParameters::readSelfFromChannel
// Description: Reads this class content from a channel.
// Arguments: ipcChannel - The channel from which this class content will be read.
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        22/2/2010
// ---------------------------------------------------------------------------
bool apOpenCLErrorParameters::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    bool isFunctionCallPresent = false;
    ipcChannel >> isFunctionCallPresent;

    if (isFunctionCallPresent)
    {
        // Read the broken-on function (and replace the existing item if needed):
        apFunctionCall* pFuncCall = new apFunctionCall;

        retVal = pFuncCall->readSelfFromChannel(ipcChannel);
        _aptrBreakedOnFunctionCall = pFuncCall;
    }
    else
    {
        _aptrBreakedOnFunctionCall = NULL;
        retVal = true;
    }

    // Read the OpenCL error code:
    gtUInt32 errCodeAsUInt32 = 0;
    ipcChannel >> errCodeAsUInt32;
    _openCLErrorCode = (int)errCodeAsUInt32;

    return retVal;
}

