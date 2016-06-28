//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionSourceCodeView.cpp
/// \brief Implementation of the SessionSourceCodeView class.
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/SessionSourceCodeView.cpp#69 $
// Last checkin:   $DateTime: 2013/07/15 10:33:50 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 474652 $
//=============================================================

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>
#include <QPushButton>

// Infra:
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTBaseTools/Include/gtHashMap.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTApplicationComponents/Include/acToolBar.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>

#include <AMDTCpuPerfEventUtils/inc/IbsEvents.h>

// Local:
#include <inc/CPUProfileUtils.h>
#include <inc/CpuProjectHandler.h>
#include <inc/CpuProfilingOptions.h>
#include <inc/SessionSourceCodeView.h>
#include <inc/SourceCodeTreeModel.h>
#include <inc/StringConstants.h>
#include <inc/SessionWindow.h>
#include <inc/Auxil.h>
#include <inc/CpuProfileTreeHandler.h>

#if defined (_WIN32)
    #pragma warning( push )
    #pragma warning( disable : 4091)
    #pragma warning( disable : 4458)
    #include <corhdr.h>
    #include <corprof.h>
    #pragma warning( pop )
#endif

#define FUNCTIONS_COMBO_MAX_LEN 75

SessionSourceCodeView::SessionSourceCodeView(QWidget* pParent, CpuSessionWindow* pSessionWindow, const QString& sessionDir) :
    DataTab(pParent, pSessionWindow, sessionDir),
    m_pWidget(nullptr),
    m_pMainVBoxLayout(nullptr),
    m_pModuleLocationInfoLabel(nullptr),
    m_pExpandAllAction(nullptr),
    m_pCollapseAllAction(nullptr),
    m_pFunctionsComboBoxAction(nullptr),
    m_pPIDComboBoxAction(nullptr),
    m_pTIDComboBoxAction(nullptr),
    m_pHotSpotIndicatorComboBoxAction(nullptr),
    m_pPIDLabelAction(nullptr),
    m_pTIDLabelAction(nullptr),
    m_pShowCodeBytesAction(nullptr),
    m_pShowAddressAction(nullptr),
    m_pSourceCodeTree(nullptr),
    m_pTreeViewModel(nullptr),
    m_pTreeItemDelegate(nullptr),
    m_ignoreVerticalScroll(false)
{

    m_exportString = CP_sourceCodeViewExportString;
    m_pShowNoteAction = nullptr;
    m_CLUNoteShown = false;
    // Create the tree model object:
    m_pTreeViewModel = new SourceCodeTreeModel(m_pSessionDisplaySettings, m_sessionDir, m_pProfileReader, m_pProfDataRdr, m_pDisplayFilter);

}


SessionSourceCodeView::~SessionSourceCodeView()
{
    if (m_pTreeViewModel != nullptr)
    {
        delete m_pTreeViewModel;
        m_pTreeViewModel = nullptr;
    }
}

bool SessionSourceCodeView::DisplayViewModule(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32> funcModInfo)
{
    bool ret = true;

    m_moduleId      = std::get<2>(funcModInfo);
    m_functionId    = std::get<0>(funcModInfo);;
    m_processId     = std::get<3>(funcModInfo);

    //store module details
    m_pTreeViewModel->SetModuleDetails(m_moduleId, m_processId);

    // Create the central widget:
    m_pWidget                   = new QWidget(this);
    m_pMainVBoxLayout           = new QVBoxLayout(m_pWidget);

    // Setup Module and File location info Label
    m_pModuleLocationInfoLabel  = new QLabel("", m_pWidget);
    m_pMainVBoxLayout->addWidget(m_pModuleLocationInfoLabel);

    setCentralWidget(m_pWidget);

    // Create the symbols information:
	//TODO : required ??
    m_pTreeViewModel->CreateSymbolInfoList(m_moduleId, m_processId);

    // Create the top layout:
    CreateTopLayout();

    // Create the view layout:
    CreateViewLayout();

    // Create the view's context menu:
    ExtendTreeContextMenu();

    // Add a source code item to the tree:
    //AddSourceCodeItemToExplorer();

    // Update display filter string:
    updateDisplaySettingsString();

    // Check if the module is cached:
    CacheFileMap cache;
    ReadSessionCacheFileMap(m_pTreeViewModel->m_sessionDir, cache);
    QString fileName = m_pTreeViewModel->m_moduleName;
    fileName.remove(QChar('\0'));
    m_pTreeViewModel->m_isModuleCached = cache.contains(fileName);

    return ret;
}

// This function assumes
// - All the rows (source lines, dasm lines) have been filled.
// - Samples are already attributed to the line items.
// It will just populate samples into the data column
void SessionSourceCodeView::UpdateTableDisplay(unsigned int updateType)
{
    (void)(updateType); // unused

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSourceCodeTree != nullptr) && (m_pTreeViewModel != nullptr))
    {
        qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

        // Get the current precision:
        m_precision = afGlobalVariablesManager::instance().floatingPointPrecision();

#if 0
        // Restore the currently selected address if not new address is requested:
        if (0 == m_pTreeViewModel->m_newAddress)
        {
            QModelIndex selectedItemIndex = m_pSourceCodeTree->selectionModel()->currentIndex();
            QModelIndex sourceCodeIndex = m_pTreeViewModel->index(selectedItemIndex.row(), SOURCE_VIEW_ADDRESS_COLUMN);
            m_pTreeViewModel->m_newAddress = m_pTreeViewModel->data(sourceCodeIndex, Qt::DisplayRole).toString().toULongLong(nullptr, 16);
        }

        // If the currentItem has its address field empty, then focus to the beginning of the function:
        if (0 == m_pTreeViewModel->m_newAddress && m_pTreeViewModel->m_currentSymbolIterator != m_pTreeViewModel->m_symbolsInfoList.end())
        {
            m_pTreeViewModel->m_newAddress = m_pTreeViewModel->m_currentSymbolIterator->m_va;
        }

#endif
        // Clear the tree and table items:
        m_pTreeViewModel->removeRows(0, m_pTreeViewModel->rowCount());
        m_pTreeViewModel->m_sourceTreeItemsMap.clear();
        m_pTreeViewModel->m_sourceLineToTreeItemsMap.clear();
        m_pTreeViewModel->m_sourceLinesToDataMap.clear();
        m_pTreeViewModel->m_sourceLineToCodeBytesMap.clear();

        // Update the display with the current displayed source:
        UpdateDisplay();

        // Update the model headers:
        bool rc = m_pTreeViewModel->UpdateHeaders();
        GT_IF_WITH_ASSERT(rc)
        {
            // Populate data for the displayed function:
            const QComboBox* pHotspotCombo = TopToolbarComboBox(m_pHotSpotIndicatorComboBoxAction);
            GT_IF_WITH_ASSERT(pHotspotCombo != nullptr)
            {
                // Set the current index if it is not set:
                int index = pHotspotCombo->currentIndex();

                if (index < 0)
                {
                    m_pHotSpotIndicatorComboBoxAction->UpdateCurrentIndex(0);
                }

                m_pTreeViewModel->PopulateCurrentFunction(pHotspotCombo->currentText());
            }

            if (!m_pTreeViewModel->m_isDisplayingOnlyDasm)
            {
                m_pSourceCodeTree->showColumn(SOURCE_VIEW_LINE_COLUMN);
            }
            else
            {
                m_pSourceCodeTree->hideColumn(SOURCE_VIEW_LINE_COLUMN);
            }

            // Disable expand / collapse all actions when only dasm is displayed:
            GT_IF_WITH_ASSERT((m_pExpandAllAction != nullptr) && (m_pCollapseAllAction != nullptr))
            {
                m_pExpandAllAction->setEnabled(!m_pTreeViewModel->m_isDisplayingOnlyDasm);
                m_pCollapseAllAction->setEnabled(!m_pTreeViewModel->m_isDisplayingOnlyDasm);
            }

            // Hide the filtered columns:
            HideFilteredColumns();

            GT_IF_WITH_ASSERT(m_pSessionDisplaySettings != nullptr)
            {
                if (m_pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU)
                {
                    // Hide column "% of hotspot samples for CLU":
                    m_pSourceCodeTree->hideColumn(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN);
                }
            }

            // Set the table percent values if needed:
            //m_pTreeViewModel->SetDataPercentValues();

            m_pSourceCodeTree->FixColumnSizes();

            // Refresh the view:
            RefreshView();

        }
        qApp->restoreOverrideCursor();
    }
}

void SessionSourceCodeView::HideFilteredColumns()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDisplaySettings != nullptr)
    {
        // Iterate each of the columns and hide the filtered ones:
        int colsAmount = m_pTreeViewModel->columnCount();

        for (int col = 0; col < colsAmount; ++col)
        {
            QString headerText = m_pTreeViewModel->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString();
            bool shouldShow = !m_pSessionDisplaySettings->m_filteredDataColumnsCaptions.contains(headerText);

            if (shouldShow)
            {
                m_pSourceCodeTree->header()->showSection(col);
            }
            else
            {
                m_pSourceCodeTree->header()->hideSection(col);
            }
        }
    }
}


bool SessionSourceCodeView::CreateViewLayout()
{
    // Create the tree control:
    m_pSourceCodeTree = new SourceCodeTreeView(this, m_pTreeViewModel, SOURCE_VIEW_CODE_BYTES_COLUMN);


    m_pTreeViewModel->m_pSessionSourceCodeTreeView = m_pSourceCodeTree;

    // Align both lists line widths:
    m_pSourceCodeTree->setUniformRowHeights(true);

    m_pSourceCodeTree->setRootIsDecorated(true);

    m_pSourceCodeTree->setAllColumnsShowFocus(true);

    bool rc = connect(m_pSourceCodeTree, SIGNAL(ItemDoubleClicked(const QModelIndex&)), SLOT(OnTreeItemDoubleClick(const QModelIndex&)));
    GT_ASSERT(rc);

    rc = connect(m_pSourceCodeTree, SIGNAL(ItemClicked(const QModelIndex&)), SLOT(OnTreeItemClick(const QModelIndex&)));
    GT_ASSERT(rc);

    rc = connect(m_pSourceCodeTree, SIGNAL(VerticalScrollPositionChanged(int)), SLOT(OnTreeVerticalScrollPositionChange(int)));
    GT_ASSERT(rc);

    rc = connect(m_pSourceCodeTree, SIGNAL(ItemExpanded(const QModelIndex&)), SLOT(OnItemExpanded(const QModelIndex&)));
    GT_ASSERT(rc);

    rc = connect(m_pSourceCodeTree, SIGNAL(ItemCollapsed(const QModelIndex&)), SLOT(OnItemCollapsed(const QModelIndex&)));
    GT_ASSERT(rc);

    rc = connect(m_pSourceCodeTree->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnItemSelectChanged(const QModelIndex&, const QModelIndex&)));
    GT_ASSERT(rc);

    m_pSourceCodeTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pSourceCodeTree->setUniformRowHeights(false);

    m_pMainVBoxLayout->addWidget(m_pSourceCodeTree, 1);

    // Create CLU notes frame for CLU sessions:
    createCLUNotesFrame(m_pMainVBoxLayout);

    // Create the bottom information frame:
    QFrame* pHintFrame = createHintLabelFrame();
    GT_IF_WITH_ASSERT(pHintFrame != nullptr)
    {
        m_pMainVBoxLayout->addWidget(pHintFrame);
        updateHint(CP_sourceCodeViewInformationHint);

        if (m_isProfiledClu)
        {
            updateHint(CP_sourceCodeViewInformationHintForCLU);
        }
    }

    // Set the tree items delegate:
    SetItemsDelegate();

    return true;
}


