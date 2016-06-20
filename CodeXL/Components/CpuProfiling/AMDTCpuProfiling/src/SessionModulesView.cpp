//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionModulesView.cpp
///
//==================================================================================

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>

/// Local:
#include <inc/SessionModulesView.h>

// Infra:
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acToolBar.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// AMDTSharedProfiling:
#include <SessionTreeNodeData.h>
#include <ProfileApplicationTreeHandler.h>

// Local:
#include <inc/AmdtCpuProfiling.h>
#include <inc/CpuProjectHandler.h>
#include <inc/CpuProfileTreeHandler.h>
#include <inc/SessionWindow.h>
#include <inc/SessionFunctionView.h>
#include <inc/SessionViewCreator.h>
#include <inc/ModulesDataTable.h>
#include <inc/ProcessesDataTable.h>
#include <inc/SessionModulesView.h>
#include <inc/StringConstants.h>
#include <inc/DisplayFilterDlg.h>


#define CP_MODULES_VIEW_TABLE_MIN_HEIGHT 50

SessionModulesView::SessionModulesView(QWidget* pParent, CpuSessionWindow* pSessionWindow)
    : DataTab(pParent, pSessionWindow)
{
    m_pList = nullptr;
    setMouseTracking(true);

    //Default status of CLU note
    m_CLUNoteShown = true;
    m_pDisplaySettings = &m_modulesTableFilter;
}

SessionModulesView::~SessionModulesView()
{
}

bool SessionModulesView::display(afApplicationTreeItemData* pItemData)
{
    bool retVal = false;

    // Set the displayed item data:
    m_pDisplayedSessionItemData = pItemData;

    // Initialize the display filters:
    initDisplayFilters();

    // Build the layout:
    setSessionWindowLayout();

    // Display the session using the input item data object:
    retVal = displaySessionDataTables();

    // Update the display string:
    updateDisplaySettingsString();

    return retVal;
}

