//------------------------------ kaApplicationTreeHandler.h ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Framework:
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTApplicationFramework/Include/afDocUpdateManager.h>
#include <AMDTApplicationFramework/Include/afBrowseAction.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaApplicationTreeHandler.h>
#include <AMDTKernelAnalyzer/src/kaBackendManager.h>
#include <AMDTKernelAnalyzer/src/kaDataAnalyzerFunctions.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaTreeDataExtension.h>
#include <AMDTKernelAnalyzer/src/kaExportBinariesDialog.h>
#include <AMDTKernelAnalyzer/src/kaProgram.h>
#include <AMDTKernelAnalyzer/src/kaUtils.h>

#include <AMDTKernelAnalyzer/Include/kaAppWrapper.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>

const QColor STAGE_HINT_BGCOLOR = QColor(255, 255, 138);
const QColor STAGE_ORIG_BGCOLOR = QColor(255, 255, 255);
// Static member initializations
kaApplicationTreeHandler* kaApplicationTreeHandler::m_psMySingleInstance = nullptr;


typedef gtMap<std::pair<osFilePath, gtString>, gtString> DevicesAndBinFilesMap;

static void ExportBiniriesForSingleSourceFile(const kaExportBinariesDialog& dlg,
                                              const DevicesAndBinFilesMap& devicesAndBinFiles,
                                              const gtString& fileName,
                                              const afTreeItemType treeItemType,
                                              const gtString& selectedFolder,
                                              const bool is32Bit,
                                              bool& isFileWritingFailure,
                                              bool& bExportSucceeded);
static void BuildDeviceAndBinFilesMap(const kaProjectDataManager::FPathToOutFPathsMap& buildFilesMap32Bit, DevicesAndBinFilesMap& devicesAndBinFiles, gtSet<gtString>& devicesToExport);

static osFilePath GetExportFilePath(const osFilePath& originaFilePath,
                                    const gtString& baseFName,
                                    const afTreeItemType treeItemType,
                                    const gtString& deviceName,
                                    const bool is32Bit,
                                    const gtString& selectedFolder);




// ---------------------------------------------------------------------------
// Name:        kaApplicationTreeHandler::kaApplicationTreeHandler
// Description: Constructor.
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
kaApplicationTreeHandler::kaApplicationTreeHandler() : afApplicationTreeHandler(), m_pApplicationTree(nullptr),
    m_pProgramsRootNode(nullptr), m_pAddFileTreeItem(nullptr), m_pNewFileTreeItem(nullptr), m_pNewProgramTreeItem(nullptr),
    m_pActiveItemData(nullptr),
    m_pAddAction(nullptr), m_pNewAction(nullptr), m_pOpenAction(nullptr), m_pBuildAction(nullptr), m_pCancelBuildAction(nullptr),
    m_pExportAction(nullptr), m_pRemoveAction(nullptr), m_pAddShaderAction(nullptr),
    m_pCreateShaderAction(nullptr), m_pGotoSourceAction(nullptr),
    m_pOpenContainingFolderAction(nullptr), m_pOpenOutputFolderAction(nullptr), m_pRenameFileAction(nullptr), m_pNewProgramAction(nullptr),
    m_pExportToCSVAction(nullptr), m_pRenamedItem(nullptr), m_shouldCreateDefaultKernel(false), m_isRenaming(false), m_pCreatedSourceItemData(nullptr),
    m_shouldTakeRenameAction(false), m_pLastHintItem(nullptr)
{
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {
        m_pApplicationTree = pApplicationCommands->applicationTree();

        if (m_pApplicationTree != nullptr)
        {
            // TODO: CL file drag: add this stage, the application tree was not created yet.
            // Should add a signal to main window that the tree was created, and register all tree handler when handling this signal
            OnApplicationTreeCreated();
        }
        else
        {
            // The tree was not create yet, connect to the signal that it is created, and register then:
            bool rc = connect(&afQtCreatorsManager::instance(), SIGNAL(ApplicationTreeCreated()), this, SLOT(OnApplicationTreeCreated()));
            GT_ASSERT(rc);
        }
    }

    // initialize pixmaps:
    m_pIconsArray = new QPixmap[KA_PIXMAP_OBJECT_TYPES_AMOUNT];
    acSetIconInPixmap(m_pIconsArray[KA_PIXMAP_KERNELS],         AC_ICON_ANALYZE_APPTREE_KERNELS);
    acSetIconInPixmap(m_pIconsArray[KA_PIXMAP_VK_FOLDER],       AC_ICON_ANALYZE_APPTREE_FOLDER_VK);
    acSetIconInPixmap(m_pIconsArray[KA_PIXMAP_CL_FOLDER],       AC_ICON_ANALYZE_APPTREE_FOLDER_CL);
    acSetIconInPixmap(m_pIconsArray[KA_PIXMAP_GL_FOLDER],       AC_ICON_ANALYZE_APPTREE_FOLDER_GL);
    acSetIconInPixmap(m_pIconsArray[KA_PIXMAP_DX_FOLDER],       AC_ICON_ANALYZE_APPTREE_FOLDER_DX);
    acSetIconInPixmap(m_pIconsArray[KA_PIXMAP_SOURCE],          AC_ICON_ANALYZE_APPTREE_SOURCE);
    acSetIconInPixmap(m_pIconsArray[KA_PIXMAP_OVERVIEW],        AC_ICON_ANALYZE_APPTREE_OVERVIEW);
    acSetIconInPixmap(m_pIconsArray[KA_PIXMAP_KERNEL],          AC_ICON_ANALYZE_APPTREE_KERNEL);
    acSetIconInPixmap(m_pIconsArray[KA_PIXMAP_ANALYSIS],        AC_ICON_ANALYZE_APPTREE_ANALYSIS);
    acSetIconInPixmap(m_pIconsArray[KA_PIXMAP_STATISTICS],      AC_ICON_ANALYZE_APPTREE_STATISTICS);
    acSetIconInPixmap(m_pIconsArray[KA_PIXMAP_IL_FILE],         AC_ICON_ANALYZE_APPTREE_IL);
    acSetIconInPixmap(m_pIconsArray[KA_PIXMAP_ISA_FILE],        AC_ICON_ANALYZE_APPTREE_ISA);
    acSetIconInPixmap(m_pIconsArray[KA_PIXMAP_IL_ISA_FILE],     AC_ICON_ANALYZE_APPTREE_IL_ISA);
    acSetIconInPixmap(m_pIconsArray[KA_PIXMAP_ADD_FILE],        AC_ICON_ANALYZE_APPTREE_OPEN);
    acSetIconInPixmap(m_pIconsArray[KA_PIXMAP_NEW_FILE],        AC_ICON_ANALYZE_APPTREE_ADDNEW);
    acSetIconInPixmap(m_pIconsArray[KA_PIXMAP_FOLDER],          AC_ICON_ANALYZE_APPTREE_FOLDER);

    // Register as an events observer
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_MANAGERS_EVENTS_HANDLING_PRIORITY);

    // Connect the backend build complete signal to a slot:
    bool rc = connect(&kaBackendManager::instance(), SIGNAL(buildComplete(const gtString&)), this, SLOT(OnBuildComplete(const gtString&)));
    GT_ASSERT(rc);

    // Create the actions for the context menu:
    m_pAddAction = new QAction(KA_STR_addFileASCII, m_pApplicationTree);
    m_pNewAction = new QAction(KA_STR_newFileASCII, m_pApplicationTree);
    m_pNewProgramAction = new QAction(KA_STR_newProgramASCII, m_pApplicationTree);
    m_pOpenAction = new QAction(KA_STR_openclOpenFile, m_pApplicationTree);
    m_pBuildAction = new QAction(KA_STR_BuildProgramASCII, m_pApplicationTree);
    m_pCancelBuildAction = new QAction(KA_STR_CancelBuildASCII, m_pApplicationTree);
    m_pExportAction = new QAction(KA_STR_exportBinariesASCII, m_pApplicationTree);
    m_pRemoveAction = new QAction(KA_STR_removeFromProjectASCII, m_pApplicationTree);
    m_pAddShaderAction = new QAction(KA_STR_addShaderFromProgramASCII, m_pApplicationTree);
    m_pCreateShaderAction = new QAction(KA_STR_createShaderFromProgramASCII, m_pApplicationTree);
    m_pGotoSourceAction = new QAction(KA_STR_gotoSourceCodeASCII, m_pApplicationTree);
    m_pOpenContainingFolderAction = new QAction(KA_STR_openContainingFolderASCII, m_pApplicationTree);
    m_pOpenOutputFolderAction = new QAction(KA_STR_openOutputFolderASCII, m_pApplicationTree);
    m_pRenameFileAction = new QAction(KA_STR_renameFileStatusbarStringASCII, m_pApplicationTree);
    m_pExportToCSVAction = new afBrowseAction(KA_STR_exportToCSV);
    m_pExportToCSVAction->setText(KA_STR_exportToCSV);

    // Connect the context menu actions to the slots:
    rc = connect(m_pAddAction, SIGNAL(triggered()), this, SLOT(OnAddFile()));
    GT_ASSERT(rc);
    rc = connect(m_pNewProgramAction, SIGNAL(triggered()), this, SLOT(OnNewProgram()));
    GT_ASSERT(rc);
    rc = connect(m_pNewAction, SIGNAL(triggered()), this, SLOT(OnNewFile()));
    GT_ASSERT(rc);
    rc = connect(m_pOpenAction, SIGNAL(triggered()), this, SLOT(OnCLFileOpen()));
    GT_ASSERT(rc);
    rc = connect(m_pBuildAction, SIGNAL(triggered()), this, SLOT(OnBuild()));
    GT_ASSERT(rc);
    rc = connect(m_pCancelBuildAction, SIGNAL(triggered()), this, SLOT(OnCancelBuild()));
    GT_ASSERT(rc);
    rc = connect(m_pExportAction, SIGNAL(triggered()), this, SLOT(OnExportBinaries()));
    GT_ASSERT(rc);
    rc = connect(m_pRemoveAction, SIGNAL(triggered()), this, SLOT(OnRemove()));
    GT_ASSERT(rc);
    rc = connect(m_pAddShaderAction, SIGNAL(triggered()), this, SLOT(OnAddCreateShader()));
    GT_ASSERT(rc);
    rc = connect(m_pCreateShaderAction, SIGNAL(triggered()), this, SLOT(OnAddCreateShader()));
    GT_ASSERT(rc);
    rc = connect(m_pGotoSourceAction, SIGNAL(triggered()), this, SLOT(OnGotoSourceCode()));
    GT_ASSERT(rc);
    rc = connect(m_pOpenContainingFolderAction, SIGNAL(triggered()), this, SLOT(OnOpenContainingFolder()));
    GT_ASSERT(rc);
    rc = connect(m_pOpenOutputFolderAction, SIGNAL(triggered()), this, SLOT(OnOpenOutputFolder()));
    GT_ASSERT(rc);
    rc = connect(m_pRenameFileAction, SIGNAL(triggered()), this, SLOT(OnRename()));
    GT_ASSERT(rc);
    rc = connect(&(afProjectManager::instance()), SIGNAL(SourcePathChanged(gtString, gtString)), this, SLOT(OnSourcePathChanged(gtString, gtString)));
    GT_ASSERT(rc);
    rc = connect(m_pExportToCSVAction, SIGNAL(triggered()), SLOT(OnExportToCSVAction()));
    GT_ASSERT(rc);

    // Set shortcuts for some of the actions:
    m_pBuildAction->setShortcut(QKeySequence("F7"));
    m_pBuildAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_pCancelBuildAction->setShortcut(QKeySequence(AF_STR_CancelBuildShortcut));
    m_pCancelBuildAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_pRemoveAction->setShortcut(QKeySequence(QKeySequence::Delete));
    m_pRemoveAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_pRenameFileAction->setShortcut(QKeySequence("F2"));
    m_pRenameFileAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    // Disable the actions with the shortcuts, to avoid multiple key shortcuts:
    m_pBuildAction->setEnabled(false);
    m_pCancelBuildAction->setEnabled(false);
    m_pRemoveAction->setEnabled(false);
    m_pRenameFileAction->setEnabled(false);


    m_unsupportedFileTypes.push_back(AF_STR_projectFileExtension);
    m_unsupportedFileTypes.push_back(AF_STR_frameAnalysisArchivedFileExtension);
    m_unsupportedFileTypes.push_back(AF_STR_visualStudioProjectFileExtension);

}

