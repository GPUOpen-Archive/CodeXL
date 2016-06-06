//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDeferredCommandEvent.cpp
///
//==================================================================================

//------------------------------ apDeferredCommandEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apDeferredCommandEvent.h>

// ---------------------------------------------------------------------------
// Name:        apDeferredCommandEvent::apDeferredCommandEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        Jan-29, 2015
// ---------------------------------------------------------------------------
apDeferredCommandEvent::apDeferredCommandEvent(apDeferredCommand command, apDeferredCommandTarget target)
    : m_command(command), m_target(target), m_pvData(nullptr), m_pfnDataCloner(nullptr), m_pfnDataReleaser(nullptr)
{
}

// ---------------------------------------------------------------------------
// Name:        apDeferredCommandEvent::~apDeferredCommandEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        Jan-29, 2015
// ---------------------------------------------------------------------------
apDeferredCommandEvent::~apDeferredCommandEvent()
{
    clearData();
}

// ---------------------------------------------------------------------------
// Name:        apDeferredCommandEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        Jan-29, 2015
// ---------------------------------------------------------------------------
osTransferableObjectType apDeferredCommandEvent::type() const
{
    return OS_TOBJ_ID_DEFERRED_COMMAND_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apDeferredCommandEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        Jan-29, 2015
// ---------------------------------------------------------------------------
bool apDeferredCommandEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32) m_command;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDeferredCommandEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        Jan-29, 2015
// ---------------------------------------------------------------------------
bool apDeferredCommandEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 commandAsint32;
    ipcChannel >> commandAsint32;
    m_command = (apDeferredCommand)commandAsint32;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDeferredCommandEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        Jan-29, 2015
// ---------------------------------------------------------------------------
apEvent::EventType apDeferredCommandEvent::eventType() const
{
    return apEvent::AP_DEFERRED_COMMAND_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apDeferredCommandEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller's responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        Jan-29, 2015
// ---------------------------------------------------------------------------
apEvent* apDeferredCommandEvent::clone() const
{
    apDeferredCommandEvent* pClone = new apDeferredCommandEvent(m_command, m_target);

    // If we can't clone the data, just copy the pointer:
    pClone->setData(m_pvData, m_pfnDataReleaser, m_pfnDataCloner);

    return pClone;
}

// ---------------------------------------------------------------------------
// Name:        apDeferredCommandEvent::clearData()
// Description: Clears the data and related function pointers
// Author:      AMD Developer Tools Team
// Date:        10/2/2016
// ---------------------------------------------------------------------------
void apDeferredCommandEvent::clearData()
{
    if (nullptr != m_pfnDataReleaser)
    {
        m_pfnDataReleaser(m_pvData);
    }

    m_pvData = nullptr;
    m_pfnDataReleaser = nullptr;
    m_pfnDataCloner = nullptr;
}

// ---------------------------------------------------------------------------
// Name:        apDeferredCommandEvent::setData
// Description: Sets a data pointer and clone and release routines
// Author:      AMD Developer Tools Team
// Date:        10/2/2016
// ---------------------------------------------------------------------------
void apDeferredCommandEvent::setData(const void* pvData, apDeferredCommandDataReleaser pfnReleaser, apDeferredCommandDataCloner pfnCloner)
{
    // Clear any data we're overwriting:
    clearData();

    m_pfnDataCloner = pfnCloner;
    m_pfnDataReleaser = pfnReleaser;

    // Clone the source data if possible, copy it otherwise:
    m_pvData = (nullptr != m_pfnDataCloner) ? m_pfnDataCloner(pvData) : (void*)pvData;
}

