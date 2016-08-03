//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionOverviewWindow.cpp
/// \brief  The implementation of the SessionOverviewWindow
///
//==================================================================================
// $Id: //devtools/branch/CPUProfileGUI/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/SessionOverviewWindow.cpp#1 $
// Last checkin:   $DateTime: 2013/02/04 06:21:40 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 463503 $
//=============================================================

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTBaseTools/Include/gtHashSet.h>
#include <AMDTApplicationComponents/Include/acQHTMLWindow.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acToolBar.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afHTMLContent.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>

// AMDTSharedProfiling:
#include <SessionTreeNodeData.h>
#include <ProfileApplicationTreeHandler.h>
#include <StringConstants.h>

// Local:
#include <inc/AmdtCpuProfiling.h>
#include <inc/CpuProjectHandler.h>
#include <inc/CpuProfileTreeHandler.h>
#include <inc/CPUProfileDataTable.h>
#include <inc/ModulesDataTable.h>
#include <inc/ProcessesDataTable.h>
#include <inc/FunctionsDataTable.h>
#include <inc/SessionFunctionView.h>
#include <inc/SessionModulesView.h>
#include <inc/SessionOverviewWindow.h>
#include <inc/SessionViewCreator.h>
#include <inc/SessionWindow.h>
#include <inc/StringConstants.h>
#include <inc/CPUProfileUtils.h>
#include <inc/Auxil.h>

#include <tuple>


SessionOverviewWindow::SessionOverviewWindow(QWidget* pParent, CpuSessionWindow* pSessionWindow)
    : DataTab(pParent, pSessionWindow)
{
    m_pList = nullptr;
    setMouseTracking(true);

    // Only display system dll's in filter:
    setEnableOnlySystemDllInfilterDlg(true);

    // Set the display filter for the display filter dialog:
    //m_pDisplaySettings = &m_functionsTablesFilter;

}

SessionOverviewWindow::~SessionOverviewWindow()
{
}

bool SessionOverviewWindow::display(afApplicationTreeItemData* pItemData)
{
    bool retVal = true;

    // Set the displayed item data:
    m_pDisplayedSessionItemData = pItemData;

    // Build the layout:
    setSessionWindowLayout();

    // Display the session using the input item data object:
    retVal = displaySession();

    return retVal;
}

bool SessionOverviewWindow::displaySession()
{
    bool retVal = false;

    // Display the hot spot indicator options:
    retVal = fillHotspotIndicatorCombo();

    // Initialize the display filters:
    initDisplayFilters();

    // Display the session data table:
    retVal = displaySessionDataTables() && retVal;

    // Display the session HTML properties:
    retVal = displaySessionProperties() && retVal;

    return retVal;
}


void SessionOverviewWindow::setSessionWindowLayout()
{
    m_pSplitterCentralWidget = new QSplitter(Qt::Vertical);

    m_pSplitterCentralWidget->setMouseTracking(true);

    // Create bottom and top widgets (top for the tables, bottom for the properties and hint window):
    QWidget* pTopWidget = new QWidget;

    pTopWidget->setMouseTracking(true);

    QWidget* pBottomWidget = new QWidget;

    pBottomWidget->setContentsMargins(0, 0, 0, 0);

    setFocusProxy(m_pSplitterCentralWidget);
    setCentralWidget(m_pSplitterCentralWidget);

    bool isCSSEnabled = false;
    CPUSessionTreeItemData* pSessionData = nullptr;
    GT_IF_WITH_ASSERT(m_pDisplayedSessionItemData != nullptr)
    {
        pSessionData = qobject_cast<CPUSessionTreeItemData*>(m_pDisplayedSessionItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pSessionData != nullptr)
        {
            isCSSEnabled = pSessionData->ShouldCollectCSS(true);
        }
    }

    gtVector<CPUProfileDataTable::TableContextMenuActionType> actions;
    actions.push_back(CPUProfileDataTable::DISPLAY_PROCESS_IN_MODULE_VIEW);
    actions.push_back(CPUProfileDataTable::DISPLAY_PROCESS_IN_FUNCTIONS_VIEW);
    m_pProcessesTable = new ProcessesDataTable(nullptr, actions, pSessionData);

    m_pProcessesTable->setMouseTracking(true);
    bool rc = connect(m_pProcessesTable, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(onTableItemActivated(QTableWidgetItem*)));
    GT_ASSERT(rc);

    rc = connect(m_pProcessesTable,
                 SIGNAL(contextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType, QTableWidgetItem*)),
                 this,
                 SLOT(onTableContextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType, QTableWidgetItem*)));
    GT_ASSERT(rc);

    actions.clear();
    actions.push_back(CPUProfileDataTable::DISPLAY_MODULE_IN_FUNCTIONS_VIEW);
    actions.push_back(CPUProfileDataTable::DISPLAY_MODULE_IN_MODULES_VIEW);
    m_pModulesTable = new ModulesDataTable(nullptr, actions, pSessionData, m_pParentSessionWindow);

    m_pModulesTable->setMouseTracking(true);
    rc = connect(m_pModulesTable, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(onTableItemActivated(QTableWidgetItem*)));
    GT_ASSERT(rc);

    rc = connect(m_pModulesTable,
                 SIGNAL(contextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType, QTableWidgetItem*)),
                 this,
                 SLOT(onTableContextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType, QTableWidgetItem*)));
    GT_ASSERT(rc);

    actions.clear();
    actions.push_back(CPUProfileDataTable::DISPLAY_FUNCTION_IN_SOURCE_CODE_VIEW);

    if (isCSSEnabled)
    {
        actions.push_back(CPUProfileDataTable::DISPLAY_FUNCTION_IN_CALLGRAPH_VIEW);
    }

    actions.push_back(CPUProfileDataTable::DISPLAY_FUNCTION_IN_FUNCTIONS_VIEW);
    m_pFunctionsTable = new FunctionsDataTable(nullptr, actions, pSessionData, m_pParentSessionWindow);

    m_pFunctionsTable->setMouseTracking(true);

    // If a file cannot be found, popup the user:
    m_pFunctionsTable->setPopupToBrowseMissingFiles(false);

    rc = connect(m_pFunctionsTable, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(onTableItemActivated(QTableWidgetItem*)));
    GT_ASSERT(rc);

    rc = connect(m_pFunctionsTable, SIGNAL(contextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType, QTableWidgetItem*)),
                 this,
                 SLOT(onTableContextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType,
                                                        QTableWidgetItem*)));
    GT_ASSERT(rc);

    m_pPropertiewView = new acQHTMLWindow(nullptr);


    // Create an hint label:
    QFrame* pFrame = createHintLabelFrame();

    m_pTopToolbar = new acToolBar(nullptr);


    // Do not allow the toolbar to float:
    m_pTopToolbar->setFloatable(false);
    m_pTopToolbar->setMovable(false);
    m_pTopToolbar->setStyleSheet("QToolBar { border-style: none; margin:5;}");

    m_pProcessesHeader = new QLabel(CP_overviewPageHottestProcessesHeader);

    QFont boldFont = m_pProcessesHeader->font();
    boldFont.setBold(true);
    m_pProcessesHeader->setFont(boldFont);

    QLabel* pModulesHeader = new QLabel(CP_overviewPageHottestModulesHeader);

    pModulesHeader->setFont(boldFont);

    int colSpan = m_isMultiProcesses ? 2 : 4;
    int modulesCol = m_isMultiProcesses ? 2 : 0;

    QWidget* emptySpacer = new QWidget;
    emptySpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QString caption = CP_strCPUProfileToolbarBase;
    caption.append(":");

    m_pTopToolbar->AddLabel(CP_overviewPageHottestFunctionsHeader, true, false, 0);
    m_pTopToolbar->addWidget(emptySpacer);
    acWidgetAction* pTestAction = m_pTopToolbar->AddLabel(caption);
    const QLabel* pTestLabel = TopToolbarLabel(pTestAction);
    int w = 25;

    if (pTestLabel != nullptr)
    {
        static QString longestDisplayFilter = "Hide System Modules";
        w = pTestLabel->fontMetrics().boundingRect(longestDisplayFilter).width();
    }

    // Create the display settings link:
    acToolbarActionData actionData(SIGNAL(linkActivated(const QString&)), this, SLOT(OnDisplaySettingsClicked()));
    actionData.m_actionType = acToolbarActionData::AC_LABEL_ACTION;
    actionData.m_margin = 5;
    actionData.m_minWidth = w - 50;
    actionData.m_objectName = "OverviewDisplayFilterLink";

    m_pDisplaySettingsAction = m_pTopToolbar->AddWidget(actionData);

    acWidgetAction* pHeaderLabelAction = m_pTopToolbar->AddLabel(CP_overviewPageHotspotIndicatorHeader, true);
    GT_ASSERT(pHeaderLabelAction != nullptr);

    acToolbarActionData comboData(SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onHotSpotComboChanged(const QString&)));
    comboData.m_actionType = acToolbarActionData::AC_COMBOBOX_ACTION;

    m_pHotSpotIndicatorComboBoxAction = m_pTopToolbar->AddWidget(comboData);
    GT_ASSERT(m_pHotSpotIndicatorComboBoxAction != nullptr);

    QGridLayout* pGridLayout = new QGridLayout;


    // First row - top toolbar:
    pGridLayout->addWidget(m_pTopToolbar, 0, 0, 1, 4);

    // Second row - functions table:
    pGridLayout->addWidget(m_pFunctionsTable , 1, 0, 1, 4);

    // Third and forth rows - process, and modules tables  and captions:
    if (m_isMultiProcesses)
    {
        pGridLayout->addWidget(m_pProcessesHeader , 2, 0, 1, 2);
        pGridLayout->addWidget(m_pProcessesTable, 3, 0, 1, 2);
    }

    pGridLayout->addWidget(pModulesHeader , 2, modulesCol, 1, colSpan);
    pGridLayout->addWidget(m_pModulesTable, 3, modulesCol, 1, colSpan);


    pGridLayout->setColumnStretch(0, 1);
    pGridLayout->setColumnStretch(1, 1);
    pGridLayout->setColumnStretch(2, 1);
    pGridLayout->setColumnStretch(3, 1);

    pTopWidget->setLayout(pGridLayout);

    m_pSplitterCentralWidget->addWidget(pTopWidget);
    m_pSplitterCentralWidget->addWidget(pBottomWidget);
    pBottomWidget->setMouseTracking(true);

    QVBoxLayout* pBottomLayout = new QVBoxLayout;

    pBottomLayout->addWidget(m_pPropertiewView);
    pBottomLayout->addWidget(pFrame);
    pBottomWidget->setLayout(pBottomLayout);
    m_pPropertiewView->setMouseTracking(true);

    // Set the split proportions:
    m_pSplitterCentralWidget->setStretchFactor(0, 1);
    m_pSplitterCentralWidget->setStretchFactor(1, 2);

    // Make sure that none of the children is hidden:
    m_pSplitterCentralWidget->setChildrenCollapsible(false);

    // Set the width of the split handle to minimum:
    m_pSplitterCentralWidget->setHandleWidth(5);
    m_pSplitterCentralWidget->setContentsMargins(0, 0, 0, 0);
    pTopWidget->setContentsMargins(0, 0, 0, 0);

    // Initialize the window hint:
    updateHint(CP_overviewInformationHint);

    // Update display filter string:
    updateDisplaySettingsString();

    m_pFunctionsTable->setTableDisplaySettings(&m_functionsTablesFilter);
    m_pModulesTable->setTableDisplaySettings(&m_modulesTablesFilter);
    m_pProcessesTable->setTableDisplaySettings(&m_processesTablesFilter);

    m_editActionsWidgetsList << m_pFunctionsTable;
    m_editActionsWidgetsList << m_pModulesTable;
    m_editActionsWidgetsList << m_pProcessesTable;
}

