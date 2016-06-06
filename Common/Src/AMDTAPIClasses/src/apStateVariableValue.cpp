//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apStateVariableValue.cpp
///
//==================================================================================

//------------------------------ apStateVariableValue.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apStateVariableValue.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// ---------------------------------------------------------------------------
// Name:        apStateVariableValue::apStateVariableValue
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        26/6/2008
// ---------------------------------------------------------------------------
apStateVariableValue::apStateVariableValue()
    : _id(-1), _pValue(NULL)
{
}

// ---------------------------------------------------------------------------
// Name:        apStateVariableValue::apStateVariableValue
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        26/6/2008
// ---------------------------------------------------------------------------
apStateVariableValue::apStateVariableValue(int id, apParameter* pValue)
    : _id(id), _pValue(pValue)
{
}



// ---------------------------------------------------------------------------
// Name:        apStateVariableValue::~apStateVariableValue
// Description: Destructor
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        29/6/2008
// ---------------------------------------------------------------------------
apStateVariableValue::~apStateVariableValue()
{
    if (_pValue != NULL)
    {
        delete _pValue;
    }
}
// ---------------------------------------------------------------------------
// Name:        apStateVariableValue::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        26/6/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apStateVariableValue::type() const
{
    return OS_TOBJ_ID_STATE_VARIABLE_VALUE;
}

// ---------------------------------------------------------------------------
// Name:        apStateVariableValue::apStateVariableValue
// Description: Writes this class data into a channel.
// Author:  AMD Developer Tools Team
// Date:        15/6/2004
// ---------------------------------------------------------------------------
bool apStateVariableValue::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_id;

    if (_pValue != NULL)
    {
        // Write the state variable value:
        _pValue->writeSelfIntoChannel(ipcChannel);
    }

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apStateVariableValue::apStateVariableValue
// Description: Reads this class data from a channel.
// Author:  AMD Developer Tools Team
// Date:        15/6/2004
// ---------------------------------------------------------------------------
bool apStateVariableValue::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 idAsInt32 = 0;
    ipcChannel >> idAsInt32;
    _id = (int)idAsInt32;

    if (_pValue != NULL)
    {
        // Read the state variable value:
        _pValue->readSelfFromChannel(ipcChannel);
    }

    return true;
}
