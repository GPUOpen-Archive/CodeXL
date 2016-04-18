//------------------------------ kaEventObserver.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>
#include <AMDTApplicationFramework/Include/views/afPropertiesView.h>
#include <AMDTApplicationFramework/Include/views/afSourceCodeView.h>

// Local:
#include <AMDTKernelAnalyzer/Include/kaAppWrapper.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTKernelAnalyzer/src/kaBackendManager.h>
#include <AMDTKernelAnalyzer/src/kaBuildToolbar.h>
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaApplicationTreeHandler.h>
#include <AMDTKernelAnalyzer/src/kaEventObserver.h>
#include <AMDTKernelAnalyzer/src/kaKernelView.h>
#include <AMDTKernelAnalyzer/src/kaMultiSourceView.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaSourceCodeView.h>
#include <AMDTKernelAnalyzer/src/kaTreeDataExtension.h>
#include <AMDTKernelAnalyzer/src/kaUtils.h>

// ---------------------------------------------------------------------------
// Name:        kaEventObserver::kaEventObserver
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
kaEventObserver::kaEventObserver() : m_pApplicationCommands(nullptr), m_pApplicationTree(nullptr)
{
    // Register as an events observer:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
}

// ---------------------------------------------------------------------------
// Name:        kaEventObserver::~kaEventObserver
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
kaEventObserver::~kaEventObserver()
{
    // Register as an events observer:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

// ---------------------------------------------------------------------------
// Name:        kaEventObserver::onEvent
// Description: Is called when a debugged process event occurs.
// Arguments:   const apEvent& eve
//              bool& vetoEvent
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
void kaEventObserver::onEvent(const apEvent& eve, bool& vetoEvent)
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
                if (nullptr == m_pApplicationTree)
                {
                    getApplicationTree();
                }

                // For empty projects, clear both project data manager and tree:
                if (afProjectManager::instance().currentProjectSettings().projectName().isEmpty())
                {
                    KA_PROJECT_DATA_MGR_INSTANCE.ClearAllAndDelete();
                    kaApplicationTreeHandler::instance()->ClearTree();

                    // clear tool bar
                    kaBuildToolbar* pKAToolBar = kaAppWrapper::instance().buildToolbar();
                    GT_IF_WITH_ASSERT(nullptr != pKAToolBar)
                    {
                        pKAToolBar->ClearToolBar();
                    }
                }

                // Check if this is a KA Mode:
                bool isInKAMode = afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode);

                if (isInKAMode)
                {
                    if (kaApplicationTreeHandler::instance()->ShouldCreateDefaultKernel())
                    {
                        QString defaultName(KA_STR_createDefaultName);
                        QString defaultExt(KA_STR_createDefaultName);
                        kaApplicationCommands::instance().CreateDefaultFile(defaultName, defaultExt);
                    }
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

            if (pItemData != nullptr)
            {
                kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

                if (pKAData != nullptr)
                {
                    // Display the item (is this is a debugger tree node):
                    bool rc = activateItem(pItemData->m_pTreeWidgetItem);
                    GT_ASSERT(rc);
                }
            }
        }
        break;

        case apEvent::AP_MDI_ACTIVATED_EVENT:
        {
            const apMDIViewActivatedEvent& activationEvent = (const apMDIViewActivatedEvent&)eve;
            OnMdiActivateEvent(activationEvent);
        }
        break;

        case apEvent::AP_MDI_CREATED_EVENT:
        {
            const apMDIViewCreateEvent& creationEvent = (const apMDIViewCreateEvent&)eve;
            RegisterToSaveSignal(creationEvent);
        }
        break;

        case apEvent::AP_EXECUTION_MODE_CHANGED_EVENT:
        {
            const apExecutionModeChangedEvent& modeEvent = (const apExecutionModeChangedEvent&)eve;

            if (modeEvent.modeType() == KA_STR_executionMode)
            {
                // Only handle the kernel creation when a project is loaded to the KA extension (otherwise is will be handled in the project change event):
                bool isProjectLoaded = !afProjectManager::instance().currentProjectFilePath().isEmpty();

                if (isProjectLoaded)
                {
                    if (kaApplicationTreeHandler::instance()->ShouldCreateDefaultKernel() && isProjectLoaded)
                    {
                        QString defaultName(KA_STR_createDefaultName);
                        QString defaultExt(KA_STR_createDefaultName);
                        kaApplicationCommands::instance().CreateDefaultFile(defaultName, defaultExt);
                    }
                }
            }
        }
        break;

        default:
            break;
    }
}


