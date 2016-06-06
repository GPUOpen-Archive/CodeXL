//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDetectedErrorParameters.cpp
///
//==================================================================================

//------------------------------ apDetectedErrorParameters.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apDetectedErrorParameters.h>
#include <AMDTAPIClasses/Include/apErrorCode.h>


// ---------------------------------------------------------------------------
// Name:        apDetectedErrorParameters::apDetectedErrorParameters
// Description: Copy constructor
// Author:  AMD Developer Tools Team
// Date:        21/1/2010
// ---------------------------------------------------------------------------
apDetectedErrorParameters::apDetectedErrorParameters(const apDetectedErrorParameters& other)
{
    operator=(other);
}

// ---------------------------------------------------------------------------
// Name:        apDetectedErrorParameters::apDetectedErrorParameters
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        7/10/2007
// ---------------------------------------------------------------------------
apDetectedErrorParameters::apDetectedErrorParameters()
{
    clearParameters();
}

// ---------------------------------------------------------------------------
// Name:        apDetectedErrorParameters::operator=
// Description: Assignment operator
// Author:  AMD Developer Tools Team
// Date:        21/1/2010
// ---------------------------------------------------------------------------
apDetectedErrorParameters& apDetectedErrorParameters::operator=(const apDetectedErrorParameters& other)
{
    _detectedErrorCode = other._detectedErrorCode;
    _detectedErrorAssociatedFunction = other._detectedErrorAssociatedFunction;
    _detectedErrorDescription = other._detectedErrorDescription;

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        apDetectedErrorParameters::clearParameters
// Description: Clears the detected error parameters.
// Author:  AMD Developer Tools Team
// Date:        7/10/2007
// ---------------------------------------------------------------------------
void apDetectedErrorParameters::clearParameters()
{
    _detectedErrorCode = AP_UNKNOWN_DETECTED_ERROR;
    _detectedErrorAssociatedFunction = apMonitoredFunctionsAmount;
    _detectedErrorDescription.makeEmpty();
}


// ---------------------------------------------------------------------------
// Name:        apDetectedErrorParameters::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        7/10/2007
// ---------------------------------------------------------------------------
osTransferableObjectType apDetectedErrorParameters::type() const
{
    return OS_TOBJ_ID_DETECTED_ERROR_PARAMS;
}


// ---------------------------------------------------------------------------
// Name:        apDetectedErrorParameters::writeSelfIntoChannel
// Description: Writes this class content into a channel.
// Arguments: ipcChannel - The channel to which this class content will be written.
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        7/10/2007
// ---------------------------------------------------------------------------
bool apDetectedErrorParameters::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    ipcChannel << (gtUInt32)_detectedErrorCode;
    ipcChannel << (gtInt32)_detectedErrorAssociatedFunction;
    ipcChannel << _detectedErrorDescription;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apDetectedErrorParameters::readSelfFromChannel
// Description: Reads this class content from a channel.
// Arguments: ipcChannel - The channel from which this class content will be read.
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        7/10/2007
// ---------------------------------------------------------------------------
bool apDetectedErrorParameters::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    gtUInt32 detectedErrorCodeAsUInt32 = 0;
    ipcChannel >> detectedErrorCodeAsUInt32;
    _detectedErrorCode = (apErrorCode)detectedErrorCodeAsUInt32;

    gtInt32 detectedErrorAssociatedFunctionAsInt32 = 0;
    ipcChannel >> detectedErrorAssociatedFunctionAsInt32;
    _detectedErrorAssociatedFunction = (apMonitoredFunctionId)detectedErrorAssociatedFunctionAsInt32;

    ipcChannel >> _detectedErrorDescription;

    return retVal;
}

