//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionFunctionView.cpp
///
//==================================================================================

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>

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
#include <inc/CpuProjectHandler.h>
#include <inc/CpuProfileTreeHandler.h>
#include <inc/FunctionsDataTable.h>
#include <inc/SessionFunctionView.h>
#include <inc/StringConstants.h>
#include <inc/DisplayFilterDlg.h>
#include <inc/Auxil.h>
#include <inc/SessionFunctionView.h>

SessionFunctionView::SessionFunctionView(QWidget* pParent, CpuSessionWindow* pSessionWindow)
    :  DataTab(pParent, pSessionWindow)
{
    setMouseTracking(true);

    //Default status of CLU note
    m_CLUNoteShown = true;

    m_updateData = true;
    setFilter(&m_functionsTablesFilter);
}


SessionFunctionView::~SessionFunctionView()
{
}

bool SessionFunctionView::display(afApplicationTreeItemData* pItemData)
{
    bool retVal = false;

    // Set the displayed item data:
    m_pDisplayedSessionItemData = pItemData;

    // Initialize the display filters:
    initDisplayFilters();

    // Build the layout:
    setSessionWindowLayout();

    fillPIDComb();

    updateDataFromPidComboBox();

    // Display the session using the input item data object:
    QString str = updateModulesFilterLinkString();

    // Fill the table's data:
    retVal = displaySessionDataTables();

    // Update display filter string:
    updateDisplaySettingsString();
    return retVal;
}

void SessionFunctionView::setSessionWindowLayout()
{
    // Create bottom and top widgets (top for the tables, bottom for the properties and hint window):
    QWidget* pCentralWidget = new QWidget;

    pCentralWidget->setMouseTracking(true);

    setFocusProxy(pCentralWidget);
    setCentralWidget(pCentralWidget);

    bool isCSSEnabled = false;
    CPUSessionTreeItemData* pSessionData = nullptr;
    GT_IF_WITH_ASSERT(m_pDisplayedSessionItemData != nullptr)
    {
        pSessionData = qobject_cast<CPUSessionTreeItemData*>(m_pDisplayedSessionItemData->extendedItemData());
        isCSSEnabled = pSessionData->ShouldCollectCSS(true);
    }

    gtVector<CPUProfileDataTable::TableContextMenuActionType> actions;
    actions.clear();
    actions.push_back(CPUProfileDataTable::DISPLAY_FUNCTION_IN_SOURCE_CODE_VIEW);

    if (isCSSEnabled)
    {
        actions.push_back(CPUProfileDataTable::DISPLAY_FUNCTION_IN_CALLGRAPH_VIEW);
    }

    if (m_isProfiledClu)
    {
        actions.push_back(CPUProfileDataTable::DISPLAY_CLU_NOTES);
    }

    m_pFunctionTable = new FunctionsDataTable(this, actions, pSessionData, m_pParentSessionWindow);

    m_pFunctionTable->setMouseTracking(true);

    // If a file cannot be found, popup the user:
    m_pFunctionTable->setPopupToBrowseMissingFiles(false);

    // Connect the activate signal in functions table:
    bool rc = connect(m_pFunctionTable, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(onTableItemActivated(QTableWidgetItem*)));
    GT_ASSERT(rc);

    // Connect context menu actions in functions table:
    rc = connect(m_pFunctionTable, SIGNAL(contextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType, QTableWidgetItem*)), this, SLOT(onTableContextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType, QTableWidgetItem*)));
    GT_ASSERT(rc);

    rc = connect(m_pFunctionTable, SIGNAL(itemSelectionChanged()), this, SLOT(onCellChanged()));
    GT_ASSERT(rc);

    // Create top toolbar:
    CreateToolbar();
    GT_ASSERT(m_pTopToolbar != nullptr);

    // Create the hint label frame:
    QFrame* pFrame = createHintLabelFrame();


    QVBoxLayout* pMainLayout = new QVBoxLayout;


    QLabel* pHeaderLabel = new QLabel(CP_overviewPageHotspotIndicatorHeader);


    QFont headerFont = pHeaderLabel->font();
    headerFont.setUnderline(true);
    pHeaderLabel->setFont(headerFont);

    pMainLayout->addWidget(m_pTopToolbar);
    pMainLayout->addWidget(m_pFunctionTable);

    // Create CLU notes frame for CLU sessions:
    //createCLUNotesFrame(pMainLayout);

    pMainLayout->addWidget(pFrame);

    pCentralWidget->setLayout(pMainLayout);

    // Initialize the window hint:
    updateHint(CP_functionsInformationHint);

    if (m_isProfiledClu)
    {
        updateHint(CP_functionsInformationHintForCLU);
    }

    // Set the table display settings:
    m_pFunctionTable->setTableDisplaySettings(&m_functionsTablesFilter);
}