bool SessionOverviewWindow::displaySessionExecutionDetails(afHTMLContent& content)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT((m_pDisplayedSessionItemData != nullptr) && (m_pFunctionsTable != nullptr))
    {
        CPUSessionTreeItemData* pSessionData = qobject_cast<CPUSessionTreeItemData*>(m_pDisplayedSessionItemData->extendedItemData());
        GT_IF_WITH_ASSERT((pSessionData != nullptr) &&
                          (m_pPropertiewView != nullptr) &&
                          (m_pProfDataRdr.get() != nullptr))
        {
            AMDTProfileSessionInfo sessionInfo;
            m_pProfDataRdr->GetProfileSessionInfo(sessionInfo);

            content.addHTMLItem(afHTMLContent::AP_HTML_TITLE, CP_overviewPageExecutionHeader);

            gtString firstColStr, secondColStr;
            firstColStr.appendFormattedString(L"<b>%ls:</b> %ls", CP_overviewPageTargetPath, sessionInfo.m_targetAppPath.asCharArray());
            content.addHTMLItem(afHTMLContent::AP_HTML_LINE, firstColStr);

            firstColStr.makeEmpty();
            firstColStr.appendFormattedString(L"<b>%ls:</b> %ls", CP_overviewPageWorkingDirectory, sessionInfo.m_targetAppWorkingDir.asCharArray());
            content.addHTMLItem(afHTMLContent::AP_HTML_LINE, firstColStr);

            firstColStr.makeEmpty();
            firstColStr.appendFormattedString(L"<b>%ls:</b> %ls", CP_overviewPageDataFolder, sessionInfo.m_sessionDir.asCharArray());
            content.addHTMLItem(afHTMLContent::AP_HTML_LINE, firstColStr);

            firstColStr.makeEmpty();
            firstColStr.appendFormattedString(L"<b>%ls:</b> %ls", CP_overviewPageCommandLineArgs, sessionInfo.m_targetAppCmdLineArgs.asCharArray());
            content.addHTMLItem(afHTMLContent::AP_HTML_LINE, firstColStr);

            firstColStr.makeEmpty();
            firstColStr.appendFormattedString(L"<b>%ls:</b> %ls", CP_overviewPageEnvVars, sessionInfo.m_targetAppEnvVars.asCharArray());
            content.addHTMLItem(afHTMLContent::AP_HTML_LINE, firstColStr);

            secondColStr.makeEmpty();
            firstColStr.makeEmpty();
            firstColStr.appendFormattedString(L"<b>%ls:</b> 0x%llx", CP_overviewPageProfileCPUAffinity, sessionInfo.m_coreAffinity);

            secondColStr.appendFormattedString(L"<b>%ls:</b> %ls", CP_overviewPageProfileScope, sessionInfo.m_sessionScope.asCharArray());
            content.addHTMLItem(afHTMLContent::AP_HTML_LINE, firstColStr, secondColStr);


            firstColStr.makeEmpty();
            secondColStr.makeEmpty();

            if (sessionInfo.m_cssEnabled)
            {
                gtString cssSupportFpoFormattedStr;
                const wchar_t* pCssScopeStr = nullptr;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

                switch (sessionInfo.m_unwindScope)
                {
                    case CP_CSS_SCOPE_USER:
                        pCssScopeStr = _T(CP_STR_cpuProfileProjectSettingsCallStackUserSpace);
                        break;

                    case CP_CSS_SCOPE_KERNEL:
                        pCssScopeStr = _T(CP_STR_cpuProfileProjectSettingsCallStackKernelSpace);
                        break;

                    case CP_CSS_SCOPE_ALL:
                        pCssScopeStr = _T(CP_STR_cpuProfileProjectSettingsCallStackUserKernelSpaces);
                        break;

                    case CP_CSS_SCOPE_UNKNOWN:
                        break;
                }

                const wchar_t* pCssSupportFpoStr = sessionInfo.m_cssFPOEnabled ? L"On" : L"Off";
                cssSupportFpoFormattedStr.appendFormattedString(CP_overviewCallStackInformationFpoSubstr, pCssSupportFpoStr);

#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

                gtString cssScopeFormattedStr;

                if (nullptr != pCssScopeStr)
                {
                    cssScopeFormattedStr.appendFormattedString(CP_overviewCallStackInformationScopeSubstr, pCssScopeStr);
                }

                secondColStr.appendFormattedString(CP_overviewCallStackInformationBold, cssScopeFormattedStr.asCharArray(),
                                                   sessionInfo.m_unwindDepth,
                                                   pSessionData->m_cssInterval,
                                                   cssSupportFpoFormattedStr.asCharArray());
            }

            const wchar_t* pBoolStr = sessionInfo.m_cssEnabled ? L"True" : L"False";
            firstColStr.appendFormattedString(L"<b>%ls:</b> %ls", CP_overviewCallStackSampling, pBoolStr);
            content.addHTMLItem(afHTMLContent::AP_HTML_LINE, firstColStr, secondColStr);
            content.addHTMLItem(afHTMLContent::AP_HTML_LINE, L"<br>");

            retVal = true;
        }
    }

    return retVal;
}