void SessionModulesView::setSessionWindowLayout()
{
    m_pSplitterCentralWidget = new QSplitter(Qt::Vertical);

    m_pSplitterCentralWidget->setMouseTracking(true);
    setFocusProxy(m_pSplitterCentralWidget);
    setCentralWidget(m_pSplitterCentralWidget);

    // Create the top toolbar:
    m_pTopToolbar = createViewTopToolbar();

    SessionTreeNodeData* pSessionData = nullptr;
    GT_IF_WITH_ASSERT(m_pDisplayedSessionItemData != nullptr)
    {
        pSessionData = qobject_cast<SessionTreeNodeData*>(m_pDisplayedSessionItemData->extendedItemData());
    }

    gtVector<CPUProfileDataTable::TableContextMenuActionType> actions;
    actions.push_back(CPUProfileDataTable::DISPLAY_PROCESS_IN_FUNCTIONS_VIEW);

    if (m_isProfiledClu)
    {
        actions.push_back(CPUProfileDataTable::DISPLAY_CLU_NOTES);
    }

    m_pTopProcessesTable = new ProcessesDataTable(nullptr, actions, pSessionData);

    m_pTopProcessesTable->setMouseTracking(true);
    m_pTopProcessesTable->setMinimumHeight(CP_MODULES_VIEW_TABLE_MIN_HEIGHT);

    actions.push_back(CPUProfileDataTable::DISPLAY_BY_PROCESS_NAME);
    m_pBottomProcessesTable = new ProcessesDataTable(nullptr, actions, pSessionData);

    m_pBottomProcessesTable->setMouseTracking(true);
    m_pBottomProcessesTable->setMinimumHeight(CP_MODULES_VIEW_TABLE_MIN_HEIGHT);

    actions.clear();
    actions.push_back(CPUProfileDataTable::DISPLAY_MODULE_IN_FUNCTIONS_VIEW);

    if (m_isProfiledClu)
    {
        actions.push_back(CPUProfileDataTable::DISPLAY_CLU_NOTES);
    }

    m_pTopModulesTable = new ModulesDataTable(nullptr, actions, pSessionData, m_pParentSessionWindow);

    m_pTopModulesTable->setMouseTracking(true);
    m_pTopModulesTable->setMinimumHeight(CP_MODULES_VIEW_TABLE_MIN_HEIGHT);

    actions.push_back(CPUProfileDataTable::DISPLAY_BY_MODULE_NAME);
    m_pBottomModulesTable = new ModulesDataTable(nullptr, actions, pSessionData, m_pParentSessionWindow);

    m_pBottomModulesTable->setMouseTracking(true);
    m_pBottomModulesTable->setMinimumHeight(CP_MODULES_VIEW_TABLE_MIN_HEIGHT);

    // Connect table item activation:
    bool rc = connect(m_pTopProcessesTable, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(onTableItemActivated(QTableWidgetItem*)));
    GT_ASSERT(rc);
    rc = connect(m_pTopProcessesTable, SIGNAL(contextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType, QTableWidgetItem*)), this, SLOT(onTableContextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType, QTableWidgetItem*)));
    GT_ASSERT(rc);
    rc = connect(m_pBottomProcessesTable, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(onTableItemActivated(QTableWidgetItem*)));
    GT_ASSERT(rc);
    rc = connect(m_pBottomProcessesTable, SIGNAL(contextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType, QTableWidgetItem*)), this, SLOT(onTableContextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType, QTableWidgetItem*)));
    GT_ASSERT(rc);
    rc = connect(m_pTopModulesTable, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(onTableItemActivated(QTableWidgetItem*)));
    GT_ASSERT(rc);
    rc = connect(m_pTopModulesTable, SIGNAL(contextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType, QTableWidgetItem*)), this, SLOT(onTableContextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType, QTableWidgetItem*)));
    GT_ASSERT(rc);
    rc = connect(m_pBottomModulesTable, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(onTableItemActivated(QTableWidgetItem*)));
    GT_ASSERT(rc);
    rc = connect(m_pBottomModulesTable, SIGNAL(contextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType, QTableWidgetItem*)), this, SLOT(onTableContextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType, QTableWidgetItem*)));
    GT_ASSERT(rc);

    // Connect the top tables to current changed slots:
    rc = connect(m_pTopProcessesTable, SIGNAL(itemSelectionChanged()), this, SLOT(onProcessesTableCellChanged()));
    GT_ASSERT(rc);

    rc = connect(m_pTopModulesTable, SIGNAL(itemSelectionChanged()), this, SLOT(onModulesTableCellChanged()));
    GT_ASSERT(rc);

    rc = connect(m_pBottomProcessesTable, SIGNAL(itemSelectionChanged()), this, SLOT(onBottomProcessesTableCellChanged()));
    GT_ASSERT(rc);

    rc = connect(m_pBottomModulesTable, SIGNAL(itemSelectionChanged()), this, SLOT(onBottomModulesTableCellChanged()));
    GT_ASSERT(rc);

    QVBoxLayout* pUpperLayout = new QVBoxLayout;

    pUpperLayout->addWidget(m_pTopToolbar);
    pUpperLayout->addWidget(m_pTopProcessesTable);
    pUpperLayout->addWidget(m_pTopModulesTable);
    m_pTopModulesTable->hide();

    m_pProcessWidget = new QWidget;

    m_pProcessWidget->setLayout(pUpperLayout);

    m_pNoteWidget = new QTextEdit;

    m_pNoteWidget->setReadOnly(true);

    //Lower layout
    QVBoxLayout* pLowerLayout = new QVBoxLayout;


    m_pBottomLabel = new QLabel(CP_strModulesFiltered);

    QFont font = m_pBottomLabel->font();
    font.setBold(true);
    m_pBottomLabel->setFont(font);

    pLowerLayout->addWidget(m_pBottomLabel);
    pLowerLayout->addWidget(m_pBottomModulesTable);
    pLowerLayout->addWidget(m_pBottomProcessesTable);
    m_pBottomProcessesTable->hide();

    QWidget* m_pModuleWidget = new QWidget;

    m_pModuleWidget->setLayout(pLowerLayout);

    // Create the hint label frame:
    m_pHintFrame = createHintLabelFrame();


    if (m_isProcessesUp)
    {
        m_pSplitterCentralWidget->addWidget(m_pProcessWidget);
        m_pSplitterCentralWidget->addWidget(m_pModuleWidget);
    }
    else
    {
        m_pSplitterCentralWidget->addWidget(m_pModuleWidget);
        m_pSplitterCentralWidget->addWidget(m_pProcessWidget);
    }

    QWidget* pNotesWidget = new QWidget;


    QVBoxLayout* pNotesLayout = new QVBoxLayout;


    // Create CLU notes frame for CLU sessions:
    createCLUNotesFrame(pNotesLayout);

    pNotesWidget->setLayout(pNotesLayout);
    m_pSplitterCentralWidget->addWidget(pNotesWidget);

    m_pSplitterCentralWidget->addWidget(m_pHintFrame);

    m_pSplitterCentralWidget->setStretchFactor(0, 1);
    m_pSplitterCentralWidget->setStretchFactor(1, 2);
    m_pSplitterCentralWidget->setStretchFactor(2, 0);
    m_pSplitterCentralWidget->setStretchFactor(3, 0);

    // Initialize the window hint:
    updateHint(CP_modulesInformationHint);

    if (m_isProfiledClu)
    {
        updateHint(CP_modulesInformationHintForCLU);
    }


    // Set the tables' display filter:
    m_pTopModulesTable->setTableDisplaySettings(&m_modulesTableFilter, CurrentSessionDisplaySettings());
    m_pBottomModulesTable->setTableDisplaySettings(&m_modulesTableFilter, CurrentSessionDisplaySettings());
    m_pTopProcessesTable->setTableDisplaySettings(&m_processesTableFilter, CurrentSessionDisplaySettings());
    m_pBottomProcessesTable->setTableDisplaySettings(&m_processesTableFilter, CurrentSessionDisplaySettings());

    // Show the tables:
    ProtectedUpdateTableDisplay(UPDATE_TABLE_REBUILD);

    m_editActionsWidgetsList << m_pTopModulesTable;
    m_editActionsWidgetsList << m_pBottomModulesTable;
    m_editActionsWidgetsList << m_pTopProcessesTable;
    m_editActionsWidgetsList << m_pBottomProcessesTable;

}

