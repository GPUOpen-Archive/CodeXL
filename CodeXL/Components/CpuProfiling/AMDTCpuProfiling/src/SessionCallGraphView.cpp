//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionCallGraphView.cpp
/// \brief  Implementation of the CssTab class.
///
//==================================================================================
// $Id: //devtools/branch/CPUProfileGUI/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/CallGraphTab.cpp#3 $
// Last checkin:   $DateTime: 2013/03/24 03:43:56 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 467122 $
//=============================================================

// Qt:
#include <QtCore>
#include <QtWidgets>
#include <QClipboard>
#include <QComboBox>
#include <QHeaderView>
#include <QShortcut>
#include <QStatusBar>

#include <algorithm>

// Infra:
#include <AMDTBaseTools/Include/gtHashMap.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTApplicationComponents/Include/acToolBar.h>
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

// Shared Profiling:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>
#include <AMDTSharedProfiling/inc/StringConstants.h>

// Local:
#include <inc/AmdtCpuProfiling.h>
#include <inc/CpuProfilingOptions.h>
#include <inc/CpuProjectHandler.h>
#include <inc/SessionCallGraphView.h>
#include <inc/SessionWindow.h>
#include <inc/SessionFunctionView.h>
#include <inc/SessionViewCreator.h>
#include <inc/Auxil.h>
#include <inc/CpuProfileCss.h>
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>

// Tooltip strings
#define    CALL_GRAPH_FUNCTION_NAME_TOOLTIP    QString("Function name")
#define    CALL_GRAPH_FUNCTION_PATH_CNT_TOOLTIP    QString("The number of call paths containing this function")
#define    CALL_GRAPH_FUNCTION_PATH_SAMPLES_TOOLTIP    QString("The number of samples from the root of the path to this function")
#define    CALL_GRAPH_FUNCTION_PATH_SAMPLES_PER_PATH_TOOLTIP    QString("Average number of samples per path")
#define    CALL_GRAPH_FUNCTION_SELF_SAMPLES_TOOLTIP    QString("The number of samples taken with this function at the end of the call path")
#define    CALL_GRAPH_FUNCTION_DEEP_SAMPLES_TOOLTIP    QString("The number of times this function appeared in a call path")
#define    CALL_GRAPH_FUNCTION_DEEP_SAMPLE_PERCENTAGE_TOOLTIP    QString("Deep samples divided by total number of samples for this profile run")
#define    CALL_GRAPH_FUNCTION_SOURCE_FILE_TOOLTIP    QString("The source file containing this function")
#define    CALL_GRAPH_FUNCTION_MODULE_TOOLTIP    QString("The module in which this function is defined")

#define    CALL_GRAPH_PATH_TREE_TOOLTIP    QString("The call path tree(s) in which this function appears")
#define    CALL_GRAPH_PATH_SELF_TOOLTIP    QString("The number of samples taken with this function at the end of the call path")
#define    CALL_GRAPH_PATH_DOWNSTREAM_SAMPLES_TOOLTIP    QString("The number of samples from this function down all its call paths")
#define    CALL_GRAPH_PATH_DOWNSTREAM_PERCENTAGE_TOOLTIP    QString("Downstream samples divided by the total downstream samples")

#define    CALL_GRAPH_BUTTERFLY_FUNCTION_TOOLTIP    QString("Function name")
#define    CALL_GRAPH_BUTTERFLY_SAMPLES_TOOLTIP    QString("Displays the number of Deep Samples for every child function, and Self Samples for the [self] row")
#define    CALL_GRAPH_BUTTERFLY_PERCENTAGE_TOOLTIP    QString("Samples divided by total samples of all children or parents")

#define    CALL_GRAPH_BUTTERFLY_UNIFORM_WIDTH 80

#define    CALL_GRAPH_MONITORED_EVENT_PADDING_FACTOR_WIDTH 1.3
#define    CALL_GRAPH_MONITORED_EVENT_MINIMUM_WIDTH 47

#define    ALL_Data  "All data"
#define    MAX_NAME_COLUMN_SIZE_OF_WINDOW 0.4

static gtUInt32 gDeepSamples = 0;

// ****************************
// Class CallGraphPathFuncList
// ***************************

CallGraphPathFuncList::CallGraphPathFuncList(QWidget* pParent)
    : QWidget(pParent)
{
    m_pFuncTable = nullptr;
    m_pCallGraphTab = (SessionCallGraphView*) pParent;
    m_pWindowLabel = nullptr;
}

void CallGraphPathFuncList::clear()
{
    if (nullptr != m_pFuncTable)
    {
        m_pFuncTable->clear();
    }
}

CallGraphFuncListItem* CallGraphPathFuncList::AcquireTopLevelItem(AMDTFunctionId funcId, gtString funcName, gtString modulePath)
{
    // Search for a matching top level item
    CallGraphFuncListItem* pFuncListItem = nullptr;

    for (int itemIndex = 0, itemsCount = m_pFuncTable->topLevelItemCount(); itemIndex < itemsCount; ++itemIndex)
    {
        CallGraphFuncListItem* pCurrFuncListItem = static_cast<CallGraphFuncListItem*>(m_pFuncTable->topLevelItem(itemIndex));

        if (nullptr != pCurrFuncListItem)
        {
            if (funcId == pCurrFuncListItem->m_functionId)
            {
                pFuncListItem = pCurrFuncListItem;
                break;
            }
        }
    }


    if (nullptr == pFuncListItem)
    {
        pFuncListItem = new CallGraphFuncListItem(funcId);
        m_pFuncTable->addTopLevelItem(pFuncListItem);

        InitializeItem(pFuncListItem, funcName, modulePath);
    }

    return pFuncListItem;
}


void CallGraphPathFuncList::InitializeItem(CallGraphFuncListItem* pFuncListItem, gtString funcName, gtString modulePath)
{
    pFuncListItem->setText(CALLGRAPH_PATH_TREE, acGTStringToQString(funcName));

    pFuncListItem->setText(CALLGRAPH_PATH_SELF, QString());
    pFuncListItem->setTextAlignment(CALLGRAPH_PATH_SELF, Qt::AlignLeft | Qt::AlignVCenter);

    pFuncListItem->setText(CALLGRAPH_PATH_DOWNSTREAM_SAMPLES, QString());
    pFuncListItem->setTextAlignment(CALLGRAPH_PATH_DOWNSTREAM_SAMPLES, Qt::AlignLeft | Qt::AlignVCenter);

    pFuncListItem->setText(CALLGRAPH_PATH_DOWNSTREAM_PERCENTAGE, QString());

    if (!modulePath.isEmpty())
    {

        pFuncListItem->setToolTip(CALLGRAPH_PATH_MODULE, acGTStringToQString(modulePath));
    }

    osFilePath moduleFullPath(modulePath);
    gtString modNameWithExt;
    moduleFullPath.getFileNameAndExtension(modNameWithExt);


    pFuncListItem->setText(CALLGRAPH_PATH_MODULE, acGTStringToQString(modNameWithExt));
}


CallGraphFuncListItem* CallGraphPathFuncList::AcquireChildItem(CallGraphFuncListItem* pParentItem, AMDTFunctionId funcId,
                                                               gtString funcName, gtString modulePath)
{
    CallGraphFuncListItem* pFuncListItem;

    if (nullptr != pParentItem)
    {
        // Search for a matching child item
        pFuncListItem = nullptr;

        for (int itemIndex = 0, itemsCount = pParentItem->childCount(); itemIndex < itemsCount; ++itemIndex)
        {
            CallGraphFuncListItem* pCurrFuncListItem = static_cast<CallGraphFuncListItem*>(pParentItem->child(itemIndex));

            if (nullptr != pCurrFuncListItem)
            {
                if (funcId == pCurrFuncListItem->m_functionId)
                {
                    pFuncListItem = pCurrFuncListItem;
                    break;
                }
            }
        }


        if (nullptr == pFuncListItem)
        {
            pFuncListItem = new CallGraphFuncListItem(funcId);
            pParentItem->addChild(pFuncListItem);

            InitializeItem(pFuncListItem, funcName, modulePath);
        }
    }
    else
    {
        pFuncListItem = AcquireTopLevelItem(funcId, funcName, modulePath);
    }

    return pFuncListItem;
}

gtUInt64 CallGraphPathFuncList::AddTreeSample(CallGraphFuncListItem* listItem)
{
    gtUInt64 deepSamples = 0;

    if (nullptr != listItem)
    {
        for (int itemIndex = 0, itemsCount = listItem->childCount(); itemIndex < itemsCount; ++itemIndex)
        {
            CallGraphFuncListItem* pCurrFuncListItem = static_cast<CallGraphFuncListItem*>(listItem->child(itemIndex));
            deepSamples += AddTreeSample(pCurrFuncListItem);
        }

        listItem->m_deepSamples = deepSamples;
        deepSamples += listItem->m_selfSample;
    }

    return deepSamples;
}


void CallGraphPathFuncList::SetSamplePercent(CallGraphFuncListItem* listItem, gtUInt64 totalDeepCount)
{
    if (nullptr != listItem)
    {
        for (int itemIndex = 0, itemsCount = listItem->childCount(); itemIndex < itemsCount; ++itemIndex)
        {
            CallGraphFuncListItem* pCurrFuncListItem = static_cast<CallGraphFuncListItem*>(listItem->child(itemIndex));

            if (nullptr != pCurrFuncListItem)
            {
                SetSamplePercent(pCurrFuncListItem, totalDeepCount);
                pCurrFuncListItem->AddCountValue(CALLGRAPH_PATH_SELF, pCurrFuncListItem->m_selfSample);
                pCurrFuncListItem->AddCountValue(CALLGRAPH_PATH_DOWNSTREAM_SAMPLES, pCurrFuncListItem->m_deepSamples);

                pCurrFuncListItem->SetPercentageValue(CALLGRAPH_PATH_DOWNSTREAM_PERCENTAGE,
                                                      static_cast<double>(pCurrFuncListItem->m_deepSamples + pCurrFuncListItem->m_selfSample),
                                                      totalDeepCount);
            }
        }
    }
}

void CallGraphPathFuncList::SetFunctionPath(std::shared_ptr<cxlProfileDataReader> pProfDataRdr,
                                            AMDTFunctionId funcId,
                                            AMDTUInt32 processId,
                                            bool displaySystemModule)
{
    GT_UNREFERENCED_PARAMETER(displaySystemModule);
    m_pFuncTable->blockSignals(true);
    clear();

    gtVector<AMDTCallGraphPath> paths;
    pProfDataRdr->GetCallGraphPaths(processId, funcId, paths);

    // get all the paths for the selected fn id
    AMDTProfileModuleInfoVec moduleInfo;

    for (const auto& path : paths)
    {
        //const double totalDeepCount = static_cast<double>(path.at(0).m_totalDeepSamples);
        CallGraphFuncListItem* pFuncListItem = nullptr;

        for (const auto& func : path)
        {
            moduleInfo.clear();
            pProfDataRdr->GetModuleInfo(processId, func.m_functionInfo.m_moduleId, moduleInfo);

            // acquire only when module info available.
            if (moduleInfo.size() > 0)
            {
                pFuncListItem = AcquireChildItem(pFuncListItem,
                                                 func.m_functionInfo.m_functionId,
                                                 func.m_functionInfo.m_name,
                                                 func.m_functionInfo.m_modulePath);

                pFuncListItem->m_functionId = func.m_functionInfo.m_functionId;
                pFuncListItem->m_functionName = func.m_functionInfo.m_name;
                pFuncListItem->m_moduleName = func.m_functionInfo.m_modulePath;
                pFuncListItem->m_moduleId = func.m_functionInfo.m_moduleId;

                pFuncListItem->setExpanded(true);
                pFuncListItem->m_deepSamples += func.m_totalDeepSamples;
                pFuncListItem->m_selfSample  += func.m_totalSelfSamples;
            }
        }
    }

    //CallGraphFuncListItem* pFuncListItem = nullptr;

    for (int itemIndex = 0, itemsCount = m_pFuncTable->topLevelItemCount(); itemIndex < itemsCount; ++itemIndex)
    {
        CallGraphFuncListItem* pCurrFuncListItem = static_cast<CallGraphFuncListItem*>(m_pFuncTable->topLevelItem(itemIndex));

        if (nullptr != pCurrFuncListItem)
        {
            AddTreeSample(pCurrFuncListItem);
            gtUInt64 deepSamples = (gDeepSamples > 0) ? gDeepSamples : pCurrFuncListItem->m_deepSamples;
            SetSamplePercent(pCurrFuncListItem, deepSamples);

            pCurrFuncListItem->AddCountValue(CALLGRAPH_PATH_SELF, pCurrFuncListItem->m_selfSample);
            pCurrFuncListItem->AddCountValue(CALLGRAPH_PATH_DOWNSTREAM_SAMPLES, pCurrFuncListItem->m_deepSamples);
            pCurrFuncListItem->SetPercentageValue(
                CALLGRAPH_PATH_DOWNSTREAM_PERCENTAGE,
                static_cast<double>(pCurrFuncListItem->m_deepSamples),
                deepSamples);
        }
    }

    if (0 >= m_pFuncTable->topLevelItemCount())
    {
        CallGraphFuncListItem* pEmptyListItem = new CallGraphFuncListItem(m_pFuncTable, nullptr);
        pEmptyListItem->setText(CALLGRAPH_PATH_TREE, "no stack information available");
    }

    bool isSortIndShown = m_pFuncTable->header()->isSortIndicatorShown();
    // disable the sort indicator - for more accurate resizing (header size will not include sort indicator size)
    m_pFuncTable->header()->setSortIndicatorShown(false);

    m_pFuncTable->resizeColumnToContents(CALLGRAPH_PATH_SELF);
    m_pFuncTable->resizeColumnToContents(CALLGRAPH_PATH_DOWNSTREAM_SAMPLES);
    // set back
    m_pFuncTable->header()->setSortIndicatorShown(isSortIndShown);


    // Unblock the signals.
    m_pFuncTable->blockSignals(false);
}

