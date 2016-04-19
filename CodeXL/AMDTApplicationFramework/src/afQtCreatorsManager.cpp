//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afQtCreatorsManager.cpp
///
//==================================================================================

// Qt:
#include <QtWidgets>

// infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationFramework/Include/afActionExecutorAbstract.h>
#include <AMDTApplicationFramework/Include/afMenuActionsExecutor.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afRecentProjectsActionsExecutor.h>
#include <AMDTApplicationFramework/Include/afViewCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>


// Static members initializations:
afQtCreatorsManager* afQtCreatorsManager::m_spMySingleInstance = nullptr;

// ---------------------------------------------------------------------------
// Name:        afQtCreatorsManager::afQtCreatorsManager
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        14/7/2011
// ---------------------------------------------------------------------------
afQtCreatorsManager::afQtCreatorsManager()
{
}

// ---------------------------------------------------------------------------
// Name:        afQtCreatorsManager::~afQtCreatorsManager
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        14/7/2011
// ---------------------------------------------------------------------------
afQtCreatorsManager::~afQtCreatorsManager()
{
}

// ---------------------------------------------------------------------------
// Name:        afQtCreatorsManager::instance
// Description: Get singleton instance
// Return Val:  afQtCreatorsManager&
// Author:      Gilad Yarnitzky
// Date:        14/7/2011
// ---------------------------------------------------------------------------
afQtCreatorsManager& afQtCreatorsManager::instance()
{
    // If my single instance was not created yet - create it:
    if (m_spMySingleInstance == nullptr)
    {
        m_spMySingleInstance = new afQtCreatorsManager;
        GT_ASSERT(m_spMySingleInstance);

        // Initialize:
        m_spMySingleInstance->initialize();
    }

    return *m_spMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        afQtCreatorsManager::registerWxViewCreator
// Description: Add a WX view creator to the manager
// Arguments:   afWxViewCreatorAbstract* viewCreator
// Author:      Gilad Yarnitzky
// Date:        14/7/2011
// ---------------------------------------------------------------------------
void afQtCreatorsManager::registerViewCreator(afViewCreatorAbstract* pViewCreator)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pViewCreator != nullptr)
    {
        m_viewsCreators.push_back(pViewCreator);
    }
}

// ---------------------------------------------------------------------------
// Name:        afQtCreatorsManager::registerActionExecutor
// Description: add an action creator to the manager
// Arguments:   afActionExecutorAbstract* pActionExecutor - the action executor
// Author:      Gilad Yarnitzky
// Date:        14/7/2011
// ---------------------------------------------------------------------------
void afQtCreatorsManager::registerActionExecutor(afActionExecutorAbstract* pActionExecutor)
{
    m_actionsExecutors.push_back(pActionExecutor);
}


// ---------------------------------------------------------------------------
// Name:        afQtCreatorsManager::initialize
// Description: Initialize the creator
// Author:      Sigal Algranaty
// Date:        15/4/2012
// ---------------------------------------------------------------------------
void afQtCreatorsManager::initialize()
{
    // Create the images and buffers actions creator:
    m_pMenuCommandsExecutor = new afMenuActionsExecutor;


    // Register the actions creator:
    afQtCreatorsManager::instance().registerActionExecutor(m_pMenuCommandsExecutor);

    // Create the recent project actions creator:
    m_pRecentProjectsActionsExecutor = new afRecentProjectsActionsExecutor;


    // Register the actions creator:
    afQtCreatorsManager::instance().registerActionExecutor(m_pRecentProjectsActionsExecutor);
    afProjectManager::instance().setRecentlyUsedActionsExecutor(m_pRecentProjectsActionsExecutor);
}