bool SessionOverviewWindow::displaySessionProfileDetails(afHTMLContent& content)
{
    bool retVal = true;
    GT_IF_WITH_ASSERT((m_pDisplayedSessionItemData != nullptr) && (m_pFunctionsTable != nullptr))
    {
        CPUSessionTreeItemData* pSessionData = qobject_cast<CPUSessionTreeItemData*>(m_pDisplayedSessionItemData->extendedItemData());
        GT_IF_WITH_ASSERT((pSessionData != nullptr) && (m_pPropertiewView != nullptr) && (m_pProfDataRdr != nullptr))
        {
            AMDTProfileSessionInfo sessionInfo;
            m_pProfDataRdr->GetProfileSessionInfo(sessionInfo);

            content.addHTMLItem(afHTMLContent::AP_HTML_TITLE, CP_overviewPageProfileDetailsHeader);

            gtString eventsCaption = CP_overviewPageMonitoredEventsHeader;
            eventsCaption.prepend(L"<b>");
            eventsCaption.append(L"</b>");

            gtString firstColStr, secondColStr;
            firstColStr.makeEmpty();
            firstColStr.appendFormattedString(L"<b>%ls:</b> %ls", CP_overviewPageSessionType, (sessionInfo.m_sessionType.asCharArray()));
            content.addHTMLItem(afHTMLContent::AP_HTML_LINE, firstColStr, eventsCaption);

            gtVector<gtString> eventsStrVector;
            secondColStr.makeEmpty();
            bool rc = buildEventsStringsVector(eventsStrVector, secondColStr);
            GT_ASSERT(rc);

            firstColStr.makeEmpty();
            firstColStr.appendFormattedString(L"<b>%ls:</b> %ls", CP_overviewPageProfileStartTime, (sessionInfo.m_sessionStartTime.asCharArray()));
            content.addHTMLItem(afHTMLContent::AP_HTML_LINE, firstColStr, secondColStr, 10);

            firstColStr.makeEmpty();
            secondColStr.makeEmpty();
            firstColStr.appendFormattedString(L"<b>%ls:</b> %ls", CP_overviewPageProfileEndTime, (sessionInfo.m_sessionEndTime.asCharArray()));
            content.addHTMLItem(afHTMLContent::AP_HTML_LINE, firstColStr, secondColStr, 10);

            firstColStr.makeEmpty();
            gtString durationStr = acDurationAsString(pSessionData->m_profileDuration);
            firstColStr.appendFormattedString(L"<b>%ls:</b> %ls", CP_overviewPageProfileDuration, (durationStr.asCharArray()));
            content.addHTMLItem(afHTMLContent::AP_HTML_LINE, firstColStr);

            firstColStr.makeEmpty();
            firstColStr.appendFormattedString(L"<b>%ls: </b>", CP_overviewPageProfileCPUDetails);
            unsigned int cpuFamily = sessionInfo.m_cpuFamily;
            unsigned int cpuModel = sessionInfo.m_cpuModel;

            AMDTCpuTopologyVec cpuToplogy;
            retVal = m_pProfDataRdr->GetCpuTopology(cpuToplogy);

            if (true == retVal)
            {
                unsigned int cpuCount = cpuToplogy.size();
                firstColStr.appendFormattedString(CP_overviewPageProfileCPUDetailsStr, cpuFamily, cpuModel, cpuCount);
                content.addHTMLItem(afHTMLContent::AP_HTML_LINE, firstColStr);
            }

            // Disable display of number of processes till new API to fetch number processes added to Reporter.
            AMDTProfileProcessInfoVec processes;
            m_pProfDataRdr->GetProcessInfo(AMDT_PROFILE_ALL_PROCESSES, processes);
            firstColStr.makeEmpty();
            firstColStr.appendFormattedString(L"<b>%ls:</b> %d", CP_overviewPageTotalProcesses, processes.size());
            content.addHTMLItem(afHTMLContent::AP_HTML_LINE, firstColStr);

            // GetThreadSummary() is a performance heavy API, delaying overview tab generation.
            // Disable display of number of threads till new API to fetch number threads added to Reporter.
#if 0
            AMDTProfileDataOptions reportOptions;
            reportOptions.m_doSort = false;
            reportOptions.m_summaryCount = 0;
            reportOptions.m_othersEntryInSummary = false;
            reportOptions.m_coreMask = AMDT_PROFILE_ALL_CORES;

            m_pProfDataRdr->SetReportOption(reportOptions);

            gtHashSet<gtUInt64> uniqueThreads;
            AMDTProfileReportConfigVec reportConfigs;

            retVal = m_pProfDataRdr->GetReportConfigurations(reportConfigs);

            if (!reportConfigs.empty())
            {
                for (auto const& counter : reportConfigs[0].m_counterDescs)
                {
                    AMDTProfileDataVec threadsList;
                    m_pProfDataRdr->GetThreadSummary(counter.m_id, threadsList);

                    for (const auto threadInfo : threadsList)
                    {
                        uniqueThreads.insert(threadInfo.m_id);
                    }
                }
            }

            // Reset reporting options
            reportOptions = m_pDisplayFilter->GetProfileDataOptions();
            m_pProfDataRdr->SetReportOption(reportOptions);

            unsigned threadsCount = uniqueThreads.size();

            firstColStr.makeEmpty();
            firstColStr.appendFormattedString(L"<b>%ls:</b> %u", CP_overviewPageTotalThreads, threadsCount);
            content.addHTMLItem(afHTMLContent::AP_HTML_LINE, firstColStr);
#endif

            retVal = true;
        }
    }
    return retVal;
}