void SessionSourceCodeView::ExtendTreeContextMenu()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSourceCodeTree != nullptr)
    {
        QMenu* pContextMenu = m_pSourceCodeTree->ContextMenu();
        GT_IF_WITH_ASSERT(pContextMenu != nullptr)
        {
            pContextMenu->addSeparator();
            m_pExpandAllAction = pContextMenu->addAction(CP_sourceCodeExpandAll, this, SLOT(OnExpandAll()));
            m_pCollapseAllAction = pContextMenu->addAction(CP_sourceCodeCollapseAll, this, SLOT(OnCollapseAll()));
            pContextMenu->addSeparator();
            m_pShowCodeBytesAction = pContextMenu->addAction(CP_sourceCodeShowCodeBytes, this, SLOT(OnShowCodeBytes()));
            GT_IF_WITH_ASSERT(m_pShowCodeBytesAction != nullptr)
            {
                m_pShowCodeBytesAction->setCheckable(true);
                m_pShowCodeBytesAction->setChecked(true);
            }
            m_pShowAddressAction = pContextMenu->addAction(CP_sourceCodeShowAddress, this, SLOT(OnShowAddress()));
            GT_IF_WITH_ASSERT(m_pShowAddressAction != nullptr)
            {
                m_pShowAddressAction->setCheckable(true);
                m_pShowAddressAction->setChecked(true);
            }

            if (m_isProfiledClu)
            {
                m_pShowNoteAction = pContextMenu->addAction(CP_sourceCodeHideCluNotes, this, SLOT(OnShowNoteWindow()));
                m_CLUNoteShown = true;
            }
        }
    }
}

void SessionSourceCodeView::CreatePidTidComboBoxes()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTopToolbar != nullptr)
    {
        // Setup the pid combobox:
        // Create the display filter link:
        acToolbarActionData actionData;
        actionData.m_actionType = acToolbarActionData::AC_LABEL_ACTION;
        actionData.m_margin = 5;
        actionData.m_text = CP_sourceCodeViewProcessPrefix;
        m_pPIDLabelAction = m_pTopToolbar->AddWidget(actionData);

        m_pPIDComboBoxAction = m_pTopToolbar->AddComboBox(QStringList(), SIGNAL(currentIndexChanged(int)), this, SLOT(OnPIDComboChange(int)));
        GT_ASSERT(m_pPIDComboBoxAction != nullptr);

        // Setup the tid comboBox:
        m_pTIDLabelAction = m_pTopToolbar->AddLabel(CP_sourceCodeViewTIDPrefix);


        m_pTIDComboBoxAction = m_pTopToolbar->AddComboBox(QStringList(), SIGNAL(currentIndexChanged(int)), this, SLOT(OnTIDComboChange(int)));
        GT_ASSERT(m_pTIDComboBoxAction != nullptr);
    }
}

void SessionSourceCodeView::CreateDisplayFilterLinkLabel()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTopToolbar != nullptr)
    {
        QString caption = CP_strCPUProfileToolbarBase;
        caption.append(":");
        acWidgetAction* pTestAction = m_pTopToolbar->AddLabel(caption);

        int width = 25;
        const QLabel* pTestLabel = TopToolbarLabel(pTestAction);
        GT_IF_WITH_ASSERT(pTestLabel != nullptr)
        {
            static QString longestDisplayFilter = "Hide System Modules";
            width = pTestLabel->fontMetrics().boundingRect(longestDisplayFilter).width();
        }

        // Create the display filter link:
        acToolbarActionData actionData(SIGNAL(linkActivated(const QString&)), this, SLOT(OnDisplaySettingsClicked()));
        actionData.m_actionType = acToolbarActionData::AC_LABEL_ACTION;
        actionData.m_margin = 5;
        actionData.m_minWidth = width;

        m_pDisplaySettingsAction = m_pTopToolbar->AddWidget(actionData);

    }
}

bool SessionSourceCodeView::FillHotspotIndicatorCombo()
{
    bool retVal = false;

    const QComboBox* pHotspotCombo = TopToolbarComboBox(m_pHotSpotIndicatorComboBoxAction);
    GT_IF_WITH_ASSERT((pHotspotCombo != nullptr) &&
                      (m_pDisplayedSessionItemData != nullptr) &&
                      (m_pDisplayFilter != nullptr) &&
                      (m_pHotSpotIndicatorComboBoxAction != nullptr))
    {
        // Find the profile type:
        SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(m_pDisplayedSessionItemData->extendedItemData());
        m_supportedCounterList.clear();
        GT_IF_WITH_ASSERT(pSessionData != nullptr)
        {
            QStringList hotSpotColumns;

            QString filterName = m_pDisplayFilter->GetCurrentCofigName();
            GT_IF_WITH_ASSERT(!filterName.isEmpty())
            {
                CounterNameIdVec countersName;
				m_pDisplayFilter->GetSelectedCounterList(countersName);
                //m_pDisplayFilter->GetConfigCounters(filterName, countersName);

                for (const auto& name : countersName)
                {
					gtString counterName = std::get<0>(name);
                    hotSpotColumns << acGTStringToQString(counterName);
                    m_supportedCounterList.push_back(counterName);
                }

                retVal = true;
            }

            // Add the hot spot columns to the list:
            m_pHotSpotIndicatorComboBoxAction->UpdateStringList(hotSpotColumns);
            m_pHotSpotIndicatorComboBoxAction->UpdateEnabled(hotSpotColumns.size() > 1);
        }

    }
    return retVal;
}

void SessionSourceCodeView::CreateFunctionsComboBox()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTopToolbar != nullptr)
    {
        // Prepare the string list for the functions combo box:
        QStringList functionsList, toolTipList;

        AMDTProfileDataVec funcProfileData;
        m_pProfDataRdr->GetFunctionProfileData(AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_MODULES, funcProfileData);

        AMDTProfileFunctionData  functionData;

        for (auto const& func : funcProfileData)
        {
            m_pProfDataRdr->GetFunctionDetailedProfileData(func.m_id,
                                                           AMDT_PROFILE_ALL_PROCESSES,
                                                           AMDT_PROFILE_ALL_THREADS,
                                                           functionData);

            gtUInt64 baseAddr = functionData.m_modBaseAddress;
            gtUInt64 startOffset = baseAddr + functionData.m_functionInfo.m_startOffset;
            gtUInt64 endOffset = startOffset + functionData.m_functionInfo.m_size;

            QString addressStart = "0x" + QString::number(startOffset, 16);
            QString addressEnd = "0x" + QString::number(endOffset, 16);
            QString functionStr = QString("[%1 - %2] : ").arg(addressStart).arg(addressEnd);
            functionStr += acGTStringToQString(functionData.m_functionInfo.m_name);
            functionsList << functionStr;
            m_functionIdVec.push_back(functionData.m_functionInfo.m_functionId);
        }

        m_pTopToolbar->AddLabel(CP_sourceCodeViewFunctionPrefix);

        for (QStringList::iterator strIt = functionsList.begin(), endIt = functionsList.end();
             strIt != endIt; ++strIt)
        {
            QString& str = *strIt;

            toolTipList << str;

            // if combo string are too long - limit the length
            if (str.length() > FUNCTIONS_COMBO_MAX_LEN)
            {
                str.resize(FUNCTIONS_COMBO_MAX_LEN);
            }
        }

        // Functions Combo box:
        m_pFunctionsComboBoxAction = m_pTopToolbar->AddComboBox(functionsList,
                                                                toolTipList,
                                                                SIGNAL(currentIndexChanged(int)),
                                                                this,
                                                                SLOT(OnFunctionsComboChange(int)));

    }
}

void SessionSourceCodeView::CreateTopLayout()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pMainVBoxLayout != nullptr)
    {
        m_pTopToolbar = new acToolBar(nullptr);

        // Do not allow the toolbar to float:
        m_pTopToolbar->setFloatable(false);
        m_pTopToolbar->setMovable(false);
        m_pTopToolbar->setStyleSheet("QToolBar { border-style: none; }");

        // Create and fill the functions combo box:
        CreateFunctionsComboBox();

        // Create the display filter link:
        CreateDisplayFilterLinkLabel();

        // Add an empty spacer between the left and right contorls of the top layout, to enable the
        // right controls to aligned right and the left ones to the left:
        QWidget* emptySpacer = new QWidget;
        emptySpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        m_pTopToolbar->addWidget(emptySpacer);

        // Create and fill the PID & TID combo boxes:
        CreatePidTidComboBoxes();

        // Create the hot spot indicator combo box:
        CreateHotSpotIndicatorComboBox();

        m_pMainVBoxLayout->addWidget(m_pTopToolbar);
    }
}

void SessionSourceCodeView::OnItemSelectChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous);

    if (m_isProfiledClu)
    {
        SourceViewTreeItem* pSrcItem = m_pTreeViewModel->getItem(current);
        gtVector<float> cluData;
        gtMap<int, int> idxList;
        int cluEndOffset = IBS_CLU_OFFSET(IBS_CLU_END);
        GT_IF_WITH_ASSERT(m_pSessionDisplaySettings != nullptr)
        {
            for (int cpuId = 0; cpuId < m_pSessionDisplaySettings->m_cpuCount; cpuId++)
            {
                if ((m_pSessionDisplaySettings->m_separateBy == SEPARATE_BY_NONE) && (cpuId > 0))
                {
                    // If samples are not separated by core/numa, no need to loop for each cpu
                    break;
                }

                for (int i = 0; i <= cluEndOffset; i++)
                {
                    EventMaskType evCluPercent = EncodeEvent((IBS_CLU_BASE + i), 0, true, true);
                    SampleKeyType key;
                    key.event = evCluPercent;
                    key.cpu = cpuId;
                    CpuEventViewIndexMap::const_iterator idxIt = m_pSessionDisplaySettings->m_eventToIndexMap.find(key);

                    int idx = *idxIt;

                    if (idxList.find(idx) == idxList.end())
                    {
                        // The check is to avoid duplicate data in case the samples are separated by numa
                        idxList.insert(std::pair<char, int>(idx, 0));

                        float dataAsVariant = pSrcItem->data(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + 1 + idx).toFloat();
                        cluData.push_back(dataAsVariant);
                    }
                }
            }

            UpdateNoteWindowContent(cluData);
        }
    }
}