// -----------------------------------------------------------------------------------------------
// brief Name:        instance
// brief Description: My single instance
// return gdDebugApplicationTreeHandler*
// Author:      Gilad Yarnitzky
// Date:        6/8/2013
// -----------------------------------------------------------------------------------------------
kaApplicationTreeHandler* kaApplicationTreeHandler::instance()
{
    if (m_psMySingleInstance == nullptr)
    {
        m_psMySingleInstance = new kaApplicationTreeHandler;
    }

    return m_psMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        kaApplicationTreeHandler::~kaApplicationTreeHandler
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
kaApplicationTreeHandler::~kaApplicationTreeHandler()
{
    delete[] m_pIconsArray;
}


// ---------------------------------------------------------------------------
// Name:        kaApplicationTreeHandler::buildContextMenuForItem
// Description: build context menu for the item
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
bool kaApplicationTreeHandler::BuildContextMenuForItems(const gtList<const afApplicationTreeItemData*> contextMenuItemsList, QMenu& menu)
{
    bool retVal = false;

    if (!contextMenuItemsList.empty())
    {
        if (contextMenuItemsList.size() == 1)
        {
            // Build the context menu item for a single item:
            const afApplicationTreeItemData* pItemData = contextMenuItemsList.front();
            retVal = BuildContextMenuForSingleItem(pItemData, menu);
        }
        else
        {
            bool isInBuild = kaBackendManager::instance().isInBuild();
            retVal = true;
            auto iter = contextMenuItemsList.begin();
            auto iterEnd = contextMenuItemsList.end();

            // Check the type of the first item. We support only multiple selection of files, or multiple selection of stages
            const afApplicationTreeItemData* pFirstItem = *iter;
            bool isMultiFileReferencesSelection = (pFirstItem->m_itemType >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) && (pFirstItem->m_itemType <= AF_TREE_ITEM_KA_LAST_FILE_ITEM_TYPE);
            bool isMultiFileSelection = (pFirstItem->m_itemType == AF_TREE_ITEM_KA_FILE);
            bool isMultiProgramsSelection = true;

            for (const auto& it : contextMenuItemsList)
            {
                if (it->m_itemType != AF_TREE_ITEM_KA_PROGRAM)
                {
                    isMultiProgramsSelection = false;
                    break;
                }
            }

            if (isMultiProgramsSelection)
            {
                // Add "Build" and "Cancel Build" actions but until multi program build implementation they will be disabled
                QString buildMenuStr = "Build";
                m_pBuildAction->setText(buildMenuStr);
                menu.addAction(m_pBuildAction);
                m_pBuildAction->setEnabled(false);

                QString cancelBuildMenuStr = KA_STR_CancelBuildASCII;
                m_pCancelBuildAction->setText(cancelBuildMenuStr);
                menu.addAction(m_pCancelBuildAction);
                m_pCancelBuildAction->setEnabled(false);

                // Add "Remove from project" action:
                m_pRemoveAction->setText(KA_STR_removeFromProjectASCII);
                menu.addAction(m_pRemoveAction);
                m_pRemoveAction->setEnabled(!isInBuild);
            }
            else if (isMultiFileSelection || isMultiFileReferencesSelection)
            {
                for (; iter != iterEnd; iter++)
                {
                    const afApplicationTreeItemData* pItemData = *iter;

                    if (pItemData != nullptr)
                    {
                        bool isFileRef = (pItemData->m_itemType >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) && (pItemData->m_itemType <= AF_TREE_ITEM_KA_LAST_FILE_ITEM_TYPE);
                        bool isFile = (pItemData->m_itemType == AF_TREE_ITEM_KA_FILE);

                        bool shouldContinue = (isFile && isMultiFileSelection) || (isFileRef && isMultiFileReferencesSelection);

                        if (!shouldContinue)
                        {
                            retVal = false;
                            break;
                        }
                    }
                }
            }

            if (isMultiFileReferencesSelection)
            {
                // Add the "Build" action:
                QString buildMenuStr = GetBuildCommandString();
                m_pBuildAction->setText(buildMenuStr);
                menu.addAction(m_pBuildAction);
                m_pBuildAction->setEnabled(!isInBuild);

                // Add the "Cancel Build" action:
                QString cancelBuildMenuStr = KA_STR_CancelBuildASCII;
                m_pCancelBuildAction->setText(cancelBuildMenuStr);
                menu.addAction(m_pCancelBuildAction);
                m_pCancelBuildAction->setEnabled(isInBuild);

                // Add the "Remove from program" action:
                if (kaProjectDataManager::instance().GetActiveProgram() != nullptr)
                {
                    QString removeFromProgramMenuStr = QString(KA_STR_removeShadersFromProgramASCII).arg(acGTStringToQString(kaProjectDataManager::instance().GetActiveProgram()->GetProgramName()));
                    m_pRemoveAction->setText(removeFromProgramMenuStr);
                }
                else
                {
                    m_pRemoveAction->setText(KA_STR_removeFromProgramASCII);
                }


                menu.addAction(m_pRemoveAction);
                m_pRemoveAction->setEnabled(!isInBuild);
            }

            if (isMultiFileSelection)
            {
                bool isInBuild = kaBackendManager::instance().isInBuild();

                // Add the "Remove shader from program" action:
                m_pRemoveAction->setText(KA_STR_removeFromProjectASCII);
                menu.addAction(m_pRemoveAction);
                m_pRemoveAction->setEnabled(!isInBuild);
            }
        }
    }

    return retVal;
}

void kaApplicationTreeHandler::PostContextMenuAction()
{
    if (m_shouldTakeRenameAction)
    {
        m_pApplicationTree->activateWindow();
        m_pApplicationTree->setFocus();

        m_pRenamedItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);

        // Make the item editable and edit it:
        m_pApplicationTree->editCurrentItem();
        QLineEdit* pLineEdit = m_pApplicationTree->treeControl()->lineEditor();

        if (pLineEdit != nullptr)
        {
            emit m_pApplicationTree->EditorStarted(pLineEdit);
        }

        m_shouldTakeRenameAction = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        kaApplicationTreeHandler::findMatchingTreeItem
// Description: find a matching item
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
afApplicationTreeItemData* kaApplicationTreeHandler::FindMatchingTreeItem(const afApplicationTreeItemData& displayedItemId)
{
    afApplicationTreeItemData* retVal = nullptr;

    // there can be no root when loading VS
    if (nullptr != m_pProgramsRootNode)
    {
        retVal = FindMatchingTreeItem(m_pProgramsRootNode, displayedItemId);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        kaApplicationTreeHandler::findMatchingTreeItem
// Description: recursive function for finding tree items starting with a tree node
// Return Val:  afApplicationTreeItemData*
// Author:      Gilad Yarnitzky
// Date:        14/8/2013
// ---------------------------------------------------------------------------
afApplicationTreeItemData* kaApplicationTreeHandler::FindMatchingTreeItem(QTreeWidgetItem* pCurrentTreeItem, const afApplicationTreeItemData& dataLookedFor)
{
    afApplicationTreeItemData* pRetVal = nullptr;

    GT_IF_WITH_ASSERT(nullptr != pCurrentTreeItem)
    {
        // Look at the current object:
        kaTreeDataExtension* pKADataLookedFor = qobject_cast<kaTreeDataExtension*>(dataLookedFor.extendedItemData());
        afApplicationTreeItemData* pCurrentData = m_pApplicationTree->getTreeItemData(pCurrentTreeItem);

        if (nullptr != pKADataLookedFor && nullptr != pCurrentData)
        {
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pCurrentData->extendedItemData());

            if (nullptr != pKAData)
            {
                if (pKAData->filePath() == pKADataLookedFor->filePath())

                    // compare data only the type and full path needs to be checked:
                    if ((dataLookedFor.m_itemType == pCurrentData->m_itemType) && (pKAData->filePath() == pKADataLookedFor->filePath()))
                    {
                        pRetVal = pCurrentData;
                    }

                // if identify path is identical then it is also "same" node for opening identification
                gtString identifyString = pKADataLookedFor->identifyFilePath().asString();

                if (!identifyString.isEmpty() && identifyString == pKAData->identifyFilePath().asString())
                {
                    pRetVal = pCurrentData;
                }
            }
        }

        // if it is not it, look at its children:
        if (nullptr == pRetVal)
        {
            int numChildren = pCurrentTreeItem->childCount();

            for (int nChild = 0; nChild < numChildren && nullptr == pRetVal; nChild++)
            {
                pRetVal = FindMatchingTreeItem(pCurrentTreeItem->child(nChild), dataLookedFor);
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        kaApplicationTreeHandler::AddNode
// Description: Add a node while creating the afApplicationTreeItemData and kaTreeDataExtension for it
// Arguments:   QString& iNodeName - the node name
//              afTreeItemType iNodeType - the note type
//              bool hasExtensionData - does it has a kaTreeDataExtension
//              osFilePath& iFilePath - the file path of the kaTreeDataExtension
//              int iLineNumber - the line number of the kaTreeDataExtension
//              QPixmap* iPixmap - pixmap to be shown with the node
//              QTreeWidgetItem* pParent - parent of the node
// Return Val:  QTreeWidgetItem*
// Author:      Gilad Yarnitzky
// Date:        6/8/2013
// ---------------------------------------------------------------------------
QTreeWidgetItem* kaApplicationTreeHandler::AddNode(const gtString& nodeName, afTreeItemType nodeType, const osFilePath& filePath, int lineNumber, QPixmap* pIconPixmap, QTreeWidgetItem* pParent)
{
    QTreeWidgetItem* pRetVal = nullptr;

    GT_IF_WITH_ASSERT(nullptr != pParent)
    {
        // add the program child node
        afApplicationTreeItemData* pKATypeData = new afApplicationTreeItemData;
        kaTreeDataExtension* pExtensionData;

        pKATypeData->m_itemType = nodeType;
        pExtensionData = new kaTreeDataExtension;
        pKATypeData->setExtendedData(pExtensionData);
        pExtensionData->setFilePath(filePath);
        pExtensionData->setLineNumber(lineNumber);

        osFilePath detailedPath;
        osFilePath identifyPath;

        GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr)
        {
            if (nodeType == AF_TREE_ITEM_KA_FILE)
            {
                // Check what should be the item to be added before
                QTreeWidgetItem* pItemBefore = nullptr;
                pRetVal = m_pApplicationTree->insertTreeItem(nodeName, pKATypeData, pParent, pItemBefore);
            }
            else
            {
                pRetVal = m_pApplicationTree->addTreeItem(nodeName, pKATypeData, pParent);
            }

            // Set the full path as tooltip:
            pRetVal->setToolTip(0, acGTStringToQString(pExtensionData->filePath().asString()));

            GT_IF_WITH_ASSERT(pRetVal != nullptr)
            {
                if (nullptr != pIconPixmap)
                {
                    pRetVal->setIcon(0, QIcon(*pIconPixmap));
                }
            }

            bool isProgramChild = false;
            isProgramChild = ((nodeType >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) && (nodeType <= AF_TREE_ITEM_KA_LAST_FILE_ITEM_TYPE));
            isProgramChild = isProgramChild || (nodeType == AF_TREE_ITEM_KA_OUT_DIR);
            isProgramChild = isProgramChild || (nodeType == AF_TREE_ITEM_KA_REF_TYPE);
            isProgramChild = isProgramChild || (nodeType == AF_TREE_ITEM_KA_STATISTICS); 

            if (isProgramChild)
            {
                afApplicationTreeItemData* pParentItemData = m_pApplicationTree->getTreeItemData(pParent);
                GT_IF_WITH_ASSERT(pParentItemData != nullptr)
                {
                    kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pParentItemData->extendedItemData());
                    GT_IF_WITH_ASSERT(pKAData != nullptr)
                    {
                        pExtensionData->SetProgram(pKAData->GetProgram());
                    }
                }
            }
        }
    }
    return pRetVal;
}



// ---------------------------------------------------------------------------
// Name:        kaApplicationTreeHandler::fillTreeFromProjectManager
// Description: fills the tree using the data in the project manager
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        7/8/2013
// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::fillTreeFromProjectManager()
{

    if (m_pProgramsRootNode == nullptr)
    {
        if (m_pApplicationTree != nullptr)
        {
            m_pProgramsRootNode = m_pApplicationTree->headerItem();
        }
    }

    // Make sure the "create cl file node" is there. In VS there can be a flow that we reach this point without this node here.
    // if node already exists it does not create it again.
    if (m_pNewFileTreeItem == nullptr)
    {
        AddFileCreateTreeItem();
    }

    if (m_pProgramsRootNode != nullptr)
    {
        // Sanity check:

        GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr)
        {
            m_pProgramsRootNode = m_pApplicationTree->headerItem();
        }

        const gtMap<int, kaSourceFile*>& sourceFilesMap = KA_PROJECT_DATA_MGR_INSTANCE.GetSourceFilesMap();

        for (const auto& itr : sourceFilesMap)
        {
            kaSourceFile* sourceFile = itr.second;
            KA_PROJECT_DATA_MGR_INSTANCE.BuildFunctionsList(sourceFile->filePath(), sourceFile);
        }

        BuildProgramsOutputTree();

        // Clear the active item that no longer valid
        m_pActiveItemData = nullptr;

        // Make sure a file node is selected if possible:
        SelectFirstSourceFileNode();
    }
}


void kaApplicationTreeHandler::BuildProgramsOutputTree()
{
    // pass through all the programs and for each program build the 32 bit directory and 64 bit directory files
    const gtPtrVector<kaProgram*>& programsVector = KA_PROJECT_DATA_MGR_INSTANCE.GetPrograms();
    int numPrograms = programsVector.size();

    for (int nProgram = 0; nProgram < numPrograms; nProgram++)
    {
        // for each program get the 32 bit dir and build files and the the 64 bit dir
        kaProgram* pCurrentProg = programsVector[nProgram];
        GT_ASSERT(pCurrentProg != nullptr);

        if (pCurrentProg != nullptr)
        {
            if (KA_PROJECT_DATA_MGR_INSTANCE.IsBuilt(pCurrentProg, kaBuildArch32_bit))
            {
                BuildProgramOutputTree(pCurrentProg, kaBuildArch32_bit);
            }

            if (KA_PROJECT_DATA_MGR_INSTANCE.IsBuilt(pCurrentProg, kaBuildArch64_bit))
            {
                BuildProgramOutputTree(pCurrentProg, kaBuildArch64_bit);
            }
        }
    }
}

void kaApplicationTreeHandler::CleanProgramOutputTree(kaProgram* pProgram)
{
    QTreeWidgetItem* pProgramTreeItem = GetProgramTreeItem(pProgram);
    GT_IF_WITH_ASSERT(pProgramTreeItem != nullptr && m_pApplicationTree != nullptr && m_pApplicationTree->treeControl() != nullptr)
    {
        // clean the 32 bit directory node:
        QString buildStr32(acGTStringToQString(KA_STR_UiOutputDir32));
        QTreeWidgetItem* pBuildDirItem = m_pApplicationTree->treeControl()->FindChild(pProgramTreeItem, buildStr32);

        if (pBuildDirItem != nullptr)
        {
            m_pApplicationTree->removeTreeItem(pBuildDirItem, false);
            pBuildDirItem = nullptr;
        }

        // clean the 64 bit directory node:
        QString buildStr64(acGTStringToQString(KA_STR_UiOutputDir64));
        pBuildDirItem = m_pApplicationTree->treeControl()->FindChild(pProgramTreeItem, buildStr64);

        if (pBuildDirItem != nullptr)
        {
            m_pApplicationTree->removeTreeItem(pBuildDirItem, false);
            pBuildDirItem = nullptr;
        }
    }
}

void kaApplicationTreeHandler::BuildProgramOutputTree(kaProgram* pProgram, const AnalyzerBuildArchitecture buildArchitecture)
{
    // for each program get the 32 bit dir and build files and the the 64 bit dir
    GT_IF_WITH_ASSERT(pProgram != nullptr)
    {
        CleanProgramOutputTree(pProgram);
        // Get the program output folders
        const bool isProgram32Bit = buildArchitecture == kaBuildArch32_bit;
        osDirectory dir32Bit, dir64Bit;
        pProgram->GetAndCreateOutputDirectories(dir32Bit, dir64Bit, isProgram32Bit, !isProgram32Bit);


        if (!dir32Bit.directoryPath().asString().isEmpty() && dir32Bit.directoryPath().exists())
        {
            BuildProgramOutputTree(dir32Bit, pProgram, true);
        }

        if (!dir64Bit.directoryPath().asString().isEmpty() && dir64Bit.directoryPath().exists())
        {
            BuildProgramOutputTree(dir64Bit, pProgram, false);
        }
    }
}

void kaApplicationTreeHandler::BuildProgramOutputTree(osDirectory& buildDir, kaProgram* pProgram, bool is32Dir)
{
    QTreeWidgetItem* pProgramTreeItem = GetProgramTreeItem(pProgram);

    GT_IF_WITH_ASSERT(pProgramTreeItem != nullptr)
    {
        // find the program node and the build node for the x32/x64 build
        QTreeWidgetItem* pBuildDirItem = nullptr;

        // get all the files in the directory
        gtList<osFilePath> filePaths;
        gtString searchName(L"*");
        buildDir.getContainedFilePaths(searchName, osDirectory::SORT_BY_NAME_ASCENDING, filePaths);

        // find the "output" node and if it does not exists create it
        gtString buildStr(is32Dir ? KA_STR_UiOutputDir32 : KA_STR_UiOutputDir64);
        QString searchText = acGTStringToQString(buildStr);
        pBuildDirItem = m_pApplicationTree->treeControl()->FindChild(pProgramTreeItem, searchText);

        // first check if the dir not exists under the program node & there are files in that output directory then create the node
        if (nullptr == pBuildDirItem && filePaths.size() > 0)
        {
            osFilePath outputPath;
            pBuildDirItem = AddNode(buildStr, AF_TREE_ITEM_KA_OUT_DIR, outputPath, 0, &(m_pIconsArray[KA_PIXMAP_FOLDER]), pProgramTreeItem);
        }

        // pass through all the file paths and parse them (ignoring names starting with "devices_" which is a ref name"
        // and take a special action for statistics
        gtList<osFilePath>::iterator filesIt = filePaths.begin();

        for (; filesIt != filePaths.end(); filesIt++)
        {
            gtString fileName;
            gtString fileExt;
            (*filesIt).getFileName(fileName);
            (*filesIt).getFileExtension(fileExt);

            // if the file starts with "device" then it is the file that holds the special info and just ignore it or it is a bin file also ignore it
            if (fileName.startsWith(KA_STR_kernelViewFile) || fileExt == KA_STR_kernelViewBinExtension ||
                fileExt == KA_STR_cssExtension || fileExt == KA_STR_htmlExtension)
            {
                continue;
            }
            // check if it is a cxltxt file that is not a statistics file. if that is the case. ignore the file.
            else if (fileExt == KA_STR_kernelViewExtension)
            {
                if (fileName.find(KA_STR_buildMainStatisticsFileName) == -1)
                {
                    continue;
                }
            }

            QStringList nameParts = acGTStringToQString(fileName).split(KA_STR_fileSectionSeperator);
            GT_IF_WITH_ASSERT(pProgram != nullptr)
            {
                if (pProgram->GetBuildType() == kaProgramDX || pProgram->GetBuildType() == kaProgramCL)
                {
                    AddNonPipedLineFile(pProgram, pBuildDirItem, nameParts, acGTStringToQString(fileExt), is32Dir);
                }
                else
                {
                    AddPipedLineFile(pProgram, pBuildDirItem, nameParts, acGTStringToQString(fileExt), is32Dir);
                }
            }
        }

        // reorder the build dir node
        if (pBuildDirItem != nullptr)
        {
            ReorderProgramOutputSubTree(pProgram, pBuildDirItem);
        }
    }
}

QTreeWidgetItem* kaApplicationTreeHandler::FindProgramTreeItem(kaProgram* pProgram) const
{
    QTreeWidgetItem* pRetVal = nullptr;

    GT_IF_WITH_ASSERT((m_pProgramsRootNode != nullptr) && (m_pApplicationTree != nullptr))
    {
        if (pProgram != nullptr)
        {
            for (int i = 0; i < m_pProgramsRootNode->childCount(); i++)
            {
                QTreeWidgetItem* pChild = m_pProgramsRootNode->child(i);
                afApplicationTreeItemData* pTemp = m_pApplicationTree->getTreeItemData(pChild);

                if (pTemp != nullptr && (pTemp->m_itemType == AF_TREE_ITEM_KA_PROGRAM))
                {
                    kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pTemp->extendedItemData());

                    if (pKAData->GetProgram() == pProgram)
                    {
                        pRetVal = pChild;
                    }
                }
            }
        }
    }
    return pRetVal;
}

QTreeWidgetItem* kaApplicationTreeHandler::FindProgramFileTreeItem(QTreeWidgetItem* pProgramParentItem, const osFilePath& filePath)
{
    QTreeWidgetItem* pRetVal = nullptr;

    GT_IF_WITH_ASSERT((pProgramParentItem != nullptr) && (m_pApplicationTree != nullptr))
    {
        for (int i = 0; i < pProgramParentItem->childCount(); i++)
        {
            QTreeWidgetItem* pChild = pProgramParentItem->child(i);
            afApplicationTreeItemData* pTemp = m_pApplicationTree->getTreeItemData(pChild);

            if (pTemp != nullptr)
            {
                kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pTemp->extendedItemData());

                if (pKAData->filePath() == filePath)
                {
                    pRetVal = pChild;
                }
            }
        }
    }
    return pRetVal;
}

QTreeWidgetItem* kaApplicationTreeHandler::GetProgramTreeItem(kaProgram* pProgram) const
{
    QTreeWidgetItem* pRetVal = nullptr;
    GT_IF_WITH_ASSERT((pProgram != nullptr) && (m_pApplicationTree != nullptr) && (m_pApplicationTree->treeControl() != nullptr))
    {
        // Get the program tree item node:
        QString programName = acGTStringToQString(pProgram->GetProgramName());

        QList<QTreeWidgetItem*> programsList = m_pApplicationTree->treeControl()->findItems(programName, Qt::MatchExactly | Qt::MatchRecursive);
        GT_IF_WITH_ASSERT(programsList.count() == 1)
        {
            pRetVal = programsList[0];
        }
    }
    return pRetVal;
}

void kaApplicationTreeHandler::AddNonPipedLineFile(kaProgram* pProgram, QTreeWidgetItem* pBuildItem, QStringList& nameParts, QString fileExt, bool is32Dir)
{
    afApplicationTree* pApplicationTree = afApplicationCommands::instance()->applicationTree();
    acTreeCtrl* pTreeCtrl = nullptr;
    GT_IF_WITH_ASSERT(pApplicationTree != nullptr)
    {
        pTreeCtrl = pApplicationTree->treeControl();
    }
    // make the name again from all the parts excluding the first parts which is the device name:
    QStringList copyNameParts(nameParts);
    copyNameParts.removeFirst();
    QString joinedName = copyNameParts.join(KA_STR_fileSectionSeperator);
    gtString joinedNameAsStr = acQStringToGTString(joinedName);

    // Create file name from its part and file ext
    osFilePath filePath;
    GT_IF_WITH_ASSERT(pProgram != nullptr && pTreeCtrl != nullptr)
    {
        QString kernelNodeName;
        // make sure the program is of the right type
        GT_IF_WITH_ASSERT(pProgram->GetBuildType() == kaProgramCL || pProgram->GetBuildType() == kaProgramDX)
        {
            const gtVector<int>& fileRefsVec = pProgram->GetFileIDsVector();
            int numFileRef = fileRefsVec.size();

            osFilePath currentFilePath;

            for (int nRef = 0; nRef < numFileRef; nRef++)
            {
                currentFilePath.clear();
                KA_PROJECT_DATA_MGR_INSTANCE.GetFilePathByID(fileRefsVec[nRef], currentFilePath);
                gtString fileName;
                currentFilePath.getFileName(fileName);
                int fileNamePos = joinedNameAsStr.reverseFind(fileName);

                if (fileNamePos != -1 && fileNamePos > 1)
                {
                    filePath = currentFilePath;
                    gtString kernelNameAsStr;
                    joinedNameAsStr.getSubString(0, fileNamePos - 2, kernelNameAsStr);
                    kernelNodeName = (acGTStringToQString(kernelNameAsStr));
                    break;
                }
            }

            GT_IF_WITH_ASSERT(!filePath.asString().isEmpty())
            {
                // make sure the parent node for the file and kernel exist (and if not create them
                gtString fileNameAndExt;
                filePath.getFileNameAndExtension(fileNameAndExt);
                QString fileNodeName = QString("%1").arg(acGTStringToQString(fileNameAndExt));

                QTreeWidgetItem* pFileNode = pTreeCtrl->FindChild(pBuildItem, fileNodeName);

                // create the node if still not created
                if (pFileNode == nullptr)
                {
                    osFilePath outputPath;
                    pFileNode = AddNode(acQStringToGTString(fileNodeName), AF_TREE_ITEM_KA_REF_TYPE, outputPath, 0, &(m_pIconsArray[KA_PIXMAP_KERNELS]), pBuildItem);
                }

                QTreeWidgetItem* pKernelNode = pTreeCtrl->FindChild(pFileNode, kernelNodeName);

                // create the node if still not created
                if (pKernelNode == nullptr)
                {
                    osFilePath outputPath;
                    pKernelNode = AddNode(acQStringToGTString(kernelNodeName), AF_TREE_ITEM_KA_REF_TYPE, outputPath, 0, &(m_pIconsArray[KA_PIXMAP_KERNEL]), pFileNode);
                }

                CreateFileTreeNode(pProgram, pKernelNode, filePath, nameParts, fileExt, is32Dir);
            }
        }
    }
}

void kaApplicationTreeHandler::AddPipedLineFile(kaProgram* pProgram, QTreeWidgetItem* pBuildItem, QStringList& nameParts, QString fileExt, bool is32Dir)
{
    afApplicationTree* pApplicationTree = afApplicationCommands::instance()->applicationTree();
    acTreeCtrl* pTreeCtrl = nullptr;
    GT_IF_WITH_ASSERT(pApplicationTree != nullptr)
    {
        pTreeCtrl = pApplicationTree->treeControl();
    }

    // Get the file path associated with the file type (which is the second part name device_section)
    if (nameParts.size() == 2)
    {
        gtString fileRefStr = acQStringToGTString(nameParts[1]);

        GT_IF_WITH_ASSERT(pProgram != nullptr && pTreeCtrl != nullptr && pBuildItem != nullptr)
        {
            kaPipelinedProgram* pPipelinedProgram = dynamic_cast<kaPipelinedProgram*>(pProgram);
            GT_IF_WITH_ASSERT(pPipelinedProgram != nullptr)
            {
                kaPipelinedProgram::PipelinedStage stage = pPipelinedProgram->GetRenderingStageTypeFromString(fileRefStr);
                osFilePath filePath;
                bool isOk = pPipelinedProgram->GetFilePath(stage, filePath);
                GT_IF_WITH_ASSERT(isOk && !filePath.asString().isEmpty())
                {
                    // make sure the parent node for the ref type exists
                    gtString fileNameAndExt;
                    filePath.getFileNameAndExtension(fileNameAndExt);
                    QString fileTypeNodeName = QString(KA_STR_refTypeNode).arg(nameParts[1]).arg(acGTStringToQString(fileNameAndExt));

                    QTreeWidgetItem* pRefTypeNopeNode = pTreeCtrl->FindChild(pBuildItem, fileTypeNodeName);

                    // create the node if still not created
                    if (pRefTypeNopeNode == nullptr)
                    {
                        osFilePath outputPath;
                        pRefTypeNopeNode = AddNode(acQStringToGTString(fileTypeNodeName), AF_TREE_ITEM_KA_REF_TYPE, outputPath, 0, &(m_pIconsArray[KA_PIXMAP_KERNELS]), pBuildItem);
                    }

                    CreateFileTreeNode(pProgram, pRefTypeNopeNode, filePath, nameParts, fileExt, is32Dir);
                }
            }
        }
    }
}

void kaApplicationTreeHandler::ReorderProgramOutputSubTree(kaProgram* pProgram, QTreeWidgetItem* pOutputNode)
{
    GT_IF_WITH_ASSERT(pProgram != nullptr && pOutputNode != nullptr)
    {
        int numChildren = pOutputNode->childCount();

        for (int nChild = 0; nChild < numChildren; nChild++)
        {
            QTreeWidgetItem* pChild = pOutputNode->child(nChild);
            GT_IF_WITH_ASSERT(pChild != nullptr)
            {
                // for DX && CL this level is the file level and we need to sort the kernel level so we need to go through another child level
                if (pProgram->GetBuildType() == kaProgramCL || pProgram->GetBuildType() == kaProgramDX)
                {
                    int numKernels = pChild->childCount();

                    for (int nKernel = 0; nKernel < numKernels; nKernel++)
                    {
                        QTreeWidgetItem* pKernel = pChild->child(nKernel);
                        GT_IF_WITH_ASSERT(pKernel != nullptr)
                        {
                            reorderKernelNodeByFamily(pKernel);
                        }
                    }
                }
                else
                {
                    reorderKernelNodeByFamily(pChild);
                }
            }
        }

        if (pProgram->GetBuildType() == kaProgramGL_Rendering || pProgram->GetBuildType() == kaProgramVK_Rendering)
        {
            ReorderRenderingProgramByPipeLine(pOutputNode);
        }
    }
}

void  kaApplicationTreeHandler::CreateFileTreeNode(kaProgram* pProgram, QTreeWidgetItem* pParentNode, osFilePath& filePath, QStringList& nameParts, QString& fileExt, bool is32Dir)
{
    bool isStatisticsFile = ((acQStringToGTString(fileExt) == KA_STR_kernelViewExtension) ? true : false);
    afApplicationTree* pApplicationTree = afApplicationCommands::instance()->applicationTree();
    GT_IF_WITH_ASSERT(pApplicationTree != nullptr && pProgram != nullptr && pParentNode != nullptr)
    {
        acTreeCtrl* pTreeCtrl = pApplicationTree->treeControl();
        GT_IF_WITH_ASSERT(pTreeCtrl != nullptr)
        {
            // make sure the device file exists
            gtString devicesFileName = CreateDevicesFile(pProgram, nameParts, is32Dir, filePath);

            // create the node name
            QPixmap* pIcon = &(m_pIconsArray[KA_PIXMAP_ISA_FILE]);

            if (fileExt == KA_STR_kernelViewILExtensionASCII)
            {
                pIcon = &(m_pIconsArray[KA_PIXMAP_IL_FILE]);
            }

            // at this stage files ending with ".cxltxt" we need special handling
            gtString fullNodeName;
            QTreeWidgetItem* pFileNode;

            if (isStatisticsFile)
            {
                pIcon = &(m_pIconsArray[KA_PIXMAP_STATISTICS]);
                fullNodeName = KA_STR_buildMainStatisticsFileName;
                pFileNode = AddNode(fullNodeName, AF_TREE_ITEM_KA_STATISTICS, filePath, 0, pIcon, pParentNode);
            }
            else
            {
                QString familyName;
                kaFindFamilyName(nameParts[0], familyName);
                gtString familyNameAsStr = acQStringToGTString(familyName);
                gtString deviceNameAsStr = acQStringToGTString(nameParts[0]);
                fullNodeName.appendFormattedString(L"%ls: %ls", familyNameAsStr.asCharArray(), deviceNameAsStr.asCharArray());
                // check if a node with the same name of the device already exists. if it does then change the icon to reflect that both isa & il exists for the node
                // if not create it with the correct icon for the current file ext
                QString fullNodeNameAsQStr = acGTStringToQString(fullNodeName);
                pFileNode = pTreeCtrl->FindChild(pParentNode, fullNodeNameAsQStr);

                if (pFileNode == nullptr)
                {
                    pFileNode = AddNode(fullNodeName, AF_TREE_ITEM_KA_DEVICE, filePath, 0, pIcon, pParentNode);
                }
                else
                {
                    pFileNode->setIcon(0, m_pIconsArray[KA_PIXMAP_IL_ISA_FILE]);
                }
            }

            // set the extended data with the paths to the files
            // get the node data so its extended data can be set
            GT_IF_WITH_ASSERT(pApplicationTree != nullptr && pFileNode != nullptr)
            {
                afApplicationTreeItemData* pTreeItemData = m_pApplicationTree->getTreeItemData(pFileNode);
                kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pTreeItemData->extendedItemData());

                if (pKAData != nullptr)
                {
                    const bool isProgram32Bit = KA_PROJECT_DATA_MGR_INSTANCE.GetBuildArchitecture() == kaBuildArch32_bit;
                    osDirectory output32Dir, output64Dir;
                    pProgram->GetAndCreateOutputDirectories(output32Dir, output64Dir, isProgram32Bit, !isProgram32Bit);
                    osDirectory outputDir = is32Dir ? output32Dir : output64Dir;
                    osFilePath outputPath = outputDir.directoryPath();
                    outputPath.appendSubDirectory(L"");
                    osFilePath detailedPath(outputPath);
                    osFilePath identifyPath(outputPath);

                    identifyPath.setFileName(devicesFileName);
                    identifyPath.setFileExtension(KA_STR_kernelViewExtension);
                    pKAData->setIdentifyFilePath(identifyPath);

                    QString fileName = nameParts.join(KA_STR_fileSectionSeperator);
                    detailedPath.setFileName(acQStringToGTString(fileName));
                    detailedPath.setFileExtension(KA_STR_kernelViewExtension);
                    pKAData->setDetailedFilePath(detailedPath);
                }
            }
        }
    }
}

gtString kaApplicationTreeHandler::CreateDevicesFile(kaProgram* pProgram, QStringList& nameParts, bool is32Dir, osFilePath& filePath)
{
    gtString retStr;
    // the device file is created under the program dir wit the output32/64 path added and the file name is
    // based on the original device name file so if it was devicename_otherparts.ext (does matter cl or gl) then the
    // file name will be "devices_otherparts.cxltxt".
    // the file is the same for all "otherparts" so before creating it just check if it does not already exist
    // Create the open device list file used by the GUI ugly but best place to put it:
    GT_IF_WITH_ASSERT(pProgram != nullptr)
    {
        const bool isProgram32Bit = KA_PROJECT_DATA_MGR_INSTANCE.GetBuildArchitecture() == kaBuildArch32_bit;
        osDirectory output32Dir, output64Dir;
        pProgram->GetAndCreateOutputDirectories(output32Dir, output64Dir, isProgram32Bit, !isProgram32Bit);

        osDirectory buildDir = is32Dir ? output32Dir : output64Dir;
        osFilePath buildPath = buildDir.directoryPath();

        QString devicesFileName = acGTStringToQString(KA_STR_kernelViewFile);
        int numParts = nameParts.size();

        for (int nPart = 1; nPart < numParts; nPart++)
        {
            devicesFileName += KA_STR_fileSectionSeperator;
            devicesFileName += nameParts[nPart];
        }

        gtString devicesFileNameAsStr = acQStringToGTString(devicesFileName);
        buildPath.setFileName(devicesFileNameAsStr);
        buildPath.setFileExtension(KA_STR_kernelViewExtension);

        if (!buildPath.exists())
        {
            osFile devicesFile;
            devicesFile.open(buildPath, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
            devicesFile.writeString(filePath.asString());
            devicesFile.close();
        }

        retStr = devicesFileNameAsStr;
    }

    return retStr;
}


struct treeItemLessThan
{
    // This is a binary less_than operator used for sorting the tree nodes of the
    // static analyzer explorer tree. The order of the nodes should be:
    //      Statistics (always first)
    //      Analysis (always second - if it exists)
    //      Latest Graphics IP generation : first lexicographic device codename
    //      Latest Graphics IP generation : ...
    //      Latest Graphics IP generation : last lexicographic device codename
    //      ...
    //      Earliest Graphics IP generation : first lexicographic device codename
    //      Earliest Graphics IP generation : ...
    //      Earliest Graphics IP generation : last lexicographic device codename
    //      CPU Device
    //
    // Examples:
    //      Statistics
    //      Analysis
    //      Graphics IP v8: Iceland
    //      Graphics IP v8: Tonga
    //      Graphics IP v7: Bonaire
    //      Graphics IP v7: Kalindi
    //      Graphics IP v7: Spectre
    //      CPU Device: AMD A8-4500
    inline bool operator()(QTreeWidgetItem* item1, QTreeWidgetItem* item2)
    {
        bool ret = false;

        if (item1 && item2)
        {
            const QString& name1 = item1->text(0);
            const QString& name2 = item2->text(0);

            // CPU loses to all, it should be last
            if (name1.startsWith("CPU"))
            {
                ret = false;
            }
            //statistics beats all - it should be first
            else if (name2 == "Statistics")
            {
                ret = false;
            }
            //Analysis should be second
            else if (name1 == "Statistics")
            {
                ret = true;
            }
            else if (name2 == "Analysis")
            {
                ret = false;
            }
            else if (name1 == "Analysis")
            {
                ret = true;
            }
            else
            {
                QStringList list1 = name1.split(':');
                QStringList list2 = name2.split(':');
                // QStringList is guaranteed to hold at least one string

                // Compare family names
                if (list1[0] < list2[0])
                {
                    ret = false;
                }
                else if (list1[0] > list2[0])
                {
                    ret = true;
                }
                else if (list1.count() > 1 && list2.count() > 1)
                {
                    // Compare model names
                    if (list1[1] < list2[1])
                    {
                        ret = true;
                    }
                    else if (list1[1] > list2[1])
                    {
                        ret = false;
                    }
                }
                else
                {
                    ret = false;
                }
            }
        }

        return ret;
    };
};

// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::reorderKernelNodeByFamily(QTreeWidgetItem* pkernelTreeItem)
{
    QString currentFamilyName;

    if (pkernelTreeItem != nullptr)
    {
        int numDevices = pkernelTreeItem->childCount();

        if (numDevices > 0)
        {
            std::vector<QTreeWidgetItem*> itemsToSort;
            itemsToSort.reserve(numDevices);
            int deviceIndex = 0;

            for (deviceIndex = 0; deviceIndex < numDevices; deviceIndex++)
            {
                // Push the item into a vector for later sorting
                QTreeWidgetItem* pItem = pkernelTreeItem->child(deviceIndex);
                itemsToSort.push_back(pItem);
            }

            // Sort the items
            std::sort(itemsToSort.begin(), itemsToSort.end(), treeItemLessThan());

            // Remove the unsorted items from the tree
            pkernelTreeItem->takeChildren();

            // Push the sorted items into the tree
            for (deviceIndex = 0; deviceIndex < numDevices; deviceIndex++)
            {
                // Push back the item in its sorted position
                QTreeWidgetItem* pItem = itemsToSort[deviceIndex];
                pkernelTreeItem->insertChild(deviceIndex, pItem);
            }
        }
    }
}

struct programItemsLessThen
{
    // order stages:
    // Vert
    // Tesc
    // Tese
    // Geom
    // Frag
    inline bool operator()(QTreeWidgetItem* item1, QTreeWidgetItem* item2)
    {
        bool ret = false;

        if (item1 && item2)
        {
            const QString& name1 = item1->text(0).toLower();
            const QString& name2 = item2->text(0).toLower();
            // for each item take the part up to the space which hold the stage part
            QStringList name1Parts = name1.split(AC_STR_SpaceA);
            QStringList name2Parts = name2.split(AC_STR_SpaceA);

            QStringList stageList = QString(KA_STR_programSortOrder).split(AC_STR_CommaA);
            int name1Pos = stageList.indexOf(name1Parts[0]);
            int name2Pos = stageList.indexOf(name2Parts[0]);
            GT_IF_WITH_ASSERT(name1Pos != -1 && name2Pos != -1)
            {
                ret = name1Pos < name2Pos;
            }
        }

        return ret;
    };
};

void kaApplicationTreeHandler::ReorderRenderingProgramByPipeLine(QTreeWidgetItem* pProgramTreeItem)
{
    if (pProgramTreeItem != nullptr)
    {
        int numStages = pProgramTreeItem->childCount();

        if (numStages > 0)
        {
            std::vector<QTreeWidgetItem*> itemsToSort;
            itemsToSort.reserve(numStages);

            for (int nStage = 0; nStage < numStages; nStage++)
            {
                // Push the item into a vector for later sorting
                QTreeWidgetItem* pItem = pProgramTreeItem->child(nStage);
                itemsToSort.push_back(pItem);
            }

            // Sort the items
            std::sort(itemsToSort.begin(), itemsToSort.end(), programItemsLessThen());

            // Remove the unsorted items from the tree
            pProgramTreeItem->takeChildren();

            // Push the sorted items into the tree
            for (int nStage = 0; nStage < numStages; nStage++)
            {
                // Push back the item in its sorted position
                QTreeWidgetItem* pItem = itemsToSort[nStage];
                pProgramTreeItem->insertChild(nStage, pItem);
            }
        }
    }
}
// ---------------------------------------------------------------------------
QString kaApplicationTreeHandler::familyName(QTreeWidgetItem* pTreeItem)
{
    QString retStr;

    GT_IF_WITH_ASSERT(nullptr != pTreeItem)
    {
        QString itemName = pTreeItem->text(0);
        // look for a family name that is not the same as our current family name:
        int familyNameEnd = itemName.indexOf(":");

        if (familyNameEnd != -1)
        {
            retStr = itemName.left(familyNameEnd);
        }
    }

    return retStr;
}

void kaApplicationTreeHandler::OnAddFile(afTreeItemType associateToItemType)
{
    if (kaBackendManager::instance().isInBuild())
    {
        acMessageBox::instance().critical(afGlobalVariablesManager::ProductNameA(), KA_STR_ERR_CANNOT_ADD_SOURCE_FILE_DURING_BUILD);
    }
    else
    {
        afApplicationTreeItemData* pSelectedItemData = m_pApplicationTree->getTreeItemData(m_pApplicationTree->getTreeSelection());
        const afApplicationTreeItemData* pParentProgramItemData = FindParentItemDataOfType(pSelectedItemData, AF_TREE_ITEM_KA_PROGRAM);

        if (pSelectedItemData != nullptr)
        {
            gtVector<osFilePath> addedFilePaths;
            kaApplicationCommands::instance().AddFileCommand(addedFilePaths);

            for (const osFilePath& it : addedFilePaths)
            {
                if (ShouldAddFileToProgramBranch(pSelectedItemData, pParentProgramItemData, it, associateToItemType))
                {
                    // Add the file to the program branch
                    AddFileNodeToProgramBranch(it, pParentProgramItemData, associateToItemType);

                    if (!IsAddingMultipleFilesToProgramBranchAllowed(pParentProgramItemData))
                    {
                        // pipeline programs don't allow adding multiple files - only first file is added
                        break;
                    }
                }
            }
        }
    }
}

bool kaApplicationTreeHandler::IsAddingMultipleFilesToProgramBranchAllowed(const afApplicationTreeItemData* pParentProgramItemData)const
{
    bool isAddingMultipleFilesAllowed = true;

    if (pParentProgramItemData != nullptr)
    {
        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pParentProgramItemData->extendedItemData());

        if ((pKAData != nullptr) && (pKAData->GetProgram() != nullptr))
        {
            kaProgram* pProgram = pKAData->GetProgram();
            kaProgramTypes programType = pProgram->GetBuildType();

            isAddingMultipleFilesAllowed = !(kaProgramGL_Compute == programType ||
                                             kaProgramGL_Rendering == programType ||
                                             kaProgramVK_Compute == programType ||
                                             kaProgramVK_Rendering == programType);
        }
    }

    return isAddingMultipleFilesAllowed;
}

bool kaApplicationTreeHandler::ShouldAddFileToProgramBranch(const afApplicationTreeItemData* pSelectedItemData, const afApplicationTreeItemData* pParentProgramItemData, osFilePath addedFilePath, afTreeItemType& associateToItemType) const
{
    bool shouldAddToProgram = false;

    if (addedFilePath.exists() && (pParentProgramItemData != nullptr))
    {
        shouldAddToProgram = true;
        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pParentProgramItemData->extendedItemData());

        if ((pKAData != nullptr) && (pKAData->GetProgram() != nullptr))
        {
            kaProgram* pProgram = pKAData->GetProgram();

            if (pProgram->HasFile(addedFilePath, AF_TREE_ITEM_ITEM_NONE))
            {
                acMessageBox::instance().information(afGlobalVariablesManager::instance().ProductNameA(), KA_STR_treeProgramFileExists);
                shouldAddToProgram = false;
            }

            if (KA_PROJECT_DATA_MGR_INSTANCE.IsProgramPipeLine(pProgram))
            {
                if (associateToItemType == AF_TREE_ITEM_ITEM_NONE && pSelectedItemData != nullptr)
                {
                    associateToItemType = pSelectedItemData->m_itemType;
                }

                // If the stage is not selected - don't add file to pipeline program
                if (associateToItemType < AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE || associateToItemType > AF_TREE_ITEM_KA_LAST_FILE_ITEM_TYPE)
                {
                    shouldAddToProgram = false;
                }
            }
        }
    }

    return shouldAddToProgram;
}

