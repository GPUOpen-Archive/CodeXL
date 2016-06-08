//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCounterID.cpp
///
//==================================================================================

//------------------------------ apCounterID.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apCounterID.h>
#include <AMDTAPIClasses/Include/apCounterInfo.h>
#include <AMDTAPIClasses/Include/apCounterType.h>

// ---------------------------------------------------------------------------
// Name:        apCounterID::apCounterID
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        20/2/2008
// ---------------------------------------------------------------------------
apCounterID::apCounterID()
    : _counterType(AP_OS_NATIVE_COUNTER), _counterLocalIndex(0), _counterScope(apCounterScope::AP_GLOBAL_COUNTER)
{
}


// ---------------------------------------------------------------------------
// Name:        apCounterID::type
// Description: Returns my transferable object type
// Author:  AMD Developer Tools Team
// Date:        20/2/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apCounterID::type() const
{
    return OS_TOBJ_ID_COUNTER_INFO;
}


// ---------------------------------------------------------------------------
// Name:        apCounterID::writeSelfIntoChannel
// Description: Write myself into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/2/2008
// ---------------------------------------------------------------------------
bool apCounterID::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_counterType;
    ipcChannel << (gtInt32)_counterLocalIndex;
    _counterScope.writeSelfIntoChannel(ipcChannel);

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCounterID::readSelfFromChannel
// Description: Read myself from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/2/2008
// ---------------------------------------------------------------------------
bool apCounterID::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 counterTypeAsInt32 = 0;
    ipcChannel >> counterTypeAsInt32;
    _counterType = (apCounterType)counterTypeAsInt32;

    gtInt32 counterLocalIndexAsInt32 = 0;
    ipcChannel >> counterLocalIndexAsInt32;
    _counterLocalIndex = (int)counterLocalIndexAsInt32;

    _counterScope.readSelfFromChannel(ipcChannel);
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCounterID::operator==
// Description: Operator ==
// Arguments: const apCounterID& other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/2/2008
// ---------------------------------------------------------------------------
bool apCounterID::operator==(const apCounterID& other) const
{
    bool retVal = false;

    if ((_counterType == other._counterType) && (_counterLocalIndex == other._counterLocalIndex) && (_counterScope == other._counterScope))
    {
        retVal = true;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        apCounterID::apCounterID
// Description: Copy constructor
// Arguments:   other - The other display list class from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        3/3/2010
// ---------------------------------------------------------------------------
apCounterID& apCounterID::operator=(const apCounterID& other)
{
    _counterLocalIndex = other._counterLocalIndex;
    _counterType = other._counterType;
    _counterScope = other._counterScope;
    return *this;
}

// ---------------------------------------------------------------------------
// Name:        apCounterActivationInfo::apCounterActivationInfo
// Description: Constructor
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        31/1/2010
// ---------------------------------------------------------------------------
apCounterActivationInfo::apCounterActivationInfo(): _counterId(), _shouldBeActivated(false)
{
}


// ---------------------------------------------------------------------------
// Name:        apCounterActivationInfo::type
// Description: Returns my transferable object type
// Return Val: osTransferableObjectType
// Author:  AMD Developer Tools Team
// Date:        31/1/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apCounterActivationInfo::type() const
{
    return OS_TOBJ_ID_COUNTER_ACTIVATION_INFO;
}


// ---------------------------------------------------------------------------
// Name:        apCounterActivationInfo::writeSelfIntoChannel
// Description: Write myself into an IPC channel.
// Arguments: osChannel& ipcChannel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        31/1/2010
// ---------------------------------------------------------------------------
bool apCounterActivationInfo::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = _counterId.writeSelfIntoChannel(ipcChannel);
    ipcChannel << _shouldBeActivated;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCounterActivationInfo::readSelfFromChannel
// Description: Read myself from an IPC channel.
// Arguments: osChannel& ipcChannel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        31/1/2010
// ---------------------------------------------------------------------------
bool apCounterActivationInfo::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = _counterId.readSelfFromChannel(ipcChannel);
    ipcChannel >> _shouldBeActivated;
    return retVal;
}