void SessionSourceCodeView::OnFetchInstrucionsRequest(SourceViewTreeItem* pSrcItem)
{
    if ((nullptr != pSrcItem) && m_pTreeViewModel->m_isDisplayingOnlyDasm)
    {
        bool isItemDoubleClickable = true;

        // Only the top/bottom most lines are double-clickable:
        int srcItemIndex = m_pTreeViewModel->indexOfTopLevelItem(pSrcItem);
        int topCount = m_pTreeViewModel->topLevelItemCount();

        if (!m_userDisplayInformation.isAtBottom() && (srcItemIndex != 0) && (srcItemIndex != (topCount - 1)))
        {
            isItemDoubleClickable = false;
        }

        // Check if this row is already filled with dasm code.
        else if (!pSrcItem->data(SOURCE_VIEW_ADDRESS_COLUMN).toString().isEmpty())
        {
            isItemDoubleClickable = false;
        }

        if (isItemDoubleClickable)
        {
            qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

            gtVAddr displayAddress = 0;

            bool itemFound = false;

            if (srcItemIndex == 0)
            {
                SourceViewTreeItem* pTopAddressItem = (SourceViewTreeItem*)m_pTreeViewModel->topLevelItem(srcItemIndex + 1);

                if (nullptr != pTopAddressItem)
                {
                    QString addressText = pTopAddressItem->data(SOURCE_VIEW_ADDRESS_COLUMN).toString();
                    displayAddress = addressText.toULongLong(nullptr, 0);

                    // Insert disassembly lines:
                    m_pTreeViewModel->InsertDasmLines(displayAddress, srcItemIndex + 1);

                    itemFound = true;
                }
            }
            else
            {
                SourceViewTreeItem* pLastAddressItem =
                    (SourceViewTreeItem*)m_pTreeViewModel->topLevelItem(srcItemIndex - 1);

                if (nullptr != pLastAddressItem)
                {
                    QString addressText = pLastAddressItem->data(SOURCE_VIEW_ADDRESS_COLUMN).toString();
                    displayAddress = addressText.toULongLong(nullptr, 0) + pLastAddressItem->asmLength();

                    // Insert disassembly lines:
                    m_pTreeViewModel->InsertDasmLines(displayAddress, srcItemIndex);

                    itemFound = true;
                }
            }

            if (itemFound)
            {
                const QComboBox* pHotSpotCombo = TopToolbarComboBox(m_pHotSpotIndicatorComboBoxAction);
                GT_IF_WITH_ASSERT(pHotSpotCombo != nullptr)
                {
                    // Populate Data
                    m_pTreeViewModel->PopulateCurrentFunction(pHotSpotCombo->currentText());
                }

                // Refresh the view:
                RefreshView();
            }

            qApp->restoreOverrideCursor();
        }
    }
}

void SessionSourceCodeView::OnTreeItemDoubleClick(const QModelIndex& index)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pTreeViewModel != nullptr) && (m_pHotSpotIndicatorComboBoxAction != nullptr))
    {
        // Get the item for this index:
        SourceViewTreeItem* pSrcItem = m_pTreeViewModel->getItem(index);
        OnFetchInstrucionsRequest(pSrcItem);
    }
}

void SessionSourceCodeView::OnTreeItemClick(const QModelIndex& index)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTreeViewModel != nullptr)
    {
        // Extract the top level item index + the top level item child index for the current selection:
        bool rc = m_pTreeViewModel->GetItemTopLevelIndex(index, 
			m_userDisplayInformation.m_selectedTopLevelItemIndex, 
			m_userDisplayInformation.m_selectedTopLevelChildItemIndex);
        GT_ASSERT(rc);
    }
}

// The isFromKeyBoard variable indicates whether it was an actual scroll or one triggered by a
// keyboard key press.
void SessionSourceCodeView::OnTreeVerticalScrollPositionChange(int value, bool isFromKeyboard/*=false*/)
{
    if (!m_ignoreVerticalScroll)
    {
        // Check if we are scrolling down.
        bool isDownScroll = (value > 1) && (value >= m_userDisplayInformation.m_verticalScrollPosition);

        m_userDisplayInformation.m_verticalScrollPosition = value;

        // Update the maximum.
        if (m_userDisplayInformation.m_verticalScrollPosition > m_userDisplayInformation.m_maxVerticalScrollPosition)
        {
            m_userDisplayInformation.m_maxVerticalScrollPosition = m_userDisplayInformation.m_verticalScrollPosition;
        }

        // Check if we need to fetch additional rows.
        if (isDownScroll && m_pSourceCodeTree != nullptr)
        {
            QScrollBar* pScrollBar = m_pSourceCodeTree->verticalScrollBar();

            if (pScrollBar != nullptr)
            {
                int max = pScrollBar->maximum();

                if ((!isFromKeyboard  && value == max) || (isFromKeyboard && value == (max - 1)))
                {
                    // Extract the index of the last item.
                    int lastItemIndex = m_pTreeViewModel->topLevelItemCount() - 1;
                    SourceViewTreeItem* pItem = m_pTreeViewModel->topLevelItem(lastItemIndex);

                    // First, set the display settings so that we will get back at where we were.
                    m_userDisplayInformation.m_selectedTopLevelItemIndex = lastItemIndex;

                    // Fetch additional instructions.
                    OnFetchInstrucionsRequest(pItem);
                }
            }
        }
    }
}

void SessionSourceCodeView::OnItemExpanded(const QModelIndex& index)
{
    GT_IF_WITH_ASSERT(m_pTreeViewModel != nullptr)
    {
        // Extract the top level item index + the top level item child index for the current selection:
        int topLevelItemIndex = -1;
        int topLevelChildItemIndex = -1;
        bool rc = m_pTreeViewModel->GetItemTopLevelIndex(index, topLevelItemIndex, topLevelChildItemIndex);
        GT_IF_WITH_ASSERT(rc)
        {
            if (!m_userDisplayInformation.m_expandedTreeItems.contains(topLevelItemIndex))
            {
                m_userDisplayInformation.m_expandedTreeItems << topLevelItemIndex;
            }
        }
    }
}

void SessionSourceCodeView::OnItemCollapsed(const QModelIndex& index)
{
    // Extract the top level item index + the top level item child index for the current selection:
    int topLevelItemIndex = -1;
    int topLevelChildItemIndex = -1;
    bool rc = m_pTreeViewModel->GetItemTopLevelIndex(index, topLevelItemIndex, topLevelChildItemIndex);
    GT_IF_WITH_ASSERT(rc)
    {
        if (m_userDisplayInformation.m_expandedTreeItems.contains(topLevelItemIndex))
        {
            m_userDisplayInformation.m_expandedTreeItems.removeAll(topLevelItemIndex);
        }
    }
}

bool SessionSourceCodeView::IsAddressInCurrentFunction(gtVAddr addr)
{
    bool retVal = false;

    if (m_pTreeViewModel->m_currentSymbolIterator != m_pTreeViewModel->m_symbolsInfoList.end())
    {
        if (addr == m_pTreeViewModel->m_currentSymbolIterator->m_va)
        {
            retVal = true;
        }
        else
        {
            // We assume the m_symInfoList is sorted by the symbolRVA.
            // Here, we check if the address is within the symbol
            auto rStartItr = m_pTreeViewModel->m_symbolsInfoList.rbegin();
            auto eRndItr = m_pTreeViewModel->m_symbolsInfoList.rbegin();

            for (auto itr = rStartItr; itr != eRndItr; ++itr)
            {
                if (addr >= itr->m_va)
                {
                    retVal = (*m_pTreeViewModel->m_currentSymbolIterator == *itr);
                    break;
                }
            }
        }
    }

    return retVal;
}

void SessionSourceCodeView::DisplayAddress(gtVAddr addr, ProcessIdType pid, ThreadIdType tid)
{
    GT_IF_WITH_ASSERT(m_pFunctionsComboBoxAction != nullptr)
    {
        m_pTreeViewModel->m_newPid = pid;
        m_pTreeViewModel->m_newTid = tid;
        m_pTreeViewModel->m_newAddress = addr;

        const QComboBox* pFunctionsComboBox = TopToolbarComboBox(m_pFunctionsComboBoxAction);

        if ((pFunctionsComboBox != nullptr) && (pFunctionsComboBox->count() > 0))
        {
            m_pFunctionsComboBoxAction->UpdateCurrentIndex(-1);
            m_pFunctionsComboBoxAction->UpdateCurrentIndex(0);
        }
    }
}


void SessionSourceCodeView::OnFunctionsComboChange(int functionIndex)
{
    // When the function is changed, clear the user selection information:
    m_userDisplayInformation.clear();


    GT_IF_WITH_ASSERT((m_pTreeViewModel != nullptr) &&
                      (m_pTIDComboBoxAction != nullptr) &&
                      (m_pPIDComboBoxAction != nullptr) &&
                      (m_pPIDLabelAction != nullptr) &&
                      (m_pTIDLabelAction != nullptr))
    {
        if (-1 == functionIndex) { return; }

        m_pTreeViewModel->m_funcId = m_functionIdVec.at(functionIndex);
        AMDTProfileFunctionData  functionData;
        m_pProfDataRdr->GetFunctionDetailedProfileData(m_pTreeViewModel->m_funcId,
                                                                 AMDT_PROFILE_ALL_PROCESSES,
                                                                 AMDT_PROFILE_ALL_THREADS,
                                                                 functionData);


        gtString srcFilePath;
        AMDTSourceAndDisasmInfoVec srcInfoVec;
        m_pProfDataRdr->GetFunctionSourceAndDisasmInfo(m_pTreeViewModel->m_funcId, srcFilePath, srcInfoVec);

        m_srcFilePath = srcFilePath;

            m_pTreeViewModel->m_pid = AMDT_PROFILE_ALL_PROCESSES;
            m_pTreeViewModel->m_tid = AMDT_PROFILE_ALL_THREADS;

            bool isMultiPID = (functionData.m_pidsList.size() > 1);
            bool isMultiTID = (functionData.m_threadsList.size() > 1);
            QStringList pidList, tidList;

            if (isMultiPID)
            {
                for (const auto& pid : functionData.m_pidsList)
                {
                    pidList << (QString::number(pid));
                }

                pidList.insert(0, CP_profileAllProcesses);
            }

            if (isMultiTID)
            {
                for (const auto& tid : functionData.m_threadsList)
                {
                    tidList << (QString::number(tid));
                }

                tidList.insert(0, CP_profileAllThreads);
            }

            // Update the PID and TID string lists:
            m_pPIDComboBoxAction->UpdateStringList(pidList);
            m_pTIDComboBoxAction->UpdateStringList(tidList);

            // Show / Hide the pid and tid combo boxes:
            // Notice: Since a widget cannot be hide on a toolbar, we should hide the action of the widget:
            m_pPIDComboBoxAction->UpdateVisible(isMultiPID);
            m_pPIDLabelAction->UpdateVisible(isMultiPID);
            m_pTIDComboBoxAction->UpdateVisible(isMultiPID || isMultiTID);
            m_pTIDLabelAction->UpdateVisible(isMultiPID || isMultiTID);

            m_pTIDComboBoxAction->UpdateEnabled(isMultiTID);

            // Update the display:
            ProtectedUpdateTableDisplay(UPDATE_TABLE_REBUILD);
    }
}


void SessionSourceCodeView::UpdateWithNewSymbol()
{
    //If there is no source, just show dasm
    m_pTreeViewModel->m_srcFile.clear();

    // We need to query new source file every time the symbol is changed:
    switch (m_pTreeViewModel->m_modType)
    {
        case AMDT_MODULE_TYPE_NATIVE:
        {
            m_pTreeViewModel->SetupSourceInfoForUnManaged();

            bool rc = ReadPE();
            GT_ASSERT(rc);

        }
        break;

        case CpuProfileModule::JAVAMODULE:
        {
            // Each Java jitted module contains one Java symbol.
            // So we need to repopulate the map everytime
            // symbol is changed.
            m_pTreeViewModel->SetupSourceInfoForJava(m_pTreeViewModel->m_loadAddr);

            if ((m_pTreeViewModel->m_pDisplayedFunction->getSourceFile() != L"Unknown Source File")
                && (m_pTreeViewModel->m_pDisplayedFunction->getSourceFile() != L"UnknownJITSource"))
            {
                m_pTreeViewModel->m_srcFile = acGTStringToQString(m_pTreeViewModel->m_pDisplayedFunction->getSourceFile());
            }
        }
        break;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        case AMDT_MODULE_TYPE_MANAGEDDPE:
        {
            // Each CLR jitted module contains one CLR symbol.
            // So we need to repopulate the map everytime
            // symbol is changed.
            m_pTreeViewModel->SetupSourceInfoForClr(m_pTreeViewModel->m_currentSymbolIterator->m_va);
        }
        break;
#endif // AMDT_WINDOWS_OS

#ifdef TBI

        case CpuProfileModule::OCLMODULE:
        {
            // Each OCL jitted module can contain multiple symbols.
            bool rc = SetupSourceInfoForOcl(addr);
            GT_ASSERT(rc);
        }
        break;
#endif

        default:
            break;
    }

    // If m_pTreeViewModel->m_srcFile is empty, no Source info available.
    // So we clear the source info line map.
    QString fileLocationStr;

    if (m_pTreeViewModel->m_srcFile.isEmpty())
    {
        m_pTreeViewModel->m_funOffsetLinenumMap.clear();

        fileLocationStr = QString("Module: ") + m_pTreeViewModel->m_moduleName;

        // For JAVAMODULE and MANAGEDPE, we update the jnc file
        // in the module label.
        if (m_pTreeViewModel->m_modType == AMDT_MODULE_TYPE_MANAGEDDPE || m_pTreeViewModel->m_modType == AMDT_MODULE_TYPE_JAVA)
        {
            fileLocationStr += " (" + acGTStringToQString(m_pTreeViewModel->m_pDisplayedFunction->getJncFileName()) + ")";
        }
    }
    else
    {
        fileLocationStr = m_pTreeViewModel->m_srcFile;
    }

    m_pModuleLocationInfoLabel->setText(fileLocationStr);
}

