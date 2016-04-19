//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afApplicationTree.cpp
///
//==================================================================================

// Include compiler warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <QMouseEvent>

// Infra:
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afHTMLContent.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>

afApplicationTreeItemData* afApplicationTree::m_pStaticEmptySelectedItem = new afApplicationTreeItemData;

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        afApplicationTree
/// \brief Description: Constructor
/// \param[in]          pProgressBar
/// \param[in]          pParent
/// \return
/// -----------------------------------------------------------------------------------------------
afApplicationTree::afApplicationTree(afProgressBarWrapper* pProgressBar, QWidget* pParent)
    : QWidget(pParent), afBaseView(pProgressBar), m_lastSelectedItemIndex(-1), m_pLastSelectedTreeItemId(nullptr), m_treatSelectAsActivate(false),
      m_pTreeCtrl(nullptr), m_pTopPanel(nullptr), m_pBackButton(nullptr), m_pForwardButton(nullptr),
      m_pHeaderItem(nullptr), m_pApplicationTreeItemData(nullptr), m_pDragItem(nullptr)
{

    // Set frame layout:
    setFrameLayout(pParent);

    // Register myself to listen to debugged process events:
    // The events priority is higher then the
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_FRAMEWORK_EVENTS_HANDLING_PRIORITY);

    // Reset the last selected item:
    resetLastSelectedItem();

    // Context menu:
    m_pTreeCtrl->setContextMenuPolicy(Qt::CustomContextMenu);


    // Disconnect the base class function and connect the tree virtual function for the tree context menu:
    bool rcConnect = m_pTreeCtrl->disconnect(SIGNAL(customContextMenuRequested(const QPoint&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(m_pTreeCtrl, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onContextMenuEvent(const QPoint&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(m_pTreeCtrl, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(onObjectTreeActivation(QTreeWidgetItem*, int)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(m_pTreeCtrl, SIGNAL(itemSelectionChanged()), this, SLOT(onItemSelectionChanged()));
    GT_ASSERT(rcConnect);

    // Update toolbar buttons:
    updateToolbarButtons();


}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::~afApplicationTree
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        23/9/2008
// ---------------------------------------------------------------------------
afApplicationTree::~afApplicationTree()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);

    // Delete the tree data:
    clearTreeItems(false);

    // Clear the selection list elements:
    m_selectionHistoryVector.deleteElementsAndClear();

}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        treeItemHasChildren
/// \brief Description: Does the tree item have children
/// \param[in]          pTreeItem
/// \return True :
/// \return False:
/// -----------------------------------------------------------------------------------------------
bool afApplicationTree::treeItemHasChildren(QTreeWidgetItem* pTreeItem) const
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(pTreeItem != nullptr)
    {
        retVal = (pTreeItem->childCount() > 0);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afApplicationTree::getTreeChildrenCount
// Description:
// Arguments:   QTreeWidgetItem* pTreeItem
//              bool recursively
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        1/11/2010
// ---------------------------------------------------------------------------
int afApplicationTree::getTreeChildrenCount(QTreeWidgetItem* pTreeItem, bool recursively) const
{
    GT_UNREFERENCED_PARAMETER(recursively);

    int retVal = 0;
    GT_IF_WITH_ASSERT(pTreeItem != nullptr)
    {
        retVal = pTreeItem->childCount();
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::getTreeItemData
// Description:
// Arguments:   QTreeWidgetItem* pTreeItem
// Return Val:  gdDebugApplicationTreeData*
// Author:      Sigal Algranaty
// Date:        1/11/2010
// ---------------------------------------------------------------------------
afApplicationTreeItemData* afApplicationTree::getTreeItemData(QTreeWidgetItem* pTreeItem) const
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
// Name:        afApplicationTree::getTreeRootItem
// Description:
// Return Val:  QTreeWidgetItem
// Author:      Sigal Algranaty
// Date:        1/11/2010
// ---------------------------------------------------------------------------
QTreeWidgetItem* afApplicationTree::getTreeRootItem() const
{
    QTreeWidgetItem* pRetVal = nullptr;
    GT_IF_WITH_ASSERT(m_pTreeCtrl != nullptr)
    {
        pRetVal = m_pTreeCtrl->headerItem();
    }
    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::getTreeItemParent
// Description:
// Arguments:   const QTreeWidgetItem* item
// Return Val:  QTreeWidgetItem
// Author:      Sigal Algranaty
// Date:        1/11/2010
// ---------------------------------------------------------------------------
QTreeWidgetItem* afApplicationTree::getTreeItemParent(const QTreeWidgetItem* pItem) const
{
    QTreeWidgetItem* pRetVal = nullptr;
    GT_IF_WITH_ASSERT(pItem != nullptr)
    {
        pRetVal = pItem->parent();
    }
    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::GetSelection
// Description:
// Return Val:  QTreeWidgetItem*
// Author:      Sigal Algranaty
// Date:        1/11/2010
// ---------------------------------------------------------------------------
QTreeWidgetItem* afApplicationTree::getTreeSelection() const
{
    QTreeWidgetItem* pRetVal = nullptr;
    GT_IF_WITH_ASSERT(m_pTreeCtrl != nullptr)
    {
        if (!m_pTreeCtrl->selectedItems().isEmpty())
        {
            pRetVal = m_pTreeCtrl->selectedItems().first();
        }
    }
    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::setFrameLayout
// Description: Builds the tree panel layout
// Arguments:   QWidget* pParent
// Author:      Sigal Algranaty
// Date:        8/8/2012
// ---------------------------------------------------------------------------
void afApplicationTree::setFrameLayout(QWidget* pParent)
{
    GT_UNREFERENCED_PARAMETER(pParent);

    // Create the top panel layout:
    createTopPanel();

    // Allocate the tree control:
    m_pTreeCtrl = new afTreeCtrl(this, 1, false);
    m_pTreeCtrl->SetAutoExpandOnSingleChild();
    QKeySequence seq("F2");
    // m_pTreeCtrl->grabShortcut(seq, Qt::ApplicationShortcut);

    // Hide the tree version header:
    m_pTreeCtrl->header()->hide();

    m_pApplicationRootPixmap = new QPixmap;
    acSetIconInPixmap(*m_pApplicationRootPixmap, AC_ICON_APPTREE_ROOT);

    // Allocate memory data for root:
    m_pApplicationTreeItemData = new afApplicationTreeItemData;
    m_pApplicationTreeItemData->m_isRoot = true;

    // Add header item:
    QStringList headerText;
    headerText << AF_STR_ApplicationTreeRootStopped;
    m_pHeaderItem = m_pTreeCtrl->addItem(headerText, m_pApplicationTreeItemData, nullptr, m_pApplicationRootPixmap);
    m_pApplicationTreeItemData->m_pTreeWidgetItem = m_pHeaderItem;

    // Create a sizer for me:
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->setContentsMargins(0, 0, 0, 0);

    m_pTreeCtrl->setContentsMargins(0, 0, 0, 0);

    // Add the controls to the sizer:
    pLayout->addWidget(m_pTopPanel, 0, Qt::AlignBottom);
    pLayout->addWidget(m_pTreeCtrl, 1);

    QPalette p = palette();
    p.setColor(m_pTreeCtrl->backgroundRole(), acGRAY_BG_COLOR);
    setAutoFillBackground(true);
    setPalette(p);

    // Set _pMainSizer as the dialog's main sizer:
    setLayout(pLayout);

    // Allow drag & drop:
    setAcceptDrops(true);

    // Allow multiple selection:
    m_pTreeCtrl->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_pTreeCtrl->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pTreeCtrl->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Connect to drag & drop signals
    bool rc = connect(m_pTreeCtrl, SIGNAL(DragAttempt(QTreeWidgetItem*, bool&)), this, SLOT(OnDragAttempt(QTreeWidgetItem*, bool&)));
    GT_ASSERT(rc);

    rc = connect(m_pTreeCtrl, SIGNAL(TreeElementDropEvent(QDropEvent*)), this, SLOT(OnTreeElementDropEvent(QDropEvent*)));
    GT_ASSERT(rc);

    rc = connect(m_pTreeCtrl, SIGNAL(TreeElementDragMoveEvent(QDragMoveEvent*)), this, SLOT(OnTreeElementDragMoveEvent(QDragMoveEvent*)));
    GT_ASSERT(rc);
}


// ---------------------------------------------------------------------------
// Name:        afApplicationTree::createTopPanel
// Description: Create the monitored tree top panel
// Author:      Sigal Algranaty
// Date:        21/2/2011
// ---------------------------------------------------------------------------
void afApplicationTree::createTopPanel()
{
    m_pTopPanel = new QWidget;

    // Create the icons for the back & forward buttons:
    QPixmap* pBackIcon = new QPixmap;
    acSetIconInPixmap(*pBackIcon, AC_ICON_APPTREE_BACK);
    QPixmap* pForwardIcon = new QPixmap;
    acSetIconInPixmap(*pForwardIcon, AC_ICON_APPTREE_FWD);
    unsigned int iconPixelSize = acIconSizeToPixelSize(acGetRecommendedIconSize());

    // Create the back and forward buttons:
    m_pBackButton = new QPushButton;
    m_pBackButton->setIcon(*pBackIcon);
    m_pBackButton->setFlat(true);
    m_pBackButton->resize(QSize(iconPixelSize, iconPixelSize));
    m_pBackButton->setMaximumWidth(iconPixelSize);
    m_pBackButton->setMaximumHeight(iconPixelSize);

    m_pForwardButton = new QPushButton;
    m_pForwardButton->setIcon(*pForwardIcon);
    m_pForwardButton->setFlat(true);
    m_pForwardButton->resize(QSize(iconPixelSize, iconPixelSize));
    m_pForwardButton->setMaximumWidth(iconPixelSize);
    m_pForwardButton->setMaximumHeight(iconPixelSize);

    // Connect both buttons to slots:
    bool rc = connect(m_pBackButton, SIGNAL(clicked()), this, SLOT(onBackTool()));
    GT_ASSERT(rc);
    rc = connect(m_pForwardButton, SIGNAL(clicked()), this, SLOT(onForwardTool()));
    GT_ASSERT(rc);

    QPalette p = m_pTopPanel->palette();
    p.setColor(m_pTopPanel->backgroundRole(), acGRAY_BG_COLOR);
    p.setColor(QPalette::Base, acGRAY_BG_COLOR);
    m_pTopPanel->setAutoFillBackground(true);
    m_pTopPanel->setPalette(p);

    QHBoxLayout* pTopLayout = new QHBoxLayout;

    // Set window icon:
    QPixmap iconPixMap;

    // Create the logo:
    QPixmap* pPixmap = new QPixmap();
    acSetIconInPixmap(*pPixmap, afGlobalVariablesManager::ProductIconID(), AC_16x16_ICON);

    QLabel* pLogoLabel = new QLabel;
    pLogoLabel->setPixmap(*pPixmap);

    // Build the top layout
    pTopLayout->addWidget(m_pBackButton, 0, Qt::AlignLeft | Qt::AlignBottom);
    pTopLayout->addWidget(m_pForwardButton, 0, Qt::AlignLeft | Qt::AlignBottom);
    pTopLayout->addStretch();
    pTopLayout->addWidget(pLogoLabel, 0, Qt::AlignRight);
    pTopLayout->setContentsMargins(6, 4, 6, 0);

    m_pTopPanel->setLayout(pTopLayout);
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::onEvent
// Description: Is called when a debugged process event occurs.
// Arguments:   const apEvent& eve - A class representing the event.
// Author:      Sigal Algranaty
// Date:        27/9/2010
// ---------------------------------------------------------------------------
void afApplicationTree::onEvent(const apEvent& eve, bool& vetoEvent)
{
    GT_UNREFERENCED_PARAMETER(vetoEvent);

    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // Update toolbar buttons on every event:
    updateToolbarButtons();

    switch (eventType)
    {

        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            // If there is a new project, set the root name:
            const afGlobalVariableChangedEvent& variableChangedEvent = (const afGlobalVariableChangedEvent&)eve;

            if (variableChangedEvent.changedVariableId() == afGlobalVariableChangedEvent::CURRENT_PROJECT)
            {
                if (m_curProjectName != afProjectManager::instance().currentProjectSettings().projectName())
                {
                    // Set the new project name:
                    m_curProjectName = afProjectManager::instance().currentProjectSettings().projectName();

                    // Set the tree root text:
                    updateTreeRootText();

                    // Reset the selection list:
                    resetLastSelectedItem();

                    // Update the back / forward buttons:
                    updateToolbarButtons();
                }
            }
        }
        break;

        case apEvent::AP_EXECUTION_MODE_CHANGED_EVENT:
        {
            onModeChanged();
        }
        break;

        default:
            // Do nothing...
            break;
    }

}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::onBackTool
// Description: The back tool was selected
// Arguments:
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        10/5/2009
// ---------------------------------------------------------------------------
void afApplicationTree::onBackTool()
{
    // Take step back:
    int stepBackIndex = m_lastSelectedItemIndex - 1;

    // Check if one step back is possible:
    if ((stepBackIndex >= 0) && (stepBackIndex < (int)m_selectionHistoryVector.size()))
    {
        // Get the viewer item for the step back:
        const afApplicationTreeItemSelection* pSelectionItem = getSelectedItem(stepBackIndex);
        GT_IF_WITH_ASSERT(pSelectionItem != nullptr)
        {
            // Turn on the flag that signals that we're processing step back or forward.
            // this flag is used to stop adding selected items:
            m_ignoreSelections = true;

            // Select the step back item:
            selectItem(pSelectionItem->_pItemData, pSelectionItem->_wasItemActivated);

            // Set the last selected item index:
            m_lastSelectedItemIndex = stepBackIndex;

            // Turn of flag:
            m_ignoreSelections = false;

            // Set tooltips for back and forward buttons:
            setTooltipsForNavigateButtons();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        afApplicationTree::onForwardTool
// Description: The forward tool was selected
// Author:      Sigal Algranaty
// Date:        10/5/2009
// ---------------------------------------------------------------------------
void afApplicationTree::onForwardTool()
{
    // Take step forward:
    int stepForwardIndex = m_lastSelectedItemIndex + 1;

    // Check if one step forward is possible:
    if ((stepForwardIndex >= 0) && (stepForwardIndex < (int)m_selectionHistoryVector.size()))
    {
        // Get the next step item data:
        const afApplicationTreeItemSelection* pSelectionItem = getSelectedItem(stepForwardIndex);
        GT_IF_WITH_ASSERT(pSelectionItem != nullptr)
        {
            // Turn on the flag that signals that we're processing step back or forward.
            // this flag is used to stop adding selected items:
            m_ignoreSelections = true;

            // Select the item:
            selectItem(pSelectionItem->_pItemData, pSelectionItem->_wasItemActivated);

            // Set the last selected item's index:
            m_lastSelectedItemIndex = stepForwardIndex;

            // Turn off flag:
            m_ignoreSelections = false;

            // Set tooltips for back and forward buttons:
            setTooltipsForNavigateButtons();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::updateBackCommand
// Description: Enable / Disable back command
// Author:      Sigal Algranaty
// Date:        10/5/2009
// ---------------------------------------------------------------------------
void afApplicationTree::updateBackCommand(bool& isEnabled)
{
    isEnabled = false;

    if (m_lastSelectedItemIndex > 0 && (m_selectionHistoryVector.size() > 1))
    {
        isEnabled = true;
    }

}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::updateForwardCommand
// Description: Enable / Disable forward command
// Author:      Sigal Algranaty
// Date:        10/5/2009
// ---------------------------------------------------------------------------
void afApplicationTree::updateForwardCommand(bool& isEnabled)
{
    isEnabled = false;

    if (m_lastSelectedItemIndex < (int)m_selectionHistoryVector.size() - 1)
    {
        isEnabled = true;
    }
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::getSelectedItem
// Description: Return the item selected in place index
// Arguments:   int index
// Return Val:  afApplicationTreeItemSelection*
// Author:      Sigal Algranaty
// Date:        11/5/2009
// ---------------------------------------------------------------------------
const afApplicationTree::afApplicationTreeItemSelection* afApplicationTree::getSelectedItem(int index) const
{
    afApplicationTreeItemSelection* pRetVal = nullptr;

    if ((index >= 0) && (index < (int)m_selectionHistoryVector.size()))
    {
        // Get the item in place index:
        pRetVal = m_selectionHistoryVector[index];
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        afApplicationTree::setTooltipsForNavigateButtons
// Description:
// Return Val: void
// Author:      Sigal Algranaty
// Date:        13/5/2009
// ---------------------------------------------------------------------------
void afApplicationTree::setTooltipsForNavigateButtons()
{
    // Calculate the back/forward items index:
    int backStepIndex = m_lastSelectedItemIndex - 1;
    int forwardStepIndex = m_lastSelectedItemIndex + 1;

    // Strings for the tooltips:
    QString backButtonTooltip = AF_STR_ApplicationTreeBackTooltip;
    QString forwardButtonTooltip = AF_STR_ApplicationTreeForwardTooltip;

    // Get the item data for each of the steps:
    const afApplicationTreeItemSelection* pBackSelection = getSelectedItem(backStepIndex);
    const afApplicationTreeItemSelection* pForwardSelection = getSelectedItem(forwardStepIndex);

    if (pBackSelection != nullptr)
    {
        // Get the original item data related to this item from tree:
        afApplicationTreeItemData* pOrigBackData = nullptr;
        bool rc = doesItemExist(pBackSelection->_pItemData, pOrigBackData);

        if (rc && (pOrigBackData != nullptr))
        {
            // Get the back item tree item id:
            QTreeWidgetItem* pBackTreeItemId = pOrigBackData->m_pTreeWidgetItem;

            if (pBackTreeItemId != nullptr)
            {
                // Build the tooltip string:
                QString tooltip = AF_STR_ApplicationTreeBackToTooltip;
                tooltip.append(pBackTreeItemId->text(0));

                // Set the tooltip:
                GT_IF_WITH_ASSERT(m_pBackButton != nullptr)
                {
                    m_pBackButton->setToolTip(tooltip);
                }
            }
        }
    }

    if (pForwardSelection != nullptr)
    {
        // Get the original item data related to this item from tree:
        afApplicationTreeItemData* pOrigForwardData = nullptr;
        bool rc = doesItemExist(pForwardSelection->_pItemData, pOrigForwardData);
        GT_IF_WITH_ASSERT(rc && (pOrigForwardData != nullptr))
        {
            // Get the back item tree item id:
            QTreeWidgetItem* pForwardTreeItemId = pOrigForwardData->m_pTreeWidgetItem;
            GT_IF_WITH_ASSERT(pForwardTreeItemId != nullptr)
            {
                // Build the tooltip string:
                QString tooltip = AF_STR_ApplicationTreeForwardToTooltip;
                tooltip.append(TreeItemText(pForwardTreeItemId));

                // Set the tooltip:
                GT_IF_WITH_ASSERT(m_pForwardButton != nullptr)
                {
                    m_pForwardButton->setToolTip(tooltip);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::doesItemExist
// Description: Browse the tree and look for the item, see if it exist according
//              to its item data
// Arguments:   QTreeWidgetItem rootTreeItemId
//              const gdTexturesAndBuffersItemData* pViewItem
//              afApplicationTreeItemData*& pOriginalItemData - output - the item
//              data for this tree item
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/5/2009
// ---------------------------------------------------------------------------
bool afApplicationTree::doesItemExist(const afApplicationTreeItemData* pSearchedForItem, afApplicationTreeItemData*& pOriginalItemData) const
{
    bool retVal = false;

    pOriginalItemData = nullptr;

    // Search the item recursively from the root:
    retVal = doesItemExist(m_pHeaderItem, pSearchedForItem, pOriginalItemData);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::doesItemExist
// Description: Recursive help function for browsing the tree
// Arguments:   QTreeWidgetItem rootTreeItemId
//              const gdTexturesAndBuffersItemData* pViewItem
//              gdTexturesAndBuffersItemData*& pOriginalItemData - output - the item
//              data for this tree item
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/5/2009
// ---------------------------------------------------------------------------
bool afApplicationTree::doesItemExist(QTreeWidgetItem* pRootTreeItemId, const afApplicationTreeItemData* pSearchedForItem, afApplicationTreeItemData*& pOriginalItemData) const
{
    bool retVal = false;

    // Sanity check:
    if (pRootTreeItemId != nullptr)
    {
        // First check if the root is the requested item:
        afApplicationTreeItemData* pViewerItem = getTreeItemData(pRootTreeItemId);
        GT_IF_WITH_ASSERT(pViewerItem != nullptr)
        {
            // Check if this the same item
            bool sameObject = pViewerItem->isSameObject(pSearchedForItem);

            if (sameObject)
            {
                retVal = true;
                pOriginalItemData = pViewerItem;
            }
            else
            {
                for (int i = 0; i < pRootTreeItemId->childCount(); i++)
                {
                    QTreeWidgetItem* pChild = pRootTreeItemId->child(i);

                    if (pChild != nullptr)
                    {
                        // Check this child:
                        retVal = doesItemExist(pChild, pSearchedForItem, pOriginalItemData);

                        if (retVal)
                        {
                            break;
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::treeUnselectAll
// Description: Clear the tree selection
// Author:      Sigal Algranaty
// Date:        1/11/2010
// ---------------------------------------------------------------------------
void afApplicationTree::treeUnselectAll()
{
    GT_IF_WITH_ASSERT(m_pTreeCtrl != nullptr)
    {
        m_pTreeCtrl->clearSelection();
    }
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::getTreeItemText
// Description: Get the text for the requested tree item
// Arguments:   QTreeWidgetItem* pTreeItem
// Return Val:  gtString
// Author:      Sigal Algranaty
// Date:        1/11/2010
// ---------------------------------------------------------------------------
gtString afApplicationTree::getTreeItemText(const QTreeWidgetItem* pTreeItem) const
{
    gtString retVal;
    GT_IF_WITH_ASSERT(pTreeItem != nullptr)
    {
        retVal.fromASCIIString(pTreeItem->text(0).toLatin1().data());
    }

    return retVal;
}
QString afApplicationTree::TreeItemText(const QTreeWidgetItem* pTreeItem) const
{
    QString retVal;
    GT_IF_WITH_ASSERT(pTreeItem != nullptr)
    {
        retVal = pTreeItem->text(0);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdTexturesAndBuffersViewer::resetLastSelectedItem
// Description: Reset the Last selected texture / buffer variable
// Author:      Eran Zinman
// Date:        18/11/2007
// ---------------------------------------------------------------------------
void afApplicationTree::resetLastSelectedItem()
{
    // Delete all items in the last selected items list:
    m_selectionHistoryVector.deleteElementsAndClear();
    m_lastSelectedItemIndex = -1;
    m_ignoreSelections = false;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::onUpdateEdit_Copy
// Description: enables the copy command in the VS edit menu
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        7/2/2011
// ---------------------------------------------------------------------------
void afApplicationTree::onUpdateEdit_Copy(bool& isEnabled)
{
    isEnabled = false;
}


// ---------------------------------------------------------------------------
// Name:        afApplicationTree::onUpdateEdit_SelectAll
// Description: enables the select all command in the VS edit menu
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        7/2/2011
// ---------------------------------------------------------------------------
void afApplicationTree::onUpdateEdit_SelectAll(bool& isEnabled)
{
    isEnabled = false;
}


// ---------------------------------------------------------------------------
// Name:        afApplicationTree::onEdit_Copy
// Description: execute the copy command in the VS edit menu
// Arguments:
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        7/2/2011
// ---------------------------------------------------------------------------
void afApplicationTree::onEdit_Copy()
{
    // does not support copy
}


// ---------------------------------------------------------------------------
// Name:        afApplicationTree::onEdit_SelectAll
// Description: execute the select command in the VS edit menu
// Arguments:
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        7/2/2011
// ---------------------------------------------------------------------------
void afApplicationTree::onEdit_SelectAll()
{
    // does not support select all
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::treeFocus
// Description: Set the tree in focus to get keyboard events
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        17/2/2011
// ---------------------------------------------------------------------------
void afApplicationTree::treeFocus()
{
    m_pTreeCtrl->setFocus();
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::onContextMenuEvent
// Description: is called when the user right clicks the tree
// Arguments:   const QPoint & - the context menu point
// Author:      Sigal Algranaty
// Date:        25/1/2011
// ---------------------------------------------------------------------------
void afApplicationTree::onContextMenuEvent(const QPoint& point)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTreeCtrl != nullptr)
    {
        // Get the item data list of the currently clicked items:
        gtList<const afApplicationTreeItemData*> listOfSelectedItemDatas;

        foreach (QTreeWidgetItem* pItem, m_pTreeCtrl->selectedItems())
        {
            const afApplicationTreeItemData* pClickedItemData = getTreeItemData(pItem);

            if (pClickedItemData != nullptr)
            {
                listOfSelectedItemDatas.push_back(pClickedItemData);
            }
        }

        // Build the context menu for this list of items:
        bool isContextMenuDisplayed = false;
        QMenu menu;

        for (int i = 0; i < (int)m_applicationTreeHandlers.size(); i++)
        {
            afApplicationTreeHandler* pTreeHandler = m_applicationTreeHandlers[i];

            if (pTreeHandler != nullptr)
            {
                isContextMenuDisplayed |= pTreeHandler->BuildContextMenuForItems(listOfSelectedItemDatas, menu);
            }
        }

        if (isContextMenuDisplayed)
        {
            // Display context menu:
            QPoint p = point;
            p.setY(p.y() + 25);
            menu.exec(acMapToGlobal(this, p));

            // handle post menu execution.
            // in some cases there might be a need to execute an action after the menu closes
            for (int i = 0; i < (int)m_applicationTreeHandlers.size(); i++)
            {
                afApplicationTreeHandler* pTreeHandler = m_applicationTreeHandlers[i];

                if (pTreeHandler != nullptr)
                {
                    pTreeHandler->PostContextMenuAction();
                }
            }

        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::updateToolbarButtons
// Description: Update the tree toolbar buttons
// Author:      Sigal Algranaty
// Date:        12/8/2012
// ---------------------------------------------------------------------------
void afApplicationTree::updateToolbarButtons()
{
    GT_IF_WITH_ASSERT((m_pBackButton != nullptr) && (m_pForwardButton != nullptr))
    {
        bool isEnabled = false;
        updateBackCommand(isEnabled);
        m_pBackButton->setEnabled(isEnabled);

        updateForwardCommand(isEnabled);
        m_pForwardButton->setEnabled(isEnabled);

    }
}


// ---------------------------------------------------------------------------
// Name:        afApplicationTree::clearTreeItemsData
// Description: Deletes all tree items data objects
// Arguments: QTreeWidgetItem* pTreeItem
// Return Val: void
// Author:      Sigal Algranaty
// Date:        20/11/2008
// ---------------------------------------------------------------------------
void afApplicationTree::clearTreeItemsData(QTreeWidgetItem* pTreeItem)
{
    m_ignoreSelections = true;

    // Check if this item is valid:
    if (pTreeItem != nullptr && (m_pTreeCtrl != nullptr))
    {
        // If the item has children, delete recursively the item's children's data:
        if (pTreeItem->childCount() > 0)
        {
            int count = pTreeItem->childCount();

            for (int i = 0 ; i < count; i++)
            {
                // Get the next child:
                QTreeWidgetItem* pChild = pTreeItem->child(i);

                if (pChild != nullptr)
                {
                    afApplicationTreeItemData* pTreeItemData = getTreeItemData(pTreeItem);
                    delete pTreeItemData;
                }
            }
        }

        // Delete this item data:
        QVariant itemData = pTreeItem->data(0, Qt::UserRole);
        afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)itemData.value<void*>();

        if (pItemData != nullptr)
        {
            // Delete me:
            delete pItemData;
            pTreeItem->setData(0, Qt::UserRole, 0);
        }
    }

    m_ignoreSelections = false;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        collapseAll
/// \brief Description: Collapses the tree items
/// \return void
/// -----------------------------------------------------------------------------------------------
void afApplicationTree::collapseAll()
{
    GT_IF_WITH_ASSERT(m_pTreeCtrl != nullptr)
    {
        m_pTreeCtrl->collapseAll();
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        expandItem
/// \brief Description: Expands the requested tree item
/// \param[in]          pItem
/// \return void
/// -----------------------------------------------------------------------------------------------
void afApplicationTree::expandItem(QTreeWidgetItem* pItem)
{
    GT_IF_WITH_ASSERT(m_pTreeCtrl != nullptr)
    {
        m_pTreeCtrl->expandItem(pItem);
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        clearSelection
/// \brief Description: Clear the tree selection
/// \return void
/// -----------------------------------------------------------------------------------------------
void afApplicationTree::clearSelection()
{
    GT_IF_WITH_ASSERT(m_pTreeCtrl != nullptr)
    {
        m_pTreeCtrl->clearSelection();
    }
}


// ---------------------------------------------------------------------------
// Name:        afApplicationTree::clearTreeItems
// Description: Clear all tree items
// Arguments:   bool addNonAvailableMessage - should add a message to the tree that
//              states that memory information is not available at the moment.
// Return Val: void
// Author:      Sigal Algranaty
// Date:        23/9/2008
// ---------------------------------------------------------------------------
void afApplicationTree::clearTreeItems(bool addNonAvailableMessage)
{
    m_ignoreSelections = true;
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pTreeCtrl != nullptr) && (m_pHeaderItem != nullptr))
    {
        // Notice: since we delete all items, and all item datas, we need to make sure that selection and activation
        // events will not be applied. Since we only apply one selection and activation event, we throw activation and selection events,
        // to make sure that other selection and activation event will be deleted from queue
        clearSelection();
        m_pTreeCtrl->setCurrentItem(nullptr);

        apMonitoredObjectsTreeActivatedEvent treeActivationEvent(nullptr);
        apEventsHandler::instance().registerPendingDebugEvent(treeActivationEvent);

        apMonitoredObjectsTreeSelectedEvent treeSelectionEvent(nullptr);
        apEventsHandler::instance().registerPendingDebugEvent(treeSelectionEvent);

        int count = m_pHeaderItem->childCount();

        for (int i = count - 1  ; i >= 0; i--)
        {
            // Get the next child:
            QTreeWidgetItem* pChild = m_pHeaderItem->child(i);

            if (pChild != nullptr)
            {
                // Check if this child should be removed.
                // A child will not be removed if he, or one of its children is "non-removable":
                bool shouldRemoveItem = true;
                afApplicationTreeItemData* pItemData = getTreeItemData(pChild);

                if (pItemData != nullptr)
                {
                    shouldRemoveItem = pItemData->m_isItemRemovable;
                }

                if (shouldRemoveItem)
                {
                    for (int j = 0; j < pChild->childCount(); j++)
                    {
                        QTreeWidgetItem* pGrandChild = pChild->child(j);

                        if (pGrandChild != nullptr)
                        {
                            pItemData = getTreeItemData(pGrandChild);

                            if (pItemData != nullptr)
                            {
                                if (!pItemData->m_isItemRemovable)
                                {
                                    shouldRemoveItem = false;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Add a message to the tree:
        if (addNonAvailableMessage)
        {
            updateTreeRootText();
        }
    }

    m_ignoreSelections = false;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::selectItem
// Description: Select an item in the objects explorer tree
// Arguments:   pViewerItem - The item to select
//              gdMonitoredObjectType& selectedItemType - output. If selected item was not found
//              this is the type of the selected item
// Author:      Sigal Algranaty
// Date:        31/10/2010
// ---------------------------------------------------------------------------
bool afApplicationTree::selectItem(const afApplicationTreeItemData* pTreeItemData, bool shouldActivate)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((m_pTreeCtrl != nullptr) && (pTreeItemData != nullptr))
    {
        // Clear selection from the explorer:
        treeUnselectAll();
        m_pLastSelectedTreeItemId = nullptr;

        // Will contain the tree item id for the selected item:
        afApplicationTreeItemData* pTreeItemDataForSelection = nullptr;

        // Get the viewer item data by its id:
        pTreeItemDataForSelection = findMatchingTreeItem(*pTreeItemData, true);

        // If the requested item data was found:
        GT_IF_WITH_ASSERT(pTreeItemDataForSelection != nullptr)
        {
            GT_IF_WITH_ASSERT(pTreeItemDataForSelection->m_pTreeWidgetItem != nullptr)
            {
                // Select the item:
                pTreeItemDataForSelection->m_pTreeWidgetItem->setSelected(true);
                m_pTreeCtrl->setCurrentItem(pTreeItemDataForSelection->m_pTreeWidgetItem);

                // Activate the item if requested:
                if (shouldActivate)
                {
                    onObjectTreeActivation(pTreeItemDataForSelection->m_pTreeWidgetItem, 0);
                }

                retVal = true;
            }
        }
    }
    return retVal;
}


afApplicationTreeItemData* afApplicationTree::FindItemByFilePath(const osFilePath& filePath)
{
    afApplicationTreeItemData* pRetVal = nullptr;

    QTreeWidgetItemIterator treeIter(m_pTreeCtrl);

    while (*treeIter)
    {
        QTreeWidgetItem* pCurrentItem = (*treeIter);
        afApplicationTreeItemData* pItemData = getTreeItemData(pCurrentItem);

        if (pItemData != nullptr)
        {
            if (pItemData->m_filePath == filePath)
            {
                pRetVal = pItemData;
                break;
            }
        }

        ++treeIter;
    }

    return pRetVal;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        onObjectTreeActivation
/// \brief Description: Is responding to tree activation. Emitting a tree activation event
/// \param[in]          pActivated
/// \param[in]          column
/// \return void
/// -----------------------------------------------------------------------------------------------
void afApplicationTree::onObjectTreeActivation(QTreeWidgetItem* pActivated, int column)
{
    Q_UNUSED(column);
    Q_UNUSED(pActivated);

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTreeCtrl != nullptr)
    {
        // Go through the selected items and activate each of them:
        foreach (QTreeWidgetItem* pSelectedItem, m_pTreeCtrl->selectedItems())
        {
            // Get the tree item data:
            afApplicationTreeItemData* pItemData = getTreeItemData(pSelectedItem);

            if (pItemData != nullptr)
            {
#pragma message ("TODO: File open cleanup : Add the option to block file open (for example: new session while other process is running")

                if (!pItemData->m_filePath.isEmpty())
                {
                    afApplicationCommands::instance()->OpenFileAtLine(pItemData->m_filePath, pItemData->m_filePathLineNumber, -1, (int)pItemData->m_itemType);
                }
                else
                {
                    // Create an event handling the tree selection:
                    apMonitoredObjectsTreeActivatedEvent treeItemEvent(pItemData);

                    // Register the event:
                    apEventsHandler::instance().registerPendingDebugEvent(treeItemEvent);
                }

                // Notice: If activation failed, and m_treatSelectAsActivate is on, this leads to an infinite loop,
                // so do not call selection as default:
                if (!m_treatSelectAsActivate)
                {
                    // Item cannot be activated, simply select it:
                    onItemSelectionChanged();
                }
            }

            // Add the item to the list of selected items:
            addSelectedItem(pItemData, true);
        }

        // Update the toolbar buttons:
        updateToolbarButtons();
    }
}


// ---------------------------------------------------------------------------
// Name:        afApplicationTree::onItemSelectionChanged
// Description: Handles the selection of objects in CodeXL explorer
// Arguments:   QTreeWidgetItem* pSelected,
//              int column
// Author:      Sigal Algranaty
// Date:        11/10/2010
// ---------------------------------------------------------------------------
void afApplicationTree::onItemSelectionChanged()
{
    if (!m_ignoreSelections)
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(m_pTreeCtrl != nullptr)
        {
            // Check if multiple items are selected:
            QTreeWidgetItem* pFirstSelectedItem = m_pTreeCtrl->selectedItems().isEmpty() ? nullptr : m_pTreeCtrl->selectedItems().first();
            bool areMultipleItemsSelected = (m_pTreeCtrl->selectedItems().size() > 1);

            if (pFirstSelectedItem != nullptr)
            {
                // Get the tree item data:
                afApplicationTreeItemData* pItemData = getTreeItemData(pFirstSelectedItem);

                if (pItemData != nullptr)
                {
                    if (!areMultipleItemsSelected)
                    {
                        // Create an event handling the tree selection:
                        apMonitoredObjectsTreeSelectedEvent treeItemEvent(pItemData);

                        // Register the event:
                        apEventsHandler::instance().registerPendingDebugEvent(treeItemEvent);

                        // Display the selected item properties:
                        DisplayItemProperties(pItemData);
                    }
                    else
                    {
                        // Set an HTML properties string describing multiple selection:
                        afHTMLContent htmlContent(AF_STR_HtmlMultipleItemsCaption);
                        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_HtmlMultipleItems);
                        gtString htmlText;
                        htmlContent.toString(htmlText);
                        QString propsStr = acGTStringToQString(htmlText);
                        afApplicationCommands::instance()->propertiesView()->setHTMLText(propsStr, nullptr);
                    }

                    // Add the item to the list of selected items:
                    addSelectedItem(pItemData, false);
                }

                if (m_treatSelectAsActivate)
                {
                    onObjectTreeActivation(pFirstSelectedItem, 0);
                }

                // Update the toolbar buttons:
                updateToolbarButtons();
            }
        }
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        addTreeItem
/// \brief Description: Add an item to the tree
/// \param[in]          itemText
/// \param[in]          pItemData
/// \param[in]          pParent
/// \return afApplicationTreeItemData*
/// -----------------------------------------------------------------------------------------------
QTreeWidgetItem* afApplicationTree::addTreeItem(const gtString& itemText, afApplicationTreeItemData* pItemData, QTreeWidgetItem* pParent)
{
    QTreeWidgetItem* pRetVal = nullptr;
    QStringList stringList;
    QString qitemText = acGTStringToQString(itemText);
    stringList << qitemText;
    GT_IF_WITH_ASSERT(m_pTreeCtrl != nullptr)
    {
        pRetVal = m_pTreeCtrl->addItem(stringList, pItemData, pParent);
        pRetVal->setToolTip(0, qitemText);
    }

    if (pItemData != nullptr)
    {
        pItemData->m_pTreeWidgetItem = pRetVal;
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        afApplicationTree::insertTreeItem
// Description: insert a tree item after a specific item
// Return Val:  QTreeWidgetItem*
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
QTreeWidgetItem* afApplicationTree::insertTreeItem(const gtString& itemText, afApplicationTreeItemData* pItemData, QTreeWidgetItem* pParent, QTreeWidgetItem* pBeforeItem)
{
    QTreeWidgetItem* pRetVal = nullptr;
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTreeCtrl != nullptr)
    {
        int index = 0;

        // Add the child at the end of the list of children:
        if (pParent != nullptr)
        {
            index = pParent->childCount();
        }

        if ((pParent != nullptr) && (pBeforeItem != nullptr))
        {
            index = pParent->indexOfChild(pBeforeItem);

            // if child is not found do not use the -1 index that might cause problems, insert the item as first
            if (-1 == index)
            {
                GT_ASSERT_EX(false, L"Child not found");
                index = 0;
            }
        }

        QStringList stringList;
        QString qitemText = acGTStringToQString(itemText);
        stringList << qitemText;
        GT_IF_WITH_ASSERT(m_pTreeCtrl != nullptr)
        {
            pRetVal = m_pTreeCtrl->insertItem(stringList, pItemData, index, pParent);
            pRetVal->setToolTip(0, qitemText);
        }

        if (pItemData != nullptr)
        {
            pItemData->m_pTreeWidgetItem = pRetVal;
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::addSelectedItem
// Description: Return the last selected item
// Argument:    gdDebugApplicationTreeData* pViewerItem - the item data to add
// Return Val: gdDebugApplicationTreeData*
// Author:      Sigal Algranaty
// Date:        11/5/2009
// ---------------------------------------------------------------------------
void afApplicationTree::addSelectedItem(afApplicationTreeItemData* pViewerItem, bool wasItemActivated)
{
    // Do not add selected item while processing back and forward events:
    if (!m_ignoreSelections)
    {
        if (pViewerItem == nullptr)
        {
            // Construct a new item data for the root object:
            pViewerItem = new afApplicationTreeItemData;
            pViewerItem->m_itemType = AF_TREE_ITEM_APP_ROOT;
        }

        bool isSameObject = false;

        // Get the currently selected item data:
        afApplicationTreeItemSelection* pCurrentlySelectionItem = (afApplicationTreeItemSelection*)getSelectedItem(m_lastSelectedItemIndex);

        if (pCurrentlySelectionItem != nullptr)
        {
            // Do not add multiple copies of the same item data:
            if (pCurrentlySelectionItem->_pItemData != pViewerItem)
            {
                // Check if the item data represent the same object:
                isSameObject = pViewerItem->isSameObject(pCurrentlySelectionItem->_pItemData);
            }
        }

        if (!isSameObject)
        {
            // Create a new item data:
            afApplicationTreeItemData* pNewItemData = new afApplicationTreeItemData;
            pViewerItem->copyID(*pNewItemData);
            pNewItemData->m_pTreeWidgetItem = pViewerItem->m_pTreeWidgetItem;

            bool itemRemoved = false;

            // If the selection is pointing to the middle of the vector, pop all the items on top of it:
            if (m_lastSelectedItemIndex < (int)(m_selectionHistoryVector.size() - 1))
            {
                int amountOfItemsToPop = (int)m_selectionHistoryVector.size() - 1 - m_lastSelectedItemIndex;

                for (int i = 0; i < amountOfItemsToPop; i++)
                {
                    m_selectionHistoryVector.pop_back();
                }

                itemRemoved = true;
            }

            // Push the new item to the vector:
            afApplicationTreeItemSelection* pNewSelectionItem = new afApplicationTreeItemSelection(pNewItemData, wasItemActivated);
            m_selectionHistoryVector.push_back(pNewSelectionItem);

            m_lastSelectedItemIndex ++;

            if (itemRemoved)
            {
                m_lastSelectedItemIndex = m_selectionHistoryVector.size() - 1;
            }
        }
        else
        {
            // If it is the same object, and it was selected, and now it is activated, activate it:
            if ((!pCurrentlySelectionItem->_wasItemActivated) && wasItemActivated)
            {
                pCurrentlySelectionItem->_wasItemActivated = true;
            }
        }


        // Set tooltips for back and forward buttons:
        setTooltipsForNavigateButtons();
    }
}


// ---------------------------------------------------------------------------
// Name:        afApplicationTree::getCurrentlySelectedItemData
// Description: Return the currently selected item data
// Return Val:  const gdDebugApplicationTreeData*
// Author:      Sigal Algranaty
// Date:        2/11/2010
// ---------------------------------------------------------------------------
const afApplicationTreeItemData* afApplicationTree::getCurrentlySelectedItemData() const
{
    const afApplicationTreeItemData* pRetVal = nullptr;

    // Check if the last selected item is within the valid range:
    if ((m_lastSelectedItemIndex >= 0) && (m_lastSelectedItemIndex < (int)m_selectionHistoryVector.size()))
    {
        const afApplicationTreeItemSelection* pCurrentSelection = getSelectedItem(m_lastSelectedItemIndex);

        if (pCurrentSelection != nullptr)
        {
            pRetVal = pCurrentSelection->_pItemData;
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::setNewSelection
// Description: Sets the tree new selection
// Arguments:   QTreeWidgetItem* pSelectedTreeItem - the new selected item id
//              bool& wasSelectionChanged - output - true iff selection is changed since last selection
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        2/11/2010
// ---------------------------------------------------------------------------
void afApplicationTree::setNewSelection(QTreeWidgetItem* pSelectedTreeItem, bool& wasSelectionChanged)
{
    wasSelectionChanged = (m_pLastSelectedTreeItemId != pSelectedTreeItem);
    m_pLastSelectedTreeItemId = pSelectedTreeItem;
}

void afApplicationTree::clearNonExistingItemsFromSelection()
{
    gtPtrVector<afApplicationTreeItemSelection*> newSelectionHistoryVector;

    // Go over each of the current selection history items:
    for (int i = 0 ; i < (int)m_selectionHistoryVector.size(); i++)
    {
        // Get the currently selected item data:
        afApplicationTreeItemSelection* pSelectionItem = m_selectionHistoryVector[i];

        if (pSelectionItem != nullptr)
        {
            GT_IF_WITH_ASSERT(pSelectionItem->_pItemData != nullptr)
            {
                bool itemExist = false;
                afApplicationTreeItemData* pDummyData = nullptr;

                // Search for the item in the buffers tree:
                itemExist = doesItemExist(pSelectionItem->_pItemData, pDummyData);

                // If the item exist, keep it in the temporary vector:
                if (itemExist)
                {
                    newSelectionHistoryVector.push_back(pSelectionItem);
                }
                else
                {
                    // Decrease the last selection indices if necessary:
                    if (i <= m_lastSelectedItemIndex)
                    {
                        m_lastSelectedItemIndex--;
                    }

                    if ((m_lastSelectedItemIndex < 0) && (newSelectionHistoryVector.size() > 0))
                    {
                        m_lastSelectedItemIndex = 0;
                    }
                }
            }
        }
    }

    // Copy the temporary list to the real one:
    if (newSelectionHistoryVector.size() != m_selectionHistoryVector.size())
    {
        m_selectionHistoryVector.clear();

        for (int i = 0 ; i < (int)newSelectionHistoryVector.size(); i++)
        {
            // Get the currently selected item data:
            afApplicationTreeItemSelection* pSelectionItem = newSelectionHistoryVector[i];
            m_selectionHistoryVector.push_back(pSelectionItem);
        }

    }

    newSelectionHistoryVector.clear();
}

void afApplicationTree::clearTreeItemFromSelectionHistoryList(QTreeWidgetItem* pItemToBeRemoved)
{
    if (pItemToBeRemoved != nullptr)
    {
        // Go over the list, and create a list the excludes one of the two:
        // 1. The index of the item itself
        // 2. The indices of any descendant item of this item:
        gtPtrVector<afApplicationTreeItemSelection*> newSelectionHistoryVector;

        for (int i = 0 ; i < (int)m_selectionHistoryVector.size(); i++)
        {
            // Get the currently selected item data:
            afApplicationTreeItemSelection* pSelectionItem = m_selectionHistoryVector[i];

            if (pSelectionItem != nullptr)
            {
                GT_IF_WITH_ASSERT(pSelectionItem->_pItemData != nullptr)
                {
                    bool shouldItemBeRemoved = (pSelectionItem->_pItemData->m_pTreeWidgetItem == pItemToBeRemoved);

                    if (!shouldItemBeRemoved)
                    {
                        shouldItemBeRemoved  = m_pTreeCtrl->isAncestor(pSelectionItem->_pItemData->m_pTreeWidgetItem, pItemToBeRemoved);
                    }

                    // If an item does not exist, delete it from the selected items list:
                    if (!shouldItemBeRemoved)

                    {
                        newSelectionHistoryVector.push_back(m_selectionHistoryVector[i]);
                    }
                    else
                    {
                        if (i <= m_lastSelectedItemIndex)
                        {
                            m_lastSelectedItemIndex--;
                        }

                        if ((m_lastSelectedItemIndex < 0) && (newSelectionHistoryVector.size() > 0))
                        {
                            m_lastSelectedItemIndex = 0;
                        }
                    }
                }
            }
        }

        if (newSelectionHistoryVector.size() != m_selectionHistoryVector.size())
        {
            m_selectionHistoryVector.clear();

            for (int i = 0 ; i < (int)newSelectionHistoryVector.size(); i++)
            {
                // Get the currently selected item data:
                afApplicationTreeItemSelection* pSelectionItem = newSelectionHistoryVector[i];
                m_selectionHistoryVector.push_back(pSelectionItem);
            }

        }

        newSelectionHistoryVector.clear();
    }
}


/// -----------------------------------------------------------------------------------------------
/// \brief Name:        treeSelectItem
/// \brief Description: Select the requested tree item
/// \param[in]          pTreeItem
/// \param[in]          select
/// \return void
/// -----------------------------------------------------------------------------------------------
void afApplicationTree::treeSelectItem(const QTreeWidgetItem* pTreeItem, bool select)
{
    GT_IF_WITH_ASSERT((m_pTreeCtrl != nullptr) && (pTreeItem != nullptr))
    {
        m_pTreeCtrl->setItemSelected(pTreeItem, select);
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        updateTreeRootText
/// \brief Description: Sets the tree root text according to the current application state
/// Format of the root string:
///                             ProjectName | Debug Mode (Running)
/// \return void
/// -----------------------------------------------------------------------------------------------
void afApplicationTree::updateTreeRootText()
{
    // If the project had changed, clear the root and update the navigation buttons:
    gtString newCurProjectName = afProjectManager::instance().currentProjectSettings().projectName();

    if (m_curProjectName != newCurProjectName)
    {
        // Clear the tree and update it with the new project details:
        clearTreeItems(false);

        // Reset the selection list:
        resetLastSelectedItem();

        // Update the back / forward buttons:
        updateToolbarButtons();
    }

    m_curProjectName = newCurProjectName;

    afPluginConnectionManager& thePluginConnectionManager = afPluginConnectionManager::instance();
    bool doesProcessExist = thePluginConnectionManager.getCurrentRunModeMask() & AF_DEBUGGED_PROCESS_EXISTS;
    bool isProcessSuspended = thePluginConnectionManager.getCurrentRunModeMask() & AF_DEBUGGED_PROCESS_SUSPENDED;


    // Build the root string:
    QString rootString;

    if (m_curProjectName.isEmpty())
    {
        rootString = AF_STR_ApplicationTreeRootStopped;
    }
    else
    {
        // Set the tree root string according to execution mode:
        QString executionStatusString, executionModeString;
        executionStatusString = " - " AF_STR_ApplicationTreeNotRunning;

        // Build the root string:
        if (doesProcessExist)
        {
            // Find the current execution status string:
            executionStatusString = (isProcessSuspended) ? "" : " " AF_STR_ApplicationTreeRootRunning;
        }

        afIExecutionMode* pActiveMode = afExecutionModeManager::instance().activeMode();
        int activeSessionType = afExecutionModeManager::instance().activeSessionType();

        if (pActiveMode != nullptr)
        {
            QString modeString = acGTStringToQString(pActiveMode->modeName());
            QString sessionTypeName = acGTStringToQString(pActiveMode->sessionTypeName(activeSessionType));

            // check if it is a simple session name: only the word "debug" with out really checking the word since it is unknown here
            if (sessionTypeName.indexOf(' ') < 0 || modeString.contains(sessionTypeName))
            {
                executionModeString = QString(AF_STR_TitleModeFormatA).arg(modeString);
            }
            else
            {
                executionModeString = QString(AF_STR_TitleModeSessionFormatA).arg(modeString).arg(sessionTypeName);
            }
        }


        rootString = acGTStringToQString(m_curProjectName);

        if (!executionModeString.isEmpty())
        {
            rootString.append(" ");
            rootString.append(executionModeString);
        }

        if (pActiveMode->IsExecutionStatusRelevant())
        {
            rootString.append(executionStatusString);
        }

    }

    // Set the tree item text:
    setTreeItemText(m_pHeaderItem, rootString);

    // Set the full path as tooltip for the root:
    QString tooltip = acGTStringToQString(afProjectManager::instance().currentProjectSettings().executablePath().asString());
    m_pHeaderItem->setToolTip(0, tooltip);
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTree::setTreeItemText
// Description: Set the text for the requested tree item
// Arguments:   QTreeWidgetItem* pTreeItem
//              const gtString& itemText
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        12/1/2011
// ---------------------------------------------------------------------------
void afApplicationTree::setTreeItemText(QTreeWidgetItem* pTreeItem, const QString& itemText)
{
    GT_IF_WITH_ASSERT(pTreeItem != nullptr)
    {
        pTreeItem->setText(0, itemText);
        pTreeItem->setToolTip(0, itemText);
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        topLevelItemCount
/// \brief Description: Return the tree top level item count
/// \return int
/// -----------------------------------------------------------------------------------------------
int afApplicationTree::topLevelItemCount() const
{
    int retVal = 0;
    GT_IF_WITH_ASSERT(m_pTreeCtrl != nullptr)
    {
        retVal = m_pTreeCtrl->topLevelItemCount();
    }
    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        topLevelItem
/// \brief Description: Return the tree top level item
/// \param[in]          index
/// \return QTreeWidgetItem
/// -----------------------------------------------------------------------------------------------
QTreeWidgetItem* afApplicationTree::topLevelItem(int index) const
{
    QTreeWidgetItem* pRetVal = nullptr;
    GT_IF_WITH_ASSERT(m_pTreeCtrl != nullptr)
    {
        pRetVal = m_pTreeCtrl->topLevelItem(index);
    }
    return pRetVal;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        scrollToItem
/// \brief Description: Wraps the tree scrollToItem method
/// \param[in]          pItem
/// \param[in]          hint
/// \return void
/// -----------------------------------------------------------------------------------------------
void afApplicationTree::scrollToItem(const QTreeWidgetItem* pItem, QAbstractItemView::ScrollHint hint)
{
    GT_IF_WITH_ASSERT(m_pTreeCtrl != nullptr)
    {
        m_pTreeCtrl->scrollToItem(pItem, hint);
    }
}


/// -----------------------------------------------------------------------------------------------
/// \brief Name:        findMatchingTreeItem
/// \brief Description: Wraps the tree scrollToItem method
/// \param[in]          displayedItemId - the looked for item
/// \param[in]          getRootItemDataByDefault - true iff the root should be returned if the item is not found
/// \return void
/// -----------------------------------------------------------------------------------------------
afApplicationTreeItemData* afApplicationTree::findMatchingTreeItem(const afApplicationTreeItemData& displayedItemId, bool getRootItemDataByDefault)
{
    GT_UNREFERENCED_PARAMETER(getRootItemDataByDefault);

    afApplicationTreeItemData* pRetVal = nullptr;

    for (int i = 0; i < (int)m_applicationTreeHandlers.size(); i++)
    {
        afApplicationTreeHandler* pTreeHandler = m_applicationTreeHandlers[i];

        if (pTreeHandler != nullptr)
        {
            pRetVal = pTreeHandler->FindMatchingTreeItem(displayedItemId);

            if (pRetVal != nullptr)
            {
                break;
            }
        }
    }

    if (pRetVal == nullptr)
    {
        pRetVal = getTreeItemData(m_pHeaderItem);
    }

    return pRetVal;
}


/// -----------------------------------------------------------------------------------------------
/// \brief Name:        editCurrentItem
/// \brief Description: Edits the current selected item
/// \return void
/// -----------------------------------------------------------------------------------------------
void afApplicationTree::editCurrentItem()
{
    GT_IF_WITH_ASSERT(m_pTreeCtrl != nullptr)
    {
        QTreeWidgetItem* pCurrentItem = m_pTreeCtrl->currentItem();

        if (nullptr != pCurrentItem)
        {
            Qt::ItemFlags itemFlags = pCurrentItem->flags();

            if (itemFlags & Qt::ItemIsEditable)
            {
                m_pTreeCtrl->editItem(pCurrentItem);
                pCurrentItem->setFlags(itemFlags);
                QLineEdit* pLineEdit = m_pTreeCtrl->lineEditor();
                GT_IF_WITH_ASSERT(nullptr != pLineEdit)
                {
                    emit EditorStarted(pLineEdit);
                }
            }
        }
    }
}


/// -----------------------------------------------------------------------------------------------
/// \brief Name:        ShouldAcceptDragDrop
/// \brief Description: Returns true if currently the tree accepts drag an drop
/// \param[in]          pEvent
/// -----------------------------------------------------------------------------------------------
bool afApplicationTree::ShouldAcceptDragDrop(QDropEvent* pEvent, QString& dragDropFile)
{
    bool retVal = false;

    for (int i = 0; i < (int)m_applicationTreeHandlers.size(); i++)
    {
        afApplicationTreeHandler* pTreeHandler = m_applicationTreeHandlers[i];

        if (pTreeHandler != nullptr)
        {
            bool shouldAccept = false;
            bool isHandling = pTreeHandler->IsDragDropSupported(pEvent, dragDropFile, shouldAccept);

            if (isHandling)
            {
                retVal = shouldAccept;

                if (shouldAccept)
                {
                    break;
                }
            }
        }
    }

    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        ShouldAcceptDragDrop
/// \brief Description: Returns true if currently the tree accepts drag an drop
/// \param[in]          pEvent
/// -----------------------------------------------------------------------------------------------
bool afApplicationTree::ExecuteDropEvent(QDropEvent* pEvent, QString& dragDropFile)
{
    bool retVal = false;

    for (int i = 0; i < (int)m_applicationTreeHandlers.size(); i++)
    {
        afApplicationTreeHandler* pTreeHandler = m_applicationTreeHandlers[i];

        if (pTreeHandler != nullptr)
        {
            bool shouldAccept = false;
            bool isHandling = pTreeHandler->IsDragDropSupported(pEvent, dragDropFile, shouldAccept);

            if (isHandling && shouldAccept)
            {
                retVal = pTreeHandler->ExecuteDropEvent(pEvent, dragDropFile);
                break;
            }
        }
    }

    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        onModeChanged
/// \brief Description: Is handling execution mode changed event
/// \return void
/// -----------------------------------------------------------------------------------------------
void afApplicationTree::onModeChanged()
{
    for (int i = 0; i < (int)m_applicationTreeHandlers.size(); i++)
    {
        afApplicationTreeHandler* pTreeHandler = m_applicationTreeHandlers[i];

        if (pTreeHandler != nullptr)
        {
            pTreeHandler->SetItemsVisibility();
        }
    }

    // Reset the selection list:
    resetLastSelectedItem();

    // Update the back / forward buttons:
    updateToolbarButtons();

    // Update the root text:
    updateTreeRootText();
}

void afApplicationTree::removeTreeItem(QTreeWidgetItem* pTreeWidgetItem, bool removeParentIfEmpty)
{
    // Block signals:
    blockSignals(true);

    m_ignoreSelections = true;

    // Remove tree node:

    GT_IF_WITH_ASSERT(pTreeWidgetItem != nullptr)
    {
        QTreeWidgetItem* pItemParent = pTreeWidgetItem->parent();
        GT_IF_WITH_ASSERT(pItemParent != nullptr)
        {
            clearTreeItemFromSelectionHistoryList(pTreeWidgetItem);

            // Get the tree item data for later deletion
            afApplicationTreeItemData* pItemData = getTreeItemData(pTreeWidgetItem);

            pItemParent->removeChild(pTreeWidgetItem);
            delete pItemData;

            if (removeParentIfEmpty)
            {
                if (pItemParent->childCount() == 0)
                {
                    QTreeWidgetItem* pItemParentParent = pItemParent->parent();
                    GT_IF_WITH_ASSERT(pItemParentParent != nullptr)
                    {
                        QTreeWidgetItem* pItemParentParentParent = pItemParentParent->parent();
                        clearTreeItemFromSelectionHistoryList(pItemParent);

                        pItemData = getTreeItemData(pItemParent);
                        pItemParentParent->removeChild(pItemParent);
                        delete pItemData;

                        if (pItemParentParentParent != nullptr)
                        {
                            if (pItemParentParent->childCount() == 0)
                            {
                                clearTreeItemFromSelectionHistoryList(pItemParentParent);

                                pItemData = getTreeItemData(pItemParentParent);
                                pItemParentParent->removeChild(pItemParentParent);
                                delete pItemData;
                            }
                        }
                    }
                }
            }
        }
    }

    clearNonExistingItemsFromSelection();

    // Unblock signals:
    blockSignals(false);

    m_ignoreSelections = false;
}

void afApplicationTree::debugPrintHistoryList()
{
    // TO_DO: Commit only commented - used for debugging of the navigation toolbar:
    gtString dbg = L"Selection list\n";

    for (int i = 0 ; i < (int)m_selectionHistoryVector.size(); i++)
    {
        const afApplicationTreeItemSelection* pCurrentSelection = getSelectedItem(i);

        if (pCurrentSelection != nullptr)
        {
            GT_IF_WITH_ASSERT(pCurrentSelection->_pItemData != nullptr)
            {
                gtString itemId;
                afApplicationTreeItemData::itemTypeAsString(pCurrentSelection->_pItemData->m_itemType, itemId);
                dbg.appendFormattedString(L"Item %d: %ls. Should Activate: %d\n", i, itemId.asCharArray(), pCurrentSelection->_wasItemActivated);
                GT_ASSERT_EX(false, L"Do not submit me!");
            }
        }
    }

    osOutputDebugString(dbg);
}

void afApplicationTree::DisplayItemProperties(const afApplicationTreeItemData* pItemData)
{
    if (pItemData != nullptr)
    {
        // Look for the tree handler displaying this item:
        afHTMLContent htmlContent;
        bool arePropsSet = false;
        gtString htmlText;

        if (pItemData->m_itemType == AF_TREE_ITEM_APP_ROOT)
        {
            // Build the application root item string:
            htmlContent.setTitle(afProjectManager::instance().currentProjectSettings().projectName());
            afIExecutionMode* pActiveMode = afExecutionModeManager::instance().activeMode();
            GT_IF_WITH_ASSERT(pActiveMode != nullptr)
            {
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_PropertiesExecutionMode, pActiveMode->modeName());
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, pActiveMode->HowToStartModeExecutionMessage());
                arePropsSet = true;
            }
        }
        else if (AF_TREE_ITEM_MESSAGE == pItemData->m_itemType)
        {
            afApplicationTreeMessageItemData* pMessageData = qobject_cast<afApplicationTreeMessageItemData*>(pItemData->extendedItemData());
            GT_IF_WITH_ASSERT(nullptr != pMessageData)
            {
                const gtString& messageTitle = pMessageData->m_messageTitle;
                static const gtString defaultTitle = AF_STR_Message;
                htmlContent.setTitle(messageTitle.isEmpty() ? defaultTitle : messageTitle);
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, pMessageData->m_messageText);
                arePropsSet = true;
            }
        }

        for (int i = 0; i < (int)m_applicationTreeHandlers.size() && !arePropsSet; i++)
        {
            afApplicationTreeHandler* pTreeHandler = m_applicationTreeHandlers[i];

            if (pTreeHandler != nullptr)
            {
                // Clear the HTML content:
                htmlContent.clear();

                // Build the current content:
                arePropsSet = pTreeHandler->BuildItemHTMLPropeties(*pItemData, htmlContent);
            }
        }

        if (arePropsSet)
        {
            // Display in properties view:
            htmlContent.toString(htmlText);
            afApplicationCommands* pAppCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(pAppCommands != nullptr)
            {
                afPropertiesView* pPropertiesView = pAppCommands->propertiesView();
                GT_IF_WITH_ASSERT(pPropertiesView != nullptr)
                {
                    pPropertiesView->setHTMLText(acGTStringToQString(htmlText), nullptr);
                }
            }
        }
    }
}

afApplicationTreeItemData* afApplicationTree::GetChildItemData(QTreeWidgetItem* pTreeItem, int childIndex)
{
    afApplicationTreeItemData* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(pTreeItem != nullptr)
    {
        QTreeWidgetItem* pChild = pTreeItem->child(childIndex);
        pRetVal = getTreeItemData(pChild);
    }

    return pRetVal;
}

void afApplicationTree::dragEnterEvent(QDragEnterEvent* pEvent)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pEvent != nullptr)
    {
        const QMimeData* pMimeData = pEvent->mimeData();

        if (pMimeData != nullptr && pMimeData->hasFormat("text/plain"))
        {
            pEvent->acceptProposedAction();
        }
        else
        {
            // Check what is the needed drag action for the current event:
            DragAction dragActionForFiles = afApplicationCommands::instance()->DragActionForDropEvent(pEvent);

            if (dragActionForFiles != DRAG_NO_ACTION)
            {
                pEvent->acceptProposedAction();
            }
        }
    }
}

void afApplicationTree::dragMoveEvent(QDragMoveEvent* pEvent)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pEvent != nullptr)
    {
        const QMimeData* pMimeData = pEvent->mimeData();

        if (pMimeData != nullptr && pMimeData->hasFormat("text/plain"))
        {
            pEvent->acceptProposedAction();
        }
        else
        {
            // Check what is the needed drag action for the current event:
            DragAction dragActionForFiles = afApplicationCommands::instance()->DragActionForDropEvent(pEvent);

            if (dragActionForFiles != DRAG_NO_ACTION)
            {
                pEvent->acceptProposedAction();
            }
        }
    }
}

void afApplicationTree::dropEvent(QDropEvent* pEvent)
{
    const QMimeData* pMimeData = pEvent->mimeData();

    if (pMimeData != nullptr)
    {
        if (pMimeData->hasUrls())
        {
            afApplicationCommands::instance()->HandleDropEvent(pEvent);
        }
        else if (pMimeData->hasFormat("text/plain"))
        {
            for (int i = 0; i < (int)m_applicationTreeHandlers.size(); i++)
            {
                afApplicationTreeHandler* pTreeHandler = m_applicationTreeHandlers[i];

                if (pTreeHandler != nullptr)
                {
                    if (m_pTreeCtrl != nullptr && m_pTreeCtrl->IsDragging())
                    {
                        pTreeHandler->ExecuteDropEvent(pEvent, QString());
                    }

                    break;
                }
            }
        }
    }
    else
    {
        afApplicationCommands::instance()->HandleDropEvent(pEvent);
    }
}

void afApplicationTree::OnTreeElementDropEvent(QDropEvent* pEvent)
{
    const QMimeData* pMimeData = pEvent->mimeData();

    if (pMimeData != nullptr)
    {
        if (pMimeData->hasUrls())
        {
            afApplicationCommands::instance()->HandleDropEvent(pEvent);
        }
        else if (pMimeData->hasFormat("text/plain"))
        {
            for (int i = 0; i < (int)m_applicationTreeHandlers.size(); i++)
            {
                afApplicationTreeHandler* pTreeHandler = m_applicationTreeHandlers[i];

                if (pTreeHandler != nullptr)
                {
                    if (m_pTreeCtrl != nullptr && m_pTreeCtrl->IsDragging())
                    {
                        pTreeHandler->ExecuteDropEvent(pEvent, QString());
                    }

                    break;
                }
            }
        }
    }
    else
    {
        afApplicationCommands::instance()->HandleDropEvent(pEvent);
    }
}

void afApplicationTree::OnTreeElementDragMoveEvent(QDragMoveEvent* pEvent)
{
    if ((pEvent != nullptr) && (m_pTreeCtrl != nullptr))
    {
        QTreeWidgetItem* pItem = m_pTreeCtrl->itemAt(pEvent->pos());

        if (pItem != nullptr)
        {
            const QMimeData* pMimeData = pEvent->mimeData();

            if (pMimeData != nullptr && pMimeData->hasFormat("text/plain"))
            {
                for (int i = 0; i < (int)m_applicationTreeHandlers.size(); i++)
                {
                    afApplicationTreeHandler* pTreeHandler = m_applicationTreeHandlers[i];

                    if (pTreeHandler != nullptr)
                    {
                        if (m_pTreeCtrl != nullptr && m_pTreeCtrl->IsDragging())
                        {
                            pEvent->setDropAction(Qt::MoveAction);

                            bool isItemDroppable = pTreeHandler->IsItemDroppable(pItem);

                            if (isItemDroppable)
                            {
                                pEvent->accept();
                                pItem->setSelected(true);
                                break;
                            }
                            else
                            {
                                pEvent->ignore();
                            }
                        }
                    }
                }
            }
        }
    }
}

void afApplicationTree::OnDragAttempt(QTreeWidgetItem* pItem, bool& canItemBeDragged)
{
    GT_IF_WITH_ASSERT(m_pTreeCtrl != nullptr)
    {
        canItemBeDragged = false;

        for (int i = 0; i < (int)m_applicationTreeHandlers.size(); i++)
        {
            afApplicationTreeHandler* pTreeHandler = m_applicationTreeHandlers[i];

            if (pTreeHandler != nullptr)
            {
                canItemBeDragged = pTreeHandler->CanItemBeDragged(pItem);

                if (canItemBeDragged)
                {
                    break;
                }
            }
        }
    }
}


