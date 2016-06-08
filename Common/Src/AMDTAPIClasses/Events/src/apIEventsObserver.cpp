//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apIEventsObserver.cpp
///
//==================================================================================

//------------------------------ apIEventsObserver.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>


// ---------------------------------------------------------------------------
// Name:        apIEventsObserver::~apIEventsObserver
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        18/12/2006
// ---------------------------------------------------------------------------
apIEventsObserver::~apIEventsObserver()
{
}

// ---------------------------------------------------------------------------
// Name:        apIEventsObserver::onEventRegistration
// Description: This method should be overridden to allow usage of event registration
// Author:  AMD Developer Tools Team
// Date:        21/4/2009
// ---------------------------------------------------------------------------
void apIEventsObserver::onEventRegistration(apEvent& eve, bool& vetoEvent)
{
    (void)(eve); // unused
    (void)(vetoEvent); // unused

}