bool kaApplicationTreeHandler::AddFileToProgram(kaProgram* pProgram, afTreeItemType programChildItemType, const int fileID, kaSourceFile* pFile) const
{
    bool shouldAddNewTreeNode = false;
    kaFileTypes fileType = pFile->FileType();
    const kaProgramTypes programType = pProgram->GetBuildType();

    switch (programType)
    {
        case kaProgramCL:
        {
            if (fileType == kaFileTypeUnknown)
            {
                pFile->SetFileType(kaFileTypeOpenCL);
            }

            shouldAddNewTreeNode = true;
            kaNonPipelinedProgram* pNPProgram = dynamic_cast<kaNonPipelinedProgram*>(pProgram);

            if (pNPProgram != nullptr)
            {
                pNPProgram->AddFile(fileID);
            }
        }
        break;

        case kaProgramDX:
        {

            if (fileType == kaFileTypeUnknown)
            {
                pFile->SetFileType(kaFileTypeDXGenericShader);
            }

            shouldAddNewTreeNode = true;
            kaDxFolder* pDXFolder = dynamic_cast<kaDxFolder*>(pProgram);
            GT_IF_WITH_ASSERT(pDXFolder != nullptr)
            {

                pDXFolder->AddFile(fileID);
                pDXFolder->SetFileModel(fileID, L"");
                pDXFolder->SetFileSelectedType(fileID, L"");
                pDXFolder->SetFileSelectedEntryPoint(fileID, L"");
            }
        }
        break;

        case kaProgramGL_Compute:
        case kaProgramVK_Compute:
        {
            if (fileType == kaFileTypeUnknown)
            {
                pFile->SetFileType(kaFileTypeGLSLGenericShader);
            }

            kaComputeProgram* pCProgram = dynamic_cast<kaComputeProgram*>(pProgram);

            if (pCProgram != nullptr)
            {
                pCProgram->SetFileID(fileID);
            }
        }
        break;

        case kaProgramGL_Rendering:
        case kaProgramVK_Rendering:
        {
            if (fileType == kaFileTypeUnknown)
            {
                pFile->SetFileType(kaFileTypeGLSLGenericShader);
            }

            kaRenderingProgram* pRProgram = dynamic_cast<kaRenderingProgram*>(pProgram);

            if (pRProgram != nullptr)
            {
                kaPipelinedProgram::PipelinedStage targetStage = kaRenderingProgram::TreeItemTypeToRenderingStage(programChildItemType);

                if (targetStage == kaRenderingProgram::KA_PIPELINE_STAGE_NONE)
                {
                    targetStage = kaRenderingProgram::FileTypeToRenderStage(pFile->FileType());
                }

                // Set the file ID
                pRProgram->SetFileID(targetStage, fileID);
            }
        }
        break;

        default:
            break;
    }

    return shouldAddNewTreeNode;
}

void kaApplicationTreeHandler::AddFileNodeToProgramBranch(const osFilePath& addedFilePath, const afApplicationTreeItemData* pProgramTreeItemData, afTreeItemType programChildItemType)
{
    GT_IF_WITH_ASSERT(pProgramTreeItemData != nullptr)
    {
        //we are under program node - add created file node to the program and it's index to the map.
        kaSourceFile* pFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(addedFilePath);
        GT_IF_WITH_ASSERT((pFile != nullptr))
        {
            int fileID = KA_PROJECT_DATA_MGR_INSTANCE.GetFileID(pFile->filePath());
            // If the child type is unknown, we should extract it from the file type
            if (programChildItemType == AF_TREE_ITEM_ITEM_NONE)
            {
                programChildItemType = ExtractProgramChildType(pProgramTreeItemData, pFile);
            }

            // Get the extended KA data
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pProgramTreeItemData->extendedItemData());
            GT_IF_WITH_ASSERT((pKAData != nullptr) && (pKAData->GetProgram() != nullptr))
            {
                //already checked or nullptr
                kaProgram* pProgram = pKAData->GetProgram();
                gtString fileNameWithExt;
                pFile->filePath().getFileNameAndExtension(fileNameWithExt);
                QString treeItemText = acGTStringToQString(fileNameWithExt);

                QTreeWidgetItem* pFileTreeNode = nullptr;
                // Get or create the file item in tree and set it's data
                const kaProgramTypes programType = pProgram->GetBuildType();
                const bool shouldCompareFileType = ((programType == kaProgramDX) || programType == kaProgramCL);
                osFilePath fileToSearch = shouldCompareFileType ? pFile->filePath() : osFilePath();
                afApplicationTreeItemData* pFileItemData = FindChildOfType(pProgramTreeItemData->m_pTreeWidgetItem, programChildItemType, fileToSearch);

                // Check if the file already exists
                bool programHasFile = (pFile->id() >= 0) && pProgram->HasFile(pFile->id(), AF_TREE_ITEM_ITEM_NONE);
                if (programHasFile)
                {
                    // the program already has this file - in case of rendering program copy to other stage should be allowed
                    if (programType == kaProgramGL_Rendering ||
                        programType == kaProgramVK_Rendering)
                    {
                        //turn the flag off to allow adding the file to another stage
                        kaRenderingProgram* pRenderingProgram = dynamic_cast<kaRenderingProgram*>(pProgram);
                        if (pRenderingProgram != nullptr)
                        {
                            kaPipelinedProgram::PipelinedStage stage = pRenderingProgram->GetFileRenderingStage(pFile->id());
                            // obtain stage tree item
                            QTreeWidgetItem *pProgramTreeItem = pProgramTreeItemData->m_pTreeWidgetItem;
                            if (pProgramTreeItem != nullptr)
                            {
                                QTreeWidgetItem *pStageItem = nullptr;
                                for (int i = 0; i < pProgramTreeItem->childCount(); i++)
                                {
                                    pStageItem = pProgramTreeItem->child(i);
                                    afApplicationTreeItemData* pExistingChildData = m_pApplicationTree->getTreeItemData(pStageItem);

                                    if (pStageItem != nullptr)
                                    {
                                        // if this is the current stage - remove file id and set placeholder text
                                        if (stage == kaRenderingProgram::TreeItemTypeToRenderingStage(pExistingChildData->m_itemType))
                                        {
                                            pProgram->OnFileRemove(fileID);
                                            pRenderingProgram->SetFileID(stage, -1);
                                            gtString stageNameAsString = ProgramItemTypeAsText(pExistingChildData->m_itemType, L"");
                                            pStageItem->setText(0, acGTStringToQString(stageNameAsString));
                                            pStageItem->setTextColor(0, acQLIST_EDITABLE_ITEM_COLOR);
                                            pExistingChildData->m_filePath = osFilePath();
                                            kaTreeDataExtension* pExistingChildKAData = qobject_cast<kaTreeDataExtension*>(pExistingChildData->extendedItemData());
                                            if (pExistingChildKAData != nullptr)
                                            {
                                                pExistingChildKAData->setFilePath(osFilePath());
                                            }
                                            programHasFile = false;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                if (!programHasFile)
                {
                    // Connect the program to this file id in the data manager
                    KA_PROJECT_DATA_MGR_INSTANCE.Connect(pFile->id(), pProgram);

                  
                    // add source file
                    bool shouldAddNewTreeNode = AddFileToProgram(pProgram, programChildItemType, fileID, pFile);

                    kaTreeDataExtension* pChildKAData = nullptr;

                    if (pFileItemData != nullptr)
                    {
                        pFileTreeNode = pFileItemData->m_pTreeWidgetItem;
                        pChildKAData = qobject_cast<kaTreeDataExtension*>(pFileItemData->extendedItemData());
                    }

                    if (shouldAddNewTreeNode || (pFileTreeNode == nullptr))
                    {
                        afApplicationTreeItemData* pNewItemData = AddProgramTreeItemChild(pProgramTreeItemData->m_pTreeWidgetItem, programChildItemType, pFile->filePath());
                        GT_IF_WITH_ASSERT(pNewItemData != nullptr)
                        {
                            pFileTreeNode = pNewItemData->m_pTreeWidgetItem;
                            pChildKAData = qobject_cast<kaTreeDataExtension*>(pNewItemData->extendedItemData());
                        }
                    }

                    GT_IF_WITH_ASSERT(pChildKAData != nullptr)
                    {
                        pChildKAData->SetProgram(pProgram);
                        pChildKAData->setFilePath(pFile->filePath());
                        pChildKAData->m_pParentData->m_filePath = pFile->filePath();
                    }

                    GT_IF_WITH_ASSERT(pFileTreeNode != nullptr)
                    {
                        gtString stageNameAsString = ProgramItemTypeAsText(programChildItemType, L"");
                        QString nodeName = acGTStringToQString(stageNameAsString);

                        if (!nodeName.isEmpty())
                        {
                            treeItemText = QString("%1 (%2)").arg(nodeName).arg(treeItemText);
                        }

                        pFileTreeNode->setText(0, treeItemText);
                        gtString filePath = pFile->filePath().asString();

                        if (!filePath.isEmpty())
                        {
                            pFileTreeNode->setToolTip(0, acGTStringToQString(filePath));
                        }

                        pFileTreeNode->setTextColor(0, Qt::black);
                        pFileTreeNode->setIcon(0, m_pIconsArray[KA_PIXMAP_SOURCE]);
                    }
                }
                else
                {
                    GT_IF_WITH_ASSERT(pFileItemData != nullptr)
                    {
                        pFileTreeNode = pFileItemData->m_pTreeWidgetItem;
                    }
                }

                GT_IF_WITH_ASSERT(pFileTreeNode != nullptr)
                {
                    m_pApplicationTree->clearSelection();
                    pFileTreeNode->setSelected(true);
                    m_pApplicationTree->treeControl()->setCurrentItem(pFileTreeNode);
                }
            }
        }
    }
}

void kaApplicationTreeHandler::RenameAllFileOccurencesInTree(const osFilePath& oldFilePath, const osFilePath& newFilePath)
{
    // Get the file id for the rename file
    int fileId = KA_PROJECT_DATA_MGR_INSTANCE.GetFileID(newFilePath);
    GT_IF_WITH_ASSERT(fileId >= 0)
    {
        const gtVector<kaProgram*> programsVec = KA_PROJECT_DATA_MGR_INSTANCE.GetPrograms();

        for (int i = 0; i < (int)programsVec.size(); i++)
        {
            kaProgram* pProgram = programsVec[i];

            if (pProgram != nullptr)
            {
                // Get the file IDs for this program
                const gtVector<int> programFileIDs = pProgram->GetFileIDsVector();

                // If this program contain a reference to the renamed file id, rename the file tree node
                if (gtFind(programFileIDs.begin(), programFileIDs.end(), fileId) != programFileIDs.end())
                {
                    QTreeWidgetItem* pProgramItem = GetProgramTreeItem(pProgram);
                    GT_IF_WITH_ASSERT(pProgramItem != nullptr)
                    {
                        for (int i = 0; i < pProgramItem->childCount(); i++)
                        {
                            QTreeWidgetItem* pChild = pProgramItem->child(i);
                            afApplicationTreeItemData* pChildData = m_pApplicationTree->getTreeItemData(pChild);

                            if (pChildData != nullptr)
                            {
                                kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pChildData->extendedItemData());

                                if (pKAData != nullptr)
                                {
                                    if (pKAData->filePath() == oldFilePath)
                                    {
                                        pKAData->setFilePath(newFilePath);
                                        pChildData->m_filePath = newFilePath;

                                        gtString oldFileName, newFileName;
                                        oldFilePath.getFileNameAndExtension(oldFileName);
                                        newFilePath.getFileNameAndExtension(newFileName);

                                        QString nodeName = pChild->text(0);

                                        if (m_fileNameBeforeEdit.contains("("))
                                        {
                                            nodeName = m_fileNameBeforeEdit;

                                        }

                                        nodeName.replace(acGTStringToQString(oldFileName), acGTStringToQString(newFileName));
                                        pChild->setText(0, nodeName);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

afTreeItemType kaApplicationTreeHandler::ExtractProgramChildType(const afApplicationTreeItemData* pProgramTreeItemData, kaSourceFile* pFile)
{
    afTreeItemType retVal = AF_TREE_ITEM_ITEM_NONE;

    kaProgram* pProgram = nullptr;

    if (pProgramTreeItemData != nullptr)
    {
        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pProgramTreeItemData->extendedItemData());

        if (pKAData != nullptr)
        {
            pProgram = pKAData->GetProgram();
        }
    }

    kaProgramTypes programType = kaProgramUnknown;
    kaFileTypes fileType = kaFileTypeUnknown;

    if (pProgram != nullptr)
    {
        programType = pProgram->GetBuildType();
    }

    if (pFile != nullptr)
    {
        fileType = pFile->FileType();
    }

    // add source file ID
    switch (programType)
    {
        case kaProgramCL:
        case kaProgramDX:
        {
            retVal = AF_TREE_ITEM_KA_PROGRAM_SHADER;
        }
        break;

        case kaProgramGL_Rendering:
        case kaProgramVK_Rendering:
        {
            switch (fileType)
            {
                case kaFileTypeGLSLVert:
                    retVal = AF_TREE_ITEM_KA_PROGRAM_GL_VERT;
                    break;

                case kaFileTypeGLSLTesc:
                    retVal = AF_TREE_ITEM_KA_PROGRAM_GL_TESC;
                    break;

                case kaFileTypeGLSLTese:
                    retVal = AF_TREE_ITEM_KA_PROGRAM_GL_TESE;
                    break;

                case kaFileTypeGLSLGeom:
                    retVal = AF_TREE_ITEM_KA_PROGRAM_GL_GEOM;
                    break;

                case kaFileTypeGLSLFrag:
                    retVal = AF_TREE_ITEM_KA_PROGRAM_GL_FRAG;
                    break;

                default:
                    break;
            }
        }
        break;

        case kaProgramGL_Compute:
        case kaProgramVK_Compute:
        {
            retVal = AF_TREE_ITEM_KA_PROGRAM_GL_COMP;
        }
        break;

        default:
            break;
    }

    return retVal;
}

void kaApplicationTreeHandler::RemoveFileFromPrograms(const QList<kaProgram*>& listOfRelatedPrograms, int fileId)
{
    for (kaProgram* pProgram : listOfRelatedPrograms)
    {
        if (pProgram != nullptr)
        {
            if (pProgram->HasFile(fileId, AF_TREE_ITEM_ITEM_NONE))
            {
                // Get the program and file items
                QTreeWidgetItem* pProgramItem = FindProgramTreeItem(pProgram);
                osFilePath filePath;
                KA_PROJECT_DATA_MGR_INSTANCE.GetFilePathByID(fileId, filePath);
                QTreeWidgetItem* pFileItem = FindProgramFileTreeItem(pProgramItem, filePath);

                GT_IF_WITH_ASSERT(pFileItem != nullptr && pProgramItem != nullptr)
                {
                    // Check what is the stage of the file
                    if ((pProgram->GetBuildType() == kaProgramCL) || (pProgram->GetBuildType() == kaProgramDX))
                    {
                        m_pApplicationTree->removeTreeItem(pFileItem, false);
                    }
                    else
                    {
                        afApplicationTreeItemData* pFileStageItemData = m_pApplicationTree->getTreeItemData(pFileItem);
                        GT_IF_WITH_ASSERT(pFileStageItemData != nullptr)
                        {
                            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pFileStageItemData->extendedItemData());
                            gtString stageNameAsString = ProgramItemTypeAsText(pFileStageItemData->m_itemType, L"");
                            pFileItem->setText(0, acGTStringToQString(stageNameAsString));
                            pFileItem->setTextColor(0, acQLIST_EDITABLE_ITEM_COLOR);

                            if (pKAData != nullptr)
                            {
                                pKAData->setFilePath(osFilePath());
                            }

                            pFileStageItemData->m_filePath = osFilePath();
                        }
                    }

                    pProgram->OnFileRemove(fileId);
                    KA_PROJECT_DATA_MGR_INSTANCE.Disconnect(fileId, pProgram);
                }
            }
        }
    }
}

void kaApplicationTreeHandler::StartStageShaderNameEditing(QLineEdit* pLineEditor)
{
    QString stageText = pLineEditor->text();

    if (!stageText.isEmpty())
    {
        int leftBracketPos = stageText.indexOf("(");
        int rightBracketPos = stageText.indexOf(")");

        if (leftBracketPos != -1 && rightBracketPos != -1)
        {
            stageText = stageText.mid(leftBracketPos + 1, rightBracketPos - leftBracketPos - 1);
            pLineEditor->setText(stageText);
            pLineEditor->selectAll();
        }
    }
}

void kaApplicationTreeHandler::StageNodeNameFromFileName(QString& newFileName, const QString& fileNameBeforeEdit) const
{
    if (!newFileName.isEmpty() && !fileNameBeforeEdit.isEmpty())
    {
        int leftBracketPos = fileNameBeforeEdit.indexOf("(");
        int rightBracketPos = fileNameBeforeEdit.indexOf(")");

        if (leftBracketPos != -1 && rightBracketPos != -1)
        {
            newFileName.insert(0, fileNameBeforeEdit.mid(0, leftBracketPos + 1));
            newFileName.append(")");
        }
    }
}

void kaApplicationTreeHandler::GetFileTypeString(kaFileTypes fileType, gtString& typeName) const
{
    switch (fileType)
    {
        case    kaFileTypeOpenCL:
            typeName = KA_STR_fileCLKernel;
            break;

        case kaFileTypeGLSLFrag:
            typeName = KA_STR_fileGLFrag;
            break;

        case kaFileTypeGLSLVert:
            typeName = KA_STR_fileGLVert;
            break;

        case kaFileTypeGLSLComp:
            typeName = KA_STR_fileGLComp;
            break;

        case kaFileTypeGLSLGeom:
            typeName = KA_STR_fileGLGeom;
            break;

        case kaFileTypeGLSLTesc:
            typeName = KA_STR_fileGLTesc;
            break;

        case kaFileTypeGLSLTese:
            typeName = KA_STR_fileGLTese;
            break;

        case kaFileTypeGLSLGenericShader:
            typeName = KA_STR_fileGLShader;
            break;

        case kaFileTypeUnknown:
            break;

        default:
            typeName = KA_STR_fileUnknown;
            break;
    }
}

afApplicationTreeItemData* kaApplicationTreeHandler::GetSelectedItemData()
{
    afApplicationTreeItemData* pRetVal = nullptr;

    // This function can be called when the toolbar is created. Sometimes, m_pApplicationTree is not initialized.
    // Do not assert at this point, this is a false assertion.
    if (m_pApplicationTree != nullptr)
    {
        if (m_pApplicationTree->treeControl()->selectedItems().size() > 0)
        {
            pRetVal = m_pApplicationTree->getTreeItemData(m_pApplicationTree->treeControl()->selectedItems().at(0));
        }

    }

    return pRetVal;
}

void kaApplicationTreeHandler::OnNewFile(afTreeItemType programChildItemType)
{
    if (kaBackendManager::instance().isInBuild())
    {
        acMessageBox::instance().critical(afGlobalVariablesManager::ProductNameA(), KA_STR_ERR_CANNOT_CREATE_SOURCE_FILE_DURING_BUILD);
    }
    else
    {
        afApplicationTreeItemData* pSelectedItemData = m_pApplicationTree->getTreeItemData(m_pApplicationTree->getTreeSelection());
        const afApplicationTreeItemData* pParentProgramItemData = FindParentItemDataOfType(pSelectedItemData, AF_TREE_ITEM_KA_PROGRAM);
        kaProgram* pProgram = nullptr;

        if (pParentProgramItemData != nullptr)
        {
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pParentProgramItemData->extendedItemData());

            if (pKAData != nullptr)
            {
                pProgram = pKAData->GetProgram();
            }
        }

        // Create a new file, and set it on the program (if not null), attached to stage (for rendering programs)
        osFilePath createdFilePath;

        kaPipelinedProgram::PipelinedStage stage = kaRenderingProgram::TreeItemTypeToRenderingStage(programChildItemType);


        if (pProgram != nullptr)
        {

            if (stage == kaPipelinedProgram::KA_PIPELINE_STAGE_NONE)
            {
                kaProgramTypes programType = pProgram->GetBuildType();

                if (programType == kaProgramGL_Compute || programType == kaProgramVK_Compute)
                {
                    stage = kaPipelinedProgram::KA_PIPELINE_STAGE_COMP;
                }
            }
        }

        kaApplicationCommands::instance().NewFileCommand(false, createdFilePath, pProgram, stage);

        if (createdFilePath.exists())
        {
            pParentProgramItemData = GetProgramItemData(pProgram);
            // we are under program branch
            AddFileNodeToProgramBranch(createdFilePath, pParentProgramItemData, programChildItemType);
        }
    }
}

afApplicationTreeItemData* kaApplicationTreeHandler::GetProgramItemData(kaProgram* pProgram) const
{
    // Find the program created or selected by the add file command
    QTreeWidgetItem* pProgramItem = FindProgramTreeItem(pProgram);
    return m_pApplicationTree->getTreeItemData(pProgramItem);
}

void kaApplicationTreeHandler::OnBuild()
{
    gtVector<osFilePath> filesForBuild;
    GT_IF_WITH_ASSERT(kaApplicationCommands::instance().activeCLFiles(filesForBuild) > 0)
    {
        kaApplicationCommands::instance().buildCommand(filesForBuild);
    }
}

void kaApplicationTreeHandler::OnCancelBuild()
{
    kaApplicationCommands::instance().cancelBuildCommand();
}


// ---------------------------------------------------------------------------
// Name:        kaApplicationTreeHandler::OnGotoSourceCode
// Description: handle the goto source context menu command
// Author:      Gilad Yarnitzky
// Date:        22/8/2013
// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::OnGotoSourceCode()
{
    GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr)
    {
        QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();
        QTreeWidgetItem* pContextMenuItem = treeSelectedItems.at(0);
        GT_IF_WITH_ASSERT(pContextMenuItem != nullptr)
        {
            afApplicationTreeItemData* pTreeItemData = m_pApplicationTree->getTreeItemData(pContextMenuItem);
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pTreeItemData->extendedItemData());

            if (nullptr != pKAData)
            {
                gtString fileNameToDisplay;
                pKAData->filePath().getFileNameAndExtension(fileNameToDisplay);
                // open the file using the generic command:
                afApplicationCommands::instance()->OpenFileAtLine(pKAData->filePath(), pKAData->lineNumber(), -1);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        kaApplicationTreeHandler::OnOpenContainingFolder
// Description: handle the open containing folder context menu command
// Author:      Gilad Yarnitzky
// Date:        22/8/2013
// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::OnOpenContainingFolder()
{
    GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr)
    {
        QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();
        QTreeWidgetItem* pContextMenuItem = treeSelectedItems.at(0);
        GT_IF_WITH_ASSERT(pContextMenuItem != nullptr)
        {
            afApplicationTreeItemData* pTreeItemData = m_pApplicationTree->getTreeItemData(pContextMenuItem);
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pTreeItemData->extendedItemData());

            if (nullptr != pKAData)
            {
                afApplicationCommands::instance()->OpenContainingFolder(pKAData->filePath());
            }
        }
    }
}

void kaApplicationTreeHandler::OnOpenOutputFolder()
{
    GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr)
    {
        QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();
        QTreeWidgetItem* pContextMenuItem = treeSelectedItems.at(0);
        GT_IF_WITH_ASSERT(pContextMenuItem != nullptr)
        {
            afApplicationTreeItemData* pTreeItemData = m_pApplicationTree->getTreeItemData(pContextMenuItem);
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pTreeItemData->extendedItemData());

            GT_IF_WITH_ASSERT((pTreeItemData != nullptr) && (pKAData != nullptr) && (pKAData->GetProgram() != nullptr))
            {
                if (pTreeItemData->m_itemType == AF_TREE_ITEM_KA_OUT_DIR)
                {
                    osDirectory output32Dir, output64Dir;
                    pKAData->GetProgram()->GetAndCreateOutputDirectories(output32Dir, output64Dir, false, false);

                    if (pContextMenuItem->text(0) == acGTStringToQString(KA_STR_UiOutputDir32))
                    {
                        afApplicationCommands::instance()->OpenContainingFolder(output32Dir.directoryPath());
                    }
                    else if (pContextMenuItem->text(0) == acGTStringToQString(KA_STR_UiOutputDir64))
                    {
                        afApplicationCommands::instance()->OpenContainingFolder(output64Dir.directoryPath());
                    }
                }
            }
        }
    }
}

void kaApplicationTreeHandler::OnRename()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pApplicationTree != nullptr) && (afApplicationCommands::instance() != nullptr))
    {
        afApplicationTreeItemData* pSelectedItemData = GetSelectedItemData();
        GT_IF_WITH_ASSERT(pSelectedItemData != nullptr)
        {
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pSelectedItemData->extendedItemData());
            QTreeWidgetItem* pSelectedItem = pSelectedItemData->m_pTreeWidgetItem;

            if ((pSelectedItemData != nullptr) && (pKAData != nullptr) && (pSelectedItem != nullptr))
            {
                // This slot can be accessed using the F2 shortcut key. We should not allow renaming of objects that cannot be renamed.
                // We only allow rename of file items, and file references.
                bool isFile = ((pSelectedItemData->m_itemType >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) && (pSelectedItemData->m_itemType <= AF_TREE_ITEM_KA_LAST_FILE_ITEM_TYPE));
                isFile = isFile || (pSelectedItemData->m_itemType == AF_TREE_ITEM_KA_FILE);
                bool isProgram = (pSelectedItemData->m_itemType <= AF_TREE_ITEM_KA_PROGRAM);

                // Rename is only supported for nodes that its file path is stored
                if (isFile)
                {
                    isFile = isFile && !pKAData->filePath().isEmpty();
                }

                if (isFile || isProgram)
                {
                    // If there are opened windows, ask the user for permission to close it
                    bool isRenamePossible = IsRenamePossible(pSelectedItemData);

                    if (isRenamePossible)
                    {
                        m_fileNameBeforeEdit = pSelectedItem->text(0);
                        m_pRenamedItem = pSelectedItem;

                        if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
                        {
                            //fixing CodeXL- 2636 unable to rename
                            // Make the item editable and edit it:
                            pSelectedItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
                            m_pApplicationTree->setFocus();
                            m_pApplicationTree->editCurrentItem();
                        }
                        else
                        {
                            m_shouldTakeRenameAction = true;
                        }
                    }
                }
            }
        }
    }
}
void kaApplicationTreeHandler::OnRemoveProgramFromProject()
{
    GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr && m_pApplicationTree->treeControl() != nullptr)
    {
        //block selection changed signal to fix crash - todo: improve the solution
        bool rc = disconnect(m_pApplicationTree->treeControl(), SIGNAL(itemSelectionChanged()), m_pApplicationTree, SLOT(onItemSelectionChanged()));
        GT_ASSERT(rc);
        const QList<QTreeWidgetItem*>& treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();

        // Go over the list of items to delete, and look for their references in the project programs
        QList<kaProgram*> listOfProgramsToDelete;

        for (int i = 0; i < treeSelectedItems.count(); i++)
        {
            afApplicationTreeItemData* pTreeItemData = m_pApplicationTree->getTreeItemData(treeSelectedItems.at(i));
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pTreeItemData->extendedItemData());

            if (AF_TREE_ITEM_KA_PROGRAM == pTreeItemData->m_itemType && nullptr != pKAData && pKAData->GetProgram() != nullptr)
            {
                listOfProgramsToDelete.push_back(pKAData->GetProgram());
            }
        }

        if (treeSelectedItems.count() > 0 && listOfProgramsToDelete.empty() == false)
        {
            QString messageStr = KA_STR_treeRemoveProgramWarning;
            int userInput = acMessageBox::instance().question(afGlobalVariablesManager::ProductNameA(), messageStr, QMessageBox::Yes | QMessageBox::No);

            if (userInput == QMessageBox::Yes)
            {
                for (int i = 0; i < (int)listOfProgramsToDelete.size(); i++)
                {
                    QTreeWidgetItem* pProgramItem = FindProgramTreeItem(listOfProgramsToDelete[i]);
                    GT_IF_WITH_ASSERT(pProgramItem != nullptr)
                    {
                        afApplicationTreeItemData* pTreeItemData = m_pApplicationTree->getTreeItemData(pProgramItem);
                        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pTreeItemData->extendedItemData());

                        if (nullptr != pKAData)
                        {
                            // Remove the file data from the tree:
                            m_pApplicationTree->removeTreeItem(pProgramItem, false);

                            // Clear active item if needed:
                            if (m_pActiveItemData == pTreeItemData)
                            {
                                m_pActiveItemData = nullptr;
                            }

                            KA_PROJECT_DATA_MGR_INSTANCE.RemoveProgram(listOfProgramsToDelete[i], true);
                        }
                    }
                }
            }
        }

        rc = connect(m_pApplicationTree->treeControl(), SIGNAL(itemSelectionChanged()), m_pApplicationTree, SLOT(onItemSelectionChanged()));
        GT_ASSERT(rc);
    }
}

void kaApplicationTreeHandler::OnAddCreateShader()
{
    GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr)
    {
        QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();
        bool shouldCreate = (sender() == m_pCreateShaderAction);

        if (treeSelectedItems.count() == 1)
        {
            QTreeWidgetItem* pContextMenuItem = treeSelectedItems.at(0);
            GT_IF_WITH_ASSERT(pContextMenuItem != nullptr)
            {
                afApplicationTreeItemData* pTreeItemData = m_pApplicationTree->getTreeItemData(pContextMenuItem);
                kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pTreeItemData->extendedItemData());

                if (nullptr != pKAData)
                {
                    GT_IF_WITH_ASSERT((pTreeItemData->m_itemType >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) && (pTreeItemData->m_itemType <= AF_TREE_ITEM_KA_PROGRAM_GL_COMP))
                    {
                        if (shouldCreate)
                        {
                            OnNewFile(pTreeItemData->m_itemType);
                        }
                        else
                        {
                            OnAddFile(pTreeItemData->m_itemType);
                        }
                    }
                }
            }
        }
    }
}

void kaApplicationTreeHandler::OnRemove()
{
    // Check what is the selected items types, and call the appropriate function
    afTreeItemType selectedItemType = GetSelectedItemType();

    switch (selectedItemType)
    {
        case AF_TREE_ITEM_KA_PROGRAM:
            OnRemoveProgramFromProject();
            break;

 
        case AF_TREE_ITEM_KA_PROGRAM_GL_GEOM:
        case AF_TREE_ITEM_KA_PROGRAM_GL_FRAG:
        case AF_TREE_ITEM_KA_PROGRAM_GL_TESC:
        case AF_TREE_ITEM_KA_PROGRAM_GL_TESE:
        case AF_TREE_ITEM_KA_PROGRAM_GL_VERT:
        case AF_TREE_ITEM_KA_PROGRAM_GL_COMP:
        case AF_TREE_ITEM_KA_PROGRAM_SHADER:
            OnRemoveShaderFromProgram();
            break;

        default:
            break;
    }
}


void kaApplicationTreeHandler::OnRemoveShaderFromProgram()
{
    GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr && m_pApplicationTree->treeControl() != nullptr)
    {
        //block selection changed signal to fix crash - todo: improve the solution
        bool rc = disconnect(m_pApplicationTree->treeControl(), SIGNAL(itemSelectionChanged()), m_pApplicationTree, SLOT(onItemSelectionChanged()));
        GT_ASSERT(rc);
        QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();

        if (treeSelectedItems.count() > 0)
        {
            for (int i = 0; i < treeSelectedItems.count(); i++)
            {
                QTreeWidgetItem* pContextMenuItem = treeSelectedItems.at(i);
                GT_IF_WITH_ASSERT(pContextMenuItem != nullptr)
                {
                    afApplicationTreeItemData* pTreeItemData = m_pApplicationTree->getTreeItemData(pContextMenuItem);
                    kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pTreeItemData->extendedItemData());
                   
                    const gtVector<kaProgram*> allPrograms = KA_PROJECT_DATA_MGR_INSTANCE.GetPrograms();
                    
                    if(pKAData != nullptr)
                    {
                        osFilePath filePath = pKAData->filePath();
                        int fileId = KA_PROJECT_DATA_MGR_INSTANCE.GetFileID(filePath);
                        // Count references
                        int fileRefCount = 0;
                        
                    
                        for (kaProgram* pProjectProgram : allPrograms)
                        {
                            if (pProjectProgram != nullptr && pProjectProgram->HasFile(fileId, AF_TREE_ITEM_ITEM_NONE))
                            {
                                fileRefCount++;
                            }
                        }
                        // If the file being removed has single reference in this project it should be removed from project
                        bool shouldRemoveFromProject = (fileRefCount == 1);

                        if (nullptr != pKAData)
                        {
                            if ((pTreeItemData->m_itemType >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) && (pTreeItemData->m_itemType <= AF_TREE_ITEM_KA_PROGRAM_GL_VERT))
                            {
                                kaRenderingProgram* pRenderingProgram = dynamic_cast<kaRenderingProgram*>(pKAData->GetProgram());
                                GT_IF_WITH_ASSERT(pRenderingProgram != nullptr)
                                {
                                    kaPipelinedProgram::PipelinedStage stage = kaRenderingProgram::TreeItemTypeToRenderingStage(pTreeItemData->m_itemType);
                                    pRenderingProgram->SetFileID(stage, -1);
                                    gtString stageNameAsString = ProgramItemTypeAsText(pTreeItemData->m_itemType, L"");
                                    pContextMenuItem->setText(0, acGTStringToQString(stageNameAsString));
                                    pContextMenuItem->setTextColor(0, acQLIST_EDITABLE_ITEM_COLOR);
                                    pKAData->setFilePath(osFilePath());
                                    pTreeItemData->m_filePath = osFilePath();
                                }
                            }
                            else if (pTreeItemData->m_itemType == AF_TREE_ITEM_KA_PROGRAM_GL_COMP)
                            {
                                kaComputeProgram* pComputeProgram = dynamic_cast<kaComputeProgram*>(pKAData->GetProgram());
                                GT_IF_WITH_ASSERT(pComputeProgram != nullptr)
                                {
                                    pComputeProgram->SetFileID(-1);
                                    gtString stageNameAsString = ProgramItemTypeAsText(pTreeItemData->m_itemType, L"");
                                    pContextMenuItem->setText(0, acGTStringToQString(stageNameAsString));
                                    pContextMenuItem->setTextColor(0, acQLIST_EDITABLE_ITEM_COLOR);
                                    pKAData->setFilePath(osFilePath());
                                    pTreeItemData->m_filePath = osFilePath();
                                }
                            }
                            else if (pTreeItemData->m_itemType == AF_TREE_ITEM_KA_PROGRAM_SHADER)
                            {
                                kaNonPipelinedProgram* pNPProgram = dynamic_cast<kaNonPipelinedProgram*>(pKAData->GetProgram());
                                GT_IF_WITH_ASSERT(pNPProgram != nullptr)
                                {
                                    int fileId = KA_PROJECT_DATA_MGR_INSTANCE.GetFileID(pKAData->filePath());
                                    GT_IF_WITH_ASSERT(fileId >= 0)
                                    {
                                        pNPProgram->RemoveFile(fileId);
                                        const afApplicationTreeItemData* pProgramItemData = FindParentItemDataOfType(pTreeItemData, AF_TREE_ITEM_KA_PROGRAM);
                                        GT_IF_WITH_ASSERT((pProgramItemData != nullptr) && (pProgramItemData->m_pTreeWidgetItem != nullptr))
                                        {
                                            m_pApplicationTree->removeTreeItem(pTreeItemData->m_pTreeWidgetItem, false);
                                        }
                                    }
                                }
                            }

                            //removing entry from fileId-program multimap
                            KA_PROJECT_DATA_MGR_INSTANCE.Disconnect(fileId, pKAData->GetProgram());
                            if (shouldRemoveFromProject)
                            {
                                KA_PROJECT_DATA_MGR_INSTANCE.removeFile(filePath);
                            }
                        }
                    }
                }
            }
        }

        rc = connect(m_pApplicationTree->treeControl(), SIGNAL(itemSelectionChanged()), m_pApplicationTree, SLOT(onItemSelectionChanged()));
        GT_ASSERT(rc);
    }
}
// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::DeleteTreeDataExtension(kaTreeDataExtension* pKAData)
{
    if (nullptr != pKAData)
    {
        osFilePath filePathToRemove = pKAData->filePath();
        // Remove the file data from the project data manager:
        KA_PROJECT_DATA_MGR_INSTANCE.removeFile(filePathToRemove);

        // Delete all files:
        osFilePath directoryFilePath = kaApplicationCommands::instance().OutputFilePathForCurrentProject();
        gtString fileName;
        filePathToRemove.getFileName(fileName);
        directoryFilePath.appendSubDirectory(fileName);
        osDirectory directoryPath(directoryFilePath);

        if (directoryPath.exists())
        {
            directoryPath.deleteRecursively();
        }

        // close all open MDI views that were connected to the files in the directory
        // go through all MDI and check that the file related to the MDI still exists:
        afApplicationCommands::instance()->closeDocumentsOfDeletedFiles();
    }
}