bool SessionModulesView::displaySessionDataTables()
{
    bool retVal = false;

    ProcessesDataTable* pProcessTable = m_isProcessesUp ? m_pTopProcessesTable : m_pBottomProcessesTable;
    ModulesDataTable* pModulesTable = m_isProcessesUp ? m_pBottomModulesTable : m_pTopModulesTable;
    SessionDisplaySettings* pSessionDisplaySettings = CurrentSessionDisplaySettings();

    // Sanity check:
    GT_IF_WITH_ASSERT((pModulesTable != nullptr) && (pProcessTable != nullptr) && (pSessionDisplaySettings != nullptr))
    {
        retVal = true;

        // Display the data according to the requested filter:
        pModulesTable->blockSignals(true);
        pProcessTable->blockSignals(true);

        //retVal = pModulesTable->displayProfileData(m_pProfileReader) && retVal;
        //retVal = pProcessTable->displayProfileData(m_pProfileReader) && retVal;

        retVal = pModulesTable->displayTableData(m_pProfDataRdr,
                                                 m_pDisplayFilter,
                                                 AMDT_PROFILE_ALL_PROCESSES,
                                                 AMDT_PROFILE_ALL_MODULES) && retVal;
        retVal = pProcessTable->displayTableData(m_pProfDataRdr,
                                                 m_pDisplayFilter,
                                                 AMDT_PROFILE_ALL_PROCESSES,
                                                 AMDT_PROFILE_ALL_MODULES) && retVal;

        pModulesTable->blockSignals(false);
        pProcessTable->blockSignals(false);
    }

    return retVal;
}

void SessionModulesView::initDisplayFilters()
{
    // Set the display filter for the modules table:
    m_modulesTableFilter.m_hotSpotIndicatorColumnCaption = "";
    m_modulesTableFilter.m_displayedColumns.push_back(TableDisplaySettings::MODULE_ID);
    m_modulesTableFilter.m_displayedColumns.push_back(TableDisplaySettings::MODULE_NAME_COL);
    m_modulesTableFilter.m_displayedColumns.push_back(TableDisplaySettings::MODULE_SYMBOLS_LOADED);

    // Set the display filter for the processes table:
    m_processesTableFilter.m_displayedColumns.clear();
    m_processesTableFilter.m_displayedColumns.push_back(TableDisplaySettings::PROCESS_NAME_COL);
    m_processesTableFilter.m_displayedColumns.push_back(TableDisplaySettings::PID_COL);


    // Sanity check:
    SessionDisplaySettings* pSessionDisplaySettings = CurrentSessionDisplaySettings();
    GT_IF_WITH_ASSERT((m_pProfileReader != nullptr) && (pSessionDisplaySettings != nullptr))
    {
        pSessionDisplaySettings->calculateDisplayedColumns(m_pProfileReader->getTopologyMap());
        pSessionDisplaySettings->calculateDisplayedColumns(m_pProfileReader->getTopologyMap());
    }
}

