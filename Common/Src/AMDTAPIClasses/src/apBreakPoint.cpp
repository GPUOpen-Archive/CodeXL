//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apBreakPoint.cpp
///
//==================================================================================

//------------------------------ apBreakPoint.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apBreakPoint.h>


// ---------------------------------------------------------------------------
// Name:        apBreakPoint::apBreakPoint
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        15/6/2004
// ---------------------------------------------------------------------------
apBreakPoint::apBreakPoint()
    : m_state(BREAKPOINT_STATE_ENABLED), m_hitCount(0)
{
}

// ---------------------------------------------------------------------------
// Name:        apBreakPoint::~apBreakPoint
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        1/11/2010
// ---------------------------------------------------------------------------
apBreakPoint::~apBreakPoint()
{

}

// ---------------------------------------------------------------------------
// Name:        apBreakPoint::apBreakPoint
// Description: Writes this class data into a channel.
// Author:  AMD Developer Tools Team
// Date:        15/6/2004
// ---------------------------------------------------------------------------
bool apBreakPoint::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)m_state;
    ipcChannel << (gtInt32)m_hitCount;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apBreakPoint::apBreakPoint
// Description: Reads this class data from a channel.
// Author:  AMD Developer Tools Team
// Date:        15/6/2004
// ---------------------------------------------------------------------------
bool apBreakPoint::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 state32;
    ipcChannel >> state32;
    m_state = (State)state32;

    gtInt32 hitCount32;
    ipcChannel >> hitCount32;
    m_hitCount = (int)hitCount32;
    return true;
}
