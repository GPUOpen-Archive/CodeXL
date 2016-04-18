//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspPowerProfilingEditorManager.cpp
///
//==================================================================================

//------------------------------ vspPowerProfilingEditorManager.cpp ------------------------------

#include "stdafx.h"

#include <algorithm>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apProfileProcessTerminatedEvent.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>
#include <AMDTApplicationFramework/Include/views/afPropertiesView.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SessionTreeNodeData.h>

// Power profiling
#include <AMDTPowerProfiling/src/ppSessionView.h>
#include <AMDTPowerProfiling/src/ppAppController.h>

// Local:
#include <Include/vscProfileSessionEditorDocumentQt.h>
#include <Include/Public/vscProfileSessionEditorDocument.h>
#include <src/vspPowerProfilingEditorManager.h>
#include <src/vspPackageWrapper.h>
#include <src/vspWindowsManager.h>


// Static members initializations:
vspPowerProfilingEditorManager* vspPowerProfilingEditorManager::m_psMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        vspPowerProfilingEditorManager::vspPowerProfilingEditorManager
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        17/9/2014
// ---------------------------------------------------------------------------
vspPowerProfilingEditorManager::vspPowerProfilingEditorManager()
{
    // Register as an events observer:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);

    m_pPPWindowsManager = new vspPowerProfilingEditorManagerHelper;

    // Set the windows helper:
    ppAppController::instance().SetWindowsHelper(m_pPPWindowsManager);

}

// ---------------------------------------------------------------------------
// Name:        vspPowerProfilingEditorManager::~vspPowerProfilingEditorManager
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        17/9/2014
// ---------------------------------------------------------------------------
vspPowerProfilingEditorManager::~vspPowerProfilingEditorManager()
{
}

// ---------------------------------------------------------------------------
// Name:        vspPowerProfilingEditorManager::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Return Val:  vspPowerProfilingEditorManager&
// Author:      Gilad Yarnitzky
// Date:        17/9/2014
// ---------------------------------------------------------------------------
vspPowerProfilingEditorManager& vspPowerProfilingEditorManager::instance()
{
    // If my single instance was not created yet - create it:
    if (m_psMySingleInstance == NULL)
    {
        m_psMySingleInstance = new vspPowerProfilingEditorManager;
    }

    return *m_psMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        vspPowerProfilingEditorManager::onEvent
// Description: Is called when a debugged process event occurs.
// Arguments:   const apEvent& eve
//              bool& vetoEvent
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        17/9/2014
// ---------------------------------------------------------------------------
void vspPowerProfilingEditorManager::onEvent(const apEvent& eve, bool& vetoEvent)
{
    GT_UNREFERENCED_PARAMETER(vetoEvent);

    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // handle the Global var changed event
    switch (eventType)
    {
        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            const afGlobalVariableChangedEvent& globalVarChangedEvent = dynamic_cast<const afGlobalVariableChangedEvent&>(eve);
            // Get id of the global variable that was changed
            afGlobalVariableChangedEvent::GlobalVariableId variableId = globalVarChangedEvent.changedVariableId();

            // If the project file path was changed
            if (variableId == afGlobalVariableChangedEvent::CURRENT_PROJECT)
            {
                ppAppController::instance().ProjectOpened();

                // Load all the open session windows:
                ProfileMDIWindowHelper::instance().LoadOpenedPowerProfileSessions();
            }
        }
        break;

        case apEvent::GD_MONITORED_OBJECT_ACTIVATED_EVENT:
        {
            // Get the activation event:
            const apMonitoredObjectsTreeActivatedEvent& activationEvent = (const apMonitoredObjectsTreeActivatedEvent&)eve;

            // Get the item data;
            afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)activationEvent.selectedItemData();

            if (pItemData != NULL)
            {
                ppSessionTreeNodeData* pPPData = qobject_cast<ppSessionTreeNodeData*>(pItemData->extendedItemData());

                if (pPPData != NULL)
                {
                    // Display the item (is this is a debugger tree node):
                    bool rc = activateItemInVS(pItemData->m_pTreeWidgetItem);
                    GT_ASSERT(rc);
                }
            }
        }
        break;

        case apEvent::AP_PROFILE_PROCESS_TERMINATED:
        {
            const apProfileProcessTerminatedEvent& profileProcessTerminateEvent = dynamic_cast<const apProfileProcessTerminatedEvent&>(eve);

            if (profileProcessTerminateEvent.profilerName() == PP_STR_dbFileExt)
            {
                SharedProfileManager::instance().stopCurrentRun();
            }
        }
        break;

        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspPowerProfilingEditorManager::activateIteminVS
// Description: activate item when working in VS
// Arguments:   QTreeWidgetItem* pItemToActivate
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        17/9/2014
// ---------------------------------------------------------------------------
bool vspPowerProfilingEditorManager::activateItemInVS(QTreeWidgetItem* pItemToActivate)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((NULL != pItemToActivate) && (afApplicationCommands::instance() != NULL) && (afApplicationCommands::instance()->applicationTree() != NULL))
    {
        afApplicationTreeItemData* pActivatedItemData = afApplicationCommands::instance()->applicationTree()->getTreeItemData(pItemToActivate);

        // Try to downcast to power profile session data, and only open if this is a power profile item:
        ppSessionTreeNodeData* pExtendedData = nullptr;
        GT_IF_WITH_ASSERT(nullptr != pActivatedItemData)
        {
            pExtendedData = qobject_cast<ppSessionTreeNodeData*>(pActivatedItemData->extendedItemData());
        }

        if (nullptr != pExtendedData && nullptr != pActivatedItemData)
        {
            osFilePath itemFilePath = pActivatedItemData->m_filePath;
            afTreeItemType itemType = pActivatedItemData->m_itemType;

            if ((AF_TREE_ITEM_PP_SUMMARY == itemType) || (AF_TREE_ITEM_PP_TIMELINE == itemType) || (AF_TREE_ITEM_PROFILE_SESSION == itemType) || (AF_TREE_ITEM_PROFILE_EMPTY_SESSION == itemType))
            {
                afApplicationCommands* pCommands = afApplicationCommands::instance();
                GT_IF_WITH_ASSERT(nullptr != pCommands)
                {
                    retVal = pCommands->OpenFileAtLine(itemFilePath, 0, -1);
                }

                if (retVal)
                {
                    // Find the session view for this session:
                    ppSessionView* pSessionView = NULL;

                    // Find the widget related to this session:
                    for (int i = 0; i < (int)ProfileMDIWindowHelper::instance().m_documents.size(); i++)
                    {
                        vscProfileSessionEditorDocument* pDoc = (vscProfileSessionEditorDocument*)ProfileMDIWindowHelper::instance().m_documents[i];
                        GT_IF_WITH_ASSERT(pDoc != NULL)
                        {
                            if (pDoc->m_filePath == itemFilePath)
                            {
                                pSessionView = qobject_cast<ppSessionView*>(pDoc->m_pInnerView);
                            }
                        }
                    }

                    GT_IF_WITH_ASSERT(pSessionView != NULL)
                    {
                        if (AF_TREE_ITEM_PP_SUMMARY == itemType)
                        {
                            pSessionView->DisplayTab(PP_TAB_SUMMARY_INDEX);
                        }
                        else
                        {
                            pSessionView->DisplayTab(PP_TAB_TIMELINE_INDEX);
                        }
                    }
                }
            }
        }

    }

    return retVal;
}