// ---------------------------------------------------------------------------
// Name:        kaApplicationTreeHandler::getOverviewHtmlInfo
// Description: return the html string of the overview and adds it to the overview node
//              to be used in the overview view and properties view
// Author:      Gilad Yarnitzky
// Date:        26/8/2013
// ---------------------------------------------------------------------------
bool kaApplicationTreeHandler::getOverviewHtmlInfo(const osFilePath& filePath, afHTMLContent& htmlContent)
{
    bool retVal = false;

    // Get the overview node:
    afApplicationTreeItemData nodeToLookFor;
    nodeToLookFor.m_itemType = AF_TREE_ITEM_KA_OVERVIEW;
    kaTreeDataExtension nodeExtendedData;
    nodeExtendedData.setFilePath(filePath);
    nodeToLookFor.setExtendedData(&nodeExtendedData);

    // Get the overview node and check if it has the cached info:
    afApplicationTreeItemData* overviewTreeNode = kaApplicationTreeHandler::instance()->FindMatchingTreeItem(nodeToLookFor);

    if (nullptr != overviewTreeNode)
    {
        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(overviewTreeNode->extendedItemData());

        if (nullptr != pKAData)
        {
            kaBuildHTMLFileInfo(filePath, htmlContent);
            retVal = true;
        }
    }
    else
    {
        kaBuildHTMLFileInfo(filePath, htmlContent);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::SetItemsVisibility()
{
    // Check if the kernel analyzer mode is currently active, and show / hide all the KA items in tree:
    const bool isProjectOpened = afProjectManager::instance().currentProjectSettings().projectName().isEmpty() == false;
    const bool shouldShow = isProjectOpened && afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode);

    GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr)
    {
        QTreeWidgetItem* pRootItem = m_pApplicationTree->headerItem();

        if (pRootItem != nullptr)
        {
            int childCount = pRootItem->childCount();

            for (int i = 0; i < childCount; i++)
            {
                afApplicationTreeItemData* pChildData = m_pApplicationTree->getTreeItemData(pRootItem->child(i));

                if (pChildData != nullptr)
                {
                    if ((pChildData->m_itemType == AF_TREE_ITEM_KA_FILE)        ||
                        (pChildData->m_itemType == AF_TREE_ITEM_KA_PROGRAM)     ||
                        (pChildData->m_itemType == AF_TREE_ITEM_KA_NEW_PROGRAM) ||
                        (pChildData->m_itemType == AF_TREE_ITEM_KA_NEW_FILE) ||
                        (pChildData->m_itemType == AF_TREE_ITEM_KA_ADD_FILE))
                    {
                        if (pChildData->m_pTreeWidgetItem != nullptr)
                        {
                            pChildData->m_pTreeWidgetItem->setHidden(!shouldShow);
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::onEvent(const apEvent& eve, bool& vetoEvent)
{
    Q_UNUSED(vetoEvent);

    if ((eve.eventType() == apEvent::GD_MONITORED_OBJECT_SELECTED_EVENT) && (afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode)))
    {
        // Get the activation event:
        const apMonitoredObjectsTreeSelectedEvent& activationEvent = (const apMonitoredObjectsTreeSelectedEvent&)eve;

        // Get the pItem data;
        afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)activationEvent.selectedItemData();

        if (pItemData != nullptr)
        {
            // Display the item:
            onCurrentTreeNodeChanged(pItemData->m_pTreeWidgetItem);
        }
    }
    else if ((eve.eventType() == apEvent::GD_MONITORED_OBJECT_ACTIVATED_EVENT) && (afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode)))
    {
        // Get the activation event:
        const apMonitoredObjectsTreeActivatedEvent& activationEvent = (const apMonitoredObjectsTreeActivatedEvent&)eve;

        // Get the pItem data;
        afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)activationEvent.selectedItemData();

        if (pItemData != nullptr)
        {
            bool shouldCreateNewFile = (pItemData->m_itemType == AF_TREE_ITEM_KA_NEW_FILE);
            afTreeItemType stageItemType = AF_TREE_ITEM_ITEM_NONE;

            if ((pItemData->m_itemType >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) && (pItemData->m_itemType <= AF_TREE_ITEM_KA_PROGRAM_GL_COMP))
            {
                kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

                if ((pKAData != nullptr) && (pKAData->filePath().isEmpty()))
                {
                    stageItemType = pItemData->m_itemType;
                    shouldCreateNewFile = true;
                }
            }

            if ((nullptr != m_pNewProgramTreeItem) && (pItemData->m_pTreeWidgetItem == m_pNewProgramTreeItem))
            {
                // Create new program
                OnNewProgram();
            }
            else if (pItemData->m_itemType == AF_TREE_ITEM_KA_ADD_FILE)
            {
                // Add an existing file:
                OnAddFile();
            }

            else if (shouldCreateNewFile)
            {
                // Create a new file:
                OnNewFile(stageItemType);
            }
        }
    }

    else if (eve.eventType() == apEvent::AP_EXECUTION_MODE_CHANGED_EVENT)
    {
        // Notice:
        // Some of the other plug-in will probably want to use "Delete" key for its actions.
        // Therefore, we only enable the delete action while in profile mode, so that we "free" the shortcut to other plug-ins:
        // Check if we're switching to profile mode:
        const apExecutionModeChangedEvent& execChangedEvent = (const apExecutionModeChangedEvent&)eve;
        bool isKAMode = (execChangedEvent.modeType() == KA_STR_executionMode);

        // If this is a mode change and not a session type change:
        if (!execChangedEvent.onlySessionTypeIndex())
        {
            if ((m_pRemoveAction != nullptr) && (m_pRenameFileAction != nullptr))
            {
                m_pRemoveAction->setEnabled(isKAMode);
                m_pRenameFileAction->setEnabled(isKAMode);
                m_pRenameFileAction->setText(KA_STR_renameFileStatusbarStringASCII);
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::onCurrentTreeNodeChanged(QTreeWidgetItem* pItem)
{
    // m_pFilesRootNode can be null if there is no project loaded
    if (pItem != nullptr && m_pProgramsRootNode != nullptr && (pItem->parent() == m_pProgramsRootNode))
    {
        // if the node of the active item was deleted and the active item selection was changed
        if ((nullptr != m_pActiveItemData))
        {
            if (m_pProgramsRootNode->indexOfChild(m_pActiveItemData->m_pTreeWidgetItem) < 0)
            {
                m_pActiveItemData = nullptr;
            }
        }

        afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pItem);

        if (pItemData != nullptr)
        {
            // Get the related CL file item data:
            const afApplicationTreeItemData* pRelatedCLFileItemData = FindParentItemDataOfType(pItemData, AF_TREE_ITEM_KA_FILE);

            if (pRelatedCLFileItemData != nullptr)
            {
                m_pActiveItemData = pRelatedCLFileItemData;
            }
        }
    }

    if (nullptr == m_pActiveItemData)
    {
        // Make sure a file node is selected:
        SelectFirstSourceFileNode();
    }

    emit KADocumentSelectionChanged();

    // process the event only for SA. for VS handlles the same event seperattly
    if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        // update the toolbars:
        afApplicationCommands::instance()->updateToolbarCommands();
    }
}

// ---------------------------------------------------------------------------
const afApplicationTreeItemData* kaApplicationTreeHandler::FindParentItemDataOfType(const afApplicationTreeItemData* pTreeItemData, afTreeItemType parentItemType) const
{
    const afApplicationTreeItemData* pRetVal = nullptr;

    const afApplicationTreeItemData* pTempItem = pTreeItemData;
    GT_IF_WITH_ASSERT((pTreeItemData != nullptr) && (m_pApplicationTree != nullptr) && (m_pApplicationTree->headerItem() != nullptr))
    {
        if (pTreeItemData->m_itemType == parentItemType)
        {
            pRetVal = pTreeItemData;
        }
        else
        {
            while ((pRetVal == nullptr) && (pTempItem != nullptr) && (pTempItem->m_pTreeWidgetItem != nullptr) && (pTempItem->m_pTreeWidgetItem != m_pApplicationTree->headerItem()))
            {
                if (pTempItem->m_itemType == parentItemType)
                {
                    pRetVal = (afApplicationTreeItemData*)pTempItem;
                    break;
                }

                // Continue to search in parent:
                QTreeWidgetItem* pParentItem = pTempItem->m_pTreeWidgetItem->parent();

                if (pParentItem != nullptr)
                {
                    pTempItem = m_pApplicationTree->getTreeItemData(pParentItem);
                }
                else
                {
                    pTempItem = nullptr;
                }
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
unsigned int kaApplicationTreeHandler::activeBuildFiles(gtVector<osFilePath>& filePaths)
{
    unsigned int retVal = 0;
    filePaths.clear();
    kaTreeDataExtension* pKAData = nullptr;
    GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr)
    {
        QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();

        foreach (QTreeWidgetItem* pSelectedItem, treeSelectedItems)
        {
            if (pSelectedItem != nullptr)
            {
                afApplicationTreeItemData* pTreeItemData = m_pApplicationTree->getTreeItemData(pSelectedItem);
                GT_IF_WITH_ASSERT(pTreeItemData != nullptr)
                {
                    if (pTreeItemData->m_itemType == AF_TREE_ITEM_KA_PROGRAM_SHADER)
                    {
                        pKAData = qobject_cast<kaTreeDataExtension*>(pTreeItemData->extendedItemData());

                        if (nullptr != pKAData && !pKAData->filePath().isEmpty())
                        {
                            // Add the file if not already in the list:
                            if (std::find(filePaths.begin(), filePaths.end(), pKAData->filePath()) == filePaths.end())
                            {
                                filePaths.push_back(pKAData->filePath());
                            }
                        }
                    }
                }
            }
        }

        retVal = filePaths.size();
    }

    if (retVal == 0)
    {
        // get current active file
        const afApplicationTreeItemData* pItemData = GetSelectedItemData();
        kaProgram* pProgram = nullptr;

        if (pItemData != nullptr)
        {
            switch (pItemData->m_itemType)
            {
                case  AF_TREE_ITEM_KA_PROGRAM_GL_VERT:
                case  AF_TREE_ITEM_KA_PROGRAM_GL_TESC:
                case  AF_TREE_ITEM_KA_PROGRAM_GL_TESE:
                case  AF_TREE_ITEM_KA_PROGRAM_GL_GEOM:
                case  AF_TREE_ITEM_KA_PROGRAM_GL_FRAG:
                case  AF_TREE_ITEM_KA_PROGRAM_GL_COMP:
                case AF_TREE_ITEM_KA_PROGRAM:
                    pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

                    if (nullptr != pKAData)
                    {
                        kaProgram* pProgram = pKAData->GetProgram();

                        if (pProgram != nullptr)
                        {
                            pProgram->GetProgramFiles(filePaths);
                        }

                        retVal = filePaths.size();
                    }

                    break;

                case AF_TREE_ITEM_KA_OUT_DIR:
                case AF_TREE_ITEM_KA_STATISTICS:
                case AF_TREE_ITEM_KA_DEVICE:
                case AF_TREE_ITEM_KA_REF_TYPE:
                {
                    pProgram = KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram();

                    if (pProgram != nullptr)
                    {
                        pProgram->GetProgramFiles(filePaths);
                    }

                    retVal = filePaths.size();
                }
                break;

                case AF_TREE_ITEM_KA_ADD_FILE:
                case AF_TREE_ITEM_KA_NEW_FILE:
                {
                    QTreeWidgetItem* pSelectedItem = pItemData->m_pTreeWidgetItem;

                    if (pSelectedItem != nullptr)
                    {
                        QTreeWidgetItem* pParentItem = pSelectedItem->parent();

                        if (pParentItem != nullptr)
                        {
                            QVariant itemData = pParentItem->data(0, Qt::UserRole);
                            afApplicationTreeItemData* pParentData = (afApplicationTreeItemData*)itemData.value<void*>();

                            if ((pParentData != nullptr) && (pParentData->m_itemType == AF_TREE_ITEM_KA_PROGRAM))
                            {
                                pProgram = KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram();

                                if (pProgram != nullptr)
                                {
                                    pProgram->GetProgramFiles(filePaths);
                                }

                                retVal = filePaths.size();
                            }
                        }
                    }
                }
                break;

                default:
                    break;
            }
        }
    }

    if (retVal == 0)
    {
        filePaths = KA_PROJECT_DATA_MGR_INSTANCE.GetLastBuildFiles();
        retVal = filePaths.size();
    }

    return retVal;
}

bool kaApplicationTreeHandler::AddFileAddTreeItem(QTreeWidgetItem* pParent)
{
    bool retVal = false;

    // if called from AddProgram
    if (pParent != nullptr)
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr)
        {
            afApplicationTreeItemData* pTreeItemData = m_pApplicationTree->getTreeItemData(pParent);
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pTreeItemData->extendedItemData());

            if (nullptr != pKAData)
            {
                // Add the item to the tree:
                afApplicationTreeItemData* pItemData = new afApplicationTreeItemData;
                pItemData->m_itemType = AF_TREE_ITEM_KA_ADD_FILE;
                pItemData->m_isItemRemovable = false;

                // extension data is needed even empty so the search will work correctly:
                kaTreeDataExtension* pExtensionData;
                pExtensionData = new kaTreeDataExtension;
                pExtensionData->setFilePath(pKAData->filePath());
                pItemData->setExtendedData(pExtensionData);
                QTreeWidgetItem* pNewItem = m_pApplicationTree->addTreeItem(KA_STR_treeAddFileNode, pItemData, pParent);

                GT_IF_WITH_ASSERT(pNewItem != nullptr)
                {
                    retVal = true;
                    pNewItem->setTextColor(0, acQLIST_EDITABLE_ITEM_COLOR);
                    pNewItem->setIcon(0, m_pIconsArray[KA_PIXMAP_ADD_FILE]);
                    pNewItem->setToolTip(0, KA_STR_treeAddFileNodeTooltip);
                }
            }
        }
    }


    return retVal;
}

// ---------------------------------------------------------------------------
bool kaApplicationTreeHandler::AddFileCreateTreeItem(QTreeWidgetItem* pParent)
{
    bool retVal = false;

    // if called from AddProgram
    if (pParent != nullptr)
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr)
        {
            afApplicationTreeItemData* pTreeItemData = m_pApplicationTree->getTreeItemData(pParent);
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pTreeItemData->extendedItemData());

            if (nullptr != pKAData)
            {
                // Add the item to the tree:
                afApplicationTreeItemData* pItemData = new afApplicationTreeItemData;
                pItemData->m_itemType = AF_TREE_ITEM_KA_NEW_FILE;
                pItemData->m_isItemRemovable = false;

                // extension data is needed even empty so the search will work correctly:
                kaTreeDataExtension* pExtensionData;
                pExtensionData = new kaTreeDataExtension;
                pExtensionData->setFilePath(pKAData->filePath());
                pItemData->setExtendedData(pExtensionData);
                QTreeWidgetItem* pNewItem = m_pApplicationTree->addTreeItem(KA_STR_treeNewFileNode, pItemData, pParent);

                GT_IF_WITH_ASSERT(pNewItem != nullptr)
                {
                    retVal = true;
                    pNewItem->setTextColor(0, acQLIST_EDITABLE_ITEM_COLOR);
                    pNewItem->setIcon(0, m_pIconsArray[KA_PIXMAP_NEW_FILE]);
                    pNewItem->setToolTip(0, KA_STR_treeNewFileNodeTooltip);
                }
            }
        }
    }

    afApplicationTreeItemData nodeToLookFor;
    nodeToLookFor.m_itemType = AF_TREE_ITEM_KA_NEW_FILE;
    kaTreeDataExtension nodeExtendedData;
    nodeToLookFor.setExtendedData(&nodeExtendedData);



    return retVal;
}


// ---------------------------------------------------------------------------
bool kaApplicationTreeHandler::AddProgramCreateTreeItem()
{
    bool retVal = false;

    afApplicationTreeItemData nodeToLookFor;
    nodeToLookFor.m_itemType = AF_TREE_ITEM_KA_NEW_PROGRAM;
    kaTreeDataExtension nodeExtendedData;
    nodeToLookFor.setExtendedData(&nodeExtendedData);

    // Get the source node which will hold all the created nodes and information:
    afApplicationTreeItemData* fileTreeNode = FindMatchingTreeItem(nodeToLookFor);

    if (fileTreeNode != nullptr)
    {
        m_pNewProgramTreeItem = fileTreeNode->m_pTreeWidgetItem;
        retVal = true;
    }
    else
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr)
        {
            if (m_pProgramsRootNode == nullptr)
            {
                m_pProgramsRootNode = m_pApplicationTree->headerItem();
            }

            GT_IF_WITH_ASSERT(m_pProgramsRootNode != nullptr)
            {
                // Add the item to the tree:
                afApplicationTreeItemData* pItemData = new afApplicationTreeItemData;
                pItemData->m_itemType = AF_TREE_ITEM_KA_NEW_PROGRAM;
                pItemData->m_isItemRemovable = false;

                // extension data is needed even empty so the search will work correctly:
                kaTreeDataExtension* pExtensionData;
                pExtensionData = new kaTreeDataExtension;
                pItemData->setExtendedData(pExtensionData);

                m_pNewProgramTreeItem = m_pApplicationTree->addTreeItem(KA_STR_treeNewProgramNode, pItemData, m_pProgramsRootNode);

                retVal = (m_pNewProgramTreeItem != nullptr);

                GT_IF_WITH_ASSERT(retVal)
                {
                    m_pNewProgramTreeItem->setTextColor(0, acQLIST_EDITABLE_ITEM_COLOR);
                    m_pNewProgramTreeItem->setIcon(0, m_pIconsArray[KA_PIXMAP_NEW_FILE]);
                }

                // Expand the item:
                m_pProgramsRootNode->setExpanded(true);
            }
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::selectFileNode(const osFilePath& clFilePath)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pProgramsRootNode != nullptr)
    {
        // Make sure that the file root is empty
        int count = m_pProgramsRootNode->childCount();

        for (int i = 0; i < count; i++)
        {
            // Get the next child:
            QTreeWidgetItem* pChild = m_pProgramsRootNode->child(i);

            if (pChild != nullptr)
            {
                afApplicationTreeItemData* pCurrentData = m_pApplicationTree->getTreeItemData(pChild);
                GT_IF_WITH_ASSERT(pCurrentData != nullptr)
                {
                    kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pCurrentData->extendedItemData());

                    if (pKAData != nullptr)
                    {
                        if (pKAData->filePath() == clFilePath)
                        {
                            // This is the requested node, set it as current:
                            m_pApplicationTree->treeControl()->setCurrentItem(pCurrentData->m_pTreeWidgetItem);
                            m_pActiveItemData = pCurrentData;
                            break;
                        }
                    }
                }
            }
        }
    }
}


void kaApplicationTreeHandler::OnBuildComplete(const gtString& filePath)
{
    GT_UNREFERENCED_PARAMETER(filePath);

    // get the program that was build
    kaProgram* pBuiltProgram = KA_PROJECT_DATA_MGR_INSTANCE.GetLastBuildProgram();
    GT_IF_WITH_ASSERT(pBuiltProgram != nullptr)
    {
        // check if this program wasn't deleted during build
        kaProgram* pSameProgramCheck = KA_PROJECT_DATA_MGR_INSTANCE.GetProgram(pBuiltProgram->GetProgramName());

        if ((pSameProgramCheck != nullptr) && (pSameProgramCheck == pBuiltProgram))
        {
            BuildProgramOutputTree(pBuiltProgram, KA_PROJECT_DATA_MGR_INSTANCE.GetBuildArchitecture());

            // update all the file paths of the program
            if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
            {
                gtVector<osFilePath> filesForBuild;
                GT_IF_WITH_ASSERT(activeBuildFiles(filesForBuild) > 0)
                {
                    for (const auto& it : filesForBuild)
                    {
                        kaApplicationCommands::instance().updateOpenViews(it);
                    }
                }
            }

            if (kaBackendManager::instance().IsBuildSucceded())
            {
                OpenLastBuildStatisticsNode(pBuiltProgram);
            }
        }
    }
}

