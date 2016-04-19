//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdBreakpointsDialog.cpp
///
//==================================================================================

//------------------------------ gdBreakpointsDialog.cpp ------------------------------

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <AMDTApplicationComponents/Include/acQtIncludes.h>
#include <QVariant>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTAPIClasses/Include/apFunctionType.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTAPIClasses/Include/apGenericBreakpoint.h>
#include <AMDTAPIClasses/Include/apKernelFunctionNameBreakpoint.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointsUpdatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acDefinitions.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acQTextFilterCtrl.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdBreakpointsItemData.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdBreakpointsDialog.h>


// Register the gdBreakpointsItemData type so we can store retrieve from QVariant without casting
Q_DECLARE_METATYPE(gdBreakpointsItemData*)

// Constants for dialog sizes:
#define GD_BREAKPOINTS_LISTS_MIN_WIDTH              (60 * AC_DEFAULT_TEXT_AVERAGE_CHAR_WIDTH)
#define GD_BREAKPOINTS_CHOSEN_LIST_MIN_HEIGHT       200
#define GD_BREAKPOINTS_AVAILABLE_LIST_MIN_HEIGHT    350


static int s_LastFocusedPage = 0;    // which list (page) was the active one. stored between opening of the dialog

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::gdBreakpointsDialog
// Description: Constructor
// Arguments:   QWidget* pParent
// Return Val:
// Author:      Yoni Rabin
// Date:        13/6/2012
// ---------------------------------------------------------------------------
gdBreakpointsDialog::gdBreakpointsDialog(QWidget* pParent)
    : acDialog(afMainAppWindow::instance(), true, true, QDialogButtonBox::Ok),
      m_pMainGroupBox(NULL), m_pTopLayoutV(NULL), m_pMainLayoutH(NULL), m_pCenterButtonsLayoutV(NULL), m_pRightLayoutV(NULL),
      m_pAPILayoutV(NULL), m_pKernelLayoutV(NULL), m_pBreakpointsLayoutV(NULL),
      m_pBottomButtonsLayoutH(NULL),
      m_pAddButton(NULL), m_pRemoveButton(NULL), m_pRemoveAllButton(NULL),
      m_pCheckBox(NULL),
      m_pAPIList(NULL), m_pKernelList(NULL), m_pGenericBreakpointsList(NULL),
      m_pChosenList(NULL), m_pFunctionsFilter(NULL), m_pKernelFilter(NULL),
      m_pDescription(NULL), m_pChosenListText(NULL),
      m_pTabs(NULL), m_pAPITab(NULL), m_pKernelTab(NULL), m_pBreakpointsTab(NULL),
      m_pLastChosenRow(NULL), m_LastChosenRowOnEdit(false),
      m_updatingCheckStatus(false), m_amountOfMonitoredFunctions(0)
{
    (void)(pParent); // unused
    // Set window flags
    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    // Get the CodeXL project type title as string:
    QString title = afGlobalVariablesManager::ProductNameA();
    title.append(AF_STR_SpaceA);
    title.append(GD_STR_BreakpointsTitle);

    // Set the title:
    setWindowTitle(title);

    // Add the Icon to the dialog:
    afLoadTitleBarIcon(this);

    // Build the dialog items and layout them:
    setDialogLayout();

    // Set the initial values of the panes:
    setInitialValues();

    // Build final dialog layout:
    buildFinalLayout();

    // Add the first row:
    addKernelEditLine();

    // Put the filter line in focus:
    m_pFunctionsFilter->setFocus(Qt::ActiveWindowFocusReason);
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::~gdBreakpointsDialog
// Description: Destructor
// Return Val:
// Author:      Yoni Rabin
// Date:        13/6/2012
// ---------------------------------------------------------------------------
gdBreakpointsDialog::~gdBreakpointsDialog()
{
    // Delete the item data objects for the right list:
    m_rightListBreakpointsDataVector.deleteElementsAndClear();

    GT_IF_WITH_ASSERT(m_pAPIList != NULL)
    {
        // Delete the item data from the API function list:
        int listSize = m_pAPIList->rowCount();

        for (int i = 0; i < listSize; ++i)
        {
            gdBreakpointsItemData* pCurrentAPIFunctionData = getBreakpointData(m_pAPIList->item(i, 0));
            GT_IF_WITH_ASSERT(pCurrentAPIFunctionData != NULL)
            {
                delete pCurrentAPIFunctionData;
            }
        }

        m_pAPIList->clearList();
    }

    GT_IF_WITH_ASSERT(m_pKernelList != NULL)
    {
        // Delete the item data from the kernel function list:
        int listSize = m_pKernelList->rowCount();

        for (int i = 0; i < listSize; i++)
        {
            gdBreakpointsItemData* pCurrentKernelFunctionData = getBreakpointData(m_pKernelList->item(i, 0));
            GT_IF_WITH_ASSERT(pCurrentKernelFunctionData != NULL)
            {
                delete pCurrentKernelFunctionData;
            }
        }

        m_pKernelList->clearList();
    }

    GT_IF_WITH_ASSERT(m_pGenericBreakpointsList != NULL)
    {
        // Delete the item data from the error / warning list:
        int listSize = m_pGenericBreakpointsList->rowCount();

        for (int i = 0; i < listSize; i++)
        {
            gdBreakpointsItemData* pCurrentErrorWarningData = getBreakpointData(m_pGenericBreakpointsList->item(i, 0));
            GT_IF_WITH_ASSERT(pCurrentErrorWarningData != NULL)
            {
                delete pCurrentErrorWarningData;
            }
        }

        m_pGenericBreakpointsList->clearList();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onOk
// Description: Logic executed when OK is clicked
// Return Val:  void
// Author:      Yoni Rabin
// Date:        13/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onOk()
{
    // No need for the kernel edit line from here:
    removeKernelEditLine();

    // Removing all the breakpoints from the infrastructure:
    bool rc1 = gaRemoveAllBreakpointsByType(OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT);
    bool rc2 = gaRemoveAllBreakpointsByType(OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT);
    bool rc3 = gaRemoveAllBreakpointsByType(OS_TOBJ_ID_GENERIC_BREAKPOINT);
    GT_IF_WITH_ASSERT(rc1 && rc2 && rc3)
    {
        // Iterate the selected breakpoints in the functions panel:
        int amountOfBreakpoints = m_pChosenList->rowCount();

        for (int i = 0; i < amountOfBreakpoints; i++)
        {
            // Get the breakpoint item data;
            gdBreakpointsItemData* pBreakpointTypeData = (gdBreakpointsItemData*)m_pChosenList->getItemData(i);
            GT_IF_WITH_ASSERT(pBreakpointTypeData != NULL)
            {
                // Check if the current breakpoint is enabled:
                apBreakPoint* pNewBreakpoint = NULL;

                // Allocate a breakpoint according to the breakpoint type:
                if (pBreakpointTypeData->_breakpointType == OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT)
                {
                    // Allocate an API function breakpoint:
                    pNewBreakpoint = new apMonitoredFunctionBreakPoint(pBreakpointTypeData->_monitoredFunctionId);
                }
                else if (pBreakpointTypeData->_breakpointType == OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT)
                {
                    // Allocate a kernel breakpoint:
                    pNewBreakpoint = new apKernelFunctionNameBreakpoint(pBreakpointTypeData->_kernelFunctionName);
                }
                else if (pBreakpointTypeData->_breakpointType == OS_TOBJ_ID_GENERIC_BREAKPOINT)
                {
                    // Allocate an API function breakpoint:
                    pNewBreakpoint = new apGenericBreakpoint(pBreakpointTypeData->_genericBreakpointType);
                }

                GT_IF_WITH_ASSERT(pNewBreakpoint != NULL)
                {
                    // Check if the breakpoint is enabled:
                    QTableWidgetItem* pItem = m_pChosenList->item(i, 0);
                    GT_IF_WITH_ASSERT(pItem != NULL)
                    {
                        bool isEnabled = (Qt::Checked == pItem->checkState());
                        pBreakpointTypeData->_isEnabled  = isEnabled;
                        // Set the breakpoint status (i.e. Checked / Unchecked)
                        pNewBreakpoint->setEnableStatus(isEnabled);
                        pNewBreakpoint->setHitCount(pBreakpointTypeData->_hitCount);
                        rc2 = gaSetBreakpoint(*pNewBreakpoint);
                        GT_ASSERT(rc2);
                    }
                    // Release the breakpoint memory:
                    delete pNewBreakpoint;
                }
            }
        }

        GT_ASSERT(rc1);

        // Trigger breakpoints update event:
        // The -1 states the all the breakpoints are updated, and lists should be updated from scratch:
        apBreakpointsUpdatedEvent eve(-1);
        apEventsHandler::instance().registerPendingDebugEvent(eve);
    }

    s_LastFocusedPage = m_pTabs->currentIndex();
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::setDialogLayout
// Description: Creates the layout
// Return Val:  void
// Author:      Yoni Rabin
// Date:        13/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::setDialogLayout()
{
    unsigned int listsWidth = acScalePixelSizeToDisplayDPI(GD_BREAKPOINTS_LISTS_MIN_WIDTH);
    unsigned int availableListHeight = acScalePixelSizeToDisplayDPI(GD_BREAKPOINTS_AVAILABLE_LIST_MIN_HEIGHT);
    unsigned int chosenListHeight = acScalePixelSizeToDisplayDPI(GD_BREAKPOINTS_CHOSEN_LIST_MIN_HEIGHT);

    // Create layouts:
    m_pTopLayoutV = new QVBoxLayout();
    m_pMainLayoutH = new QHBoxLayout();
    m_pCenterButtonsLayoutV = new QVBoxLayout();
    m_pRightLayoutV = new QVBoxLayout();
    m_pBottomButtonsLayoutH = new QHBoxLayout();

    // Set top label:
    m_pDescription = new QLabel(GD_STR_BreakpointsDescription);

    // Set chosen list text:
    m_pChosenListText = new QLabel(GD_STR_BreakpointsChoosenListText);

    // Group Box for all the dialog components:
    m_pMainGroupBox = new QGroupBox(GD_STR_BreakpointsBoxTitle);

    m_pBottomButtonsLayoutH = getBottomButtonLayout(false);
    bool rc = connect(this, SIGNAL(accepted()), this, SLOT(onOk()));
    GT_ASSERT(rc);

    // Add / Remove / Remove All buttons
    // Add Button:
    m_pAddButton = new QPushButton(AF_STR_AddButton, this);
    m_pAddButton->setDefault(false);
    m_pAddButton->setAutoDefault(false);
    m_pAddButton->setEnabled(false);

    // Remove Button:
    m_pRemoveButton = new QPushButton(AF_STR_RemoveButton, this);
    m_pRemoveButton->setDefault(false);
    m_pRemoveButton->setAutoDefault(false);
    m_pRemoveButton->setEnabled(false);

    // Remove All Button:
    m_pRemoveAllButton = new QPushButton(AF_STR_RemoveAllButton, this);
    m_pRemoveAllButton->setDefault(false);
    m_pRemoveAllButton->setAutoDefault(false);
    m_pRemoveAllButton->setEnabled(false);

    // Set the min size to the other buttons (according to the longest item):
    QSize minButtonSize = m_pRemoveButton->size();
    m_pRemoveAllButton->setMinimumSize(minButtonSize);
    m_pRemoveButton->setMinimumSize(minButtonSize);
    m_pAddButton->setMinimumSize(minButtonSize);

    // Connect the buttons to slots:
    rc = connect(m_pAddButton, SIGNAL(clicked()), this, SLOT(onAdd()));
    GT_ASSERT(rc);
    rc = connect(m_pRemoveButton, SIGNAL(clicked()), this, SLOT(onRemove()));
    GT_ASSERT(rc);
    rc = connect(m_pRemoveAllButton, SIGNAL(clicked()), this, SLOT(onRemoveAll()));
    GT_ASSERT(rc);

    // Initialize CheckBox:
    m_pCheckBox = new QCheckBox(GD_STR_BreakpointsEnableDisableAllBreakpoints);

    // Initialize tabbed lists:
    // API:
    m_pAPITab = new QWidget();
    m_pAPIList = new acListCtrl(m_pAPITab, AC_DEFAULT_LINE_HEIGHT, false);
    m_pAPIList->setMinimumSize(listsWidth, availableListHeight);
    m_pAPIList->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pAPIList->setAutoScroll(true);
    m_pAPIList->setColumnCount(1);
    m_pAPIList->horizontalHeader()->hide();
    m_pAPIList->verticalHeader()->hide();
    m_pAPIList->setTabKeyNavigation(false);
    m_pAPIList->setFocusPolicy(Qt::StrongFocus);

    // Kernels:
    m_pKernelTab = new QWidget();
    m_pKernelList = new acListCtrl(m_pKernelTab, AC_DEFAULT_LINE_HEIGHT, false);
    m_pKernelList->setMinimumSize(listsWidth, availableListHeight);
    m_pKernelList->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pKernelList->setAutoScroll(true);
    m_pKernelList->setColumnCount(1);
    m_pKernelList->horizontalHeader()->hide();
    m_pKernelList->verticalHeader()->hide();
    m_pKernelList->setTabKeyNavigation(false);
    m_pKernelList->setFocusPolicy(Qt::StrongFocus);

    // Generic BPs:
    m_pBreakpointsTab = new QWidget();
    m_pGenericBreakpointsList = new acListCtrl(m_pBreakpointsTab, AC_DEFAULT_LINE_HEIGHT, false);
    m_pGenericBreakpointsList->setMinimumSize(listsWidth, availableListHeight);
    m_pGenericBreakpointsList->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pGenericBreakpointsList->setAutoScroll(true);
    m_pGenericBreakpointsList->setColumnCount(1);
    m_pGenericBreakpointsList->horizontalHeader()->hide();
    m_pGenericBreakpointsList->verticalHeader()->hide();
    m_pGenericBreakpointsList->setTabKeyNavigation(false);
    m_pGenericBreakpointsList->setFocusPolicy(Qt::StrongFocus);

    // Initialize right acListCtrl:
    m_pChosenList = new acListCtrl(this, AC_DEFAULT_LINE_HEIGHT, true);
    m_pChosenList->setMinimumSize(listsWidth, chosenListHeight);
    m_pChosenList->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pChosenList->setAutoScroll(true);
    m_pChosenList->setColumnCount(2);

    // Set the headers:
    QStringList chosenColumnCaptions;
    chosenColumnCaptions << GD_STR_BreakPointsFunctionColumn;
    chosenColumnCaptions << GD_STR_BreakPointsTypeColumn;
    m_pChosenList->initHeaders(chosenColumnCaptions, false);
    m_pChosenList->setColumnWidth(0, m_pChosenList->width() - 55);
    m_pChosenList->setColumnWidth(1, 50);
    m_pChosenList->setTabKeyNavigation(false);
    m_pChosenList->setFocusPolicy(Qt::StrongFocus);

    // Text filters:
    m_pFunctionsFilter = new acQTextFilterCtrl();
    m_pFunctionsFilter->setMinimumWidth(listsWidth);
    m_pFunctionsFilter->setFocusPolicy(Qt::StrongFocus);

    m_pKernelFilter = new acQTextFilterCtrl();
    m_pKernelFilter->setMinimumWidth(listsWidth);
    m_pKernelFilter->setFocusPolicy(Qt::StrongFocus);

    //Initialize main tab view;
    m_pAPILayoutV = new QVBoxLayout();
    m_pAPILayoutV->addWidget(m_pAPIList, 1);
    m_pAPILayoutV->addWidget(m_pFunctionsFilter, 0, Qt::AlignTop);
    m_pAPITab->setLayout(m_pAPILayoutV);

    m_pKernelLayoutV = new QVBoxLayout();
    m_pKernelLayoutV->addWidget(m_pKernelList, 1);
    m_pKernelLayoutV->addWidget(m_pKernelFilter, 0, Qt::AlignTop);
    m_pKernelTab->setLayout(m_pKernelLayoutV);

    m_pBreakpointsLayoutV = new QVBoxLayout();
    m_pBreakpointsLayoutV->addWidget(m_pGenericBreakpointsList, 0);
    m_pBreakpointsTab->setLayout(m_pBreakpointsLayoutV);

    m_pTabs = new QTabWidget();

    // Set the tab texts:
    m_pTabs->addTab(m_pAPITab, GD_STR_BreakpointsListText);
    m_pTabs->addTab(m_pKernelTab, GD_STR_BreakpointsKernelListText);
    m_pTabs->addTab(m_pBreakpointsTab, GD_STR_BreakPointsGenericText);

    // Set the current tab
    m_pTabs->setCurrentIndex(s_LastFocusedPage);

    // Central layout with buttons:
    m_pCenterButtonsLayoutV->addStretch(1);
    m_pCenterButtonsLayoutV->addWidget(m_pAddButton, 0, Qt::AlignTop);
    m_pCenterButtonsLayoutV->addWidget(m_pRemoveButton, 0, Qt::AlignTop);
    m_pCenterButtonsLayoutV->addWidget(m_pRemoveAllButton, 0, Qt::AlignTop);
    m_pCenterButtonsLayoutV->addStretch(1);

    // Right layout with chosen breakpoints:
    m_pRightLayoutV->addWidget(m_pChosenListText, 0, Qt::AlignLeft);
    m_pRightLayoutV->addWidget(m_pChosenList, 0, Qt::AlignLeft);
    m_pRightLayoutV->addWidget(m_pCheckBox, 0, Qt::AlignLeft);
    m_pRightLayoutV->setMargin(5);

    // Main horizontal layout:
    m_pMainLayoutH->addWidget(m_pTabs);
    m_pMainLayoutH->addLayout(m_pCenterButtonsLayoutV);
    m_pMainLayoutH->addLayout(m_pRightLayoutV);

    // Main GroupBox:
    m_pMainGroupBox->setLayout(m_pMainLayoutH);

    // Top layout:
    m_pTopLayoutV->addWidget(m_pDescription, 0, Qt::AlignLeft);
    m_pTopLayoutV->addWidget(m_pMainGroupBox, 0, Qt::AlignLeft);
    m_pTopLayoutV->addLayout(m_pBottomButtonsLayoutH);
    //m_pTopLayoutV->addWidget(m_pButtonBox, 0, Qt::AlignRight);

    setLayout(m_pTopLayoutV);
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::setInitialValues
// Description: Set Initial values of the different break panes
// Return Val:  void
// Author:      Yoni Rabin
// Date:        6/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::setInitialValues()
{
    // Initialize the left list control functions list:
    initAPIFunctionsList();

    // Initialize the kernel list control function list:
    initKernelList();

    // Initialize the generic breakpoints list:
    initGenericBreakpointsList();

    // Apply the text filter on the left list control:
    m_pFunctionsFilter->initialize(m_pAPIList);
    m_pKernelFilter->initialize(m_pKernelList);

    // Mark the currently active breakpoints:
    setDialogActiveBreakpoints();
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::buildFinalLayout
// Description: Connects the signals to slots
// Return Val:  void
// Author:      Yoni Rabin
// Date:        22/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::buildFinalLayout()
{
    bool rc = connect(m_pFunctionsFilter, SIGNAL(textChanged(const QString&)), this, SLOT(onBreakpointsFilterTextChanged(const QString&)));
    GT_ASSERT(rc);
    rc = connect(m_pFunctionsFilter, SIGNAL(focused(bool)), this, SLOT(onFunctionsFilterFocused(bool)));
    GT_ASSERT(rc);
    rc = connect(m_pKernelFilter, SIGNAL(textChanged(const QString&)), this, SLOT(onKernelFilterTextChanged(const QString&)));
    GT_ASSERT(rc);
    rc = connect(m_pKernelFilter, SIGNAL(focused(bool)), this, SLOT(onKernelFilterFocused(bool)));
    GT_ASSERT(rc);
    rc = connect(m_pChosenList, SIGNAL(removingRow(int)), this, SLOT(onBeforeRemoveRow(int)));
    GT_ASSERT(rc);
    rc = connect(m_pChosenList, SIGNAL(afterRemovingRow(int)), this, SLOT(onAfterRemoveRow(int)));
    GT_ASSERT(rc);
    rc = connect(m_pAPIList, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onAdd()));
    GT_ASSERT(rc);
    rc = connect(m_pKernelList, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onAdd()));
    GT_ASSERT(rc);
    rc = connect(m_pGenericBreakpointsList, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onAdd()));
    GT_ASSERT(rc);
    rc = connect(m_pChosenList, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(onChosenBreakpointDoubleClicked(QTableWidgetItem*)));
    GT_ASSERT(rc);
    rc = connect(m_pChosenList, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(onChosenListKeyDown(QKeyEvent*)));
    GT_ASSERT(rc);
    rc = connect(m_pCheckBox, SIGNAL(clicked(bool)), this, SLOT(onSelectDeselectAll(bool)));
    GT_ASSERT(rc);
    rc = connect(m_pChosenList, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onChosenItemChanged(QTableWidgetItem*)));
    GT_ASSERT(rc);
    rc = connect(m_pAPIList, SIGNAL(itemSelectionChanged()), this, SLOT(onLeftListSelectionChanged()));
    GT_ASSERT(rc);
    rc = connect(m_pKernelList, SIGNAL(itemSelectionChanged()), this, SLOT(onLeftListSelectionChanged()));
    GT_ASSERT(rc);
    rc = connect(m_pGenericBreakpointsList, SIGNAL(itemSelectionChanged()), this, SLOT(onLeftListSelectionChanged()));
    GT_ASSERT(rc);
    rc = connect(m_pChosenList, SIGNAL(itemSelectionChanged()), this, SLOT(onRightListSelectionChanged()));
    GT_ASSERT(rc);
    rc = connect(m_pTabs, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onChosenListKeyDown
// Description: Responds to delete and enter
// Arguments:   QKeyEvent* pEvent
// Return Val:  void
// Author:      Yoni Rab
// Date:        24/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onChosenListKeyDown(QKeyEvent* pEvent)
{
    GT_IF_WITH_ASSERT(pEvent != NULL)
    {
        if (Qt::Key_Delete == pEvent->key() || Qt::Key_Backspace == pEvent->key())
        {
            if (m_pChosenList->rowCount() <= 1)
            {
                pEvent->ignore();
            }
            else
            {
                onRemove();
            }
        }

        setButtonStates();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onTabChanged
// Description: even which occurs when user changes tabs
// Arguments:   int index
// Author:      Yoni Rabin
// Date:        27/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onTabChanged(int index)
{
    switch (index)
    {
        case 0:
            m_pFunctionsFilter->setFocus(Qt::TabFocusReason);
            break;

        case 1:
            m_pKernelFilter->setFocus(Qt::TabFocusReason);
            break;

        case 2:
            m_pGenericBreakpointsList->setFocus(Qt::TabFocusReason);
            break;

        default:
            break;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onFunctionsFilterFocused
// Description: If the filter has been clicked while it is in initial state, remove text
// Return Val:  void
// Author:      Yoni Rabin
// Date:        27/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onFunctionsFilterFocused(bool hasFocus)
{
    GT_IF_WITH_ASSERT(m_pFunctionsFilter != NULL)
    {
        if (hasFocus)
        {
            if (m_pFunctionsFilter->isDefaultString())
            {
                m_pFunctionsFilter->clear();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onKernelFilterFocused
// Description: If the filter has been clicked while it is in initial state, remove text
// Return Val:  void
// Author:      Yoni Rabin
// Date:        27/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onKernelFilterFocused(bool hasFocus)
{
    if (hasFocus)
    {
        if (m_pKernelFilter->isDefaultString())
        {
            m_pKernelFilter->clear();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onAdd
// Description: Event handler for the "Add breakpoint" button
// Author:      Sigal Algranaty
// Date:        6/7/2011
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onAdd()
{
    // Get the active list from the notebook:
    acListCtrl* pActiveList = getActiveList();
    // Sanity Check:
    pActiveList->blockSignals(true);
    GT_IF_WITH_ASSERT(pActiveList != NULL && m_pChosenList != NULL)
    {
        // Remove the kernel function name edit line:
        removeKernelEditLine();

        // Iterate over the selected items in the left list:
        foreach (QTableWidgetItem* pItem, pActiveList->selectedItems())
        {
            // Get the item data:
            gdBreakpointsItemData* pItemData2 = (gdBreakpointsItemData*)pActiveList->getItemData(pItem->row());
            GT_ASSERT(pItemData2 != NULL);

            gdBreakpointsItemData* pItemData = getBreakpointData(pItem);
            GT_IF_WITH_ASSERT(pItemData != NULL)
            {
                // Get the item name:
                gtString funcName;
                funcName.fromASCIIString(pItem->text().toLatin1());

                // Add the breakpoint to the right list control. The default Checked status is true:
                addBreakpointToRightListCtrl(pItemData, funcName, true);

                // Set the left list item color to blue, indicating that it is in the right list:
                pItem->setTextColor(Qt::blue);
            }
        }
    }
    // Add the kernel edit line back to the chosen list;
    addKernelEditLine();

    // Update the select / deselect all checkBox
    updateSelectAllCheckBoxStatus();

    setButtonStates();
    pActiveList->blockSignals(false);
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onChosenBreakpointDoubleClicked
// Description: Is called when the user double clicks a breakpoint in
//              the right list control.
// Author:      Yoni Rabin
// Date:        18/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onChosenBreakpointDoubleClicked(QTableWidgetItem* pItem)
{
    GT_IF_WITH_ASSERT(m_pChosenList != NULL)
    {
        if (pItem->column() == 0)
        {
            if (pItem != m_pLastChosenRow)
            {
                Qt::CheckState state = pItem->checkState();
                pItem->setCheckState(state == Qt::Checked ? Qt::Unchecked : Qt::Checked);
                updateSelectAllCheckBoxStatus();
            }
            else
            {
                // Empty the text item:
                m_pChosenList->blockSignals(true);
                pItem->setTextColor(Qt::black);
                pItem->setText(AF_STR_EmptyA);
                pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
                m_pChosenList->blockSignals(false);
                m_pChosenList->setFocus(Qt::MouseFocusReason);
                m_pChosenList->editItem(pItem);
                m_LastChosenRowOnEdit = true;
                // Once last row is on edit, connect focus change event to catch the row exit event
                bool rc = connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(onFocusChange(QWidget*, QWidget*)));
                GT_ASSERT(rc);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onRemove
// Description: Is called when the user press the "Remove" breakpoint button.
// Return Val:  void
// Author:      Yoni Rabin
// Date:        29/5/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onRemove()
{
    // Sanity Check:
    GT_IF_WITH_ASSERT(m_pChosenList != NULL)
    {
        m_pChosenList->blockSignals(true);

        foreach (QTableWidgetItem* pItem, m_pChosenList->selectedItems())
        {
            // because this row contains just text and not a BP object but is selected, we need to break in order not to remove a null pointer and not to get caught in an endless loop:
            GT_IF_WITH_ASSERT(pItem != NULL)
            {
                if (pItem == m_pLastChosenRow)
                {
                    break;
                }

                if (pItem->column() > 0)
                {
                    continue;
                }

                // Get the item data:
                gdBreakpointsItemData* pRemovedBreakpointItemData = getBreakpointData(pItem);

                // Find the list control that contain the breakpoint:
                acListCtrl* pListCtrl = NULL;

                if (pRemovedBreakpointItemData->_breakpointType == OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT)
                {
                    pListCtrl = m_pAPIList;
                }
                else if (pRemovedBreakpointItemData->_breakpointType == OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT)
                {
                    pListCtrl = m_pKernelList;
                }
                else if (pRemovedBreakpointItemData->_breakpointType == OS_TOBJ_ID_GENERIC_BREAKPOINT)
                {
                    pListCtrl = m_pGenericBreakpointsList;
                }

                // Find the item of the left list with the same item data object:
                GT_IF_WITH_ASSERT(pListCtrl != NULL)
                {
                    // Get the amount of items in this list:
                    int amountOfItems = pListCtrl->rowCount();

                    for (int i = 0 ; i < amountOfItems ; i++)
                    {
                        // Get the current item data:
                        gdBreakpointsItemData* pCurrentItemData = (gdBreakpointsItemData*)pListCtrl->getItemData(i);

                        if (pCurrentItemData == pRemovedBreakpointItemData)
                        {
                            // Changing the left list item color:
                            pListCtrl->item(i, 0)->setTextColor(Qt::black);
                            break;
                        }
                    }
                }
            }
        }

        QVector<QTableWidgetItem*> vec;

        foreach (QTableWidgetItem* pItem, m_pChosenList->selectedItems())
        {
            if (pItem->column() == 0 && pItem != m_pLastChosenRow)
            {
                vec.push_back(pItem);
            }
        }

        // Deleting the item from the right list:
        int start = vec.size();

        for (int i = start; i > 0 ; --i)
        {
            m_pChosenList->removeRow(vec[i - 1]->row());
        }

        //
        // Update the select / deselect all checkBox
        updateSelectAllCheckBoxStatus();
        setButtonStates();
        m_pChosenList->blockSignals(false);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onRemoveAll
// Description: Is called when the user press the "Remove All" breakpoints.
// Author:      Avi Shapira
// Date:        20/4/2004
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onRemoveAll()
{
    // Sanity check
    GT_IF_WITH_ASSERT((m_pChosenList != NULL) && (m_pAPIList != NULL) && (m_pKernelList != NULL) && (m_pGenericBreakpointsList != NULL))
    {
        // Remove all selected breakpoints from the right list control:
        removeKernelEditLine();
        m_pChosenList->clearList();

        // Color all left list control items in black:
        resetListColor(m_pAPIList);
        resetListColor(m_pKernelList);
        resetListColor(m_pGenericBreakpointsList);
        // Add extra line:
        addKernelEditLine();
        // Clearing the Select / Deselect all checkBox:
        updateSelectAllCheckBoxStatus();
        // Update button states:
        setButtonStates();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::addListItem
// Description: Add a new row to a list
// Arguments:   acListCtrl* pList
//              const gtString& name
//              gdBreakpointsItemData* pItemData
// Return Val:  QTableWidgetItem* - null means failure
// Author:      Yoni Rabin
// Date:        14/6/2012
// ---------------------------------------------------------------------------
QTableWidgetItem* gdBreakpointsDialog::addListItem(acListCtrl* pList, const gtString& name, gdBreakpointsItemData* pItemData)
{
    QTableWidgetItem* retVal = NULL;
    GT_IF_WITH_ASSERT(pList != NULL)
    {
        bool rc = pList->addRow(acGTStringToQString(name), (void*)pItemData, false, Qt::Unchecked, NULL, false);
        GT_IF_WITH_ASSERT(rc)
        {
            // Return the list item
            retVal = pList->item(pList->rowCount() - 1, 0);
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::addChosenListItem
// Description: Add a new row to the chosen list
// Arguments:   const gtString& name
//              gdBreakpointsItemData* pItemData
// Return Val:  QTableWidgetItem* - null means failure
// Author:      Yoni Rabin
// Date:        14/6/2012
// ---------------------------------------------------------------------------
QTableWidgetItem* gdBreakpointsDialog::addChosenListItem(const QString& name, const QString& type, gdBreakpointsItemData* pItemData, bool checked)
{
    QTableWidgetItem* retVal = NULL;
    GT_IF_WITH_ASSERT(m_pChosenList != NULL)
    {
        bool isLastRow = (pItemData == NULL);
        QStringList str;
        str << name << (isLastRow ? "" : type);

        bool rc = m_pChosenList->addRow(str, pItemData, !isLastRow, checked ? Qt::Checked : Qt::Unchecked);
        GT_IF_WITH_ASSERT(rc)
        {
            retVal = m_pChosenList->item(m_pChosenList->rowCount() - 1, 0);
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::initAPIFunctionsList
// Description: Set the monitored functions into the left dialog list control.
// Return Val:  bool - Success / failure.
// Author:      Yoni Rabin
// Date:        14/6/2012
// ---------------------------------------------------------------------------
bool gdBreakpointsDialog::initAPIFunctionsList()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_pAPIList)
    {
        // Clear the current list content:
        m_pAPIList->clearList();

        // Get the amount of all monitored functions
        bool rc1 = gaGetAmountOfMonitoredFunctions(m_amountOfMonitoredFunctions);
        GT_IF_WITH_ASSERT(rc1)
        {
            retVal = true;

            // Get the monitored functions mask according to the current project type:
            unsigned int functionsMask = getMonitoredFunctionsFilterByCurrentProjectType();

            // Iterate the monitored functions:
            gtString functionName;

            for (int functionID = 0; functionID < m_amountOfMonitoredFunctions; functionID++)
            {
                // Get the function type (OpenGL / OpenGL ES):
                unsigned int functionType = 0;
                bool rc2 = gaGetMonitoredFunctionAPIType((apMonitoredFunctionId)functionID, functionType);
                GT_IF_WITH_ASSERT(rc2)
                {
                    // If the function pass the type filter:
                    if (functionType & functionsMask)
                    {
                        // Get the function name
                        bool rc3 = gaGetMonitoredFunctionName((apMonitoredFunctionId)functionID, functionName);
                        GT_IF_WITH_ASSERT(rc3)
                        {
                            // Prepare the data item:
                            gdBreakpointsItemData* pItemData = new gdBreakpointsItemData;

                            // Set the breakpoint type:
                            pItemData->_breakpointType = OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT;
                            pItemData->_monitoredFunctionId = (apMonitoredFunctionId)functionID;
                            // Add the item to the list:
                            QTableWidgetItem* pItem = addListItem(m_pAPIList, functionName, pItemData);
                            GT_ASSERT(pItem != NULL);
                        }
                    }
                }
            }

            // Sort the list items:
            m_pAPIList->sortByColumn(0, Qt::AscendingOrder);
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::initKernelList
// Description: Fill the list control with kernel functions names
// Return Val:  void
// Author:      Yoni Rabin
// Date:        17/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::initKernelList()
{
    int currentContextsAmount = 0;

    // Get amount of OpenCL contexts:
    bool rc = gaGetAmountOfOpenCLContexts(currentContextsAmount);

    for (int nContext = 1; nContext < currentContextsAmount; nContext++)
    {
        // Get amount of program objects:
        int numberOfPrograms = 0;
        rc = gaGetAmountOfOpenCLProgramObjects(nContext, numberOfPrograms);
        GT_IF_WITH_ASSERT(rc)
        {
            for (int nProgram = 0; nProgram < numberOfPrograms; nProgram++)
            {
                // Get the program details:
                apCLProgram currentProgram(0);
                rc = gaGetOpenCLProgramObjectDetails(nContext, nProgram, currentProgram);

                if (!currentProgram.wasMarkedForDeletion())
                {
                    const gtVector<oaCLKernelHandle>& progKernelHandles = currentProgram.kernelHandles();

                    // Iterate the kernelHandles:
                    int numberOfKernels = (int)progKernelHandles.size();

                    for (int nKernel = 0; nKernel < numberOfKernels; nKernel++)
                    {
                        // Get the kernel details:
                        apCLKernel kernelDetails;
                        bool rcKernel = gaGetOpenCLKernelObjectDetails(nContext, progKernelHandles[nKernel], kernelDetails);
                        GT_IF_WITH_ASSERT(rcKernel)
                        {
                            gtString kernelDisplayName = kernelDetails.kernelFunctionName();

                            GT_IF_WITH_ASSERT(m_pKernelList)
                            {
                                // Check that function name dose not already exists in the list:
                                int numberItems = m_pKernelList->rowCount();
                                bool itemFound = false;

                                for (int nItems = 0 ; nItems < numberItems ; nItems++)
                                {
                                    gtString curItemName;
                                    bool rc1 = m_pKernelList->getItemText(nItems, 0, curItemName);
                                    GT_IF_WITH_ASSERT(rc1)
                                    {
                                        if (curItemName == kernelDisplayName)
                                        {
                                            itemFound = true;
                                            break;
                                        }
                                    }
                                }

                                if (!itemFound)
                                {
                                    // Prepare the data item:
                                    gdBreakpointsItemData* pItemData = new gdBreakpointsItemData;

                                    // Set the breakpoint type:
                                    pItemData->_breakpointType = OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT;
                                    pItemData->_kernelFunctionName = kernelDisplayName;
                                    // Add the item to the list:
                                    QTableWidgetItem* pItem = addListItem(m_pKernelList, kernelDisplayName, pItemData);
                                    bool rc2 = (NULL != pItem);
                                    GT_ASSERT(rc2);
                                }
                            }
                        }
                    }

                    // Sort the list items:
                    m_pKernelList->sortByColumn(0, Qt::AscendingOrder);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::initGenericBreakpointsList
// Description: Initialize the list of generic breakpoints
// Return Val:  bool - Success / failure.
// Author:      Yoni Rabin
// Date:        14/6/2012
// ---------------------------------------------------------------------------
bool gdBreakpointsDialog::initGenericBreakpointsList()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_pGenericBreakpointsList != NULL)
    {
        // Clear the current list content:
        m_pGenericBreakpointsList->clearList();

        // Iterate the generic breakpoints:
        for (int i = (int)AP_BREAK_ON_GL_ERROR;  i < (int)AP_AMOUNT_OF_GENERIC_BREAKPOINT_TYPES; i++)
        {
            // Convert the integer into a breakpoint type enumeration:
            apGenericBreakpointType breakType = (apGenericBreakpointType)i;

            gtString breakpointName;
            bool rcGetName = apGenericBreakpoint::breakpointTypeToString(breakType, breakpointName);
            GT_IF_WITH_ASSERT(rcGetName)
            {
                // Prepare the data item:
                gdBreakpointsItemData* pItemData = new gdBreakpointsItemData;

                // Set the breakpoint type:
                pItemData->_breakpointType = OS_TOBJ_ID_GENERIC_BREAKPOINT;
                pItemData->_genericBreakpointType = breakType;
                // Add the item to the list:
                QTableWidgetItem* pItem = addListItem(m_pGenericBreakpointsList, breakpointName, pItemData);
                GT_ASSERT(pItem != NULL);
            }
        }

        // Sort the list items:
        m_pGenericBreakpointsList->sortByColumn(0, Qt::AscendingOrder);
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::setDialogActiveBreakpoints
// Description: Add the active breakpoints into the dialog items.
// Return Val:  void
// Author:      Yoni Rabin
// Date:        19/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::setDialogActiveBreakpoints()
{
    // Add the active breakpoints to the right list control:
    addActiveBreakpointsToRightListCtrl();
    // Color the active breakpoints in the left list control:
    colorActiveBreakpointsInLeftListCtrl();
    // Set the Select / Deselect state (do not ignore last dummy item since it was not inserted yet):
    updateSelectAllCheckBoxStatus(false);
    // Initialize
    setButtonStates();
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::addActiveBreakpointsToRightListCtrl
// Description: Adds the active breakpoints to the right list control,
//                And set the Select / Deselect all CheckBox status.
// Author:      Yaki Tebeka
// Date:        20/11/2007
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::addActiveBreakpointsToRightListCtrl()
{
    // Get the amount of active breakpoints:
    int amountOfBreakpoints = 0;
    bool rc1 = gaGetAmountOfBreakpoints(amountOfBreakpoints);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Iterate on the active breakpoints
        for (int i = 0; i < amountOfBreakpoints; i++)
        {
            // Get the current breakpoint
            gtAutoPtr<apBreakPoint> aptrBreakpoint;
            bool rc2 = gaGetBreakpoint(i, aptrBreakpoint);
            GT_IF_WITH_ASSERT(rc2)
            {
                // Get the item data matching the current breakpoint:
                QTableWidgetItem* pItem = findBreakpointMatchingItem(aptrBreakpoint);

                if (pItem != NULL)
                {
                    gdBreakpointsItemData* pBreakpointData = getBreakpointData(pItem);

                    if (pBreakpointData != NULL)
                    {
                        // If the breakpoint exist, add it to the right size:
                        gtString breakpointName;
                        addBreakpointToRightListCtrl(pBreakpointData, breakpointName, aptrBreakpoint.pointedObject()->isEnabled());
                        // Set the breakpoint hit count:
                        pBreakpointData->_hitCount = aptrBreakpoint->hitCount();
                    }
                }
                else
                {
                    if (aptrBreakpoint->type() == OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT)
                    {
                        // It must be kernel - add it to the right list control:
                        addKernelToRightListCtrl(aptrBreakpoint);
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::findBreakpointMatchingItemData
// Description: Given an API breakpoint - find the item data that is related to
//              the item matching item in the left list controls
// Arguments:   gtAutoPtr<apBreakPoint>& aptrBreakpoint
// Return Val:  gdBreakpointsItemData*
// Author:      Yoni Rabin
// Date:        17/6/2012
// ---------------------------------------------------------------------------
gdBreakpointsItemData* gdBreakpointsDialog::findBreakpointMatchingItemData(gtAutoPtr<apBreakPoint>& aptrBreakpoint)
{
    gdBreakpointsItemData* pRetVal = NULL;

    // Get the breakpoint type:
    osTransferableObjectType breakpointType = aptrBreakpoint->type();

    // API BP:
    if (breakpointType == OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT)
    {
        // Down cast it to apMonitoredFunctionBreakPoint:
        apMonitoredFunctionBreakPoint* pFunctionBreakpoint = (apMonitoredFunctionBreakPoint*)(aptrBreakpoint.pointedObject());
        GT_IF_WITH_ASSERT(pFunctionBreakpoint != NULL)
        {
            GT_IF_WITH_ASSERT(m_pAPIList != NULL)
            {
                int numItems = m_pAPIList->rowCount();

                for (int i = 0 ; i < numItems; ++i)
                {
                    // Get the current item data:
                    gdBreakpointsItemData* pBreakpointData = (gdBreakpointsItemData*)m_pAPIList->getItemData(i);
                    GT_IF_WITH_ASSERT(pBreakpointData != NULL)
                    {
                        if (pBreakpointData->_monitoredFunctionId == pFunctionBreakpoint->monitoredFunctionId())
                        {
                            // The breakpoint is found:
                            pRetVal = pBreakpointData;
                            break;
                        }
                    }
                }
            }
        }
    }
    // Kernel function BP:
    else if (breakpointType == OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT)
    {
        // Down cast it to kernel breakpoint:
        apKernelFunctionNameBreakpoint* pKernelBreakpoint = (apKernelFunctionNameBreakpoint*)(aptrBreakpoint.pointedObject());
        GT_IF_WITH_ASSERT(pKernelBreakpoint != NULL)
        {
            GT_IF_WITH_ASSERT(m_pKernelList != NULL)
            {
                int numItems = m_pKernelList->rowCount();

                for (int i = 0 ; i < numItems; i++)
                {
                    // Get the current item data:
                    gdBreakpointsItemData* pBreakpointData = (gdBreakpointsItemData*)m_pKernelList->getItemData(i);
                    GT_IF_WITH_ASSERT(pBreakpointData != NULL)
                    {
                        if (pBreakpointData->_kernelFunctionName == pKernelBreakpoint->kernelFunctionName())
                        {
                            // The breakpoint is found:
                            pRetVal = pBreakpointData;
                            break;
                        }
                    }
                }
            }
        }
    }
    // Generic BP:
    else if (breakpointType == OS_TOBJ_ID_GENERIC_BREAKPOINT)
    {
        // Down-cast to generic breakpoint:
        apGenericBreakpoint* pGenericBreakpoint = (apGenericBreakpoint*)(aptrBreakpoint.pointedObject());
        GT_IF_WITH_ASSERT(pGenericBreakpoint != NULL)
        {
            GT_IF_WITH_ASSERT(m_pGenericBreakpointsList != NULL)
            {
                int numItems = m_pKernelList->rowCount();

                for (int i = 0 ; i < numItems; i++)
                {
                    // Get the current item data:
                    gdBreakpointsItemData* pBreakpointData = (gdBreakpointsItemData*)m_pGenericBreakpointsList->getItemData(i);
                    GT_IF_WITH_ASSERT(pBreakpointData != NULL)
                    {
                        if (pBreakpointData->_genericBreakpointType == pGenericBreakpoint->breakpointType())
                        {
                            // The breakpoint is found:
                            pRetVal = pBreakpointData;
                            break;
                        }
                    }
                }
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::findBreakpointMatchingItem
// Description: Given an API breakpoint - find the item that is related to
//              the item matching item in the left list controls
// Arguments:   gtAutoPtr<apBreakPoint>& aptrBreakpoint
// Return Val:  QTableWidgetItem*
// Author:      Yoni Rabin
// Date:        17/6/2012
// ---------------------------------------------------------------------------
QTableWidgetItem* gdBreakpointsDialog::findBreakpointMatchingItem(gtAutoPtr<apBreakPoint>& aptrBreakpoint)
{
    QTableWidgetItem* pRetVal = NULL;

    // Get the breakpoint type:
    osTransferableObjectType breakpointType = aptrBreakpoint->type();

    // API BP:
    if (breakpointType == OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT)
    {
        // Down cast it to apMonitoredFunctionBreakPoint:
        apMonitoredFunctionBreakPoint* pFunctionBreakpoint = (apMonitoredFunctionBreakPoint*)(aptrBreakpoint.pointedObject());
        GT_IF_WITH_ASSERT(pFunctionBreakpoint != NULL)
        {
            GT_IF_WITH_ASSERT(m_pAPIList != NULL)
            {
                int numItems = m_pAPIList->rowCount();

                for (int i = 0 ; i < numItems; ++i)
                {
                    QTableWidgetItem* pItem = m_pAPIList->item(i, 0);
                    // Get the current item data:
                    gdBreakpointsItemData* pBreakpointData = getBreakpointData(pItem);
                    GT_IF_WITH_ASSERT(pBreakpointData != NULL)
                    {
                        if (pBreakpointData->_monitoredFunctionId == pFunctionBreakpoint->monitoredFunctionId())
                        {
                            // The breakpoint is found:
                            pRetVal = pItem;
                            break;
                        }
                    }
                }
            }
        }
    }
    // Kernel function BP:
    else if (breakpointType == OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT)
    {
        // Down cast it to kernel breakpoint:
        apKernelFunctionNameBreakpoint* pKernelBreakpoint = (apKernelFunctionNameBreakpoint*)(aptrBreakpoint.pointedObject());
        GT_IF_WITH_ASSERT(pKernelBreakpoint != NULL)
        {
            GT_IF_WITH_ASSERT(m_pKernelList != NULL)
            {
                int numItems = m_pKernelList->rowCount();

                for (int i = 0 ; i < numItems; i++)
                {
                    QTableWidgetItem* pItem = m_pKernelList->item(i, 0);
                    gdBreakpointsItemData* pBreakpointData = getBreakpointData(pItem);
                    GT_IF_WITH_ASSERT(pBreakpointData != NULL)
                    {
                        if (pBreakpointData->_kernelFunctionName == pKernelBreakpoint->kernelFunctionName())
                        {
                            // The breakpoint is found:
                            pRetVal = pItem;
                            break;
                        }
                    }
                }
            }
        }
    }
    // Generic BP:
    else if (breakpointType == OS_TOBJ_ID_GENERIC_BREAKPOINT)
    {
        // Down-cast to generic breakpoint:
        apGenericBreakpoint* pGenericBreakpoint = (apGenericBreakpoint*)(aptrBreakpoint.pointedObject());
        GT_IF_WITH_ASSERT(pGenericBreakpoint != NULL)
        {
            GT_IF_WITH_ASSERT(m_pGenericBreakpointsList != NULL)
            {
                int numItems = m_pGenericBreakpointsList->rowCount();

                for (int i = 0 ; i < numItems; i++)
                {
                    QTableWidgetItem* pItem = m_pGenericBreakpointsList->item(i, 0);
                    gdBreakpointsItemData* pBreakpointData = getBreakpointData(pItem);
                    GT_IF_WITH_ASSERT(pBreakpointData != NULL)
                    {
                        if (pBreakpointData->_genericBreakpointType == pGenericBreakpoint->breakpointType())
                        {
                            // The breakpoint is found:
                            pRetVal = pItem;
                            break;
                        }
                    }
                }
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::colorActiveBreakpointsInLeftListCtrl
// Description: Colors the active breakpoints in the left list control.
// Return Val:  void
// Author:      Yoni Rabin
// Date:        20/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::colorActiveBreakpointsInLeftListCtrl()
{
    // Iterate the right list control selected breakpoints:
    int selectedBreakpointsAmount = m_pChosenList->rowCount();

    for (int i = 0; i < selectedBreakpointsAmount; i++)
    {
        // Get the current left list item data:
        gdBreakpointsItemData* pSelectedItemData = (gdBreakpointsItemData*)(m_pChosenList->getItemData(i));

        if (pSelectedItemData != NULL)
        {
            // Get the active list from the notebook:
            acListCtrl* pActiveList = NULL;

            if (pSelectedItemData->_breakpointType == OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT)
            {
                pActiveList = m_pAPIList;
            }
            else if (pSelectedItemData->_breakpointType == OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT)
            {
                pActiveList = m_pKernelList;
            }
            else if (pSelectedItemData->_breakpointType == OS_TOBJ_ID_GENERIC_BREAKPOINT)
            {
                pActiveList = m_pGenericBreakpointsList;
            }

            if (pActiveList != NULL)
            {
                // Iterate the left list control:
                int amountOfListItems = pActiveList->rowCount();

                for (int j = 0; j < amountOfListItems; j++)
                {
                    // Get the current left list item data:
                    QTableWidgetItem* pItem = pActiveList->item(j, 0);
                    gdBreakpointsItemData* pItemData = getBreakpointData(pItem);

                    if (pItemData != NULL)
                    {
                        // If the current left list item represents the same function as the
                        // current breakpoint represents:
                        if (pItemData == pSelectedItemData)
                        {
                            // Changing the left list item color:
                            pItem->setTextColor(Qt::blue);
                            break;
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::addBreakpointToRightListCtrl
// Description: Add a breakpoint to the right list control (_pChosenBreakpointsList)
// Arguments:
//   functionId - The monitored function id.
//   functionName - The monitored function name.
//     checkStatus - The Checked / Unchecked status
// Author:      Avi Shapira
// Date:        20/4/2004
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::addBreakpointToRightListCtrl(const gdBreakpointsItemData* pBreakpointItemData, gtString& breakpointName, bool checkStatus)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pBreakpointItemData != NULL)
    {
        if (breakpointName.isEmpty())
        {
            if (pBreakpointItemData->_breakpointType == OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT)
            {
                // Get the breakpoint name:
                bool rc = gaGetMonitoredFunctionName(pBreakpointItemData->_monitoredFunctionId, breakpointName);
                GT_ASSERT(rc);
            }
            else if (pBreakpointItemData->_breakpointType == OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT)
            {
                breakpointName = pBreakpointItemData->_kernelFunctionName;
            }
            else if (pBreakpointItemData->_breakpointType == OS_TOBJ_ID_GENERIC_BREAKPOINT)
            {
                // Get the breakpoint name:
                bool rc = apGenericBreakpoint::breakpointTypeToString(pBreakpointItemData->_genericBreakpointType, breakpointName);
                GT_ASSERT(rc);
            }
        }

        QTableWidgetItem* pBreakPointItem = NULL;
        // Check whether the specified function was not already chosen:
        bool isAlreadyChosen = isBreakpointChosen(pBreakpointItemData, pBreakPointItem);

        if (!isAlreadyChosen)
        {
            gdBreakpointsItemData* pNewBreakpointItemData = (gdBreakpointsItemData*)pBreakpointItemData;

            if (pNewBreakpointItemData == NULL)
            {
                // Allocate a new item data:
                pNewBreakpointItemData = new gdBreakpointsItemData;

                // Set the details for the item data:
                pNewBreakpointItemData->_kernelFunctionName = breakpointName;
                pNewBreakpointItemData->_breakpointType = OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT;

                // Add this item data for later deletion:
                m_rightListBreakpointsDataVector.push_back(pNewBreakpointItemData);
            }

            QString breakpointType;
            getBreakpointTypeString(pNewBreakpointItemData, breakpointType);
            // Add the item into the chosen breakpoints list:
            QTableWidgetItem* pItem = addChosenListItem(acGTStringToQString(breakpointName), breakpointType, pNewBreakpointItemData, checkStatus);
            GT_ASSERT(pItem != NULL);
        }
        else
        {
            GT_IF_WITH_ASSERT(pBreakPointItem != NULL)
            {
                pBreakPointItem->setCheckState(Qt::Checked);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::removeSelectedBreakpoint
// Description: Removes the chosen breakpoint from the right list and reset the left item's color (if present)
// Arguments:   QTableWidgetItem* pItem
// Return Val:  void
// Author:      Yoni Rabin
// Date:        19/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::removeSelectedBreakpoint(QTableWidgetItem* pItem)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pChosenList != NULL) && (pItem != NULL))
    {
        // Get the item data:
        gdBreakpointsItemData* pRemovedBreakpointItemData = getBreakpointData(pItem);

        // Find the list control that contain the breakpoint:
        acListCtrl* pListCtrl = NULL;

        if (pRemovedBreakpointItemData->_breakpointType == OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT)
        {
            pListCtrl = m_pAPIList;
        }
        else if (pRemovedBreakpointItemData->_breakpointType == OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT)
        {
            pListCtrl = m_pKernelList;
        }
        else if (pRemovedBreakpointItemData->_breakpointType == OS_TOBJ_ID_GENERIC_BREAKPOINT)
        {
            pListCtrl = m_pGenericBreakpointsList;
        }

        // Find the item of the right list with the same item data object:
        GT_IF_WITH_ASSERT(pListCtrl != NULL)
        {
            // Get the amount of items in this list:
            (void) m_pAPIList->rowCount();
            int amountOfItems = pListCtrl->rowCount();

            for (int i = 0 ; i < amountOfItems ; i++)
            {
                // Get the current item data:
                gdBreakpointsItemData* pCurrentItemData = (gdBreakpointsItemData*)pListCtrl->getItemData(i);

                if (pCurrentItemData == pRemovedBreakpointItemData)
                {
                    // Changing the left list item color:
                    pListCtrl->item(i, 0)->setTextColor(Qt::black);
                    break;
                }
            }
        }
        // Deleting the item from the right list:
        m_pChosenList->removeRow(pItem->row());
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onBreakpointsFilterTextChanged
// Description: Process the filter-change event
// Arguments:   const QString & filterText
// Return Val:  void
// Author:      Yoni Rabin
// Date:        19/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onBreakpointsFilterTextChanged(const QString& filterText)
{
    (void)(filterText); // unused
    colorActiveBreakpointsInLeftListCtrl();
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onKernelFilterTextChanged
// Description: Process the filter-change event
// Arguments:   const QString & filterText
// Return Val:  void
// Author:      Yoni Rabin
// Date:        19/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onKernelFilterTextChanged(const QString& filterText)
{
    (void)(filterText); // unused
    colorActiveKernelInLeftListCtrl();
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::getMonitoredFunctionsFilterByCurrentProjectType
// Description: Returns the monitored functions mask as defined by the current
//              application project type.
// Return Val: unsigned int - Will get the monitored functions mask.
// Author:      Yaki Tebeka
// Date:        20/11/2007
// ---------------------------------------------------------------------------
unsigned int gdBreakpointsDialog::getMonitoredFunctionsFilterByCurrentProjectType()
{
    unsigned int functionsMask = 0;
    unsigned int platformSpecificMask = 0;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    platformSpecificMask |= (AP_WGL_FUNC | AP_WGL_EXTENSION_FUNC);
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
    // Add no more other than glX.
    platformSpecificMask |= (AP_GLX_FUNC | AP_GLX_EXTENSION_FUNC);
#elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
    platformSpecificMask |= (AP_CGL_FUNC /* | AP_CGL_EXTENSION_FUNC */);
#else
#error Unknown Linux variant
#endif
#else
#error Unknown configuration!
#endif

    functionsMask |= (platformSpecificMask | AP_OPENGL_GENERIC_FUNC | AP_OPENGL_EXTENSION_FUNC);

    // Add gremedy OpenGL functions:
    functionsMask |= AP_OPENGL_GREMEDY_EXTENSION_FUNC;
    functionsMask |= (AP_OPENCL_GENERIC_FUNC | AP_OPENCL_EXTENSION_FUNC);
    // Add gremedy OpenCL extension functions:
    functionsMask |= AP_OPENCL_AMD_EXTENSION_FUNC;

    return functionsMask;
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::isBreakpointChosen
// Description: Checks whether the selected function (identified by it's id)
//                is already in the right list.
// Arguments:   const gdBreakpointsItemData* pBreakpointItemData
//              QTableWidgetItem& pRetItem
// Return Val:  bool - Success / failure.
// Author:      Yoni Rabin
// Date:        19/6/2012
// ---------------------------------------------------------------------------
bool gdBreakpointsDialog::isBreakpointChosen(const gdBreakpointsItemData* pBreakpointItemData, QTableWidgetItem*& pRetItem)
{
    bool retVal = false;
    pRetItem = NULL;
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pChosenList != NULL)
    {
        // Init the index:
        int numItems = m_pChosenList->rowCount();

        // Iterate over the right list
        for (int i = 0; i < numItems; ++i)
        {
            gdBreakpointsItemData* pCurrentItemData = (gdBreakpointsItemData*)m_pChosenList->getItemData(i);
            GT_IF_WITH_ASSERT(pCurrentItemData != NULL)
            {
                // Compare item data:
                if (pCurrentItemData == pBreakpointItemData)
                {
                    pRetItem = m_pChosenList->item(i, 0);
                    retVal = true;
                    break;
                }
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        void gdBreakpointsDialog::onSelectDeselectAll
// Description: Called upon clicking the Select / Deselect All checkBox.
//                If the Status was undefined - it turns into Checked.
// Author:      Guy Ilany
// Date:        2007/12/03
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onSelectDeselectAll(bool checked)
{
    Qt::CheckState newState = checked ? Qt::Checked : Qt::Unchecked; //m_pCheckBox->checkState();
    GT_IF_WITH_ASSERT(newState != Qt::PartiallyChecked)
    {
        int itemCount = m_pChosenList->rowCount() - 1;

        for (int i = 0; i < itemCount; i++)
        {
            QTableWidgetItem* pItem = m_pChosenList->item(i, 0);
            GT_IF_WITH_ASSERT(pItem != NULL)
            {
                pItem->setCheckState(newState);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::updateSelectAllCheckBoxStatus
// Description: Updates the Select / Deselect all checkBox. In case all the list items have the same checked state,
//                We set the checkBox to the same state, otherwise - we set it to undefined state.
// Arguments:   bool ignoreLastItem - in most cases we can ignore the last item which is the "type...." dummy item:
// Return Val:  void
// Author:      Yoni Rabin
// Date:        18/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::updateSelectAllCheckBoxStatus(bool ignoreLastItem /* = true */)
{
    if (!m_updatingCheckStatus)
    {
        m_updatingCheckStatus = true;
        GT_IF_WITH_ASSERT(m_pCheckBox != NULL && m_pChosenList != NULL)
        {
            // The default checked state is unchecked (in case the list is empty)
            Qt::CheckState newState = Qt::Unchecked;
            // Marks whether there are at least two items with a different checked status
            bool areDifferent = false;
            int itemCount = m_pChosenList->rowCount();

            // in most cases we can ignore the last item which is the "type...." dummy item:
            if (ignoreLastItem)
            {
                itemCount -= 1;
            }

            // Go over the selected breakpoints list
            if (itemCount > 0)
            {
                Qt::CheckState firstState = m_pChosenList->item(0, 0)->checkState();

                // Iterate over the rest of the rest of the list items and
                // check if there is another item with a different checked status
                for (int i = 1; i < itemCount; ++i)
                {
                    Qt::CheckState itemState = m_pChosenList->item(i, 0)->checkState();

                    if (itemState != firstState)
                    {
                        areDifferent = true;
                        break;
                    }
                }

                // If found two different items - set the Select / Deselect all to undefined status
                if (areDifferent)
                {
                    newState = Qt::PartiallyChecked;
                }
                else
                {
                    newState = firstState;
                }
            }

            // Set the checkBox new state
            m_pCheckBox->setCheckState(newState);
        }
        m_updatingCheckStatus = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::addKernelToRightListCtrl
// Description: Add a kernel function Item to the chosen list of functions
// Arguments:   gtAutoPtr<apBreakPoint>& aptrBreakpoint
// Return Val:  void
// Author:      Yoni Rabin
// Date:        19/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::addKernelToRightListCtrl(gtAutoPtr<apBreakPoint>& aptrBreakpoint)
{
    // This function should only be called with kernel breakpoints:
    GT_IF_WITH_ASSERT(aptrBreakpoint->type() == OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT)
    {
        // Down cast it to kernel breakpoint:
        apKernelFunctionNameBreakpoint* pKernelBreakpoint = (apKernelFunctionNameBreakpoint*)(aptrBreakpoint.pointedObject());
        GT_IF_WITH_ASSERT(pKernelBreakpoint != NULL)
        {
            // Check whether the specified function was not already chosen:
            QTableWidgetItem* pBreakPointItem = NULL;

            if (!isKernelMarkedAsBreakpoint(pKernelBreakpoint->kernelFunctionName(), pBreakPointItem))
            {
                // Item data should be in the map:
                gdBreakpointsItemData* pKernelBreakpointItemData = new gdBreakpointsItemData;

                // Set the details for the item data:
                pKernelBreakpointItemData->_kernelFunctionName = pKernelBreakpoint->kernelFunctionName();
                pKernelBreakpointItemData->_breakpointType = OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT;

                // Add this item data for deletion:
                m_rightListBreakpointsDataVector.push_back(pKernelBreakpointItemData);

                QString breakpointType;
                getBreakpointTypeString(pKernelBreakpointItemData, breakpointType);
                // Add it into the chosen breakpoints list:
                QTableWidgetItem* pItem = addChosenListItem(acGTStringToQString(pKernelBreakpointItemData->_kernelFunctionName), breakpointType, pKernelBreakpointItemData, pKernelBreakpoint->isEnabled());
                GT_ASSERT(pItem != NULL);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::getBreakpointTypeString
// Description: Gets the type screen to be put in the second column of the chosen list
// Arguments:   gdBreakpointsItemData* pNewBreapointData
//              QString& typeString
// Author:      Yoni Rabin
// Date:        20/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::getBreakpointTypeString(gdBreakpointsItemData* pNewBreapointData, QString& typeString)
{
    typeString = AF_STR_NotAvailableA;
    GT_IF_WITH_ASSERT(pNewBreapointData != NULL)
    {
        if (pNewBreapointData->_breakpointType == OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT)
        {
            if (gdIsHSAKernelName(pNewBreapointData->_kernelFunctionName))
            {
                typeString = GD_STR_BreakPointsHSAKernelType;
            }
            else
            {
                typeString = GD_STR_BreakPointsCLKernelType;
            }
        }
        else if (pNewBreapointData->_breakpointType == OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT)
        {
            apMonitoredFunctionId fid = pNewBreapointData->_monitoredFunctionId;

            if ((apFirstOpenGLFunction <= fid) && (apLastOpenGLFunction >= fid))
            {
                typeString = GD_STR_BreakPointsGLFunctionType;
            }
            else if ((apFirstOpenCLFunction <= fid) && (apLastOpenCLFunction >= fid))
            {
                typeString = GD_STR_BreakPointsCLFunctionType;
            }
            else if ((apFirstOpenGLESFunction <= fid) && (apLastOpenGLESFunction >= fid))
            {
                // OpenGL ES not currently supported;
                GT_ASSERT(false);
                typeString = GD_STR_BreakPointsGLESFunctionType;
            }
            else
            {
                // Unsupported API!
                GT_ASSERT(false);
            }
        }
        else if (pNewBreapointData->_breakpointType == OS_TOBJ_ID_GENERIC_BREAKPOINT)
        {
            typeString = GD_STR_BreakPointsGenericType;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::isKernelMarkedAsBreakpoint
// Description: checks if a kernel function name is already in breakpoint list
// Arguments:   const gtString& kernelName
//              QTableWidgetItem* pRetItem
// Return Val:  bool - Success / failure.
// Author:      Yoni Rabin
// Date:        19/6/2012
// ---------------------------------------------------------------------------
bool gdBreakpointsDialog::isKernelMarkedAsBreakpoint(const gtString& kernelName, QTableWidgetItem*& pRetItem)
{
    bool retVal = false;
    pRetItem = NULL;

    int numItems = m_pChosenList->rowCount() - 1;

    // Iterate over the right list
    for (int nItem = 0; nItem < numItems; nItem++)
    {
        QTableWidgetItem* pItem = m_pChosenList->item(nItem, 0);
        GT_IF_WITH_ASSERT(pItem != NULL)
        {
            gdBreakpointsItemData* pItemData = getBreakpointData(pItem);
            GT_IF_WITH_ASSERT(pItemData != NULL)
            {
                // Compare by function id
                if ((pItemData->_breakpointType == OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT) && (pItem->text() ==  acGTStringToQString(kernelName)))
                {
                    retVal = true;
                    pRetItem = pItem;
                    break;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::colorActiveKernelInLeftListCtrl
// Description: Color kernel functions in the list based on chosen kernelHandles
// Author:      Yoni Rabin
// Date:        19/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::colorActiveKernelInLeftListCtrl()
{
    // Iterate the right list control selected breakpoints:
    int selectedKernelsAmount = m_pChosenList->rowCount();

    for (int nChosen = 0; nChosen < selectedKernelsAmount; nChosen++)
    {
        // Get the kernel function name:
        gtString chosenName;
        m_pChosenList->getItemText(nChosen, 0, chosenName);

        // Iterate the left list control:
        int amountOfListItems = m_pKernelList->rowCount();

        for (int nKernels = 0; nKernels < amountOfListItems; nKernels++)
        {
            // Get the current left list item data:
            gtString kernelName;
            m_pKernelList->getItemText(nKernels, 0, kernelName);

            // If the current left list item represents the same function as the
            // current breakpoint represents:
            if (kernelName == chosenName)
            {
                // Changing the left list item color:
                m_pKernelList->setItemTextColor(nKernels, 0, Qt::blue);
                break;
            }
        } // for nKernel
    } // for nChosen
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::addKernelEditLine
// Description: Add the "Type.." line to the right list
// Return Val:  void
// Author:      Yoni Rabin
// Date:        21/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::addKernelEditLine()
{
    GT_IF_WITH_ASSERT(m_pChosenList)
    {
        m_pChosenList->blockSignals(true);
        // Add the "Type.." message to the kernel chosen list
        QString addMessage = GD_STR_BreakpointsKernelAddMessage;
        m_pLastChosenRow = addChosenListItem(addMessage, "", NULL, false);
        m_pLastChosenRow->setTextColor(Qt::gray);
        Qt::ItemFlags flags = m_pLastChosenRow->flags() & ~Qt::ItemIsUserCheckable;
        flags |= Qt::ItemIsEditable;
        m_pLastChosenRow->setFlags(flags);
        m_pChosenList->blockSignals(false);
    }
}

// ---------------------------------------------------------------------------
void gdBreakpointsDialog::verifyChosenListLastRow()
{
    GT_ASSERT(NULL != m_pChosenList)
    {
        if (m_LastChosenRowOnEdit)
        {
            m_pChosenList->blockSignals(true);
            removeKernelEditLine();
            addKernelEditLine();
            // chosen list last row is no longer on edit, stop listening to focus change events
            disconnect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(onFocusChange(QWidget*, QWidget*)));
            // reset flag
            m_LastChosenRowOnEdit = false;
            m_pChosenList->blockSignals(false);
        }
    }
}

// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onFocusChange(QWidget* pOldItem, QWidget* pNewItem)
{
    if (pOldItem != pNewItem)
    {
        verifyChosenListLastRow();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::removeKernelEditLine
// Description: Remove the "Type.." line from the right list
// Return Val:  void
// Author:      Yoni Rabin
// Date:        21/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::removeKernelEditLine()
{
    GT_IF_WITH_ASSERT(m_pChosenList && m_pLastChosenRow)
    {
        m_pChosenList->blockSignals(true);
        int row = m_pLastChosenRow->row();
        GT_IF_WITH_ASSERT(row < m_pChosenList->rowCount())
        {
            m_pChosenList->removeRow(m_pLastChosenRow->row());
            m_pLastChosenRow = NULL;
        }
        m_pChosenList->blockSignals(false);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::resetListColor
// Description: helper function to color an acListCtrl
// Arguments:   acListCtrl * pList
//              QColor col - default is Qt::black
// Return Val:  void
// Author:      Yoni Rabin
// Date:        19/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::resetListColor(acListCtrl* pList, QColor col)
{
    GT_IF_WITH_ASSERT(pList != NULL)
    {
        int listSize = pList->rowCount();

        for (int i = 0; i < listSize; i++)
        {
            QTableWidgetItem* pItem = m_pAPIList->item(i, 0);
            GT_IF_WITH_ASSERT(pItem != NULL)
            {
                pItem->setTextColor(col);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::getBreakpointData
// Description: get the inner value from the input QTableWidgetItem
// Arguments:   QTableWidgetItem* pItem
// Return Val:  gdBreakpointsItemData*
// Author:      Yoni Rabin
// Date:        19/6/2012
// ---------------------------------------------------------------------------
gdBreakpointsItemData* gdBreakpointsDialog::getBreakpointData(QTableWidgetItem* pItem)
{
    gdBreakpointsItemData* pRetVal = NULL;
    GT_IF_WITH_ASSERT(pItem != NULL)
    {
        // Get the item data:
        QVariant itemData = pItem->data(Qt::UserRole);
        pRetVal = (gdBreakpointsItemData*)itemData.value<void*>();
    }
    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onBeforeRemoveRow
// Description: Called before a row is removed
// Author:      Uri Shomroni
// Date:        15/9/2011
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onBeforeRemoveRow(int row)
{
    // If this is the last row:
    if (row == (m_pChosenList->rowCount() - 1))
    {
        // Add a new "enter expression" row to replace it:
        addKernelEditLine();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onAfterRemoveRow
// Description: Called after a row is removed
// Author:      Yoni Rabin
// Date:        1/7/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onAfterRemoveRow(int row)
{
    (void)(row); // unused
    setButtonStates();
}
// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onChosenItemChanged
// Description: used to catch check box change events
// Arguments:   QTableWidgetItem* pItem
// Return Val:  void
// Author:      Yoni Rabin
// Date:        20/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onChosenItemChanged(QTableWidgetItem* pItem)
{
    GT_IF_WITH_ASSERT(m_pChosenList != NULL && pItem != NULL)
    {
        // Ignore events for column 1:
        if (pItem->column() == 0)
        {
            m_pChosenList->blockSignals(true);

            if (m_LastChosenRowOnEdit && !pItem->text().isEmpty())
            {
                m_LastChosenRowOnEdit = false;
            }

            if (pItem != m_pLastChosenRow)
            {
                updateSelectAllCheckBoxStatus(true);
            }
            else
            {
                onKernelEndEditLabel();
            }

            m_pChosenList->blockSignals(false);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::getActiveList
// Description: gets the active left list from the current tab
// Return Val:  acListCtrl*
// Author:      Yoni Rabin
// Date:        20/6/2012
// ---------------------------------------------------------------------------
acListCtrl* gdBreakpointsDialog::getActiveList()
{
    acListCtrl* pActiveList = NULL;
    GT_IF_WITH_ASSERT(m_pTabs != NULL && m_pAPIList != NULL && m_pKernelList != NULL && m_pGenericBreakpointsList != NULL)
    {
        QWidget* currentTab = m_pTabs->currentWidget();

        if (currentTab == m_pAPITab)
        {
            pActiveList = m_pAPIList;
        }
        else if (currentTab == m_pKernelTab)
        {
            pActiveList = m_pKernelList;
        }
        else if (currentTab == m_pBreakpointsTab)
        {
            pActiveList = m_pGenericBreakpointsList;
        }
        else
        {
            GT_ASSERT(false);
        }
    }
    return pActiveList;
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onLeftListSelectionChanged
// Description: Called when the selection changes in one of the left side lists
// Author:      Yoni Rabin
// Date:        27/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onLeftListSelectionChanged()
{
    acListCtrl* pActiveList = getActiveList();
    onListSelectionChanged(pActiveList);
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onRightListSelectionChanged
// Description: Called when the right side list selection change
// Author:      Yoni Rabin
// Date:        27/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onRightListSelectionChanged()
{
    onListSelectionChanged(m_pChosenList);
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onListSelectionChanged
// Description: slot to update button status
// Author:      Yoni Rabin
// Date:        17/5/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onListSelectionChanged(acListCtrl* pCallingList)
{
    GT_IF_WITH_ASSERT(m_pChosenList != NULL)
    {
        acListCtrl* pActiveList = getActiveList();
        GT_IF_WITH_ASSERT(pActiveList != NULL)
        {
            pActiveList->blockSignals(true);
            m_pChosenList->blockSignals(true);

            bool isLeftSelected = (pActiveList->amountOfSelectedRows() > 0);
            bool isLeftFocused = pActiveList == pCallingList;
            bool isRightSelected = (m_pChosenList->amountOfSelectedRows() > 0);
            bool isRightFocused = m_pChosenList == pCallingList;

            if ((!isLeftFocused) && isLeftSelected)
            {
                pActiveList->clearSelection();
            }
            else if ((!isRightFocused) && isRightSelected)
            {
                m_pChosenList->clearSelection();

            }

            pActiveList->blockSignals(false);
            m_pChosenList->blockSignals(false);
            setButtonStates();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::setButtonStates
// Description: enables/disables buttons
// Return Val:  void
// Author:      Yoni Rabin
// Date:        24/5/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::setButtonStates()
{
    GT_IF_WITH_ASSERT(m_pAddButton != NULL && m_pRemoveButton != NULL && m_pRemoveAllButton != NULL)
    {
        acListCtrl* pActiveList = getActiveList();
        GT_IF_WITH_ASSERT(pActiveList != NULL)
        {
            bool isLeftSelected = (pActiveList->amountOfSelectedRows() > 0);

            if (m_pAddButton->isEnabled() != isLeftSelected)
            {
                m_pAddButton->setEnabled(isLeftSelected);
            }

            int numRows = m_pChosenList->rowCount();
            int numSelected = m_pChosenList->amountOfSelectedRows();
            bool isLastRowSelected = false;

            if (numSelected == 1)
            {
                foreach (QTableWidgetItem* pItem, m_pChosenList->selectedItems())
                {
                    if (pItem == m_pLastChosenRow)
                    {
                        isLastRowSelected = true;
                        break;
                    }
                }
            }

            bool isRightSelected = (numSelected > 1) || (numSelected == 1 && !isLastRowSelected);

            if (m_pRemoveButton->isEnabled() != isRightSelected)
            {
                m_pRemoveButton->setEnabled(isRightSelected);
            }

            // We have rows to remove if there is a valid breakpoint in the chosen list
            // Either we have more than 1, or we have 1 and it is not the 'type..' line:
            bool anyRowsToRemove = (numRows > 1) || (numRows == 1 && m_pLastChosenRow == NULL);

            if (m_pRemoveAllButton->isEnabled() != anyRowsToRemove)
            {
                m_pRemoveAllButton->setEnabled(anyRowsToRemove);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::onKernelEndEditLabel
// Description: Called when we finish editing the right list label
// Return Val:  void
// Author:      Yoni Rabin
// Date:        24/6/2012
// ---------------------------------------------------------------------------
void gdBreakpointsDialog::onKernelEndEditLabel()
{
    bool isFunctionNameValid = true;
    bool popupMessageForInvalidName = true;
    bool duplicateName = false;
    // Get the edited kernel function name:
    QString functionName = m_pLastChosenRow->text();

    // Valid function name should contain only alpha numeric characters:
    for (int i = 0; i < functionName.size(); i++)
    {
        const QChar currentChar = functionName.at(i);

        if (!currentChar.isLetterOrNumber())
        {
            if ((currentChar != '_') &&
                ((currentChar != '&') || (0 != i)))
            {
                isFunctionNameValid = false;
                break;
            }
        }
    }

    // Special cases for invalid name:
    if (functionName.isEmpty() || (functionName == GD_STR_BreakpointsKernelAddMessage))
    {
        isFunctionNameValid = false;
        popupMessageForInvalidName = false;
    }

    // Check that the function name does not already appear in the list:
    if (isFunctionNameValid)
    {
        int numberItems = m_pChosenList->rowCount() - 1;
        QString currText;

        for (int nItem = 0; nItem < numberItems; nItem++)
        {
            bool rc = m_pChosenList->getItemText(nItem, 0, currText);
            GT_IF_WITH_ASSERT(rc)
            {
                if (functionName == currText)
                {
                    isFunctionNameValid = false;
                    duplicateName = true;
                    break;
                }
            }
        }
    }

    // Check if it is in the kernel list to paint it blue:
    if (isFunctionNameValid)
    {
        int numberItems = m_pKernelList->rowCount();
        QString currText ;

        for (int nItem = 0; nItem < numberItems; nItem++)
        {
            bool rc = m_pKernelList->getItemText(nItem, 0, currText);
            GT_IF_WITH_ASSERT(rc)
            {
                if (functionName == currText)
                {
                    m_pKernelList->setItemTextColor(nItem, 0, Qt::blue);
                    break;
                }
            }
        }
    }

    // If valid name add a new "Type.." in the end, if not do not except the change:
    if (isFunctionNameValid)
    {
        // Set the item check status & color:
        m_pLastChosenRow->setCheckState(Qt::Checked);
        m_pLastChosenRow->setTextColor(Qt::black);

        // Allocate a new item data:
        gdBreakpointsItemData* pNewBreakpointItemData = new gdBreakpointsItemData;

        // Set the details for the item data:
        pNewBreakpointItemData->_kernelFunctionName = acQStringToGTString(functionName);
        pNewBreakpointItemData->_breakpointType = OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT;

        // Add this item data for deletion:
        m_rightListBreakpointsDataVector.push_back(pNewBreakpointItemData);

        // Remove the temp row:
        removeKernelEditLine();

        // Add the breakpoint row:
        // Add the item type and information:
        QString bpType;
        getBreakpointTypeString(pNewBreakpointItemData, bpType);
        addChosenListItem(functionName, bpType, pNewBreakpointItemData, true);

        // Add the kernel edit line:
        addKernelEditLine();
    }
    else
    {
        // Popup message only if needed:
        if (popupMessageForInvalidName)
        {
            QString message = duplicateName ? GD_STR_BreakPointsDuplicateKernelName : GD_STR_BreakPointsInvalidKernelName;
            acMessageBox::instance().warning(AF_STR_WarningA, message, QMessageBox::Ok);
        }

        // Remove the kernel edit line (the empty line):
        removeKernelEditLine();

        // Add the kernel edit line again:
        addKernelEditLine();
    }

    updateSelectAllCheckBoxStatus(true);
}
