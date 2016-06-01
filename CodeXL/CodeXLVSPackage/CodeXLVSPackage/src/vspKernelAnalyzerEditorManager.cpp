//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspKernelAnalyzerEditorManager.cpp
///
//==================================================================================

//------------------------------ vspKernelAnalyzerEditorManager.cpp ------------------------------

#include "stdafx.h"

#include <algorithm>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>
#include <AMDTApplicationFramework/Include/views/afPropertiesView.h>

// Local:
#include <src/vspKernelAnalyzerEditorManager.h>
#include <src/vspPackageWrapper.h>
#include <src/vspWindowsManager.h>

// VS:
#include <Include/Public/vscVspDTEInvoker.h>
#include <Include/Public/vscPackageCommandHandler.h>

// Kernel Analyzer
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTKernelAnalyzer/src/kaApplicationTreeHandler.h>
#include <AMDTKernelAnalyzer/src/kaBackendManager.h>
#include <AMDTKernelAnalyzer/src/kaKernelView.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaTreeDataExtension.h>
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>

// Static members initializations:
vspKernelAnalyzerEditorManager* vspKernelAnalyzerEditorManager::m_psMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        vspKernelAnalyzerEditorManager::vspKernelAnalyzerEditorManager
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
vspKernelAnalyzerEditorManager::vspKernelAnalyzerEditorManager(): m_pActiveView(NULL), m_pApplicationTree(NULL)
{
    // Register as an events observer:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);

    m_pQtHelper = new vspKernelAnalyzerEditorManagerHelper;
    // connect to the output of the backend manager:
}

// ---------------------------------------------------------------------------
// Name:        vspKernelAnalyzerEditorManager::~vspKernelAnalyzerEditorManager
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
vspKernelAnalyzerEditorManager::~vspKernelAnalyzerEditorManager()
{
    delete m_pQtHelper;
}

// ---------------------------------------------------------------------------
// Name:        vspKernelAnalyzerEditorManager::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Return Val:  vspKernelAnalyzerEditorManager&
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
vspKernelAnalyzerEditorManager& vspKernelAnalyzerEditorManager::instance()
{
    // If my single instance was not created yet - create it:
    if (m_psMySingleInstance == NULL)
    {
        m_psMySingleInstance = new vspKernelAnalyzerEditorManager;

    }

    return *m_psMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        vspKernelAnalyzerEditorManager::onEvent
// Description: Is called when a debugged process event occurs.
// Arguments:   const apEvent& eve
//              bool& vetoEvent
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
void vspKernelAnalyzerEditorManager::onEvent(const apEvent& eve, bool& vetoEvent)
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
                gtString newCurProjectName = afProjectManager::instance().currentProjectSettings().projectName();

                if (m_curProjectName != newCurProjectName)
                {
                    // if the change was during a build, make sure the build is stopped:
                    if (getCurrentRunModeMask() != 0)
                    {
                        kaBackendManager::instance().reset();
                    }

                    // save the name:
                    m_curProjectName = newCurProjectName;

                    if (NULL == m_pApplicationTree)
                    {
                        getApplicationTree();
                    }


                    // Get cl files from new project loaded and add to codeXL project
                    AddCLFilesToTree();
                }
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
                kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

                if (pKAData != NULL && pItemData->m_itemType != AF_TREE_ITEM_KA_ADD_FILE && pItemData->m_itemType != AF_TREE_ITEM_KA_NEW_FILE)
                {
                    // Display the item (is this is a debugger tree node):
                    bool rc = activateItemInVS(pItemData->m_pTreeWidgetItem);
                    GT_ASSERT(rc);
                }
            }
        }
        break;

        case apEvent::GD_MONITORED_OBJECT_SELECTED_EVENT:
        {
            afApplicationCommands::instance()->updateToolbarCommands();
        }
        break;

        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspKernelAnalyzerEditorManager::activateIteminVS
