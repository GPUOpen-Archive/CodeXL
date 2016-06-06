//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCounterInfo.cpp
///
//==================================================================================

//------------------------------ apCounterInfo.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apCounterInfo.h>
#include <AMDTAPIClasses/Include/apCounterType.h>


// ---------------------------------------------------------------------------
// Name:        apCounterInfo::apCounterInfo
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        24/7/2005
// ---------------------------------------------------------------------------
apCounterInfo::apCounterInfo()
    : _counterType(AP_OS_NATIVE_COUNTER), _counterScopeType(apCounterScope::AP_GLOBAL_COUNTER), _GPUIndex(-1), _minValue(0), _maxValue(100.0),
      _defaultScale(1.0), _counterDataType(AP_COUNTER_DATA_NUMBER)
{
}


// ---------------------------------------------------------------------------
// Name:        apCounterInfo::type
// Description: Returns my transferable object type
// Author:  AMD Developer Tools Team
// Date:        24/7/2005
// ---------------------------------------------------------------------------
osTransferableObjectType apCounterInfo::type() const
{
    return OS_TOBJ_ID_COUNTER_INFO;
}


// ---------------------------------------------------------------------------
// Name:        apCounterInfo::writeSelfIntoChannel
// Description: Write myself into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        24/7/2005
// ---------------------------------------------------------------------------
bool apCounterInfo::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _name;
    ipcChannel << _osID;
    ipcChannel << (gtInt32)_counterType;
    ipcChannel << (gtInt32)_counterScopeType;
    ipcChannel << (gtInt32)_GPUIndex;
    ipcChannel << _description;
    ipcChannel << _minValue;
    ipcChannel << _maxValue;
    ipcChannel << _defaultScale;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCounterInfo::readSelfFromChannel
// Description: Read myself from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        24/7/2005
// ---------------------------------------------------------------------------
bool apCounterInfo::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _name;
    ipcChannel >> _osID;

    gtInt32 counterTypeAsInt32 = 0;
    ipcChannel >> counterTypeAsInt32;
    _counterType = (apCounterType)counterTypeAsInt32;

    gtInt32 scopeTypeAsInt = 0;
    ipcChannel >> scopeTypeAsInt;
    _counterScopeType = (apCounterScope::apCounterScopeType)scopeTypeAsInt;


    gtInt32 GPUIndexAsInt32 = 0;
    ipcChannel >> GPUIndexAsInt32;
    _GPUIndex = (int)GPUIndexAsInt32;

    ipcChannel >> _description;
    ipcChannel >> _minValue;
    ipcChannel >> _maxValue;
    ipcChannel >> _defaultScale;

    return true;
}



// ---------------------------------------------------------------------------
// Name:        apCounterInfo::getDescriptionWithScope
// Description: Builds a counter description for a counter with the requested ID
// Arguments: const apCounterScope& counterScope
//            gtString& counterDescription
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        7/3/2010
// ---------------------------------------------------------------------------
void apCounterInfo::getDescriptionWithScope(const apCounterScope& counterScope, gtString& counterDescription) const
{
    if (_counterScopeType == apCounterScope::AP_CONTEXT_COUNTER)
    {
        int pos = _description.find(L"%d");

        if (pos >= 0)
        {
            counterDescription.appendFormattedString(_description.asCharArray(), counterScope._contextID._contextId);
        }
    }
    else if (_counterScopeType == apCounterScope::AP_QUEUE_COUNTER)
    {
        int pos1 = _description.find(L"%d");
        int pos2 = _description.find(L"%d", pos1 + 1);

        if ((pos1 >= 0) && (pos2 >= 0))
        {
            counterDescription.appendFormattedString(_description.asCharArray(), counterScope._contextID._contextId, counterScope.displayQueueId());
        }
    }

    if (counterDescription.isEmpty())
    {
        counterDescription = _description;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCounterInfo::getNameWithScope
// Description:
// Arguments: const apCounterScope& counterScope
//            gtString& counterName
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        7/3/2010
// ---------------------------------------------------------------------------
void apCounterInfo::getNameWithScope(const apCounterScope& counterScope, gtString& counterName) const
{
    if (_counterScopeType == apCounterScope::AP_CONTEXT_COUNTER)
    {
        counterName.appendFormattedString(_name.asCharArray(), counterScope._contextID._contextId);
    }
    else if (_counterScopeType == apCounterScope::AP_QUEUE_COUNTER)
    {
        // Make sure that the counter name string contain 2 %d's for the context id and the queue id:
        int pos1 = _name.find(L"%d");
        int pos2 = _name.find(L"%d", pos1 + 1);

        if ((pos1 > 0) && (pos2 > 0))
        {
            counterName.appendFormattedString(_name.asCharArray(), counterScope._contextID._contextId, counterScope.displayQueueId());
        }
    }
    else
    {
        counterName = _name;
    }
}
