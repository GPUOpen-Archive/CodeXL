//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afGeneralViewsCreator.cpp
///
//==================================================================================

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QtWidgets>

// Local:
#include <AMDTApplicationFramework/Include/views/afInformationView.h>

// Infra:
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/Include/acVirtualListCtrl.h>

// Local:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afGeneralActionsCreator.h>
#include <AMDTApplicationFramework/Include/afGeneralViewsCreator.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>
#include <AMDTApplicationFramework/Include/views/afPropertiesView.h>

// View size definition:
#define AF_BOTTOM_ROW_HEIGHT_PROPORTION(x) (x / 4)
#define AF_BOTTOM_ROW_NARROW_VIEWS_WIDTH_PROPORTION(x) (x / 5)
#define AF_MAIN_VIEWS_HEIGHT_PROPORTION(x) ((x * 3) / 4)
#define AF_TREE_VIEW_WIDTH_PROPORTION(x) (x / 5)

// Static member initialization:
afPropertiesView* afGeneralViewsCreator::m_spPropertiesView = nullptr;
afApplicationTree* afGeneralViewsCreator::m_spApplicationTree = nullptr;
afInformationView* afGeneralViewsCreator::m_spInformationView = nullptr;
afGeneralViewsCreator* afGeneralViewsCreator::m_spMySingleInstance = nullptr;

// ---------------------------------------------------------------------------
// Name:        afGeneralViewsCreator::afGeneralViewsCreator
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
afGeneralViewsCreator::afGeneralViewsCreator()

{
    // Create an action creator:
    _pViewActionCreator = new afGeneralActionsCreator;

    _pViewActionCreator->initializeCreator();
}

