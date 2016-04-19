//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afProcessDebuggerPendingEventEvent.cpp
///
//==================================================================================

// Local:
#include <src/afProcessDebuggerPendingEventEvent.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>


// Static member initializations:
afProcessDebuggerPendingEventEvent* afProcessDebuggerPendingEventEvent::m_pMySingleInstance = nullptr;

// ---------------------------------------------------------------------------
// Name:        afProcessDebuggerPendingEventEvent::afProcessDebuggerPendingEventEvent
// Description: Constructor -
// Author:      Yaki Tebeka
// Date:        6/4/2004
// ---------------------------------------------------------------------------
afProcessDebuggerPendingEventEvent::afProcessDebuggerPendingEventEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        afProcessDebuggerPendingEventEvent::~afProcessDebuggerPendingEventEvent
// Description: destructor
// Author:      Gilad Yarnitzky
// Date:        6/19/2012
// ---------------------------------------------------------------------------
afProcessDebuggerPendingEventEvent::~afProcessDebuggerPendingEventEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        afProcessDebuggerEventHandler::instance
// Description: Returns the single instance of this class.
//              (If it does not exist - create it)
// Author:      Gilad Yarnitzky
// Date:        6/19/2012
// ---------------------------------------------------------------------------
afProcessDebuggerPendingEventEvent& afProcessDebuggerPendingEventEvent::instance()
{
    if (m_pMySingleInstance == nullptr)
    {
        m_pMySingleInstance = new afProcessDebuggerPendingEventEvent;
        GT_ASSERT(m_pMySingleInstance);
    }

    return *m_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        afProcessDebuggerPendingEventEvent::emitEvent
// Description: emit the signal
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        19/6/2012
// ---------------------------------------------------------------------------
void afProcessDebuggerPendingEventEvent::emitEvent()
{
    emit pendingDebugEvent();
}



