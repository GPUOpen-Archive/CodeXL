//------------------------------ kaMDIViewCreator.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>

// Framework:
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afSourceCodeViewsManager.h>
#include <AMDTApplicationFramework/src/afUtils.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>
#include <AMDTApplicationFramework/Include/afTreeItemType.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaApplicationTreeHandler.h>
#include <AMDTKernelAnalyzer/src/kaKernelView.h>
#include <AMDTKernelAnalyzer/src/kaMDIViewCreator.h>
#include <AMDTKernelAnalyzer/src/kaMenuActionsExecutor.h>
#include <AMDTKernelAnalyzer/src/kaMultiSourceView.h>
#include <AMDTKernelAnalyzer/src/kaOverviewView.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaTreeDataExtension.h>
#include <AMDTKernelAnalyzer/Include/kaCommandIDs.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTKernelAnalyzer/src/kaMultiSourceActionCreator.h>

// ---------------------------------------------------------------------------
// Name:        kaMDIViewCreator::kaMDIViewCreator
// Description: Creator
// Author:      Gilad Yarnitzky
// Date:        10/8/2013
// ---------------------------------------------------------------------------
kaMDIViewCreator::kaMDIViewCreator()
{
    // Create the view actions creator:
    _pViewActionCreator = new kaMultiSourceActionCreator;

    _pViewActionCreator->initializeCreator();
}

// ---------------------------------------------------------------------------
// Name:        kaMDIViewCreator::~kaMDIViewCreator
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        10/8/2013
// ---------------------------------------------------------------------------
kaMDIViewCreator::~kaMDIViewCreator()
{
}

// ---------------------------------------------------------------------------
// Name:        kaMDIViewCreator::titleString
// Description: Get the title of the created view
// Arguments:   int viewIndex
// Author:      Gilad Yarnitzky
// Date:        10/8/2013
// ---------------------------------------------------------------------------
void kaMDIViewCreator::titleString(int viewIndex, gtString& viewTitle, gtString& viewMenuCommand)
{
    (void)(viewIndex); // unused
    apMDIViewCreateEvent* pCurrentEvent = dynamic_cast<apMDIViewCreateEvent*>(_pCreationEvent);

    if (nullptr != pCurrentEvent)
    {
        viewTitle = pCurrentEvent->viewTitle();
        viewMenuCommand = viewTitle;
    }
}