bool SessionSourceCodeView::UpdateDisplay()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTreeViewModel != nullptr)
    {
        // Fill the hot spot indicator combo:
        FillHotspotIndicatorCombo();

        // Set the code to point the current display symbol:
        UpdateWithNewSymbol();

        // Create the model data:
        retVal = CreateModelData();

        // Update the column widths:
        UpdateColumnWidths();

        // Update the percent columns to display in the delegate item:
        UpdatePercentDelegate();

        // Refresh the view:
        RefreshView();
    }

    return retVal;
}


void SessionSourceCodeView::OnPIDComboChange(int index)
{
    if (index >= 0)
    {
        m_pTreeViewModel->m_pid = AMDT_PROFILE_ALL_PROCESSES;
        const QComboBox* pPIDComboBox = (QComboBox*)TopToolbarComboBox(m_pPIDComboBoxAction);
        const QComboBox* pTIDComboBox = (QComboBox*)TopToolbarComboBox(m_pTIDComboBoxAction);
        GT_IF_WITH_ASSERT((pPIDComboBox != nullptr) && (m_pPIDComboBoxAction != nullptr)
                          && (pTIDComboBox != nullptr) && (m_pTIDComboBoxAction != nullptr))
        {
            if (index != SHOW_ALL_PIDS)
            {
                m_pTreeViewModel->m_pid = pPIDComboBox->currentText().toUInt();
            }

            // set tid to ALL_TIDS
            m_pTreeViewModel->m_tid = AMDT_PROFILE_ALL_THREADS;

            QStringList tidList;

            // For each sample
            if (m_pTreeViewModel->m_pDisplayedFunction != nullptr)
            {
                AptAggregatedSampleMap::const_iterator sit = m_pTreeViewModel->m_pDisplayedFunction->getBeginSample();
                AptAggregatedSampleMap::const_iterator send = m_pTreeViewModel->m_pDisplayedFunction->getEndSample();

                for (; sit != send; ++sit)
                {
                    bool isSamePid = (sit->first.m_pid == m_pTreeViewModel->m_pid) || m_pTreeViewModel->m_pid == SHOW_ALL_PIDS;

                    if (SHOW_ALL_TIDS == sit->first.m_tid || !isSamePid || !IsAddressInCurrentFunction(sit->first.m_addr + m_pTreeViewModel->m_pDisplayedFunction->getBaseAddr()))
                    {
                        continue;
                    }

                    int found_index = tidList.indexOf(QString::number(sit->first.m_tid));

                    if (0 > found_index)
                    {
                        tidList << QString::number(sit->first.m_tid);
                    }
                }

            }

            bool isMultiTID =  false;

            if (pTIDComboBox->count() > 0)
            {
                isMultiTID = true;
            }

            if (isMultiTID)
            {
                // Block signals (do not update GUI while adding the new TID list to the table. This will be done
                // later, when the TID will be selected):
                m_pTIDComboBoxAction->blockSignals(true);
                m_pPIDComboBoxAction->blockSignals(true);

                // Set the list of TIDs:
                tidList.insert(0, CP_profileAllThreads);
                m_pTIDComboBoxAction->UpdateStringList(tidList);

                // Unblock signals:
                m_pTIDComboBoxAction->blockSignals(false);
                m_pPIDComboBoxAction->blockSignals(false);
            }

            m_pTIDComboBoxAction->UpdateEnabled(isMultiTID);

            // Set the TID current index (source code table will be updated):
            m_pTIDComboBoxAction->UpdateCurrentIndex(-1);
            m_pTIDComboBoxAction->UpdateCurrentIndex(SHOW_ALL_TIDS);
        }
    }
}


void SessionSourceCodeView::OnTIDComboChange(int index)
{
    if (index >= 0)
    {
        const QComboBox* pTIDComboBox = TopToolbarComboBox(m_pTIDComboBoxAction);
        GT_IF_WITH_ASSERT((m_pTreeViewModel != nullptr) && (pTIDComboBox != nullptr))
        {
            if (SHOW_ALL_TIDS == index)
            {
                m_pTreeViewModel->m_tid = AMDT_PROFILE_ALL_THREADS;
            }
            else
            {
                m_pTreeViewModel->m_tid = pTIDComboBox->currentText().toUInt();
            }

            ProtectedUpdateTableDisplay(UPDATE_TABLE_REBUILD);
        }
    }
}


void SessionSourceCodeView::HighlightMatchingSourceLine(gtVAddr functionFirstAddressLine)
{
    gtVAddr currentLineAddress = 0;
    gtVAddr nextLineAddress = 0;

    // Reset selection information:
    m_userDisplayInformation.clear();

    for (int lineNumber = 0; lineNumber < m_pTreeViewModel->topLevelItemCount() && (m_userDisplayInformation.m_selectedTopLevelItemIndex  == -1); lineNumber++)
    {
        // Get the current and next source line items:
        SourceViewTreeItem* pSrcItem = (SourceViewTreeItem*)m_pTreeViewModel->topLevelItem(lineNumber);
        SourceViewTreeItem* pNextSrcItem = (SourceViewTreeItem*)m_pTreeViewModel->topLevelItem(lineNumber + 1);

        if (pSrcItem != nullptr)
        {
            // Get the current and next line addresses:
            currentLineAddress = pSrcItem->data(SOURCE_VIEW_ADDRESS_COLUMN).toString().toULongLong(nullptr, 16);

            if (pNextSrcItem != nullptr)
            {
                nextLineAddress = pNextSrcItem->data(SOURCE_VIEW_ADDRESS_COLUMN).toString().toULongLong(nullptr, 16);
            }

            bool isLocated = false;

            // Check the address of the next source line item. If the address is the requested one, then select the current item. In this case, we will select the function header:
            bool isNextAddressInScope = (nextLineAddress == functionFirstAddressLine);

            // In cases where an IBS event is attributed to an address that falls within the code bytes of an instruction
            // and not to the address of the instruction itself, compare the scope:
            if (!isNextAddressInScope && m_pTreeViewModel->m_isDisplayingOnlyDasm)
            {
                isNextAddressInScope = (currentLineAddress != 0 && currentLineAddress < functionFirstAddressLine && nextLineAddress > functionFirstAddressLine);
            }

            if (isNextAddressInScope)
            {
                // Select the assembly item:
                m_userDisplayInformation.m_selectedTopLevelItemIndex = (pNextSrcItem != nullptr) ? (lineNumber + 1) : lineNumber;
                m_userDisplayInformation.m_shouldExpandSelected = false;
                isLocated = true;
                break;
            }

            for (int jLine = 0; jLine < pSrcItem->childCount(); jLine++)
            {
                // Search in the children level:
                SourceViewTreeItem* pAsmItem = (SourceViewTreeItem*)pSrcItem->child(jLine);

                if (!pAsmItem)
                {
                    continue;
                }

                currentLineAddress = pAsmItem->data(SOURCE_VIEW_ADDRESS_COLUMN).toString().toULongLong(nullptr, 16);

                nextLineAddress = 0;
                SourceViewTreeItem* pNextAsmItem = (SourceViewTreeItem*)pSrcItem->child(jLine + 1);

                if (pNextAsmItem)
                {
                    nextLineAddress = pNextAsmItem->data(SOURCE_VIEW_ADDRESS_COLUMN).toString().toULongLong(nullptr, 16);
                }

                // In cases where an IBS event is attributed to an address that falls within the code bytes of an instruction
                // and not to the address of the instruction itself, compare the scope:
                if ((currentLineAddress == functionFirstAddressLine) || (currentLineAddress != 0 && currentLineAddress < functionFirstAddressLine && nextLineAddress > functionFirstAddressLine))
                {
                    // Select the assembly item:
                    m_userDisplayInformation.m_selectedTopLevelItemIndex = (pNextSrcItem != nullptr) ? (lineNumber + 1) : lineNumber;
                    m_userDisplayInformation.m_selectedTopLevelChildItemIndex = jLine;
                    m_userDisplayInformation.m_shouldExpandSelected = true;
                    isLocated = true;
                    break;
                }
            }

            if (isLocated)
            {
                break;
            }
        }
    }

    // reset m_newAddress to 0.
    m_pTreeViewModel->m_newAddress = 0;
}

void SessionSourceCodeView::OnExpandAll()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSourceCodeTree != nullptr)
    {
        m_pSourceCodeTree->ExpandAll();
        m_userDisplayInformation.m_expandedTreeItems.clear();

        for (int i = 0; i < m_pTreeViewModel->topLevelItemCount(); i++)
        {
            SourceViewTreeItem* pItem = m_pTreeViewModel->topLevelItem(i);

            if (pItem != nullptr)
            {
                if (pItem->childCount() > 0)
                {
                    m_userDisplayInformation.m_expandedTreeItems << i;
                }
            }
        }
    }

}

void SessionSourceCodeView::OnCollapseAll()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSourceCodeTree != nullptr)
    {
        m_pSourceCodeTree->CollapseAll();
        m_userDisplayInformation.m_expandedTreeItems.clear();
    }
}

void SessionSourceCodeView::OnShowAddress()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pShowAddressAction != nullptr) && (m_pSourceCodeTree != nullptr))
    {
        bool shouldShow = m_pShowAddressAction->isChecked();
        m_pSourceCodeTree->ShowColumn(SOURCE_VIEW_ADDRESS_COLUMN, shouldShow);
        m_pTreeViewModel->UpdateHeaders();
    }
}

void SessionSourceCodeView::OnShowNoteWindow()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pShowNoteAction != nullptr) && (m_pNoteWidget != nullptr))
    {
        if (m_CLUNoteShown)
        {
            m_CLUNoteShown = false;
            m_pNoteWidget->setHidden(true);
            m_pNoteHeader->setHidden(true);
            m_pShowNoteAction->setText(CP_sourceCodeShowCluNotes);
        }
        else
        {
            m_CLUNoteShown = true;
            m_pNoteWidget->setHidden(false);
            m_pNoteHeader->setHidden(false);
            m_pShowNoteAction->setText(CP_sourceCodeHideCluNotes);
        }
    }
}

void SessionSourceCodeView::OnShowCodeBytes()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pShowCodeBytesAction != nullptr) && (m_pSourceCodeTree != nullptr))
    {
        // The column should show / hide if the action is checked
        bool shouldShow = m_pShowCodeBytesAction->isChecked();
        m_pSourceCodeTree->ShowColumn(SOURCE_VIEW_CODE_BYTES_COLUMN, shouldShow);
    }
}