bool SessionFunctionView::displaySessionDataTables()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pFunctionTable != nullptr) &&
                      (m_pProfDataRdr != nullptr) &&
                      (m_pDisplayFilter != nullptr))
    {
        // Use case
        //1. all module all process
        //2. all module single process
        int moduleShown = m_functionsTablesFilter.m_filterByModulePathsList.size();

        // all module shown
        if (0 == moduleShown)
        {
            // all process
            AMDTProcessId pid = AMDT_PROFILE_ALL_PROCESSES;

            // single process
            if (1 == m_functionsTablesFilter.m_filterByPIDsList.size())
            {
                pid = m_functionsTablesFilter.m_filterByPIDsList.at(0);
            }

            retVal = m_pFunctionTable->displayTableData(m_pProfDataRdr,
                                                        m_pDisplayFilter,
                                                        pid,
                                                        m_moduleId);
        }
        else
        {
            //3. multiple/single module single Process
            //4. multiple / single module all process
            int selModSize = m_functionsTablesFilter.m_filterByModulePathsList.size();
            std::vector<AMDTUInt64> selecedModIdVec;

            for (int i = 0; i < selModSize; ++i)
            {
                gtString modName = acQStringToGTString(m_functionsTablesFilter.m_filterByModulePathsList.at(i));
                auto itr = m_moduleNameIdMap.find(modName);

                if (m_moduleNameIdMap.end() != itr)
                {
                    selecedModIdVec.push_back(itr->second);
                }
            }

            retVal = m_pFunctionTable->displayTableData(m_pProfDataRdr,
                                                        m_pDisplayFilter,
                                                        AMDT_PROFILE_ALL_PROCESSES,
                                                        m_moduleId,
                                                        selecedModIdVec);
        }

    }

    return retVal;
}

void SessionFunctionView::initDisplayFilters()
{
    // Set the display filter for the modules table:
    m_functionsTablesFilter.m_displayedColumns.push_back(TableDisplaySettings::FUNCTION_ID);
    m_functionsTablesFilter.m_displayedColumns.push_back(TableDisplaySettings::FUNCTION_NAME_COL);
    m_functionsTablesFilter.m_displayedColumns.push_back(TableDisplaySettings::MODULE_NAME_COL);
}


void SessionFunctionView::onDisplayFilterChanged(int index)
{
    Q_UNUSED(index);

    // Update the view with the new filter configuration:
    displaySessionDataTables();
}