afGeneralViewsCreator& afGeneralViewsCreator::Instance()
{
    // If my single instance was not created yet - create it:
    if (m_spMySingleInstance == nullptr)
    {
        m_spMySingleInstance = new afGeneralViewsCreator;
        GT_ASSERT(m_spMySingleInstance);

        // Initialize:
        m_spMySingleInstance->initialize();
    }

    return *m_spMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        afGeneralViewsCreator::initialize
// Description: Initialize the creator
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
void afGeneralViewsCreator::initialize()
{
    acIconId invertedIconId = (acIconId)(afGlobalVariablesManager::ProductIconID() + 1);

    initSingleViewIcon(afApplicationTreeViewIndex, invertedIconId);
    initSingleViewIcon(afPropertiesViewIndex, AC_ICON_VIEW_PROPERTIES);
    initSingleViewIcon(afInformationViewIndex, AC_ICON_VIEW_INFORMATION);
}

// ---------------------------------------------------------------------------
// Name:        afGeneralViewsCreator::~afGeneralViewsCreator
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
afGeneralViewsCreator::~afGeneralViewsCreator()
{
    delete m_spApplicationTree;
    delete m_spInformationView;
    delete m_spPropertiesView;

    m_spApplicationTree = nullptr;
    m_spInformationView = nullptr;
    m_spPropertiesView = nullptr;
}

// ---------------------------------------------------------------------------
// Name:        afGeneralViewsCreator::titleString
// Description: Get the title of the created view
// Arguments:   int viewIndex
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
void afGeneralViewsCreator::titleString(int viewIndex, gtString& viewTitle, gtString& viewMenuCommand)
{
    if (afApplicationTreeViewIndex == viewIndex)
    {
        viewTitle.appendFormattedString(AF_STR_ApplicationTreeViewCaption, afGlobalVariablesManager::ProductName().asCharArray());
        gtString codeXLVersion = afGlobalVariablesManager::instance().versionCaption();

        if (!codeXLVersion.isEmpty())
        {
            viewTitle.appendFormattedString(L" (%ls)", codeXLVersion.asCharArray());
        }

        viewMenuCommand.appendFormattedString(AF_STR_ApplicationTreeViewCommandName, afGlobalVariablesManager::ProductName().asCharArray());
    }
    else if (afPropertiesViewIndex == viewIndex)
    {
        // Return the view caption:
        viewTitle = AF_STR_propertiesViewCaption;
        viewMenuCommand = AF_STR_propertiesViewCommandName;
    }
    else if (afInformationViewIndex == viewIndex)
    {
        // Return the view caption:
        viewTitle = AF_STR_informationViewCaption;
        viewMenuCommand = AF_STR_informationViewCommandName;
    }
}

// ---------------------------------------------------------------------------
// Name:        afGeneralViewsCreator::associatedToolbar
// Description: Get the name of the toolbar associated with the requested view
// Arguments:   int viewIndex
// Return Val:  gtString
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
gtString afGeneralViewsCreator::associatedToolbar(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    gtString retVal;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afGeneralViewsCreator::type
// Description: Get view type
// Arguments:   int viewIndex
// Return Val:  afViewType
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
afViewCreatorAbstract::afViewType afGeneralViewsCreator::type(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    afViewCreatorAbstract::afViewType retDockArea = AF_VIEW_dock;

    return retDockArea;
}

// ---------------------------------------------------------------------------
// Name:        afGeneralViewsCreator::dockArea
// Description: Get the docking area
// Arguments:   int viewIndex
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
int afGeneralViewsCreator::dockArea(int viewIndex)
{
    int retVal = AF_VIEW_DOCK_BottomDockWidgetArea;

    if (viewIndex == afApplicationTreeViewIndex)
    {
        retVal = AF_VIEW_DOCK_LeftDockWidgetArea;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afGeneralViewsCreator::dockWith
// Description: Define for each dock if it is docked with another view
// Arguments:   int viewIndex
// Return Val:  gtString
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
gtString afGeneralViewsCreator::dockWith(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    gtString retVal = L"";
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afGeneralViewsCreator::dockWidgetFeatures
// Description: Get the docking features
// Return Val:  DockWidgetFeatures
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
QDockWidget::DockWidgetFeatures afGeneralViewsCreator::dockWidgetFeatures(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    QDockWidget::DockWidgetFeatures retVal = QDockWidget::AllDockWidgetFeatures;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afGeneralViewsCreator::initialSize
// Description: Get the initial size
// Arguments:   int viewIndex
// Return Val:  QSize - size of the view
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
QSize afGeneralViewsCreator::initialSize(int viewIndex)
{
    QSize retSize(0, 0);
    QSize desktopSize = QApplication::desktop()->rect().size();

    // Set the default minimum view height and width
    int minViewWidth = 768;
    int minViewHeight = 292;
    QSize minViewSize(minViewWidth, minViewHeight);
    retSize = minViewSize;

    if (viewIndex == afApplicationTreeViewIndex)
    {
        retSize.setWidth(AF_TREE_VIEW_WIDTH_PROPORTION(desktopSize.width()));
        retSize.setHeight(AF_MAIN_VIEWS_HEIGHT_PROPORTION(desktopSize.height()));
    }
    else if (viewIndex == afPropertiesViewIndex)
    {
        retSize.setWidth(AF_BOTTOM_ROW_NARROW_VIEWS_WIDTH_PROPORTION(desktopSize.width()));
        retSize.setHeight(AF_BOTTOM_ROW_HEIGHT_PROPORTION(desktopSize.height()));
    }

    return retSize;
}

// ---------------------------------------------------------------------------
// Name:        afGeneralViewsCreator::visibility
// Description: Get the initial visibility of the view
// Arguments:   int viewIndex
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
bool afGeneralViewsCreator::visibility(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    bool retVal = true;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afGeneralViewsCreator::initiallyActive
// Description: Get the initial active/inactive status of the view
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
bool afGeneralViewsCreator::initiallyActive(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    bool retVal = true;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afGeneralViewsCreator::amountOfViewTypes
// Description: Get number of views that can be created by this creator
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
int afGeneralViewsCreator::amountOfViewTypes()
{
    return afAmountOfGenericViews;
}

// ---------------------------------------------------------------------------
// Name:        afGeneralViewsCreator::createViewContent
// Description: Create the WX inner window
// Arguments:   int viewIndex
// Return Val:  bool - Success / Failure
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
bool afGeneralViewsCreator::createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent)
{
    bool retVal = true;

    pContentQWidget = nullptr;

    switch (viewIndex)
    {
        case afApplicationTreeViewIndex:
        {
            // Initialize the default sizes array:
            m_spApplicationTree = new afApplicationTree(&afProgressBarWrapper::instance(), pQParent);

            pContentQWidget = m_spApplicationTree;

            // Emit a signal stating that the tree was created:
            afQtCreatorsManager::instance().EmitApplicationTreeCreatedSignal();
        }
        break;

        case afPropertiesViewIndex:
        {
            // Create the properties view:
            m_spPropertiesView = new afPropertiesView(&afProgressBarWrapper::instance(), pQParent);

            pContentQWidget = m_spPropertiesView;
        }
        break;

        case afInformationViewIndex:
        {
            // Create the properties view:
            m_spInformationView = new afInformationView(pQParent);

            pContentQWidget = m_spInformationView;
        }
        break;

        default:
            GT_ASSERT_EX(false, L"Invalid view index");
            break;

    }

    // Set the created window:
    m_viewsCreated.push_back(pContentQWidget);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afGeneralViewsCreator::handleTrigger
// Description: Handle the action when it is triggered
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
void afGeneralViewsCreator::handleTrigger(int viewIndex, int actionIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    // One of the components of the statistics view:
    GT_IF_WITH_ASSERT(m_spPropertiesView != nullptr)
    {
        // Sanity check:
        GT_IF_WITH_ASSERT((actionIndex >= 0) && (actionIndex < (int) supportedCommandIds().size()))
        {
            // Get the main window:
            afMainAppWindow* pMainWindow = afMainAppWindow::instance();
            GT_IF_WITH_ASSERT(pMainWindow != nullptr)
            {
                // Get the current focused widget:
                QWidget* pFocusWidget = pMainWindow->findFocusedWidget();
                GT_IF_WITH_ASSERT(pFocusWidget != nullptr)
                {

                    // Check if this is a properties of information view:
                    afBaseView* pBaseView = nullptr;

                    if (pFocusWidget == m_spPropertiesView)
                    {
                        pBaseView = m_spPropertiesView;
                    }

                    if (pFocusWidget == m_spInformationView || (pFocusWidget->parent() == m_spInformationView))
                    {
                        pBaseView = m_spInformationView;
                    }

                    if (pBaseView != nullptr)
                    {
                        // Get the command id:
                        int commandId = actionIndexToCommandId(actionIndex);

                        switch (commandId)
                        {
                            case ID_COPY:
                                pBaseView->onEdit_Copy();
                                break;

                            case ID_SELECT_ALL:
                                pBaseView->onEdit_SelectAll();
                                break;

                            default:
                            {
                                GT_ASSERT_EX(false, L"Unsupported application command");
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afGeneralViewsCreator::handleUiUpdate
// Description: Handle UI update
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
void afGeneralViewsCreator::handleUiUpdate(int viewIndex, int actionIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    bool isActionEnabled = false, isActionChecked = false, isActionCheckable = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pViewActionCreator)
    {
        // Get the QT action:
        QAction* pAction = _pViewActionCreator->action(actionIndex);
        GT_IF_WITH_ASSERT(pAction != nullptr)
        {
            // Get the main window:
            afMainAppWindow* pMainWindow = afMainAppWindow::instance();
            GT_IF_WITH_ASSERT(pMainWindow != nullptr)
            {
                // Get the current focused widget:
                QWidget* pFocusWidget = pMainWindow->findFocusedWidget();
                GT_IF_WITH_ASSERT(pFocusWidget != nullptr)
                {
                    // Check if this is an acListCtrl (if we got here, the focused widget should be a list control):
                    // Check if this is a properties of information view:
                    afBaseView* pBaseView = nullptr;

                    if (pFocusWidget == m_spPropertiesView)
                    {
                        pBaseView = m_spPropertiesView;
                    }

                    if (pFocusWidget == m_spInformationView || (pFocusWidget->parent() == m_spInformationView))
                    {
                        pBaseView = m_spInformationView;
                    }

                    if (pBaseView != nullptr)
                    {
                        // Get the command id:
                        int commandId = actionIndexToCommandId(actionIndex);

                        switch (commandId)
                        {
                            case ID_COPY:
                            case ID_SELECT_ALL:
                                isActionEnabled = true;
                                break;

                            default:
                            {
                                isActionEnabled = false;
                                break;
                            }
                        }
                    }
                }
            }

            // Set the action enable / disable:
            pAction->setEnabled(isActionEnabled);

            // Set the action checkable state:
            pAction->setCheckable(isActionCheckable);

            // Set the action check state:
            pAction->setChecked(isActionChecked);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afGeneralViewsCreator::addSeparator
// Description: Should a separator be added before the view menu item?
// Arguments:   int viewIndex
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/2/2012
// ---------------------------------------------------------------------------
bool afGeneralViewsCreator::addSeparator(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    bool retVal = false;
    return retVal;
}