bool SessionSourceCodeView::GetActualSourceFile(const QString& targetFile, QString& tryFile)
{
    if (targetFile.isEmpty())
    {
        return false;
    }

    GetCachedFile(m_sessionDir, targetFile, tryFile);

    bool ret = true;

    if (!QFile::exists(tryFile))
    {
        tryFile = FindSourceFile(targetFile);

        if (tryFile.isEmpty())
        {
            return false;
        }

        ret = CacheRelocatedSource(m_sessionDir, targetFile, tryFile, m_pTreeViewModel->m_isModuleCached);
    }
    else if (m_pTreeViewModel->m_isModuleCached && (0 == targetFile.compare(tryFile)))
    {

        CacheFile(m_sessionDir, targetFile);
    }

    // Display the complete source file path for java source files
    if ((m_pTreeViewModel->m_modType == AMDT_MODULE_TYPE_JAVA)
        && (! tryFile.isEmpty()))
    {
        m_pModuleLocationInfoLabel->setText(tryFile);
    }

    return ret;
}


QString SessionSourceCodeView::FindSourceFile(QString fileName)
{
    QString tryFile = fileName;

    gtString sourceDirectories = afProjectManager::instance().currentProjectSettings().SourceFilesDirectories();
    QString srcDirs = acGTStringToQString(sourceDirectories);

    // add the working directory
    QString wrkDir = QString::fromWCharArray(m_pProfileInfo->m_wrkDirectory.asCharArray());

    QStringList dirList;

    if (!srcDirs.isEmpty())
    {
        dirList = srcDirs.split(";");
    }

    // Add the working directory
    if (! wrkDir.isEmpty())
    {
        dirList += wrkDir.split(";");
    }

    QStringList::iterator curDir = dirList.begin();

    while (!QFile::exists(tryFile))
    {
        //try default source directory
        if (curDir != dirList.end())
        {
            tryFile = *curDir + PATH_SLASH + fileName.section(PATH_SLASH, -1);
            curDir ++;
        }
        else
        {
            // We did not find the file, ask user where they are.
            // Save the path of the user selected and use it next time.
            tryFile = QFileDialog::getOpenFileName(parentWidget(),
                                                   "Locate source file " + fileName.section(PATH_SLASH, -1),
                                                   fileName, "Source File (*.*)");

            if (tryFile.isEmpty())
            {
                return QString::null;
            }

            QFileInfo fileInfo(tryFile);

            QString questionStr = QString(AF_STR_sourceCodeQuestionInclude).arg(fileInfo.dir().absolutePath());

            if (acMessageBox::instance().question(AF_STR_QuestionA, questionStr, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
            {
                if ((!srcDirs.isEmpty()) && (!srcDirs.endsWith(";")))
                {
                    srcDirs += ";";
                }

                // QFileinfo returns linux based path-slash separator. Convert it to OS specific
                osFilePath path;
                path.setFullPathFromString(acQStringToGTString(tryFile));
                tryFile = acGTStringToQString(path.asString());

                osDirectory dirPath;
                path.getFileDirectory(dirPath);
                srcDirs += acGTStringToQString(dirPath.directoryPath().asString());

                afProjectManager::instance().SetSourceFilesDirectories(acQStringToGTString(srcDirs));
            }
        }
    }

    return tryFile;
}


#ifdef TBI
bool SessionSourceCodeView::SetupSourceInfoForOcl(gtVAddr address)
{
    if (m_pTreeViewModel->m_currentSymbolIterator == m_pTreeViewModel->m_symbolsInfoList.end())
    {
        return false;
    }

    // If the JNC file does not change, we can just return
    QString jncFilePath(m_sessionDir + "/" +
                        QString::fromStdWString(m_pDisplayedFunction->getJncFileName()));

    // Get the module name (i.e. the path of OCL file)
    wchar_t modName[OS_MAX_PATH];

    if (m_oclJncReader.GetModuleName(modName, OS_MAX_PATH)
        && jncFilePath.compare(QString::fromWCharArray(modName)) == 0)
    {
        return true;
    }

    // In case the JNC file changes, we open the new JNC file
    m_pTreeViewModel->m_pCode = nullptr;
    m_oclJncReader.Close();

    // Try open the OCL JNC file
    gtVAddr jitLoadAddr = m_pTreeViewModel->m_currentSymbolIterator->symbolRVA;

    if (!m_oclJncReader.Open
        ((TCHAR*)jncFilePath.toStdWString().c_str(), jitLoadAddr, m_pSymbolEngine))
    {
        return false;
    }

    gtVAddr symRVA = jitLoadAddr - m_oclJncReader.GetJITLoadAddress() -
                     m_oclJncReader.GetCodeOffset();

    SESYMBOL sym;
    unsigned int symSize = 0;

    if (m_pSymbolEngine->GetSymbolFromAddress
        (jitLoadAddr - m_oclJncReader.GetJITLoadAddress(), &sym))
    {
        symSize = sym.symbolSize;
    }

    // Get code pointer
    unsigned int codeOffset = 0;
    unsigned int codeSize = 0;
    m_pTreeViewModel->m_pCode = m_oclJncReader.GetCodeBytesOfTextSection(&codeOffset, &codeSize);
    UINT64 base = 0;
    m_pSymbolEngine->GetBaseLoad(&base);

    if (codeSize > symRVA)
    {
        codeSize -= symRVA;
    }

    m_pTreeViewModel->m_pCode += symRVA;

    if (m_pTreeViewModel->m_pCode == nullptr)
    {
        m_oclJncReader.Close();
        return false;
    }

    // Get code size
    if (symSize != 0)
    {
        m_pTreeViewModel->m_codeSize = symSize;
    }
    else if (codeSize != 0)
    {
        m_pTreeViewModel->m_codeSize = codeSize;
    }
    else
    {
        return false;
    }

    // Get long mode
    m_pTreeViewModel->m_isLongMode = (m_oclJncReader.OperandSize() == OS_64BIT);

    // Update Source file
    SELINE seline;

    if (!m_pSymbolEngine->GetSourceLineFromAddress(symRVA + m_oclJncReader.GetCodeOffset(), &seline))
    {
        return false;
    }

    m_pTreeViewModel->m_srcFile.clear();
    m_pTreeViewModel->m_srcFile = QString::fromWCharArray(seline.filePathName);

    // Get Offset from symbol to line number map
    LinenumMap* pMap = m_oclJncReader.GetpLineMap();
    LinenumMap::iterator it = pMap->begin();
    LinenumMap::iterator itEnd = pMap->end();
    m_pTreeViewModel->m_funOffsetLinenumMap.clear();

    for (; it != itEnd; it++)
    {
        if (it->first >= symRVA && (it->first <= symRVA + m_pTreeViewModel->m_codeSize))
        {
            m_pTreeViewModel->m_funOffsetLinenumMap.insert(it->first - symRVA, it->second);
        }
    }

    return true;
}
#endif

bool SessionSourceCodeView::ReadPE()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pTreeViewModel != nullptr) && (m_pTreeViewModel->m_pExecutable != nullptr))
    {
        // Tell the disassembler if the code is 64-bits
        m_pTreeViewModel->m_isLongMode = m_pTreeViewModel->m_pExecutable->Is64Bit();

        gtRVAddr funStartAddr = static_cast<gtRVAddr>(m_pTreeViewModel->m_loadAddr - m_pTreeViewModel->m_pModule->getBaseAddr());

        if (0 == funStartAddr)
        {
            funStartAddr = m_pTreeViewModel->m_pExecutable->GetCodeBase();
        }

        unsigned sectionIndex = m_pTreeViewModel->m_pExecutable->LookupSectionIndex(funStartAddr);

        if (m_pTreeViewModel->m_pExecutable->GetSectionsCount() <= sectionIndex)
        {
            retVal = false;
        }
        else
        {
            m_pTreeViewModel->m_pCode = m_pTreeViewModel->m_pExecutable->GetSectionBytes(sectionIndex);

            gtRVAddr sectionStartRva = 0, sectionEndRva = 0;
            m_pTreeViewModel->m_pExecutable->GetSectionRvaLimits(sectionIndex, sectionStartRva, sectionEndRva);

            // GetCodeBytes return the pointer to the sectionStart
            // We need to add the offset to the beginning of the function
            gtRVAddr sectionFuncOffset = funStartAddr - sectionStartRva;
            m_pTreeViewModel->m_pCode += sectionFuncOffset;
            m_pTreeViewModel->m_codeSize = m_pTreeViewModel->m_pExecutable->GetCodeSize();

            // Do not pass beyond the section's limits!
            unsigned int sectionVirtualSize = sectionEndRva - sectionStartRva;

            if (sectionVirtualSize < m_pTreeViewModel->m_codeSize)
            {
                m_pTreeViewModel->m_codeSize = sectionVirtualSize;
            }

            // The code size is relative to the function's address
            m_pTreeViewModel->m_codeSize -= sectionFuncOffset;

            if (m_pTreeViewModel->m_currentSymbolIterator != m_pTreeViewModel->m_symbolsInfoList.end() && 0 < m_pTreeViewModel->m_currentSymbolIterator->m_size && m_pTreeViewModel->m_currentSymbolIterator->m_size < m_pTreeViewModel->m_codeSize)
            {
                m_pTreeViewModel->m_codeSize = m_pTreeViewModel->m_currentSymbolIterator->m_size;
            }

            retVal = true;
        }
    }

    return retVal;
}


void SessionSourceCodeView::OnHotSpotComboChanged(const QString& text)
{
    // Set the selected hot spot (to be selected later when switching pid / tid):
    m_userDisplayInformation.m_selectedHotSpot = text;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTreeViewModel != nullptr)
    {
        auto counterIdx = std::find(m_supportedCounterList.begin(), m_supportedCounterList.end(), acQStringToGTString(text));

        if (counterIdx != m_supportedCounterList.end())
        {
            //int val = counterIdx - m_supportedCounterList.begin();
            // Re-Set the tree samples column:
            //m_pTreeViewModel->SetTreeSamples(text);
            AMDTUInt32 counterId = m_pDisplayFilter->GetCounterId(text);

            if (0 != counterId)
            {
                m_pTreeViewModel->SetHotSpotSamples(counterId);
            }
        }

        // Refresh the view:
        RefreshView();

    }
}

void SessionSourceCodeView::OnSelectAll()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSourceCodeTree != nullptr)
    {
        m_pSourceCodeTree->selectAll();
    }
}

void SessionSourceCodeView::SetItemsDelegate()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSourceCodeTree != nullptr)
    {
        // Create the tree item delegate:
        m_pTreeItemDelegate = new acTreeItemDeletate;


        // Set the frozen tree as owner to the delegate item:
        m_pTreeItemDelegate->SetOwnerFrozenTree(m_pSourceCodeTree);

        // Set the delegate item for the tree:
        m_pSourceCodeTree->SetItemDelegate(m_pTreeItemDelegate);
    }
}