acToolBar* SessionModulesView::createViewTopToolbar()
{
    // Create the layout for the top items:
    acToolBar* pRetVal = new acToolBar(nullptr);


    // Do not allow the toolbar to float:
    pRetVal->setFloatable(false);
    pRetVal->setMovable(false);
    pRetVal->setStyleSheet("QToolBar { border-style: none; }");

    m_pTopLabelAction = pRetVal->AddLabel(CP_strProcesses, true, false, 0);
    GT_ASSERT(m_pTopLabelAction != nullptr);

    QWidget* pEmptySpacer = new QWidget;

    pEmptySpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    pRetVal->addWidget(pEmptySpacer);

    QString caption = CP_strCPUProfileToolbarBase;
    caption.append(":");
    acWidgetAction* pAction = pRetVal->AddLabel(caption);

    const QLabel* pTest = TopToolbarLabel(pAction);
    int width = 25;

    if (pTest != nullptr)
    {
        static QString longestDisplayFilter = "Misaligned access assessment, ";
        longestDisplayFilter.append("Percentages, System DLLs, All CPUs, ");
        longestDisplayFilter.append(CP_strCPUProfilePerNuma);
        width = pTest->fontMetrics().boundingRect(longestDisplayFilter).width();
    }

    // Create the display filter link:
    acToolbarActionData actionData(SIGNAL(linkActivated(const QString&)), this, SLOT(OnDisplaySettingsClicked()));
    actionData.m_actionType = acToolbarActionData::AC_LABEL_ACTION;
    actionData.m_margin = 5;
    actionData.m_minWidth = width;
    actionData.m_text = CP_strDisplayBy;
    actionData.m_objectName = "ModulesDisplayFilterLink";

    m_pDisplaySettingsAction = pRetVal->AddWidget(actionData);
    GT_ASSERT(m_pDisplaySettingsAction != nullptr);
    pRetVal->AddLabel(CP_strDisplayBy);

    QStringList comboTexts;
    comboTexts << CP_strProcesses;
    comboTexts << CP_strModules;
    m_pComboBoxDisplayByAction = pRetVal->AddComboBox(comboTexts, SIGNAL(currentIndexChanged(int)), this, SLOT(onDisplayByChanged(int)));
    GT_ASSERT(m_pComboBoxDisplayByAction);

    return pRetVal;
}

void SessionModulesView::selectModule(const QString& moduleName)
{
    ModulesDataTable* pModulesTable = m_isProcessesUp ? m_pBottomModulesTable : m_pTopModulesTable;

    // Make sure that we display by module:
    onDisplayByChanged(0);

    // Sanity check:
    GT_IF_WITH_ASSERT(pModulesTable != nullptr)
    {
        selectTableItem(pModulesTable, moduleName, 0);
    }
}

void SessionModulesView::selectProcess(const QString& processName)
{
    displayByProcesses();

    ProcessesDataTable* pProcessTable = m_isProcessesUp ? m_pTopProcessesTable : m_pBottomProcessesTable;

    // Sanity check:
    GT_IF_WITH_ASSERT(pProcessTable != nullptr)
    {
        selectTableItem(pProcessTable, processName, 0);
    }
}

void SessionModulesView::onDisplayByChanged(int index)
{
    (void)(index); // unused
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pComboBoxDisplayByAction != nullptr) && (m_pTopProcessesTable != nullptr) && (m_pTopModulesTable != nullptr)
                      && (m_pBottomProcessesTable != nullptr) && (m_pBottomModulesTable != nullptr) && (m_pTopLabelAction != nullptr) && (m_pTopToolbar != nullptr))
    {
        const QComboBox* pComboBoxDisplayBy = TopToolbarComboBox(m_pComboBoxDisplayByAction);
        GT_IF_WITH_ASSERT(pComboBoxDisplayBy != nullptr)
        {
            m_isProcessesUp = (pComboBoxDisplayBy->itemText(index) == CP_strProcesses);
        }

        //GT_ASSERT_EX(false, L"Fix the following commented lines (compatibility issues in with Qt 5.2).");
        m_pBottomModulesTable->setVisible(m_isProcessesUp);
        m_pTopProcessesTable->setVisible(m_isProcessesUp);

        m_pTopModulesTable->setVisible(!m_isProcessesUp);
        m_pBottomProcessesTable->setVisible(!m_isProcessesUp);

        QString topLabel = m_isProcessesUp ? CP_strProcesses : CP_strModules;
        QString bottomLabel = m_isProcessesUp ? CP_strModules : CP_strProcesses;
        m_pTopLabelAction->UpdateText(topLabel);
        m_pBottomLabel->setText(bottomLabel);

        // Reset the filters:
        m_modulesTableFilter.m_filterByPIDsList.clear();
        m_processesTableFilter.m_filterByModulePathsList.clear();

        // Update the view with the new filter configuration:
        ProtectedUpdateTableDisplay(UPDATE_TABLE_REBUILD);
    }
}