void SessionFunctionView::CreateToolbar()
{
    // Create the layout for the top items:
    m_pTopToolbar = new acToolBar(nullptr);


    // Do not allow the toolbar to float:
    m_pTopToolbar->setFloatable(false);
    m_pTopToolbar->setMovable(false);
    m_pTopToolbar->setStyleSheet("QToolBar { border-style: none; }");
    m_pTopToolbar->setContentsMargins(0, 0, 0, 0);

    // Display system module is initialized with the value of the current display system module value:
    m_functionsTablesFilter.m_shouldDisplaySystemModuleInModulesDlg = !m_pDisplayFilter->IsSystemModuleIgnored();

    m_pTopToolbar->AddLabel(CP_strFunctions, true, false, 0);

    QWidget* emptySpacer = new QWidget;
    emptySpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_pTopToolbar->addWidget(emptySpacer);

    acToolbarActionData captionActionData;
    captionActionData.m_text = CP_strCPUProfileToolbarBase;
    captionActionData.m_text.append(":");
    captionActionData.m_actionType = acToolbarActionData::AC_LABEL_ACTION;
    captionActionData.m_textAlignment = Qt::AlignRight | Qt::AlignVCenter;
    captionActionData.m_objectName = "FunctionDisplayFilterLink";

    acWidgetAction* pTestAction = m_pTopToolbar->AddWidget(captionActionData);
    const QLabel* pTestLabel = TopToolbarLabel(pTestAction);

    if (pTestLabel != nullptr)
    {
        static QString longestDisplayFilter = "Misaligned access assessment, ";
        longestDisplayFilter.append("Percentages, System DLLs, All CPUs, ");
        longestDisplayFilter.append(CP_strCPUProfilePerNuma);
    }

    acToolbarActionData settingsActionData;
    settingsActionData.m_text = CP_strDisplayBy;
    settingsActionData.m_actionType = acToolbarActionData::AC_LABEL_ACTION;
    settingsActionData.m_pSignal = SIGNAL(linkActivated(const QString&));
    settingsActionData.m_pReceiver = this;
    settingsActionData.m_pMember = SLOT(OnDisplaySettingsClicked());
    settingsActionData.m_objectName = "DisplayFilterLink";
    settingsActionData.m_margin = 5;
    settingsActionData.m_textAlignment = Qt::AlignRight | Qt::AlignVCenter;
    m_pDisplaySettingsAction = m_pTopToolbar->AddWidget(settingsActionData);
    GT_ASSERT(m_pDisplaySettingsAction != nullptr);

    acToolbarActionData modulesActionData;
    modulesActionData.m_text = CP_strDisplayFilterSelectModules;
    modulesActionData.m_actionType = acToolbarActionData::AC_LABEL_ACTION;
    modulesActionData.m_pSignal = SIGNAL(linkActivated(const QString&));
    modulesActionData.m_pReceiver = this;
    modulesActionData.m_pMember = SLOT(onOpenModuleSelector(const QString&));

    m_pTopToolbar->AddLabel(" | ");

    m_pLabelModuleSelectedAction = m_pTopToolbar->AddWidget(modulesActionData);
    GT_ASSERT(m_pLabelModuleSelectedAction != nullptr);

    m_pTopToolbar->AddLabel(CP_strProcess);
    m_pPIDComboBoxAction = m_pTopToolbar->AddComboBox(QStringList(), SIGNAL(currentIndexChanged(int)), this, SLOT(onSelectPid(int)));
}

void SessionFunctionView::selectFunction(const QString& funcId)
{
    GT_IF_WITH_ASSERT(m_pFunctionTable != nullptr)
    {
        int itemRowIndex = -1;

        for (int i = 0; i < m_pFunctionTable->rowCount(); i++)
        {
            QTableWidgetItem* pItem = m_pFunctionTable->item(i, 0);

            if (pItem != nullptr)
            {
                if (pItem->text() == funcId)
                {
                    itemRowIndex = i;
                    break;
                }
            }
        }

        // Select and ensure visible:
        GT_IF_WITH_ASSERT(itemRowIndex < m_pFunctionTable->rowCount())
        {
            m_pFunctionTable->ensureRowVisible(itemRowIndex, true);
            m_pFunctionTable->setFocus();
            setFocus();
            raise();
            activateWindow();
        }
    }
}

