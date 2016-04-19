//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppWindowsManagerHelper.cpp
///
//==================================================================================

//------------------------------ ppWindowsManagerHelper.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Power profiling:
#include <AMDTPowerProfiling/Include/ppWindowsManagerHelper.h>
#include <AMDTPowerProfiling/Include/ppAppWrapper.h>
#include <AMDTPowerProfiling/src/ppMDIViewCreator.h>

ppWindowsManagerHelper::ppWindowsManagerHelper()
{
}

ppWindowsManagerHelper::~ppWindowsManagerHelper()
{
}

bool ppWindowsManagerHelper::ActivateSessionWindow(SessionTreeNodeData* pSessionData)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(pSessionData != nullptr)
    {
        // Get the session view, and make him start listening to events:
        ppMDIViewCreator* pViewsCreator = ppAppWrapper::instance().MDIViewCreator();

        if (pViewsCreator != nullptr)
        {
            // Activate the session (find the relevant view and start listen to its events):
            retVal = pViewsCreator->ActivateSession(pSessionData);
        }
    }

    return retVal;
}
