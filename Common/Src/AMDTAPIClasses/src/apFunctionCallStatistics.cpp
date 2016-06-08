//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apFunctionCallStatistics.cpp
///
//==================================================================================

//------------------------------ apFunctionCallStatistics.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTAPIClasses/Include/apFunctionCallStatistics.h>


// ---------------------------------------------------------------------------
// Name:        apFunctionCallStatistics::apFunctionCallStatistics
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        29/1/2006
// ---------------------------------------------------------------------------
apFunctionCallStatistics::apFunctionCallStatistics()
{
    init();
}


// ---------------------------------------------------------------------------
// Name:        apFunctionCallStatistics::init
// Description: Initialize struct member values.
// Author:  AMD Developer Tools Team
// Date:        29/1/2006
// ---------------------------------------------------------------------------
void apFunctionCallStatistics::init()
{
    _functionId = apMonitoredFunctionsAmount;
    _amountOfTimesCalled = 0;
    _averageAmountPerFrame = 0;
    _averageRedundantAmountPerFrame = 0;
    _amountOfRedundantTimesCalled = 0;
    _usedEnumerators.clear();

    for (int i = 0; i < AP_DEPRECATION_STATUS_AMOUNT; i++)
    {
        _amountOfDeprecatedTimesCalled[i] = 0;
        _averageDeprecatedAmountPerFrame[i] = 0;
    }
}


// ---------------------------------------------------------------------------
// Name:        ~apFunctionCallStatistics::apFunctionCallStatistics
// Description: Destructor
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        6/8/2008
// ---------------------------------------------------------------------------
apFunctionCallStatistics::~apFunctionCallStatistics()
{

}
// ---------------------------------------------------------------------------
// Name:        apFunctionCallStatistics::type
// Description: Returns my transferable object type.
// Return Val: osTransferableObjectType
// Author:  AMD Developer Tools Team
// Date:        29/1/2006
// ---------------------------------------------------------------------------
osTransferableObjectType apFunctionCallStatistics::type() const
{
    return OS_TOBJ_ID_FUNC_CALL_STATISTICS;
}


// ---------------------------------------------------------------------------
// Name:        apFunctionCallStatistics::writeSelfIntoChannel
// Description: Writes my content into a channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/1/2006
// ---------------------------------------------------------------------------
bool apFunctionCallStatistics::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    ipcChannel << (gtInt32)_functionId;
    ipcChannel << _amountOfTimesCalled;
    ipcChannel << _amountOfRedundantTimesCalled;
    ipcChannel << _averageAmountPerFrame;
    ipcChannel << _averageRedundantAmountPerFrame;

    for (int i = 0; i < AP_DEPRECATION_STATUS_AMOUNT; i++)
    {
        ipcChannel << _amountOfDeprecatedTimesCalled[i];
        ipcChannel << _averageDeprecatedAmountPerFrame[i];
    }

    // Write the used enumerators:
    gtInt32 amountOfUsedEnums = (gtInt32)_usedEnumerators.size();
    ipcChannel << amountOfUsedEnums;

    for (int i = 0; i < amountOfUsedEnums; i++)
    {
        bool rc = _usedEnumerators[i].writeSelfIntoChannel(ipcChannel);
        retVal = retVal && rc;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apFunctionCallStatistics::readSelfFromChannel
// Description: Reads my content from an IPC channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/1/2006
// ---------------------------------------------------------------------------
bool apFunctionCallStatistics::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Initialize class members:
    init();

    // Read simple members:
    gtInt32 functionIdAsInt32 = 0;
    ipcChannel >> functionIdAsInt32;
    _functionId = (apMonitoredFunctionId)functionIdAsInt32;

    ipcChannel >> _amountOfTimesCalled;
    ipcChannel >> _amountOfRedundantTimesCalled;
    ipcChannel >> _averageAmountPerFrame;
    ipcChannel >> _averageRedundantAmountPerFrame;

    for (int i = 0; i < AP_DEPRECATION_STATUS_AMOUNT; i++)
    {
        ipcChannel >> _amountOfDeprecatedTimesCalled[i];
        ipcChannel >> _averageDeprecatedAmountPerFrame[i];
    }

    // Read the used enumerators:
    gtInt32 amountOfUsedEnums = 0;
    ipcChannel >> amountOfUsedEnums;

    apEnumeratorUsageStatistics currentEnumStatistics;

    for (int i = 0; i < amountOfUsedEnums; i++)
    {
        bool rc = currentEnumStatistics.readSelfFromChannel(ipcChannel);
        GT_IF_WITH_ASSERT(rc)
        {
            _usedEnumerators.push_back(currentEnumStatistics);
        }
        else
        {
            retVal = false;
        }
    }

    return retVal;
}

