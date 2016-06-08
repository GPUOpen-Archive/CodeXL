//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCounterScope.cpp
///
//==================================================================================

//------------------------------ apCounterScope.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apCounterScope.h>
#include <AMDTAPIClasses/Include/apCounterNames.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>

// Static members initialization:
apCounterScope apCounterScope::_stat_defaultGLContextScope(apContextID(AP_OPENGL_CONTEXT, 1));
apCounterScope apCounterScope::_stat_defaultGLNoContextScope(apContextID(AP_OPENGL_CONTEXT, 0));
apCounterScope apCounterScope::_stat_defaultGlobalScope(apCounterScope::AP_GLOBAL_COUNTER);
apCounterScope apCounterScope::_stat_defaultCLQueueScope(1, 0);
apCounterScope apCounterScope::_stat_defaultCLNoContextScope(0, 0);


// ---------------------------------------------------------------------------
// Name:        apCounterScope::apCounterScope
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        3/3/2010
// ---------------------------------------------------------------------------
apCounterScope::apCounterScope(apCounterScopeType counterScopeType):
    _counterScopeType(counterScopeType), _contextID(AP_OPENGL_CONTEXT, -1), _queueId(-1)
{
    if (counterScopeType == AP_CONTEXT_COUNTER)
    {
        _contextID._contextType = AP_OPENGL_CONTEXT;
        _contextID._contextId = 1;
    }
    else if (counterScopeType == AP_QUEUE_COUNTER)
    {
        _contextID._contextType = AP_OPENCL_CONTEXT;
        _contextID._contextId = 1;
        _queueId = 0;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCounterScope::apCounterScope
// Description: Constructor for context scope
// Arguments: apContextID contextID
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        7/3/2010
// ---------------------------------------------------------------------------
apCounterScope::apCounterScope(apContextID contextID):
    _counterScopeType(AP_CONTEXT_COUNTER), _contextID(contextID), _queueId(-1)
{

}

// ---------------------------------------------------------------------------
// Name:        apCounterScope::apCounterScope
// Description: Constructor for queue scope
// Arguments: apContextID contextID
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        7/3/2010
// ---------------------------------------------------------------------------
apCounterScope::apCounterScope(int contextId, int queueId):
    _counterScopeType(AP_QUEUE_COUNTER), _contextID(AP_OPENCL_CONTEXT, contextId), _queueId(queueId)
{

}

// ---------------------------------------------------------------------------
// Name:        apCounterScope::apCounterScope
// Description: Constructor for OpenGL context
// Arguments: apContextID contextID
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        7/3/2010
// ---------------------------------------------------------------------------
apCounterScope::apCounterScope(int contextId):
    _counterScopeType(AP_CONTEXT_COUNTER), _contextID(AP_OPENGL_CONTEXT, contextId), _queueId(-1)
{

}

// ---------------------------------------------------------------------------
// Name:        apCounterScope::~apCounterScope
// Description: Destructor
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        3/3/2010
// ---------------------------------------------------------------------------
apCounterScope::~apCounterScope()
{

}

// ---------------------------------------------------------------------------
// Name:        apCounterScope::type
// Description: Returns my transferable object type
// Author:  AMD Developer Tools Team
// Date:        3/3/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apCounterScope::type() const
{
    return OS_TOBJ_ID_COUNTER_SCOPE_ID;
}


// ---------------------------------------------------------------------------
// Name:        apCounterScope::writeSelfIntoChannel
// Description: Write myself into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        3/3/2010
// ---------------------------------------------------------------------------
bool apCounterScope::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the counter scope type:
    ipcChannel << (gtInt32)_counterScopeType;

    // Write the context id:
    _contextID.writeSelfIntoChannel(ipcChannel);

    // Write the queue id:
    ipcChannel << (gtInt32)_queueId;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCounterScope::readSelfFromChannel
// Description: Read myself from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        3/3/2010
// ---------------------------------------------------------------------------
bool apCounterScope::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the counter scope type:
    gtInt32 varAsInt32 = 0;
    ipcChannel >> varAsInt32;
    _counterScopeType = (apCounterScopeType)varAsInt32;

    // Read the context id:
    _contextID.readSelfFromChannel(ipcChannel);

    // Read the queue id:
    varAsInt32 = 0;
    ipcChannel >> varAsInt32;
    _queueId = (int)varAsInt32;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCounterScope::operator==
// Description: Context id comparison operator
// Arguments: const apCounterScope& other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        3/3/2010
// ---------------------------------------------------------------------------
bool apCounterScope::operator==(const apCounterScope& other) const
{
    bool retVal = false;

    if ((_counterScopeType == other._counterScopeType) && (_contextID == other._contextID) && (_queueId == other._queueId))
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCounterScope::operator!=
// Description: Context id comparison operator
// Arguments: const apCounterScope& other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        3/3/2010
// ---------------------------------------------------------------------------
bool apCounterScope::operator!=(const apCounterScope& other) const
{
    bool retVal = false;

    if ((_counterScopeType != other._counterScopeType) || (_contextID != other._contextID) || (_queueId != other._queueId))
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCounterScope::operator=
// Description: Copy other content into self.
// Author:  AMD Developer Tools Team
// Date:        3/3/2010
// ---------------------------------------------------------------------------
apCounterScope& apCounterScope::operator=(const apCounterScope& other)
{
    _counterScopeType = other._counterScopeType;
    _contextID = other._contextID;
    _queueId = other._queueId;

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        apCounterScope::toString
// Description: Convert a context id to a string
// Return Val: gtString
// Author:  AMD Developer Tools Team
// Date:        3/3/2010
// ---------------------------------------------------------------------------
void apCounterScope::toString(gtString& queueStr)
{
    // Empty the string:
    queueStr.makeEmpty();
    _contextID.toString(queueStr);

    if (_queueId >= 0)
    {
        queueStr.append(AP_STR_Space);
        int queueDisplayId = _queueId + 1;
        queueStr.appendFormattedString(AP_STR_PerformanceCountersQueueSuffix, queueDisplayId);
    }

}


// ---------------------------------------------------------------------------
// Name:        apCounterScope::operator<
// Description: Context id comparison
// Arguments:   other - Other context to which I am compared to.
// Author:  AMD Developer Tools Team
// Date:        3/3/2010
// ---------------------------------------------------------------------------
bool apCounterScope::operator<(const apCounterScope& other) const
{

    bool retVal = false;

    if ((int)_counterScopeType < (int)other._counterScopeType)
    {
        retVal = true;
    }
    else if ((int)_queueId < (int)other._queueId)
    {
        retVal = true;
    }
    else if ((int)_queueId == (int)other._queueId)
    {
        retVal = (_contextID < other._contextID);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCounterScope::isDefault
// Description: Return true iff the scope is either OpenCL default queue or
//              OpenGL default context
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        7/3/2010
// ---------------------------------------------------------------------------
bool apCounterScope::isDefault() const
{
    bool retVal = false;

    if (((*this) == defaultGLContextScope()) || ((*this) == defaultCLQueueScope()) || ((*this) == _stat_defaultGLNoContextScope) || ((*this) == _stat_defaultCLNoContextScope))
    {
        retVal = true;
    }

    return retVal;
}