void kaApplicationTreeHandler::OpenLastBuildStatisticsNode(kaProgram* pActiveProgram) const
{
    GT_ASSERT(pActiveProgram != nullptr)
    QTreeWidgetItem* pProgramTreeItem = GetProgramTreeItem(pActiveProgram);
    GT_IF_WITH_ASSERT(pProgramTreeItem != nullptr && m_pApplicationTree != nullptr)
    {
        // find the program node and the build node for the x32/x64 build
        QString searcNodeName = QString::fromStdWString(KA_PROJECT_DATA_MGR_INSTANCE.GetBuildArchitecture() == kaBuildArch32_bit ? KA_STR_UiOutputDir32 : KA_STR_UiOutputDir64);
        QTreeWidgetItem*  pSearchNode = m_pApplicationTree->treeControl()->FindChild(pProgramTreeItem, searcNodeName);

        if (nullptr != pSearchNode)
        {
            searcNodeName = QString::fromWCharArray(KA_STR_buildMainStatisticsFileName);

            list<QTreeWidgetItem*> statsNodes;
            m_pApplicationTree->treeControl()->FindDescendants(pSearchNode, searcNodeName, statsNodes);

            GT_IF_WITH_ASSERT(statsNodes.empty() == false)
            {
                gtVector<int> lastBuildIds = kaBackendManager::instance().GetLastBuildProgramFileIds();
                set<osFilePath> lastBuldFilePaths = KA_PROJECT_DATA_MGR_INSTANCE.GetFilePathsByIDs(lastBuildIds);

                GT_IF_WITH_ASSERT(lastBuldFilePaths.empty() == false)
                {
                    for (QTreeWidgetItem* pSearchNode : statsNodes)
                    {
                        afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pSearchNode);
                        GT_IF_WITH_ASSERT(pItemData != nullptr)
                        {
                            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

                            if (pKAData != nullptr)
                            {
                                osFilePath path = pKAData->filePath();

                                if (lastBuldFilePaths.find(path) != lastBuldFilePaths.end())
                                {
                                    m_pApplicationTree->scrollToItem(pSearchNode);
                                    pProgramTreeItem->setSelected(false);

                                    //unselect items besides statistics node to be opened
                                    for (int i = 0; i < pProgramTreeItem->childCount(); ++i)
                                    {
                                        pProgramTreeItem->child(i)->setSelected(false);
                                    }

                                    pSearchNode->setSelected(true);
                                    //open statistics node
                                    emit m_pApplicationTree->treeControl()->itemActivated(pSearchNode, 0);
                                    //found last build statistics node, breaking
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void kaApplicationTreeHandler::OnCLFileOpen()
{
    // Get the clicked item from application tree:
    GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr)
    {
        QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();
        QTreeWidgetItem* pContextMenuItem = treeSelectedItems.at(0);
        GT_IF_WITH_ASSERT(pContextMenuItem != nullptr)
        {
            afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pContextMenuItem);
            GT_IF_WITH_ASSERT(pItemData != nullptr)
            {
                // Expand and activate the session item:
                m_pApplicationTree->expandItem(pItemData->m_pTreeWidgetItem);
                m_pApplicationTree->selectItem(pItemData, true);
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::SelectFirstSourceFileNode()
{
    if ((nullptr == m_pActiveItemData) && (nullptr != m_pProgramsRootNode))
    {
        bool found = false;
        int numChild = m_pProgramsRootNode->childCount();

        for (int nChild = 0; (nChild < numChild) && !found; nChild++)
        {
            QTreeWidgetItem* pCurrentItem = m_pProgramsRootNode->child(nChild);

            if (nullptr != pCurrentItem)
            {
                afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pCurrentItem);
                GT_IF_WITH_ASSERT(pItemData != nullptr)
                {
                    // Look for a cl file node and one if found select it and we are done:
                    if (pItemData->m_itemType == AF_TREE_ITEM_KA_FILE)
                    {
                        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

                        if (nullptr != pKAData)
                        {
                            selectFileNode(pKAData->filePath());
                            found = true;
                        }
                    }
                }
            }
        }
    }
}

void kaApplicationTreeHandler::UpdateIncludeCheckBoxes(kaExportBinariesDialog* pDlg, const bool disable32BitCheckBox, const bool disable64BitCheckbox) const
{
    GT_ASSERT(pDlg != nullptr);
    const kaProgram* pActiveProgram = KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram();
    const kaProgramTypes activeProgramBuildType = pActiveProgram == nullptr ? kaProgramUnknown : pActiveProgram->GetBuildType();
    const bool isOPenGlProgram = activeProgramBuildType == kaProgramGL_Compute || activeProgramBuildType == kaProgramGL_Rendering;

    if (kaProgramVK_Rendering == activeProgramBuildType || kaProgramVK_Compute == activeProgramBuildType ||
        kaProgramDX == activeProgramBuildType ||
        isOPenGlProgram)
    {
        pDlg->ILEnabled(false);
        pDlg->LLVM_IREnabled(false);
        pDlg->DebugInfoEnabled(false);

        pDlg->ILCehcked(false);
        pDlg->LLVM_IRCehcked(false);
        pDlg->DebugInfoCehcked(false);

        if (false == isOPenGlProgram)
        {
            pDlg->SourceEnabled(false);
            pDlg->ISAEnabled(false);
            pDlg->SourceCehcked(false);
            pDlg->ISACehcked(true);
        }
    }

    if (disable32BitCheckBox)
    {
        pDlg->Bitness32Enabled(false);
    }

    if (disable64BitCheckbox)
    {
        pDlg->Bitness64Enabled(false);
    }
}

// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::OnExportBinaries()
{
    // Get the position where to place the exported file:
    gtString selectedFolder;
    kaProjectDataManager::FPathToOutFPathsMap buildFilesMap32Bit, buildFilesMap64Bit;
    gtString fileName;

    // This flag is true if we fail in writing to the destination directory.
    bool isFileWritingFailure = false;

    // Get the default folder as the folder of the cl file and the build files:
    QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();
    QTreeWidgetItem* pContextMenuItem = treeSelectedItems.at(0);
    afTreeItemType treeItemType = AF_TREE_ITEM_ITEM_NONE;
    QString BaseFileName;
    QString  exportfileBaseSuffix;
    GT_IF_WITH_ASSERT(pContextMenuItem != nullptr)
    {
        afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pContextMenuItem);
        GT_IF_WITH_ASSERT(pItemData != nullptr)
        {
            treeItemType = pItemData->m_itemType;
            kaProgram* activeProgram = KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram();
            bool isGlProgram = KA_PROJECT_DATA_MGR_INSTANCE.IsActiveProgramGL();

            switch (treeItemType)
            {
                case AF_TREE_ITEM_KA_PROGRAM:
                {

                    GT_IF_WITH_ASSERT(activeProgram != nullptr)
                    {
                        BaseFileName = activeProgram->GetProgramName().asASCIICharArray();
                    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                    KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgramBuildFiles(kaBuildArch32_bit, buildFilesMap32Bit);
#endif
                    KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgramBuildFiles(kaBuildArch64_bit, buildFilesMap64Bit);
                    exportfileBaseSuffix = isGlProgram ? KA_STR_ExportBinariesDialogBaseNameDevicesPart :
                                           KA_PROJECT_DATA_MGR_INSTANCE.IsActiveProgramRender() ? KA_STR_ExportBinariesDialogBaseNameProgramDevicesStagePart : KA_STR_ExportBinariesDialogBaseNameProgramDevicesPart;
                }
                break;

                case AF_TREE_ITEM_KA_PROGRAM_SHADER:
                case AF_TREE_ITEM_KA_PROGRAM_GL_FRAG:
                case AF_TREE_ITEM_KA_PROGRAM_GL_TESC:
                case AF_TREE_ITEM_KA_PROGRAM_GL_TESE:
                case AF_TREE_ITEM_KA_PROGRAM_GL_VERT:
                case AF_TREE_ITEM_KA_PROGRAM_GL_COMP:
                {

                    kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());
                    exportfileBaseSuffix = isGlProgram ? KA_STR_ExportBinariesDialogBaseNameDevicesPart :
                                           KA_PROJECT_DATA_MGR_INSTANCE.IsActiveProgramRender() ? KA_STR_ExportBinariesDialogBaseNameDevicesSatgePart : KA_STR_ExportBinariesDialogBaseNameDevicesPart;

                    if (nullptr != pKAData)
                    {
                        pKAData->filePath().getFileName(fileName);
                        BaseFileName = fileName.asASCIICharArray();
                        // get the devices that have ISA or IL
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                        KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgramBuildFiles(pKAData->filePath(), kaBuildArch32_bit, buildFilesMap32Bit);
#endif
                        KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgramBuildFiles(pKAData->filePath(), kaBuildArch64_bit, buildFilesMap64Bit);
                    }
                }
                break;

                default:
                    break;
            }


        }
    }
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {

        //maps original filename and device to output binary file
        DevicesAndBinFilesMap devicesAndBinFiles32Bit, devicesAndBinFiles64Bit;
        gtSet<gtString> devicesToExport;

        // Export only the files that are binary
        BuildDeviceAndBinFilesMap(buildFilesMap32Bit, devicesAndBinFiles32Bit, devicesToExport);
        BuildDeviceAndBinFilesMap(buildFilesMap64Bit, devicesAndBinFiles64Bit, devicesToExport);

        // exports the binaries:

        QStringList deviceQtList;

        //transform to  devicesToExport set to QtList
        for (const gtString& itr : devicesToExport)
        {
            deviceQtList.push_back(acGTStringToQString(itr));
        }

        kaExportBinariesDialog dlg(nullptr, BaseFileName, exportfileBaseSuffix, deviceQtList,
                                   buildFilesMap32Bit.empty() == false,
                                   buildFilesMap64Bit.empty() == false);
        UpdateIncludeCheckBoxes(&dlg, buildFilesMap32Bit.empty() == true, buildFilesMap64Bit.empty() == true);
        int rc = pApplicationCommands->showModal(&dlg);

        if (rc)
        {

            QStringList fileNames = dlg.selectedFiles();
            selectedFolder = acQStringToGTString(fileNames.first());
            BaseFileName = dlg.GetBaseName();
            fileName = acQStringToGTString(BaseFileName);
            bool bExportSucceeded32 = false,
                 bExportSucceded64 = false;

            if (dlg.Is32BitIncluded())
            {
                ExportBiniriesForSingleSourceFile(dlg, devicesAndBinFiles32Bit,
                                                  fileName, treeItemType, selectedFolder, true, isFileWritingFailure, bExportSucceeded32);
            }

            if (dlg.Is64BitIncluded())
            {
                ExportBiniriesForSingleSourceFile(dlg, devicesAndBinFiles64Bit,
                                                  fileName, treeItemType, selectedFolder, false, isFileWritingFailure, bExportSucceded64);
            }

            if (!(bExportSucceeded32 || bExportSucceded64))
            {
                if (isFileWritingFailure)
                {
                    acMessageBox::instance().critical(afGlobalVariablesManager::ProductNameA(), KA_STR_FileWritePermissionError, QMessageBox::Ok);
                }
                else
                {
                    acMessageBox::instance().critical(afGlobalVariablesManager::ProductNameA(), KA_STR_NoBinaries, QMessageBox::Ok);
                }
            }
            else
            {
                acMessageBox::instance().information(afGlobalVariablesManager::ProductNameA(), KA_STR_BinariesExportedSuccessfully, QMessageBox::Ok);
            }
        }

    }
}

osFilePath GetExportFilePath(const osFilePath& originaFilePath,
                             const gtString& baseFName,
                             const afTreeItemType treeItemType,
                             const gtString& deviceName,
                             const bool is32Bit,
                             const gtString& selectedFolder)
{
    gtString newFileName = baseFName;

    if (AF_TREE_ITEM_KA_PROGRAM == treeItemType && originaFilePath.isEmpty() == false)
    {
        gtString originalFfileName;
        originaFilePath.getFileName(originalFfileName);
        newFileName.append(L"-").append(originalFfileName);

    }

    newFileName.append(L"-");
    newFileName.append(deviceName);
    newFileName.append(is32Bit ? KA_STR_ExportBinariesOutPutFile32BitnessSuffix : KA_STR_ExportBinariesOutPutFile64BitnessSuffix);
    gtString fileExtension(L".bin");
    gtString currentSelectFolder = selectedFolder;
    currentSelectFolder.append(osFilePath::osPathSeparator);
    currentSelectFolder.append(newFileName);
    currentSelectFolder.append(fileExtension);
    osFilePath fileToSave(currentSelectFolder);
    return fileToSave;
}

void ExportBiniriesForSingleSourceFile(const kaExportBinariesDialog& dlg,
                                       const DevicesAndBinFilesMap& devicesAndBinFiles,
                                       const gtString& fileName,
                                       const afTreeItemType treeItemType,
                                       const gtString& selectedFolder,
                                       const bool is32Bit,
                                       bool& isFileWritingFailure,
                                       bool& bExportSucceeded)
{
    //const bool  isActiveProgramPipeLine = KA_PROJECT_DATA_MGR_INSTANCE.IsActiveProgramPipeLine();
    const bool  isActiveProgramGl = KA_PROJECT_DATA_MGR_INSTANCE.IsActiveProgramGL();

    for (const auto& iter : devicesAndBinFiles)
    {
        //for devicesAndBinFiles
        const osFilePath& originaFilePath = iter.first.first;
        const gtString& deviceName = iter.first.second;
        const gtString& binFile = iter.second;


        //just copy binaries

        const kaProgramTypes  buildType = KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram()->GetBuildType();
        const osFilePath toAppendFilePath = isActiveProgramGl ? osFilePath() : originaFilePath;
        gtString outPutSuffixFileName = deviceName;

        if (buildType == kaProgramVK_Compute || buildType == kaProgramVK_Rendering)
        {
            osFilePath(binFile).getFileName(outPutSuffixFileName);
        }

        const osFilePath fileToSave = GetExportFilePath(toAppendFilePath, fileName, treeItemType, outPutSuffixFileName, is32Bit, selectedFolder);
        bool exportToLocation = true;

        //ask user if he wants to overwrites
        if (fileToSave.exists())
        {
            QString msg = KA_STR_BinariesExportOverwriteWarningMessage;
            msg = msg.arg(acGTStringToQString(fileToSave.asString()));
            int userAnswer = acMessageBox::instance().warning(afGlobalVariablesManager::ProductNameA(), msg, QMessageBox::Yes | QMessageBox::No);

            exportToLocation = userAnswer == QMessageBox::Yes;
            //success if at least one file exported
            bExportSucceeded |= exportToLocation;
        }

        if (exportToLocation)
        {
            if (buildType == kaProgramDX || buildType == kaProgramVK_Compute || buildType == kaProgramVK_Rendering)
            {
                bExportSucceeded = osCopyFile(binFile, fileToSave, true);
                isFileWritingFailure = false == bExportSucceeded;
            }
            else
            {
                vector<char> pContentToSave;
                kaBackendManager::instance().getBinary(deviceName, binFile,
                                                       dlg.IsSourceIncluded(),
                                                       dlg.IsILIncluded(),
                                                       dlg.IsDebugInfoIncluded(),
                                                       dlg.IsLLVM_IRIncluded(),
                                                       dlg.IsISAIncluded(),
                                                       pContentToSave);

                if (pContentToSave.size() > 0)
                {

                    osFile binaryFile;
                    int rc = binaryFile.open(fileToSave, osChannel::OS_BINARY_CHANNEL, osFile::OS_OPEN_TO_WRITE);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        qint64 len = pContentToSave.size();
                        bExportSucceeded |= binaryFile.write(&(pContentToSave)[0], len);
                        binaryFile.close();
                    }
                    else
                    {
                        isFileWritingFailure = true;
                        bExportSucceeded = false;
                    }
                }
                else
                {
                    bExportSucceeded &= false;
                }
            }
        }

    }//for
}

// ---------------------------------------------------------------------------
bool kaApplicationTreeHandler::BuildContextMenuForSingleItem(const afApplicationTreeItemData* pItemData, QMenu& menu)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(nullptr != pItemData)
    {
        switch (pItemData->m_itemType)
        {
            case AF_TREE_ITEM_APP_ROOT:
            {
                gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();

                // Add the menu item to Root only if in KA mode:
                if (afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode) && !projectName.isEmpty())
                {
                    // Need to add this menu command before any other commands in the menu and then add a separator
                    // this is done a bit differently then just add an action to the menu:
                    QAction* pSeparator = new QAction(nullptr);
                    pSeparator->setSeparator(true);
                    QList<QAction*> menuActions = menu.actions();
                    QAction* pFirstAction = nullptr;

                    if (menuActions.count() != 0)
                    {
                        pFirstAction = menuActions[0];
                    }

                    menu.insertAction(pFirstAction, pSeparator);
                    menu.insertAction(pSeparator, m_pAddAction);
                    menu.insertAction(m_pAddAction, m_pNewAction);
                    menu.insertAction(m_pNewAction, m_pNewProgramAction);

                    // Enable "Add" and "Create" OpenCL file, only when the build is not in progress:
                    bool shouldEnable = !kaBackendManager::instance().isInBuild();
                    m_pNewProgramAction->setEnabled(shouldEnable);
                    m_pAddAction->setEnabled(shouldEnable);
                    m_pNewAction->setEnabled(shouldEnable);

                    retVal = true;
                }
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
                BuildMenuForShaderFileItem(pItemData, menu);
                retVal = true;
            }
            break;

            case AF_TREE_ITEM_KA_FILE:
            {
                BuildMenuForFileItem(pItemData, menu);
                retVal = true;

            }
            break;

            case AF_TREE_ITEM_KA_KERNEL:
            {
                menu.addAction(m_pGotoSourceAction);
                menu.addSeparator();
                menu.addAction(m_pOpenContainingFolderAction);
                retVal = true;
            }
            break;

            case AF_TREE_ITEM_KA_OUT_DIR:
            {
                menu.addAction(m_pOpenOutputFolderAction);
                retVal = true;
            }
            break;

            case AF_TREE_ITEM_KA_DEVICE:
            {
                menu.addAction(m_pExportToCSVAction);
                retVal = true;
            }
            break;

            case AF_TREE_ITEM_KA_PROGRAM:
            {
                // Build the program item menu
                BuildMenuForProgramItem(pItemData, menu);
                retVal = true;
            }
            break;

            default:
                // all other tree nodes do not have context menu:
                break;
        }
    }

    return retVal;
}

void kaApplicationTreeHandler::BuildMenuForFileItem(const afApplicationTreeItemData* pItemData, QMenu& menu)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pItemData != nullptr) && (m_pOpenAction != nullptr) && (m_pRenameFileAction != nullptr) &&
                      (m_pExportAction != nullptr) && (m_pRemoveAction != nullptr) && (m_pOpenContainingFolderAction != nullptr))
    {
        // Get the KA item data:
        bool isInBuild = kaBackendManager::instance().isInBuild();
        bool isItemFilePathSet = false;
        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pKAData != nullptr)
        {
            gtString fileName;
            gtString fileExtension;
            pKAData->filePath().getFileNameAndExtension(fileName);
            pKAData->filePath().getFileExtension(fileExtension);
            isItemFilePathSet = !pKAData->filePath().isEmpty();

            // Set the string and font for the "Open" action:
            QFont font = m_pOpenAction->font();
            font.setBold(true);
            m_pOpenAction->setFont(font);
            QString openItemStr = KA_STR_openclOpenFile;
            openItemStr.append(" ");
            openItemStr.append(acGTStringToQString(fileName));
            m_pOpenAction->setText(openItemStr);
        }

        // Build the menu structure
        if (isItemFilePathSet)
        {
            m_pRemoveAction->setText(KA_STR_removeFromProjectASCII);
            menu.addAction(m_pOpenAction);
            menu.addSeparator();
            menu.addAction(m_pRenameFileAction);
            menu.addSeparator();
            menu.addAction(m_pRemoveAction);
            m_pRemoveAction->setEnabled(!isInBuild);
            m_pRenameFileAction->setEnabled(!isInBuild);
            m_pRenameFileAction->setText(KA_STR_renameFileStatusbarStringASCII);
            menu.addAction(m_pOpenContainingFolderAction);
        }
    }
}

void kaApplicationTreeHandler::BuildMenuForShaderFileItem(const afApplicationTreeItemData* pItemData, QMenu& menu)
{
    bool isInBuild = kaBackendManager::instance().isInBuild();

    // Sanity check:
    GT_IF_WITH_ASSERT(pItemData != nullptr            &&
                      m_pOpenAction != nullptr        &&
                      m_pRenameFileAction != nullptr  &&
                      m_pExportAction != nullptr      &&
                      m_pRemoveAction != nullptr      &&
                      m_pOpenContainingFolderAction != nullptr)
    {
        bool isItemFileSet = false;
        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pKAData != nullptr)
        {
            isItemFileSet = !pKAData->filePath().isEmpty();

            if (isItemFileSet)
            {
                gtString fileName;
                gtString fileExtension;
                pKAData->filePath().getFileNameAndExtension(fileName);
                pKAData->filePath().getFileExtension(fileExtension);


                // Set the string and font for the "Open" action:
                QFont font = m_pOpenAction->font();
                font.setBold(true);
                m_pOpenAction->setFont(font);
                QString openItemStr = KA_STR_openclOpenFile;
                openItemStr.append(" ");
                openItemStr.append(acGTStringToQString(fileName));
                m_pOpenAction->setText(openItemStr);

                // Add the "Build" action:
                QString buildMenuStr = GetBuildCommandString();
                m_pBuildAction->setText(buildMenuStr);
                m_pBuildAction->setEnabled(!isInBuild);

                // Add the "Cancel Build" action:
                QString cancelBuildMenuStr = KA_STR_CancelBuildASCII;
                m_pCancelBuildAction->setText(cancelBuildMenuStr);
                m_pCancelBuildAction->setEnabled(isInBuild);


                m_pExportAction->setEnabled(false);

                // if extension is not cl - don't enable export
                kaProgram* activeProgramm = KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram();
                kaSourceFile* pDataFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(pKAData->filePath());

                GT_IF_WITH_ASSERT(activeProgramm != nullptr)
                {
                    if (pDataFile != nullptr)
                    {
                        if (
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                            KA_PROJECT_DATA_MGR_INSTANCE.IsBuilt(activeProgramm, pDataFile->id(), kaBuildArch32_bit) ||
#endif
                            KA_PROJECT_DATA_MGR_INSTANCE.IsBuilt(activeProgramm, pDataFile->id(), kaBuildArch64_bit))
                        {
                            m_pExportAction->setEnabled(true);
                        }
                    }
                }

                m_pRemoveAction->setText(KA_STR_removeShaderFromProgramASCII);
                m_pRemoveAction->setEnabled(!isInBuild);
                m_pRenameFileAction->setEnabled(!isInBuild);
                m_pRenameFileAction->setText(KA_STR_renameFileStatusbarStringASCII);
                // Build the menu
                menu.addAction(m_pOpenAction);
                menu.addSeparator();
                menu.addAction(m_pBuildAction);
                menu.addAction(m_pCancelBuildAction);
                menu.addAction(m_pRenameFileAction);
                menu.addAction(m_pRemoveAction);

                if (m_pExportAction->isEnabled())
                {
                    menu.addSeparator();
                    menu.addAction(m_pExportAction);
                }

                menu.addSeparator();
                menu.addAction(m_pOpenContainingFolderAction);
            }

            else
            {
                // Add "Add" and "Create" shader actions
                QFont font = m_pOpenAction->font();
                font.setBold(true);
                m_pCreateShaderAction->setFont(font);
                menu.addAction(m_pCreateShaderAction);
                gtString shaderName;
                kaUtils::TreeItemTypeToPipeLineStageString(pItemData->m_itemType, shaderName);
                QString attachShaderStr = QString(KA_STR_attachStageShaderToASCII).arg(acGTStringToQString(shaderName));
                m_pAddShaderAction->setText(attachShaderStr);
                menu.addAction(m_pAddShaderAction);
            }
        }
    }
}