bool SessionOverviewWindow::displaySessionProperties()
{
    afHTMLContent content;

    bool bRetVal = displaySessionExecutionDetails(content);

    if (true == bRetVal)
    {
        bRetVal = displaySessionProfileDetails(content);

        if (true == bRetVal)
        {
            gtString htmlStr;
            content.toString(htmlStr);
            m_pPropertiewView->setHtml(acGTStringToQString(htmlStr));
        }
    }

    return bRetVal;
}

bool SessionOverviewWindow::displaySessionDataTables()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pModulesTable != nullptr) &&
                      (m_pProcessesTable != nullptr) &&
                      (m_pFunctionsTable != nullptr) &&
                      (m_pProcessesHeader != nullptr))
    {
        retVal = m_pProcessesTable->displayTableSummaryData(m_pProfDataRdr, m_pDisplayFilter, m_counterIdx, m_isCLU);
        retVal = m_pFunctionsTable->displayTableSummaryData(m_pProfDataRdr, m_pDisplayFilter, m_counterIdx, m_isCLU) && retVal;
        retVal = m_pModulesTable->displayTableSummaryData(m_pProfDataRdr, m_pDisplayFilter, m_counterIdx, m_isCLU) && retVal;
    }

    return retVal;
}

bool SessionOverviewWindow::updateTablesHotspotIndicator()
{
    bool retVal = false;

    // Sanity check
    GT_IF_WITH_ASSERT((m_pModulesTable != nullptr) &&
                      (m_pProcessesTable != nullptr) &&
                      (m_pFunctionsTable != nullptr) &&
                      (m_pProcessesHeader != nullptr))
    {
        retVal = true;

        // If there are multiple processes:
        if (m_isMultiProcesses)
        {
            // Display the data according to the new hot spot indicator:
            bool rc = m_pProcessesTable->organizeTableByHotSpotIndicator() && retVal;
            retVal = retVal && rc;
        }

        QTableWidgetItem* newItem = new QTableWidgetItem(acGTStringToQString(L"testing"));

        m_pProcessesTable->setHorizontalHeaderItem(2, newItem);

        // Show / hide the processes section:
        m_pProcessesTable->setVisible(m_isMultiProcesses);
        m_pProcessesHeader->setVisible(m_isMultiProcesses);

        // Display the data according to the new hot spot indicator:
        bool rc = m_pModulesTable->organizeTableByHotSpotIndicator() && retVal;
        retVal = retVal && rc;

        // Display the data according to the requested filter:
        rc = m_pFunctionsTable->organizeTableByHotSpotIndicator() && retVal;
        retVal = retVal && rc;
    }

    return retVal;
}

void SessionOverviewWindow::openProcessesView()
{
    openModulesView();

    // Get the tree instance:
    afApplicationCommands* pCommands = afApplicationCommands::instance();
    SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();

    GT_IF_WITH_ASSERT((pCommands != nullptr) &&
                      (pSessionViewCreator != nullptr) &&
                      (m_pFunctionsTable != nullptr) &&
                      (m_pDisplayedSessionItemData != nullptr))
    {
        // Find the functions item data for this session:
        afApplicationTreeItemData* pActivatedItemMacthingItemData = ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(m_pDisplayedSessionItemData, AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS);
        GT_IF_WITH_ASSERT(pActivatedItemMacthingItemData != nullptr)
        {
            // Find the matching session window:
            CpuSessionWindow* pSessionWindow = pSessionViewCreator->findSessionWindow(pActivatedItemMacthingItemData);

            GT_IF_WITH_ASSERT(pSessionWindow != nullptr)
            {
                SessionModulesView* pSessionModulesView = pSessionWindow->sessionModulesView();
                GT_IF_WITH_ASSERT(pSessionModulesView != nullptr)
                {
                    pSessionModulesView->displayByProcesses();
                }
            }
        }
    }
}
void SessionOverviewWindow::openModulesView()
{
    GT_IF_WITH_ASSERT(m_pDisplayedSessionItemData != nullptr)
    {
        // Get the modules item data for the current session:
        afApplicationTreeItemData* pModulesItemData = CpuProfileTreeHandler::instance().findChildItemData(m_pDisplayedSessionItemData,
                                                      AF_TREE_ITEM_PROFILE_CPU_MODULES);
        GT_IF_WITH_ASSERT((pModulesItemData != nullptr) && (pModulesItemData->m_pTreeWidgetItem != nullptr))
        {
            afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
            {
                afApplicationTree* pApplicationTree = pApplicationCommands->applicationTree();
                GT_IF_WITH_ASSERT(pApplicationTree != nullptr)
                {
                    pApplicationTree->selectItem(pModulesItemData, true);
                    pApplicationTree->expandItem(pModulesItemData->m_pTreeWidgetItem);
                }
            }
        }
    }
}
void SessionOverviewWindow::openFunctionsView()
{
    GT_IF_WITH_ASSERT(m_pDisplayedSessionItemData != nullptr)
    {
        // Get the modules item data for the current session:
        afApplicationTreeItemData* pModulesItemData = CpuProfileTreeHandler::instance().findChildItemData(m_pDisplayedSessionItemData, AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS);
        GT_IF_WITH_ASSERT((pModulesItemData != nullptr) && (pModulesItemData->m_pTreeWidgetItem != nullptr))
        {
            afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
            {
                afApplicationTree* pApplicationTree = pApplicationCommands->applicationTree();
                GT_IF_WITH_ASSERT(pApplicationTree != nullptr)
                {
                    pApplicationTree->selectItem(pModulesItemData, true);
                    pApplicationTree->expandItem(pModulesItemData->m_pTreeWidgetItem);
                }
            }
        }
    }
}