void CallGraphPathFuncList::OnDblClicked(QTreeWidgetItem* pItem)
{
    if (nullptr != pItem)
    {
        CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(pItem);

        if (nullptr != pFuncListItem)
        {
            emit functionSelected(pFuncListItem->m_functionId);
        }
    }
}

void CallGraphPathFuncList::onSelectionChanged()
{
    CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(m_pFuncTable->currentItem());

    if (nullptr != pFuncListItem)
    {
        emit functionSelected(pFuncListItem->m_functionId);
    }
}

void CallGraphPathFuncList::onFunctionClicked(QTreeWidgetItem* pItem)
{
    if (nullptr != pItem)
    {
        CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(pItem);

        if (nullptr != pFuncListItem)
        {
            emit functionSelected(pFuncListItem->m_functionId);
        }
    }
}

void CallGraphPathFuncList::onCurrentListItemChanged(QTreeWidgetItem* pItem)
{
    if (nullptr != pItem)
    {
        CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(pItem);

        if (nullptr != pFuncListItem)
        {
            emit functionSelected(pFuncListItem->m_functionId);
        }
    }
}

bool CallGraphPathFuncList::initialize()
{
    m_pFuncTable = new FunctionsTreeCtrl(m_pCallGraphTab, nullptr, true, CALLGRAPH_PATH_OFFSET);


    acPercentItemDelegate* pDelegate = new acPercentItemDelegate;

    pDelegate->SetPercent(false);
    m_pFuncTable->setItemDelegate(pDelegate);

    // Double click event
    bool rc = connect(m_pFuncTable, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(OnDblClicked(QTreeWidgetItem*)));
    GT_ASSERT(rc);


    //default for all columns
    m_pFuncTable->header()->setSortIndicatorShown(false);
    m_pFuncTable->setSortingEnabled(false);
    m_pFuncTable->setAllColumnsShowFocus(true);  // full row select
    m_pFuncTable->setRootIsDecorated(true);

    QString headerString;
    QFontMetrics fm(font());
    m_pFuncTable->header()->setSectionResizeMode(QHeaderView::Stretch);

    headerString = "Function";
    m_pFuncTable->headerItem()->setText(CALLGRAPH_PATH_TREE, headerString);

    m_pFuncTable->header()->setSectionResizeMode(CALLGRAPH_PATH_TREE, QHeaderView::Interactive);

    m_pFuncTable->headerItem()->setToolTip(CALLGRAPH_PATH_TREE, CALL_GRAPH_PATH_TREE_TOOLTIP);

    headerString = "Self samples";
    m_pFuncTable->headerItem()->setText(CALLGRAPH_PATH_SELF, headerString);

    m_pFuncTable->header()->setSectionResizeMode(CALLGRAPH_PATH_SELF, QHeaderView::Interactive);

    m_pFuncTable->headerItem()->setTextAlignment(CALLGRAPH_PATH_SELF, Qt::AlignLeft | Qt::AlignVCenter);
    m_pFuncTable->headerItem()->setToolTip(CALLGRAPH_PATH_SELF, CALL_GRAPH_PATH_SELF_TOOLTIP);

    headerString = "Downstream samples";
    m_pFuncTable->headerItem()->setText(CALLGRAPH_PATH_DOWNSTREAM_SAMPLES, headerString);

    m_pFuncTable->header()->setSectionResizeMode(CALLGRAPH_PATH_DOWNSTREAM_SAMPLES, QHeaderView::Interactive);

    m_pFuncTable->headerItem()->setTextAlignment(CALLGRAPH_PATH_DOWNSTREAM_SAMPLES, Qt::AlignLeft | Qt::AlignVCenter);
    m_pFuncTable->headerItem()->setToolTip(CALLGRAPH_PATH_DOWNSTREAM_SAMPLES, CALL_GRAPH_PATH_DOWNSTREAM_SAMPLES_TOOLTIP);

    headerString = "% of Downstream samples";
    m_pFuncTable->headerItem()->setText(CALLGRAPH_PATH_DOWNSTREAM_PERCENTAGE, headerString);

    m_pFuncTable->header()->setSectionResizeMode(CALLGRAPH_PATH_DOWNSTREAM_PERCENTAGE, QHeaderView::Interactive);

    m_pFuncTable->headerItem()->setTextAlignment(CALLGRAPH_PATH_DOWNSTREAM_PERCENTAGE, Qt::AlignLeft | Qt::AlignVCenter);
    m_pFuncTable->headerItem()->setToolTip(CALLGRAPH_PATH_DOWNSTREAM_PERCENTAGE, CALL_GRAPH_PATH_DOWNSTREAM_PERCENTAGE_TOOLTIP);

    acTablePercentItemDelegate* pPercentDelegate = new acTablePercentItemDelegate();

    pDelegate->SetOwnerTree(m_pFuncTable);
    m_pFuncTable->setItemDelegateForColumn(CALLGRAPH_PATH_DOWNSTREAM_PERCENTAGE, pPercentDelegate);

    headerString = "Module";
    m_pFuncTable->headerItem()->setText(CALLGRAPH_PATH_MODULE, headerString);

    m_pFuncTable->header()->setSectionResizeMode(CALLGRAPH_PATH_MODULE, QHeaderView::Interactive);

    m_pFuncTable->headerItem()->setTextAlignment(CALLGRAPH_PATH_MODULE, Qt::AlignLeft | Qt::AlignVCenter);
    m_pFuncTable->headerItem()->setToolTip(CALLGRAPH_PATH_MODULE, CALL_GRAPH_FUNCTION_MODULE_TOOLTIP);

    ResizeFunctionNameColumn();

    bool isSortIndShown = m_pFuncTable->header()->isSortIndicatorShown();
    // disable the sort indicator - for more accurate resizing (header size will not include sort indicator size)
    m_pFuncTable->header()->setSortIndicatorShown(false);

    m_pFuncTable->resizeColumnToContents(CALLGRAPH_PATH_SELF);
    m_pFuncTable->resizeColumnToContents(CALLGRAPH_PATH_DOWNSTREAM_SAMPLES);
    m_pFuncTable->resizeColumnToContents(CALLGRAPH_PATH_DOWNSTREAM_PERCENTAGE);
    m_pFuncTable->resizeColumnToContents(CALLGRAPH_PATH_MODULE);

    //set back
    m_pFuncTable->header()->setSortIndicatorShown(isSortIndShown);

    // Setup layout properly
    QGridLayout* pLay = new QGridLayout();

    if (m_pWindowLabel != nullptr)
    {
        pLay->addWidget(m_pWindowLabel, 0, 0, Qt::AlignLeft);
    }

    QCheckBox* pCheckPath = new QCheckBox("Show Call Graph selection path");
    pLay->addWidget(pCheckPath, 0, 1, Qt::AlignLeft);

    m_pFuncTable->connect(pCheckPath, SIGNAL(stateChanged(int)), SLOT(PathIndicatorToggled(int)));

    pLay->addWidget(m_pFuncTable, 1, 0, 1, 2);
    pLay->setContentsMargins(0, 0, 0, 0);
    setLayout(pLay);

    m_pFuncTable->SetPathIndicatorLevel(true);

    return true;
}

CallGraphPathFuncList::~CallGraphPathFuncList()
{
}

void CallGraphPathFuncList::ResizeFunctionNameColumn()
{
    GT_IF_WITH_ASSERT(m_pFuncTable)
    {
        // set function path tree column to 40% of table size
        int maxColumnSize = m_pFuncTable->width() * MAX_NAME_COLUMN_SIZE_OF_WINDOW;
        m_pFuncTable->setColumnWidth(CALLGRAPH_PATH_TREE, maxColumnSize);
    }
}

// ****************************
// Class CallGraphFuncListItem
// ***************************

CallGraphFuncListItem::CallGraphFuncListItem(acTreeCtrl* pParent, QTreeWidgetItem* pAfter) : QTreeWidgetItem(pParent, pAfter)/*,
    m_pFuncNode(nullptr)*/
{
}

CallGraphFuncListItem::CallGraphFuncListItem(QTreeWidgetItem* pParent, QTreeWidgetItem* pAfter) : QTreeWidgetItem(pParent, pAfter)/*,
    m_pFuncNode(nullptr)*/
{
}

CallGraphFuncListItem::CallGraphFuncListItem(/*const FunctionGraph::Node* pFuncNode*/) /*: m_pFuncNode(pFuncNode)
*/
{
}

CallGraphFuncListItem::~CallGraphFuncListItem()
{
}

qulonglong CallGraphFuncListItem::GetCountValue(int column) const
{
    qulonglong value;

    QVariant variant = data(column, Qt::DisplayRole);

    if (QVariant::ULongLong == variant.type())
    {
        value = variant.toULongLong();
    }
    else
    {
        value = 0ULL;
    }

    return value;
}

qulonglong CallGraphFuncListItem::AddCountValue(int column, qulonglong value)
{
    value += GetCountValue(column);

    if (0ULL != value)
    {
        setData(column, Qt::DisplayRole, QVariant(value));
    }

    return value;
}

void CallGraphFuncListItem::SetPercentageValue(int column, double numerator, double denominator)
{
    if (0ULL != numerator)
    {
        double percentage = numerator / denominator * 100.0;
        GT_ASSERT(percentage <= 100.0);
        setData(column, Qt::DisplayRole, QVariant(percentage));
    }
}


// ****************************
// Class CallGraphFuncList
// ***************************

CallGraphFuncList::CallGraphFuncList(QWidget* pParent)
    : QWidget()
{
    m_pFuncTable = nullptr;
    m_DeepSampleTotal = 0;
    m_pCallGraphTab = (SessionCallGraphView*) pParent;
    m_pWindowLabel = nullptr;
    m_lastSortColumn = -1;
    m_lastSortOrder = Qt::DescendingOrder;
    m_pEmptyTableMsgItem = nullptr;
}


void CallGraphFuncList::clear()
{
    m_pFuncTable->clear();
    m_DeepSampleTotal = 0;
}

void CallGraphFuncList::OnDblClicked(QTreeWidgetItem* pItem)
{
    if (nullptr != pItem)
    {
        CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(pItem);

        if (nullptr != pFuncListItem)
        {
            emit functionSelected(pFuncListItem->m_functionId);
        }
    }
}

void CallGraphFuncList::onSelectionChanged()
{
    if (AMDT_PROFILE_ALL_FUNCTIONS == GetSelectedFuncId())
    {
        CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(m_pFuncTable->currentItem());

        if (nullptr != pFuncListItem)
        {
            if (nullptr != pFuncListItem)
            {
                emit functionSelected(pFuncListItem->m_functionId);
            }
        }
    }
    else
    {
        emit functionSelected(m_selectedFuncId);
    }
}

void CallGraphFuncList::onFunctionClicked(QTreeWidgetItem* pItem)
{
    if (nullptr != pItem)
    {
        CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(pItem);

        if (nullptr != pFuncListItem)
        {
            emit functionSelected(pFuncListItem->m_functionId);
        }
    }
}

void CallGraphFuncList::sortIndicatorChanged(int col, Qt::SortOrder order)
{
    m_lastSortColumn = col;
    m_lastSortOrder = order;
}

void CallGraphFuncList::selectAFunction(AMDTFunctionId funcId)
{
    GT_IF_WITH_ASSERT(nullptr != m_pCallGraphTab && nullptr != m_pFuncTable)
    {
        if (nullptr != m_pCallGraphTab->m_pPathFuncTable)
        {
            m_pCallGraphTab->m_pPathFuncTable->blockSignals(true);
        }

        if (nullptr != m_pCallGraphTab->m_pFuncTable)
        {
            m_pCallGraphTab->m_pFuncTable->blockSignals(true);
        }

        if (nullptr != m_pCallGraphTab->m_pButterfly)
        {
            m_pCallGraphTab->m_pButterfly->blockSignals(true);
        }

        for (QTreeWidgetItemIterator itemIterator(m_pFuncTable); nullptr != *itemIterator; ++itemIterator)
        {
            CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(*itemIterator);

            if (funcId == pFuncListItem->m_functionId)
            {
                if (!pFuncListItem->isHidden())
                {
                    pFuncListItem->setSelected(true);
                    m_pFuncTable->scrollToItem(pFuncListItem);
                    m_selectedFuncId = funcId;
                }
                else
                {
                    pFuncListItem->setSelected(false);
                }
            }
            else
            {
                pFuncListItem->setSelected(false);
            }
        }

        if (nullptr != m_pCallGraphTab->m_pPathFuncTable)
        {
            m_pCallGraphTab->m_pPathFuncTable->blockSignals(false);
        }

        if (nullptr != m_pCallGraphTab->m_pFuncTable)
        {
            m_pCallGraphTab->m_pFuncTable->blockSignals(false);
        }

        if (nullptr != m_pCallGraphTab->m_pButterfly)
        {
            m_pCallGraphTab->m_pButterfly->blockSignals(false);
        }
    }
}