// ---------------------------------------------------------------------------
vspPowerProfilingEditorManagerHelper::vspPowerProfilingEditorManagerHelper()
{
    bool rc = connect(&afProjectManager::instance(), SIGNAL(OnClearCurrentProjectSettings()), SLOT(onClearProjectSetting()));
    GT_ASSERT(rc);
}


// ---------------------------------------------------------------------------
afRunModes vspPowerProfilingEditorManager::getCurrentRunModeMask()
{
    afRunModes retVal = 0;

    //if (kaBackendManager::instance().isInBuild())
    //{
    //    retVal = AF_ANALYZE_CURRENTLY_BUILDING;
    //}

    return retVal;

}

// ---------------------------------------------------------------------------
bool vspPowerProfilingEditorManager::canStopCurrentRun()
{
    return true;
}

// ---------------------------------------------------------------------------
bool vspPowerProfilingEditorManager::stopCurrentRun()
{
    return true;
}

// ---------------------------------------------------------------------------
bool vspPowerProfilingEditorManager::getExceptionEventDetails(const apExceptionEvent& exceptionEve, osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce)
{
    (void)(&exceptionEve); // unused
    (void)(exceptionCallStack); // unused
    (void)(openCLEnglineLoaded); // unused
    (void)(openGLEnglineLoaded); // unused
    (void)(kernelDebuggingEnteredAtLeastOnce); // unused
    return true;
}


// ---------------------------------------------------------------------------
void vspPowerProfilingEditorManagerHelper::onClearProjectSetting()
{
}

bool vspPowerProfilingEditorManagerHelper::ActivateSessionWindow(SessionTreeNodeData* pSessionData)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((pSessionData != NULL) && (pSessionData->m_pParentData != nullptr))
    {
        // Find the session view for this session:
        ppSessionView* pSessionView = NULL;

        // Find the widget related to this session:
        int documentsCount = (int)ProfileMDIWindowHelper::instance().m_documents.size();

        for (int i = 0; i < documentsCount; i++)
        {
            // Down cast the current document void* to a vscProfileSessionEditorDocument:
            vscProfileSessionEditorDocument* pDoc = (vscProfileSessionEditorDocument*)ProfileMDIWindowHelper::instance().m_documents[i];
            GT_IF_WITH_ASSERT(pDoc != NULL)
            {
                if (pDoc->m_filePath == pSessionData->m_pParentData->m_filePath)
                {
                    pSessionView = qobject_cast<ppSessionView*>(pDoc->m_pInnerView);
                    break;
                }
            }
        }

        // Activate the session window:
        GT_IF_WITH_ASSERT(pSessionView != NULL)
        {
            pSessionView->ActivateSession();
            retVal = true;
        }
    }

    return retVal;
}