bool SessionOverviewWindow::fillHotspotIndicatorCombo()
{
    bool retVal = false;

    const QComboBox* pHotSpotIndicatorComboBox = TopToolbarComboBox(m_pHotSpotIndicatorComboBoxAction);
    GT_IF_WITH_ASSERT((m_pHotSpotIndicatorComboBoxAction != nullptr) &&
                      (pHotSpotIndicatorComboBox != nullptr) &&
                      (m_pDisplayedSessionItemData != nullptr) &&
                      (m_pDisplayFilter != nullptr))
    {
        if ((false == m_pDisplayFilter->IsSeperatedByCoreEnabled()) &&
            (false == m_pDisplayFilter->IsSeperatedByNumaEnabled()))
        {
            // Do not refill the combo box again if it already has items in it:
            if (pHotSpotIndicatorComboBox->count() == 0)
            {
                // Find the profile type:
                SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(m_pDisplayedSessionItemData->extendedItemData());
                GT_IF_WITH_ASSERT(pSessionData != nullptr)
                {
                    QStringList hotSpotColumns;

                    // counters for "All Data" config type selected
                    QString filterName = CP_strCPUProfileDisplayFilterAllData;
                    GT_IF_WITH_ASSERT(!filterName.isEmpty())
                    {
                        CounterNameIdVec countersName;
                        m_pDisplayFilter->GetConfigCounters(filterName, countersName);

                        for (const auto& name : countersName)
                        {
                            hotSpotColumns << acGTStringToQString(std::get<0>(name)); // counter name
                        }

                        retVal = true;
                    }

                    m_pHotSpotIndicatorComboBoxAction->UpdateStringList(hotSpotColumns);
                    m_pHotSpotIndicatorComboBoxAction->UpdateEnabled(hotSpotColumns.size() > 1);
                }
            }
            else
            {
                m_pHotSpotIndicatorComboBoxAction->UpdateEnabled(pHotSpotIndicatorComboBox->count() > 1);
                retVal = true;
            }
        }
        else
        {
            m_pHotSpotIndicatorComboBoxAction->UpdateTooltip(CP_overviewHotspotWarning);
            m_pHotSpotIndicatorComboBoxAction->UpdateEnabled(false);
            retVal = true;
        }
    }
    return retVal;
}

void SessionOverviewWindow::onHotSpotComboChanged(const QString& text)
{
    GT_IF_WITH_ASSERT((m_pFunctionsTable != nullptr) &&
                      (m_pModulesTable != nullptr) &&
                      (m_pProcessesTable != nullptr))
    {
        GT_IF_WITH_ASSERT((m_pFunctionsTable->tableDisplaySettings() != nullptr) &&
                          (m_pModulesTable->tableDisplaySettings() != nullptr) &&
                          (m_pProcessesTable->tableDisplaySettings() != nullptr))
        {
            Qt::SortOrder defaultSortOrder = Qt::DescendingOrder;

            // Set the default sort order:
            m_pFunctionsTable->tableDisplaySettings()->m_lastSortOrder = defaultSortOrder;
            m_pModulesTable->tableDisplaySettings()->m_lastSortOrder = defaultSortOrder;
            m_pProcessesTable->tableDisplaySettings()->m_lastSortOrder = defaultSortOrder;
            m_pDisplayFilter->SetCLUOVHdrName(text);

            auto itr = m_CounterIdxMap.find(text.toStdWString().c_str());

            if (m_CounterIdxMap.end() != itr)
            {
                bool rc = m_pProcessesTable->displayTableSummaryData(m_pProfDataRdr, m_pDisplayFilter, itr->second, m_isCLU);
                GT_ASSERT(rc);

                rc = m_pFunctionsTable->displayTableSummaryData(m_pProfDataRdr, m_pDisplayFilter, itr->second, m_isCLU);
                GT_ASSERT(rc);

                rc = m_pModulesTable->displayTableSummaryData(m_pProfDataRdr, m_pDisplayFilter, itr->second, m_isCLU);
                GT_ASSERT(rc);
            }
        }
    }
    //emit hotspotIndicatorChanged(text);
}

void SessionOverviewWindow::initDisplayFilters()
{
    const QComboBox* pHotSpotIndicatorComboBox = TopToolbarComboBox(m_pHotSpotIndicatorComboBoxAction);
    GT_IF_WITH_ASSERT((m_pProfDataRdr != nullptr) &&
                      (m_pDisplayFilter != nullptr) &&
                      (pHotSpotIndicatorComboBox != nullptr) &&
                      (m_pFunctionsTable != nullptr) &&
                      (m_pModulesTable != nullptr) &&
                      (m_pProcessesTable != nullptr))
    {
        // Get the view name from the combo:
        QString hotSpotIndicatorName = pHotSpotIndicatorComboBox->currentText();

        // Set the display filter for the processes table:
        m_processesTablesFilter.m_amountOfItemsInDisplay = 5;
        m_processesTablesFilter.m_displayedColumns.push_back(TableDisplaySettings::PROCESS_NAME_COL);
        m_processesTablesFilter.m_displayedColumns.push_back(TableDisplaySettings::PID_COL);
        m_processesTablesFilter.m_displayedColumns.push_back(TableDisplaySettings::SAMPLES_COUNT_COL);

        if (false == m_isCLU)
        {
            m_processesTablesFilter.m_displayedColumns.push_back(TableDisplaySettings::SAMPLES_PERCENT_COL);
        }

        // Set the display filter for the modules table:
        m_modulesTablesFilter.m_amountOfItemsInDisplay = 5;
        m_modulesTablesFilter.m_hotSpotIndicatorColumnCaption = "";
        m_modulesTablesFilter.m_displayedColumns.push_back(TableDisplaySettings::MODULE_NAME_COL);
        m_modulesTablesFilter.m_displayedColumns.push_back(TableDisplaySettings::SAMPLES_COUNT_COL);

        if (false == m_isCLU)
        {
            m_modulesTablesFilter.m_displayedColumns.push_back(TableDisplaySettings::SAMPLES_PERCENT_COL);
        }

        // Set the display filter for the functions table:
        m_functionsTablesFilter.m_amountOfItemsInDisplay = 5;
        m_functionsTablesFilter.m_hotSpotIndicatorColumnCaption = "";
        m_functionsTablesFilter.m_displayedColumns.push_back(TableDisplaySettings::FUNCTION_ID);
        m_functionsTablesFilter.m_displayedColumns.push_back(TableDisplaySettings::FUNCTION_NAME_COL);
        m_functionsTablesFilter.m_displayedColumns.push_back(TableDisplaySettings::SAMPLES_COUNT_COL);

        if (false == m_isCLU)
        {
            m_functionsTablesFilter.m_displayedColumns.push_back(TableDisplaySettings::SAMPLES_PERCENT_COL);
        }

        m_functionsTablesFilter.m_displayedColumns.push_back(TableDisplaySettings::MODULE_NAME_COL);


        if (true == m_isCLU)
        {
            updateHint(CP_overviewInformationHintForCLU);
        }
        else
        {
            updateHint(CP_overviewInformationHint);
        }
    }
}

bool SessionOverviewWindow::buildEventsStringsVector(gtVector<gtString>& eventsStrVector,
                                                     gtString& eventsAsHTMLTable)
{
    bool retVal = false;

    QString viewName;
    GT_IF_WITH_ASSERT(nullptr != m_pDisplayFilter)
    {
        bool isIBS = false;
        // Find the profile type:
        SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(m_pDisplayedSessionItemData->extendedItemData());

        GT_IF_WITH_ASSERT(pSessionData != nullptr)
        {
            isIBS = (pSessionData->m_profileTypeStr == PM_profileTypeInstructionBasedSampling);
        }

        viewName =  "All Data";

        if (isIBS)
        {
            // IBS, only add "IBS Fetch All" and "IBS Ops":
            eventsStrVector.push_back(L"IBS Fetch All and IBS Ops");
        }
        else
        {
            CounterNameIdVec eventsList;
            retVal = m_pDisplayFilter->GetConfigCounters(viewName, eventsList);

            if (true == retVal)
            {
                for (const auto& events : eventsList)
                {
                    eventsStrVector.push_back(std::get<0>(events)); // counter name
                }
            }
        }

        eventsAsHTMLTable.append(L"<table>");

        for (int i = 0; i < (int)eventsStrVector.size(); i++)
        {
            int column = i % 3;

            if (column == 0)
            {
                eventsAsHTMLTable.append(L"<tr>");
            }

            eventsAsHTMLTable.appendFormattedString(L"<td>%ls</td>", eventsStrVector[i].asCharArray());

            if (column == 2)
            {
                eventsAsHTMLTable.append(L"</tr>");
            }
        }

        if (eventsStrVector.size() % 3 != 0)
        {
            eventsAsHTMLTable.append(L"</tr>");
        }

        eventsAsHTMLTable.append(L"<table>");

        retVal = true;
    }

    return retVal;
}