void CallGraphFuncList::onCurrentListItemChanged(QTreeWidgetItem* pItem)
{
    (void)(pItem); // unused
}

CallGraphFuncList::~CallGraphFuncList()
{
}

void CallGraphFuncList::sortByColumn(int column, Qt::SortOrder way)
{
    if ((column < CALLGRAPH_FUNCTION_NAME) ||
        (column >= CALLGRAPH_FUNCTION_OFFSET))
    {
        return;
    }

    m_pFuncTable->sortItems(column, way);
}

void CallGraphFuncList::Initialize()
{
    m_pFuncTable = new FunctionsTreeCtrl(m_pCallGraphTab, nullptr, false, CALLGRAPH_FUNCTION_OFFSET);


    acPercentItemDelegate* pDelegate = new acPercentItemDelegate;

    pDelegate->SetPercent(false);
    m_pFuncTable->setItemDelegate(pDelegate);

    m_pFuncTable->setUniformRowHeights(true);

    // Setup layout for this area of the splitter.
    QGridLayout* pLay = new QGridLayout();

    if (m_pWindowLabel != nullptr)
    {
        pLay->addWidget(m_pWindowLabel);
    }

    pLay->addWidget(m_pFuncTable);
    pLay->setContentsMargins(0, 0, 0, 0);
    setLayout(pLay);

    //default for all columns
    m_pFuncTable->header()->setSortIndicatorShown(true);
    m_pFuncTable->setSortingEnabled(true);
    m_pFuncTable->setAllColumnsShowFocus(true);  // full row select


    m_pFuncTable->header()->setSectionResizeMode(QHeaderView::Stretch);

    m_pFuncTable->headerItem()->setText(CALLGRAPH_FUNCTION_NAME, "Function");

    m_pFuncTable->header()->setSectionResizeMode(CALLGRAPH_FUNCTION_NAME, QHeaderView::Interactive);

    m_pFuncTable->setColumnWidth(CALLGRAPH_FUNCTION_NAME, 140);
    m_pFuncTable->headerItem()->setToolTip(CALLGRAPH_FUNCTION_NAME, CALL_GRAPH_FUNCTION_NAME_TOOLTIP);

    m_pFuncTable->headerItem()->setText(CALLGRAPH_FUNCTION_SELF_SAMPLES, "Self Samples");

    m_pFuncTable->header()->setSectionResizeMode(CALLGRAPH_FUNCTION_SELF_SAMPLES, QHeaderView::Interactive);

    m_pFuncTable->setColumnWidth(CALLGRAPH_FUNCTION_SELF_SAMPLES, 70);
    m_pFuncTable->headerItem()->setTextAlignment(CALLGRAPH_FUNCTION_SELF_SAMPLES, Qt::AlignLeft | Qt::AlignVCenter);
    m_pFuncTable->headerItem()->setToolTip(CALLGRAPH_FUNCTION_SELF_SAMPLES, CALL_GRAPH_FUNCTION_SELF_SAMPLES_TOOLTIP);

    m_pFuncTable->headerItem()->setText(CALLGRAPH_FUNCTION_PATH_CNT, "No. of Paths");

    m_pFuncTable->header()->setSectionResizeMode(CALLGRAPH_FUNCTION_PATH_CNT, QHeaderView::Interactive);

    m_pFuncTable->setColumnWidth(CALLGRAPH_FUNCTION_PATH_CNT, 75);
    m_pFuncTable->headerItem()->setTextAlignment(CALLGRAPH_FUNCTION_PATH_CNT, Qt::AlignLeft | Qt::AlignVCenter);
    m_pFuncTable->headerItem()->setToolTip(CALLGRAPH_FUNCTION_PATH_CNT, CALL_GRAPH_FUNCTION_PATH_CNT_TOOLTIP);

#ifdef CSS_FUNC_PATH_INFO
    m_pFuncTable->headerItem()->setText(CALLGRAPH_FUNCTION_PATH_SAMPLES, "Path Samples");

    m_pFuncTable->header()->setSectionResizeMode(CALLGRAPH_FUNCTION_PATH_SAMPLES, QHeaderView::Interactive);

    m_pFuncTable->setColumnWidth(CALLGRAPH_FUNCTION_PATH_SAMPLES, 80);
    m_pFuncTable->headerItem()->setTextAlignment(CALLGRAPH_FUNCTION_PATH_SAMPLES, Qt::AlignLeft | Qt::AlignVCenter);
    m_pFuncTable->headerItem()->setToolTip(CALLGRAPH_FUNCTION_PATH_SAMPLES, CALL_GRAPH_FUNCTION_PATH_SAMPLES_TOOLTIP);

    m_pFuncTable->headerItem()->setText(CALLGRAPH_FUNCTION_PATH_SAMPLES_PER_PATH, "Avg. samples per path");

    m_pFuncTable->header()->setSectionResizeMode(CALLGRAPH_FUNCTION_PATH_SAMPLES_PER_PATH, QHeaderView::Interactive);

    m_pFuncTable->setColumnWidth(CALLGRAPH_FUNCTION_PATH_SAMPLES_PER_PATH, 125);
    m_pFuncTable->headerItem()->setTextAlignment(CALLGRAPH_FUNCTION_PATH_SAMPLES_PER_PATH, Qt::AlignLeft | Qt::AlignVCenter);
    m_pFuncTable->headerItem()->setToolTip(CALLGRAPH_FUNCTION_PATH_SAMPLES_PER_PATH, CALL_GRAPH_FUNCTION_PATH_SAMPLES_PER_PATH_TOOLTIP);
#endif

    m_pFuncTable->headerItem()->setText(CALLGRAPH_FUNCTION_DEEP_SAMPLES, "Deep Samples");

    m_pFuncTable->header()->setSectionResizeMode(CALLGRAPH_FUNCTION_DEEP_SAMPLES, QHeaderView::Interactive);

    m_pFuncTable->setColumnWidth(CALLGRAPH_FUNCTION_DEEP_SAMPLES, 85);
    m_pFuncTable->headerItem()->setTextAlignment(CALLGRAPH_FUNCTION_DEEP_SAMPLES, Qt::AlignLeft | Qt::AlignVCenter);
    m_pFuncTable->headerItem()->setToolTip(CALLGRAPH_FUNCTION_DEEP_SAMPLES, CALL_GRAPH_FUNCTION_DEEP_SAMPLES_TOOLTIP);

    m_pFuncTable->headerItem()->setText(CALLGRAPH_FUNCTION_DEEP_SAMPLE_PERCENTAGE, "% of Deep Samples");

    m_pFuncTable->header()->setSectionResizeMode(CALLGRAPH_FUNCTION_DEEP_SAMPLE_PERCENTAGE, QHeaderView::Interactive);

    m_pFuncTable->setColumnWidth(CALLGRAPH_FUNCTION_DEEP_SAMPLE_PERCENTAGE, 125);
    m_pFuncTable->headerItem()->setTextAlignment(CALLGRAPH_FUNCTION_DEEP_SAMPLE_PERCENTAGE, Qt::AlignLeft | Qt::AlignVCenter);
    m_pFuncTable->headerItem()->setToolTip(CALLGRAPH_FUNCTION_DEEP_SAMPLE_PERCENTAGE, CALL_GRAPH_FUNCTION_DEEP_SAMPLE_PERCENTAGE_TOOLTIP);

    acTablePercentItemDelegate* pPercentDelegate = new acTablePercentItemDelegate();

    pPercentDelegate->SetOwnerTree(m_pFuncTable);

    m_pFuncTable->setItemDelegateForColumn(CALLGRAPH_FUNCTION_DEEP_SAMPLE_PERCENTAGE, pPercentDelegate);

    m_pFuncTable->headerItem()->setText(CALLGRAPH_FUNCTION_SOURCE_FILE, "Source File");

    m_pFuncTable->header()->setSectionResizeMode(CALLGRAPH_FUNCTION_SOURCE_FILE, QHeaderView::Interactive);

    m_pFuncTable->setColumnWidth(CALLGRAPH_FUNCTION_SOURCE_FILE, 85);
    m_pFuncTable->headerItem()->setToolTip(CALLGRAPH_FUNCTION_SOURCE_FILE, CALL_GRAPH_FUNCTION_SOURCE_FILE_TOOLTIP);

    m_pFuncTable->headerItem()->setText(CALLGRAPH_FUNCTION_MODULE, "Module");

    m_pFuncTable->header()->setSectionResizeMode(CALLGRAPH_FUNCTION_MODULE, QHeaderView::Interactive);

    m_pFuncTable->setColumnWidth(CALLGRAPH_FUNCTION_MODULE, 100);
    m_pFuncTable->headerItem()->setToolTip(CALLGRAPH_FUNCTION_MODULE, CALL_GRAPH_FUNCTION_MODULE_TOOLTIP);

#ifdef CSS_FUNC_PATH_INFO
    m_pFuncTable->sortByColumn(CALLGRAPH_FUNCTION_PATH_SAMPLES_PER_PATH, Qt::DescendingOrder);
#endif

    QObject::connect(m_pFuncTable, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
                     SLOT(OnDblClicked(QTreeWidgetItem*)));
    QObject::connect(m_pFuncTable, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
                     SLOT(OnDblClicked(QTreeWidgetItem*)));

    QObject::connect(m_pFuncTable, SIGNAL(itemSelectionChanged()), this,
                     SLOT(onSelectionChanged()));

    //allow multi-select
    m_pFuncTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    QObject::connect(m_pFuncTable, SIGNAL(itemSelectionChanged()), this,
                     SLOT(onSelectionChanged()));

    // Connect the sort signal:
    bool rc = connect(m_pFuncTable->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(sortIndicatorChanged(int, Qt::SortOrder)));
    GT_ASSERT(rc);
}



QString CallGraphFuncList::GetFileNameEntry(const gtString& srcFile, int srcFileLine)
{
    osFilePath filePath(srcFile);
    gtString fileName, fileExt;
    filePath.getFileName(fileName);
    QString fileNameEntry("");

    if (!fileName.isEmpty())
    {
        filePath.getFileExtension(fileExt);
        gtString FullFileName = fileName.append(L".").append(fileExt);
        fileNameEntry = QString("%1(%2)").arg(acGTStringToQString(FullFileName)).arg(srcFileLine);
    }

    return fileNameEntry;
}