void SessionSourceCodeView::AddSourceCodeItemToExplorer()
{
    // Add a source code item to the tree:
    GT_IF_WITH_ASSERT((ProfileApplicationTreeHandler::instance() != nullptr) && (m_pDisplayedSessionItemData != nullptr))
    {
        CPUSessionTreeItemData* pDispalyedSessionData = qobject_cast<CPUSessionTreeItemData*>(m_pDisplayedSessionItemData->extendedItemData());
        afApplicationTreeItemData* pSessionData = ProfileApplicationTreeHandler::instance()->FindItemByProfileDisplayName(pDispalyedSessionData->m_displayName);

        if (nullptr != pSessionData)
        {
            afApplicationTreeItemData* pSourceNode = CpuProfileTreeHandler::instance().FindSessionSourceCodeItemData(pSessionData,
                                                     m_pTreeViewModel->m_moduleName);

            if (nullptr == pSourceNode)
            {
                CPUSessionTreeItemData* pSourceCodeData = new CPUSessionTreeItemData;

                pSourceCodeData->CopyFrom(pDispalyedSessionData, true);
                pSourceCodeData->m_exeFullPath = m_pTreeViewModel->m_moduleName;

                afApplicationTreeItemData* pSourceCodeCreatedData = CpuProfileTreeHandler::instance().AddSourceCodeToSessionNode(pSourceCodeData);
                GT_IF_WITH_ASSERT(pSourceCodeCreatedData != nullptr)
                {
                    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
                    GT_IF_WITH_ASSERT((pApplicationCommands != nullptr) && (pApplicationCommands->applicationTree() != nullptr))
                    {
                        // The file path for the source code node should be the session file. We want it to open first before opening the
                        // source code view
                        pSourceCodeCreatedData->m_filePath = m_pDisplayedSessionItemData->m_filePath;
                        pSourceCodeCreatedData->m_filePathLineNumber = AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODE;

                        // Select the new source code item in the tree and activate it
                        pApplicationCommands->applicationTree()->selectItem(pSourceCodeCreatedData, false);

                        m_pDisplayedSessionItemData = pSourceCodeCreatedData;
                    }
                }
            }
        }
    }
}

void SessionSourceCodeView::CreateHotSpotIndicatorComboBox()
{
    GT_IF_WITH_ASSERT(m_pTopToolbar != nullptr)
    {
        m_pTopToolbar->AddLabel(CP_overviewPageHotspotIndicatorHeader, true, true);

        m_pHotSpotIndicatorComboBoxAction = m_pTopToolbar->AddComboBox(QStringList(),
                                                                       SIGNAL(currentIndexChanged(const QString&)),
                                                                       this,
                                                                       SLOT(OnHotSpotComboChanged(const QString&)));
    }
}

void SessionSourceCodeView::SetTreeSelection(SourceViewTreeItem* pItemToSelect)
{
    // Sanity check:
    if (pItemToSelect != nullptr)
    {
        SourceViewTreeItem* pItemToExpand = pItemToSelect;

        if ((pItemToSelect->parent() != nullptr) && m_userDisplayInformation.m_shouldExpandSelected)
        {
            pItemToExpand = (SourceViewTreeItem*)pItemToSelect->parent();
        }

        // Set the focus on the source code tree:
        m_pSourceCodeTree->setFocus();

        // Get the index for the requested item:
        QModelIndex index = m_pTreeViewModel->indexOfItem(pItemToSelect);
        GT_IF_WITH_ASSERT(index.isValid())
        {
            // Expand / collapse the item:
            QModelIndex indexToExpand = m_pTreeViewModel->indexOfItem(pItemToExpand);
            m_pSourceCodeTree->SetItemExpanded(indexToExpand, m_userDisplayInformation.m_shouldExpandSelected);

            // Select the item:
            m_pSourceCodeTree->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);

            // Scroll to the item before this one:
            QModelIndex indexToScrollTo = index;

            if (pItemToSelect->parent() != nullptr)
            {
                if (!m_pTreeViewModel->isItemTopLevel(pItemToSelect))
                {
                    pItemToSelect = pItemToSelect->parent();
                }

                // Get the index of the selected item:
                int indexOfChild = pItemToSelect->parent()->indexOfChild(pItemToSelect);
                SourceViewTreeItem* pPrevItem = pItemToSelect->parent()->child(indexOfChild - 1);

                if (pPrevItem != nullptr)
                {
                    indexToScrollTo = m_pTreeViewModel->indexOfItem(pPrevItem);
                }
            }

            if (m_userDisplayInformation.m_verticalScrollPosition < 0)
            {
                // If the user didn't scroll, scroll the view to the requested item:
                m_pSourceCodeTree->ScrollToBottom();
                m_pSourceCodeTree->ScrollToItem(indexToScrollTo);
            }
        }
    }
}

bool SessionSourceCodeView::CreateModelData()
{
    bool retVal = false;

    QString tryFile;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTreeViewModel != nullptr)
    {
        // Go over the functions of the current modules, and aggregate the sample count:
        //m_pTreeViewModel->CalculateTotalModuleCountVector(m_pProfileReader);

        bool alertMissingSourceFile = false;
        bool failedToAnnotateSource = false;

        if (!m_pTreeViewModel->m_isDisplayingOnlyDasm)
        {
            // Get the source file and store it in the cache
            if (GetActualSourceFile(acGTStringToQString(m_srcFilePath), tryFile))
            {
                bool rc = m_pTreeViewModel->SetSourceLines(tryFile, 1, GT_INT32_MAX);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Build the source and disassembly tree structure:
                    retVal = m_pTreeViewModel->BuildSourceAndDASMTree();
                    failedToAnnotateSource = !retVal;
                }
            }
            else
            {
                alertMissingSourceFile = true;
            }

            if (failedToAnnotateSource || alertMissingSourceFile)
            {
                qApp->restoreOverrideCursor();

                if ((alertMissingSourceFile)
                    && (afGlobalVariablesManager::instance().ShouldAlertMissingSourceFile()))
                {
                    acMessageBox::instance().warning(AF_STR_WarningA, CP_sourceCodeErrorSourceNotFound);
                }
                else if (failedToAnnotateSource)
                {
                    acMessageBox::instance().warning(AF_STR_WarningA, CP_sourceCodeErrorFailedToAnnotateSource);
                }

                qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
                m_pTreeViewModel->m_isDisplayingOnlyDasm = true;
            }
        }

        if (m_pTreeViewModel->m_isDisplayingOnlyDasm)
        {
            // Render DASM
            retVal = m_pTreeViewModel->BuildDisassemblyTree();
			m_pTreeViewModel->m_isDisplayingOnlyDasm = false;
        }
	}
    return retVal;
}

void SessionSourceCodeView::UpdateColumnWidths()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSourceCodeTree != nullptr) && (m_pTreeViewModel != nullptr))
    {
        m_pSourceCodeTree->setColumnWidth(SOURCE_VIEW_SAMPLES_COLUMN, 100);
        m_pSourceCodeTree->setColumnWidth(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, 125);
        m_pSourceCodeTree->setColumnWidth(SOURCE_VIEW_ADDRESS_COLUMN, 80);
        m_pSourceCodeTree->setColumnWidth(SOURCE_VIEW_LINE_COLUMN, 50);
        m_pSourceCodeTree->setColumnWidth(SOURCE_VIEW_SOURCE_COLUMN, 350);
        m_pSourceCodeTree->setColumnWidth(SOURCE_VIEW_CODE_BYTES_COLUMN, 80);
    }
}

#if 0
void SessionSourceCodeView::UpdatePercentDelegate()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pTreeItemDelegate != nullptr) && (m_pSessionDisplaySettings != nullptr))
    {
        QList<int> percentColumnsList;
        percentColumnsList << SOURCE_VIEW_SAMPLES_PERCENT_COLUMN;

        // Add the data items to the list of percent items, if percentage should be displayed:
        bool isProfilingCLU = m_pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU;
        bool displayPercentageInColumn = CPUGlobalDisplayFilter::instance().m_displayPercentageInColumn && (!isProfilingCLU);

        if (displayPercentageInColumn)
        {
            for (int i = 0; i < (int)m_pSessionDisplaySettings->m_availableDataColumnCaptions.size(); i++)
            {
                gtVector<int>::const_iterator valsItBegin = m_pSessionDisplaySettings->m_simpleValuesVector.begin();
                gtVector<int>::const_iterator valsItEnd = m_pSessionDisplaySettings->m_simpleValuesVector.end();

                // Check if this is a simple value counter:
                gtVector<int>::const_iterator findIt = gtFind(valsItBegin, valsItEnd, i);

                if (valsItEnd != findIt)
                {
                    // Add this column to the list:
                    percentColumnsList << SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + 1 + i;
                }
            }
        }

        m_pTreeItemDelegate->SetPercentColumnsList(percentColumnsList);
        m_pTreeItemDelegate->SetPercentForgroundColor(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, acRED_NUMBER_COLOUR);
    }
}
#endif

void SessionSourceCodeView::UpdatePercentDelegate()
{
	// Sanity check:
	GT_IF_WITH_ASSERT((m_pTreeItemDelegate != nullptr) && (m_pDisplayFilter != nullptr))
	{
		QList<int> percentColumnsList;
		percentColumnsList << SOURCE_VIEW_SAMPLES_PERCENT_COLUMN;

		m_pTreeItemDelegate->SetPercentColumnsList(percentColumnsList);
		m_pTreeItemDelegate->SetPercentForgroundColor(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, acRED_NUMBER_COLOUR);
	}
}

void SessionSourceCodeView::RefreshView()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTreeViewModel != nullptr)
    {
        // Update the progress bar + dialog:
        afProgressBarWrapper::instance().setProgressDetails(CP_sourceCodeViewProgressViewUpdate, 1);

        // Update the model:
        m_pTreeViewModel->UpdateModel();

        // Select the selected item:
        applyUserDisplayInformation();

        // Reset the displayed address:
        m_pTreeViewModel->m_newAddress = 0;

        afProgressBarWrapper::instance().hideProgressBar();
    }
}

void SessionSourceCodeView::applyUserDisplayInformation()
{
    // Do not record scroll position:
    m_ignoreVerticalScroll = true;

    // Sanity check:
    const QComboBox* pHotSpotIndicatorComboBox = TopToolbarComboBox(m_pHotSpotIndicatorComboBoxAction);
    GT_IF_WITH_ASSERT((m_pSourceCodeTree != nullptr) && (pHotSpotIndicatorComboBox != nullptr))
    {
        // Go through all the expanded items, and expand it:
        foreach (int topLevelItem, m_userDisplayInformation.m_expandedTreeItems)
        {
            SourceViewTreeItem* pItemToExpand = (SourceViewTreeItem*)m_pTreeViewModel->topLevelItem(topLevelItem);
            GT_IF_WITH_ASSERT(pItemToExpand != nullptr)
            {
                // Get the item index:
                QModelIndex index = m_pTreeViewModel->indexOfItem(pItemToExpand);

                // Expand the item:
                m_pSourceCodeTree->expand(index);
            }
        }

        if (m_userDisplayInformation.m_selectedTopLevelItemIndex >= 0)
        {
            // Select the requested table row:
            SourceViewTreeItem* pSelectedItem = (SourceViewTreeItem*)m_pTreeViewModel->topLevelItem(m_userDisplayInformation.m_selectedTopLevelItemIndex);

            if (pSelectedItem != nullptr)
            {
                if (m_userDisplayInformation.m_selectedTopLevelChildItemIndex >= 0)
                {
                    // Get the selected child of the top level item:
                    pSelectedItem = pSelectedItem->child(m_userDisplayInformation.m_selectedTopLevelChildItemIndex);

                    if (m_userDisplayInformation.m_expandedTreeItems.contains(m_userDisplayInformation.m_selectedTopLevelItemIndex))
                    {
                        // Make sure that the item is expanded if necessary:
                        m_userDisplayInformation.m_shouldExpandSelected = true;
                    }
                }

                if (pSelectedItem != nullptr)
                {
                    // Apply the tree selection:
                    SetTreeSelection(pSelectedItem);
                }
            }
        }
        else
        {
            if (m_pTreeViewModel->m_newAddress != 0)
            {
                HighlightMatchingSourceLine(m_pTreeViewModel->m_newAddress);
            }
        }

        if (m_userDisplayInformation.m_verticalScrollPosition >= 0)
        {
            m_pSourceCodeTree->verticalScrollBar()->setValue(m_userDisplayInformation.m_verticalScrollPosition);
        }

        m_ignoreVerticalScroll = false;

        // Show / Hide address and code bytes columns according to the user selection:
        OnShowAddress();
        OnShowCodeBytes();

        if (!m_userDisplayInformation.m_selectedHotSpot.isEmpty())
        {
            // Set the hot spot indicator combo box value:
            int index = pHotSpotIndicatorComboBox->findText(m_userDisplayInformation.m_selectedHotSpot, Qt::MatchExactly);

            if (index != pHotSpotIndicatorComboBox->currentIndex())
            {
                m_pHotSpotIndicatorComboBoxAction->UpdateCurrentIndex(index);

                m_pTreeViewModel->UpdateModel();
            }
        }

        //
        m_pSourceCodeTree->setFocus(Qt::MouseFocusReason);
    }
}

