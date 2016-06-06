//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apFlushTextureImageEvent.cpp
///
//==================================================================================

//------------------------------ apFlushTextureImageEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apFlushTextureImageEvent.h>


// ---------------------------------------------------------------------------
// Name:        apFlushTextureImageEvent::apFlushTextureImageEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        28/2/2011
// ---------------------------------------------------------------------------
apFlushTextureImageEvent::apFlushTextureImageEvent()
{

}


// ---------------------------------------------------------------------------
// Name:        apFlushTextureImageEvent::~apFlushTextureImageEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        28/2/2011
// ---------------------------------------------------------------------------
apFlushTextureImageEvent::~apFlushTextureImageEvent()
{

}



// ---------------------------------------------------------------------------
// Name:        apFlushTextureImageEvent::type
// Description:
// Author:  AMD Developer Tools Team
// Date:        28/2/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apFlushTextureImageEvent::type() const
{
    return OS_TOBJ_ID_FLUSH_TEXTURE_IMAGES_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apFlushTextureImageEvent::writeSelfIntoChannel
// Description:
// Arguments:   osChannel& ipcChannel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/2/2011
// ---------------------------------------------------------------------------
bool apFlushTextureImageEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    (void)(ipcChannel); // unused
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apFlushTextureImageEvent::readSelfFromChannel
// Description:
// Arguments:   osChannel& ipcChannel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/2/2011
// ---------------------------------------------------------------------------
bool apFlushTextureImageEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    (void)(ipcChannel); // unused
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apFlushTextureImageEvent::eventType
// Description:
// Return Val:  EventType
// Author:  AMD Developer Tools Team
// Date:        28/2/2011
// ---------------------------------------------------------------------------
apEvent::EventType apFlushTextureImageEvent::eventType() const
{
    return apEvent::AP_FLUSH_TEXTURE_IMAGES_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apFlushTextureImageEvent::clone
// Description:
// Return Val:  apEvent*
// Author:  AMD Developer Tools Team
// Date:        28/2/2011
// ---------------------------------------------------------------------------
apEvent* apFlushTextureImageEvent::clone() const
{
    apFlushTextureImageEvent* pEventCopy = new apFlushTextureImageEvent();

    return pEventCopy;
}