QString CallGraphFuncList::GetModuleNameEntry(const gtString& moduleName)
{
    osFilePath modulePath(moduleName);
    gtString modName;
    gtString modExt;
    gtString moduleFullName(L"");

    modulePath.getFileName(modName);

    if (!modName.isEmpty())
    {
        modulePath.getFileExtension(modExt);

        moduleFullName = modName.append(L".").append(modExt);
    }

    return acGTStringToQString(moduleFullName);
}
CallGraphFuncListItem* CallGraphFuncList::AddFuncListItem(AMDTFunctionId functionId,
                                                          gtString functionName,
                                                          gtUInt64 totalSelfSamples,
                                                          gtUInt64 totalDeepSamples,
                                                          double   deepSamplesPerc,
                                                          gtUInt32 pathCount,
                                                          gtString srcFile,
                                                          gtString moduleName,
                                                          gtUInt32 srcFileLine,
                                                          AMDTUInt32 moduleId)
{
    CallGraphFuncListItem* pFuncListItem = new CallGraphFuncListItem;
    m_pFuncTable->addTopLevelItem(pFuncListItem);

    // set module id
    pFuncListItem->m_moduleId = moduleId;

    //Function Name
    pFuncListItem->setText(CALLGRAPH_FUNCTION_NAME, acGTStringToQString(functionName));
    pFuncListItem->m_functionName = functionName;

    pFuncListItem->m_functionId = functionId;

    // Self counts
    if (0ULL != totalSelfSamples)
    {
        pFuncListItem->setData(CALLGRAPH_FUNCTION_SELF_SAMPLES, Qt::DisplayRole, QVariant(static_cast<qulonglong>(totalSelfSamples)));
        pFuncListItem->setTextAlignment(CALLGRAPH_FUNCTION_SELF_SAMPLES, Qt::AlignLeft | Qt::AlignVCenter);
    }
    else
    {
        pFuncListItem->setText(CALLGRAPH_FUNCTION_SELF_SAMPLES, "");
    }

    pFuncListItem->m_selfSample = totalSelfSamples;

    //Deep counts
    if (0ULL != totalDeepSamples)
    {
        pFuncListItem->setData(CALLGRAPH_FUNCTION_DEEP_SAMPLES, Qt::DisplayRole, QVariant(static_cast<qulonglong>(totalDeepSamples)));
        pFuncListItem->setTextAlignment(CALLGRAPH_FUNCTION_DEEP_SAMPLES, Qt::AlignLeft | Qt::AlignVCenter);
    }
    else
    {
        pFuncListItem->setText(CALLGRAPH_FUNCTION_DEEP_SAMPLES, "");
    }

    //Deep SamplePercent
    pFuncListItem->setData(CALLGRAPH_FUNCTION_DEEP_SAMPLE_PERCENTAGE, Qt::DisplayRole, QVariant(deepSamplesPerc));

    //Path Count
    if (0U != pathCount)
    {
        pFuncListItem->setData(CALLGRAPH_FUNCTION_PATH_CNT, Qt::DisplayRole, QVariant(pathCount));
        pFuncListItem->setTextAlignment(CALLGRAPH_FUNCTION_PATH_CNT, Qt::AlignLeft | Qt::AlignVCenter);
    }
    else
    {
        pFuncListItem->setText(CALLGRAPH_FUNCTION_PATH_CNT, "");
    }

    //Source File
    if (!srcFile.isEmpty())
    {
        pFuncListItem->setToolTip(CALLGRAPH_FUNCTION_SOURCE_FILE, acGTStringToQString(srcFile));
    }

    QString fileNameEntry = GetFileNameEntry(srcFile, srcFileLine);
    pFuncListItem->setText(CALLGRAPH_FUNCTION_SOURCE_FILE, fileNameEntry);

    //Module Name
    QString modName = GetModuleNameEntry(moduleName);
    pFuncListItem->m_moduleName = moduleName;

    if (!moduleName.isEmpty())
    {
        pFuncListItem->setToolTip(CALLGRAPH_FUNCTION_MODULE, acGTStringToQString(moduleName));
    }

    pFuncListItem->setText(CALLGRAPH_FUNCTION_MODULE, modName);

    return pFuncListItem;
}



bool SessionCallGraphView::ShowParentChild(AMDTFunctionId functionId)
{
    bool ret = false;

    GT_IF_WITH_ASSERT((nullptr != m_pProfDataRdr) &&
                      (m_pFuncTable != nullptr) &&
                      (m_pDisplayFilter != nullptr))
    {
        if (nullptr != m_pButterfly)
        {
            m_pButterfly->clear();

            AMDTProfileFunctionData  functionData;
            ret = m_pProfDataRdr->GetFunctionData(functionId,
                                                  m_selectedPID,
                                                  AMDT_PROFILE_ALL_THREADS,
                                                  functionData);

            if (ret)
            {
                m_pButterfly->setWindowTitle(tr("Immediate Parents and Children of Function: <b>") + acGTStringToQString(functionData.m_functionInfo.m_name) + "</b>");
            }

            m_pButterfly->SetChildrenFunction(m_pProfDataRdr,
                                              !m_pDisplayFilter->IsSystemModuleIgnored(),
                                              m_selectedPID,
                                              m_selectedCounter, functionId);
            m_pButterfly->SetParentsFunction(m_pProfDataRdr,
                                             !m_pDisplayFilter->IsSystemModuleIgnored(),
                                             m_selectedPID,
                                             m_selectedCounter, functionId);
        }
    }

    return ret;
}

void CallGraphFuncList::GetFunctionCount(std::pair<int, int>& sysDllCount,
                                         const std::shared_ptr<DisplayFilter> pDisplayFilter,
                                         std::shared_ptr<cxlProfileDataReader> pProfDataRdr,
                                         AMDTUInt32 counterId,
                                         AMDTUInt32 processId)
{
    if (nullptr != pDisplayFilter)
    {
        AMDTCallGraphFunctionVec callGraphFuncs;

        // save previous system module state
        bool origState = pDisplayFilter->IsSystemModuleIgnored();


        pProfDataRdr->GetCallGraphFunctions(processId, counterId, callGraphFuncs);
        AMDTUInt32 shown = callGraphFuncs.size();
        AMDTUInt32 total;

        if (origState == false)
        {
            total = callGraphFuncs.size();
        }
        else
        {
            callGraphFuncs.clear();
            pDisplayFilter->setIgnoreSysDLL(!origState);
            pDisplayFilter->SetReportConfig();

            pProfDataRdr->GetCallGraphFunctions(processId, counterId, callGraphFuncs);
            total = callGraphFuncs.size();

            //restore orignal state
            pDisplayFilter->setIgnoreSysDLL(origState);
            pDisplayFilter->SetReportConfig();
        }

        sysDllCount = std::make_pair(total, shown);
    }


}

bool CallGraphFuncList::FillDisplayFuncList(std::shared_ptr<cxlProfileDataReader> pProfDataRdr,
                                            std::shared_ptr<DisplayFilter> pDisplayFilter,
                                            AMDTUInt32 counterId,
                                            AMDTUInt32 processId,
                                            AMDTFunctionId& funcIdMaxSamples)
{
    bool ret = false;

    GT_IF_WITH_ASSERT((nullptr != pProfDataRdr) &&
                      (m_pFuncTable != nullptr) &&
                      (pDisplayFilter != nullptr))
    {
        m_pFuncTable->blockSignals(true);
        clear();
        m_pFuncTable->setSortingEnabled(false);
        gtUInt64 masSampleValue = 0;

        // first element contain shown and second total
        std::pair<int, int> sysDllCount;
        GetFunctionCount(sysDllCount, pDisplayFilter, pProfDataRdr,
                         counterId, processId);

        SetFunctionNameHeader(sysDllCount.first, sysDllCount.second);

        AMDTCallGraphFunctionVec callGraphFuncs;
        ret = pProfDataRdr->GetCallGraphFunctions(processId, counterId, callGraphFuncs);

        m_funcIdVec.clear();
        m_FunctionIdSampleMap.clear();

        for (const auto& callGraphFunc : callGraphFuncs)
        {
            AddFuncListItem(callGraphFunc.m_functionInfo.m_functionId,
                            callGraphFunc.m_functionInfo.m_name,
                            callGraphFunc.m_totalSelfSamples,
                            callGraphFunc.m_totalDeepSamples,
                            callGraphFunc.m_deepSamplesPerc,
                            callGraphFunc.m_pathCount,
                            callGraphFunc.m_srcFile,
                            callGraphFunc.m_functionInfo.m_modulePath,
                            callGraphFunc.m_srcFileLine,
                            callGraphFunc.m_functionInfo.m_moduleId);

            if (masSampleValue < callGraphFunc.m_totalSelfSamples)
            {
                masSampleValue      = callGraphFunc.m_totalSelfSamples;
                funcIdMaxSamples    = callGraphFunc.m_functionInfo.m_functionId;
            }

            m_funcIdVec.push_back(callGraphFunc.m_functionInfo.m_functionId);
            m_FunctionIdSampleMap.insert(std::make_pair(callGraphFunc.m_functionInfo.m_functionId,
                                                        callGraphFunc.m_totalSelfSamples ? true : false));

        }

        m_pFuncTable->setSortingEnabled(true);
        m_pFuncTable->sortByColumn(CALLGRAPH_FUNCTION_DEEP_SAMPLE_PERCENTAGE, Qt::DescendingOrder);

        // resize CALLGRAPH_FUNCTION_NAME column
        ResizeFunctionNameColumn();

        bool isSortIndShown = m_pFuncTable->header()->isSortIndicatorShown();
        // disable the sort indicator - for more accurate resizing (header size will not include sort indicator size)
        m_pFuncTable->header()->setSortIndicatorShown(false);

        m_pFuncTable->resizeColumnToContents(CALLGRAPH_FUNCTION_SELF_SAMPLES);
        m_pFuncTable->resizeColumnToContents(CALLGRAPH_FUNCTION_DEEP_SAMPLES);
        m_pFuncTable->resizeColumnToContents(CALLGRAPH_FUNCTION_PATH_CNT);
        m_pFuncTable->resizeColumnToContents(CALLGRAPH_FUNCTION_DEEP_SAMPLE_PERCENTAGE);
        m_pFuncTable->resizeColumnToContents(CALLGRAPH_FUNCTION_SOURCE_FILE);
        m_pFuncTable->resizeColumnToContents(CALLGRAPH_FUNCTION_MODULE);

        // set back the sort indicator
        m_pFuncTable->header()->setSortIndicatorShown(isSortIndShown);

        // Unblock the signals.
        m_pFuncTable->blockSignals(false);

        InitEmptyTableRow();
        //HandleDisplayEmptyTableItem(shownFuncCount);

        ret = true;
    }

    return ret;
}



void CallGraphFuncList::ResizeFunctionNameColumn()
{
    GT_IF_WITH_ASSERT(m_pFuncTable)
    {
        m_pFuncTable->resizeColumnToContents(CALLGRAPH_FUNCTION_NAME);

        // if function name column size is more then 40% of table width - change it
        int maxColumnSize = m_pFuncTable->width() * MAX_NAME_COLUMN_SIZE_OF_WINDOW;

        if (m_pFuncTable->columnWidth(CALLGRAPH_FUNCTION_NAME) > maxColumnSize)
        {
            m_pFuncTable->setColumnWidth(CALLGRAPH_FUNCTION_NAME, maxColumnSize);
        }
    }
}

void CallGraphFuncList::setWindowTitle(const QString& title)  // Overload this
{
    if (nullptr == m_pWindowLabel)
    {
        m_pWindowLabel = new QLabel;
    }

    m_pWindowLabel->setText(title);
}

CallGraphFuncListItem* CallGraphFuncList::getSelectedFunction()
{
    QList<QTreeWidgetItem*> lst = m_pFuncTable->selectedItems();

    if (1 != lst.size())
    {
        return nullptr;
    }

    return (CallGraphFuncListItem*) lst.at(0);
}

bool CallGraphFuncList::isFuncHidden(const FunctionGraph::Node& funcNode)
{
    bool hidden = false;

    if (nullptr != m_pFuncTable)
    {
        for (QTreeWidgetItemIterator itemIterator(m_pFuncTable); nullptr != *itemIterator; ++itemIterator)
        {
            CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(*itemIterator);

            if (&funcNode == pFuncListItem->m_pFuncNode)
            {
                hidden = pFuncListItem->isHidden();
                break;
            }
        }
    }

    return hidden;
}

CallGraphFuncListItem* CallGraphFuncList::HideSystemModules(CpuProfileCss& css)
{
    GT_UNREFERENCED_PARAMETER(css);
    CallGraphFuncListItem* pSelectedItem = nullptr;

    if (nullptr != m_pFuncTable)
    {
        gtUInt64 selfCountMax = 0ULL;
        gtUInt64 deepCountMax = 0ULL;
        int totalFuncCount = 0;
        int shownFuncCount = 0;

        for (int itemIndex = 0, itemsCount = m_pFuncTable->topLevelItemCount(); itemIndex < itemsCount; ++itemIndex)
        {
            CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(m_pFuncTable->topLevelItem(itemIndex));

            if (nullptr != pFuncListItem && nullptr != pFuncListItem->m_pFuncNode)
            {
                const FunctionGraph::Node* pFuncNode = pFuncListItem->m_pFuncNode;

                const CssFunctionMetadata* pMetadata = static_cast<const CssFunctionMetadata*>(pFuncNode->m_val);

                bool hidden = false;

                if (nullptr != pMetadata->m_pModule && pMetadata->m_pModule->isSystemModule())
                {
                    hidden = true;
                }

                pFuncListItem->setHidden(hidden);

                if (!hidden)
                {
                    shownFuncCount++;

                    gtUInt64 selfCount = pFuncListItem->GetCountValue(CALLGRAPH_FUNCTION_SELF_SAMPLES);

                    if (selfCountMax < selfCount)
                    {
                        selfCountMax = selfCount;
                        pSelectedItem = pFuncListItem;
                    }
                    else
                    {
                        gtUInt64 deepCount = pFuncListItem->GetCountValue(CALLGRAPH_FUNCTION_DEEP_SAMPLES);

                        if (0 == selfCountMax && deepCountMax < deepCount)
                        {
                            deepCountMax = deepCount;
                            pSelectedItem = pFuncListItem;
                        }
                    }
                }

                totalFuncCount++;
            }
        }

        SetFunctionNameHeader(totalFuncCount, shownFuncCount);

        CallGraphFuncListItem* pCurrentSelectedItem = getSelectedFunction();

        if (nullptr != pCurrentSelectedItem)
        {
            if (!pCurrentSelectedItem->isHidden())
            {
                pSelectedItem = pCurrentSelectedItem;
            }
        }

        HandleDisplayEmptyTableItem(shownFuncCount);
    }

    return pSelectedItem;
}

