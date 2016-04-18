//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afSingletonsDelete.cpp
///
//==================================================================================

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afDocUpdateManager.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGeneralViewsCreator.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afNewProjectDialog.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTApplicationFramework/Include/afSourceCodeViewsManager.h>

//#include <AMDTApplicationFramework/Include/afSoftwareUpdaterWindow.h>
#include <AMDTApplicationFramework/Include/dialogs/afGlobalSettingsDialog.h>
#include <src/afProcessDebuggerEventHandler.h>
#include <src/afProcessDebuggerPendingEventEvent.h>
#include <src/afSingletonsDelete.h>

//#include <AMDTApplicationComponents/Include/acSoftwareUpdaterProxySetting.h>

static afSingletonsDelete afSingletonsDeleteInstance;


// ---------------------------------------------------------------------------
// Name:        afSingletonsDelete::afSingletonsDelete
// Description: Constructor
// Author:      Uri Shomroni
// Date:        12/9/2012
// ---------------------------------------------------------------------------
afSingletonsDelete::afSingletonsDelete()
{

}


// ---------------------------------------------------------------------------
// Name:        afSingletonsDelete::~afSingletonsDelete
// Description: Destructor. Deletes the singletons.
// Author:      Uri Shomroni
// Date:        12/9/2012
// ---------------------------------------------------------------------------
afSingletonsDelete::~afSingletonsDelete()
{
    //OS_OUTPUT_DEBUG_LOG(L"Before deleting acSoftwareUpdaterProxySetting", OS_DEBUG_LOG_DEBUG);
    //delete acSoftwareUpdaterProxySetting::m_spMySingleInstance;
    //acSoftwareUpdaterProxySetting::m_spMySingleInstance = nullptr;
    //OS_OUTPUT_DEBUG_LOG(L"After deleting acSoftwareUpdaterProxySetting", OS_DEBUG_LOG_DEBUG);

    OS_OUTPUT_DEBUG_LOG(L"Before deleting afProcessDebuggerEventHandler", OS_DEBUG_LOG_DEBUG);
    delete afProcessDebuggerEventHandler::m_pMySingleInstance;
    afProcessDebuggerEventHandler::m_pMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting afProcessDebuggerEventHandler", OS_DEBUG_LOG_DEBUG);

    OS_OUTPUT_DEBUG_LOG(L"Before deleting afProcessDebuggerPendingEventEvent", OS_DEBUG_LOG_DEBUG);
    delete afProcessDebuggerPendingEventEvent::m_pMySingleInstance;
    afProcessDebuggerPendingEventEvent::m_pMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting afProcessDebuggerPendingEventEvent", OS_DEBUG_LOG_DEBUG);

    OS_OUTPUT_DEBUG_LOG(L"Before afApplicationCommands::cleanupInstance", OS_DEBUG_LOG_DEBUG);
    afApplicationCommands::cleanupInstance();
    OS_OUTPUT_DEBUG_LOG(L"After afApplicationCommands::cleanupInstance", OS_DEBUG_LOG_DEBUG);

    OS_OUTPUT_DEBUG_LOG(L"Before deleting afExecutionModeManager", OS_DEBUG_LOG_DEBUG);
    delete afExecutionModeManager::m_spMySingleInstance;
    afExecutionModeManager::m_spMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting afExecutionModeManager", OS_DEBUG_LOG_DEBUG);

    OS_OUTPUT_DEBUG_LOG(L"Before deleting afPluginConnectionManager", OS_DEBUG_LOG_DEBUG);
    delete afPluginConnectionManager::m_spMySingleInstance;
    afPluginConnectionManager::m_spMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting afPluginConnectionManager", OS_DEBUG_LOG_DEBUG);

    OS_OUTPUT_DEBUG_LOG(L"Before afProgressBarWrapper::cleanupInstance", OS_DEBUG_LOG_DEBUG);
    afProgressBarWrapper::cleanupInstance();
    OS_OUTPUT_DEBUG_LOG(L"After afProgressBarWrapper::cleanupInstance", OS_DEBUG_LOG_DEBUG);

    OS_OUTPUT_DEBUG_LOG(L"Before deleting afProjectManager", OS_DEBUG_LOG_DEBUG);
    delete afProjectManager::m_spMySingleInstance;
    afProjectManager::m_spMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting afProjectManager", OS_DEBUG_LOG_DEBUG);

    OS_OUTPUT_DEBUG_LOG(L"Before deleting afQtCreatorsManager", OS_DEBUG_LOG_DEBUG);
    delete afQtCreatorsManager::m_spMySingleInstance;
    afQtCreatorsManager::m_spMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting afQtCreatorsManager", OS_DEBUG_LOG_DEBUG);

    OS_OUTPUT_DEBUG_LOG(L"Before deleting afSourceCodeViewsManager", OS_DEBUG_LOG_DEBUG);
    delete afSourceCodeViewsManager::_pMySingleInstance;
    afSourceCodeViewsManager::_pMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting afSourceCodeViewsManager", OS_DEBUG_LOG_DEBUG);

    OS_OUTPUT_DEBUG_LOG(L"Before deleting afDocUpdateManager", OS_DEBUG_LOG_DEBUG);
    delete afDocUpdateManager::m_pMySingleInstance;
    afDocUpdateManager::m_pMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting afDocUpdateManager", OS_DEBUG_LOG_DEBUG);

    OS_OUTPUT_DEBUG_LOG(L"Before deleting afGeneralViewsCreator", OS_DEBUG_LOG_DEBUG);
    delete afGeneralViewsCreator::m_spMySingleInstance;
    afGeneralViewsCreator::m_spMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting afGeneralViewsCreator", OS_DEBUG_LOG_DEBUG);

    OS_OUTPUT_DEBUG_LOG(L"Before deleting afGlobalVariablesManager", OS_DEBUG_LOG_DEBUG);
    delete afGlobalVariablesManager::m_pMySingleInstance;
    afGlobalVariablesManager::m_pMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting afGlobalVariablesManager", OS_DEBUG_LOG_DEBUG);

}