AMDTProcessId SessionFunctionView::getCurrentPid()
{
    AMDTProcessId pid = 0;

    GT_IF_WITH_ASSERT(nullptr != m_pPIDComboBoxAction)
    {
        const QComboBox* pPIDComboBox = TopToolbarComboBox(m_pPIDComboBoxAction);

        GT_IF_WITH_ASSERT(nullptr != pPIDComboBox)
        {
            QString processString = pPIDComboBox->currentText();

            int i = processString.lastIndexOf("(");

            if (-1 != i)
            {
                bool ok;
                pid = processString.mid(i + 1, processString.length() - i - 2).toUInt(&ok);

                if (!ok)
                {
                    pid = 0;
                }
            }
        }
    }

    return pid;
}

void SessionFunctionView::filterByPID(int pid)
{
    const QComboBox* pPIDComboBox = TopToolbarComboBox(m_pPIDComboBoxAction);
    GT_IF_WITH_ASSERT((m_pFunctionTable != nullptr) &&
                      (m_pPIDComboBoxAction != nullptr) &&
                      (pPIDComboBox != nullptr))
    {
        // bool found = false;
        int count = pPIDComboBox->count();

        for (int i = 0; i < count; ++i)
        {
            if (0 != pPIDComboBox->itemText(i).compare(CP_profileAllProcesses))
            {
                AMDTProcessId currendPID = 0;
                bool rc = ProcessNameToPID(pPIDComboBox->itemText(i), currendPID);

                if (rc && ((unsigned int)pid == currendPID))
                {
                    m_pPIDComboBoxAction->UpdateCurrentIndex(i);
                    break;
                }
            }
        }
    }
}

void SessionFunctionView::FilterByModuleFilePaths(AMDTModuleId moduleId)
{
    const QComboBox* pPIDComboBox = TopToolbarComboBox(m_pPIDComboBoxAction);
    GT_IF_WITH_ASSERT((m_pFunctionTable != nullptr) && (m_pPIDComboBoxAction != nullptr) && (pPIDComboBox != nullptr))
    {
        m_updateData = false;

        // Get the "All Processes" item index:
        int index = pPIDComboBox->findText(CP_profileAllProcesses);

        if (index < 0)
        {
            index = 0;
        }

        m_pPIDComboBoxAction->UpdateCurrentIndex(index);

        m_moduleId = moduleId;

        // Update the link string:
        updateModulesFilterLinkString();

        m_updateData = true;

        // Display the function data table:
        bool rc = displaySessionDataTables();
        GT_ASSERT(rc);
    }
}
void SessionFunctionView::onOpenModuleSelector(const QString& link)
{
    (void)(link); // unused

    CPUSessionTreeItemData* pSessionData = nullptr;
    GT_IF_WITH_ASSERT(nullptr != m_pDisplayedSessionItemData)
    {
        pSessionData = qobject_cast<CPUSessionTreeItemData*>(m_pDisplayedSessionItemData->extendedItemData());
    }

    ModuleFilterDialog mfd(m_pProfDataRdr, m_pDisplayFilter, &m_functionsTablesFilter, pSessionData, afMainAppWindow::instance());

    if (QDialog::Accepted == mfd.exec())
    {
        // Update the link string:
        updateModulesFilterLinkString();

        // Update the view with the new filter configuration:
        displaySessionDataTables();
    }
}