void SessionOverviewWindow::activateTableItem(QTableWidgetItem* pActivateItem, CPUProfileDataTable* pDataTable)
{
    // Get the tree instance:
    afApplicationCommands* pCommands = afApplicationCommands::instance();
    SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();

    GT_IF_WITH_ASSERT((pCommands != nullptr) && (pSessionViewCreator != nullptr) && (pDataTable != nullptr))
    {
        afApplicationTree* pApplicationTree = pCommands->applicationTree();
        GT_IF_WITH_ASSERT((m_pDisplayedSessionItemData != nullptr) && (pApplicationTree != nullptr) && (m_pModulesTable != nullptr))
        {
            afApplicationTreeItemData* pActivatedItemMacthingItemData = nullptr;

            if ((pDataTable == m_pModulesTable) || (pDataTable == m_pProcessesTable))
            {
                pActivatedItemMacthingItemData = ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(m_pDisplayedSessionItemData, AF_TREE_ITEM_PROFILE_CPU_MODULES);
            }
            else
            {
                pActivatedItemMacthingItemData = ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(m_pDisplayedSessionItemData, AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS);
            }

            GT_IF_WITH_ASSERT(pActivatedItemMacthingItemData != nullptr)
            {
                // Get the tree instance:
                pApplicationTree->expandItem(pActivatedItemMacthingItemData->m_pTreeWidgetItem);
                pApplicationTree->selectItem(pActivatedItemMacthingItemData, true);

                QTableWidgetItem* pFirstColItem = pDataTable->item(pActivateItem->row(), 0);
                // Find the matching session window:
                CpuSessionWindow* pSessionWindow = pSessionViewCreator->findSessionWindow(pActivatedItemMacthingItemData);

                GT_IF_WITH_ASSERT((pSessionWindow != nullptr) && (pFirstColItem != nullptr))
                {
                    if ((pDataTable == m_pModulesTable) || (pDataTable == m_pProcessesTable))
                    {
                        SessionModulesView* pModulesView = pSessionWindow->sessionModulesView();

                        if (pModulesView == nullptr)
                        {
                            pSessionWindow->onViewModulesView(AGGREGATE_BY_PROCESSES);
                            pModulesView = pSessionWindow->sessionModulesView();
                        }

                        GT_IF_WITH_ASSERT(pModulesView != nullptr)
                        {
                            GT_IF_WITH_ASSERT(pFirstColItem != nullptr)
                            {
                                if (pDataTable == m_pProcessesTable)
                                {
                                    pModulesView->selectProcess(pFirstColItem->text());
                                }
                                else
                                {
                                    pModulesView->selectModule(pFirstColItem->text());
                                }
                            }
                        }
                    }
                    else
                    {
                        GT_IF_WITH_ASSERT(pDataTable == m_pFunctionsTable)
                        {
                            SessionFunctionView* pFunctionsView = pSessionWindow->sessionFunctionsView();

                            if (nullptr == pFunctionsView)
                            {
                                pSessionWindow->onViewFunctionTab(0U);
                            }

                            pFunctionsView = pSessionWindow->sessionFunctionsView();
                        }
                    }
                }
            }
        }
    }
}

void SessionOverviewWindow::onTableItemActivated(QTableWidgetItem* pActivateItem)
{
    // Sanity check:
    if (pActivateItem != nullptr)
    {
        CPUProfileDataTable* pTable = qobject_cast<CPUProfileDataTable*>(sender());
        int rowNum = pActivateItem->row();

        // open related window only if not empty row or "other" row
        if (rowNum != pTable->GetEmptyTableItemRowNum())
        {
            if (pTable == m_pFunctionsTable)
            {
                QTableWidgetItem* funcName = m_pFunctionsTable->item(rowNum, AMDT_FUNC_SUMMMARY_FUNC_NAME_COL);
                gtString str = acQStringToGTString(funcName->text());
                bool isOther = str.compare(L"other") == 0;
                bool isUnknown = str.startsWith(L"Unknown Module");

                // if double click on "other" row  open functions table
                if (isOther)
                {
                    openFunctionViewForModule(nullptr);
                }
                else
                {
                    if (!isUnknown)
                    {
                        openSourceCodeView(pActivateItem);
                    }
                }
            }
            else if (pTable == m_pModulesTable)
            {
                // if double click on "other" row  open modules table
                if (rowNum == pTable->GetOtherSamplesItemRowNum())
                {
                    openModulesViewForProcess(nullptr);
                }
                else
                {
                    openFunctionViewForModule(pActivateItem);
                }
            }
            else if (pTable == m_pProcessesTable)
            {
                openModulesViewForProcess(pActivateItem);
            }
        }
    }
}


void SessionOverviewWindow::onTableContextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType actionType,
                                                              QTableWidgetItem* pTableItem)
{
    GT_IF_WITH_ASSERT((pTableItem != nullptr) && (m_pFunctionsTable != nullptr))
    {
        switch (actionType)
        {
            case CPUProfileDataTable::DISPLAY_FUNCTION_IN_FUNCTIONS_VIEW:
            {
                openFunctionViewForFunction(pTableItem);
                break;
            }

            case CPUProfileDataTable::DISPLAY_FUNCTION_IN_CALLGRAPH_VIEW:
            {
                QString funcId = m_pFunctionsTable->getFunctionId(pTableItem->row());
                AMDTProfileFunctionData  functionData;
                m_pProfDataRdr->GetFunctionDetailedProfileData(funcId.toInt(), AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_THREADS, functionData);

                QList<ProcessIdType> pidList;

                for (const auto& pid : functionData.m_pidsList)
                {
                    pidList << pid;
                }

                GT_IF_WITH_ASSERT(!pidList.isEmpty())
                {
                    openCallGraphViewForFunction(m_pFunctionsTable->getFunctionId(pTableItem->row()).toInt(), pidList.first());
                }
                break;
            }

            case CPUProfileDataTable::DISPLAY_FUNCTION_IN_SOURCE_CODE_VIEW:
            {
                openSourceCodeView(pTableItem);
                break;
            }

            case CPUProfileDataTable::DISPLAY_MODULE_IN_MODULES_VIEW:
            {
                openModulesViewForModule(pTableItem);
                break;
            }

            case CPUProfileDataTable::DISPLAY_MODULE_IN_FUNCTIONS_VIEW:
            {
                openFunctionViewForModule(pTableItem);
                break;
            }

            case CPUProfileDataTable::DISPLAY_PROCESS_IN_MODULE_VIEW:
            {
                openModulesViewForProcess(pTableItem);
                break;
            }

            case CPUProfileDataTable::DISPLAY_PROCESS_IN_FUNCTIONS_VIEW:
            {
                openFunctionViewForProcess(pTableItem);
                break;
            }

            default:
            {
                GT_ASSERT_EX(false, L"Unsupported action");
                break;
            }
        }
    }
}