void SessionModulesView::onProcessesTableCellChanged()
{
    GT_IF_WITH_ASSERT((m_pTopProcessesTable != nullptr) && (m_pBottomModulesTable != nullptr) &&
                      (m_pBottomLabel != nullptr))
    {
        ProcessIdType selectedProcessID = AMDT_PROFILE_ALL_PROCESSES;

        foreach (QTableWidgetItem* pSelectedItem, m_pTopProcessesTable->selectedItems())
        {
            QString processFileName;
            bool rc = m_pTopProcessesTable->findProcessDetails(pSelectedItem->row(), selectedProcessID, processFileName);
            GT_ASSERT(rc);
        }

        // Display the modules table with the updated filter:
        m_pBottomModulesTable->displayTableData(m_pProfDataRdr,
                                                m_pDisplayFilter,
                                                static_cast<AMDTProcessId>(selectedProcessID),
                                                AMDT_PROFILE_ALL_MODULES);
        m_pBottomLabel->setText(CP_strModulesFiltered);

#if 0
        m_modulesTableFilter.m_filterByPIDsList.clear();

        // Add all current selected items to the filters modules list:
        foreach (QTableWidgetItem* pSelectedItem, m_pTopProcessesTable->selectedItems())
        {
            // Get the PID for the selected index:
            ProcessIdType selectedProcessID = 0;
            QString processFileName;
            bool rc = m_pTopProcessesTable->findProcessDetails(pSelectedItem->row(), selectedProcessID, processFileName);
            GT_IF_WITH_ASSERT(rc)
            {
                int index = m_modulesTableFilter.m_filterByPIDsList.indexOf(selectedProcessID);

                if (index < 0)
                {
                    m_modulesTableFilter.m_filterByPIDsList << selectedProcessID;
                }
            }

            if (m_isProfiledClu)
            {
                gtVector<float> cluData;
                m_pTopProcessesTable->GetCluDataInRow(pSelectedItem->row(), SAMPLE_INDEX_IN_TABLE_PROCESS, cluData);
                UpdateNoteWindowContent(cluData);
            }
        }

        // Display the modules table with the updated filter:
        bool rc = m_pBottomModulesTable->displayProfileData(m_pProfileReader);
        GT_ASSERT(rc);

        // Set the bottom label:
#endif
        //bool rc = m_pTopProcessesTable->displayTableData(m_pProfDataRdr, m_pDisplayFilter);
        //GT_ASSERT(rc);
        //rc = m_pBottomModulesTable->displayTableData(m_pProfDataRdr, m_pDisplayFilter);
        //m_pBottomLabel->setText(CP_strModulesFiltered);
    }
}

void SessionModulesView::onModulesTableCellChanged()
{

    GT_IF_WITH_ASSERT((m_pTopModulesTable != nullptr) && (m_pBottomProcessesTable != nullptr)
                      && (m_pBottomLabel != nullptr))
    {
        AMDTModuleId modId = AMDT_PROFILE_ALL_MODULES;

        foreach (QTableWidgetItem* pSelectedItem, m_pTopModulesTable->selectedItems())
        {
            QString processFileName;
            bool rc = m_pTopModulesTable->findModueId(pSelectedItem->row(), modId);
            GT_ASSERT(rc);
        }

        // Display the modules table with the updated filter:
		m_pBottomProcessesTable->displayTableData(m_pProfDataRdr,
                                                m_pDisplayFilter,
                                                AMDT_PROFILE_ALL_PROCESSES,
                                                static_cast<AMDTProcessId>(modId));
        m_pBottomLabel->setText(CP_strModulesFiltered);

#if 0
        // Set the module file name as filter to the processes table:
        m_processesTableFilter.m_filterByModulePathsList.clear();

        // Add all current selected items to the filters modules list:
        foreach (QTableWidgetItem* pSelectedItem, m_pTopModulesTable->selectedItems())
        {
            // Get the file module name for the selected index:
            QString moduleFilePath;
            int selectedRow = pSelectedItem->row();
            bool rc = m_pTopModulesTable->findModuleFilePath(selectedRow, moduleFilePath);
            GT_IF_WITH_ASSERT(rc)
            {
                int index = m_processesTableFilter.m_filterByModulePathsList.indexOf(moduleFilePath);

                if (index < 0)
                {
                    m_processesTableFilter.m_filterByModulePathsList << moduleFilePath;
                }
            }

            if (m_isProfiledClu)
            {
                gtVector<float> cluData;
                m_pTopModulesTable->GetCluDataInRow(pSelectedItem->row(), SAMPLE_INDEX_IN_TABLE_MODULE, cluData);
                UpdateNoteWindowContent(cluData);
            }
        }

        // Display the modules table with the updated filter:
        bool rc = m_pBottomProcessesTable->displayProfileData(m_pProfileReader);
        GT_ASSERT(rc);

        // Set the bottom label:
#endif
        //bool rc = m_pTopModulesTable->displayTableData(m_pProfDataRdr, m_pDisplayFilter);
        //GT_ASSERT(rc);
        //rc = m_pBottomProcessesTable->displayTableData(m_pProfDataRdr, m_pDisplayFilter);
        //m_pBottomLabel->setText(CP_strProcessesFiltered);
    }
}

