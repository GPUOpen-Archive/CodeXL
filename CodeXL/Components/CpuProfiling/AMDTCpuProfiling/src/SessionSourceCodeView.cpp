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
    m_pTreeViewModel = new SourceCodeTreeModel(m_pSessionDisplaySettings, m_sessionDir, m_pProfDataRdr, m_pDisplayFilter);
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
    // Baskar: FIXME Not required
    // m_pTreeViewModel->CreateSymbolInfoList(m_moduleId, m_processId);

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
    //createCLUNotesFrame(m_pMainVBoxLayout);

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

        // Baskar: Though this approach will have folowing side effects, its OK for 2.2 release
        //      - function combo box will have functions that do not have IP samples
        //              a) functions having only deep samples due to CSS
        //              b) functions from some other instance of the same application (instance-1 may have ip samples
        //                  but instance-2 may not have ip samples)
        AMDTProfileFunctionInfoVec funcInfoVec;
        gtVAddr baseAddr;

        if (m_pProfDataRdr->GetFunctionInfoByModuleId(m_moduleId, funcInfoVec, baseAddr))
        {
            for (auto const& func : funcInfoVec)
            {
                gtUInt64 startOffset = baseAddr + func.m_startOffset;
                gtUInt64 endOffset = startOffset + func.m_size;

                QString addressStart = "0x" + QString::number(startOffset, 16);
                QString addressEnd = "0x" + QString::number(endOffset, 16);
                QString functionStr = QString("[%1 - %2] : ").arg(addressStart).arg(addressEnd);
                functionStr += acGTStringToQString(func.m_name);
                functionsList << functionStr;
                m_functionIdVec.push_back(func.m_functionId);
            }
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
    Q_UNUSED(current);

    if (m_isProfiledClu)
    {
        //SourceViewTreeItem* pSrcItem = m_pTreeViewModel->getItem(current);
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
#if 0
                    CpuEventViewIndexMap::const_iterator idxIt = m_pSessionDisplaySettings->m_eventToIndexMap.find(key);

                    int idx = *idxIt;

                    if (idxList.find(idx) == idxList.end())
                    {
                        // The check is to avoid duplicate data in case the samples are separated by numa
                        idxList.insert(std::pair<char, int>(idx, 0));

                        float dataAsVariant = pSrcItem->data(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + 1 + idx).toFloat();
                        cluData.push_back(dataAsVariant);
                    }

#endif
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
            // Now find the index for this functionId
            AMDTUInt32 index = 0;

            for (auto& aFundId : m_functionIdVec)
            {
                if (aFundId == m_functionId)
                {
                    break;
                }

                index++;
            }

            // Baskar: This is used as hack to generate index change notification
            m_pFunctionsComboBoxAction->UpdateCurrentIndex(-1);
            m_pFunctionsComboBoxAction->UpdateCurrentIndex(index);
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
        if (-1 != functionIndex)
        {
            m_pTreeViewModel->m_funcId = m_functionIdVec.at(functionIndex);

            AMDTProfileFunctionData  functionData;
            QStringList pidList, tidList;
            bool isMultiPID = false;
            bool isMultiTID = false;

            m_pTreeViewModel->m_pid = AMDT_PROFILE_ALL_PROCESSES;
            m_pTreeViewModel->m_tid = AMDT_PROFILE_ALL_THREADS;

            bool ret = m_pProfDataRdr->GetFunctionData(m_pTreeViewModel->m_funcId,
                                                       AMDT_PROFILE_ALL_PROCESSES,
                                                       AMDT_PROFILE_ALL_THREADS,
                                                       functionData);

            if (ret)
            {
                //m_pTreeViewModel->m_newAddress = functionData.m_modBaseAddress + functionData.m_instDataList[0].m_offset;
                m_pTreeViewModel->m_newAddress = functionData.m_modBaseAddress + functionData.m_functionInfo.m_startOffset;

                isMultiPID = (functionData.m_pidsList.size() > 1);
                isMultiTID = (functionData.m_threadsList.size() > 1);

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
}

void SessionSourceCodeView::UpdateWithNewSymbol()
{
    m_pTreeViewModel->SetupSourceInfo();

    // If m_pTreeViewModel->m_srcFile is empty, no Source info available.
    // So we clear the source info line map.
    QString fileLocationStr;

    if (m_pTreeViewModel->m_srcFile.isEmpty())
    {
        m_pTreeViewModel->m_funOffsetLinenumMap.clear();

        fileLocationStr = QString("Module: ") + m_pTreeViewModel->m_moduleName;
        // TODO: For JAVAMODULE and MANAGEDPE, we update the jnc file in the module label.
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
    //QString wrkDir = QString::fromWCharArray(m_pProfileInfo->m_wrkDirectory.asCharArray());
    QString wrkDir = "";

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
        bool alertMissingSourceFile = false;
        bool failedToAnnotateSource = false;

        if (!m_pTreeViewModel->m_isDisplayingOnlyDasm)
        {
            // Get the source file and store it in the cache
            if (GetActualSourceFile(m_pTreeViewModel->m_srcFile, tryFile))
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
    const QComboBox* pHotSpotIndicatorComboBox = TopToolbarComboBox(m_pHotSpotIndicatorComboBoxAction);
    GT_IF_WITH_ASSERT((m_pSourceCodeTree != nullptr) && (pHotSpotIndicatorComboBox != nullptr))
    {
        AMDTUInt64 firstSrcLine = m_pTreeViewModel->GetFuncSrcFirstLnNum();
        const std::vector<SourceViewTreeItem*>& srcLineVec = m_pTreeViewModel->GetSrcLineViewMap();

		if (!srcLineVec.empty())
		{
			auto pSelectedItem = srcLineVec.at(firstSrcLine);

			if (pSelectedItem != nullptr)
			{
				// Apply the tree selection:
				SetTreeSelection(pSelectedItem);

				int max = m_pSourceCodeTree->verticalScrollBar()->maximum();
				int totalLines = srcLineVec.size();
				int scrollVal = max;

				if (0 != totalLines)
				{
					//one line scaling is
					int oneLineScaling = max / totalLines;
					scrollVal = oneLineScaling * firstSrcLine;
				}

				m_pSourceCodeTree->verticalScrollBar()->setValue(scrollVal);
			}
		}
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