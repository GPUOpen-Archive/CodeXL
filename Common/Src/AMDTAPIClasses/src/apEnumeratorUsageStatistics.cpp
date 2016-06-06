//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apEnumeratorUsageStatistics.cpp
///
//==================================================================================

//------------------------------ apEnumeratorUsageStatistics.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apEnumeratorUsageStatistics.h>


// ---------------------------------------------------------------------------
// Name:        apEnumeratorUsageStatistics::apEnumeratorUsageStatistics
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        29/1/2006
// ---------------------------------------------------------------------------
apEnumeratorUsageStatistics::apEnumeratorUsageStatistics()
{
    init();
}


// ---------------------------------------------------------------------------
// Name:        apEnumeratorUsageStatistics::init
// Description: Initialize struct member values.
// Author:  AMD Developer Tools Team
// Date:        29/1/2006
// ---------------------------------------------------------------------------
void apEnumeratorUsageStatistics::init()
{
    _enum = 0;
    _amountOfTimesUsed = 0;
    _amountOfRedundantTimesUsed = 0;
    _enumType = OS_TOBJ_ID_GL_ENUM_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apEnumeratorUsageStatistics::type
// Description: Returns my transferable object type.
// Return Val: osTransferableObjectType
// Author:  AMD Developer Tools Team
// Date:        29/1/2006
// ---------------------------------------------------------------------------
osTransferableObjectType apEnumeratorUsageStatistics::type() const
{
    return OS_TOBJ_ID_ENUM_USAGE_STATISTICS;
}


// ---------------------------------------------------------------------------
// Name:        apEnumeratorUsageStatistics::writeSelfIntoChannel
// Description: Writes my content into a channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/1/2006
// ---------------------------------------------------------------------------
bool apEnumeratorUsageStatistics::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_enum;
    ipcChannel << _amountOfTimesUsed;
    ipcChannel << _amountOfRedundantTimesUsed;
    ipcChannel << (gtUInt32)_enumType;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apEnumeratorUsageStatistics::readSelfFromChannel
// Description: Reads my content from an IPC channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/1/2006
// ---------------------------------------------------------------------------
bool apEnumeratorUsageStatistics::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 enumAsInt32 = 0;
    ipcChannel >> enumAsInt32;
    _enum = (GLenum)enumAsInt32;

    ipcChannel >> _amountOfTimesUsed;
    ipcChannel >> _amountOfRedundantTimesUsed;

    gtUInt32 enumTypeAsUInt32 = 0;
    ipcChannel >> enumTypeAsUInt32;
    _enumType = (osTransferableObjectType)enumTypeAsUInt32;

    return true;
}