CallGraphFuncListItem* CallGraphFuncList::UnhideAllItems()
{
    CallGraphFuncListItem* pSelectedItem = nullptr;

    if (nullptr != m_pFuncTable)
    {
        pSelectedItem = getSelectedFunction();
        const bool noneSelected = (nullptr == pSelectedItem);

        gtUInt64 selfCountMax = 0ULL;
        gtUInt64 deepCountMax = 0ULL;
        int totalFuncCount = 0;

        for (int itemIndex = 0, itemsCount = m_pFuncTable->topLevelItemCount(); itemIndex < itemsCount; ++itemIndex)
        {
            CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(m_pFuncTable->topLevelItem(itemIndex));

            if (nullptr != pFuncListItem && nullptr != pFuncListItem->m_pFuncNode)
            {
                pFuncListItem->setHidden(false);

                if (noneSelected)
                {
                    gtUInt64 selfCount = pFuncListItem->GetCountValue(CALLGRAPH_FUNCTION_SELF_SAMPLES);

                    if (selfCountMax < selfCount)
                    {
                        selfCountMax = selfCount;
                        pSelectedItem = pFuncListItem;
                    }
                    else
                    {
                        gtUInt64 deepCount = pFuncListItem->GetCountValue(CALLGRAPH_FUNCTION_DEEP_SAMPLES);

                        if (0 == selfCountMax && deepCountMax < deepCount)
                        {
                            deepCountMax = deepCount;
                            pSelectedItem = pFuncListItem;
                        }
                    }
                }

                totalFuncCount++;
            }
        }

        SetFunctionNameHeader(totalFuncCount, totalFuncCount);

        HandleDisplayEmptyTableItem(totalFuncCount);
    }

    return pSelectedItem;
}

void CallGraphFuncList::SetFunctionNameHeader(int totalFuncCount, int shownFuncCount)
{
    if (0 != totalFuncCount)
    {
        QString countStr = QString("Function (" + QVariant(totalFuncCount).toString() + " functions");

        if (totalFuncCount == shownFuncCount)
        {
            countStr +=  QString(")");
        }
        else
        {
            countStr += QString(", " + QVariant(shownFuncCount).toString() + " shown)");
        }

        m_pFuncTable->headerItem()->setText(CALLGRAPH_FUNCTION_NAME, countStr);
    }
    else
    {
        m_pFuncTable->headerItem()->setText(CALLGRAPH_FUNCTION_NAME, QString("Function"));
    }
}

void CallGraphFuncList::HandleDisplayEmptyTableItem(int numberOfRow)
{
    if (numberOfRow)
    {
        m_pEmptyTableMsgItem->setHidden(true);
        // resize the function name column - up to max size
        ResizeFunctionNameColumn();
    }
    else
    {
        m_pEmptyTableMsgItem->setHidden(false);
        // resize name column to full
        GT_IF_WITH_ASSERT(m_pFuncTable)
        {
            m_pFuncTable->resizeColumnToContents(CALLGRAPH_FUNCTION_NAME);
        }
    }
}

void CallGraphFuncList::InitEmptyTableRow()
{
    // Create new item with "empty table" message and insert into table:
    m_pEmptyTableMsgItem = new CallGraphFuncListItem;
    m_pFuncTable->addTopLevelItem(m_pEmptyTableMsgItem);

    QString emptyTableMessage = CP_emptyTableMessage;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pCallGraphTab != nullptr)
    {
        // Check what is the CSS scope for this session, and suggest to change it, since there are no samples:
        afApplicationTreeItemData* pDisplayedData = m_pCallGraphTab->m_pDisplayedSessionItemData;
        GT_IF_WITH_ASSERT(pDisplayedData != nullptr)
        {
            CPUSessionTreeItemData* pSessionSettings = qobject_cast<CPUSessionTreeItemData*>(pDisplayedData->extendedItemData());
            GT_IF_WITH_ASSERT(pSessionSettings != nullptr)
            {
                QString curretCSSScope = CP_STR_cpuProfileProjectSettingsCallStackUserSpace;

                if (pSessionSettings->m_cssScope == CP_CSS_SCOPE_KERNEL)
                {
                    curretCSSScope = CP_STR_cpuProfileProjectSettingsCallStackKernelSpace;
                }
                else if (pSessionSettings->m_cssScope == CP_CSS_SCOPE_ALL)
                {
                    curretCSSScope = CP_STR_cpuProfileProjectSettingsCallStackUserKernelSpaces;
                }

                emptyTableMessage = QString(CP_emptyCallGraphTableMessage).arg(curretCSSScope);
            }
        }
    }

    m_pEmptyTableMsgItem->setText(CALLGRAPH_FUNCTION_NAME, emptyTableMessage);

    // in init hide the row
    m_pEmptyTableMsgItem->setHidden(true);
}

// ****************************
// Class CallGraphButterfly
// ***************************

CallGraphButterfly::CallGraphButterfly(QWidget* pParent)
    : QWidget(pParent), m_pLastTree(nullptr), m_pParentsTreeControl(nullptr), m_pChildrenTreeControl(nullptr), m_pWindowLabel(nullptr)
{
    m_pCallGraphTab = (SessionCallGraphView*) pParent;
}


CallGraphButterfly::~CallGraphButterfly()
{
}

bool CallGraphButterfly::Initialize()
{
    m_pParentsTreeControl = new FunctionsTreeCtrl(m_pCallGraphTab, nullptr, false, CALLGRAPH_BUTTERFLY_OFFSET);


    m_pChildrenTreeControl = new FunctionsTreeCtrl(m_pCallGraphTab, nullptr, false, CALLGRAPH_BUTTERFLY_OFFSET);


    m_pWindowLabel = new QLabel;


    acPercentItemDelegate* pDelegate = new acPercentItemDelegate;

    pDelegate->SetPercent(false);

    m_pChildrenTreeControl->setItemDelegate(pDelegate);
    m_pParentsTreeControl->setItemDelegate(pDelegate);

    m_pChildrenTreeControl->setAlternatingRowColors(false);
    m_pParentsTreeControl->setAlternatingRowColors(false);

    setWindowTitle("Parents/Children");

    bool rc = connect(m_pParentsTreeControl, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(onFunctionClicked(QTreeWidgetItem*)));
    GT_ASSERT(rc);

    rc = connect(m_pChildrenTreeControl, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(onFunctionClicked(QTreeWidgetItem*)));
    GT_ASSERT(rc);

    rc = connect(m_pChildrenTreeControl->header(), SIGNAL(sectionHandleDoubleClicked(int)), m_pChildrenTreeControl, SLOT(update()));
    GT_ASSERT(rc);
    rc = connect(m_pParentsTreeControl->header(), SIGNAL(sectionHandleDoubleClicked(int)), m_pParentsTreeControl, SLOT(update()));
    GT_ASSERT(rc);

    m_pChildrenTreeControl->header()->setSortIndicatorShown(true);
    m_pChildrenTreeControl->setSortingEnabled(true);
    m_pChildrenTreeControl->headerItem()->setTextAlignment(CALLGRAPH_BUTTERFLY_SAMPLES, Qt::AlignLeft | Qt::AlignVCenter);
    m_pChildrenTreeControl->header()->setSortIndicatorShown(true);
    m_pChildrenTreeControl->setSortingEnabled(true);

    m_pChildrenTreeControl->headerItem()->setText(CALLGRAPH_BUTTERFLY_FUNCTION, "Self + children");
    m_pChildrenTreeControl->headerItem()->setText(CALLGRAPH_BUTTERFLY_SAMPLES, "Samples");
    m_pChildrenTreeControl->headerItem()->setText(CALLGRAPH_BUTTERFLY_PERCENTAGE, "% of samples");
    m_pChildrenTreeControl->headerItem()->setText(CALLGRAPH_BUTTERFLY_MODULE, "Module");

    m_pChildrenTreeControl->setColumnWidth(CALLGRAPH_BUTTERFLY_FUNCTION, CALL_GRAPH_BUTTERFLY_UNIFORM_WIDTH * 2);
    m_pChildrenTreeControl->setColumnWidth(CALLGRAPH_BUTTERFLY_SAMPLES, CALL_GRAPH_BUTTERFLY_UNIFORM_WIDTH);
    m_pChildrenTreeControl->sortByColumn(CALLGRAPH_BUTTERFLY_SAMPLES, Qt::DescendingOrder);

    acTablePercentItemDelegate* pPercentDelegate1 = new acTablePercentItemDelegate();

    pPercentDelegate1->SetOwnerTree(m_pChildrenTreeControl);
    m_pChildrenTreeControl->setItemDelegateForColumn(CALLGRAPH_BUTTERFLY_PERCENTAGE, pPercentDelegate1);

    m_pChildrenTreeControl->headerItem()->setToolTip(CALLGRAPH_BUTTERFLY_FUNCTION, CALL_GRAPH_BUTTERFLY_FUNCTION_TOOLTIP);
    m_pChildrenTreeControl->headerItem()->setToolTip(CALLGRAPH_BUTTERFLY_SAMPLES, CALL_GRAPH_BUTTERFLY_SAMPLES_TOOLTIP);
    m_pChildrenTreeControl->headerItem()->setToolTip(CALLGRAPH_BUTTERFLY_PERCENTAGE, CALL_GRAPH_BUTTERFLY_PERCENTAGE_TOOLTIP);
    m_pChildrenTreeControl->headerItem()->setToolTip(CALLGRAPH_BUTTERFLY_MODULE, CALL_GRAPH_FUNCTION_MODULE_TOOLTIP);

    m_pParentsTreeControl->header()->setSortIndicatorShown(true);
    m_pParentsTreeControl->setSortingEnabled(true);
    m_pParentsTreeControl->headerItem()->setTextAlignment(CALLGRAPH_BUTTERFLY_SAMPLES, Qt::AlignLeft | Qt::AlignVCenter);
    m_pParentsTreeControl->header()->setSortIndicatorShown(true);
    m_pParentsTreeControl->setSortingEnabled(true);

    m_pParentsTreeControl->setColumnWidth(CALLGRAPH_BUTTERFLY_FUNCTION, CALL_GRAPH_BUTTERFLY_UNIFORM_WIDTH * 2);
    m_pParentsTreeControl->setColumnWidth(CALLGRAPH_BUTTERFLY_SAMPLES, CALL_GRAPH_BUTTERFLY_UNIFORM_WIDTH);
    m_pParentsTreeControl->sortByColumn(CALLGRAPH_BUTTERFLY_SAMPLES, Qt::DescendingOrder);

    acTablePercentItemDelegate* pPercentDelegate2 = new acTablePercentItemDelegate();

    pPercentDelegate2->SetOwnerTree(m_pParentsTreeControl);
    m_pParentsTreeControl->setItemDelegateForColumn(CALLGRAPH_BUTTERFLY_PERCENTAGE, pPercentDelegate2);

    m_pParentsTreeControl->headerItem()->setToolTip(CALLGRAPH_BUTTERFLY_FUNCTION, CALL_GRAPH_BUTTERFLY_FUNCTION_TOOLTIP);
    m_pParentsTreeControl->headerItem()->setToolTip(CALLGRAPH_BUTTERFLY_SAMPLES, CALL_GRAPH_BUTTERFLY_SAMPLES_TOOLTIP);
    m_pParentsTreeControl->headerItem()->setToolTip(CALLGRAPH_BUTTERFLY_PERCENTAGE, CALL_GRAPH_BUTTERFLY_PERCENTAGE_TOOLTIP);
    m_pParentsTreeControl->headerItem()->setToolTip(CALLGRAPH_BUTTERFLY_MODULE, CALL_GRAPH_FUNCTION_MODULE_TOOLTIP);

    m_pParentsTreeControl->headerItem()->setText(CALLGRAPH_BUTTERFLY_FUNCTION, "Parents");
    m_pParentsTreeControl->headerItem()->setText(CALLGRAPH_BUTTERFLY_SAMPLES, "Samples");
    m_pParentsTreeControl->headerItem()->setText(CALLGRAPH_BUTTERFLY_PERCENTAGE, "% of samples");
    m_pParentsTreeControl->headerItem()->setText(CALLGRAPH_BUTTERFLY_MODULE, "Module");

    //allow multi-select
    m_pChildrenTreeControl->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pParentsTreeControl->setSelectionMode(QAbstractItemView::ExtendedSelection);


    rc = connect(m_pChildrenTreeControl, SIGNAL(itemSelectionChanged()), this, SLOT(onSelectionChanged()));
    GT_ASSERT(rc);

    rc = connect(m_pChildrenTreeControl, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(onCurrentListItemChanged(QTreeWidgetItem*)));
    GT_ASSERT(rc);

    rc = connect(m_pParentsTreeControl, SIGNAL(itemSelectionChanged()), this, SLOT(onSelectionChanged()));
    GT_ASSERT(rc);

    rc = connect(m_pParentsTreeControl, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(onCurrentListItemChanged(QTreeWidgetItem*)));
    GT_ASSERT(rc);

    QGridLayout* pLayout = new QGridLayout;
    pLayout->addWidget(m_pWindowLabel, 0, 0, 1, 2);
    pLayout->addWidget(m_pParentsTreeControl, 1, 0 , 1, 1);
    pLayout->addWidget(m_pChildrenTreeControl, 1, 1, 1, 1);

    this->setLayout(pLayout);

    return true;
}


