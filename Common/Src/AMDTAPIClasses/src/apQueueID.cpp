//------------------------------ apQueueID.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apQueueID.h>
#include <AMDTAPIClasses/Include/apCounterNames.h>
#include <inc/apStringConstants.h>


// ---------------------------------------------------------------------------
// Name:        apQueueID::apQueueID
// Description: Constructor.
// Author:      Sigal Algranaty
// Date:        3/3/2010
// ---------------------------------------------------------------------------
apQueueID::apQueueID(apContextID contextID, int queueId)
    : _contextID(contextID), _queueId(queueId)
{
}

apQueueID::apQueueID(int contextID, apContextType contextType, int queueId)
    : _contextID(contextType, contextID), _queueId(queueId)
{

}
// ---------------------------------------------------------------------------
// Name:        apQueueID::~apQueueID
// Description: Destructor
// Return Val:
// Author:      Sigal Algranaty
// Date:        3/3/2010
// ---------------------------------------------------------------------------
apQueueID::~apQueueID()
{

}

// ---------------------------------------------------------------------------
// Name:        apQueueID::type
// Description: Returns my transferable object type
// Author:      Sigal Algranaty
// Date:        3/3/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apQueueID::type() const
{
    return OS_TOBJ_ID_QUEUE_ID;
}


// ---------------------------------------------------------------------------
// Name:        apQueueID::writeSelfIntoChannel
// Description: Write myself into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/3/2010
// ---------------------------------------------------------------------------
bool apQueueID::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    _contextID.writeSelfIntoChannel(ipcChannel);
    ipcChannel << (gtInt32)_queueId;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apQueueID::readSelfFromChannel
// Description: Read myself from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/3/2010
// ---------------------------------------------------------------------------
bool apQueueID::readSelfFromChannel(osChannel& ipcChannel)
{
    _contextID.readSelfFromChannel(ipcChannel);

    gtInt32 queueIdAsInt32 = 0;
    ipcChannel >> queueIdAsInt32;
    _queueId = (int)queueIdAsInt32;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apQueueID::operator==
// Description: Context id comparison operator
// Arguments: const apQueueID& other
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/3/2010
// ---------------------------------------------------------------------------
bool apQueueID::operator==(const apQueueID& other) const
{
    bool retVal = false;

    if ((_contextID == other._contextID) && (_queueId == other._queueId))
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apQueueID::operator==
// Description: Context id comparison operator
// Arguments: const apQueueID& other
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/3/2010
// ---------------------------------------------------------------------------
bool apQueueID::operator!=(const apQueueID& other) const
{
    bool retVal = false;

    if ((_contextID != other._contextID) || (_queueId != other._queueId))
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apQueueID::operator=
// Description: Copy other content into self.
// Author:      Sigal Algranaty
// Date:        3/3/2010
// ---------------------------------------------------------------------------
apQueueID& apQueueID::operator=(const apQueueID& other)
{
    _contextID = other._contextID;
    _queueId = other._queueId;
    return *this;
}


// ---------------------------------------------------------------------------
// Name:        apQueueID::toString
// Description: Convert a context id to a string
// Return Val: gtString
// Author:      Sigal Algranaty
// Date:        3/3/2010
// ---------------------------------------------------------------------------
void apQueueID::toString(gtString& queueStr, bool wasDeleted, int sharedContextId)
{
    // Empty the string:
    queueStr.makeEmpty();
    _contextID.toString(queueStr, wasDeleted, sharedContextId);


    if (_queueId >= 0)
    {
        queueStr.append(AP_STR_Space);
        queueStr.appendFormattedString(AP_STR_PerformanceCountersQueueSuffix, _queueId);
    }

}


// ---------------------------------------------------------------------------
// Name:        apQueueID::operator<
// Description: Context id comparison
// Arguments:   other - Other context to which I am compared to.
// Author:      Sigal Algranaty
// Date:        3/3/2010
// ---------------------------------------------------------------------------
bool apQueueID::operator<(const apQueueID& other) const
{

    bool retVal = false;

    if ((int)_queueId < (int)other._queueId)
    {
        retVal = true;
    }
    else if ((int)_queueId == (int)other._queueId)
    {
        retVal = (_contextID < other._contextID);
    }

    return retVal;
}