bool SessionOverviewWindow::openSourceCodeView(QTableWidgetItem* pTableItem)
{
    bool ret = true;

    GT_IF_WITH_ASSERT((m_pFunctionsTable != nullptr) &&
                      (m_pDisplayedSessionItemData != nullptr))
    {
        // Find the functions item data for this session:
        ProfileApplicationTreeHandler* instance = ProfileApplicationTreeHandler::instance();
        afApplicationTreeItemData* pActivatedItemMacthingItemData = instance->FindSessionChildItemData(m_pDisplayedSessionItemData,
                                                                    AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS);
        GT_IF_WITH_ASSERT(pActivatedItemMacthingItemData != nullptr)
        {
            int itemRow = pTableItem->row();

            // get the function id
            QString funcId = m_pFunctionsTable->getFunctionId(itemRow);
            QString modPath = m_pFunctionsTable->getModuleName(itemRow);
            AMDTUInt32 moduleId = AMDT_PROFILE_ALL_MODULES;
            AMDTUInt32  processId = AMDT_PROFILE_ALL_PROCESSES;

            //get the module id
            AMDTProfileModuleInfoVec moduleInfo;
            m_pProfDataRdr->GetModuleInfo(AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_MODULES, moduleInfo);

            for (const auto& mod : moduleInfo)
            {
                if (mod.m_path == acQStringToGTString(modPath))
                {
                    moduleId = mod.m_moduleId;
                    break;
                }
            }

            AMDTProfileDataVec processProfileData;
            m_pProfDataRdr->GetProcessProfileData(AMDT_PROFILE_ALL_PROCESSES, moduleId, processProfileData);

            for (const auto& proc : processProfileData)
            {
                processId = proc.m_id;
            }


            auto funcModInfo = std::make_tuple(funcId.toInt(), acQStringToGTString(modPath), moduleId, processId);
            // Emit a function activated signal (will open a source code view):
            emit opensourceCodeViewSig(funcModInfo);
        }
    }
    return ret;
}

const CpuProfileModule* SessionOverviewWindow::findModuleHandler(const osFilePath& filePath) const
{
    GT_UNREFERENCED_PARAMETER(filePath);
    const CpuProfileModule* pRetVal = nullptr;
    return pRetVal;
}

void SessionOverviewWindow::openFunctionViewForFunction(QTableWidgetItem* pTableItem)
{
    // Make sure that the function view is opened for this session:
    openFunctionsView();

    // Get the tree instance:
    afApplicationCommands* pCommands = afApplicationCommands::instance();
    SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();

    GT_IF_WITH_ASSERT((pCommands != nullptr) &&
                      (pSessionViewCreator != nullptr) &&
                      (m_pFunctionsTable != nullptr) &&
                      (m_pDisplayedSessionItemData != nullptr))
    {
        // Find the functions item data for this session:
        ProfileApplicationTreeHandler* instance = ProfileApplicationTreeHandler::instance();
        afApplicationTreeItemData* pActivatedItemMacthingItemData = instance->FindSessionChildItemData(m_pDisplayedSessionItemData,
                                                                    AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS);
        GT_IF_WITH_ASSERT(pActivatedItemMacthingItemData != nullptr)
        {
            int itemRow = pTableItem->row();

            // get the function id
            QString funcId = m_pFunctionsTable->getFunctionId(itemRow);

            // Find the matching session window:
            CpuSessionWindow* pSessionWindow = pSessionViewCreator->findSessionWindow(pActivatedItemMacthingItemData);

            GT_IF_WITH_ASSERT((pSessionWindow != nullptr) && (!funcId.isEmpty()))
            {
                SessionFunctionView* pFunctionsView = pSessionWindow->sessionFunctionsView();

                if (nullptr == pFunctionsView)
                {
                    pSessionWindow->onViewFunctionTab(0U);
                }

                pFunctionsView = pSessionWindow->sessionFunctionsView();

                pFunctionsView->selectFunction(funcId);
            }
        }
    }
}

void SessionOverviewWindow::openModulesViewForModule(QTableWidgetItem* pTableItem)
{
    openModulesView();

    // Get the tree instance:
    afApplicationCommands* pCommands = afApplicationCommands::instance();
    SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();

    GT_IF_WITH_ASSERT((pCommands != nullptr) && (pSessionViewCreator != nullptr) && (m_pFunctionsTable != nullptr) && (m_pDisplayedSessionItemData != nullptr))
    {
        afApplicationTreeItemData* pActivatedItemMacthingItemData = ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(m_pDisplayedSessionItemData, AF_TREE_ITEM_PROFILE_CPU_MODULES);
        GT_IF_WITH_ASSERT(pActivatedItemMacthingItemData != nullptr)
        {
            // Get the modules first column item:
            QTableWidgetItem* pFirstColItem = m_pModulesTable->item(pTableItem->row(), 0);

            // Find the matching session window:
            CpuSessionWindow* pSessionWindow = pSessionViewCreator->findSessionWindow(pActivatedItemMacthingItemData);

            GT_IF_WITH_ASSERT((pSessionWindow != nullptr) && (pFirstColItem != nullptr))
            {
                SessionModulesView* pModulesView = pSessionWindow->sessionModulesView();

                if (pModulesView == nullptr)
                {
                    pSessionWindow->onViewModulesView(AGGREGATE_BY_PROCESSES);
                    pModulesView = pSessionWindow->sessionModulesView();
                }

                GT_IF_WITH_ASSERT(pModulesView != nullptr)
                {
                    pModulesView->selectModule(pFirstColItem->text());
                }
            }
        }
    }
}

void SessionOverviewWindow::openFunctionViewForModule(QTableWidgetItem* pTableItem)
{
    // if item is nullptr just open table (happens in case of "other" row)
    if (pTableItem == nullptr)
    {
        openFunctionsView();
    }
    else
    {
        // Get the tree instance:
        afApplicationCommands* pCommands = afApplicationCommands::instance();
        SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();

        GT_IF_WITH_ASSERT((pCommands != nullptr) && (pSessionViewCreator != nullptr) && (m_pFunctionsTable != nullptr) && (m_pDisplayedSessionItemData != nullptr) && (m_pModulesTable != nullptr))
        {
            QString modulesFullPath;
            bool rc = m_pModulesTable->findModuleFilePath(pTableItem->row(), modulesFullPath);
            GT_IF_WITH_ASSERT(rc)
            {
                // Check if the module path contain "unknown module":
                bool isModuleUnKnown = modulesFullPath.toLower().startsWith("unknown module");

                if (isModuleUnKnown)
                {
                    acMessageBox::instance().information(AF_STR_InformationA, CP_strUnknownModule);
                }
                else
                {
                    // Make sure that the function view is opened for this session:
                    openFunctionsView();

                    // Find the functions item data for this session:
                    afApplicationTreeItemData* pActivatedItemMacthingItemData = ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(m_pDisplayedSessionItemData, AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS);
                    GT_IF_WITH_ASSERT(pActivatedItemMacthingItemData != nullptr)
                    {
                        // Find the matching session window:
                        CpuSessionWindow* pSessionWindow = pSessionViewCreator->findSessionWindow(pActivatedItemMacthingItemData);

                        GT_IF_WITH_ASSERT(pSessionWindow != nullptr)
                        {
                            SessionFunctionView* pFunctionsView = pSessionWindow->sessionFunctionsView();

                            if (pFunctionsView == nullptr)
                            {
                                pSessionWindow->onViewFunctionTab(0U);
                            }

                            pFunctionsView = pSessionWindow->sessionFunctionsView();
                            GT_IF_WITH_ASSERT(pFunctionsView != nullptr)
                            {
                                pFunctionsView->displayModule(modulesFullPath);
                            }
                        }
                    }
                }
            }
        }
    }
}