void kaApplicationTreeHandler::BuildMenuForProgramItem(const afApplicationTreeItemData* pItemData, QMenu& menu)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pItemData != nullptr) && (m_pOpenAction != nullptr) &&
                      (m_pRenameFileAction != nullptr) && (m_pExportAction != nullptr) &&
                      (m_pRemoveAction != nullptr) && (m_pOpenContainingFolderAction != nullptr))
    {
        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pKAData != nullptr)
        {
            kaProgram* pProgram = pKAData->GetProgram();
            // Enable "Add" and "Create" OpenCL file, only when the build is not in progress:
            bool shouldEnable = !kaBackendManager::instance().isInBuild();
            m_pAddAction->setEnabled(shouldEnable);
            m_pNewAction->setEnabled(shouldEnable);
            m_pBuildAction->setEnabled(shouldEnable);
            m_pRenameFileAction->setEnabled(shouldEnable);
            m_pRenameFileAction->setText(KA_STR_renameProgramStatusbarStringASCII);
            QString removeProgramMenuStr = QString(KA_STR_removeProgramFromProjectASCII).arg(acGTStringToQString(pKAData->GetProgram()->GetProgramName()));
            m_pRemoveAction->setText(removeProgramMenuStr);

            GT_IF_WITH_ASSERT(pProgram != nullptr)
            {
                if (pProgram->HasFile())
                {
                    QString buildMenuStr = GetBuildCommandString(pKAData, pItemData->m_itemType);
                    m_pBuildAction->setText(buildMenuStr);
                    menu.addAction(m_pBuildAction);
                    menu.addSeparator();
                }

                menu.addAction(m_pRenameFileAction);
                menu.addAction(m_pRemoveAction);
                kaProgramTypes programType = pProgram->GetBuildType();

                if (programType == kaProgramCL || programType == kaProgramDX)
                {
                    //for CL and DX folders add menu items for source file attachment ( new or existing)
                    menu.addAction(m_pNewAction);
                    menu.addAction(m_pAddAction);
                }
              
                if (
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                    KA_PROJECT_DATA_MGR_INSTANCE.IsBuilt(pProgram, kaBuildArch32_bit) ||
#endif
                    KA_PROJECT_DATA_MGR_INSTANCE.IsBuilt(pProgram, kaBuildArch64_bit))
                {


                    bool isExportAvailable = ((programType != kaProgramVK_Compute) && (programType != kaProgramVK_Rendering));
                    // Currently exporting for Vulkan is not supported
                    if (isExportAvailable)
                    {
                        m_pExportAction->setEnabled(isExportAvailable);
                        menu.addSeparator();
                        menu.addAction(m_pExportAction);
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
bool kaApplicationTreeHandler::ExecuteDropEvent(QDropEvent* pEvent, const QString& dragDropFile)
{
    GT_UNREFERENCED_PARAMETER(dragDropFile);

    bool retVal = false;

    // Check if there is a project loaded:
    bool isProjectLoaded = !afProjectManager::instance().currentProjectFilePath().isEmpty();

    if (!isProjectLoaded)
    {
        // Create a new default project:
        afApplicationCommands::instance()->CreateDefaultProject(KA_STR_executionMode);
    }

    // Check if a process is running:

    if (pEvent != nullptr)
    {
        const QMimeData* pMimeData = pEvent->mimeData();

        GT_IF_WITH_ASSERT(pMimeData != nullptr)
        {
            if (pMimeData->hasUrls())
            {
                // Dropped element is a file item dragged from the outside of the application.
                // Add the file to the tree
                QList<QUrl> urlList = pMimeData->urls();

                if (urlList.size() >= 1)
                {
                    retVal = true;

                    for (int i = 0; i < urlList.size() && i < 32; ++i)
                    {
                        QString currentFile = urlList.at(i).toLocalFile();
                        osFilePath clFilePath(acQStringToGTString(currentFile));

                        // Add the file to the tree:
                        kaSourceFile* pCurrentFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(clFilePath);

                        if (pCurrentFile == nullptr)
                        {
                            kaApplicationCommands::instance().AddSourceFile(clFilePath);
                        }

                    }
                }

                // Save the project (so that the file is saved in this project):
                afApplicationCommands::instance()->OnFileSaveProject();

                if (!isProjectLoaded)
                {
                    afApplicationCommands::instance()->OnFileOpenProject(afProjectManager::instance().currentProjectFilePath().asString());
                }

                DropOutertemsOnRelevantProgram(pMimeData, pEvent);
            }
            else if (pMimeData->hasFormat("text/plain"))
            {
                // Dropped element is dragged from the tree, it should be a file element, and dropped into a program child
                ExecuteDropForDraggedTreeItem(pMimeData, pEvent);
            }
        }
    }

    return retVal;
}


void kaApplicationTreeHandler::DropOutertemsOnRelevantProgram(const QMimeData* pMimeData, QDropEvent* pEvent)
{
    afApplicationTreeItemData* pProgramItemData = nullptr;
    kaTreeDataExtension* pProgramKAItemData = nullptr;
    kaProgram* pProgram = nullptr;
    GT_IF_WITH_ASSERT((pMimeData != nullptr) && (pEvent != nullptr) && (m_pApplicationTree != nullptr))
    {
        gtVector<osFilePath> addedFilePaths;

        foreach (QUrl url, pMimeData->urls())
        {
            // Get the current file path:
            gtString fileUrl = acQStringToGTString(url.toLocalFile());
            osFilePath droppedFilePath(fileUrl);
            addedFilePaths.push_back(droppedFilePath);
        }

        QTreeWidgetItem* pDropDestinationItem = m_pApplicationTree->treeControl()->itemAt(pEvent->pos());

        if (pDropDestinationItem != nullptr)
        {
            if (m_pLastHintItem == pDropDestinationItem && m_pLastHintItem != nullptr)
            {
                m_pLastHintItem->setBackgroundColor(0, STAGE_ORIG_BGCOLOR);
            }
            afApplicationTreeItemData* pDropDestinationItemData = m_pApplicationTree->getTreeItemData(pDropDestinationItem);

            if (pDropDestinationItemData != nullptr)
            {
                afTreeItemType destinationItemType = pDropDestinationItemData->m_itemType;

                if (destinationItemType == AF_TREE_ITEM_KA_PROGRAM)
                {
                    pProgramItemData = pDropDestinationItemData;
                    pProgramKAItemData = qobject_cast<kaTreeDataExtension*>(pProgramItemData->extendedItemData());

                    if (pProgramKAItemData != nullptr)
                    {
                        pProgram = pProgramKAItemData->GetProgram();
                    }

                    pDropDestinationItem->setExpanded(true);
                }
                else if (((destinationItemType >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) &&
                          (destinationItemType <= AF_TREE_ITEM_KA_LAST_FILE_ITEM_TYPE)) ||
                         (destinationItemType == AF_TREE_ITEM_KA_ADD_FILE) ||
                         (destinationItemType == AF_TREE_ITEM_KA_NEW_FILE) ||
                         (destinationItemType == AF_TREE_ITEM_KA_REF_TYPE) ||
                         (destinationItemType == AF_TREE_ITEM_KA_DEVICE) ||
                         (destinationItemType == AF_TREE_ITEM_KA_OUT_DIR))
                {
                    QTreeWidgetItem* pParent = nullptr;
                    afApplicationTreeItemData* pParentData = nullptr;

                    while ((pParent = pDropDestinationItem->parent()) != m_pProgramsRootNode)
                    {
                        pParentData = m_pApplicationTree->getTreeItemData(pParent);

                        if (pParentData != nullptr)
                        {
                            if (pParentData->m_itemType == AF_TREE_ITEM_KA_PROGRAM)
                            {
                                pProgramItemData = pParentData;
                                kaTreeDataExtension* pProgramKAItemData = qobject_cast<kaTreeDataExtension*>(pParentData->extendedItemData());
                                // obtain kaProgram* from extended data
                                pProgram = pProgramKAItemData->GetProgram();
                                // if drop target was rendering program stage shader placeholder its type should be preserved
                                if (!(destinationItemType >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) &&
                                    (destinationItemType <= AF_TREE_ITEM_KA_LAST_FILE_ITEM_TYPE))
                                {
                                    destinationItemType = AF_TREE_ITEM_KA_PROGRAM;
                                }
                                break;
                            }
                        }
                        pDropDestinationItem = pParent;
                    }
                }

                if (pProgram != nullptr)
                {
                    kaProgramTypes programType = pProgram->GetBuildType();
                    // Check if the destination is a program stage. If not, item type should be calculated according to the parent program

                    bool isProgramStage = (destinationItemType >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) && (destinationItemType <= AF_TREE_ITEM_KA_PROGRAM_GL_VERT);

                    if (!isProgramStage)
                    {
                        if (AF_TREE_ITEM_KA_PROGRAM == destinationItemType)
                        {
                            if (kaProgramTypes::kaProgramGL_Compute == programType || kaProgramVK_Compute == programType)
                            {
                                destinationItemType = AF_TREE_ITEM_KA_PROGRAM_GL_COMP;
                            }
                            else if (kaProgramDX == programType || kaProgramCL == programType)
                            {
                                destinationItemType = AF_TREE_ITEM_KA_PROGRAM_SHADER;

                            }
                        }
                    }

                    if (isProgramStage || (!isProgramStage && (kaProgramDX == programType || kaProgramCL == programType)))
                    {
                        for (const osFilePath& it : addedFilePaths)
                        {
                            // Add the file node to the program branch
                            if (!pProgram->HasFile(it, AF_TREE_ITEM_ITEM_NONE))
                            {
                                AddFileNodeToProgramBranch(it, pProgramItemData, destinationItemType);
                                if (!IsAddingMultipleFilesToProgramBranchAllowed(pProgramItemData))
                                {
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void kaApplicationTreeHandler::ExecuteDropForDraggedTreeItem(const QMimeData* pMimeData, QDropEvent* pEvent)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pMimeData != nullptr) && (pEvent != nullptr) && (m_pApplicationTree != nullptr))
    {
        gtVector<osFilePath> addedFilePaths;

        if (pMimeData->hasUrls())
        {

            QString draggedItemText = pMimeData->text();

            foreach (QUrl url, pMimeData->urls())
            {
                // Get the current file path:
                gtString fileUrl = acQStringToGTString(url.toLocalFile());
                osFilePath droppedFilePath(fileUrl);
                addedFilePaths.push_back(droppedFilePath);
            }
        }

        // Dropped element is a tree elements dragged from the application tree
        // The dragged element (which is expected to be a source file)
        // should be set as the pipeline stage for the dropped target (which is expected to be a pipeline stage)
        QTreeWidgetItem* pDroppedItem = m_pApplicationTree->treeControl()->itemAt(pEvent->pos());

        if (pDroppedItem != nullptr)
        {
            if (m_pLastHintItem == pDroppedItem && m_pLastHintItem != nullptr)
            {
                m_pLastHintItem->setBackgroundColor(0, STAGE_ORIG_BGCOLOR);
            }
            GT_IF_WITH_ASSERT(IsItemDroppable(pDroppedItem))
            {
                afApplicationTreeItemData* pDroppedOnItemData = m_pApplicationTree->getTreeItemData(pDroppedItem);

                if (pDroppedOnItemData != nullptr)
                {
                    afApplicationTreeItemData* pDraggedItemData = nullptr;
                    kaTreeDataExtension* pDraggedItemKAData = nullptr;

                    QTreeWidgetItem* pDraggedItem = m_pApplicationTree->treeControl()->DraggedItem();

                    if (pDraggedItem != nullptr)
                    {
                        pDraggedItemData = m_pApplicationTree->getTreeItemData(pDraggedItem);

                        if (pDraggedItemData != nullptr)
                        {
                            pDraggedItemKAData = qobject_cast<kaTreeDataExtension*>(pDraggedItemData->extendedItemData());

                            if (pDraggedItemKAData != nullptr)
                            {
                                if (!pDraggedItemKAData->filePath().isEmpty())
                                {
                                    addedFilePaths.push_back(pDraggedItemKAData->filePath());
                                }
                            }
                        }
                    }

                    // Find the parent program for the dropped item
                    const afApplicationTreeItemData* pProgramItemData = FindParentItemDataOfType(pDroppedOnItemData, AF_TREE_ITEM_KA_PROGRAM);
                    GT_IF_WITH_ASSERT(pProgramItemData != nullptr)
                    {
                        kaTreeDataExtension* pDroppedKAData = qobject_cast<kaTreeDataExtension*>(pProgramItemData->extendedItemData());
                        GT_IF_WITH_ASSERT(pDroppedKAData != nullptr)
                        {
                            // Check if the dropped item is a program stage. If not, item type should be calculated according to the parent program
                            afTreeItemType droppedItemType = pDroppedOnItemData->m_itemType;
                            bool isProgramStage = (droppedItemType >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) && (droppedItemType <= AF_TREE_ITEM_KA_PROGRAM_GL_VERT);

                            if (!isProgramStage)
                            {
                                //
                                droppedItemType = AF_TREE_ITEM_KA_PROGRAM_GL_COMP;

                                if ((pDroppedKAData->GetProgram()->GetBuildType() == kaProgramCL) || (pDroppedKAData->GetProgram()->GetBuildType() == kaProgramDX))
                                {
                                    droppedItemType = AF_TREE_ITEM_KA_PROGRAM_SHADER;
                                }
                            }

                            for (const osFilePath& it : addedFilePaths)
                            {
                                // Add the file node to the program branch
                                AddFileNodeToProgramBranch(it, pProgramItemData, droppedItemType);
                            }
                        }
                    }
                }

                pEvent->setDropAction(Qt::MoveAction);
                pEvent->accept();
            }
        }
    }
}

// ---------------------------------------------------------------------------
bool kaApplicationTreeHandler::IsDragDropSupported(QDropEvent* pEvent, QString& dragDropFile, bool& shouldAccpet)
{
    GT_UNREFERENCED_PARAMETER(pEvent);
    GT_UNREFERENCED_PARAMETER(dragDropFile);
    GT_UNREFERENCED_PARAMETER(shouldAccpet);

    bool shouldHandle = afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode);

    bool retVal = false;

    if (pEvent != nullptr && shouldHandle)
    {
        const QMimeData* mimeData = pEvent->mimeData();

        if (mimeData->hasUrls())
        {
            QList<QUrl> urlList = mimeData->urls();

            if (urlList.size() >= 1)
            {
                bool typeSupported = true;

                for (int i = 0; i < urlList.size() && i < 32; ++i)
                {
                    QString importFilePath = urlList.at(i).toLocalFile();
                    QFileInfo fileInfo(importFilePath);

                    if (fileInfo.exists())
                    {
                        gtString fileSuffix = acQStringToGTString(fileInfo.suffix());

                        foreach (gtString unsupportedFileType, m_unsupportedFileTypes)
                        {
                            if (fileSuffix == unsupportedFileType)
                            {
                                typeSupported = false;
                                break;
                            }
                        }
                    }
                }


                if (typeSupported)
                {
                    retVal = true;

                    for (int i = 0; i < urlList.size() && i < 32; ++i)
                    {
                        QString currentFile = urlList.at(i).toLocalFile();
                        QFileInfo fileInfo(currentFile);

                        if (fileInfo.exists())
                        {
                            retVal = retVal && true;
                            shouldAccpet = retVal;
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::OnApplicationTreeCreated()
{
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {
        m_pApplicationTree = pApplicationCommands->applicationTree();
        GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr)
        {
            // Should add a signal to main window that the tree was created, and register all tree handler when handling this signal
            m_pApplicationTree->registerApplicationTreeHandler(this);

            // Connect an editor started slot:
            bool rc = connect(m_pApplicationTree, SIGNAL(EditorStarted(QLineEdit*)), this, SLOT(OnEditorStarted(QLineEdit*)));
            GT_ASSERT(rc);

            // Connect an editor closed slot:
            rc = connect(m_pApplicationTree->treeControl(), SIGNAL(onItemCloseEditor(const QString&)), this, SLOT(OnEditorClosed(const QString&)));
            GT_ASSERT(rc);

            // Connect an editor closed slot:
            rc = connect(m_pApplicationTree->treeControl(), SIGNAL(itemSelectionChanged()), this, SLOT(OnTreeItemSelectionChanged()));
            GT_ASSERT(rc);


            // Connect changing selection to program node with setting active program
            rc = connect(m_pApplicationTree->treeControl(), SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
                         this, SLOT(OnTreeCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
            GT_ASSERT(rc);

            // Connect drag move event
            rc = connect(m_pApplicationTree->treeControl(), SIGNAL(TreeElementDragMoveEvent(QDragMoveEvent*)), this, SLOT(OnDragMoveEvent(QDragMoveEvent*)));
            GT_ASSERT(rc);

            // nodes:
            rc = AddProgramCreateTreeItem();
            GT_ASSERT(rc);

         
            // The actions with shortcuts needs to be added to the tree as actions (otherwise the shortcut is not found):
            m_pApplicationTree->treeControl()->addAction(m_pBuildAction);
            m_pApplicationTree->treeControl()->addAction(m_pCancelBuildAction);
            m_pApplicationTree->treeControl()->addAction(m_pRemoveAction);
            m_pApplicationTree->treeControl()->addAction(m_pRenameFileAction);
            m_pApplicationTree->setAcceptDrops(false);
        }
    }
}

// ---------------------------------------------------------------------------
bool kaApplicationTreeHandler::BuildItemHTMLPropeties(const afApplicationTreeItemData& displayedItemId, afHTMLContent& htmlContent)
{
    bool retVal = false;

    kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(displayedItemId.extendedItemData());

    if (pKAData != nullptr)
    {
        retVal = true;

        switch (displayedItemId.m_itemType)
        {
            case AF_TREE_ITEM_KA_FILE:
            case AF_TREE_ITEM_KA_OVERVIEW:
            case AF_TREE_ITEM_KA_SOURCE:
            {
                kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();
                GT_IF_WITH_ASSERT(nullptr != pTreeHandler)
                {
                    retVal = pTreeHandler->getOverviewHtmlInfo(pKAData->filePath(), htmlContent);
                }
            }
            break;

            case AF_TREE_ITEM_KA_KERNEL:
            case AF_TREE_ITEM_KA_STATISTICS:
            case AF_TREE_ITEM_KA_ANALYSIS:
            case AF_TREE_ITEM_KA_DEVICE:
            case AF_TREE_ITEM_KA_ADD_FILE:
            case AF_TREE_ITEM_KA_NEW_FILE:
            case AF_TREE_ITEM_KA_NEW_PROGRAM:
            case AF_TREE_ITEM_KA_PROGRAM:
            case AF_TREE_ITEM_KA_PROGRAM_GL_GEOM:
            case AF_TREE_ITEM_KA_PROGRAM_GL_FRAG:
            case AF_TREE_ITEM_KA_PROGRAM_GL_TESC:
            case AF_TREE_ITEM_KA_PROGRAM_GL_TESE:
            case AF_TREE_ITEM_KA_PROGRAM_GL_VERT:
            case AF_TREE_ITEM_KA_PROGRAM_GL_COMP:
            case AF_TREE_ITEM_KA_PROGRAM_SHADER:
            case AF_TREE_ITEM_KA_OUT_DIR:
            case AF_TREE_ITEM_KA_REF_TYPE:
                break;

            //                    AF_TREE_ITEM_KA_HISTORY,
            //                    AF_TREE_ITEM_KA_EXE_FILE,
            default:
                GT_ASSERT(false);
                break;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::OnEditorStarted(QLineEdit* pLineEditor)
{
    QString fileName = pLineEditor->text();
    GT_IF_WITH_ASSERT(!fileName.isEmpty())
    {
        //check the item type
        afApplicationTreeItemData* pSelectedItemData = GetSelectedItemData();

        if (pSelectedItemData != nullptr)
        {
            if ((pSelectedItemData->m_itemType >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) &&
                (pSelectedItemData->m_itemType <= AF_TREE_ITEM_KA_LAST_FILE_ITEM_TYPE))
            {
                //editing stage shader name
                StartStageShaderNameEditing(pLineEditor);
            }
        }
        else
        {
            int dotPos = fileName.indexOf(".");

            if (dotPos != -1)
            {
                pLineEditor->setSelection(0, dotPos);
            }
        }
    }
}


// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::OnEditorClosed(const QString& editedText)
{
    bool shouldHandle = afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode);

    if (shouldHandle)
    {
        GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr && m_pApplicationTree->treeControl() != nullptr)
        {
            // Find the selected (renamed) item data and call the right rename function according to the item type
            afApplicationTreeItemData* pRenamedItemData = GetSelectedItemData();

            if (pRenamedItemData != nullptr)
            {
                bool isFileRef = (pRenamedItemData->m_itemType >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) && (pRenamedItemData->m_itemType <= AF_TREE_ITEM_KA_LAST_FILE_ITEM_TYPE);
                bool isFile = (pRenamedItemData->m_itemType == AF_TREE_ITEM_KA_FILE);
                bool isProgram = (pRenamedItemData->m_itemType == AF_TREE_ITEM_KA_PROGRAM);

                if (isFile || isFileRef)
                {
                    RenameFile(pRenamedItemData, editedText);
                }
                else if (isProgram)
                {
                    RenameProgram(pRenamedItemData, editedText);
                }
            }
        }
    }
}

void kaApplicationTreeHandler::OnTreeItemSelectionChanged()
{
    QString buildString = GetBuildCommandString();

    if (!buildString.isEmpty())
    {
        buildString.append(" (");
        buildString.append(AF_STR_BuildShortcut);
        buildString.append(")");
        buildString.remove("&");
        afExecutionModeManager::instance().SetActionTooltip(AF_EXECUTION_ID_BUILD, buildString);
    }
}

void kaApplicationTreeHandler::RenameFile(afApplicationTreeItemData* pRenamedItemData, const QString& newFileName)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pRenamedItemData != nullptr)
    {
        QString restoredNewFileName = newFileName;
        StageNodeNameFromFileName(restoredNewFileName, m_fileNameBeforeEdit);

        // Check if the name was changed:
        bool wasNameChanged = (restoredNewFileName != m_fileNameBeforeEdit);
        QTreeWidgetItem* pItem = pRenamedItemData->m_pTreeWidgetItem;

        if (wasNameChanged && pItem != nullptr)
        {
            // Disable editing of the kernel node:
            pItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            QString errorMessage;
            QString newNameAfterRevision = newFileName;
            GT_IF_WITH_ASSERT(pRenamedItemData != nullptr)
            {
                kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pRenamedItemData->extendedItemData());
                GT_IF_WITH_ASSERT(pKAData != nullptr)
                {

                    // Check if this file name is valid:
                    if (IsNewFileNameValid(newFileName, errorMessage))
                    {
                        // Rename the source file according to the user request
                        RenameSourceFile(pRenamedItemData, pKAData->filePath(), newFileName);
                    }
                    else
                    {
                        acMessageBox::instance().critical(afGlobalVariablesManager::ProductNameA(), errorMessage);
                        newNameAfterRevision = m_fileNameBeforeEdit;
                        // Set the previous name:
                        pItem->setText(0, newNameAfterRevision);
                        // If the rename was rejected, force retry:
                        pItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
                        afApplicationCommands::instance()->applicationTree()->editCurrentItem();
                    }
                }
            }
        }

        if (!wasNameChanged)
        {
            //name was not change - restore stage name brackets
            if ((pRenamedItemData->m_itemType >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) &&
                (pRenamedItemData->m_itemType <= AF_TREE_ITEM_KA_LAST_FILE_ITEM_TYPE))
            {
                // Set the previous name:
                if ((pItem != nullptr) && (pItem == m_pRenamedItem))
                {
                    pItem->setText(0, m_fileNameBeforeEdit);
                }
            }
        }


        m_pCreatedSourceItemData = nullptr;
        m_pRenamedItem = nullptr;
    }
}

void kaApplicationTreeHandler::RenameProgram(afApplicationTreeItemData* pRenamedItemData, const QString& newProgramName)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pRenamedItemData != nullptr)
    {
        // Check if the name was changed:
        bool wasNameChanged = (newProgramName != m_fileNameBeforeEdit);
        QTreeWidgetItem* pItem = pRenamedItemData->m_pTreeWidgetItem;

        if (wasNameChanged && pItem != nullptr)
        {
            // Disable editing of the kernel node:
            pItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pRenamedItemData->extendedItemData());
            GT_IF_WITH_ASSERT(pKAData != nullptr)
            {

                // Check if this file name is valid:
                QString errorMessage;

                if (IsProgramNameValid(newProgramName, errorMessage))
                {
                    // Rename the program according to the user request
                    RenameProgram(pRenamedItemData, pKAData->GetProgram()->GetProgramName(), acQStringToGTString(newProgramName));
                }
                else
                {
                    // Output an invalid name message box
                    acMessageBox::instance().critical(afGlobalVariablesManager::ProductNameA(), errorMessage);

                    // Set the previous name:
                    pItem->setText(0, m_fileNameBeforeEdit);

                    // If the rename was rejected, force retry:
                    pItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
                    afApplicationCommands::instance()->applicationTree()->editCurrentItem();
                }
            }
        }
    }
}

void kaApplicationTreeHandler::RenameProgram(afApplicationTreeItemData* pItemData, const gtString& oldProgramName, const gtString& newProgramName)
{
    GT_UNREFERENCED_PARAMETER(oldProgramName);
    // Sanity check:
    GT_IF_WITH_ASSERT(pItemData != nullptr)
    {
        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT((pKAData != nullptr) && (pKAData->GetProgram() != nullptr))
        {
            // Set the program item name
            pKAData->GetProgram()->SetProgramName(newProgramName);

            // Get the output directories, and check if one of them is not empty
            osDirectory outputDir32, outputDir64;
            pKAData->GetProgram()->GetAndCreateOutputDirectories(outputDir32, outputDir64, false, false);
            bool doesBuildArtifactsExist = !outputDir64.IsEmpty() || !outputDir32.IsEmpty();

            if (doesBuildArtifactsExist)
            {
                // Close all files related to this program
                gtVector<osFilePath> listOfOpenedWindows;
                afApplicationCommands::instance()->GetListOfOpenedWindowsForFile(outputDir32.directoryPath().asString(), listOfOpenedWindows);

                for (auto it : listOfOpenedWindows)
                {
                    afApplicationCommands::instance()->closeFile(it);
                }

                afApplicationCommands::instance()->GetListOfOpenedWindowsForFile(outputDir64.directoryPath().asString(), listOfOpenedWindows);

                for (auto it : listOfOpenedWindows)
                {
                    afApplicationCommands::instance()->closeFile(it);
                }
            }

            // Remove the build artifacts from disk
            outputDir32.deleteRecursively();
            outputDir64.deleteRecursively();

            // Remove the output nodes from the program tree
            GT_IF_WITH_ASSERT(pItemData->m_pTreeWidgetItem != nullptr)
            {
                for (int i = 0; i < pItemData->m_pTreeWidgetItem->childCount(); i++)
                {
                    QTreeWidgetItem* pChild = pItemData->m_pTreeWidgetItem->child(i);

                    if (pChild != nullptr)
                    {
                        afApplicationTreeItemData* pChildData = m_pApplicationTree->getTreeItemData(pChild);

                        if ((pChildData != nullptr) && (pChildData->m_itemType == AF_TREE_ITEM_KA_OUT_DIR))
                        {
                            m_pApplicationTree->removeTreeItem(pChild, false);
                        }
                    }
                }
            }
        }
    }
}


afTreeItemType kaApplicationTreeHandler::GetSelectedItemType()
{
    afTreeItemType retVal = AF_TREE_ITEM_ITEM_NONE;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pApplicationTree != nullptr) && (m_pApplicationTree->treeControl() != nullptr))
    {
        // Get the item data list of the currently clicked items:
        gtList<const afApplicationTreeItemData*> listOfSelectedItemDatas;

        foreach (QTreeWidgetItem* pItem, m_pApplicationTree->treeControl()->selectedItems())
        {
            const afApplicationTreeItemData* pClickedItemData = m_pApplicationTree->getTreeItemData(pItem);

            if (pClickedItemData != nullptr)
            {
                listOfSelectedItemDatas.push_back(pClickedItemData);
            }
        }


        auto iter = listOfSelectedItemDatas.begin();
        auto iterEnd = listOfSelectedItemDatas.end();

        // Check the type of the first item. We support only multiple selection of files, or multiple selection of stages
        for (; iter != iterEnd; iter++)
        {
            const afApplicationTreeItemData* pItemData = *iter;

            if (pItemData != nullptr)
            {
                bool isSelectedFileRef = (pItemData->m_itemType >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) && (pItemData->m_itemType <= AF_TREE_ITEM_KA_LAST_FILE_ITEM_TYPE);
                bool isRetValFileRef = (retVal >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) && (retVal <= AF_TREE_ITEM_KA_LAST_FILE_ITEM_TYPE);
                isRetValFileRef = isRetValFileRef || (retVal == AF_TREE_ITEM_ITEM_NONE);

                bool isSelectedProgram = (pItemData->m_itemType == AF_TREE_ITEM_KA_PROGRAM);
                bool isRetValProgram = (retVal == AF_TREE_ITEM_KA_PROGRAM) || (retVal == AF_TREE_ITEM_ITEM_NONE);

                bool isSelectedFile = (pItemData->m_itemType == AF_TREE_ITEM_KA_FILE);
                bool isRetValFile = (retVal == AF_TREE_ITEM_KA_FILE) || (retVal == AF_TREE_ITEM_ITEM_NONE);

                if (isSelectedFileRef && isRetValFileRef)
                {
                    // All selected items until now are file refs
                    retVal = AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE;
                }
                else if (isSelectedProgram && isRetValProgram)
                {
                    // All selected items until now are programs
                    retVal = AF_TREE_ITEM_KA_PROGRAM;
                }
                else if (isSelectedFile && isRetValFile)
                {
                    // All selected items until now are files
                    retVal = AF_TREE_ITEM_KA_FILE;
                }
                else
                {
                    // Different item types
                    retVal = AF_TREE_ITEM_ITEM_NONE;
                    break;
                }
            }
        }
    }

    return retVal;
}

kaProgram* kaApplicationTreeHandler::GetActiveProgram() const
{
    kaProgram* pRetVal = nullptr;

    const afApplicationTreeItemData* pProgramItemData = GetActiveProgramApplicaionTreeData();

    if (pProgramItemData != nullptr)
    {
        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pProgramItemData->extendedItemData());

        if (pKAData != nullptr)
        {
            pRetVal = pKAData->GetProgram();
        }
    }

    return pRetVal;
}
const afApplicationTreeItemData*  kaApplicationTreeHandler::GetActiveProgramApplicaionTreeData() const
{
    const afApplicationTreeItemData* pProgramItemData = nullptr;

    // This function can be called when the toolbar is created. Sometimes, m_pApplicationTree is not initialized.
    // Do not assert at this point, this is a false assertion.
    if ((m_pApplicationTree != nullptr) && (m_pApplicationTree->treeControl() != nullptr))
    {
        if (m_pApplicationTree->treeControl()->selectedItems().size() > 0)
        {
            afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(m_pApplicationTree->treeControl()->selectedItems().at(0));
            GT_IF_WITH_ASSERT(pItemData != nullptr)
            {
                pProgramItemData = FindParentItemDataOfType(pItemData, AF_TREE_ITEM_KA_PROGRAM);
            }
        }
    }

    return pProgramItemData;
}
// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::SetNewFilePath(QTreeWidgetItem* treeItem, const osFilePath& filePath)
{
    afApplicationTreeItemData* pTreeItemData = m_pApplicationTree->getTreeItemData(treeItem);

    if (nullptr != pTreeItemData)
    {
        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pTreeItemData->extendedItemData());

        if (nullptr != pKAData)
        {
            pKAData->setFilePath(filePath);
        }
    }
}

// ---------------------------------------------------------------------------
bool kaApplicationTreeHandler::IsNewFileNameValid(const QString& fileName, QString& errorMessage, bool moreThenOnce)
{
    bool retVal = false;
    errorMessage.clear();
    // Make sure that the name contain single file extension
    bool isExtensionStructureValid = false;
    QString extensionBeforeEdit = acGetFileExtension(m_fileNameBeforeEdit, isExtensionStructureValid);
    isExtensionStructureValid = fileName.indexOf(".") > 0 ? true : false;

    if (fileName.trimmed().isEmpty())
    {
        errorMessage = KA_STR_FileNameEmpty;
    }
    else if ((fileName.length() > 0) && (fileName.trimmed().isEmpty()))
    {
        errorMessage = KA_STR_FileNameOnlyWhitespaces;
    }
    else if (fileName.length() != fileName.trimmed().length())
    {
        errorMessage = KA_STR_FileNameLeadingOrTrailing;
    }
    else if (fileName.contains(QRegExp("[*?/:<>%|\\\\]")))
    {
        errorMessage = KA_STR_FileNameSpecial;
    }
    else if (!isExtensionStructureValid)
    {
        errorMessage = QString(KA_STR_KernelFileStructureInvalid);
    }
    else
    {
        afApplicationTreeItemData* pSelectedItemData = GetSelectedItemData();

        if (pSelectedItemData != nullptr)
        {
            kaTreeDataExtension* pSelectedItemDataExtension = qobject_cast<kaTreeDataExtension*> (pSelectedItemData->extendedItemData());

            if (pSelectedItemDataExtension != nullptr)
            {
                QString fullFileName = acGTStringToQString(pSelectedItemDataExtension->filePath().asString());
                gtString fileNameExt;
                pSelectedItemDataExtension->filePath().getFileNameAndExtension(fileNameExt);
                QString qsFileNameExt = acGTStringToQString(fileNameExt);
                fullFileName.replace(qsFileNameExt, fileName);
                QFile fileForRename(fullFileName);

                if (fileForRename.exists())
                {
                    errorMessage = KA_STR_FileNameExists;
                    retVal = false;
                }
                else
                {
                    retVal = true;
                }
            }
        }
    }

    GT_IF_WITH_ASSERT(m_pProgramsRootNode != nullptr)
    {
        // Make sure that a kernel with the same name doesn't exist:
        int kernelsCount = m_pProgramsRootNode->childCount();

        // Count the number of times the kernel name appears once for itself and if it appear twice
        // it is invalid
        int numOfAppearance = 0;

        for (int i = 0; i < kernelsCount; i++)
        {
            QTreeWidgetItem* pKernelItem = m_pProgramsRootNode->child(i);

            if (!(pKernelItem->flags() & Qt::ItemIsEditable))
            {
                QString currentKernelName = pKernelItem->text(0);

                currentKernelName = currentKernelName.mid(0, currentKernelName.indexOf("."));// .remove(".cl");

                if (currentKernelName.toLower() == fileName.toLower())
                {
                    numOfAppearance++;
                }
            }
        }

        if ((numOfAppearance > 1) || (!moreThenOnce && 1 == numOfAppearance))
        {
            retVal = false;
            errorMessage = KA_STR_FileNameExists;
        }
    }
    return retVal;
}

bool kaApplicationTreeHandler::IsProgramNameValid(const QString& programName, QString& errorMessage)
{
    bool retVal = false;
    errorMessage.clear();

    if (programName.trimmed().isEmpty())
    {
        errorMessage = KA_STR_ProgramNameEmpty;
    }
    else if ((programName.length() > 0) && (programName.trimmed().isEmpty()))
    {
        errorMessage = KA_STR_ProgramNameOnlyWhitespaces;
    }
    else if (programName.length() != programName.trimmed().length())
    {
        errorMessage = KA_STR_ProgramNameLeadingOrTrailing;
    }
    else if (programName.contains(QRegExp("[*?/:<>%|\\\\]")))
    {
        errorMessage = KA_STR_ProgramNameSpecial;
    }
    else
    {
        retVal = true;
        gtString programNameGT = acQStringToGTString(programName);
        int programsCount = KA_PROJECT_DATA_MGR_INSTANCE.GetPrograms().size();

        for (int i = 0; i < programsCount; i++)
        {
            if (KA_PROJECT_DATA_MGR_INSTANCE.GetPrograms().at(i)->GetProgramName() == programNameGT)
            {
                retVal = false;
                errorMessage = KA_STR_ProgramNameExists;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::CloseEditor()
{
    if (nullptr != m_pApplicationTree)
    {
        afTreeCtrl* pTreeCtrl = m_pApplicationTree->treeControl();

        if (nullptr != pTreeCtrl)
        {
            QList<QTreeWidgetItem*> treeSelectedItems = pTreeCtrl->selectedItems();
            QTreeWidgetItem* pItem = nullptr;

            if (!treeSelectedItems.isEmpty())
            {
                pItem = treeSelectedItems.at(0);
            }

            if (nullptr != pItem)
            {
                // before closing check if it is a valid item and notify the user here in VS and not in OnEditorClosed
                // The event loop that is created in while opening a dialog there causes a crash (BUG449423)
                // the notification is after we are done closing the edit field otherwise the creation of the messagebox
                // will force the closing (again) and that is causing the crash:
                QLineEdit* pLineEditor = pTreeCtrl->lineEditor();
                bool shouldShowMessage = false;
                QString errorMessage;

                if (nullptr != pLineEditor)
                {
                    // Store old tree item string this is needed for the validity check it assume the tree node already has the new edit value
                    QString oldStringValue = pItem->text(0);
                    pItem->setText(0, pLineEditor->text());

                    if (!IsNewFileNameValid(pLineEditor->text(), errorMessage, false))
                    {
                        shouldShowMessage = true;
                        pLineEditor->setText(oldStringValue);
                        pItem->setText(0, oldStringValue);
                    }
                }

                pTreeCtrl->closePersistentEditor(pItem, 0);

                if (shouldShowMessage)
                {
                    acMessageBox::instance().critical(afGlobalVariablesManager::ProductNameA(), errorMessage);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::ClearTree()
{
    // Sanity check
    GT_IF_WITH_ASSERT((m_pProgramsRootNode != nullptr) && (m_pApplicationTree != nullptr))
    {
        // Make sure that the file root is empty
        int count = m_pProgramsRootNode->childCount();

        for (int i = count - 1; i >= 0; i--)
        {
            // Get the next child:
            QTreeWidgetItem* pChild = m_pProgramsRootNode->child(i);
            afApplicationTreeItemData* pTreeItemData = m_pApplicationTree->getTreeItemData(pChild);

            if (pTreeItemData != nullptr)
            {
                if (pTreeItemData->m_itemType == AF_TREE_ITEM_KA_PROGRAM)
                {
                    if ((pChild != nullptr) && (pChild != m_pAddFileTreeItem))
                    {
                        m_pApplicationTree->removeTreeItem(pChild, false);
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::OnSourcePathChanged(gtString oldSourcePaths, gtString newSourcePaths)
{
    // Verify both string are in matching condition for comparison and both ends with semicolon
    newSourcePaths = newSourcePaths.trim().append(AF_STR_Semicolon);
    oldSourcePaths = oldSourcePaths.trim().append(AF_STR_Semicolon);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    newSourcePaths = newSourcePaths.toLowerCase();
    oldSourcePaths = oldSourcePaths.toLowerCase();
#endif

    gtStringTokenizer strTokenizer(newSourcePaths, AF_STR_Semicolon);
    gtString sourcePathDiff;
    gtString currentPath;

    while (strTokenizer.getNextToken(currentPath))
    {
        currentPath = currentPath.append(AF_STR_Semicolon);

        if (oldSourcePaths.find(currentPath) < 0)
        {
            // Path is new to the project
            sourcePathDiff.append(currentPath);
        }
    }

    // Add CL files from source dir to project three
    kaApplicationCommands::instance().AddCLFilesToProject(sourcePathDiff);
}


void kaApplicationTreeHandler::RenameSourceFile(afApplicationTreeItemData* pItemData, osFilePath oldFilePath, const QString& newFileName)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pItemData != nullptr) 
    {
        // Assert again for the item data after looking for the "real" source file item
        GT_IF_WITH_ASSERT(pItemData != nullptr)
        {
            // Downcast to KA data:
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());
            GT_IF_WITH_ASSERT(pKAData != nullptr)
            {
                QString fullFileName = acGTStringToQString(pKAData->filePath().asString());
                QFile fileForRename(fullFileName);
                gtString oldFileName;
                QString oldName;
                pKAData->filePath().getFileNameAndExtension(oldFileName);
                GT_IF_WITH_ASSERT(!oldFileName.isEmpty())
                {
                    oldName = acGTStringToQString(oldFileName);
                }
                fullFileName.replace(oldName, newFileName);
                bool rc = fileForRename.copy(fullFileName);

                GT_IF_WITH_ASSERT(rc)
                {
                    osFilePath newFilePath(acQStringToGTString(fullFileName));

                    // Replace the file path in the update documents manager:
                    afDocUpdateManager::instance().RenameFile(oldFilePath, newFilePath);

                    // Rename the folder name:
                    osFilePath oldDirPath = kaApplicationCommands::instance().GetOutputDirectoryFilePath(oldFilePath, false);
                    osFilePath newDirPath = kaApplicationCommands::instance().GetOutputDirectoryFilePath(newFilePath, false);

                    // Replace the file path in the project data manager:
                    KA_PROJECT_DATA_MGR_INSTANCE.RenameFile(oldFilePath, newFilePath, newDirPath);

                    // Go over all the children of this item, and rename the file path in the item data:
                    RenameFileRecursive(pItemData, newFilePath);

                    // Set the new file path:
                    pKAData->setFilePath(newFilePath);

                    if (oldDirPath.exists())
                    {
                        osDirectory oldDir;
                        GT_IF_WITH_ASSERT(oldDirPath.getFileDirectory(oldDir))
                        {
                            oldDir.rename(newDirPath.asString());
                        }
                    }

                    // Go through all the "devices.cxltxt" and "overview.cxlovr" files, and write the new cl file path into them:
                    SetNewPathInTxtFiles(newDirPath, newFilePath);

                    // Rename all opened views:
                    HandleRenamedOpenedViews(oldFilePath, oldDirPath, pItemData);

                    // Rename the file references in the programs
                    RenameAllFileOccurencesInTree(oldFilePath, newFilePath);

                }
            }
        }
    }
}

void kaApplicationTreeHandler::RenameFileRecursive(afApplicationTreeItemData* pItemData, const osFilePath& newFilePath)
{
    if (pItemData != nullptr)
    {
        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

        if (pKAData != nullptr)
        {
            osFilePath identifyPath, detailedPath;
            gtString newFileName;
            newFilePath.getFileNameAndExtension(newFileName);
            gtString nodeName = acQStringToGTString(pItemData->m_pTreeWidgetItem->text(0));

            pKAData->setFilePath(newFilePath);
        }

        QTreeWidgetItem* pItem = pItemData->m_pTreeWidgetItem;

        if (pItem != nullptr)
        {
            for (int i = 0; i < pItem->childCount(); i++)
            {
                QTreeWidgetItem* pChild = pItem->child(i);

                if (pChild != nullptr)
                {
                    // Get the child item data:
                    afApplicationTreeItemData* pChildData = m_pApplicationTree->getTreeItemData(pChild);

                    // Call the recursive function to update the children's file path:
                    RenameFileRecursive(pChildData, newFilePath);
                }

            }
        }
    }
}

void kaApplicationTreeHandler::HandleRenamedOpenedViews(const osFilePath& oldFilePath, const osFilePath& oldBuildDirectory, afApplicationTreeItemData* pRenamedItemData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pRenamedItemData != nullptr)
    {
        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pRenamedItemData->extendedItemData());
        GT_IF_WITH_ASSERT(nullptr != pKAData)
        {
            // Get the file info from the project data manager:
            kaSourceFile* pFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(pKAData->filePath());

            if ((afMainAppWindow::instance() != nullptr) && (pFile != nullptr))
            {
                // Handle opened source file path window:
                afQMdiSubWindow* pExistingSubwindow = afMainAppWindow::instance()->findMDISubWindow(oldFilePath);

                if (pExistingSubwindow != nullptr)
                {
                    // Find the new and old names for the window:
                    gtString oldViewName, newViewName;
                    oldFilePath.getFileNameAndExtension(oldViewName);
                    pKAData->filePath().getFileNameAndExtension(newViewName);

                    // Set the window title:
                    afMainAppWindow::instance()->renameMDIWindow(acGTStringToQString(oldViewName), acGTStringToQString(newViewName), acGTStringToQString(pKAData->filePath().asString()));

                    pExistingSubwindow->setWindowTitle(acGTStringToQString(newViewName));
                    pExistingSubwindow->setFilePath(pKAData->filePath().asString());
                }

                // Handle opened overview window:
                osFilePath oldOverviewFilePath = oldBuildDirectory;
                osFilePath newOverviewFilePath = pFile->buildDirectory().directoryPath();

                oldOverviewFilePath.setFileName(KA_STR_overviewName);
                oldOverviewFilePath.setFileExtension(KA_STR_overviewExtension);

                newOverviewFilePath.setFileName(KA_STR_overviewName);
                newOverviewFilePath.setFileExtension(KA_STR_overviewExtension);

                pExistingSubwindow = afMainAppWindow::instance()->findMDISubWindow(oldOverviewFilePath);

                if (pExistingSubwindow != nullptr)
                {
                    // Find the new and old names for the window:
                    gtString newViewName;
                    newOverviewFilePath.getFileNameAndExtension(newViewName);

                    // Set the window title:
                    afMainAppWindow::instance()->renameMDIWindow(acGTStringToQString(oldOverviewFilePath.asString()), acGTStringToQString(newOverviewFilePath.asString()), acGTStringToQString(newOverviewFilePath.asString()));

                    pExistingSubwindow->setWindowTitle(acGTStringToQString(newViewName));
                    pExistingSubwindow->setFilePath(newOverviewFilePath.asString());
                }

            }
        }
    }
}

void kaApplicationTreeHandler::SetNewPathInTxtFiles(osFilePath& newDirPath, osFilePath& newFilePath)
{
    // Find all "devices.cxltxt" files under the new directory:
    osDirectory projectDir(newDirPath.asString());

    gtList<osFilePath> kernelDirectortiesList;
    projectDir.getSubDirectoriesPaths(osDirectory::SORT_BY_DATE_ASCENDING, kernelDirectortiesList);
    kernelDirectortiesList.push_front(newDirPath);

    // Iterate the file paths, open each of them and write the new cl / DirectX file path:
    for (auto dirIter = kernelDirectortiesList.begin(); dirIter != kernelDirectortiesList.end(); dirIter++)
    {
        // Prepare the file filter for "devices"cxltxt" files:
        gtString fileFilter;
        fileFilter.appendFormattedString(L"%ls.%ls", KA_STR_kernelViewFile, KA_STR_kernelViewExtension);

        osDirectory kernelDir(*dirIter);
        gtList<osFilePath> deviceTXTFiles;
        kernelDir.getContainedFilePaths(fileFilter, osDirectory::SORT_BY_DATE_ASCENDING, deviceTXTFiles);

        fileFilter.makeEmpty();
        fileFilter.appendFormattedString(L"%ls.%ls", KA_STR_overviewName, KA_STR_overviewExtension);
        gtList<osFilePath> deviceOverviewFiles;
        kernelDir.getContainedFilePaths(fileFilter, osDirectory::SORT_BY_DATE_ASCENDING, deviceTXTFiles);


        gtList<osFilePath> allFilesList;

        // Iterate the file paths, open each of them and write the new cl / DirectX file path:
        for (auto iter = deviceTXTFiles.begin(); iter != deviceTXTFiles.end(); iter++)
        {
            allFilesList.push_front(*iter);
        }

        for (auto iter = deviceOverviewFiles.begin(); iter != deviceOverviewFiles.end(); iter++)
        {
            allFilesList.push_front(*iter);
        }

        // Iterate the file paths, open each of them and write the new cl / DirectX file path:
        for (auto iter = allFilesList.begin(); iter != allFilesList.end(); iter++)
        {
            osFile currentFile;
            bool rc = currentFile.open(*iter, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
            GT_IF_WITH_ASSERT(rc)
            {
                currentFile.writeString(newFilePath.asString());
            }

            currentFile.close();

        }
    }
}

bool kaApplicationTreeHandler::IsRenamePossible(const afApplicationTreeItemData* pRenamedItemData)
{
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT(pRenamedItemData != nullptr)
    {
        // File rename operation
        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pRenamedItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pKAData != nullptr)
        {
            if (pRenamedItemData->m_itemType == AF_TREE_ITEM_KA_PROGRAM)
            {
                // Check if there are build artifacts for this program
                GT_IF_WITH_ASSERT(pKAData->GetProgram() != nullptr)
                {
                    // Get the output directories, and check if one of them is not empty
                    osDirectory outputDir32, outputDir64;
                    pKAData->GetProgram()->GetAndCreateOutputDirectories(outputDir32, outputDir64, false, false);
                    bool doesBuildArtifactsExist = !outputDir64.IsEmpty() || !outputDir32.IsEmpty();

                    if (doesBuildArtifactsExist)
                    {
                        // Check if an item related to this program is opened
                        bool doesProgramFilesOpen = false;
                        gtVector<osFilePath> listOfOpenedWindows;
                        afApplicationCommands::instance()->GetListOfOpenedWindowsForFile(outputDir32.directoryPath().asString(), listOfOpenedWindows);
                        doesProgramFilesOpen = !listOfOpenedWindows.empty();
                        afApplicationCommands::instance()->GetListOfOpenedWindowsForFile(outputDir64.directoryPath().asString(), listOfOpenedWindows);
                        doesProgramFilesOpen = doesProgramFilesOpen || !listOfOpenedWindows.empty();

                        // Output a message box warns the user for opened windows, and existing build artifacts
                        QString errorMessage = KA_STR_treeRenameProgramQuestion;

                        if (doesProgramFilesOpen)
                        {
                            errorMessage = KA_STR_treeRenameProgramWithOpenedQuestion;
                        }

                        int userInput = acMessageBox::instance().question(afGlobalVariablesManager::ProductNameA(), errorMessage, QMessageBox::Yes | QMessageBox::No);
                        retVal = (userInput == QMessageBox::Yes);
                    }
                }
            }
            else
            {

                // Get the output folder for the requested file. Do not create it, we only need it to look for opened windows:
                osFilePath outputFolderPath = kaApplicationCommands::instance().GetOutputDirectoryFilePath(pKAData->filePath(), false);
                gtString outputFolderStr = outputFolderPath.asString();

                QStringList sourceFilesExtensions = QString(AF_STR_AnalyzedSourceFileExtension).split(AF_STR_CommaA);

                // Find all the windows that are opened for this file:
                gtVector<osFilePath> listOfOpenedWindows;
                afApplicationCommands::instance()->GetListOfOpenedWindowsForFile(outputFolderStr, listOfOpenedWindows);

                // A popup should appear if there are opened windows, and one of them is not a cl file:
                bool shouldPopupUser = false;

                // Check if there is a file with an extension different then "cl":
                for (auto iter = listOfOpenedWindows.begin(); iter != listOfOpenedWindows.end(); iter++)
                {
                    gtString extension;
                    (*iter).getFileExtension(extension);

                    if (!sourceFilesExtensions.contains(acGTStringToQString(extension)))
                    {
                        shouldPopupUser = true;
                        break;
                    }
                }

                // If there are opened windows for this file, ask the user if these windows should be closed:
                if (shouldPopupUser)
                {
                    int userAction = acMessageBox::instance().question(afGlobalVariablesManager::ProductNameA(), KA_STR_treeRenameQuestion, QMessageBox::Yes | QMessageBox::No);
                    retVal = (userAction == QMessageBox::Yes);

                    if (retVal)
                    {
                        // Close all sub-windows:
                        for (auto iter = listOfOpenedWindows.begin(); iter != listOfOpenedWindows.end(); iter++)
                        {
                            gtString extension;
                            (*iter).getFileExtension(extension);

                            if (extension != AF_STR_clSourceFileExtension)
                            {
                                afApplicationCommands::instance()->closeFile(*iter);
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

afApplicationTreeItemData* kaApplicationTreeHandler::FindFileMatchingTreeItemData(const osFilePath& filePath)
{
    afApplicationTreeItemData* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pProgramsRootNode != nullptr) && (m_pApplicationTree != nullptr))
    {
        for (int i = 0; i < m_pProgramsRootNode->childCount(); i++)
        {
            QTreeWidgetItem* pChild = m_pProgramsRootNode->child(i);

            if (pChild != nullptr)
            {
                afApplicationTreeItemData* pChildData = m_pApplicationTree->getTreeItemData(pChild);

                if (pChildData != nullptr)
                {
                    if (pChildData->m_itemType == AF_TREE_ITEM_KA_FILE)
                    {
                        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pChildData->extendedItemData());

                        if (pKAData != nullptr)
                        {
                            if (pKAData->filePath() == filePath)
                            {
                                pRetVal = pChildData;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    return pRetVal;
}

QString kaApplicationTreeHandler::GetBuildCommandString(kaTreeDataExtension* pKAData, afTreeItemType itemType)
{
    QString retVal(KA_STR_openclBuildASCII);

    GT_IF_WITH_ASSERT((pKAData != nullptr) && (pKAData->GetProgram() != nullptr))
    {
        // For pipeline programs, or a program selected item, build the program. Otherwise, build the file
        bool shouldBuildProgram = (itemType == AF_TREE_ITEM_KA_PROGRAM) || (itemType >= AF_TREE_ITEM_KA_PROGRAM_GL_GEOM && itemType <= AF_TREE_ITEM_KA_PROGRAM_GL_COMP);

        if (shouldBuildProgram)
        {
            retVal = QString(KA_STR_buildCommandFormat).arg(acGTStringToQString(pKAData->GetProgram()->GetProgramName()));
        }
        else
        {
            GetBuildCommandString(pKAData->filePath());
        }
    }

    return retVal;
}

QString kaApplicationTreeHandler::GetBuildCommandString(const osFilePath& filePath)
{
    QString retVal(KA_STR_openclBuildASCII);

    kaSourceFile* pCurrentFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(filePath);

    if (pCurrentFile != nullptr)
    {
        gtString fileName;
        filePath.getFileNameAndExtension(fileName);

        if (acGTStringToQString(pCurrentFile->BuildPlatform()) == KA_STR_platformDirectX)
        {
            retVal = QString(KA_STR_buildCommandFormatDirectX).arg(acGTStringToQString(pCurrentFile->EntryPointFunction())).arg(acGTStringToQString(fileName));
        }
        else
        {
            retVal = QString(KA_STR_buildCommandFormat).arg(acGTStringToQString(fileName));
        }
    }

    return retVal;
}


QString kaApplicationTreeHandler::GetBuildCommandString()
{
    QString buildString = KA_STR_openclBuildASCII;

    gtVector<osFilePath> filesForBuild;
    afApplicationTreeItemData* pSelectedItemData = GetSelectedItemData();

    if (pSelectedItemData != nullptr)
    {
        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pSelectedItemData->extendedItemData());

        switch (pSelectedItemData->m_itemType)
        {
            case  AF_TREE_ITEM_KA_PROGRAM_GL_VERT:
            case  AF_TREE_ITEM_KA_PROGRAM_GL_TESC:
            case  AF_TREE_ITEM_KA_PROGRAM_GL_TESE:
            case  AF_TREE_ITEM_KA_PROGRAM_GL_GEOM:
            case  AF_TREE_ITEM_KA_PROGRAM_GL_FRAG:
            case  AF_TREE_ITEM_KA_PROGRAM_GL_COMP:
            case AF_TREE_ITEM_KA_PROGRAM:
                if (pKAData != nullptr)
                {
                    buildString = GetBuildCommandString(pKAData, pSelectedItemData->m_itemType);
                }

                break;

            case AF_TREE_ITEM_KA_ADD_FILE:
            case AF_TREE_ITEM_KA_NEW_FILE:
            {
                if (pSelectedItemData->m_pTreeWidgetItem != nullptr)
                {
                    QTreeWidgetItem* pParentItem = pSelectedItemData->m_pTreeWidgetItem->parent();

                    if (pParentItem != nullptr)
                    {
                        QVariant itemData = pParentItem->data(0, Qt::UserRole);
                        afApplicationTreeItemData* pParentData = (afApplicationTreeItemData*)itemData.value<void*>();

                        if ((pParentData != nullptr) && (pParentData->m_itemType == AF_TREE_ITEM_KA_PROGRAM))
                        {
                            pKAData = qobject_cast<kaTreeDataExtension*>(pParentData->extendedItemData());

                            if (pKAData != nullptr)
                            {
                                buildString = GetBuildCommandString(pKAData, pParentData->m_itemType);
                            }
                        }
                    }
                }
            }
            break;

            case AF_TREE_ITEM_KA_STATISTICS:
            case AF_TREE_ITEM_KA_ANALYSIS:
            case AF_TREE_ITEM_KA_DEVICE:
            case AF_TREE_ITEM_KA_OUT_DIR:
            case AF_TREE_ITEM_KA_REF_TYPE:
            {
                //get name of active program
                kaProgram* pProgram = KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram();

                if (pProgram != nullptr)
                {
                    buildString.append(" ");
                    buildString.append(acGTStringToQString(pProgram->GetProgramName()));
                }
            }
            break;

            case AF_TREE_ITEM_KA_PROGRAM_SHADER:
                if (activeBuildFiles(filesForBuild) == 1)
                {
                    buildString = GetBuildCommandString(filesForBuild[0]);
                }
                else if (activeBuildFiles(filesForBuild) > 1)
                {
                    // building selected files
                    buildString.append(" ");
                    buildString.append(KA_STR_SelectedFiles);
                }

                break;

            default:

                // when only one file is selected add the file name to the command caption
                if (activeBuildFiles(filesForBuild) == 1)
                {
                    buildString = GetBuildCommandString(filesForBuild[0]);
                }

                break;
        }
    }

    return buildString;
}

kaApplicationTreeHandler::kaTreeIconIndex kaApplicationTreeHandler::GetIconForFile(const osFilePath& filePath, const kaSourceFile* pFileData) const
{
    kaTreeIconIndex fileIcon = KA_PIXMAP_FOLDER;

    if (nullptr == pFileData)
    {
        pFileData = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(filePath);
    }

    if (nullptr != pFileData)
    {
        gtString buildPlatform = pFileData->BuildPlatform();

        if (buildPlatform.compareNoCase(KA_STR_platformVulkan_GT) == 0)
        {
            fileIcon = KA_PIXMAP_VK_FOLDER;
        }
        else if (buildPlatform.compareNoCase(KA_STR_platformOpenCL_GT) == 0)
        {
            fileIcon = KA_PIXMAP_CL_FOLDER;
        }
        else if (buildPlatform.compareNoCase(KA_STR_platformDirectX_GT) == 0)
        {
            fileIcon = KA_PIXMAP_DX_FOLDER;
        }
        else if (buildPlatform.compareNoCase(KA_STR_platformOpenGL_GT) == 0)
        {
            fileIcon = KA_PIXMAP_GL_FOLDER;
        }
    }

    return fileIcon;
}

void kaApplicationTreeHandler::BuildComputeProgramTreeItemNode(kaComputeProgram* pComputeProgram, QTreeWidgetItem* pProgramTreeNodeItem)
{
    GT_IF_WITH_ASSERT((pComputeProgram != nullptr) && (pProgramTreeNodeItem != nullptr))
    {
        AddProgramTreeItemChild(pProgramTreeNodeItem, AF_TREE_ITEM_KA_PROGRAM_GL_COMP, osFilePath());

    }
}

afApplicationTreeItemData* kaApplicationTreeHandler::FindChildOfType(QTreeWidgetItem* pParent, afTreeItemType childType, const osFilePath& filePath)
{
    afApplicationTreeItemData* pRetVal = nullptr;

    if (pParent != nullptr)
    {
        for (int i = 0; i < pParent->childCount(); i++)
        {
            afApplicationTreeItemData* pTemp = m_pApplicationTree->getTreeItemData(pParent->child(i));
            bool isSameItem = (pTemp && pTemp->m_itemType == childType);

            if (isSameItem && (!filePath.isEmpty()))
            {
                isSameItem = false;
                kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pTemp->extendedItemData());

                if (pKAData && pKAData->filePath() == filePath)
                {
                    isSameItem = true;
                }
            }

            if (isSameItem)
            {
                pRetVal = pTemp;
                break;
            }
        }
    }

    return pRetVal;
}

afApplicationTreeItemData* kaApplicationTreeHandler::AddProgramTreeItemChild(QTreeWidgetItem* pProgramTreeNodeItem, afTreeItemType itemType, const osFilePath& childFilePath)
{
    afApplicationTreeItemData* pRetVal = nullptr;
    GT_IF_WITH_ASSERT((pProgramTreeNodeItem != nullptr) && (m_pIconsArray != nullptr))
    {
        afApplicationTreeItemData* pProgramItemData = m_pApplicationTree->getTreeItemData(pProgramTreeNodeItem);
        GT_IF_WITH_ASSERT(pProgramItemData != nullptr)
        {
            kaTreeDataExtension* pExtendedData = qobject_cast<kaTreeDataExtension*>(pProgramItemData->extendedItemData());
            GT_IF_WITH_ASSERT((pExtendedData != nullptr) && (pExtendedData->GetProgram() != nullptr))
            {
                // Find the item text
                gtString fileName;

                if (!childFilePath.isEmpty())
                {
                    childFilePath.getFileNameAndExtension(fileName);
                }

                gtString childText = ProgramItemTypeAsText(itemType, fileName);
                QColor itemColor = acQLIST_EDITABLE_ITEM_COLOR;

                pRetVal = new afApplicationTreeItemData;
                pRetVal->m_itemType = itemType;
                pRetVal->m_filePath = childFilePath;

                // Create an extended item data
                kaTreeDataExtension* pChildExtendedData = new kaTreeDataExtension;
                pChildExtendedData->SetProgram(pExtendedData->GetProgram());
                pRetVal->setExtendedData(pChildExtendedData);
                pChildExtendedData->setFilePath(childFilePath);

                int fileId = -1;

                // Find the file id according to the program type
                kaProgram* pProgram = pExtendedData->GetProgram();
                kaRenderingProgram* pRenderingProgram = dynamic_cast<kaRenderingProgram*>(pProgram);
                kaComputeProgram* pComputeProgram = dynamic_cast<kaComputeProgram*>(pProgram);
                kaNonPipelinedProgram* pNPProgram = dynamic_cast<kaNonPipelinedProgram*>(pProgram);

                if (pRenderingProgram != nullptr)
                {
                    // Get the reference type for this item type
                    kaPipelinedProgram::PipelinedStage stage = kaRenderingProgram::TreeItemTypeToRenderingStage(itemType);
                    fileId = pRenderingProgram->GetFileID(stage);
                    if (fileId != -1)
                    {
                        // show stage name
                        gtString stageNameAsString = ProgramItemTypeAsText(itemType, L"");
                        QString nodeName = acGTStringToQString(stageNameAsString);

                        if (!nodeName.isEmpty())
                        {
                            childText = acQStringToGTString( QString("%1 (%2)").arg(nodeName).arg(acGTStringToQString(childText)));
                        }                   
                    }
                }
                else if (pComputeProgram != nullptr)
                {
                    fileId = pComputeProgram->GetFileID();
                }
                else if (pNPProgram != nullptr)
                {
                    fileId = KA_PROJECT_DATA_MGR_INSTANCE.GetFileID(childFilePath);
                }

                if (fileId >= 0)
                {
                    gtString fileName;
                    osFilePath refFilePath;
                    KA_PROJECT_DATA_MGR_INSTANCE.GetFilePathByID(fileId, refFilePath);
                    refFilePath.getFileNameAndExtension(fileName);

                    if (!fileName.isEmpty() && (childFilePath.isEmpty()))
                    {
                        childText.appendFormattedString(L" (%ls)", fileName.asCharArray());
                    }

                    itemColor = Qt::black;
                    pChildExtendedData->setFilePath(refFilePath);
                }


                // Find where to add the new file item (before which item)
                QTreeWidgetItem* pBeforeItem = FindProgramFileBeforeItem(pProgramTreeNodeItem);
                QTreeWidgetItem* pNewNode = m_pApplicationTree->insertTreeItem(childText, pRetVal, pProgramTreeNodeItem, pBeforeItem);

                if (pNewNode != nullptr)
                {
                    gtString fileTooltip = childFilePath.asString();

                    if (fileTooltip.isEmpty())
                    {
                        kaUtils::TreeItemTypeToPipeLineStageString(itemType, fileTooltip);
                        QString attachShaderStr = QString(KA_STR_attachStageShaderToASCII).arg(acGTStringToQString(fileTooltip));
                        pNewNode->setToolTip(0, attachShaderStr);
                    }
                    else
                    {
                        pNewNode->setToolTip(0, acGTStringToQString(fileTooltip));
                    }

                    pNewNode->setTextColor(0, itemColor);
                    pNewNode->setIcon(0, m_pIconsArray[KA_PIXMAP_SOURCE]);
                }
            }
        }
    }

    return pRetVal;
}

QTreeWidgetItem* kaApplicationTreeHandler::FindProgramFileBeforeItem(QTreeWidgetItem* pProgramItem)
{
    QTreeWidgetItem* pRetVal = nullptr;
    GT_IF_WITH_ASSERT((pProgramItem != nullptr) && (m_pApplicationTree != nullptr))
    {
        for (int i = 0; i < pProgramItem->childCount(); i++)
        {
            QTreeWidgetItem* pChild = pProgramItem->child(i);
            afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pChild);

            if (pItemData != nullptr)
            {
                if ((pItemData->m_itemType > AF_TREE_ITEM_KA_LAST_FILE_ITEM_TYPE) || (pItemData->m_itemType < AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE))
                {
                    pRetVal = pChild;
                    break;
                }
            }
        }
    }
    return pRetVal;
}

gtString kaApplicationTreeHandler::ProgramItemTypeAsText(afTreeItemType itemType, const gtString& itemText)
{
    gtString retVal = itemText;

    if (retVal.isEmpty())
    {
        switch (itemType)
        {
            case AF_TREE_ITEM_KA_PROGRAM_GL_GEOM:
                retVal = KA_STR_fileGLGeom;
                break;

            case AF_TREE_ITEM_KA_PROGRAM_GL_FRAG:
                retVal = KA_STR_fileGLFrag;
                break;

            case AF_TREE_ITEM_KA_PROGRAM_GL_TESC:
                retVal = KA_STR_fileGLTesc;
                break;

            case AF_TREE_ITEM_KA_PROGRAM_GL_TESE:
                retVal = KA_STR_fileGLTese;
                break;

            case AF_TREE_ITEM_KA_PROGRAM_GL_VERT:
                retVal = KA_STR_fileGLVert;
                break;

            case AF_TREE_ITEM_KA_PROGRAM_GL_COMP:
                retVal = KA_STR_fileGLComp;
                break;

            case AF_TREE_ITEM_KA_ADD_FILE:
            case AF_TREE_ITEM_KA_NEW_FILE:
            case AF_TREE_ITEM_KA_PROGRAM:
            case AF_TREE_ITEM_KA_PROGRAM_SHADER:
                break;

            default:
                GT_ASSERT_EX(false, L"Unsupported program item")
                break;
        }
    }

    return retVal;
}

void kaApplicationTreeHandler::BuildNPProgramTreeItemNode(kaNonPipelinedProgram* pNPProgram, QTreeWidgetItem* pProgramTreeNodeItem)
{
    GT_IF_WITH_ASSERT((pNPProgram != nullptr) && (pProgramTreeNodeItem != nullptr))
    {
        gtVector<int> shaderIDList = pNPProgram->GetFileIDsVector();
        osFilePath filePath;

        for (const int& it : shaderIDList)
        {
            filePath.clear();
            // Get the file path from the data manager
            KA_PROJECT_DATA_MGR_INSTANCE.GetFilePathByID(it, filePath);
            AddProgramTreeItemChild(pProgramTreeNodeItem, AF_TREE_ITEM_KA_PROGRAM_SHADER, filePath);
        }
    }
}

void kaApplicationTreeHandler::BuildRenderingProgramTreeItemNode(kaRenderingProgram* pRenderingProgram, QTreeWidgetItem* pProgramTreeNodeItem)
{
    GT_IF_WITH_ASSERT((pRenderingProgram) && (pProgramTreeNodeItem != nullptr))
    {
        gtVector<int> filesIds = pRenderingProgram->GetFileIDsVector();
        int filesCount = filesIds.size();
        gtVector<osFilePath> filesPaths;
        filesPaths.resize(filesCount);
        gtVector<afTreeItemType> shaderStages;

        for (int i = 0; i < filesCount; ++i)
        {
            if (filesIds[i] != -1)
            {
                KA_PROJECT_DATA_MGR_INSTANCE.GetFilePathByID(filesIds[i], filesPaths[i]);
            }
        }

        AddProgramTreeItemChild(pProgramTreeNodeItem, AF_TREE_ITEM_KA_PROGRAM_GL_VERT, filesPaths[0]);
        AddProgramTreeItemChild(pProgramTreeNodeItem, AF_TREE_ITEM_KA_PROGRAM_GL_TESC, filesPaths[1]);
        AddProgramTreeItemChild(pProgramTreeNodeItem, AF_TREE_ITEM_KA_PROGRAM_GL_TESE, filesPaths[2]);
        AddProgramTreeItemChild(pProgramTreeNodeItem, AF_TREE_ITEM_KA_PROGRAM_GL_GEOM, filesPaths[3]);
        AddProgramTreeItemChild(pProgramTreeNodeItem, AF_TREE_ITEM_KA_PROGRAM_GL_FRAG, filesPaths[4]);
    }
}

bool kaApplicationTreeHandler::GetActiveItemFilePath(osFilePath& activeFilePath)
{
    bool retVal = false;

    const afApplicationTreeItemData* pItemData = activeItemData();

    while (nullptr != pItemData)
    {
        if (pItemData->m_itemType == AF_TREE_ITEM_KA_FILE)
        {
            // select the correct platform based on the file type
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

            if (nullptr != pKAData)
            {
                activeFilePath = pKAData->filePath();
                retVal = true;

            }

            break;
        }
        else
        {
            pItemData = FindParentItemDataOfType(pItemData, AF_TREE_ITEM_KA_FILE);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void kaApplicationTreeHandler::OnExportToCSVAction()
{
    gtString deviceName;
    gtString fileName;
    gtString filePath;

    if (m_pApplicationTree != nullptr && m_pApplicationTree->treeControl() != nullptr && !m_pApplicationTree->treeControl()->selectedItems().isEmpty())
    {
        QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();
        QTreeWidgetItem* pContextMenuItem = treeSelectedItems.at(0);
        GT_IF_WITH_ASSERT(pContextMenuItem != nullptr)
        {
            afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pContextMenuItem);
            GT_IF_WITH_ASSERT(pItemData != nullptr)
            {
                kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

                if (nullptr != pKAData)
                {
                    osFilePath detailedFilePath = pKAData->detailedFilePath();
                    detailedFilePath.getFileName(deviceName);
                    fileName = deviceName;
                    detailedFilePath.setFileName(fileName);
                    detailedFilePath.setFileExtension(KA_STR_kernelViewISAExtension);

                    if (detailedFilePath.exists())
                    {
                        osDirectory fileDir;
                        gtString gtNewFileName;
                        detailedFilePath.getFileName(gtNewFileName);
                        detailedFilePath.getFileDirectory(fileDir);
                        osFilePath containingPath;
                        containingPath = fileDir.directoryPath();
                        gtString strFileDir = containingPath.asString();
                        QFileInfo dirInfo(acGTStringToQString(strFileDir));
                        // Get project name as base for file name:
                        QString dirName = dirInfo.baseName();
                        QString csvFileName = acGTStringToQString(afProjectManager::instance().currentProjectSettings().projectName());
                        csvFileName.append(AF_STR_HyphenA);
                        csvFileName.append(afGlobalVariablesManager::ProductNameA());

                        dirName.prepend(AF_STR_Hyphen);
                        dirName.append(AF_STR_Hyphen);
                        gtNewFileName.prepend(acQStringToGTString(dirName));
                        QDateTime dateTime = dateTime.currentDateTime();
                        QString dateTimeString = dateTime.toString("yyyyMMdd-hhmm");
                        dateTimeString.prepend(AF_STR_Hyphen);
                        gtNewFileName << acQStringToGTString(dateTimeString);
                        filePath = detailedFilePath.asString();
                        QString qFilePath = acGTStringToQString(gtNewFileName);
                        QString csvFilters = acGTStringToQString(AF_STR_csvFileDetails);
                        csvFileName.append(qFilePath);
                        csvFileName.append(AF_STR_saveCSVFilePostfix);

                        QString loadFileName = afApplicationCommands::instance()->ShowFileSelectionDialog(KA_STR_exportToCSV, csvFileName, csvFilters, m_pExportToCSVAction, true);

                        if (!loadFileName.isEmpty())
                        {
                            // Read the file.
                            std::string strFilePath(detailedFilePath.asString().asASCIICharArray());
                            std::string strISAFileContent;
                            std::ifstream file(strFilePath);
                            strISAFileContent = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                            ParserISA parserISA;
                            bool parseSucceeded = parserISA.Parse(strISAFileContent);

                            if (parseSucceeded)
                            {
                                const std::vector<Instruction*> isaInstructions = parserISA.GetInstructions();

                                // Read the buffer into the source table:
                                if (!isaInstructions.empty())
                                {
                                    QString instructionsAsCSV;
                                    instructionsAsCSV.append(CSV_HEADER);
                                    std::string emptyString;
                                    afProgressBarWrapper::instance().setProgressDetails(L"Parsing ISA for CSV Export", isaInstructions.size());

                                    for (Instruction* pInstruction : isaInstructions)
                                    {
                                        GT_IF_WITH_ASSERT(pInstruction != nullptr)
                                        {
                                            pInstruction->GetCSVString(deviceName.asASCIICharArray(), emptyString);
                                            instructionsAsCSV.append(emptyString.c_str());
                                            afProgressBarWrapper::instance().incrementProgressBar();
                                        }

                                    }

                                    afProgressBarWrapper::instance().hideProgressBar();

                                    if (!instructionsAsCSV.isEmpty())
                                    {
                                        // Save the file:
                                        osFile file1(acQStringToGTString(loadFileName));
                                        bool rcOpen = file1.open(osFile::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
                                        GT_IF_WITH_ASSERT(rcOpen)
                                        {
                                            // Write the data:
                                            file1.writeString(acQStringToGTString(instructionsAsCSV));
                                            file1.close();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void kaApplicationTreeHandler::OnNewProgram()
{
    if (kaBackendManager::instance().isInBuild())
    {
        acMessageBox::instance().critical(afGlobalVariablesManager::ProductNameA(), KA_STR_ERR_CANNOT_CREATE_PROGRAM_DURING_BUILD);
    }
    else
    {
        // Create new Program (do not force project creation):
        kaApplicationCommands::instance().NewProgramCommand(false);
    }
}

void kaApplicationTreeHandler::AddProgram(bool focusNode, kaProgram* pProgram)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr)
    {
        if (pProgram != nullptr)
        {
            m_pApplicationTree->blockSignals(true);

            // Add the item to the tree:
            afApplicationTreeItemData* pItemData = new afApplicationTreeItemData;
            pItemData->m_itemType = AF_TREE_ITEM_KA_PROGRAM;
            pItemData->m_isItemRemovable = false;
            // extension data is needed even empty so the search will work correctly:
            kaTreeDataExtension* pExtensionData;
            pExtensionData = new kaTreeDataExtension;
            pExtensionData->SetProgram(pProgram);
            pItemData->setExtendedData(pExtensionData);

            // Add the program tree node item
            QTreeWidgetItem* pProgramTreeNodeItem = m_pApplicationTree->insertTreeItem(pProgram->GetProgramName(), pItemData, m_pProgramsRootNode, nullptr/*m_pSourcesTreeItem*/);
            GT_IF_WITH_ASSERT(pProgramTreeNodeItem != nullptr)
            {
                // Set the program node text and icon
                pProgramTreeNodeItem->setTextColor(0, Qt::black);

                kaProgramTypes programBuildType = pProgram->GetBuildType();
                kaTreeIconIndex iconIndex = KA_PIXMAP_FOLDER;

                // Add children to the program node
                switch (programBuildType)
                {
                    case kaProgramVK_Rendering:
                    case kaProgramGL_Rendering:
                    {
                        iconIndex = ((kaProgramGL_Rendering == programBuildType) ? KA_PIXMAP_GL_FOLDER : KA_PIXMAP_VK_FOLDER);

                        // Down cast to a rendering program and add rendering tree items
                        kaRenderingProgram* pRenderingProgram = dynamic_cast<kaRenderingProgram*>(pProgram);
                        BuildRenderingProgramTreeItemNode(pRenderingProgram, pProgramTreeNodeItem);
                    }
                    break;

                    case kaProgramVK_Compute:
                    case kaProgramGL_Compute:
                    {
                        iconIndex = ((kaProgramGL_Compute == programBuildType) ? KA_PIXMAP_GL_FOLDER : KA_PIXMAP_VK_FOLDER);

                        // Down cast to a rendering program and add rendering tree items
                        kaComputeProgram* pComputeProgram = dynamic_cast<kaComputeProgram*>(pProgram);
                        BuildComputeProgramTreeItemNode(pComputeProgram, pProgramTreeNodeItem);
                    }
                    break;

                    case kaProgramCL:
                    case kaProgramDX:
                    {
                        iconIndex = ((kaProgramCL == programBuildType) ? KA_PIXMAP_CL_FOLDER : KA_PIXMAP_DX_FOLDER);
                        kaNonPipelinedProgram* pNPProgram = dynamic_cast<kaNonPipelinedProgram*>(pProgram);
                        BuildNPProgramTreeItemNode(pNPProgram, pProgramTreeNodeItem);

                        // Add "Add file" and "Create file" tree items
                        AddFileCreateTreeItem(pProgramTreeNodeItem);
                        AddFileAddTreeItem(pProgramTreeNodeItem);
                    }
                    break;

                    default:
                        GT_ASSERT(false);
                        break;
                }

                pProgramTreeNodeItem->setIcon(0, m_pIconsArray[iconIndex]);
            }

            if (focusNode)
            {
                // Make sure the node is visible and expanded:
                m_pApplicationTree->expandItem(pProgramTreeNodeItem);
                m_pApplicationTree->scrollToItem(pProgramTreeNodeItem);
                m_pApplicationTree->treeControl()->setCurrentItem(pProgramTreeNodeItem);
            }

            m_pApplicationTree->blockSignals(false);
        }
    }
}

bool kaApplicationTreeHandler::IsItemDroppable(QTreeWidgetItem* pDestinationItem)
{
    bool retVal = afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode);

    if (retVal && (m_pApplicationTree != nullptr) && (m_pApplicationTree->treeControl() != nullptr))
    {
        retVal = false;

        if (pDestinationItem != nullptr)
        {
            // Get the dragged item file path
            QTreeWidgetItem* pDraggedItem = m_pApplicationTree->treeControl()->DraggedItem();
            afApplicationTreeItemData* pDraggedItemData = nullptr;
            kaTreeDataExtension* pKADraggedItemData = nullptr;

            if (pDraggedItem != nullptr)
            {
                pDraggedItemData = m_pApplicationTree->getTreeItemData(pDraggedItem);

                if (pDraggedItemData != nullptr)
                {
                    pKADraggedItemData = qobject_cast<kaTreeDataExtension*>(pDraggedItemData->extendedItemData());
                }
            }

            afApplicationTreeItemData* pDestinationItemData = m_pApplicationTree->getTreeItemData(pDestinationItem);

            if ((pDestinationItemData != nullptr))
            {
                kaTreeDataExtension* pKADestinationItemData = qobject_cast<kaTreeDataExtension*>(pDestinationItemData->extendedItemData());

                osFilePath draggedFile;

                if (pKADraggedItemData != nullptr)
                {
                    draggedFile = pKADraggedItemData->filePath();
                }

                if (pKADestinationItemData != nullptr)
                {
                    // if a destination node is a node under program node
                    kaProgram* pProgram = nullptr;
                    afTreeItemType destinationItemType = pDestinationItemData->m_itemType;

                    if (((destinationItemType >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) &&
                         (destinationItemType <= AF_TREE_ITEM_KA_LAST_FILE_ITEM_TYPE)) ||
                        (destinationItemType == AF_TREE_ITEM_KA_ADD_FILE) ||
                        (destinationItemType == AF_TREE_ITEM_KA_NEW_FILE) ||
                        (destinationItemType == AF_TREE_ITEM_KA_PROGRAM) ||
                        (destinationItemType == AF_TREE_ITEM_KA_OUT_DIR))
                    {
                        // Get the item related to the program parent
                        QTreeWidgetItem* pParentProgram = FindProgramTreeItem(pKADestinationItemData->GetProgram());

                        if (pParentProgram != nullptr)
                        {
                            afApplicationTreeItemData* pParentData = m_pApplicationTree->getTreeItemData(pParentProgram);

                            if ((pParentData != nullptr)  && (pParentData->m_itemType == AF_TREE_ITEM_KA_PROGRAM))
                            {
                                kaTreeDataExtension* pParentKAItemData = qobject_cast<kaTreeDataExtension*>(pParentData->extendedItemData());

                                // obtain kaProgram* from extended data
                                pProgram = pParentKAItemData->GetProgram();

                                if (pProgram != nullptr)
                                {
                                    retVal = true;

                                    if (pDestinationItemData->m_itemType == AF_TREE_ITEM_KA_PROGRAM)
                                    {
                                        if ((pProgram->GetBuildType() == kaProgramDX) || (pProgram->GetBuildType() == kaProgramCL))
                                        {
                                            retVal = true;
                                        }
                                        else
                                        {
                                            retVal = false;
                                        }

                                        // Expand the program item when hovering
                                        pDestinationItem->setExpanded(true);
                                    }


                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

bool kaApplicationTreeHandler::CanItemBeDragged(QTreeWidgetItem* pItem)
{
    bool retVal = afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode);

    if (retVal && (m_pApplicationTree != nullptr))
    {
        retVal = false;

        if (pItem != nullptr)
        {
            afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pItem);

            if (pItemData != nullptr)
            {
                if ((pItemData->m_itemType >= AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE) &&
                    (pItemData->m_itemType <= AF_TREE_ITEM_KA_LAST_FILE_ITEM_TYPE))
                {
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}


void kaApplicationTreeHandler::UpdateItemProgramNodeToBold(QTreeWidgetItem* pCurrentItem) const
{

    afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pCurrentItem);
    GT_IF_WITH_ASSERT(pItemData != nullptr)
    {

        const afApplicationTreeItemData* pActiveProgramApplicaionTreeData = FindParentItemDataOfType(pItemData, AF_TREE_ITEM_KA_PROGRAM);

        if (pActiveProgramApplicaionTreeData != nullptr && pActiveProgramApplicaionTreeData->m_pTreeWidgetItem != nullptr)
        {
            QTreeWidgetItem* pActiveProgramWidget = pActiveProgramApplicaionTreeData->m_pTreeWidgetItem;

            if (pActiveProgramWidget != nullptr && pActiveProgramWidget->parent() != nullptr)
            {
                QTreeWidgetItem* parent = pActiveProgramWidget->parent();

                for (int i = 0; i < parent->childCount(); ++i)
                {
                    QTreeWidgetItem* child = parent->child(i);

                    if (child != nullptr)
                    {
                        QFont qFont = child->font(0);
                        qFont.setBold(false);
                        child->setFont(0, qFont);
                    }
                }

                QFont qFont = pActiveProgramWidget->font(0);
                qFont.setBold(true);
                pActiveProgramWidget->setFont(0, qFont);
            }
        }
    }

}


void kaApplicationTreeHandler::OnTreeCurrentItemChanged(QTreeWidgetItem* pCurrentItem, QTreeWidgetItem* pPreviousItem)
{
    GT_UNREFERENCED_PARAMETER(pPreviousItem);

    if (pCurrentItem != nullptr)
    {
        UpdateItemProgramNodeToBold(pCurrentItem);
    }
}

void BuildDeviceAndBinFilesMap(const kaProjectDataManager::FPathToOutFPathsMap& buildFilesMap, DevicesAndBinFilesMap& devicesAndBinFiles, gtSet<gtString>& devicesToExport)
{
    for (const auto& mapItr : buildFilesMap)
    {
        const osFilePath originaFilePath = mapItr.first;
        const gtList<osFilePath>& buildFiles = mapItr.second;

        for (const auto& buildFile : buildFiles)
        {
            const gtString originalFName;
            gtString kernelName, deviceName, codeRep;
            kaParseBuildFile(buildFile, kernelName, deviceName, codeRep);

            if (codeRep == KA_STR_buildMainBinaryFileName)
            {
                devicesAndBinFiles.insert(std::pair<std::pair<osFilePath, gtString>, gtString>(make_pair(originaFilePath, deviceName), buildFile.asString()));
                devicesToExport.insert(deviceName);
            }
        }
    }//for all original files
}

void kaApplicationTreeHandler::OnDragMoveEvent(QDragMoveEvent *pEvent)
{
    if ((pEvent != nullptr) && (m_pApplicationTree->treeControl() != nullptr))
    {
        QTreeWidgetItem* pItem = m_pApplicationTree->treeControl()->itemAt(pEvent->pos());

        if (pItem != nullptr)
        {
            // check if dragging above stage shader item that is a candidate for hint
            afApplicationTreeItemData* pTargetItemData = m_pApplicationTree->getTreeItemData(pItem);
            kaTreeDataExtension* pTargetItemKAData = nullptr;
            if (pTargetItemData != nullptr)
            {
                pTargetItemKAData  = qobject_cast<kaTreeDataExtension*>(pTargetItemData->extendedItemData());
                afTreeItemType hoveredItemType = pTargetItemData->m_itemType;
                
                if (pTargetItemKAData != nullptr && pTargetItemKAData->filePath().isEmpty() && hoveredItemType >= AF_TREE_ITEM_KA_PROGRAM_GL_GEOM &&  hoveredItemType <= AF_TREE_ITEM_KA_PROGRAM_GL_COMP)
                {
                    //stage shader detected - now check if a file being dragged fits this stage
                    bool shouldHint = false;
                    const QMimeData* pMimeData = pEvent->mimeData();
                    //get file type from dragged item
                    if (pMimeData != nullptr)
                    {
                        osFilePath sourceFilePath;
                        gtString fileExtension;
                        //if a file is being dragged from OS file browser
                        if (pMimeData->hasUrls())
                        {
                            QList<QUrl> urlList = pMimeData->urls();
                            if (urlList.size() == 1)
                            {
                                //only single file drag can trigger hint
                                QString currentFile = urlList.at(0).toLocalFile();
                                sourceFilePath.setFullPathFromString(acQStringToGTString(currentFile));
                                sourceFilePath.getFileExtension(fileExtension);
                            }
                        }
                        else if (pMimeData->hasFormat("text/plain"))
                        {
                            // tree item is being dragged
                            afApplicationTreeItemData* pDraggedItemData = nullptr;
                            kaTreeDataExtension* pDraggedItemKAData = nullptr;

                            QTreeWidgetItem* pDraggedItem = m_pApplicationTree->treeControl()->DraggedItem();

                            if (pDraggedItem != nullptr)
                            {
                                pDraggedItemData = m_pApplicationTree->getTreeItemData(pDraggedItem);

                                if (pDraggedItemData != nullptr)
                                {
                                    pDraggedItemKAData = qobject_cast<kaTreeDataExtension*>(pDraggedItemData->extendedItemData());

                                    if (pDraggedItemKAData != nullptr)
                                    {
                                        if (!pDraggedItemKAData->filePath().isEmpty())
                                        {
                                            sourceFilePath = pDraggedItemKAData->filePath();
                                            sourceFilePath.getFileExtension(fileExtension);
                                        }
                                    }
                                }
                            }
                        }
                        if (!sourceFilePath.isEmpty())
                        {
                            QString qsFileExtension = acGTStringToQString(fileExtension);

                            int shaderIndex = QString(KA_STR_OpenGLShaderExtensions).split(AF_STR_CommaA).indexOf(qsFileExtension);
                            if (shaderIndex != -1)
                            {
                                switch (shaderIndex)
                                {
                                case 0:
                                case 1:
                                    if (AF_TREE_ITEM_KA_PROGRAM_GL_FRAG == hoveredItemType)
                                    {
                                        shouldHint = true;
                                    }
                                    break;
                                case 2:
                                case 3:
                                    if (AF_TREE_ITEM_KA_PROGRAM_GL_VERT == hoveredItemType)
                                    {
                                        shouldHint = true;
                                    }
                                    break;
                                case 4:
                                case 5:
                                    if (AF_TREE_ITEM_KA_PROGRAM_GL_COMP == hoveredItemType)
                                    {
                                        shouldHint = true;
                                    }
                                    break;
                                case 6:
                                case 7:
                                    if (AF_TREE_ITEM_KA_PROGRAM_GL_GEOM == hoveredItemType)
                                    {
                                        shouldHint = true;
                                    }
                                    break;
                                case 8:
                                    if (AF_TREE_ITEM_KA_PROGRAM_GL_TESC == hoveredItemType)
                                    {
                                        shouldHint = true;
                                    }
                                    break;
                                case 9:
                                    if (AF_TREE_ITEM_KA_PROGRAM_GL_TESE == hoveredItemType)
                                    {
                                        shouldHint = true;
                                    }
                                    break;
                                default:
                                    shouldHint = false;
                                    break;
                                }

                            }
                        }
                        if (m_pLastHintItem != nullptr)
                        {
                            m_pLastHintItem->setBackgroundColor(0, STAGE_ORIG_BGCOLOR);
                        }
                        if ( shouldHint)
                        {
                            m_pLastHintItem = pItem;
                            m_pLastHintItem->setBackgroundColor(0, STAGE_HINT_BGCOLOR);
                        }
                    }
                }
                else
                {
                    if (m_pLastHintItem != nullptr)
                    {
                        m_pLastHintItem->setBackgroundColor(0, STAGE_ORIG_BGCOLOR);
                    }

                }
            }
        }
    }
}