QString SessionFunctionView::updateModulesFilterLinkString()
{
    QString str;
    str = "<a href=""Open_Module_Selector""><u>";

    // Calculate how many modules are shown. The logic:
    // If m_filterByModulePathsList is empty - all the modules are shown.
    // If m_filterByModulePathsList is empty and system dll's are hidden - all the modules are shown minus the amount of system modules:
    // If m_filterByModulePathsList is not empty - the count is m_filterByModulePathsList size
    int amountOfModulesShown = m_functionsTablesFilter.m_filterByModulePathsList.size();

    if (amountOfModulesShown == 0)
    {
        amountOfModulesShown = m_functionsTablesFilter.m_allModulesFullPathsList.size();

        if (!CPUGlobalDisplayFilter::instance().m_displaySystemModules)
        {
            int systemModuleCount = 0;

            foreach (bool isSys, m_functionsTablesFilter.m_isSystemModuleList)
            {
                if (isSys)
                {
                    systemModuleCount ++;
                }
            }

            amountOfModulesShown -= systemModuleCount;
        }
    }

    // Calculate the hidden modules count:
    int amountOfModulesHidden = m_functionsTablesFilter.m_allModulesFullPathsList.size() - amountOfModulesShown;

    // Build the amount of modules shown / hidden string:
    if (amountOfModulesShown == 1)
    {
        str += "1 Modules Shown";
    }
    else
    {
        str += QString("%1 modules shown").arg(amountOfModulesShown);
    }

    str += ", ";

    if (amountOfModulesHidden == 1)
    {
        str += "1 Modules Hidden";
    }
    else
    {
        str += QString("%1 modules hidden").arg(amountOfModulesHidden);
    }

    str += "</u></a>";

    if (nullptr != m_pLabelModuleSelectedAction)
    {
        m_pLabelModuleSelectedAction->UpdateText(str);
    }

    return str;
}

void SessionFunctionView::fillPIDComb()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pPIDComboBoxAction != nullptr) && (m_pProfDataRdr != nullptr))
    {
        QStringList pidList;

        AMDTProfileProcessInfoVec procInfo;
        m_pProfDataRdr->GetProcessInfo(AMDT_PROFILE_ALL_PROCESSES, procInfo);

        // Check if this session is a multi-process session:
        bool isMultiProcess = procInfo.size() > 1;

        // If there are multiple processes, add "All Processes" item:
        if (isMultiProcess)
        {
            pidList << CP_profileAllProcesses;
        }

        for (const auto& proc : procInfo)
        {
            QString pidAsString = QString("%1(%2)").arg(acGTStringToQString(proc.m_name)).arg(proc.m_pid);
            pidList << pidAsString;
        }

        m_pPIDComboBoxAction->UpdateEnabled(isMultiProcess);
        m_pPIDComboBoxAction->UpdateStringList(pidList);

        // Select the "All Processes" item index:
        int index = pidList.indexOf(CP_profileAllProcesses);

        if (index < 0)
        {
            index = 0;
        }

        m_pPIDComboBoxAction->UpdateCurrentIndex(index);
    }
}

void SessionFunctionView::onSelectPid(int index)
{
    Q_UNUSED(index);

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pFunctionTable != nullptr)
    {
        m_functionsTablesFilter.m_filterByPIDsList.clear();
        m_functionsTablesFilter.m_filterByModulePathsList.clear();

        updateDataFromPidComboBox();

        if (m_updateData)
        {
            updateModulesFilterLinkString();
            // Update the table with the new displayed profile:
            bool rc = displaySessionDataTables();
            GT_ASSERT(rc);
        }
    }
}