// Description: activate item when working in VS
// Arguments:   QTreeWidgetItem* pItemToActivate
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        27/8/2013
// ---------------------------------------------------------------------------
bool vspKernelAnalyzerEditorManager::activateItemInVS(QTreeWidgetItem* pItemToActivate)
{
    bool retVal = false;
    int lineNumber = 1;

    kaApplicationTreeHandler* pTreeHandler  = kaApplicationTreeHandler::instance();
    afApplicationTreeItemData* pActivatedItemData = getTreeItemData(pItemToActivate);
    kaTreeDataExtension* pExtendedData = nullptr;
    GT_IF_WITH_ASSERT(nullptr != pActivatedItemData)
    {
        pExtendedData = qobject_cast<kaTreeDataExtension*>(pActivatedItemData->extendedItemData());
    }
    GT_IF_WITH_ASSERT((nullptr != pItemToActivate->parent()) && (nullptr != pExtendedData) && (nullptr != pTreeHandler))
    {
        pTreeHandler->setActiveItemData(pActivatedItemData);

        osFilePath basePath;
        // Finish building exact file location and type based on the node:
        afTreeItemType itemType = pActivatedItemData->m_itemType;

        switch (itemType)
        {
            case AF_TREE_ITEM_KA_FILE:
            case AF_TREE_ITEM_KA_SOURCE:
            {
                basePath = pExtendedData->filePath();
                retVal = true;
            }
            break;

            case AF_TREE_ITEM_KA_PROGRAM_GL_GEOM:
            case AF_TREE_ITEM_KA_PROGRAM_GL_FRAG:
            case AF_TREE_ITEM_KA_PROGRAM_GL_TESC:
            case AF_TREE_ITEM_KA_PROGRAM_GL_TESE:
            case AF_TREE_ITEM_KA_PROGRAM_GL_VERT:
            case AF_TREE_ITEM_KA_PROGRAM_GL_COMP:
            case AF_TREE_ITEM_KA_PROGRAM_SHADER:
            {
                // for all these files always give the same name for the statistics so even if pressing
                // other files in the same sub tree it will not open different files
                basePath = pActivatedItemData->m_filePath;
                retVal = true;
            }
            break;
            case AF_TREE_ITEM_KA_OVERVIEW:
            case AF_TREE_ITEM_KA_STATISTICS:
            case AF_TREE_ITEM_KA_ANALYSIS:
            case AF_TREE_ITEM_KA_DEVICE:
            {
                // for all these files always give the same name for the statistics so even if pressing
                // other files in the same sub tree it will not open different files
                basePath = pExtendedData->identifyFilePath();
                retVal = true;
            }
            break;

            case AF_TREE_ITEM_KA_NEW_PROGRAM:
                retVal = true;
                break;

            default:
                break;
        }

        // Open the file:
        if (retVal && basePath.isEmpty() == false)
        {
            afApplicationCommands* pCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(nullptr != pCommands)
            {
                retVal = pCommands->OpenFileAtLine(basePath, lineNumber, -1);

                // post open file action in some of the nodes:
                if (retVal && (AF_TREE_ITEM_KA_STATISTICS == itemType || AF_TREE_ITEM_KA_ANALYSIS == itemType || AF_TREE_ITEM_KA_DEVICE == itemType))
                {
                    kaKernelView* pKernelView = qobject_cast<kaKernelView*>(activeView());

                    if (pKernelView != NULL)
                    {
                        loadData();
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaEventObserver::getApplicationTree
// Description: Get the application tree
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
void vspKernelAnalyzerEditorManager::getApplicationTree()
{
    // Get the single instance of the application commands object:
    m_pApplicationCommands = afApplicationCommands::instance();
    GT_ASSERT(NULL != m_pApplicationCommands);
    m_pApplicationTree = m_pApplicationCommands->applicationTree();
    GT_ASSERT(NULL != m_pApplicationTree);
}

// ---------------------------------------------------------------------------
// Name:        vspKernelAnalyzerEditorManager::getTreeItemData
// Description:
// Arguments:   QTreeWidgetItem* pTreeItem
// Return Val:  gdDebugApplicationTreeData*
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
afApplicationTreeItemData* vspKernelAnalyzerEditorManager::getTreeItemData(QTreeWidgetItem* pTreeItem) const
{
    afApplicationTreeItemData* pRetVal = NULL;
    GT_IF_WITH_ASSERT(pTreeItem != NULL)
    {

        QVariant itemData = pTreeItem->data(0, Qt::UserRole);
        pRetVal = (afApplicationTreeItemData*)itemData.value<void*>();
    }
    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        vspKernelAnalyzerEditorManager::loadData
// Description: Load the data to the view by setting the display file to the view and leting it handle it correctly
// Author:      Gilad Yarnitzky
// Date:        29/8/2013
// ---------------------------------------------------------------------------
void vspKernelAnalyzerEditorManager::loadData()
{
    // Get the last selected tree node that will hold the rest of the data we need to load:
    // the type of the node, and the kernel file
    kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();
    GT_IF_WITH_ASSERT(nullptr != pTreeHandler)
    {
        if (pTreeHandler->WasTreeCreated())
        {
            // Get the last activated item data:
            const afApplicationTreeItemData* pActivatedItemData = pTreeHandler->activeItemData();
            GT_IF_WITH_ASSERT(nullptr != pActivatedItemData)
            {
                kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pActivatedItemData->extendedItemData());
                GT_IF_WITH_ASSERT(nullptr != pKAData)
                {
                    kaKernelView* pKernelView = qobject_cast<kaKernelView*>(m_pActiveView);
                    GT_IF_WITH_ASSERT(nullptr != pKernelView)
                    {
                        osFilePath detailedPath = pKAData->detailedFilePath();

                        pKernelView->displayFile(detailedPath, pKAData->filePath(), pActivatedItemData->m_itemType, pActivatedItemData->m_pTreeWidgetItem->text(0));
                    }
                }
            }
        }
    }
}

void vspKernelAnalyzerEditorManager::LoadDataForFilePath(kaKernelView* pKernelView, const osFilePath& filePath)
{
    // Sanity check:
    if ((pKernelView != nullptr) && (afApplicationCommands::instance() != nullptr) && (afApplicationCommands::instance()->applicationTree() != nullptr))
    {
        // Iterate the tree and find the item that is related to this file path
        kaTreeDataExtension* pFilePathItemData = nullptr;

        QTreeWidgetItemIterator treeIter(afApplicationCommands::instance()->applicationTree()->treeControl());

        while (*treeIter)
        {
            QTreeWidgetItem* pCurrentItem = (*treeIter);
            afApplicationTreeItemData* pItemData = getTreeItemData(pCurrentItem);

            if (pItemData != nullptr)
            {
                kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

                if ((pKAData != nullptr) && (pKAData->identifyFilePath() == filePath))
                {
                    // Found the item data for the requested path
                    pFilePathItemData = pKAData;
                    break;
                }
            }

            ++treeIter;
        }

        if (pFilePathItemData != nullptr && pFilePathItemData->m_pParentData != nullptr)
        {
            osFilePath detailedPath = pFilePathItemData->detailedFilePath();

            pKernelView->displayFile(detailedPath, pFilePathItemData->filePath(), pFilePathItemData->m_pParentData->m_itemType, pFilePathItemData->m_pParentData->m_pTreeWidgetItem->text(0));
        }
    }
}

vspKernelAnalyzerEditorManagerHelper::vspKernelAnalyzerEditorManagerHelper()
{
    bool rc = connect(&kaBackendManager::instance(), SIGNAL(buildComplete(const gtString&)), this, SLOT(onBuildComplete(const gtString&)));
    GT_ASSERT(rc);

    rc = connect(&afProjectManager::instance(), SIGNAL(ExecutableChanged(const QString&, bool, bool)), this,  SLOT(updateViewsAfterProjectChanged(const QString&, bool, bool)));
    GT_ASSERT(rc);

    rc = connect(&afProjectManager::instance(), SIGNAL(OnClearCurrentProjectSettings()), SLOT(onClearProjectSetting()));
    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
void vspKernelAnalyzerEditorManager::addKernelView(kaKernelView* pView)
{
    m_kernelViews.push_back(pView);
}

// ---------------------------------------------------------------------------
void vspKernelAnalyzerEditorManager::removeKernelView(kaKernelView* pView)
{
    gtVector<kaKernelView*>::iterator itemToFind = gtFind(m_kernelViews.begin(), m_kernelViews.end(), pView);

    if (itemToFind != m_kernelViews.end())
    {
        m_kernelViews.erase(itemToFind);
    }
}

// ---------------------------------------------------------------------------
void vspKernelAnalyzerEditorManager::updateViewsAfterProjectChanged(const QString& projetcName, bool isFinal)
{
    GT_UNREFERENCED_PARAMETER(projetcName);
    GT_UNREFERENCED_PARAMETER(isFinal);

    gtVector<kaKernelView*>::iterator currentViewIt;

    for (currentViewIt = m_kernelViews.begin() ; currentViewIt != m_kernelViews.end() ; currentViewIt++)
    {
        kaKernelView* pCurrentView = *currentViewIt;

        if (NULL != pCurrentView)
        {
            osFilePath viewPath = pCurrentView->dataFile();

            gtString dataFileExt;
            viewPath.getFileExtension(dataFileExt);

            if (dataFileExt == KA_STR_overviewExtension)
            {
                pCurrentView->updateOverview();
            }
            else if (dataFileExt == KA_STR_kernelViewExtension)
            {
                // Load the data for the requested file path
                LoadDataForFilePath(pCurrentView, pCurrentView->dataFile());
            }
        }
    }
}

// ---------------------------------------------------------------------------
void vspKernelAnalyzerEditorManager::onBuildComplete(const gtString& clFilePath)
{
    // signals that we really finished to build all files + ISA and IL
    if (clFilePath.isEmpty())
    {
        // Vector copy is used to fix crash because updateView erases vector elements.
        gtVector<kaKernelView*> vectorCopy(m_kernelViews);

        for (kaKernelView* pCurrentView : vectorCopy)
        {
            if (nullptr != pCurrentView)
            {
                pCurrentView->updateView();
            }
        }

        if (NULL != afApplicationCommands::instance())
        {
            // save the project
            afApplicationCommands::instance()->OnFileSaveProject();
            // update the UI
            afApplicationCommands::instance()->updateToolbarCommands();
        }
    }
}

// ---------------------------------------------------------------------------
afRunModes vspKernelAnalyzerEditorManager::getCurrentRunModeMask()
{
    afRunModes retVal = 0;

    if (kaBackendManager::instance().isInBuild())
    {
        retVal = AF_ANALYZE_CURRENTLY_BUILDING;
    }

    return retVal;

}

// ---------------------------------------------------------------------------
bool vspKernelAnalyzerEditorManager::canStopCurrentRun()
{
    return true;
}

// ---------------------------------------------------------------------------
bool vspKernelAnalyzerEditorManager::stopCurrentRun()
{
    return true;
}

// ---------------------------------------------------------------------------
bool vspKernelAnalyzerEditorManager::getExceptionEventDetails(const apExceptionEvent& exceptionEve, osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce)
{
    (void)(&exceptionEve); // unused
    (void)(exceptionCallStack); // unused
    (void)(openCLEnglineLoaded); // unused
    (void)(openGLEnglineLoaded); // unused
    (void)(kernelDebuggingEnteredAtLeastOnce); // unused
    return true;
}

// ---------------------------------------------------------------------------
void vspKernelAnalyzerEditorManager::AddCLFilesToTree()
{
    wchar_t** pClProgramFiles = NULL;
    int clProgramFilesItemCount = 0;
    gtVector<osFilePath> clProgramFiles;
    vscVspDTEInvoker_GetKernelSourceFilePath(pClProgramFiles, clProgramFilesItemCount, true);

    if (pClProgramFiles != NULL && clProgramFilesItemCount > 0)
    {
        // Create a vector of osFilePath objects.

        for (int i = 0; i < clProgramFilesItemCount; i++)
        {
            osFilePath filePath(pClProgramFiles[i] != NULL ? pClProgramFiles[i] : L"");
            clProgramFiles.push_back(filePath);
        }
    }

    kaApplicationCommands::instance().AddFilesToTree(clProgramFiles, false);
}

// ---------------------------------------------------------------------------
void vspKernelAnalyzerEditorManagerHelper::updateViewsAfterProjectChanged(const QString& projetcName, bool isFinal, bool isUserModelId)
{
    (void)isUserModelId; // not used
    vspKernelAnalyzerEditorManager::instance().updateViewsAfterProjectChanged(projetcName, isFinal);
}

// ---------------------------------------------------------------------------
void vspKernelAnalyzerEditorManagerHelper::onBuildComplete(const gtString& clFilePath)
{
    vspKernelAnalyzerEditorManager::instance().onBuildComplete(clFilePath);
}

// ---------------------------------------------------------------------------
void vspKernelAnalyzerEditorManagerHelper::onClearProjectSetting()
{
    kaProjectDataManager::instance().ClearAllAndDelete();

    kaApplicationTreeHandler* pKATreeHandler = kaApplicationTreeHandler::instance();
    GT_IF_WITH_ASSERT(nullptr != pKATreeHandler)
    {
        if (pKATreeHandler->WasTreeCreated())
        {
            pKATreeHandler->ClearTree();
            pKATreeHandler->SetItemsVisibility();
        }
    }
}
