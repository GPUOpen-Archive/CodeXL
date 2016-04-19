//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdUpdateUIEvent.cpp
///
//==================================================================================

//------------------------------ gdUpdateUIEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdUpdateUIEvent.h>


// ---------------------------------------------------------------------------
// Name:        gdUpdateUIEvent::gdUpdateUIEvent
// Description: Constructor.
// Author:      Sigal Algranaty
// Date:        10/1/2011
// ---------------------------------------------------------------------------
gdUpdateUIEvent::gdUpdateUIEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        gdUpdateUIEvent::type
// Description: Returns my transferable object type.
// Author:      Sigal Algranaty
// Date:        10/1/2011
// ---------------------------------------------------------------------------
osTransferableObjectType gdUpdateUIEvent::type() const
{
    return OS_TOBJ_ID_UPDATE_UI_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        gdUpdateUIEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        10/1/2011
// ---------------------------------------------------------------------------
bool gdUpdateUIEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdUpdateUIEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        10/1/2011
// ---------------------------------------------------------------------------
bool gdUpdateUIEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdUpdateUIEvent::type
// Description: Returns my event type.
// Author:      Sigal Algranaty
// Date:        10/1/2011
// ---------------------------------------------------------------------------
apEvent::EventType gdUpdateUIEvent::eventType() const
{
    return apEvent::APP_UPDATE_UI_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        gdUpdateUIEvent::clone
// Description: Returns a copy of self.
// Author:      Sigal Algranaty
// Date:        10/1/2011
// ---------------------------------------------------------------------------
apEvent* gdUpdateUIEvent::clone() const
{
    return new gdUpdateUIEvent();
}