void SessionOverviewWindow::openFunctionViewForProcess(QTableWidgetItem* pTableItem)
{
    // Make sure that the function view is opened for this session:
    openFunctionsView();

    // Get the tree instance:
    afApplicationCommands* pCommands = afApplicationCommands::instance();
    SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();

    GT_IF_WITH_ASSERT((pCommands != nullptr) &&
                      (pSessionViewCreator != nullptr) &&
                      (m_pProcessesTable != nullptr) &&
                      (m_pDisplayedSessionItemData != nullptr) &&
                      (pTableItem != nullptr))
    {
        // Find the functions item data for this session:
        afApplicationTreeItemData* pActivatedItemMacthingItemData = ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(m_pDisplayedSessionItemData, AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS);
        GT_IF_WITH_ASSERT(pActivatedItemMacthingItemData != nullptr)
        {
            // Find the matching session window:
            CpuSessionWindow* pSessionWindow = pSessionViewCreator->findSessionWindow(pActivatedItemMacthingItemData);

            GT_IF_WITH_ASSERT(pSessionWindow != nullptr)
            {
                SessionFunctionView* pFunctionsView = pSessionWindow->sessionFunctionsView();

                if (pFunctionsView == nullptr)
                {
                    pSessionWindow->onViewFunctionTab(0U);
                }

                pFunctionsView = pSessionWindow->sessionFunctionsView();
                GT_IF_WITH_ASSERT(pFunctionsView != nullptr)
                {
                    ProcessIdType pid = 0;
                    QString processFileName;
                    bool rc = m_pProcessesTable->findProcessDetails(pTableItem->row(), pid, processFileName);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        pFunctionsView->filterByPID(pid);
                    }
                }
            }
        }
    }
}

void SessionOverviewWindow::openModulesViewForProcess(QTableWidgetItem* pTableItem)
{
    openModulesView();

    // if item is nullptr - just open table (happens in case of "other" row)
    if (pTableItem)
    {
        // Get the tree instance:
        afApplicationCommands* pCommands = afApplicationCommands::instance();
        SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();

        GT_IF_WITH_ASSERT((pCommands != nullptr) && (pSessionViewCreator != nullptr) && (m_pFunctionsTable != nullptr) && (m_pDisplayedSessionItemData != nullptr))
        {
            afApplicationTreeItemData* pActivatedItemMacthingItemData = ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(m_pDisplayedSessionItemData, AF_TREE_ITEM_PROFILE_CPU_MODULES);
            GT_IF_WITH_ASSERT(pActivatedItemMacthingItemData != nullptr)
            {
                // Get the modules first column item:
                QTableWidgetItem* pFirstColItem = m_pProcessesTable->item(pTableItem->row(), 0);

                // Find the matching session window:
                CpuSessionWindow* pSessionWindow = pSessionViewCreator->findSessionWindow(pActivatedItemMacthingItemData);

                GT_IF_WITH_ASSERT((pSessionWindow != nullptr) && (pFirstColItem != nullptr))
                {
                    SessionModulesView* pModulesView = pSessionWindow->sessionModulesView();

                    if (pModulesView == nullptr)
                    {
                        pSessionWindow->onViewModulesView(AGGREGATE_BY_PROCESSES);
                        pModulesView = pSessionWindow->sessionModulesView();
                    }

                    GT_IF_WITH_ASSERT(pModulesView != nullptr)
                    {
                        pModulesView->selectProcess(pFirstColItem->text());
                    }
                }
            }
        }
    }
}

void SessionOverviewWindow::UpdateTableDisplay(unsigned int updateType)
{
    // Avoid multiple updates
    m_isUpdatingData = true;

    (void)(updateType); // unused

    // Display the session data table:
    bool rcData = displaySessionDataTables();
    GT_ASSERT(rcData);

    // Display the session HTML properties:
    bool rcProps = displaySessionProperties();
    GT_ASSERT(rcProps);

    // Enable / disable the hot spot indicator combo box:
    bool rcHotSpot = fillHotspotIndicatorCombo();
    GT_ASSERT(rcHotSpot);

    m_isUpdatingData = false;
}

void SessionOverviewWindow::onEditCopy()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pFunctionsTable != nullptr) && (m_pProcessesTable != nullptr) && (m_pModulesTable != nullptr))
    {
        CPUProfileDataTable* pFocusedTable = m_pFunctionsTable;

        if (m_pModulesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pModulesTable;
        }

        if (m_pProcessesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pProcessesTable;
        }

        pFocusedTable->onEditCopy();
    }
}


void SessionOverviewWindow::onEditSelectAll()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pFunctionsTable != nullptr) && (m_pProcessesTable != nullptr) && (m_pModulesTable != nullptr))
    {
        if (m_pFunctionsTable == m_pLastFocusedWidget)
        {
            m_pFunctionsTable->onEditSelectAll();
            m_pFunctionsTable->setFocus(Qt::ActiveWindowFocusReason);
            m_pFunctionsTable->activateWindow();
            m_pProcessesTable->clearSelection();
            m_pModulesTable->clearSelection();
        }

        if (m_pModulesTable == m_pLastFocusedWidget)
        {
            m_pModulesTable->onEditSelectAll();
            m_pModulesTable->setFocus(Qt::ActiveWindowFocusReason);
            m_pModulesTable->activateWindow();
            m_pProcessesTable->clearSelection();
            m_pFunctionsTable->clearSelection();
        }

        if (m_pProcessesTable == m_pLastFocusedWidget)
        {
            m_pProcessesTable->onEditSelectAll();
            m_pProcessesTable->setFocus(Qt::ActiveWindowFocusReason);
            m_pProcessesTable->activateWindow();
            m_pModulesTable->clearSelection();
            m_pFunctionsTable->clearSelection();
        }
    }
}

void SessionOverviewWindow::onFindClick()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pFunctionsTable != nullptr) && (m_pProcessesTable != nullptr) && (m_pModulesTable != nullptr))
    {
        CPUProfileDataTable* pFocusedTable = m_pFunctionsTable;

        if (m_pModulesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pModulesTable;
        }

        if (m_pProcessesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pProcessesTable;
        }

        pFocusedTable->onFindClick();
    }
}

void SessionOverviewWindow::onFindNext()
{
    GT_IF_WITH_ASSERT((m_pFunctionsTable != nullptr) && (m_pProcessesTable != nullptr) && (m_pModulesTable != nullptr))
    {
        CPUProfileDataTable* pFocusedTable = m_pFunctionsTable;

        if (m_pModulesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pModulesTable;
        }

        if (m_pProcessesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pProcessesTable;
        }

        pFocusedTable->onFindNext();
    }

}