void CallGraphButterfly::onSelectionChanged()
{

}

void CallGraphButterfly::onCurrentListItemChanged(QTreeWidgetItem* pItem)
{
    if (nullptr != pItem)
    {
        m_pLastTree = pItem->treeWidget();
    }
}

void CallGraphButterfly::clear()
{
    setWindowTitle("No function selected");
    m_pParentsTreeControl->clear();
    m_pChildrenTreeControl->clear();
}



void CallGraphButterfly::SetParentsFunction(std::shared_ptr<cxlProfileDataReader> pProfDataRdr,
                                            bool displaySystemModule,
                                            AMDTUInt32 processId,
                                            AMDTUInt32 counterId,
                                            AMDTFunctionId  funcId)
{
    displaySystemModule = displaySystemModule;
    m_pParentsTreeControl->blockSignals(true);
    m_pParentsTreeControl->clear();

    AMDTCallGraphFunctionVec cgFuncs;
    pProfDataRdr->GetCallGraphFunctions(processId, counterId, cgFuncs);

    AMDTCallGraphFunctionVec parents;
    AMDTCallGraphFunctionVec children;

    pProfDataRdr->GetCallGraphFunctionInfo(processId, funcId, parents, children);
    double totalDeepCount = 0;

    if (!cgFuncs.empty())
    {
        totalDeepCount = cgFuncs.at(0).m_totalDeepSamples;
    }

    const bool noSamples = (0ULL == totalDeepCount);
    double systemSamplesCount = 0ULL;

    for (const auto& parent : parents)
    {
        AddTopLevelItem(*m_pParentsTreeControl,
                        parent.m_functionInfo.m_modulePath,
                        parent.m_functionInfo.m_moduleId,
                        parent.m_functionInfo.m_name,
                        parent.m_functionInfo.m_functionId,
                        parent.m_totalDeepSamples,
                        parent.m_totalDeepSamples,
                        parent.m_deepSamplesPerc,
                        noSamples);

    }

    if (parents.empty())
    {
        AddTopLevelItem(*m_pParentsTreeControl, L"", AMDT_PROFILE_ALL_MODULES, L"[system modules]", AMDT_PROFILE_ALL_FUNCTIONS,
                        systemSamplesCount, systemSamplesCount, totalDeepCount, noSamples);
    }

    bool isSortIndShown = m_pParentsTreeControl->header()->isSortIndicatorShown();
    // disable the sort indicator - for more accurate resizing (header size will not include sort indicator size)
    m_pParentsTreeControl->header()->setSortIndicatorShown(false);

    m_pParentsTreeControl->resizeColumnToContents(CALLGRAPH_BUTTERFLY_SAMPLES);
    m_pParentsTreeControl->resizeColumnToContents(CALLGRAPH_BUTTERFLY_PERCENTAGE);

    // set SortIndicatorShown back
    m_pParentsTreeControl->header()->setSortIndicatorShown(isSortIndShown);

    // Unblock the signals.
    m_pParentsTreeControl->blockSignals(false);

}

void CallGraphButterfly::SetChildrenFunction(std::shared_ptr<cxlProfileDataReader> pProfDataRdr,
                                             bool displaySystemModule,
                                             AMDTUInt32 processId,
                                             AMDTUInt32 counterId,
                                             AMDTFunctionId  funcId)
{
    displaySystemModule = displaySystemModule;
    m_pChildrenTreeControl->blockSignals(true);
    m_pChildrenTreeControl->clear();

    AMDTCallGraphFunctionVec cgFuncs;
    pProfDataRdr->GetCallGraphFunctions(processId, counterId, cgFuncs);

    AMDTCallGraphFunctionVec parents;
    AMDTCallGraphFunctionVec childrens;

    pProfDataRdr->GetCallGraphFunctionInfo(processId, funcId, parents, childrens);
    double totalDeepCount = 0;

    if (!cgFuncs.empty())
    {
        totalDeepCount = cgFuncs.at(0).m_totalDeepSamples;
    }

    const bool noSamples = (0ULL == totalDeepCount);
    //qulonglong systemSamplesCount = 0ULL;

    for (const auto& children : childrens)
    {
        gtString functionName = children.m_functionInfo.m_name;
        AMDTFunctionId functionId = children.m_functionInfo.m_functionId;

        double deepSamples = children.m_totalDeepSamples;

        if (L"[self]" == functionName)
        {
            deepSamples = children.m_totalSelfSamples;
            gDeepSamples = children.m_totalDeepSamples; // this is used to compute percentage in paths view
        }

        AddTopLevelItem(*m_pChildrenTreeControl,
                        children.m_functionInfo.m_modulePath,
                        children.m_functionInfo.m_moduleId,
                        functionName,
                        functionId,
                        deepSamples,
                        deepSamples,    //TODO : redundant arg need to be removed
                        children.m_deepSamplesPerc,
                        noSamples);

    }

    bool isSortIndShown = m_pChildrenTreeControl->header()->isSortIndicatorShown();
    // disable the sort indicator - for more accurate resizing (header size will not include sort indicator size)
    m_pChildrenTreeControl->header()->setSortIndicatorShown(false);

    m_pChildrenTreeControl->resizeColumnToContents(CALLGRAPH_BUTTERFLY_SAMPLES);
    m_pChildrenTreeControl->resizeColumnToContents(CALLGRAPH_BUTTERFLY_PERCENTAGE);

    // set SortIndicatorShown back
    m_pChildrenTreeControl->header()->setSortIndicatorShown(isSortIndShown);

    // Unblock the signals.
    m_pChildrenTreeControl->blockSignals(false);

}


// When a parent item is double-clicked, should show the function data in another tab
void CallGraphButterfly::onFunctionClicked(QTreeWidgetItem* pItem)
{
    if (nullptr != m_pLastTree)
    {
        pItem = m_pLastTree->currentItem();

        if (nullptr != pItem)
        {
            CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(pItem);

            if (nullptr != pFuncListItem)
            {
                emit functionSelected(pFuncListItem->m_functionId);
            }
        }
    }
}


// ****************************
// Class CallGraphTab
// ***************************


SessionCallGraphView::SessionCallGraphView(QWidget* pParent,
                                           CpuSessionWindow* pSessionWindow,
                                           afApplicationTreeItemData* pDisplayedSessionItemData) :
    DataTab(pParent, pSessionWindow)
{
    m_exportString = "&Export call-stack data...";
    m_pDisplayedSessionItemData = pDisplayedSessionItemData;
    setFilter(&m_displaySettings);

    setEnableOnlySystemDllInfilterDlg(true);

    // Build the view layout:
    SetViewLayout();

    m_editActionsWidgetsList << m_pFuncTable->functionsTreeControl();
    m_editActionsWidgetsList << m_pPathFuncTable->functionsTreeControl();
    m_editActionsWidgetsList << m_pButterfly->childrenTreeControl();
    m_editActionsWidgetsList << m_pButterfly->parentsTreeControl();

    if (nullptr != m_pDisplayFilter)
    {
        m_isSystemDLLDisplayed = !m_pDisplayFilter->IsSystemModuleIgnored();

        CounterNameIdVec counterDetails;
        gtString configName = L"All Data";

        if (true == m_pDisplayFilter->GetConfigCounters(acGTStringToQString(configName), counterDetails))
        {
            if (!counterDetails.empty())
            {
                m_selectedCounter = std::get<3>(counterDetails.at(0));
            }
        }
    }
}


SessionCallGraphView::~SessionCallGraphView()
{
    if (nullptr != m_pSplitter)
    {
        delete m_pSplitter;
    }
}

void SessionCallGraphView::FunctionListSelectionDone(AMDTFunctionId functionId)
{
    if (m_pFuncIdSelected != functionId)
    {
        m_pFuncIdSelected = functionId;

        ShowParentChild(functionId);
        ShowPaths(functionId);
        emit functionSelected(functionId);
    }
}

void SessionCallGraphView::editSource(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32> info)
{
    auto itr = m_pFuncTable->m_FunctionIdSampleMap.find(std::get<0>(info));

    if ((m_pFuncTable->m_FunctionIdSampleMap.end() != itr) &&
        (itr->second == true))
    {
        emit opensourceCodeViewSig(info);
    }
}

// The display is for a particular PID
bool SessionCallGraphView::Display(const QString& caption, unsigned int pid)
{
    m_pid = 0U;
    m_indexOffset = CALLGRAPH_OFFSET_INDEX;
    m_sessionFile.setFile(caption);

    // Fill the process IDs combo box:
    fillPIDComb(pid);

    // Fill the hot spot indicator combo box:
    bool rcHotSpot = fillCounterIndicatorCombo();
    GT_ASSERT(rcHotSpot);

    // Set the information panel content:
    updateHint(CP_callgraphInformationHint);

    const QComboBox* pPIDComboBox = TopToolbarComboBox(m_pPidComboAction);
    GT_IF_WITH_ASSERT(pPIDComboBox != nullptr)
    {
        QString pidText = pPIDComboBox->currentText();
        showPid(getPidFromProcessString(pidText));
    }

    qApp->restoreOverrideCursor();

    // Update display filter string:
    updateDisplaySettingsString();

    return true;
}

void SessionCallGraphView::OnDblClicked(QTreeWidgetItem* pItem)
{
    (void)(pItem); // unused
}


void SessionCallGraphView::OnButterflyClicked(QString funName, gtUInt64 address, bool parent)
{
    (void)(funName); // unused
    (void)(address); // unused
    (void)(parent); // unused
}

//We need to update the butterfly view to show correct data for the selected
//  function
void SessionCallGraphView::OnSelectionChange()
{

}

void SessionCallGraphView::onOpenDisplayFilterDialog()
{
}

// When a list item is expanded, show the functions called for it
void SessionCallGraphView::OnExpandItem(QTreeWidgetItem* pItem)
{
    (void)(pItem); // unused
}


void SessionCallGraphView::onViewChanged()
{
    m_precision = afGlobalVariablesManager::instance().floatingPointPrecision();
}


void SessionCallGraphView::showPid(unsigned int pid)
{
    GT_IF_WITH_ASSERT((m_pDisplayedSessionItemData != nullptr) &&
                      (nullptr != m_pProfDataRdr))
    {
        m_selectedPID = pid;
        m_precision = afGlobalVariablesManager::instance().floatingPointPrecision();
        m_pFuncTable->clear();

        AMDTFunctionId funcIdMaxSamples = 0;
        m_pFuncTable->FillDisplayFuncList(m_pProfDataRdr, m_pDisplayFilter, m_selectedCounter, m_selectedPID, funcIdMaxSamples);

        // initially set the functionId to function having maximum samples.
        if (AMDT_PROFILE_ALL_FUNCTIONS == m_pFuncIdSelected)
        {
            m_pFuncIdSelected = funcIdMaxSamples;
        }

        const std::vector<AMDTFunctionId>& funcVector = m_pFuncTable->m_funcIdVec;

        // scenario when the selected funcid does not exist, point to the
        // function having maximum samples.
        if (std::end(funcVector) == std::find(std::begin(funcVector), std::end(funcVector), m_pFuncIdSelected))
        {
            m_pFuncIdSelected = funcIdMaxSamples;
        }

        ShowParentChild(m_pFuncIdSelected);
        ShowPaths(m_pFuncIdSelected);

        emit functionSelected(m_pFuncIdSelected);

    }
}

EventMaskType SessionCallGraphView::getFilterEvent() const
{
    EventMaskType eventId;

    if (ColumnValue == m_cuurentColSpec.type)
    {
        eventId = EncodeEvent(m_cuurentColSpec.dataSelectLeft.eventSelect,
                              m_cuurentColSpec.dataSelectLeft.eventUnitMask,
                              m_cuurentColSpec.dataSelectLeft.bitOs,
                              m_cuurentColSpec.dataSelectLeft.bitUsr);
    }
    else
    {
        eventId = EventMaskType(-1);
    }

    return eventId;
}

void SessionCallGraphView::ShowPaths(AMDTFunctionId functionId)
{
    if (nullptr != m_pPathFuncTable)
    {
        qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

        m_pPathFuncTable->SetFunctionPath(m_pProfDataRdr, functionId, m_selectedPID, m_isSystemDLLDisplayed);

        qApp->restoreOverrideCursor();
    }
}

//When the user selects a pid from the drop down box in the toolbar
void SessionCallGraphView::OnSelectPid(int index)
{
    (void)(index); // Unused
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

    const QComboBox* pPIDComboBox = TopToolbarComboBox(m_pPidComboAction);
    GT_IF_WITH_ASSERT(pPIDComboBox != nullptr)
    {
        QString pidText = pPIDComboBox->currentText();
        showPid(getPidFromProcessString(pidText));
    }

    qApp->restoreOverrideCursor();
}