// ---------------------------------------------------------------------------
// Name:        kaMDIViewCreator::associatedToolbar
// Description: Get the name of the toolbar associated with the requested view
// Arguments:   int viewIndex
// Return Val:  gtString
// Author:      Gilad Yarnitzky
// Date:        10/8/2013
// ---------------------------------------------------------------------------
gtString kaMDIViewCreator::associatedToolbar(int viewIndex)
{
    (void)(viewIndex); // unused
    gtString retVal;

    GT_IF_WITH_ASSERT(nullptr != _pCreationEvent)
    {
        // Source code creation event:
        {
            retVal = L"";
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        kaMDIViewCreator::type
// Description: Get view type
// Arguments:   int viewIndex
// Return Val:  afViewType
// Author:      Gilad Yarnitzky
// Date:        10/8/2013
// ---------------------------------------------------------------------------
kaMDIViewCreator::afViewType kaMDIViewCreator::type(int viewIndex)
{
    (void)(viewIndex); // unused
    afViewCreatorAbstract::afViewType retDockArea = AF_VIEW_mdi;

    return retDockArea;
}

// ---------------------------------------------------------------------------
// Name:        kaMDIViewCreator::dockArea
// Description: Get the docking area
// Arguments:   int viewIndex
// Author:      Gilad Yarnitzky
// Date:        10/8/2013
// ---------------------------------------------------------------------------
int kaMDIViewCreator::dockArea(int viewIndex)
{
    (void)(viewIndex); // unused
    int retVal = AF_VIEW_DOCK_LeftDockWidgetArea;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        kaMDIViewCreator::dockWidgetFeatures
// Description: Irrelevant for MDI views
// Return Val:  DockWidgetFeatures
// Author:      Gilad Yarnitzky
// Date:        10/8/2013
// ---------------------------------------------------------------------------
QDockWidget::DockWidgetFeatures kaMDIViewCreator::dockWidgetFeatures(int viewIndex)
{
    (void)(viewIndex); // unused
    return QDockWidget::NoDockWidgetFeatures;
}

// ---------------------------------------------------------------------------
// Name:        kaMDIViewCreator::initialSize
// Description: Get the initial size
// Arguments:   int viewIndex
// Return Val:  QSize - size of the view
// Author:      Gilad Yarnitzky
// Date:        10/8/2013
// ---------------------------------------------------------------------------
QSize kaMDIViewCreator::initialSize(int viewIndex)
{
    (void)(viewIndex); // unused
    QSize retSize(0, 0);

    return retSize;
}

// ---------------------------------------------------------------------------
// Name:        kaMDIViewCreator::visibility
// Description: Get the initial visibility of the view
// Arguments:   int viewIndex
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        10/8/2013
// ---------------------------------------------------------------------------
bool kaMDIViewCreator::visibility(int viewIndex)
{
    (void)(viewIndex); // unused
    bool retVal = true;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaMDIViewCreator::amountOfViewTypes
// Description: Get number of views that can be created by this creator
// Return Val:  int
// Author:      Gilad Yarnitzky
// Date:        10/8/2013
// ---------------------------------------------------------------------------
int kaMDIViewCreator::amountOfViewTypes()
{
    return 1;
}

// ---------------------------------------------------------------------------
// Name:        kaMDIViewCreator::createViewContent
// Description: Create the content for the view
// Arguments:   int viewIndex
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        10/8/2013
// ---------------------------------------------------------------------------
bool kaMDIViewCreator::createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent)
{
    (void)(viewIndex); // unused
    bool retVal = false;

    apMDIViewCreateEvent* pCurrentEvent = dynamic_cast<apMDIViewCreateEvent*>(_pCreationEvent);
    osFilePath kernelPath, identifyPath, detailedPath;

    bool rc = getInformationPaths(pCurrentEvent, kernelPath, identifyPath, detailedPath);

    // Create the views based on the event:
    GT_IF_WITH_ASSERT(rc && (nullptr != pCurrentEvent) && (nullptr != pCurrentEvent->ItemData()))
    {
        afApplicationTreeItemData* pTreeData = afApplicationCommands::instance()->applicationTree()->getTreeItemData((QTreeWidgetItem*)pCurrentEvent->ItemData());
        GT_IF_WITH_ASSERT(nullptr != pTreeData)
        {
            switch (pTreeData->m_itemType)
            {
                case AF_TREE_ITEM_KA_OVERVIEW:
                {
                    ValidateOverviewFileExists(identifyPath);
                    pContentQWidget = new kaOverviewView(pQParent, identifyPath);

                    m_createdViewsMap[identifyPath.asString()] = pContentQWidget;
                }
                break;

                case AF_TREE_ITEM_KA_ANALYSIS:
                case AF_TREE_ITEM_KA_STATISTICS:
                case AF_TREE_ITEM_KA_DEVICE:
                {
                    kaKernelView* pKernelView = new kaKernelView(pQParent);
                    pKernelView->setDataFile(identifyPath);

                    // pass the node text in case of device node
                    kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pTreeData->extendedItemData());

                    if (nullptr != pKAData && pTreeData->m_itemType == AF_TREE_ITEM_KA_DEVICE)
                    {
                        pKernelView->displayFile(detailedPath, kernelPath, pTreeData->m_itemType, pTreeData->m_pTreeWidgetItem->text(0));
                    }
                    else
                    {
                        pKernelView->displayFile(detailedPath, kernelPath, pTreeData->m_itemType);
                    }

                    pContentQWidget = pKernelView;
                    m_createdViewsMap[identifyPath.asString()] = pContentQWidget;
                }
                break;

                default:
                    break;
            }
        }
    }

    if (nullptr != pContentQWidget)
    {
        // add the created content widget to the created list:
        retVal = true;
        m_viewsCreated.push_back(pContentQWidget);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaMDIViewCreator::getCurrentlyDisplayedFilePath
// Description: Get the currently displayed file path
// Arguments:   osFilePath& filePath
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        10/8/2013
// ---------------------------------------------------------------------------
bool kaMDIViewCreator::getCurrentlyDisplayedFilePath(osFilePath& filePath)
{
    bool retVal = false;

    apMDIViewCreateEvent* pCurrentEvent = dynamic_cast<apMDIViewCreateEvent*>(_pCreationEvent);
    osFilePath kernelPath, identifyPath, detailedPath;

    bool rc = getInformationPaths(pCurrentEvent, kernelPath, identifyPath, detailedPath);

    // Create the views based on the event:
    GT_IF_WITH_ASSERT(rc && (nullptr != pCurrentEvent) && (nullptr != pCurrentEvent->ItemData()))
    {
        filePath =  identifyPath;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        kaMDIViewCreator::displayExistingView
// Description:
// Arguments:   const apMDIViewCreateEvent& mdiViewEvent
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        10/8/2013
// ---------------------------------------------------------------------------
bool kaMDIViewCreator::displayExistingView(const apMDIViewCreateEvent& mdiViewEvent)
{
    bool retVal = false;

    apMDIViewCreateEvent* pCurrentEvent = (apMDIViewCreateEvent*)(&mdiViewEvent);
    osFilePath kernelPath, identifyPath, detailedPath;

    bool rc = getInformationPaths(pCurrentEvent, kernelPath, identifyPath, detailedPath);

    // Create the views based on the event:
    GT_IF_WITH_ASSERT(rc && (nullptr != pCurrentEvent) && (nullptr != pCurrentEvent->ItemData()))
    {
        gtString searchString = identifyPath.asString();

        if (m_createdViewsMap.find(searchString) != m_createdViewsMap.end())
        {
            retVal = true;
        }

        if (retVal)
        {
            afApplicationTreeItemData* pTreeData = afApplicationCommands::instance()->applicationTree()->getTreeItemData((QTreeWidgetItem*)pCurrentEvent->ItemData());
            GT_IF_WITH_ASSERT(nullptr != pTreeData)
            {
                switch (pTreeData->m_itemType)
                {
                    case AF_TREE_ITEM_KA_ANALYSIS:
                    case AF_TREE_ITEM_KA_STATISTICS:
                    case AF_TREE_ITEM_KA_DEVICE:
                    {
                        // Find the subwindow that holds this path:
                        kaKernelView* pKernelView = qobject_cast<kaKernelView*>(m_createdViewsMap[searchString]);
                        GT_IF_WITH_ASSERT(nullptr != pKernelView)
                        {
                            // display the detailed file in case it is a different file then the one displayed there:

                            // pass the node text in case of device node
                            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pTreeData->extendedItemData());

                            if (nullptr != pKAData && pTreeData->m_itemType == AF_TREE_ITEM_KA_DEVICE)
                            {
                                pKernelView->displayFile(detailedPath, kernelPath, pTreeData->m_itemType, pTreeData->m_pTreeWidgetItem->text(0));
                            }
                            else
                            {
                                pKernelView->displayFile(detailedPath, kernelPath, pTreeData->m_itemType);
                            }
                        }
                    }
                    break;

                    default:
                        break;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        kaMDIViewCreator::onMDISubWindowClose
// Description: Handle sub window close
// Arguments:   afQMdiSubWindow* pMDISubWindow
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        10/8/2013
// ---------------------------------------------------------------------------
bool kaMDIViewCreator::onMDISubWindowClose(afQMdiSubWindow* pMDISubWindow)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(pMDISubWindow != nullptr)
    {
        // Get the sub window widget:
        QWidget* pWidget = pMDISubWindow->widget();

        if (pWidget != nullptr)
        {
            gtMap<gtString, QWidget*>::iterator mapIterator;

            for (mapIterator = m_createdViewsMap.begin() ; mapIterator != m_createdViewsMap.end() ; mapIterator++)
            {
                if ((*mapIterator).second == pWidget)
                {
                    m_createdViewsMap.erase(mapIterator);
                    break;
                }
            }

            int existingViewIndex = -1;

            for (int i = 0; i < (int)m_viewsCreated.size(); i++)
            {
                if (pWidget == m_viewsCreated[i])
                {
                    existingViewIndex = i;
                    break;
                }
            }

            // Remove the view:
            if (existingViewIndex >= 0)
            {
                m_viewsCreated.removeItem(existingViewIndex);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        kaMDIViewCreator::handleTrigger
// Description: Handle the action when it is triggered
// Arguments:   int actionIndex
// Author:      Gilad Yarnitzky
// Date:        10/8/2013
// ---------------------------------------------------------------------------
void kaMDIViewCreator::handleTrigger(int viewIndex, int actionIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pMainWindow != nullptr)
    {
        // Get the current sub window:
        afQMdiSubWindow* pSubWindow = pMainWindow->activeMDISubWindow();

        // Get the current source code view:
        kaKernelView* pKernelView = qobject_cast<kaKernelView*>(pSubWindow->widget());

        // Get the current source code view:
        kaOverviewView* pOverView = qobject_cast<kaOverviewView*>(pSubWindow->widget());

        int commandId = actionIndexToCommandId(actionIndex);

        if (ID_SHOW_LINE_NUMBERS == commandId)
        {
            afSourceCodeViewsManager& theSourceCodeViewsManager = afSourceCodeViewsManager::instance();
            bool isWSVisible = theSourceCodeViewsManager.showLineNumbers();

            // Set all the open source code windows white spaces view flag:
            theSourceCodeViewsManager.setViewLineNumbers(!isWSVisible);
            toggleAllMultiSourceViewsLineNumbers();
        }
        else
        {

            if (nullptr != pKernelView)
            {
                switch (commandId)
                {
                    case ID_CUT:
                        pKernelView->onEdit_Cut();
                        break;

                    case ID_COPY:
                    {
                        pKernelView->onEdit_Copy();

                        if (nullptr != pOverView)
                        {
                            pOverView->onEdit_Copy();
                        }

                        break;
                    }

                    case ID_PASTE:
                        pKernelView->onEdit_Paste();
                        break;

                    case ID_SELECT_ALL:
                    {
                        pKernelView->onEdit_SelectAll();

                        if (nullptr != pOverView)
                        {
                            pOverView->onEdit_SelectAll();
                        }

                        break;
                    }

                    case ID_FIND:
                    {
                        // Store the source code view from which the find dialog was called:
                        pKernelView->storeFindClickedView();

                        // Open the find widget (and respond to single characters click)
                        pMainWindow->OnFind(true);
                        break;
                    }

                    case ID_FIND_NEXT:
                        pKernelView->onEdit_FindNext();
                        break;

                    case AF_ID_SAVE_FILE:
                        pKernelView->FileSave();
                        break;

                    case AF_ID_SAVE_FILE_AS:
                        pKernelView->FileSaveAs();
                        break;

                    default:
                    {
                        GT_ASSERT_EX(false, L"Should not get here");
                        break;
                    }
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
void kaMDIViewCreator::toggleAllMultiSourceViewsLineNumbers()
{
    afSourceCodeViewsManager& theSourceCodeViewsManager = afSourceCodeViewsManager::instance();
    bool isWSVisible = theSourceCodeViewsManager.showLineNumbers();

    // Pass through all the MDI files and close those that are do not exist
    QMdiArea* pMdiArea = afMainAppWindow::instance()->mdiArea();

    QList<QMdiSubWindow*> windowsSubList = pMdiArea->subWindowList();

    foreach (QMdiSubWindow* pCurrentSubWindow, windowsSubList)
    {
        // Get the widget from the window:
        afQMdiSubWindow* pAfQTSubWindow = qobject_cast<afQMdiSubWindow*>(pCurrentSubWindow);

        if (pAfQTSubWindow != nullptr)
        {
            kaKernelView* pKernelView = qobject_cast<kaKernelView*>(pAfQTSubWindow->widget());

            if (nullptr != pKernelView)
            {
                pKernelView->showLineNumbers(isWSVisible);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        kaMDIViewCreator::handleUiUpdate
// Description: Handle UI update
// Arguments:   int actionIndex
// Author:      Gilad Yarnitzky
// Date:        10/8/2013
// ---------------------------------------------------------------------------
void kaMDIViewCreator::handleUiUpdate(int viewIndex, int actionIndex)
{
    (void)viewIndex;

    bool isActionEnabled = false, isActionChecked = false, isActionCheckable = false;
    QString updatedActionText;

    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(nullptr != pMainWindow)
    {
        // Get the current sub window:
        afQMdiSubWindow* pSubWindow = pMainWindow->activeMDISubWindow();

        // Get the current source code view:
        kaKernelView* pKernelView = qobject_cast<kaKernelView*>(pSubWindow->widget());

        // Get the current source code view:
        kaOverviewView* pOverView = qobject_cast<kaOverviewView*>(pSubWindow->widget());

        isActionCheckable = false;
        int commandId = actionIndexToCommandId(actionIndex);

        if (nullptr != pKernelView)
        {
            isActionCheckable = false;

            switch (commandId)
            {
                case ID_CUT:
                    pKernelView->onUpdateEdit_Copy(isActionEnabled);
                    break;

                case ID_COPY:
                {
                    pKernelView->onUpdateEdit_Copy(isActionEnabled);

                    if (nullptr != pOverView)
                    {
                        pOverView->onUpdateEdit_Copy(isActionEnabled);
                    }

                    break;
                }

                case ID_PASTE:
                    pKernelView->onUpdateEdit_Copy(isActionEnabled);
                    break;

                case ID_SELECT_ALL:
                {
                    pKernelView->onUpdateEdit_SelectAll(isActionEnabled);

                    if (nullptr != pOverView)
                    {
                        pOverView->onUpdateEdit_SelectAll(isActionEnabled);
                    }

                    break;
                }

                case ID_FIND:
                    pKernelView->onUpdateEdit_Find(isActionEnabled);
                    break;

                case ID_FIND_NEXT:
                    pKernelView->onUpdateEdit_FindNext(isActionEnabled);

                //not implemented - caused assertion in default case
                case   ID_FIND_PREV:
                    pKernelView->onUpdateEdit_FindNext(isActionEnabled);
                    break;

                case ID_SHOW_LINE_NUMBERS:
                {
                    isActionEnabled = true;
                    isActionCheckable = true;
                    isActionChecked = afSourceCodeViewsManager::instance().showLineNumbers();
                    break;
                }

                case AF_ID_SAVE_FILE:
                case AF_ID_SAVE_FILE_AS:
                    // If the active tab inside the kernel view is a multi source view enable the command:
                    pKernelView->OnUpdateFileSave(isActionEnabled);
                    break;

                default:
                {
                    GT_ASSERT_EX(false, L"Should not get here");
                    break;
                }
            }
        }
    }

    // Get the QT action:
    QAction* pAction = _pViewActionCreator->action(actionIndex);
    GT_IF_WITH_ASSERT(pAction != nullptr)
    {
        // Set the action enable / disable:
        pAction->setEnabled(isActionEnabled);

        // Set the action checkable state:
        pAction->setCheckable(isActionCheckable);

        // Set the action check state:
        pAction->setChecked(isActionChecked);

        // Update the text if needed:
        if (!updatedActionText.isEmpty())
        {
            pAction->setText(updatedActionText);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        kaMDIViewCreator::getInformationPaths
// Description: Get the information about the node
// Author:      Gilad Yarnitzky
// Date:        20/8/2013
// ---------------------------------------------------------------------------
bool kaMDIViewCreator::getInformationPaths(apMDIViewCreateEvent* pCreateEvent, osFilePath& kernelPath, osFilePath& identifyPath, osFilePath& detailedPath)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT((nullptr != pCreateEvent) && (nullptr != pCreateEvent->ItemData()) && (nullptr != afApplicationCommands::instance()))
    {
        QTreeWidgetItem* pTreeItem = (QTreeWidgetItem*)pCreateEvent->ItemData();
        afApplicationTreeItemData* pTreeData = afApplicationCommands::instance()->applicationTree()->getTreeItemData(pTreeItem);

        GT_IF_WITH_ASSERT(nullptr != pTreeData)
        {
            kaTreeDataExtension* pTreeDataExtension = qobject_cast<kaTreeDataExtension*>(pTreeData->extendedItemData());
            GT_IF_WITH_ASSERT(nullptr != pTreeDataExtension)
            {
                kaSourceFile* pFileData = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(pTreeDataExtension->filePath());

                GT_IF_WITH_ASSERT(nullptr != pFileData)
                {
                    kernelPath = pTreeDataExtension->filePath();
                }

                switch (pTreeData->m_itemType)
                {
                    case AF_TREE_ITEM_KA_OVERVIEW:
                    case AF_TREE_ITEM_KA_ANALYSIS:
                    case AF_TREE_ITEM_KA_STATISTICS:
                    case AF_TREE_ITEM_KA_DEVICE:
                    {
                        detailedPath = pTreeDataExtension->detailedFilePath();
                        identifyPath = pTreeDataExtension->identifyFilePath();
                        retVal = true;
                    }
                    break;

                    default:
                        break;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void kaMDIViewCreator::ValidateOverviewFileExists(const osFilePath& filePath)
{
    // Validate that the over file exists.
    // It might not exists when moving from CodeXL1.6 to CodeXL1.7 and opening the same project
    // or if the user moved some files. since we can reconstruct the overview file there is no
    // reason to fail. Theoretically we can remove this file altogether but it makes things simpler
    // and enable us future extensibility
    if (!filePath.exists())
    {
        // Get the parent item of the overview and take from it the file path
        afApplicationTree* pAppTree = afApplicationCommands::instance()->applicationTree();
        GT_IF_WITH_ASSERT(nullptr != pAppTree)
        {
            QTreeWidgetItem* pSelectedItem = pAppTree->getTreeSelection();
            GT_IF_WITH_ASSERT(nullptr != pSelectedItem)
            {
                QTreeWidgetItem* pSelectedParent = pAppTree->getTreeItemParent(pSelectedItem);
                GT_IF_WITH_ASSERT(nullptr != pSelectedParent)
                {
                    afApplicationTreeItemData* pItemData = pAppTree->getTreeItemData(pSelectedParent);

                    if (nullptr != pItemData)
                    {
                        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

                        if (nullptr != pKAData)
                        {
                            osFile overViewFile;
                            overViewFile.open(filePath, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
                            overViewFile.writeString(pKAData->filePath().asString());
                            overViewFile.close();
                        }
                    }
                }
            }
        }
    }
}