void SessionFunctionView::updateDataFromPidComboBox()
{
    const QComboBox* pPIDComboBox = TopToolbarComboBox(m_pPIDComboBoxAction);
    GT_IF_WITH_ASSERT((m_pFunctionTable != nullptr) && (pPIDComboBox != nullptr))
    {
        // Clear all lists in the table settings:
        m_functionsTablesFilter.m_filterByPIDsList.clear();
        m_functionsTablesFilter.m_filterByModulePathsList.clear();
        m_functionsTablesFilter.m_allModulesFullPathsList.clear();
        m_functionsTablesFilter.m_isModule32BitList.clear();
        m_functionsTablesFilter.m_isSystemModuleList.clear();

        AMDTProcessId pid = 0;

        if (CP_profileAllProcesses == pPIDComboBox->currentText())
        {
            pid = AMDT_PROFILE_ALL_PROCESSES;
            int count = pPIDComboBox->count();

            for (int i = 0; i < count; ++i)
            {
                if (0 != pPIDComboBox->itemText(i).compare(CP_profileAllProcesses))
                {
                    AMDTProcessId currentPID = 0;
                    bool rc = ProcessNameToPID(pPIDComboBox->itemText(i), currentPID);

                    if (rc)
                    {
                        m_functionsTablesFilter.m_filterByPIDsList.append(currentPID);
                    }
                }
            }
        }
        else
        {
            bool ret = ProcessNameToPID(pPIDComboBox->currentText(), pid);

            if (true == ret)
            {
                m_functionsTablesFilter.m_filterByPIDsList.append(pid);
            }
        }

        gtVector<AMDTProfileModuleInfo> modInfo;
        bool ret = m_pProfDataRdr->GetModuleInfo(pid, AMDT_PROFILE_ALL_MODULES, modInfo);
        GT_ASSERT(ret);

        for (const auto& module : modInfo)
        {
            if (!m_functionsTablesFilter.m_allModulesFullPathsList.contains(acGTStringToQString(module.m_path)))
            {
                m_functionsTablesFilter.m_allModulesFullPathsList.append(acGTStringToQString(module.m_path));
                m_functionsTablesFilter.m_isModule32BitList.append(module.m_is64Bit ? false : true);
                m_functionsTablesFilter.m_isSystemModuleList.append(module.m_isSystemModule);
                m_moduleNameIdMap.insert(std::make_pair(module.m_path, module.m_moduleId));
            }
        }
    }
}

void SessionFunctionView::UpdateTableDisplay(unsigned int updateType)
{
    GT_UNREFERENCED_PARAMETER(updateType);

    bool displaySystemModules = !m_pDisplayFilter->IsSystemModuleIgnored();

    // This function is called after the session display filter is changed:
    if (m_functionsTablesFilter.m_shouldDisplaySystemModuleInModulesDlg != displaySystemModules)
    {
        m_functionsTablesFilter.m_shouldDisplaySystemModuleInModulesDlg = displaySystemModules;

        if (m_functionsTablesFilter.m_filterByModulePathsList.size() > 0)
        {
            // Go through the modules and add the system modules:
            for (int i = 0 ; i < m_functionsTablesFilter.m_allModulesFullPathsList.size(); i++)
            {
                // Get the module for the current index:
                QString moduleFilePath = m_functionsTablesFilter.m_allModulesFullPathsList.at(i);

                if (m_functionsTablesFilter.m_isSystemModuleList[i])
                {
                    // Check if the modules should be added / remove from the filter list:
                    bool shouldAdd = m_functionsTablesFilter.m_shouldDisplaySystemModuleInModulesDlg &&
                                     !m_functionsTablesFilter.m_filterByModulePathsList.contains(moduleFilePath);
                    bool shouldRemove = m_functionsTablesFilter.m_shouldDisplaySystemModuleInModulesDlg &&
                                        m_functionsTablesFilter.m_filterByModulePathsList.contains(moduleFilePath);

                    if (shouldAdd)
                    {
                        // Add this modules to the filters list, since it is a system module, and the user requested to add these modules:
                        m_functionsTablesFilter.m_filterByModulePathsList << moduleFilePath;
                    }

                    if (shouldRemove)
                    {
                        // Add this modules to the filters list, since it is a system module, and the user requested to add these modules:
                        int index = m_functionsTablesFilter.m_filterByModulePathsList.indexOf(moduleFilePath);

                        if (index >= 0)
                        {
                            m_functionsTablesFilter.m_filterByModulePathsList.removeAt(index);
                        }
                    }
                }
            }
        }
    }

    // Update the filtered modules:
    updateModulesFilterLinkString();

    // Update the view with the new filter configuration:
    displaySessionDataTables();
}