//When the user selects a pid from the drop down box in the toolbar
void SessionCallGraphView::OnSelectHotSpotIndicator(int index)
{
    m_selectedCounter = m_hotSpotCounterIdMap.at(index);

    // Sanity check:
    const QComboBox* pHotSpotIndicatorComboBox = TopToolbarComboBox(m_pHotSpotIndicatorComboBoxAction);
    GT_IF_WITH_ASSERT((nullptr != m_pParentSessionWindow) &&
                      (pHotSpotIndicatorComboBox != nullptr))
    {
        qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
        const QComboBox* pPIDComboBox = TopToolbarComboBox(m_pPidComboAction);
        GT_IF_WITH_ASSERT(pPIDComboBox != nullptr)
        {
            QString pidText = pPIDComboBox->currentText();
            showPid(getPidFromProcessString(pidText));
        }
        qApp->restoreOverrideCursor();
    }
}

bool SessionCallGraphView::fillCounterIndicatorCombo()
{
    GT_IF_WITH_ASSERT((m_pProfDataRdr != nullptr) &&
                      (m_pHotSpotIndicatorComboBoxAction != nullptr) &&
                      (m_pParentSessionWindow != nullptr))
    {
        AMDTProfileCounterDescVec counterDesc;
        m_pProfDataRdr->GetSampledCountersList(counterDesc);

        QStringList supportedCounterList;

        for (const auto& counter : counterDesc)
        {
            // TODO : change with abbreviation
            supportedCounterList << acGTStringToQString(counter.m_name);
            m_hotSpotCounterIdMap.push_back(counter.m_id);
        }

        if (!supportedCounterList.empty())
        {
            m_pHotSpotIndicatorComboBoxAction->UpdateStringList(supportedCounterList);
        }

        // Enable the combo if there are strings in the list:
        m_pHotSpotIndicatorComboBoxAction->UpdateEnabled(supportedCounterList.size() > 1);
    }
    return true;
}

acToolBar* SessionCallGraphView::CreateToolbar()
{
    m_pTopToolbar = new acToolBar(nullptr);


    // Do not allow the toolbar to float:
    m_pTopToolbar->setFloatable(false);
    m_pTopToolbar->setMovable(false);
    m_pTopToolbar->setStyleSheet("QToolBar { border-style: none; margin:5;}");

    QWidget* emptySpacer = new QWidget;
    emptySpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QString caption = CP_strCPUProfileToolbarBase;
    caption.append(":");

    m_pTopToolbar->AddLabel(CP_strFunctions, true, false, 0);
    m_pTopToolbar->addWidget(emptySpacer);
    acWidgetAction* pTestAction = m_pTopToolbar->AddLabel(caption);
    const QLabel* pTestLabel = TopToolbarLabel(pTestAction);
    int w = 25;

    if (pTestLabel != nullptr)
    {
        static QString longestDisplayFilter = "System Modules Hidden";
        w = pTestLabel->fontMetrics().boundingRect(longestDisplayFilter).width();

        // Add margin to be on the safe side:
        w += 50;
    }

    // Create the display settings link:
    acToolbarActionData actionData(SIGNAL(linkActivated(const QString&)), this, SLOT(OnDisplaySettingsClicked()));
    actionData.m_actionType = acToolbarActionData::AC_LABEL_ACTION;
    actionData.m_margin = 5;
    actionData.m_minWidth = w;
    actionData.m_objectName = "CallgraphDisplayFilterLink";
    m_pDisplaySettingsAction = m_pTopToolbar->AddWidget(actionData);
    GT_ASSERT(m_pDisplaySettingsAction != nullptr);

    m_pTopToolbar->AddLabel(CP_sourceCodeViewProcessPrefix);

    // Add the process ID combo box:
    m_pPidComboAction = m_pTopToolbar->AddComboBox(QStringList(), SIGNAL(currentIndexChanged(int)), this, SLOT(OnSelectPid(int)));
    GT_ASSERT(m_pPidComboAction);

    m_pTopToolbar->AddLabel("Monitored event: ");

    m_pHotSpotIndicatorComboBoxAction = m_pTopToolbar->AddComboBox(QStringList(), SIGNAL(activated(int)), this, SLOT(OnSelectHotSpotIndicator(int)));
    GT_ASSERT(m_pHotSpotIndicatorComboBoxAction);

    return m_pTopToolbar;
}

int SessionCallGraphView::getPidFromProcessString(QString processString)
{
    int i = processString.lastIndexOf("(");
    int numnber = 0;

    if (-1 != i)
    {
        bool ok;
        numnber = processString.mid(i + 1, processString.length() - i - 2).toInt(&ok);

        if (!ok)
        {
            numnber = 0;
        }
    }

    return numnber;
}

void SessionCallGraphView::fillPIDComb(unsigned int pid)
{
    GT_IF_WITH_ASSERT((m_pPidComboAction != nullptr) &&
                      (m_pTopToolbar != nullptr) &&
                      (m_pParentSessionWindow != nullptr) &&
                      (m_pProfDataRdr != nullptr))
    {
        const QComboBox* pPidCombo = TopToolbarComboBox(m_pPidComboAction);
        GT_IF_WITH_ASSERT(pPidCombo != nullptr)
        {
            int index = -1;
            QStringList cssProcessesList;
            gtVector<AMDTProcessId> cssProcesses;
            m_pProfDataRdr->GetCallGraphProcesses(cssProcesses);

            gtVector<AMDTProfileProcessInfo> procInfo;
            m_pProfDataRdr->GetProcessInfo(AMDT_PROFILE_ALL_PROCESSES, procInfo);


            for (const auto& cssProcess : procInfo)
            {
                auto itr = std::find(cssProcesses.begin(), cssProcesses.end(), cssProcess.m_pid);

                if (itr != cssProcesses.end())
                {
                    // TODO : need to be revisited
                    if (cssProcess.m_pid == pid)
                    {
                        index = cssProcessesList.count();
                    }

                    QString pidEntry = QString("%1(%2)").arg(acGTStringToQString(cssProcess.m_name)).arg(cssProcess.m_pid);
                    cssProcessesList << pidEntry;
                }
            }

            // Update the combo box with the list of processes:
            m_pPidComboAction->UpdateStringList(cssProcessesList);

            if (-1 == index)
            {
                index = 0;

                AMDTProfileSessionInfo sessionInfo;
                bool ret = m_pProfDataRdr->GetProfileSessionInfo(sessionInfo);

                if (true == ret)
                {
                    QString strLaunchProcess = acGTStringToQString(sessionInfo.m_targetAppPath);
                    QFileInfo f(strLaunchProcess);
                    strLaunchProcess = f.fileName();

                    for (int i = 0, count = pPidCombo->count(); i < count; ++i)
                    {
                        if (pPidCombo->itemText(i).startsWith(strLaunchProcess))
                        {
                            index = i;
                            break;
                        }
                    }
                }
            }

            m_pPidComboAction->UpdateCurrentIndex(index);
        }
    }
}

void SessionCallGraphView::UpdateTableDisplay(unsigned int updateType)
{
    updateType = updateType;

    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

    const QComboBox* pPIDComboBox = TopToolbarComboBox(m_pPidComboAction);
    GT_IF_WITH_ASSERT(pPIDComboBox != nullptr)
    {
        QString pidText = pPIDComboBox->currentText();
        showPid(getPidFromProcessString(pidText));
    }

    qApp->restoreOverrideCursor();
}

void SessionCallGraphView::selectFunction(AMDTFunctionId funcID, ProcessIdType pid)
{
    GT_IF_WITH_ASSERT(nullptr != m_pPidComboAction)
    {
        const QComboBox* pPIDComboBox = TopToolbarComboBox(m_pPidComboAction);

        GT_IF_WITH_ASSERT(nullptr != pPIDComboBox)
        {
            // Construct the PID string suffix.
            QString pidText = '(' + QString::number(pid) + ')';

            // Search for the item in the Combo-Box with the requested PID, and select it.
            for (int i = 0, num = pPIDComboBox->count(); i < num; ++i)
            {
                if (pPIDComboBox->itemText(i).endsWith(pidText))
                {
                    if (pPIDComboBox->currentIndex() != i)
                    {
                        m_pPidComboAction->UpdateCurrentIndex(i);
                    }

                    break;
                }
            }
        }
    }

    GT_IF_WITH_ASSERT(nullptr != m_pFuncTable && nullptr != m_pFuncTable->functionsTreeControl())
    {
        AMDTFunctionId functionId = funcID;
        ShowParentChild(functionId);
        ShowPaths(functionId);
        emit functionSelected(functionId);
    }
}

void SessionCallGraphView::onEditCopy()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pFuncTable != nullptr) && (m_pPathFuncTable != nullptr) && (m_pButterfly != nullptr))
    {
        if (m_pFuncTable->functionsTreeControl() == m_pLastFocusedWidget)
        {
            m_pFuncTable->functionsTreeControl()->onEditCopy();
        }
        else if (m_pPathFuncTable->functionsTreeControl() == m_pLastFocusedWidget)
        {
            m_pPathFuncTable->functionsTreeControl()->onEditCopy();
        }
        else if (m_pButterfly->parentsTreeControl() == m_pLastFocusedWidget)
        {
            m_pButterfly->parentsTreeControl()->onEditCopy();
        }
        else if (m_pButterfly->childrenTreeControl() == m_pLastFocusedWidget)
        {
            m_pButterfly->childrenTreeControl()->onEditCopy();
        }
    }
}


void SessionCallGraphView::onEditSelectAll()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pFuncTable != nullptr) && (m_pPathFuncTable != nullptr) && (m_pButterfly != nullptr))
    {
        if (m_pFuncTable->functionsTreeControl() == m_pLastFocusedWidget)
        {
            m_pFuncTable->functionsTreeControl()->onEditSelectAll();
            m_pFuncTable->functionsTreeControl()->setFocus(Qt::ActiveWindowFocusReason);
            m_pFuncTable->functionsTreeControl()->activateWindow();
            m_pPathFuncTable->functionsTreeControl()->clearSelection();
            m_pButterfly->childrenTreeControl()->clearSelection();
            m_pButterfly->parentsTreeControl()->clearSelection();
        }
        else if (m_pPathFuncTable->functionsTreeControl() == m_pLastFocusedWidget)
        {
            m_pPathFuncTable->functionsTreeControl()->onEditSelectAll();
            m_pPathFuncTable->functionsTreeControl()->setFocus(Qt::ActiveWindowFocusReason);
            m_pPathFuncTable->functionsTreeControl()->activateWindow();
            m_pFuncTable->functionsTreeControl()->clearSelection();
            m_pButterfly->childrenTreeControl()->clearSelection();
            m_pButterfly->parentsTreeControl()->clearSelection();
        }
        else if (m_pButterfly->parentsTreeControl() == m_pLastFocusedWidget)
        {
            m_pButterfly->parentsTreeControl()->onEditSelectAll();
            m_pButterfly->parentsTreeControl()->setFocus(Qt::ActiveWindowFocusReason);
            m_pButterfly->parentsTreeControl()->activateWindow();
            m_pPathFuncTable->functionsTreeControl()->clearSelection();
            m_pButterfly->childrenTreeControl()->clearSelection();
            m_pFuncTable->functionsTreeControl()->clearSelection();
        }
        else if (m_pButterfly->childrenTreeControl() == m_pLastFocusedWidget)
        {
            m_pButterfly->childrenTreeControl()->onEditSelectAll();
            m_pButterfly->childrenTreeControl()->setFocus(Qt::ActiveWindowFocusReason);
            m_pButterfly->childrenTreeControl()->activateWindow();
            m_pPathFuncTable->functionsTreeControl()->clearSelection();
            m_pButterfly->parentsTreeControl()->clearSelection();
            m_pFuncTable->functionsTreeControl()->clearSelection();
        }
    }
}

void SessionCallGraphView::onFindClick()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pFuncTable != nullptr) && (m_pPathFuncTable != nullptr) && (m_pButterfly != nullptr))
    {
        if (m_pFuncTable->functionsTreeControl() == m_pLastFocusedWidget)
        {
            m_pFuncTable->functionsTreeControl()->onEditFind();
        }
        else if (m_pPathFuncTable->functionsTreeControl() == m_pLastFocusedWidget)
        {
            m_pPathFuncTable->functionsTreeControl()->onEditFind();
        }
        else if (m_pButterfly->parentsTreeControl() == m_pLastFocusedWidget)
        {
            m_pButterfly->parentsTreeControl()->onEditFind();
        }
        else if (m_pButterfly->childrenTreeControl() == m_pLastFocusedWidget)
        {
            m_pButterfly->childrenTreeControl()->onEditFind();
        }
    }
}

void SessionCallGraphView::onFindNext()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pFuncTable != nullptr) && (m_pPathFuncTable != nullptr) && (m_pButterfly != nullptr))
    {
        if (m_pFuncTable->functionsTreeControl() == m_pLastFocusedWidget)
        {
            m_pFuncTable->functionsTreeControl()->onEditFindNext();
        }
        else if (m_pPathFuncTable->functionsTreeControl() == m_pLastFocusedWidget)
        {
            m_pPathFuncTable->functionsTreeControl()->onEditFindNext();
        }
        else if (m_pButterfly->parentsTreeControl() == m_pLastFocusedWidget)
        {
            m_pButterfly->parentsTreeControl()->onEditFindNext();
        }
        else if (m_pButterfly->childrenTreeControl() == m_pLastFocusedWidget)
        {
            m_pButterfly->childrenTreeControl()->onEditFindNext();
        }
    }
}