// ---------------------------------------------------------------------------
// Name:        kaEventObserver::getApplicationTree
// Description: Get the application tree
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
void kaEventObserver::getApplicationTree()
{
    // Get the single instance of the application commands object:
    m_pApplicationCommands = afApplicationCommands::instance();
    GT_ASSERT(nullptr != m_pApplicationCommands);
    m_pApplicationTree = m_pApplicationCommands->applicationTree();
    GT_ASSERT(nullptr != m_pApplicationTree);
}

// ---------------------------------------------------------------------------
// Name:        kaEventObserver::getTreeItemData
// Description:
// Arguments:   QTreeWidgetItem* pTreeItem
// Return Val:  gdDebugApplicationTreeData*
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
afApplicationTreeItemData* kaEventObserver::getTreeItemData(QTreeWidgetItem* pTreeItem) const
{
    afApplicationTreeItemData* pRetVal = nullptr;
    GT_IF_WITH_ASSERT(pTreeItem != nullptr)
    {

        QVariant itemData = pTreeItem->data(0, Qt::UserRole);
        pRetVal = (afApplicationTreeItemData*)itemData.value<void*>();
    }
    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        kaEventObserver::getApplicationTree
// Description: handle the activation of the tree item
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
bool kaEventObserver::activateItem(QTreeWidgetItem* pItemToActivate)
{
    bool retVal = false;

    // Get the tree item data:
    kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();
    afApplicationTreeItemData* pActivatedItemData = getTreeItemData(pItemToActivate);
    kaTreeDataExtension* pExtendedData = nullptr;
    GT_IF_WITH_ASSERT(nullptr != pActivatedItemData)
    {
        pExtendedData = qobject_cast<kaTreeDataExtension*>(pActivatedItemData->extendedItemData());
    }
    GT_IF_WITH_ASSERT((nullptr != pExtendedData) && (nullptr != pActivatedItemData->m_pTreeWidgetItem) && (nullptr != pTreeHandler))
    {
        // Get the activated object type:
        afTreeItemType itemType = pActivatedItemData->m_itemType;
        retVal = true;

        switch (itemType)
        {
            case AF_TREE_ITEM_KA_FILE:
            case AF_TREE_ITEM_KA_SOURCE:
            case AF_TREE_ITEM_KA_PROGRAM_GL_GEOM:
            case AF_TREE_ITEM_KA_PROGRAM_GL_FRAG:
            case AF_TREE_ITEM_KA_PROGRAM_GL_TESC:
            case AF_TREE_ITEM_KA_PROGRAM_GL_TESE:
            case AF_TREE_ITEM_KA_PROGRAM_GL_VERT:
            case AF_TREE_ITEM_KA_PROGRAM_GL_COMP:
            case AF_TREE_ITEM_KA_PROGRAM_SHADER:
            {
                kaApplicationCommands::instance().OpenSourceFile(pExtendedData, pExtendedData->lineNumber());

            }
            break;

            case AF_TREE_ITEM_KA_OVERVIEW:
            {
                gtString overViewTitle;
                pExtendedData->filePath().getFileName(overViewTitle);
                overViewTitle += KA_STR_overViewTitle;
                // Identify file path must match the Identify file path in kaMDIViewCreator:
                osFilePath identifyFilePath = pExtendedData->identifyFilePath();
                apMDIViewCreateEvent kaViewEvent(AF_STR_KernelAnalyzerViewsCreatorID, identifyFilePath, overViewTitle, 0, 0);
                kaViewEvent.SetItemData(pItemToActivate);
                apEventsHandler::instance().registerPendingDebugEvent(kaViewEvent);
            }
            break;

            case AF_TREE_ITEM_KA_STATISTICS:
            case AF_TREE_ITEM_KA_ANALYSIS:
            case AF_TREE_ITEM_KA_DEVICE:
            {
                QTreeWidgetItem* pParentTreeNode = pActivatedItemData->m_pTreeWidgetItem->parent();
                GT_IF_WITH_ASSERT(pParentTreeNode != nullptr)
                {
                    // title is "kernel name (file name) - bitness"
                    //// Get kernelName:
                    QString kernelName = kaUtils::GetKernelNameFromPath(pExtendedData->detailedFilePath());

                    //// Get file name:
                    gtString fileName;
                    pExtendedData->filePath().getFileNameAndExtension(fileName);

                    //// Get bitness
                    gtString outDirFolderName = kaUtils::ToGtString(boost::filesystem::path(pExtendedData->detailedFilePath().asString().asCharArray()).parent_path().filename());
                    gtString bitness  = KA_STR_FileSystemOutputDir32 == outDirFolderName ? gtString().fromASCIIString(KA_STR_ExportBinariesDialogBittness32CheckboxText) :
                                        gtString().fromASCIIString(KA_STR_ExportBinariesDialogBittness64CheckboxText);
                    //// Format the caption:
                    gtString overViewTitle;
                    overViewTitle.appendFormattedString(KA_STR_mdiCaptionFormat, kernelName.toStdWString().c_str(), fileName.asCharArray(), bitness.asCharArray());

                    // Identify file path must match the Identify file path in kaMDIViewCreator:
                    osFilePath identifyFilePath = pExtendedData->identifyFilePath();
                    apMDIViewCreateEvent kaViewEvent(AF_STR_KernelAnalyzerViewsCreatorID, identifyFilePath, overViewTitle, 0, 0);
                    kaViewEvent.SetItemData((void*)pItemToActivate);
                    apEventsHandler::instance().registerPendingDebugEvent(kaViewEvent);
                }
            }
            break;

            /*             AF_TREE_ITEM_KA_FILE_ROOT,
            AF_TREE_ITEM_KA_EXE_FILE,
            AF_TREE_ITEM_KA_SOURCE,
            AF_TREE_ITEM_KA_KERNEL,
            AF_TREE_ITEM_KA_HISTORY,*/

            default:
                break;
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
afRunModes kaEventObserver::getCurrentRunModeMask()
{
    afRunModes retVal = 0;

    if (kaBackendManager::instance().isInBuild())
    {
        retVal = AF_ANALYZE_CURRENTLY_BUILDING;
    }

    return retVal;

}

// ---------------------------------------------------------------------------
bool kaEventObserver::canStopCurrentRun()
{
    return true;
}

// ---------------------------------------------------------------------------
bool kaEventObserver::stopCurrentRun()
{
    return true;
}

// ---------------------------------------------------------------------------
bool kaEventObserver::getExceptionEventDetails(const apExceptionEvent& exceptionEve, osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce)
{
    (void)(&exceptionEve); // unused
    (void)(exceptionCallStack); // unused
    (void)(openCLEnglineLoaded); // unused
    (void)(openGLEnglineLoaded); // unused
    (void)(kernelDebuggingEnteredAtLeastOnce); // unused
    return true;
}

void kaEventObserver::RegisterToSaveSignal(const apMDIViewCreateEvent& createEvent)
{
    osFilePath filePath = createEvent.filePath();

    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pMainWindow != nullptr)
    {
        // Get the MDI sub window:
        afQMdiSubWindow* pSubWindow = pMainWindow->findMDISubWindow(createEvent.filePath());

        if (pSubWindow != nullptr)
        {
            GT_IF_WITH_ASSERT(pSubWindow->widget() != nullptr)
            {
                gtASCIIString className(pSubWindow->widget()->metaObject()->className());

                if (className == "afSourceCodeView")
                {
                    bool rc = QObject::connect(pSubWindow->widget(), SIGNAL(DocumentSaved(QString)), &KA_PROJECT_DATA_MGR_INSTANCE, SLOT(OnDocumentSaved(QString)));
                    GT_ASSERT(rc);
                }
            }
        }
    }
}

void kaEventObserver::OnMdiActivateEvent(const apMDIViewActivatedEvent& activateEvent)
{
    // Update the toolbar
    kaAppWrapper::buildToolbar()->updateUIonMDI(activateEvent.filePath());

    // Set focus to the correct tree item if needed:
    osFilePath filePath = activateEvent.filePath();

    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pMainWindow != nullptr)
    {
        // Get the MDI sub window:
        afQMdiSubWindow* pSubWindow = pMainWindow->findMDISubWindow(activateEvent.filePath());

        if (pSubWindow != nullptr)
        {
            filePath.clear();//reuse var here again
            // two cases are handled here: afSourceCodeView and kaMultiSource
            afSourceCodeView* pSourceView = qobject_cast<afSourceCodeView*>(pSubWindow->widget());

            if (pSourceView != nullptr)
            {
                filePath = pSourceView->filePath();
            }
            else
            {
                kaKernelView* pMultiView = qobject_cast<kaKernelView*>(pSubWindow->widget());

                if (pMultiView != nullptr)
                {
                    kaMultiSourceView* pMultiSourceView = pMultiView->GetActiveMultiSourceView();

                    if (pMultiSourceView != nullptr && pMultiSourceView->SourceView() != nullptr)
                    {
                        pMultiSourceView->UpdateDirtyViewsOnSubWindowChange();
                        filePath = pMultiSourceView->SourceView()->filePath();
                    }
                }
            }

            if (!filePath.asString().isEmpty())
            {
                kaApplicationTreeHandler::instance()->selectFileNode(filePath);
            }
        }
    }
}