void SessionFunctionView::onTableItemActivated(QTableWidgetItem* pActivateItem)
{
    GT_IF_WITH_ASSERT(pActivateItem != nullptr)
    {
        const QComboBox* pPIDComboBox = TopToolbarComboBox(m_pPIDComboBoxAction);
        GT_IF_WITH_ASSERT((m_pFunctionTable != nullptr) && (pPIDComboBox != nullptr))
        {
            // don't make action on empty table message row
            if (pActivateItem->row() != m_pFunctionTable->GetEmptyTableItemRowNum())
            {
                gtString functionName = acQStringToGTString(m_pFunctionTable->getFunctionName(pActivateItem->row()));

                if (!functionName.startsWith(L"Unknown Module"))
                {
                    // Get the address for the activated function:
                    QString funcIdStr = m_pFunctionTable->getFunctionId(pActivateItem->row());
                    AMDTUInt32 funcId = funcIdStr.toInt();

                    // Get the module info
                    AMDTProfileModuleInfo modInfo;
                    bool ret = m_pProfDataRdr->GetModuleInfoForFunction(funcId, modInfo);
                    AMDTModuleId moduleId = (ret) ? modInfo.m_moduleId : AMDT_PROFILE_ALL_MODULES;
                    AMDTUInt32  processId = AMDT_PROFILE_ALL_PROCESSES;

                    gtString processIdStr = acQStringToGTString(pPIDComboBox->currentText());

                    if (processIdStr != L"All Processes")
                    {
                        // Extract pid from processIdStr format: process_name(PID)
                        int idxStart = processIdStr.reverseFind(L"(");
                        int idxEnd = processIdStr.length() - 1;

                        if (idxStart >= 0 && idxEnd > 1)
                        {
                            processIdStr.truncate(idxStart + 1, idxEnd - 1);
                            processIdStr.toUnsignedIntNumber(processId);
                        }
                    }

                    auto funcModInfo = std::make_tuple(funcId, modInfo.m_path, moduleId, processId);
                    // Emit a function activated signal (will open a source code view):
                    emit opensourceCodeViewSig(funcModInfo);
                }
            }
        }
    }
}

void SessionFunctionView::onTableContextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType actionType, QTableWidgetItem* pTableItem)
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

    GT_IF_WITH_ASSERT(pTableItem != nullptr)
    {
        if (actionType == CPUProfileDataTable::DISPLAY_FUNCTION_IN_SOURCE_CODE_VIEW)
        {
            onTableItemActivated(pTableItem);
        }
        else if (actionType == CPUProfileDataTable::DISPLAY_FUNCTION_IN_CALLGRAPH_VIEW)
        {
            AMDTProcessId pid = getCurrentPid();
            openCallGraphViewForFunction(m_pFunctionTable->getFunctionId(pTableItem->row()).toInt(), pid);
        }
    }
}


void SessionFunctionView::onOpenDisplayFilterDialog()
{
    OnDisplaySettingsClicked();

    m_functionsTablesFilter.m_shouldDisplaySystemModuleInModulesDlg = !m_pDisplayFilter->IsSystemModuleIgnored();
}

void SessionFunctionView::displayModule(const QString& moduleFullPath)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pFunctionTable != nullptr)
    {
        // Set this modules as filter for the functions table:
        m_functionsTablesFilter.m_filterByPIDsList.clear();

        updateDataFromPidComboBox();

        m_functionsTablesFilter.m_filterByModulePathsList.clear();
        m_functionsTablesFilter.m_filterByModulePathsList << moduleFullPath;

        // Update the link string:
        updateModulesFilterLinkString();

        // Update the view with the new filter configuration:
        displaySessionDataTables();
    }
}

void SessionFunctionView::onCellChanged()
{
    //unused
}

void SessionFunctionView::onEditCopy()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pFunctionTable != nullptr)
    {
        m_pFunctionTable->onEditCopy();
    }
}

void SessionFunctionView::onEditSelectAll()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pFunctionTable != nullptr)
    {
        m_pFunctionTable->onEditSelectAll();
    }
}

void SessionFunctionView::onFindClick()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pFunctionTable != nullptr)
    {
        m_pFunctionTable->onFindClick();
    }
}

void SessionFunctionView::onFindNext()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pFunctionTable != nullptr)
    {
        m_pFunctionTable->onFindNext();
    }
}