void SessionCallGraphView::SetViewLayout()
{
    m_pTopToolbar = CreateToolbar();
    GT_IF_WITH_ASSERT(m_pTopToolbar != nullptr)
    {
        if (nullptr != m_pSplitter)
        {
            delete m_pSplitter;
        }

        m_pSplitter = new QSplitter(Qt::Vertical, this);    // Add splitter control to the main window


        m_pFuncTable = new CallGraphFuncList(this);  // Add  widget to CallGraphTab


        m_pFuncTable->Initialize();

        // User select a function
        //bool rc = connect(m_pFuncTable, SIGNAL(functionSelected(const FunctionGraph::Node&)), SLOT(FunctionListSelectionDone(const FunctionGraph::Node&)));
        bool rc = connect(m_pFuncTable, SIGNAL(functionSelected(AMDTFunctionId)), SLOT(FunctionListSelectionDone(AMDTFunctionId)));
        GT_ASSERT(rc);

        //rc = connect(this, SIGNAL(functionSelected(const FunctionGraph::Node&)), m_pFuncTable, SLOT(selectAFunction(const FunctionGraph::Node&)));
        rc = connect(this, SIGNAL(functionSelected(AMDTFunctionId)), m_pFuncTable, SLOT(selectAFunction(AMDTFunctionId)));
        GT_ASSERT(rc);

        rc = connect(m_pFuncTable->functionsTreeControl(), SIGNAL(editSourceFile(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32>)),
                     SLOT(editSource(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32>)));
        GT_ASSERT(rc);

        //allocate the butterfly widget
        m_pButterfly = new CallGraphButterfly(this);


        //rc = connect(m_pButterfly, SIGNAL(functionSelected(const FunctionGraph::Node&)), SLOT(FunctionListSelectionDone(const FunctionGraph::Node&)));
        rc = connect(m_pButterfly, SIGNAL(functionSelected(AMDTFunctionId)), SLOT(FunctionListSelectionDone(AMDTFunctionId)));
        GT_ASSERT(rc);

        m_pPathFuncTable = new CallGraphPathFuncList(this);

        m_pPathFuncTable->setWindowTitle("Paths");
        m_pPathFuncTable->initialize();

        //rc = connect(m_pPathFuncTable, SIGNAL(functionSelected(const FunctionGraph::Node&)), SLOT(FunctionListSelectionDone(const FunctionGraph::Node&)));
        rc = connect(m_pPathFuncTable, SIGNAL(functionSelected(AMDTFunctionId)), SLOT(FunctionListSelectionDone(AMDTFunctionId)));
        GT_ASSERT(rc);

        rc = connect(m_pPathFuncTable->functionsTreeControl(), SIGNAL(editSourceFile(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32>)),
                     SLOT(editSource(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32>)));
        GT_ASSERT(rc);

        m_pButterfly->Initialize();

        rc = connect(m_pButterfly->parentsTreeControl(), SIGNAL(editSourceFile(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32>)),
                     SLOT(editSource(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32>)));
        GT_ASSERT(rc);

        rc = connect(m_pButterfly->childrenTreeControl(), SIGNAL(editSourceFile(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32>)),
                     SLOT(editSource(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32>)));
        GT_ASSERT(rc);

        m_pFuncTable->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        setCentralWidget(m_pSplitter); // This give a center point

        m_pSplitter->setChildrenCollapsible(false);

        QVBoxLayout* pTopLayout = new QVBoxLayout;


        pTopLayout->addWidget(m_pTopToolbar);
        pTopLayout->addWidget(m_pFuncTable);

        QWidget* pTopWidget = new QWidget;

        pTopWidget->setLayout(pTopLayout);

        // Add the top widget (contain the toolbar and the top table):
        m_pSplitter->addWidget(pTopWidget);

        // Add the butterfly widget to the splitter:
        m_pSplitter->addWidget(m_pButterfly);

        // Create the information frame:
        QFrame* pInfoFrame = createHintLabelFrame();

        QVBoxLayout* pBottomLayout = new QVBoxLayout;


        pBottomLayout->addWidget(m_pPathFuncTable);
        pBottomLayout->addWidget(pInfoFrame);

        QWidget* pBottomWidget = new QWidget;

        pBottomWidget->setLayout(pBottomLayout);

        // Add the top widget (contain the toolbar and the top table):
        m_pSplitter->addWidget(pBottomWidget);

        m_pSplitter->setContentsMargins(0, 0, 0, 0);
        m_pSplitter->setStretchFactor(0, 10);
        m_pSplitter->setStretchFactor(1, 5);
        m_pSplitter->setStretchFactor(2, 7);

        qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

    }
}


void SessionCallGraphView::displayInFunctionView(AMDTFunctionId functionId)
{
    // Get the tree instance:
    afApplicationCommands* pCommands = afApplicationCommands::instance();
    SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();

    GT_IF_WITH_ASSERT((pCommands != nullptr) && (pSessionViewCreator != nullptr))
    {
        afApplicationTree* pApplicationTree = pCommands->applicationTree();
        GT_IF_WITH_ASSERT((m_pDisplayedSessionItemData != nullptr) && (pApplicationTree != nullptr))
        {
            afApplicationTreeItemData* pSessionItemData = ProfileApplicationTreeHandler::instance()->FindItemByProfileFilePath(m_pDisplayedSessionItemData->m_filePath);
            afApplicationTreeItemData* pActivatedItemMacthingItemData = ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(pSessionItemData, AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS);
            GT_IF_WITH_ASSERT(pActivatedItemMacthingItemData != nullptr)
            {
                // Get the tree instance:
                pApplicationTree->expandItem(pActivatedItemMacthingItemData->m_pTreeWidgetItem);
                pApplicationTree->selectItem(pActivatedItemMacthingItemData, true);

                CPUSessionTreeItemData* pSessionData = qobject_cast<CPUSessionTreeItemData*>(pActivatedItemMacthingItemData->extendedItemData());
                GT_IF_WITH_ASSERT(pSessionData != nullptr)
                {
                    CpuSessionWindow* pSessionWindow = nullptr;
                    const gtVector<CpuSessionWindow*>& openedSessions = pSessionViewCreator->currentlyOpenedSessionWindows();

                    for (int i = 0; i < (int)openedSessions.size(); i++)
                    {
                        if (openedSessions[i] != nullptr)
                        {
                            if (openedSessions[i]->displayedSessionFilePath() == pActivatedItemMacthingItemData->m_filePath)
                            {
                                pSessionWindow = openedSessions[i];
                            }
                        }
                    }

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
                            pFunctionsView->selectFunction(QString::number(functionId));
                        }
                    }
                }
            }
        }
    }
}


// ****************************
// Class FunctionsTreeCtrl
// ***************************

FunctionsTreeCtrl::FunctionsTreeCtrl(SessionCallGraphView* pCallGraphTab,
                                     QWidget* pParent,
                                     bool addExpandCollapeAllActions,
                                     int numberOfColumns) : acTreeCtrl(pParent, numberOfColumns, false, addExpandCollapeAllActions),
    m_pCallGraphTab(pCallGraphTab), m_pSourceCodeAction(nullptr), m_pDisplayInFunctionsAction(nullptr)
{
    AddCallGraphContextMenuActions();

    bool rc = connect(this, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(GoToSource()));
    GT_ASSERT(rc);
}

FunctionsTreeCtrl::~FunctionsTreeCtrl()
{
}

void FunctionsTreeCtrl::onContextMenuEvent(const QPoint& position)
{
    // dont show context menu if not selected items
    if (selectedItems().count() > 0)
    {
        acTreeCtrl::onContextMenuEvent(position);
    }
}

void FunctionsTreeCtrl::onAboutToShowContextMenu()
{
    // Call the base class implementation:
    acTreeCtrl::onAboutToShowContextMenu();

    // Find the action
    if (nullptr != _pContextMenu && nullptr != m_pCallGraphTab && m_pSourceCodeAction != nullptr && m_pDisplayInFunctionsAction != nullptr)
    {
        // Check if the source code and functions view actions should be enabled:
        // Actions are enabled if there is only 1 item selected, and functions view / source code view is available:
        bool enableSourceCodeAction = false;
        bool enableFunctionsAction = false;

        const CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(currentItem());

        if ((selectedItems().count() == 1) && (nullptr != pFuncListItem))
        {
            enableFunctionsAction = (0ULL != pFuncListItem->m_selfSample);
            enableSourceCodeAction = enableFunctionsAction; // Need to set module states if loaded && (nullptr != pMetadata->m_pModule);
        }

        m_pDisplayInFunctionsAction->setEnabled(enableFunctionsAction);
        m_pSourceCodeAction->setEnabled(enableSourceCodeAction);
    }
}

void FunctionsTreeCtrl::AddCallGraphContextMenuActions()
{
    // Add the actions for the functions tree:
    GT_IF_WITH_ASSERT(_pContextMenu != nullptr)
    {
        QList<QAction*> actions;

        // Create source code action (bold):
        m_pSourceCodeAction = new QAction(CA_STR_MENU_DISPLAY_IN_SOURCE_CODE_VIEW, _pContextMenu);

        QFont font = m_pSourceCodeAction->font();
        font.setBold(true);
        m_pSourceCodeAction->setFont(font);

        m_pDisplayInFunctionsAction = new QAction(CA_STR_MENU_DISPLAY_IN_FUNCTIONS_VIEW, _pContextMenu);

        actions << m_pSourceCodeAction;
        actions << m_pDisplayInFunctionsAction;

        // Connect the action to the handlers:
        bool rcConnect = connect(m_pSourceCodeAction, SIGNAL(triggered()), this, SLOT(GoToSource()));
        GT_ASSERT(rcConnect);

        rcConnect = connect(m_pDisplayInFunctionsAction, SIGNAL(triggered()), this, SLOT(GoToFunctionsView()));
        GT_ASSERT(rcConnect);

        _pContextMenu->insertSeparator(_pContextMenu->actions().first());
        _pContextMenu->insertActions(_pContextMenu->actions().first(), actions);
    }
}

void FunctionsTreeCtrl::GoToSource()
{
    CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(currentItem());


    auto funcModInfo = std::make_tuple(pFuncListItem->m_functionId,
                                       pFuncListItem->m_moduleName,
                                       pFuncListItem->m_moduleId,
                                       AMDT_PROFILE_ALL_PROCESSES);

    emit editSourceFile(funcModInfo);
}

void FunctionsTreeCtrl::GoToFunctionsView()
{
    GT_IF_WITH_ASSERT(nullptr != m_pCallGraphTab)
    {
        CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(currentItem());

        if (nullptr != pFuncListItem)
        {
            m_pCallGraphTab->displayInFunctionView(pFuncListItem->m_functionId);
        }
    }
}

void FunctionsTreeCtrl::PathIndicatorToggled(int state)
{
    SetPathIndicator(Qt::Checked == state);
}

CallGraphFuncListItem* CallGraphButterfly::AddTopLevelItem(FunctionsTreeCtrl& treeCtrl,
                                                           const gtString modulePath,
                                                           AMDTModuleId    moduleId,
                                                           const gtString& funcName,
                                                           AMDTFunctionId funcId,
                                                           double samplesCount,
                                                           double totalDeepCount,
                                                           double samplePercent,
                                                           bool noSamples)
{

    totalDeepCount = totalDeepCount;
    CallGraphFuncListItem* pFuncListItem = new CallGraphFuncListItem;

    GT_IF_WITH_ASSERT(pFuncListItem != nullptr)
    {
        treeCtrl.addTopLevelItem(pFuncListItem);
        pFuncListItem->setText(CALLGRAPH_BUTTERFLY_FUNCTION, acGTStringToQString(funcName));
        pFuncListItem->setData(CALLGRAPH_BUTTERFLY_SAMPLES, Qt::DisplayRole, QVariant(samplesCount));
        pFuncListItem->setTextAlignment(CALLGRAPH_BUTTERFLY_SAMPLES, Qt::AlignLeft | Qt::AlignVCenter);
        pFuncListItem->m_functionId = funcId;
        pFuncListItem->m_functionName = funcName;
        pFuncListItem->m_moduleName = modulePath;
        pFuncListItem->m_moduleId = moduleId;

        if (0ULL != samplesCount && !noSamples)
        {
            pFuncListItem->setData(CALLGRAPH_BUTTERFLY_PERCENTAGE, Qt::DisplayRole, QVariant(samplePercent));
        }
        else
        {
            pFuncListItem->setText(CALLGRAPH_BUTTERFLY_PERCENTAGE, QString());
        }


        gtString modFileName;

        if (!modulePath.isEmpty())
        {
            pFuncListItem->setToolTip(CALLGRAPH_BUTTERFLY_MODULE, acGTStringToQString(modulePath));
        }

        osFilePath path(modulePath);
        gtString filename;
        path.getFileNameAndExtension(filename);
        pFuncListItem->setText(CALLGRAPH_BUTTERFLY_MODULE, acGTStringToQString(filename));
    }

    return pFuncListItem;
}