void SessionSourceCodeView::onEditCopy()
{
    GT_IF_WITH_ASSERT(m_pSourceCodeTree != nullptr)
    {
        m_pSourceCodeTree->onEditCopy();
    }
}

void SessionSourceCodeView::onEditSelectAll()
{
    GT_IF_WITH_ASSERT(m_pSourceCodeTree != nullptr)
    {
        m_pSourceCodeTree->onEditSelectAll();
    }
}

void SessionSourceCodeView::onFindClick()
{
    GT_IF_WITH_ASSERT(m_pSourceCodeTree != nullptr)
    {
        m_pSourceCodeTree->onEditFind();
    }
}

void SessionSourceCodeView::onFindNext()
{
    GT_IF_WITH_ASSERT(m_pSourceCodeTree != nullptr)
    {
        m_pSourceCodeTree->onEditFindNext();
    }
}

// Handle key press event to capture down scrolls using the
// page down keyboard key and the down arrow keyboard key.
void SessionSourceCodeView::keyPressEvent(QKeyEvent* pKeyEvent)
{
    // Check if it's the page down key or the down arrow key.
    if (pKeyEvent != nullptr && (pKeyEvent->key() == Qt::Key_PageDown || pKeyEvent->key() == Qt::Key_Down))
    {
        // Simulate a scroll event and handle it accordingly.
        int currScrollValue = m_pSourceCodeTree->verticalScrollBar()->value();
        OnTreeVerticalScrollPositionChange(currScrollValue, true);
    }
}



#if 0
bool SessionSourceCodeView::FillHotspotIndicatorCombo()
{
    bool retVal = false;

    SessionDisplaySettings* pSessionDisplaySettings = CurrentSessionDisplaySettings();
    const QComboBox* pHotspotCombo = TopToolbarComboBox(m_pHotSpotIndicatorComboBoxAction);
    GT_IF_WITH_ASSERT((pHotspotCombo != nullptr) && (m_pDisplayedSessionItemData != nullptr) && (m_pProfileInfo != nullptr) && (pSessionDisplaySettings != nullptr) && (m_pHotSpotIndicatorComboBoxAction != nullptr))
    {
        // Find the profile type:
        SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(m_pDisplayedSessionItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pSessionData != nullptr)
        {
            QStringList hotSpotColumns;
            pSessionDisplaySettings->calculateDisplayedColumns(m_pProfileReader->getTopologyMap());

            for (int i = 0; i < (int)pSessionDisplaySettings->m_availableDataColumnCaptions.size(); i++)
            {
                QString caption = pSessionDisplaySettings->m_availableDataColumnCaptions[i];
                bool shouldShow = !m_pSessionDisplaySettings->m_filteredDataColumnsCaptions.contains(caption);

                if (shouldShow)
                {
                    QString fullName = pSessionDisplaySettings->m_availableDataFullNames[i];
                    hotSpotColumns << fullName;
                }
            }

            retVal = true;

            // Add the hot spot columns to the list:
            m_pHotSpotIndicatorComboBoxAction->UpdateStringList(hotSpotColumns);
            m_pHotSpotIndicatorComboBoxAction->UpdateEnabled(hotSpotColumns.size() > 1);
        }

    }
    return retVal;
}

void SessionSourceCodeView::DisplayAddress(gtVAddr addr, ProcessIdType pid, ThreadIdType tid)
{
    GT_IF_WITH_ASSERT(m_pFunctionsComboBoxAction != nullptr)
    {
        // Set the m_newAddress to the addr argument. m_newAddress
        // holds the address where the display will be focused once
        // the src/data are redrawn. DisplayAddress resets m_newAddress.
        if (0 != addr)
        {
            m_pTreeViewModel->m_newAddress = addr;
            // Set the new pid and tid's
            // These two variables will be reset to their default values
            // i.e. SHOW_ALL_PIDS and SHOW_ALL_TIDS once
            // the m_pPIDComboBox and m_pTIDComboBox comboboxes are updated.
            m_pTreeViewModel->m_newPid = pid;
            m_pTreeViewModel->m_newTid = tid;

            // Reset current function selection:
            m_pFunctionsComboBoxAction->UpdateCurrentIndex(-1);

            // We assume the m_symInfoList is sorted by the symbolRVA.
            FuncSymbolsList::const_reverse_iterator rit = m_pTreeViewModel->m_symbolsInfoList.rbegin();
            FuncSymbolsList::const_reverse_iterator rend = m_pTreeViewModel->m_symbolsInfoList.rend();
            FuncSymbolsList::iterator it = m_pTreeViewModel->m_symbolsInfoList.end();

            int i = m_pTreeViewModel->m_symbolsInfoList.size() - 1;

            for (; rit != rend; --i, ++rit, --it)
            {
                // Check start address
                if (addr >= rit->m_va)
                {
                    int size = 0;

                    if (0 == rit->m_size)
                    {
                        size = 1;
                    }
                    else
                    {
                        size = rit->m_size;
                    }

                    // Check the function range
                    if (addr < (rit->m_va + size))
                    {
                        // Set the new function as current
                        m_pFunctionsComboBoxAction->UpdateCurrentIndex(i);
                        return;
                    }

                    break;
                }
            }

            // If Java module, just bail out
            if (AMDT_MODULE_TYPE_JAVA == m_pTreeViewModel->m_modType)
            {
                return;
            }

            // There are no samples collected for this symbol, therefore is not listed in the pFunctionsComboBox.
            FunctionSymbolInfo funcInfo;
            memset(&funcInfo, 0, sizeof(funcInfo));

            const CpuProfileFunction* pFunction = m_pTreeViewModel->m_pModule->findFunction(addr);

            gtVAddr baseAddr = m_pTreeViewModel->m_pModule->getBaseAddr();

            gtRVAddr rva;

            if (nullptr != pFunction)
            {
                rva = static_cast<gtRVAddr>(pFunction->getBaseAddr() - baseAddr);
            }
            else
            {
                rva = static_cast<gtRVAddr>(addr - baseAddr);
            }

            const FunctionSymbolInfo* pFuncInfo = (nullptr != m_pTreeViewModel->m_pExecutable && nullptr != m_pTreeViewModel->m_pExecutable->GetSymbolEngine()) ?
                                                  m_pTreeViewModel->m_pExecutable->GetSymbolEngine()->LookupFunction(rva, nullptr, true) : nullptr;

            // normal PE module
            if (nullptr == pFuncInfo)
            {
                funcInfo.m_rva = rva;
                funcInfo.m_size = (nullptr != pFunction) ? pFunction->getSize() : 0;
            }
            else if (0 != pFuncInfo->m_size && rva > (pFuncInfo->m_rva + pFuncInfo->m_size))
            {
                funcInfo.m_rva = rva;
                funcInfo.m_size = pFuncInfo->m_size;
            }
            else
            {
                funcInfo = *pFuncInfo;
            }

            UiFunctionSymbolInfo funcSymbol(funcInfo, baseAddr);

            if (nullptr == funcInfo.m_pName)
            {
                AuxGetParentFunctionName(m_pTreeViewModel->m_pModule, pFunction, funcSymbol.m_va, funcSymbol.m_name);
            }

            // There are no samples collected for this symbol, therefore is not listed in the pFunctionsComboBox.
            // We need to add this in the respective CpuProfileModule::m_funcMap, so that subsequent findFunction()
            // would succeed..
            if (nullptr == pFunction)
            {
                wchar_t funcName[OS_MAX_PATH];
                memset(funcName, 0, OS_MAX_PATH);

                funcSymbol.m_name.toWCharArray(funcName);
                CpuProfileFunction func(funcName,
                                        funcSymbol.m_va,
                                        funcInfo.m_size,
                                        L"",
                                        L"");

                CpuProfileModule* pMod = const_cast<CpuProfileModule*>(m_pTreeViewModel->m_pModule);
                pMod->recordFunction((gtVAddr)funcSymbol.m_va, (const CpuProfileFunction*)&func);
            }

            m_pTreeViewModel->m_symbolsInfoList.insert(it, funcSymbol);
            QString addressStart = "0x" + QString::number(funcSymbol.m_va, 16);
            QString addressEnd = "0x" + QString::number(funcSymbol.m_va + static_cast<gtVAddr>(funcSymbol.m_size), 16);
            QString functionStr = QString("[%1 - %2] : ").arg(addressStart).arg(addressEnd);
            functionStr += funcSymbol.m_name;
            m_pFunctionsComboBoxAction->InsertItemToStringList(i + 1, functionStr);
            m_pFunctionsComboBoxAction->UpdateCurrentIndex(i + 1);
        }
        else
        {
            const QComboBox* pFunctionsComboBox = TopToolbarComboBox(m_pFunctionsComboBoxAction);

            if ((pFunctionsComboBox != nullptr) && (pFunctionsComboBox->count() > 0))
            {
                m_pFunctionsComboBoxAction->UpdateCurrentIndex(-1);
                m_pFunctionsComboBoxAction->UpdateCurrentIndex(0);
            }
        }
    }
}

bool SessionSourceCodeView::DisplayModule(const CpuProfileModule* pModule)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT((pModule != nullptr) && (m_pProfileInfo != nullptr) && (m_pTreeViewModel != nullptr))
    {
        // Store the module:
        m_pTreeViewModel->SetModuleDetails(pModule);

        // Create the central widget:
        m_pWidget = new QWidget(this);


        m_pMainVBoxLayout = new QVBoxLayout(m_pWidget);


        // Setup Module and File location info Label
        m_pModuleLocationInfoLabel = new QLabel("", m_pWidget);


        m_pMainVBoxLayout->addWidget(m_pModuleLocationInfoLabel);

        setCentralWidget(m_pWidget);

        // Create the symbols information:
        m_pTreeViewModel->CreateSymbolInfoList();

        // Create the top layout:
        CreateTopLayout();

        // Create the view layout:
        CreateViewLayout();

        // Create the view's context menu:
        ExtendTreeContextMenu();

        // Add a source code item to the tree:
        AddSourceCodeItemToExplorer();

        // Update display filter string:
        updateDisplaySettingsString();

        // Check if the module is cached:
        CacheFileMap cache;
        ReadSessionCacheFileMap(m_pTreeViewModel->m_sessionDir, cache);
        QString fileName = m_pTreeViewModel->m_moduleName;
        fileName.remove(QChar('\0'));
        m_pTreeViewModel->m_isModuleCached = cache.contains(fileName);

        retVal = true;
    }

    return retVal;
}


