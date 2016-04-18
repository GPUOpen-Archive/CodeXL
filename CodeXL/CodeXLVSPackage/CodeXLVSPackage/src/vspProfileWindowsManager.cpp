//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspProfileWindowsManager.cpp
///
//==================================================================================

//------------------------------ vspProfileWindowsManager.cpp ------------------------------

#include "stdafx.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>

// AMDTGpuProfiling:
#include <AMDTGpuProfiling/gpViewsCreator.h>

// AMDTCpuProfiling:
#include <AMDTCpuProfiling/inc/AmdtCpuProfiling.h>
#include <AMDTCpuProfiling/inc/SessionViewCreator.h>

// Local:
#include <src/vspProfileWindowsManager.h>


// Static members initializations:
vspProfileWindowsManager* vspProfileWindowsManager::m_pMySingleInstance = NULL;

vspProfileWindowsManager::vspProfileWindowsManager() : apIEventsObserver()
{
    // Register as an events observer:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_FRAMEWORK_EVENTS_HANDLING_PRIORITY);
}

vspProfileWindowsManager::~vspProfileWindowsManager()
{
    // Register as an events observer:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

vspProfileWindowsManager& vspProfileWindowsManager::instance()
{
    // If my single instance was not created yet - create it:
    if (m_pMySingleInstance == NULL)
    {
        m_pMySingleInstance = new vspProfileWindowsManager;

    }

    return *m_pMySingleInstance;
}


void vspProfileWindowsManager::onEvent(const apEvent& eve, bool& vetoEvent)
{
    GT_UNREFERENCED_PARAMETER(vetoEvent);

    // Get the event type:
    apEvent::EventType eventType = eve.eventType();


    // Sanity check. Make sure that the creators are initialize:
    GT_IF_WITH_ASSERT((gpViewsCreator::Instance() != nullptr) && (AmdtCpuProfiling::sessionViewCreator() != nullptr))
    {
        switch (eventType)
        {

            case apEvent::AP_MDI_CREATED_EVENT:
            {
                const apMDIViewCreateEvent mdiEvent = (const apMDIViewCreateEvent&)eve;

                // Check if this is a GPU profile view:
                if (mdiEvent.CreatedMDIType() == gpViewsCreator::Instance()->CreatedMDIType())
                {
                    bool rc = gpViewsCreator::Instance()->displayExistingView(mdiEvent);
                    GT_ASSERT(rc)
                }

                // Check if this is a CPU profile view:
                if (mdiEvent.CreatedMDIType() == AmdtCpuProfiling::sessionViewCreator()->CreatedMDIType())
                {
                    bool rc = AmdtCpuProfiling::sessionViewCreator()->displayExistingView(mdiEvent);
                    GT_ASSERT(rc)
                }

                break;
            }

            default:
                // Do nothing...
                break;
        }
    }
}

