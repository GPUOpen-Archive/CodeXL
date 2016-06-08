//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apSpyProgressEvent.cpp
///
//==================================================================================

//------------------------------ apSpyProgressEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apSpyProgressEvent.h>

// ---------------------------------------------------------------------------
// Name:        apSpyProgressEvent::apSpyProgressEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        29/7/2010
// ---------------------------------------------------------------------------
apSpyProgressEvent::apSpyProgressEvent(int progress): _progress(progress)
{
}

// ---------------------------------------------------------------------------
// Name:        apSpyProgressEvent::~apSpyProgressEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        29/7/2010
// ---------------------------------------------------------------------------
apSpyProgressEvent::~apSpyProgressEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apSpyProgressEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        29/7/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apSpyProgressEvent::type() const
{
    return OS_TOBJ_ID_SPY_PROGRESS_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apSpyProgressEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2010
// ---------------------------------------------------------------------------
bool apSpyProgressEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write my progress:
    ipcChannel << (gtUInt32)_progress;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apSpyProgressEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2010
// ---------------------------------------------------------------------------
bool apSpyProgressEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read my progress:
    gtUInt32 varAsUInt32 = 0;
    ipcChannel >> varAsUInt32;
    _progress = varAsUInt32;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apSpyProgressEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        29/7/2010
// ---------------------------------------------------------------------------
apEvent::EventType apSpyProgressEvent::eventType() const
{
    return apEvent::AP_SPY_PROGRESS_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apSpyProgressEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller's responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        29/7/2010
// ---------------------------------------------------------------------------
apEvent* apSpyProgressEvent::clone() const
{
    apSpyProgressEvent* pClone = new apSpyProgressEvent(_progress);


    return pClone;
}