void SessionModulesView::onTableItemActivated(QTableWidgetItem* pActivateItem)
{
    GT_IF_WITH_ASSERT(pActivateItem != nullptr)
    {
        CPUProfileDataTable* pTable = qobject_cast<CPUProfileDataTable*>(sender());

        // activateTableItem only if not empty row message (no "other" row in module view
        if (pActivateItem->row() != pTable->GetEmptyTableItemRowNum())
        {
            activateTableItem(pActivateItem, pTable);
        }
    }
}


void SessionModulesView::onTableContextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType actionType, QTableWidgetItem* pTableItem)
{
    if (CPUProfileDataTable::DISPLAY_CLU_NOTES == actionType)
    {
        // Select the module item in top table:
        if (m_pNoteWidget->isHidden())
        {
            // BUG421904: Module view and Function view m_CLUNoteShown note should
            // be kept separate
            CPUProfileDataTable::m_CLUNoteShown = m_CLUNoteShown = true;
            m_pNoteWidget->show();
            m_pNoteHeader->show();
        }
        else
        {
            CPUProfileDataTable::m_CLUNoteShown = m_CLUNoteShown = false;
            m_pNoteWidget->hide();
            m_pNoteHeader->hide();
        }

        return;
    }

    GT_IF_WITH_ASSERT((pTableItem != nullptr) && (m_pComboBoxDisplayByAction != nullptr) && (m_pTopToolbar != nullptr))
    {
        switch (actionType)
        {
            case CPUProfileDataTable::DISPLAY_MODULE_IN_FUNCTIONS_VIEW:
            {
                // Perform an activation item:
                CPUProfileDataTable* pTable = m_isProcessesUp ? m_pBottomModulesTable : m_pTopModulesTable ;
                activateTableItem(pTableItem, pTable);
                break;
            }

            case CPUProfileDataTable::DISPLAY_PROCESS_IN_FUNCTIONS_VIEW:
            {
                // Perform an activation item:
                CPUProfileDataTable* pTable = m_isProcessesUp ? m_pTopProcessesTable : m_pBottomProcessesTable ;
                activateTableItem(pTableItem, pTable);
                break;
            }

            case CPUProfileDataTable::DISPLAY_BY_MODULE_NAME:
            {
                const QComboBox* pComboBoxDisplayBy = TopToolbarComboBox(m_pComboBoxDisplayByAction);
                GT_IF_WITH_ASSERT(pComboBoxDisplayBy != nullptr)
                {
                    m_isProcessesUp = (pComboBoxDisplayBy->currentText() == CP_strProcesses);

                    // Set the display by to modules:
                    m_pComboBoxDisplayByAction->UpdateCurrentIndex(1);
                }

                // Select the module item in top table:
                selectModule(pTableItem->text());

                break;
            }

            case CPUProfileDataTable::DISPLAY_BY_PROCESS_NAME:
            {
                // Select the module item in top table:
                selectProcess(pTableItem->text());

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

void SessionModulesView::activateTableItem(QTableWidgetItem* pActivateItem, CPUProfileDataTable* pDataTable)
{
    bool isProcessesTable = false;
    bool isModulesTable = false;
    ProcessIdType pid = 0;
    bool isModuleUnKnown = true;
    QStringList modulesFilePaths;

    // check if Process table
    ProcessesDataTable* pProcessesTable = qobject_cast<ProcessesDataTable*>(pDataTable);

    if (pProcessesTable != nullptr)
    {
        isProcessesTable = true;
        // Find the activated item pid:
        QString processFilePath;

        bool rc = pProcessesTable->findProcessDetails(pActivateItem->row(), pid, processFilePath);
        GT_ASSERT(rc);
    }
    else  // check if Modules table
    {
        ModulesDataTable* pModulesTable = qobject_cast<ModulesDataTable*>(pDataTable);

        if (pModulesTable != nullptr)
        {
            isModulesTable = true;
            // get selected module
            QTableWidgetItem* pSelectedItem = pModulesTable->selectedItems().first();

            QString moduleFilePath;

            // findModuleFilePath - gets the name of the module file for each items from the row =>
            // no need for doing the same for all the items because all will get the same file name
            bool rc = pModulesTable->findModuleFilePath(pSelectedItem->row(), moduleFilePath);
            // if rc is false - the case is like unknown module
            GT_IF_WITH_ASSERT(rc)
            {
                isModuleUnKnown = moduleFilePath.toLower().startsWith("unknown module");
            }

            if (isModuleUnKnown)
            {
                acMessageBox::instance().information(AF_STR_InformationA, CP_strUnknownModule);
                return;
            }

            modulesFilePaths << moduleFilePath;

        }
    }

    // Get the tree instance:
    afApplicationCommands* pCommands = afApplicationCommands::instance();
    SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();

    GT_IF_WITH_ASSERT((pCommands != nullptr) && (pSessionViewCreator != nullptr) && (pDataTable != nullptr))
    {
        afApplicationTree* pApplicationTree = pCommands->applicationTree();
        SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(m_pDisplayedSessionItemData->extendedItemData());
        GT_IF_WITH_ASSERT((m_pDisplayedSessionItemData != nullptr) && (pApplicationTree != nullptr) && (pSessionData != nullptr))
        {
            afApplicationTreeItemData* pSessionItemData = ProfileApplicationTreeHandler::instance()->FindItemByProfileFilePath(m_pDisplayedSessionItemData->m_filePath);
            GT_IF_WITH_ASSERT(pSessionItemData != nullptr)
            {
                afApplicationTreeItemData* pActivatedItemMacthingItemData = ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(pSessionItemData, AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS);
                GT_IF_WITH_ASSERT(pActivatedItemMacthingItemData != nullptr)
                {
                    // Get the tree instance:
                    pApplicationTree->expandItem(pActivatedItemMacthingItemData->m_pTreeWidgetItem);
                    pApplicationTree->selectItem(pActivatedItemMacthingItemData, true);

                    QTableWidgetItem* pFirstColItem = pDataTable->item(pActivateItem->row(), 0);

                    //SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();
                    GT_IF_WITH_ASSERT(pSessionViewCreator != nullptr)
                    {
                        // Find the session window related to this path:
                        CpuSessionWindow* pSessionWindow = pSessionViewCreator->findSessionWindow(pActivatedItemMacthingItemData);
                        GT_IF_WITH_ASSERT(pSessionWindow != nullptr)
                        {
                            // Open this process / module in the functions view:
                            SessionFunctionView* pFunctionsView = pSessionWindow->sessionFunctionsView();

                            if (pFunctionsView == nullptr)
                            {
                                pSessionWindow->onViewFunctionTab(0U);
                            }

                            pFunctionsView = pSessionWindow->sessionFunctionsView();
                            GT_IF_WITH_ASSERT((pFunctionsView != nullptr) && (pFirstColItem != nullptr))
                            {
                                GT_IF_WITH_ASSERT(pFirstColItem != nullptr)
                                {
                                    if (isProcessesTable)
                                    {
                                        pFunctionsView->filterByPID(pid);
                                    }
                                    else if (isModulesTable && !isModuleUnKnown)
                                    {
                                        // Filter the functions view with the list:
                                        pFunctionsView->FilterByModuleFilePaths(modulesFilePaths);
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


void SessionModulesView::UpdateTableDisplay(unsigned int updateType)
{
    (void)(updateType); // unused

    SessionDisplaySettings* pSessionDisplaySettings = CurrentSessionDisplaySettings();
    GT_IF_WITH_ASSERT(pSessionDisplaySettings != nullptr)
    {
        // Update the table's selection:
        if (m_isProcessesUp)
        {
            // Display the modules table with the updated filter:
            //bool rc = m_pTopProcessesTable->displayProfileData(m_pProfileReader);
            bool rc = m_pTopProcessesTable->displayTableData(m_pProfDataRdr, m_pDisplayFilter,
                                                             AMDT_PROFILE_ALL_PROCESSES,
                                                             AMDT_PROFILE_ALL_MODULES);
            GT_ASSERT(rc);

            onProcessesTableCellChanged();
        }
        else
        {
            // Display the modules table with the updated filter:
            //bool rc = m_pTopModulesTable->displayProfileData(m_pProfileReader);
            bool rc = m_pTopModulesTable->displayTableData(m_pProfDataRdr, m_pDisplayFilter,
                                                           AMDT_PROFILE_ALL_PROCESSES,
                                                           AMDT_PROFILE_ALL_MODULES);
            GT_ASSERT(rc);

            onModulesTableCellChanged();
        }
    }
}

void SessionModulesView::displayByProcesses()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pComboBoxDisplayByAction != nullptr) && (m_pTopToolbar != nullptr))
    {
        const QComboBox* pComboBoxDisplayBy = TopToolbarComboBox(m_pComboBoxDisplayByAction);
        GT_IF_WITH_ASSERT(pComboBoxDisplayBy != nullptr)
        {
            int index = pComboBoxDisplayBy->findText(CP_strProcesses);
            GT_IF_WITH_ASSERT((index >= 0) && (index < pComboBoxDisplayBy->count()))
            {
                m_pComboBoxDisplayByAction->UpdateCurrentIndex(index);
            }
        }
    }
}

void SessionModulesView::onBottomProcessesTableCellChanged()
{
    if (m_isProfiledClu)
    {
        foreach (QTableWidgetItem* pSelectedItem, m_pBottomProcessesTable->selectedItems())
        {
            gtVector<float> cluData;
            m_pBottomProcessesTable->GetCluDataInRow(pSelectedItem->row(), SAMPLE_INDEX_IN_TABLE_PROCESS, cluData);
            UpdateNoteWindowContent(cluData);
        }

        m_latestSelectedTable = TOP_PROCESS_TABLE;
    }
}

void SessionModulesView::onBottomModulesTableCellChanged()
{
    if (m_isProfiledClu)
    {
        foreach (QTableWidgetItem* pSelectedItem, m_pBottomModulesTable->selectedItems())
        {
            gtVector<float> cluData;
            m_pBottomModulesTable->GetCluDataInRow(pSelectedItem->row(), SAMPLE_INDEX_IN_TABLE_MODULE, cluData);
            UpdateNoteWindowContent(cluData);
        }

        m_latestSelectedTable = TOP_MODULE_TABLE;
    }
}

void SessionModulesView::onEditCopy()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pTopProcessesTable != nullptr) && (m_pBottomProcessesTable != nullptr) &&
                      (m_pTopModulesTable != nullptr) && (m_pBottomModulesTable != nullptr))

    {
        CPUProfileDataTable* pFocusedTable = m_pTopProcessesTable;

        if (m_pBottomProcessesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pBottomProcessesTable;
        }

        if (m_pTopModulesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pTopModulesTable;
        }

        if (m_pBottomModulesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pBottomModulesTable;
        }

        pFocusedTable->onEditCopy();
    }
}


void SessionModulesView::onEditSelectAll()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pTopProcessesTable != nullptr) && (m_pBottomProcessesTable != nullptr) &&
                      (m_pTopModulesTable != nullptr) && (m_pBottomModulesTable != nullptr))

    {
        CPUProfileDataTable* pFocusedTable = m_pTopProcessesTable;

        if (m_pBottomProcessesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pBottomProcessesTable;
        }

        if (m_pTopModulesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pTopModulesTable;
        }

        if (m_pBottomModulesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pBottomModulesTable;
        }

        pFocusedTable->onEditSelectAll();

        pFocusedTable->setFocus();
        pFocusedTable->activateWindow();
        setFocus();
    }
}

void SessionModulesView::onFindClick()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pTopProcessesTable != nullptr) && (m_pBottomProcessesTable != nullptr) &&
                      (m_pTopModulesTable != nullptr) && (m_pBottomModulesTable != nullptr))

    {
        CPUProfileDataTable* pFocusedTable = m_pTopProcessesTable;

        if (m_pBottomProcessesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pBottomProcessesTable;
        }

        if (m_pTopModulesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pTopModulesTable;
        }

        if (m_pBottomModulesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pBottomModulesTable;
        }

        pFocusedTable->onFindClick();
    }
}

void SessionModulesView::onFindNext()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pTopProcessesTable != nullptr) && (m_pBottomProcessesTable != nullptr) &&
                      (m_pTopModulesTable != nullptr) && (m_pBottomModulesTable != nullptr))

    {
        CPUProfileDataTable* pFocusedTable = m_pTopProcessesTable;

        if (m_pBottomProcessesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pBottomProcessesTable;
        }

        if (m_pTopModulesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pTopModulesTable;
        }

        if (m_pBottomModulesTable == m_pLastFocusedWidget)
        {
            pFocusedTable = m_pBottomModulesTable;
        }

        pFocusedTable->onFindNext();
    }
}

