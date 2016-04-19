//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdMemoryView.cpp
///
//==================================================================================

//------------------------------ gdMemoryView.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

#include <AMDTApplicationComponents/Include/acChartWindow.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acChartWindow.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdAllocatedObjectsCreationStackView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMemoryView.h>

#define GD_MEMORY_VIEW_MIN_CHART_WIDTH 150
#define GD_MEMORY_VIEW_MIN_CHART_HEIGHT 170

// ---------------------------------------------------------------------------
// Name:        vspPropertiesView::gdMemoryView
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        17/1/2011
// ---------------------------------------------------------------------------
gdMemoryView::gdMemoryView(afProgressBarWrapper* pProgressBar, QWidget* pParent)
    : QSplitter(Qt::Vertical, pParent), gdMemoryViewBase(pProgressBar, NULL), _isShown(false),
      _pBottomWidget(NULL), _bottomPanelProp(1), _isFirstTimeLayout(true)
{
    // Get the application commands handler:
    _pApplicationCommands = afApplicationCommands::instance();
    _pGDApplicationCommands = gdApplicationCommands::gdInstance();

    // Set my frame layout:
    setFrameLayout();

    // Initialize the debugged process suspended flag:
    _isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

    _isTreeUpdatedByMe = false;

    // Connect selection handlers:
    bool rcConnect = connect(_pMemoryDetailsView, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onListItemSelected(QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(_pMemoryDetailsView, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onListItemSelected(QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(_pMemoryDetailsView, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(onListItemActivated(QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(_pMemoryDetailsView, SIGNAL(itemSelectionChanged()), this, SLOT(onListItemSelectionChanged()));

    // Connect the chart item clicked signal:
    bool rc = connect(_pChartWindow, SIGNAL(chartItemClicked(int)), _pMemoryDetailsView, SLOT(onChartItemClicked(int)));
    GT_ASSERT(rc);

}

// ---------------------------------------------------------------------------
// Name:        gdMemoryView::~gdMemoryView
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        17/1/2011
// ---------------------------------------------------------------------------
gdMemoryView::~gdMemoryView()
{
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryView::setFrameLayout
// Description: Build the layout of the view
// Author:      Sigal Algranaty
// Date:        17/1/2011
// ---------------------------------------------------------------------------
void gdMemoryView::setFrameLayout()
{
    // Get the application commands handler:
    GT_IF_WITH_ASSERT(_pGDApplicationCommands != NULL)
    {
        // The tree should already be initialized:
        GT_IF_WITH_ASSERT(gdDebugApplicationTreeHandler::instance() != NULL)
        {
            // Create the memory list view:
            _pMemoryDetailsView = new gdMemoryAnalysisDetailsView(this, NULL, gdDebugApplicationTreeHandler::instance());


            _pBottomWidget = new QWidget(this);


            // Create the chart window:
            _pChartWindow = new acChartWindow(_pBottomWidget, AC_NO_CHART);


            // Set min size:
            _pChartWindow->resize(GD_MEMORY_VIEW_MIN_CHART_WIDTH, GD_MEMORY_VIEW_MIN_CHART_HEIGHT);

            // Set the chart window size:
            _pChartWindow->clearAllData();
            _pChartWindow->setChartType(AC_NO_CHART);
            _pChartWindow->recalculateArrays();
            _pChartWindow->setHighlightColor(QColor(0, 0, 0, 125));

            // Show the statistics view:
            _pChartWindow->show();

            // Create the creation stacks view (note this has to be done after the properties window creation):
            _pAllocatedObjectsCreationStackView = new gdAllocatedObjectsCreationStackView(_pBottomWidget, GD_STR_MemoryAnalysisViewerObjectCreationCallStackCaption);


            // Create an horizontal layout for the bottom window:
            QHBoxLayout* pBottomLayout = new QHBoxLayout(_pBottomWidget);


            // Add the chart and properties to the bottom window:
            pBottomLayout->addWidget(_pChartWindow, 1);
            pBottomLayout->addWidget(_pAllocatedObjectsCreationStackView, 1);

            // Set the bottom widget layout:
            _pBottomWidget->setLayout(pBottomLayout);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryView::updateView
// Description:
// Arguments:   bool isViewShown
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        18/1/2011
// ---------------------------------------------------------------------------
bool gdMemoryView::updateView(bool isViewShown)
{
    bool retVal = false;

    _isShown = isViewShown;

    // Get the application commands handler:
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT((_pGDApplicationCommands != NULL) && (pApplicationCommands != NULL))
    {
        // Get the instance for the CodeXL monitored tree:
        gdDebugApplicationTreeHandler* pMonitoredObjectsTree = gdDebugApplicationTreeHandler::instance();
        afApplicationTree* pApplicationTree = pApplicationCommands->applicationTree();

        // The tree should already be initialized:
        GT_IF_WITH_ASSERT((pMonitoredObjectsTree != NULL) && (_pMemoryDetailsView != NULL) && (pApplicationTree != NULL))
        {
            retVal = true;

            if (isViewShown != pMonitoredObjectsTree->shouldUpdateMemoryData())
            {
                // If the tree update data status is different then is was, update the memory data:
                pMonitoredObjectsTree->setShouldUpdateMemoryData(isViewShown);

                // Get the selected object and update the view:
                if (isViewShown)
                {
                    if (_isDebuggedProcessSuspended)
                    {
                        // Get the currently selected item:
                        QTreeWidgetItem* pSelectedItem = pApplicationTree->getTreeSelection();

                        // Update the memory view with the selected item:
                        retVal = _pMemoryDetailsView->updateListItems(pSelectedItem, true);
                    }
                    else
                    {
                        // Clear the view and display a message that the view is shown only on process suspension:
                        _pMemoryDetailsView->clearAndDisplayMessage();
                    }
                }
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryView::onTreeItemSelection
// Description: Handling a tree item selection event
// Arguments:   apMonitoredObjectsTreeSelectedEvent& eve - the event thrown from the tree
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        18/1/2011
// ---------------------------------------------------------------------------
void gdMemoryView::onTreeItemSelection(const apMonitoredObjectsTreeSelectedEvent& eve)
{
    if (!_isAutomaticallySelectingTreeItem && _isDebuggedProcessSuspended)
    {
        // Get the item data from the event:
        const afApplicationTreeItemData* pTreeItemData = (afApplicationTreeItemData*)eve.selectedItemData();

        if (pTreeItemData != NULL)
        {
            gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pTreeItemData->extendedItemData());

            if (pGDData != NULL)
            {
                // Display the item memory:
                bool rcDisplay = displayItemMemory(pTreeItemData);
                GT_ASSERT(rcDisplay);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryView::displayItemMemory
// Description: Display the requested item in my views
// Arguments:   gdDebugApplicationTreeData* pTreeItemData
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/1/2011
// ---------------------------------------------------------------------------
bool gdMemoryView::displayItemMemory(const afApplicationTreeItemData* pTreeItemData)
{
    bool retVal = false;
    gdDebugApplicationTreeHandler* pMonitoredObjectsTree = gdDebugApplicationTreeHandler::instance();

    // Sanity check:
    GT_IF_WITH_ASSERT((pTreeItemData != NULL) && (_pMemoryDetailsView != NULL) && (_pChartWindow != NULL) && (_pApplicationCommands != NULL) && (pMonitoredObjectsTree != NULL))
    {
        // If the tree update data status is different then is was, update the memory data:
        pMonitoredObjectsTree->setShouldUpdateMemoryData(true);

        // TO_DO: VS MEMORY: find out what this flag is and remove if not necessary:
        _updatingOnProcessSuspension = true;

        // Get the selected tree item id:
        QTreeWidgetItem* pSelectedItem = pTreeItemData->m_pTreeWidgetItem;

        // m_pLastSelectedTreeItemId - is used for working around WX bug - tree selection event is called twice.
        if (m_pLastSelectedTreeItemId != pSelectedItem)
        {
            retVal = true;

            GT_IF_WITH_ASSERT(pSelectedItem != NULL)
            {
                // Uri, 15/12/09: Do not select a list item if this event comes from selecting a list item.
                // This causes an endless feedback loop in Mac:
                if (!_isAutomaticallySelectingTreeItem)
                {
                    // Update the list according to the tree selected item:
                    retVal = _pMemoryDetailsView->updateListItems(pSelectedItem, _updatingOnProcessSuspension);
                }

                // Display the currently selected memory item and update the pie chart with the currently selected tree node:
                // IMPORTANT: this call should remain after the call to the list view items update,
                // since we use the list view to get information for texture types amount.
                updateChart(pTreeItemData);

                // Display the call stack for the requested item:
                displayItemCallStack(pTreeItemData);

                // TO_DO: VS MEMORY: Perform an operation that fixes GL window update!
                gtString caption = _pApplicationCommands->captionPrefix();

                // Get the displayed id from the detailed view:
                afApplicationTreeItemData& displayedListID = _pMemoryDetailsView->currentDisplayedListID();

                // Get my display string:
                gtString itemDisplayString;
                bool rcGetName = false;

                if (displayedListID.m_itemType != AF_TREE_ITEM_APP_ROOT)
                {
                    rcGetName = gdHTMLProperties::itemIDAsString(displayedListID, itemDisplayString);
                }

                if (rcGetName)
                {
                    // Set my caption:
                    caption.appendFormattedString(GD_STR_memoryViewCaptionWithContext, itemDisplayString.asCharArray());
                }
                else
                {
                    // Default caption:
                    caption.append(GD_STR_memoryViewCaptionDefault);
                }

                // Set the caption for the tool window:
                _pApplicationCommands->setWindowCaption(this, caption);
            }
        }

        // TO_DO: VS MEMORY: find out what this flag is and remove if not necessary:
        _updatingOnProcessSuspension = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryView::onListItemSelected
// Description: Call the base class event implementation
// Author:      Sigal Algranaty
// Date:        19/1/2011
// ---------------------------------------------------------------------------
void gdMemoryView::onListItemSelected(QTableWidgetItem* pItem)
{
    // Call the base class implementation:
    gdMemoryViewBase::onListItemSelected(pItem);
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryView::onListItemSelectionChanged
// Description: Is handling a list item selection changed event
// Author:      Sigal Algranaty
// Date:        23/1/2012
// ---------------------------------------------------------------------------
void gdMemoryView::onListItemSelectionChanged()
{
    // Sanity check
    GT_IF_WITH_ASSERT(_pMemoryDetailsView != NULL)
    {
        if (_pMemoryDetailsView->amountOfSelectedRows() >= 1)
        {
            // Find the selected item:
            QTableWidgetItem* pSelectedItem = _pMemoryDetailsView->selectedItems().first();
            GT_IF_WITH_ASSERT(pSelectedItem != NULL)
            {
                onListItemSelected(pSelectedItem);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdMemoryView::onListItemActivated
// Description: Call the base class event implementation
// Arguments:   wxListEvent &eve
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        19/1/2011
// ---------------------------------------------------------------------------
void gdMemoryView::onListItemActivated(QTableWidgetItem* pItem)
{
    // Call the base class implementation:
    gdMemoryViewBase::onListItemActivated(pItem);
}


// ---------------------------------------------------------------------------
// Name:        gdMemoryView::onListItemDeselected
// Description: Call the base class event implementation
// Arguments:   wxListEvent &eve
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        9/2/2011
// ---------------------------------------------------------------------------
void gdMemoryView::onListItemDeselected(QTableWidgetItem* pItem)
{
    // Call the base class implementation:
    gdMemoryViewBase::onListItemDeselected(pItem);
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryView::raiseView
// Description: Show the view as top window
// Author:      Sigal Algranaty
// Date:        25/1/2011
// ---------------------------------------------------------------------------
void gdMemoryView::raiseView()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pGDApplicationCommands != NULL)
    {
        _pGDApplicationCommands->raiseMemoryView();
        _isShown = true;
    }
}