void SessionSourceCodeView::OnFunctionsComboChange(int functionIndex)
{
    // When the function is changed, clear the user selection information:
    m_userDisplayInformation.clear();

    // Find the function within the displayed module:
    bool rc = FindRequestedFunctionInModule(functionIndex);
    GT_IF_WITH_ASSERT((m_pTreeViewModel != nullptr) &&
                      (m_pTIDComboBoxAction != nullptr) &&
                      (m_pPIDComboBoxAction != nullptr) &&
                      (m_pPIDLabelAction != nullptr) &&
                      (m_pTIDLabelAction != nullptr))
    {
        if (rc && m_pTreeViewModel->m_pDisplayedFunction != nullptr)
        {
            // Indexes of m_newPid and m_pTreeViewModel->m_newTid in m_pPIDComboBox and pTIDComboBox respectively:
            int new_pid_index = 0;
            int new_tid_index = 0;

            QStringList pidList, tidList;
            // For each sample:
            AptAggregatedSampleMap::const_iterator sit = m_pTreeViewModel->m_pDisplayedFunction->getBeginSample();
            AptAggregatedSampleMap::const_iterator send = m_pTreeViewModel->m_pDisplayedFunction->getEndSample();

            for (; sit != send; sit++)
            {
                // Find and add all matching pids
                if (SHOW_ALL_PIDS == sit->first.m_pid || !IsAddressInCurrentFunction(sit->first.m_addr + m_pTreeViewModel->m_pDisplayedFunction->getBaseAddr()))
                {
                    continue;
                }

                QString processName;
                rc = PIDToProcessName(sit->first.m_pid, processName);
                GT_IF_WITH_ASSERT(rc)
                {
                    int found_index = pidList.indexOf(processName);

                    if (0 > found_index)
                    {
                        pidList << processName;

                        if (sit->first.m_pid == m_pTreeViewModel->m_newPid)
                        {
                            new_pid_index = pidList.size() - 1;
                        }
                    }

                    // Find and add all matching tids:
                    bool shouldSkipTID = SHOW_ALL_TIDS == sit->first.m_tid;

                    if (m_pTreeViewModel->m_newPid != SHOW_ALL_PIDS)
                    {
                        shouldSkipTID = shouldSkipTID || (sit->first.m_pid != m_pTreeViewModel->m_newPid);
                    }

                    if (shouldSkipTID)
                    {
                        continue;
                    }

                    found_index = tidList.indexOf(QString::number(sit->first.m_tid));

                    if (0 > found_index)
                    {
                        tidList << (QString::number(sit->first.m_tid));

                        if (sit->first.m_tid == m_pTreeViewModel->m_newTid)
                        {
                            new_tid_index = tidList.size() - 1;
                        }
                    }
                }
            }

            bool isMultiPID = (pidList.size() > 1);
            bool isMultiTID = (tidList.size() > 1);

            if (isMultiPID)
            {
                pidList.insert(0, CP_profileAllProcesses);
            }

            if (isMultiTID)
            {
                tidList.insert(0, CP_profileAllThreads);
            }

            // Update the PID and TID string lists:
            m_pPIDComboBoxAction->UpdateStringList(pidList);
            m_pTIDComboBoxAction->UpdateStringList(tidList);

            // Show / Hide the pid and tid combo boxes:
            // Notice: Since a widget cannot be hide on a toolbar, we should hide the action of the widget:
            m_pPIDComboBoxAction->UpdateVisible(isMultiPID);
            m_pPIDLabelAction->UpdateVisible(isMultiPID);
            m_pTIDComboBoxAction->UpdateVisible(isMultiPID || isMultiTID);
            m_pTIDLabelAction->UpdateVisible(isMultiPID || isMultiTID);

            m_pTIDComboBoxAction->UpdateEnabled(isMultiTID);

            // Set current index for the new pid
            m_pPIDComboBoxAction->UpdateCurrentIndex(new_pid_index);
            m_pTreeViewModel->m_pid = m_pTreeViewModel->m_newPid;
            m_pTreeViewModel->m_newPid = SHOW_ALL_PIDS;

            // Set current index for the new tid
            m_pTIDComboBoxAction->UpdateCurrentIndex(new_tid_index);
            m_pTreeViewModel->m_tid = m_pTreeViewModel->m_newTid;
            m_pTreeViewModel->m_newTid = SHOW_ALL_TIDS;

            // Update the display:
            ProtectedUpdateTableDisplay(UPDATE_TABLE_REBUILD);
        }
    }
}

void SessionSourceCodeView::OnFunctionsComboChange(int functionIndex)
{
    // When the function is changed, clear the user selection information:
    m_userDisplayInformation.clear();

    // Find the function within the displayed module:
    bool rc = FindRequestedFunctionInModule(functionIndex);
    GT_IF_WITH_ASSERT((m_pTreeViewModel != nullptr) &&
                      (m_pTIDComboBoxAction != nullptr) &&
                      (m_pPIDComboBoxAction != nullptr) &&
                      (m_pPIDLabelAction != nullptr) &&
                      (m_pTIDLabelAction != nullptr))
    {
        if (rc && m_pTreeViewModel->m_pDisplayedFunction != nullptr)
        {
            // Indexes of m_newPid and m_pTreeViewModel->m_newTid in m_pPIDComboBox and pTIDComboBox respectively:
            int new_pid_index = 0;
            int new_tid_index = 0;

            QStringList pidList, tidList;
            // For each sample:
            AptAggregatedSampleMap::const_iterator sit = m_pTreeViewModel->m_pDisplayedFunction->getBeginSample();
            AptAggregatedSampleMap::const_iterator send = m_pTreeViewModel->m_pDisplayedFunction->getEndSample();

            for (; sit != send; sit++)
            {
                // Find and add all matching pids
                if (SHOW_ALL_PIDS == sit->first.m_pid || !IsAddressInCurrentFunction(sit->first.m_addr + m_pTreeViewModel->m_pDisplayedFunction->getBaseAddr()))
                {
                    continue;
                }

                QString processName;
                rc = PIDToProcessName(sit->first.m_pid, processName);
                GT_IF_WITH_ASSERT(rc)
                {
                    int found_index = pidList.indexOf(processName);

                    if (0 > found_index)
                    {
                        pidList << processName;

                        if (sit->first.m_pid == m_pTreeViewModel->m_newPid)
                        {
                            new_pid_index = pidList.size() - 1;
                        }
                    }

                    // Find and add all matching tids:
                    bool shouldSkipTID = SHOW_ALL_TIDS == sit->first.m_tid;

                    if (m_pTreeViewModel->m_newPid != SHOW_ALL_PIDS)
                    {
                        shouldSkipTID = shouldSkipTID || (sit->first.m_pid != m_pTreeViewModel->m_newPid);
                    }

                    if (shouldSkipTID)
                    {
                        continue;
                    }

                    found_index = tidList.indexOf(QString::number(sit->first.m_tid));

                    if (0 > found_index)
                    {
                        tidList << (QString::number(sit->first.m_tid));

                        if (sit->first.m_tid == m_pTreeViewModel->m_newTid)
                        {
                            new_tid_index = tidList.size() - 1;
                        }
                    }
                }
            }

            bool isMultiPID = (pidList.size() > 1);
            bool isMultiTID = (tidList.size() > 1);

            if (isMultiPID)
            {
                pidList.insert(0, CP_profileAllProcesses);
            }

            if (isMultiTID)
            {
                tidList.insert(0, CP_profileAllThreads);
            }

            // Update the PID and TID string lists:
            m_pPIDComboBoxAction->UpdateStringList(pidList);
            m_pTIDComboBoxAction->UpdateStringList(tidList);

            // Show / Hide the pid and tid combo boxes:
            // Notice: Since a widget cannot be hide on a toolbar, we should hide the action of the widget:
            m_pPIDComboBoxAction->UpdateVisible(isMultiPID);
            m_pPIDLabelAction->UpdateVisible(isMultiPID);
            m_pTIDComboBoxAction->UpdateVisible(isMultiPID || isMultiTID);
            m_pTIDLabelAction->UpdateVisible(isMultiPID || isMultiTID);

            m_pTIDComboBoxAction->UpdateEnabled(isMultiTID);

            // Set current index for the new pid
            m_pPIDComboBoxAction->UpdateCurrentIndex(new_pid_index);
            m_pTreeViewModel->m_pid = m_pTreeViewModel->m_newPid;
            m_pTreeViewModel->m_newPid = SHOW_ALL_PIDS;

            // Set current index for the new tid
            m_pTIDComboBoxAction->UpdateCurrentIndex(new_tid_index);
            m_pTreeViewModel->m_tid = m_pTreeViewModel->m_newTid;
            m_pTreeViewModel->m_newTid = SHOW_ALL_TIDS;

            // Update the display:
            ProtectedUpdateTableDisplay(UPDATE_TABLE_REBUILD);
        }
    }
}

bool SessionSourceCodeView::FindRequestedFunctionInModule(int functionIndex)
{
	bool retVal = false;

	// Can we safely bailout if functionIndex is -1 ?
	if (-1 == functionIndex)
	{
		return false;
	}

	// Sanity check:
	GT_IF_WITH_ASSERT(m_pTreeViewModel != nullptr)
	{
		// Check if the function is already in the current address:
		retVal = IsAddressInCurrentFunction(m_pTreeViewModel->m_newAddress);

		if (!retVal)
		{
			retVal = true;

			// Reset the "Display only dasm flag":
			m_pTreeViewModel->m_isDisplayingOnlyDasm = false;

			// Reset the symbol iterator to the beginning of the function:
			m_pTreeViewModel->m_currentSymbolIterator = m_pTreeViewModel->m_symbolsInfoList.begin();
			FuncSymbolsList::iterator itEnd = m_pTreeViewModel->m_symbolsInfoList.end();

			// Increment the iterator to point to the "functionIndex" position in the list:
			for (int i = 0; i < functionIndex; ++i, ++m_pTreeViewModel->m_currentSymbolIterator)
			{
				if (m_pTreeViewModel->m_currentSymbolIterator == itEnd)
				{
					// Function index exceeds the list size:
					retVal = false;
					break;
				}
			}


			GT_IF_WITH_ASSERT(retVal)
			{
				if (0 == m_pTreeViewModel->m_newAddress)
				{
					m_pTreeViewModel->m_newAddress = m_pTreeViewModel->m_currentSymbolIterator->m_va;
				}

				// Find the function within the module:
				m_pTreeViewModel->m_pDisplayedFunction = m_pTreeViewModel->m_pModule->findFunction(m_pTreeViewModel->m_currentSymbolIterator->m_va);

				if (m_pTreeViewModel->m_pDisplayedFunction == nullptr)
				{
					m_pTreeViewModel->m_pDisplayedFunction = m_pTreeViewModel->m_pModule->getUnchartedFunction();
				}

				if (m_pTreeViewModel->m_pDisplayedFunction != nullptr)
				{
					// Set the new display function properties:
					m_pTreeViewModel->m_loadAddr = m_pTreeViewModel->m_currentSymbolIterator->m_va;

					GT_IF_WITH_ASSERT(m_pTreeViewModel->m_pDisplayedFunction != nullptr)
					{
						if (CA_NO_SYMBOL == m_pTreeViewModel->m_currentSymbolIterator->m_name)
						{
							m_pTreeViewModel->m_loadAddr = m_pTreeViewModel->m_pDisplayedFunction->getBaseAddr();
						}
					}
				}
				else
				{
					// Function not found:
					retVal = false;
				}
			}

			UpdateWithNewSymbol();
		}
	}

	return retVal;
}
#endif
