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

void CallGraphPathFuncList::InitializeItem(CallGraphFuncListItem* pFuncListItem)
{
    const CssFunctionMetadata* pMetadata = static_cast<const CssFunctionMetadata*>(pFuncListItem->m_pFuncNode->m_val);

    pFuncListItem->setText(CALLGRAPH_PATH_TREE, pMetadata->m_funcName);

    pFuncListItem->setText(CALLGRAPH_PATH_SELF, QString());
    pFuncListItem->setTextAlignment(CALLGRAPH_PATH_SELF, Qt::AlignLeft | Qt::AlignVCenter);

    pFuncListItem->setText(CALLGRAPH_PATH_DOWNSTREAM_SAMPLES, QString());
    pFuncListItem->setTextAlignment(CALLGRAPH_PATH_DOWNSTREAM_SAMPLES, Qt::AlignLeft | Qt::AlignVCenter);

    pFuncListItem->setText(CALLGRAPH_PATH_DOWNSTREAM_PERCENTAGE, QString());

    gtString moduleName;

    if (nullptr != pMetadata->m_pModule)
    {
        if (pMetadata->m_pModule->extractFileName(moduleName))
        {
            pFuncListItem->setToolTip(CALLGRAPH_PATH_MODULE, acGTStringToQString(pMetadata->m_pModule->getPath()));
        }
    }

    pFuncListItem->setText(CALLGRAPH_PATH_MODULE, acGTStringToQString(moduleName));
}

CallGraphFuncListItem* CallGraphPathFuncList::AcquireTopLevelItem(const FunctionGraph::Node* pFuncNode)
{
    // Search for a matching top level item
    CallGraphFuncListItem* pFuncListItem = nullptr;

    for (int itemIndex = 0, itemsCount = m_pFuncTable->topLevelItemCount(); itemIndex < itemsCount; ++itemIndex)
    {
        CallGraphFuncListItem* pCurrFuncListItem = static_cast<CallGraphFuncListItem*>(m_pFuncTable->topLevelItem(itemIndex));

        if (nullptr != pCurrFuncListItem && nullptr != pCurrFuncListItem->m_pFuncNode)
        {
            if (pFuncNode == pCurrFuncListItem->m_pFuncNode)
            {
                pFuncListItem = pCurrFuncListItem;
                break;
            }
        }
    }


    if (nullptr == pFuncListItem)
    {
        pFuncListItem = new CallGraphFuncListItem(pFuncNode);
        m_pFuncTable->addTopLevelItem(pFuncListItem);

        InitializeItem(pFuncListItem);
    }

    return pFuncListItem;
}

CallGraphFuncListItem* CallGraphPathFuncList::AcquireChildItem(CallGraphFuncListItem* pParentItem, const FunctionGraph::Node* pFuncNode)
{
    CallGraphFuncListItem* pFuncListItem;

    if (nullptr != pParentItem)
    {
        // Search for a matching child item
        pFuncListItem = nullptr;

        for (int itemIndex = 0, itemsCount = pParentItem->childCount(); itemIndex < itemsCount; ++itemIndex)
        {
            CallGraphFuncListItem* pCurrFuncListItem = static_cast<CallGraphFuncListItem*>(pParentItem->child(itemIndex));

            if (nullptr != pCurrFuncListItem && nullptr != pCurrFuncListItem->m_pFuncNode)
            {
                if (pFuncNode == pCurrFuncListItem->m_pFuncNode)
                {
                    pFuncListItem = pCurrFuncListItem;
                    break;
                }
            }
        }


        if (nullptr == pFuncListItem)
        {
            pFuncListItem = new CallGraphFuncListItem(pFuncNode);
            pParentItem->addChild(pFuncListItem);

            InitializeItem(pFuncListItem);
        }
    }
    else
    {
        pFuncListItem = AcquireTopLevelItem(pFuncNode);
    }

    return pFuncListItem;
}

void CallGraphPathFuncList::SetFunctionPath(CpuProfileCss& css, const FunctionGraph::Node& funcNode)
{
    // Block the table model signals, otherwise the table is sorted while setting the data (which causes a mess).
    m_pFuncTable->blockSignals(true);

    clear();

    const bool displaySystemModules = CPUGlobalDisplayFilter::instance().m_displaySystemDLLs;

    CssFunctionMetadata* pRootMetadata = static_cast<CssFunctionMetadata*>(funcNode.m_val);
    pRootMetadata->m_childPathCount  = 0ULL;
    pRootMetadata->m_parentPathCount = 0ULL;
    pRootMetadata->m_childPathIndex  = unsigned(-1);
    pRootMetadata->m_parentPathIndex = unsigned(-1);

    for (FunctionGraph::NodeList::const_iterator it = funcNode.m_children.begin(), itEnd = funcNode.m_children.end(); it != itEnd; ++it)
    {
        CssFunctionMetadata* pMetadata = static_cast<CssFunctionMetadata*>((*it)->m_val);
        pMetadata->m_childPathCount  = 0ULL;
        pMetadata->m_parentPathCount = 0ULL;
        pMetadata->m_childPathIndex  = unsigned(-1);
        pMetadata->m_parentPathIndex = unsigned(-1);
    }

    for (FunctionGraph::NodeList::const_iterator it = funcNode.m_parents.begin(), itEnd = funcNode.m_parents.end(); it != itEnd; ++it)
    {
        CssFunctionMetadata* pMetadata = static_cast<CssFunctionMetadata*>((*it)->m_val);
        pMetadata->m_childPathCount  = 0ULL;
        pMetadata->m_parentPathCount = 0ULL;
        pMetadata->m_childPathIndex  = unsigned(-1);
        pMetadata->m_parentPathIndex = unsigned(-1);
    }

    const double totalDeepCount = static_cast<double>(pRootMetadata->m_deepCount);
    const bool noSamples = (0ULL == (pRootMetadata->m_deepCount));
    const EventMaskType eventId = css.GetEventId();
    const FunctionGraph& funcGraph = css.GetFunctionGraph();

    for (PathIndexSet::const_iterator it = funcNode.m_pathIndices.begin(), itEnd = funcNode.m_pathIndices.end(); it != itEnd; ++it)
    {
        const unsigned pathIndex = *it;
        const FunctionGraph::Path& path = *funcGraph.GetPath(pathIndex);
        const LeafFunctionList& leaves = path.GetData();

        bool isRootAtEnd = false;
        bool isRootFound = false;
        CallGraphFuncListItem* pFuncListItem = nullptr;

        if (0U != path.GetLength())
        {
            const FunctionGraph::Path::const_iterator itNodeBegin = path.begin(), itNodeEnd = path.end();
            FunctionGraph::Path::const_iterator itNodeEndPath = itNodeBegin;
            FunctionGraph::Path::const_iterator itNode = itNodeBegin;

            for (; itNode != itNodeEnd; ++itNode)
            {
                if (&*itNode == &funcNode)
                {
                    isRootFound = true;
                    itNodeEndPath = itNode;
                    break;
                }
            }

            if (displaySystemModules)
            {
                itNodeEndPath = itNodeEnd;
            }
            else
            {
                while (++itNodeEndPath != itNodeEnd)
                {
                    CssFunctionMetadata* pMetadata = static_cast<CssFunctionMetadata*>(itNodeEndPath->m_val);

                    if (nullptr != pMetadata->m_pModule && pMetadata->m_pModule->isSystemModule())
                    {
                        // We need the next node as we need to calculate also the parent's metadata.
                        ++itNodeEndPath;
                        break;
                    }
                }
            }

            isRootAtEnd = (&funcNode == &(*itNodeBegin));

            gtUInt64 downstreamCount = 0ULL;

            for (LeafFunctionList::const_iterator itLeaf = leaves.begin(), itLeafEnd = leaves.end(); itLeaf != itLeafEnd; ++itLeaf)
            {
                const LeafFunction& leaf = *itLeaf;

                if ((isRootFound || &funcNode == leaf.m_pNode) && (eventId == EventMaskType(-1) || eventId == leaf.m_eventId))
                {
                    downstreamCount += leaf.m_count;
                }
            }

            isRootFound = false;

            itNode = itNodeEndPath;

            while (itNode != itNodeBegin)
            {
                const FunctionGraph::Node& pathFuncNode = *(--itNode);

                if (&pathFuncNode == &funcNode)
                {
                    isRootFound = true;
                }

                CssFunctionMetadata* pMetadata = static_cast<CssFunctionMetadata*>(pathFuncNode.m_val);

                if (&pathFuncNode == &funcNode)
                {
                    FunctionGraph::Path::const_iterator itChildNode = itNode;

                    if (itNode != itNodeBegin)
                    {
                        --itChildNode;
                        CssFunctionMetadata* pChildMetadata = static_cast<CssFunctionMetadata*>((*itChildNode).m_val);

                        if (pathIndex != pChildMetadata->m_childPathIndex)
                        {
                            pChildMetadata->m_childPathIndex = pathIndex;
                            pChildMetadata->m_childPathCount += downstreamCount;
                        }
                    }

                    FunctionGraph::Path::const_iterator itParentNode = itNode;
                    ++itParentNode;

                    if (itParentNode != itNodeEndPath)
                    {
                        CssFunctionMetadata* pParentMetadata = static_cast<CssFunctionMetadata*>((*itParentNode).m_val);

                        if (pathIndex != pParentMetadata->m_parentPathIndex)
                        {
                            pParentMetadata->m_parentPathIndex = pathIndex;
                            pParentMetadata->m_parentPathCount += downstreamCount;
                        }
                    }
                }

                if (displaySystemModules || nullptr == pMetadata->m_pModule || !pMetadata->m_pModule->isSystemModule())
                {
                    pFuncListItem = AcquireChildItem(pFuncListItem, &pathFuncNode);

                    if (!isRootFound)
                    {
                        pFuncListItem->setExpanded(true);
                    }

                    qulonglong deepCount = pFuncListItem->AddCountValue(CALLGRAPH_PATH_DOWNSTREAM_SAMPLES, downstreamCount);

                    if (!noSamples)
                    {
                        deepCount += pFuncListItem->GetCountValue(CALLGRAPH_PATH_SELF);

                        pFuncListItem->SetPercentageValue(CALLGRAPH_PATH_DOWNSTREAM_PERCENTAGE,
                                                          static_cast<double>(deepCount),
                                                          totalDeepCount);
                    }
                }
                else if (isRootFound)
                {
                    pFuncListItem = nullptr;
                    break;
                }
            }
        }


        for (LeafFunctionList::const_iterator itLeaf = leaves.begin(), itLeafEnd = leaves.end(); itLeaf != itLeafEnd; ++itLeaf)
        {
            const LeafFunction& leaf = *itLeaf;

            if (eventId == EventMaskType(-1) || eventId == leaf.m_eventId)
            {
                CssFunctionMetadata* pMetadata = static_cast<CssFunctionMetadata*>(leaf.m_pNode->m_val);

                if (&funcNode == leaf.m_pNode)
                {
                    if (path.GetLength() != 0)
                    {
                        CssFunctionMetadata* pParentMetadata = static_cast<CssFunctionMetadata*>((*path.begin()).m_val);

                        if (pathIndex != pParentMetadata->m_parentPathIndex)
                        {
                            pParentMetadata->m_parentPathIndex = pathIndex;
                            pParentMetadata->m_parentPathCount += leaf.m_count;
                        }
                    }
                }
                else
                {
                    if (isRootAtEnd)
                    {
                        if (pathIndex != pMetadata->m_childPathIndex)
                        {
                            pMetadata->m_childPathCount += leaf.m_count;
                        }
                    }
                }

                if ((nullptr != pFuncListItem && isRootFound) || &funcNode == leaf.m_pNode)
                {
                    if (displaySystemModules || nullptr == pMetadata->m_pModule || !pMetadata->m_pModule->isSystemModule())
                    {
                        CallGraphFuncListItem* pLeafListItem = AcquireChildItem(pFuncListItem, leaf.m_pNode);
                        qulonglong deepCount = pLeafListItem->AddCountValue(CALLGRAPH_PATH_SELF, leaf.m_count);

                        if (!noSamples)
                        {
                            deepCount += pLeafListItem->GetCountValue(CALLGRAPH_PATH_DOWNSTREAM_SAMPLES);

                            pLeafListItem->SetPercentageValue(CALLGRAPH_PATH_DOWNSTREAM_PERCENTAGE,
                                                              static_cast<double>(deepCount),
                                                              totalDeepCount);
                        }
                    }
                }
            }
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

        if (nullptr != pFuncListItem->m_pFuncNode)
        {
            emit functionSelected(*pFuncListItem->m_pFuncNode);
        }
    }
}

void CallGraphPathFuncList::onSelectionChanged()
{
}

void CallGraphPathFuncList::onFunctionClicked(QTreeWidgetItem* pItem)
{
    if (nullptr != pItem)
    {
        CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(pItem);

        if (nullptr != pFuncListItem->m_pFuncNode)
        {
            emit functionSelected(*pFuncListItem->m_pFuncNode);
        }
    }
}

void CallGraphPathFuncList::onCurrentListItemChanged(QTreeWidgetItem* pItem)
{
    if (nullptr != pItem)
    {
        CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(pItem);

        if (nullptr != pFuncListItem->m_pFuncNode)
        {
            emit functionSelected(*pFuncListItem->m_pFuncNode);
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

CallGraphFuncListItem::CallGraphFuncListItem(acTreeCtrl* pParent, QTreeWidgetItem* pAfter) : QTreeWidgetItem(pParent, pAfter),
    m_pFuncNode(nullptr)
{
}

CallGraphFuncListItem::CallGraphFuncListItem(QTreeWidgetItem* pParent, QTreeWidgetItem* pAfter) : QTreeWidgetItem(pParent, pAfter),
    m_pFuncNode(nullptr)
{
}

CallGraphFuncListItem::CallGraphFuncListItem(const FunctionGraph::Node* pFuncNode) : m_pFuncNode(pFuncNode)
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

        if (nullptr != pFuncListItem->m_pFuncNode)
        {
            emit functionSelected(*pFuncListItem->m_pFuncNode);
        }
    }
}

void CallGraphFuncList::onSelectionChanged()
{
    CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(m_pFuncTable->currentItem());

    if (nullptr != pFuncListItem)
    {
        if (nullptr != pFuncListItem->m_pFuncNode)
        {
            emit functionSelected(*pFuncListItem->m_pFuncNode);
        }
    }
}

void CallGraphFuncList::onFunctionClicked(QTreeWidgetItem* pItem)
{
    if (nullptr != pItem)
    {
        CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(pItem);

        if (nullptr != pFuncListItem->m_pFuncNode)
        {
            emit functionSelected(*pFuncListItem->m_pFuncNode);
        }
    }
}

void CallGraphFuncList::sortIndicatorChanged(int col, Qt::SortOrder order)
{
    m_lastSortColumn = col;
    m_lastSortOrder = order;
}

void CallGraphFuncList::selectAFunction(const FunctionGraph::Node& funcNode)
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

            if (&funcNode == pFuncListItem->m_pFuncNode)
            {
                if (!pFuncListItem->isHidden())
                {
                    pFuncListItem->setSelected(true);
                    m_pFuncTable->scrollToItem(pFuncListItem);
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
    if ((column < CALLGRAPH_FUNCTION_NAME) || (column >= CALLGRAPH_FUNCTION_OFFSET))
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

CallGraphFuncListItem* CallGraphFuncList::FindTopLevelItem(const FunctionGraph::Node& funcNode)
{
    CallGraphFuncListItem* pFuncListItem = nullptr;

    for (int itemIndex = 0, itemsCount = m_pFuncTable->topLevelItemCount(); itemIndex < itemsCount; ++itemIndex)
    {
        CallGraphFuncListItem* pCurrFuncListItem = static_cast<CallGraphFuncListItem*>(m_pFuncTable->topLevelItem(itemIndex));

        if (nullptr != pCurrFuncListItem && nullptr != pCurrFuncListItem->m_pFuncNode)
        {
            if (&funcNode == pCurrFuncListItem->m_pFuncNode)
            {
                pFuncListItem = pCurrFuncListItem;
                break;
            }
        }
    }

    return pFuncListItem;
}

CallGraphFuncListItem* CallGraphFuncList::AddTopLevelItem(const FunctionGraph::Node& funcNode,
                                                          unsigned pathsNumber, gtUInt64 selfCount, gtUInt64 deepCount)
{
    CallGraphFuncListItem* pFuncListItem = new CallGraphFuncListItem(&funcNode);
    m_pFuncTable->addTopLevelItem(pFuncListItem);

    CssFunctionMetadata* pMetadata = static_cast<CssFunctionMetadata*>(funcNode.m_val);

    pMetadata->m_selfCount = selfCount;
    pMetadata->m_deepCount = deepCount;

    if (nullptr == pMetadata->m_pModule || nullptr == pMetadata->m_pFunction ||
        pMetadata->m_pModule->isUnchartedFunction(*pMetadata->m_pFunction))
    {
        pFuncListItem->setToolTip(CALLGRAPH_FUNCTION_NAME, QString::fromWCharArray(L"No debug info available"));
    }

    pFuncListItem->setText(CALLGRAPH_FUNCTION_NAME, pMetadata->m_funcName);

    if (0ULL != selfCount)
    {
        pFuncListItem->setData(CALLGRAPH_FUNCTION_SELF_SAMPLES, Qt::DisplayRole, QVariant(static_cast<qulonglong>(selfCount)));
        pFuncListItem->setTextAlignment(CALLGRAPH_FUNCTION_SELF_SAMPLES, Qt::AlignLeft | Qt::AlignVCenter);
    }
    else
    {
        pFuncListItem->setText(CALLGRAPH_FUNCTION_SELF_SAMPLES, "");
    }

    if (0ULL != deepCount)
    {
        pFuncListItem->setData(CALLGRAPH_FUNCTION_DEEP_SAMPLES, Qt::DisplayRole, QVariant(static_cast<qulonglong>(deepCount)));
        pFuncListItem->setTextAlignment(CALLGRAPH_FUNCTION_DEEP_SAMPLES, Qt::AlignLeft | Qt::AlignVCenter);
    }
    else
    {
        pFuncListItem->setText(CALLGRAPH_FUNCTION_DEEP_SAMPLES, "");
    }

    pFuncListItem->setText(CALLGRAPH_FUNCTION_DEEP_SAMPLE_PERCENTAGE, "");

    if (0U != pathsNumber)
    {
        pFuncListItem->setData(CALLGRAPH_FUNCTION_PATH_CNT, Qt::DisplayRole,  QVariant(pathsNumber));
        pFuncListItem->setTextAlignment(CALLGRAPH_FUNCTION_PATH_CNT, Qt::AlignLeft | Qt::AlignVCenter);
    }
    else
    {
        pFuncListItem->setText(CALLGRAPH_FUNCTION_PATH_CNT, "");
    }

#ifdef CSS_FUNC_PATH_INFO
    //TODO: Remove path samples information!
    pFuncListItem->setText(CALLGRAPH_FUNCTION_PATH_SAMPLES, "");
    pFuncListItem->setText(CALLGRAPH_FUNCTION_PATH_SAMPLES_PER_PATH, "");
#endif

    gtString sourceInfo;

    if (nullptr != pMetadata->m_pFunction)
    {
        pMetadata->m_pFunction->getSourceInfo(sourceInfo);

        if (!sourceInfo.isEmpty())
        {
            pFuncListItem->setToolTip(CALLGRAPH_FUNCTION_SOURCE_FILE, acGTStringToQString(pMetadata->m_pFunction->getSourceFile()));
        }
    }

    pFuncListItem->setText(CALLGRAPH_FUNCTION_SOURCE_FILE, acGTStringToQString(sourceInfo));

    gtString modFileName;

    if (nullptr != pMetadata->m_pModule && !pMetadata->m_pModule->getPath().isEmpty())
    {
        pMetadata->m_pModule->extractFileName(modFileName);
        pFuncListItem->setToolTip(CALLGRAPH_FUNCTION_MODULE, acGTStringToQString(pMetadata->m_pModule->getPath()));
    }

    pFuncListItem->setText(CALLGRAPH_FUNCTION_MODULE, acGTStringToQString(modFileName));

    return pFuncListItem;
}

CallGraphFuncListItem* CallGraphFuncList::DisplayFunctions(CpuProfileCss& css, int precision)
{
    (void)precision; // Unused

    CallGraphFuncListItem* pSelectedItem = nullptr;

    if (m_pFuncTable != nullptr)
    {
        // Block the table model signals, otherwise the table is sorted while setting the data (which causes a mess).
        m_pFuncTable->blockSignals(true);

        clear();

        m_pFuncTable->setSortingEnabled(false);

        const bool displaySystemModules = CPUGlobalDisplayFilter::instance().m_displaySystemDLLs;

        const EventMaskType eventId = css.GetEventId();
        FunctionGraph& funcGraph = css.GetFunctionGraph();

        int totalFuncCount = 0;
        int shownFuncCount = 0;

        gtUInt64 selfCountMax = 0ULL;
        gtUInt64 deepCountMax = 0ULL;

        gtUInt64 totalCount = 0ULL;

        for (FunctionGraph::const_node_iterator itFunc = funcGraph.GetBeginNode(), itFuncEnd = funcGraph.GetEndNode();
             itFunc != itFuncEnd; ++itFunc)
        {
            const FunctionGraph::Node& funcNode = *itFunc;

            unsigned pathsNumber = 0U;
            gtUInt64 selfCount = 0ULL;
            gtUInt64 deepCount = 0ULL;

            for (PathIndexSet::const_iterator it = funcNode.m_pathIndices.begin(), itEnd = funcNode.m_pathIndices.end();
                 it != itEnd; ++it)
            {
                const FunctionGraph::Path& path = *funcGraph.GetPath(*it);
                const LeafFunctionList& leaves = path.GetData();

                bool isRootFound = false;

                for (FunctionGraph::Path::const_iterator itNode = path.begin(), itNodeEnd = path.end(); itNode != itNodeEnd; ++itNode)
                {
                    if (&*itNode == &funcNode)
                    {
                        isRootFound = true;
                        break;
                    }
                }

                const FunctionGraph::Node* pPrevLeafNode = nullptr;

                for (LeafFunctionList::const_iterator itLeaf = leaves.begin(), itLeafEnd = leaves.end(); itLeaf != itLeafEnd; ++itLeaf)
                {
                    const LeafFunction& leaf = *itLeaf;

                    if (eventId == EventMaskType(-1) || eventId == leaf.m_eventId)
                    {
                        const bool isCurrentNode = (&funcNode == leaf.m_pNode);

                        if (isCurrentNode)
                        {
                            selfCount += leaf.m_count;
                        }

                        if (isCurrentNode || isRootFound)
                        {
                            deepCount += leaf.m_count;

                            if (pPrevLeafNode != leaf.m_pNode)
                            {
                                pPrevLeafNode = leaf.m_pNode;
                                pathsNumber++;
                            }
                        }
                    }
                }
            }

            if (0U == pathsNumber)
            {
                continue;
            }

            totalCount += selfCount;
            totalFuncCount++;

            CallGraphFuncListItem* pFuncListItem = AddTopLevelItem(funcNode, pathsNumber, selfCount, deepCount);

            bool hidden = false;

            // If system libs hidden, determine whether this is a system library
            if (!displaySystemModules)
            {
                const CssFunctionMetadata* pMetadata = static_cast<const CssFunctionMetadata*>(funcNode.m_val);

                if (nullptr != pMetadata->m_pModule && pMetadata->m_pModule->isSystemModule())
                {
                    hidden = true;
                }
            }

            // Hide this item if required
            pFuncListItem->setHidden(hidden);

            if (!hidden)
            {
                shownFuncCount++;

                if (selfCountMax < selfCount)
                {
                    selfCountMax = selfCount;
                    pSelectedItem = pFuncListItem;
                }
                else if (0 == selfCountMax && deepCountMax < deepCount)
                {
                    deepCountMax = deepCount;
                    pSelectedItem = pFuncListItem;
                }
            }
        }

        if (0ULL != totalCount)
        {
            for (int itemIndex = 0, itemsCount = m_pFuncTable->topLevelItemCount(); itemIndex < itemsCount; ++itemIndex)
            {
                CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(m_pFuncTable->topLevelItem(itemIndex));

                if (nullptr != pFuncListItem)
                {
                    qulonglong deepCount = pFuncListItem->GetCountValue(CALLGRAPH_FUNCTION_DEEP_SAMPLES);

                    pFuncListItem->SetPercentageValue(CALLGRAPH_FUNCTION_DEEP_SAMPLE_PERCENTAGE,
                                                      static_cast<double>(deepCount),
                                                      static_cast<double>(totalCount));
                }
            }
        }

        SetFunctionNameHeader(totalFuncCount, shownFuncCount);

        m_pFuncTable->setSortingEnabled(true);

        // Sort the table with the last sorted parameters or with default:
        if (m_lastSortColumn > 0)
        {
            m_pFuncTable->sortByColumn(m_lastSortColumn, Qt::DescendingOrder);
        }
        else
        {
            m_pFuncTable->sortByColumn(CALLGRAPH_FUNCTION_DEEP_SAMPLE_PERCENTAGE, m_lastSortOrder);
        }

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
        HandleDisplayEmptyTableItem(shownFuncCount);
    }

    return pSelectedItem;
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
    m_pEmptyTableMsgItem = new CallGraphFuncListItem(nullptr);
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

CallGraphFuncListItem* CallGraphButterfly::AddTopLevelItem(FunctionsTreeCtrl& treeCtrl,
                                                           const CpuProfileModule* pModule,
                                                           const FunctionGraph::Node* pFuncNode,
                                                           const QString& funcName,
                                                           qulonglong samplesCount,
                                                           double totalDeepCount,
                                                           bool noSamples)
{
    CallGraphFuncListItem* pFuncListItem = new CallGraphFuncListItem(pFuncNode);

    GT_IF_WITH_ASSERT(pFuncListItem != nullptr)
    {
        treeCtrl.addTopLevelItem(pFuncListItem);

        pFuncListItem->setText(CALLGRAPH_BUTTERFLY_FUNCTION, funcName);

        pFuncListItem->setData(CALLGRAPH_BUTTERFLY_SAMPLES, Qt::DisplayRole, QVariant(samplesCount));
        pFuncListItem->setTextAlignment(CALLGRAPH_BUTTERFLY_SAMPLES, Qt::AlignLeft | Qt::AlignVCenter);

        if (0ULL != samplesCount && !noSamples)
        {
            double deepPercentage = static_cast<double>(samplesCount) / totalDeepCount * 100.00;
            GT_ASSERT(deepPercentage <= 100.0);
            pFuncListItem->setData(CALLGRAPH_BUTTERFLY_PERCENTAGE, Qt::DisplayRole, QVariant(deepPercentage));
        }
        else
        {
            pFuncListItem->setText(CALLGRAPH_BUTTERFLY_PERCENTAGE, QString());
        }


        gtString modFileName;

        if (nullptr != pModule && !pModule->getPath().isEmpty())
        {
            pModule->extractFileName(modFileName);
            pFuncListItem->setToolTip(CALLGRAPH_BUTTERFLY_MODULE, acGTStringToQString(pModule->getPath()));
        }

        pFuncListItem->setText(CALLGRAPH_BUTTERFLY_MODULE, acGTStringToQString(modFileName));
    }

    return pFuncListItem;
}

void CallGraphButterfly::SetParentsFunction(CpuProfileCss& css, const FunctionGraph::Node& funcNode)
{
    (void)css; // Unused

    // Block the table model signals, otherwise the table is sorted while setting the data (which causes a mess).
    m_pParentsTreeControl->blockSignals(true);

    m_pParentsTreeControl->clear();

    const bool displaySystemModules = CPUGlobalDisplayFilter::instance().m_displaySystemDLLs;

    const CssFunctionMetadata* pMetadata = static_cast<const CssFunctionMetadata*>(funcNode.m_val);
    const double totalDeepCount = static_cast<double>(pMetadata->m_deepCount);
    const bool noSamples = (0ULL == (pMetadata->m_deepCount));
    qulonglong systemSamplesCount = 0ULL;

    for (FunctionGraph::NodeList::const_iterator it = funcNode.m_parents.begin(), itEnd = funcNode.m_parents.end(); it != itEnd; ++it)
    {
        const FunctionGraph::Node& parentNode = **it;

        pMetadata = static_cast<const CssFunctionMetadata*>(parentNode.m_val);

        if (displaySystemModules || nullptr == pMetadata->m_pModule || !pMetadata->m_pModule->isSystemModule())
        {
            AddTopLevelItem(*m_pParentsTreeControl,
                            pMetadata->m_pModule,
                            &parentNode,
                            pMetadata->m_funcName,
                            pMetadata->m_parentPathCount,
                            totalDeepCount,
                            noSamples);
        }
        else
        {
            systemSamplesCount += pMetadata->m_parentPathCount;
        }
    }

    if (0ULL != systemSamplesCount)
    {
        AddTopLevelItem(*m_pParentsTreeControl, nullptr, nullptr, "[system modules]", systemSamplesCount, totalDeepCount, noSamples);
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

void CallGraphButterfly::SetChildrenFunction(CpuProfileCss& css, const FunctionGraph::Node& funcNode)
{
    (void)css; // Unused

    // Block the table model signals, otherwise the table is sorted while setting the data (which causes a mess).
    m_pChildrenTreeControl->blockSignals(true);

    m_pChildrenTreeControl->clear();

    const bool displaySystemModules = CPUGlobalDisplayFilter::instance().m_displaySystemDLLs;

    const CssFunctionMetadata* pMetadata = static_cast<const CssFunctionMetadata*>(funcNode.m_val);
    const double totalDeepCount = static_cast<double>(pMetadata->m_deepCount);
    const bool noSamples = (0ULL == (pMetadata->m_deepCount));
    qulonglong systemSamplesCount = 0ULL;

    AddTopLevelItem(*m_pChildrenTreeControl,
                    pMetadata->m_pModule,
                    &funcNode,
                    "[self]",
                    pMetadata->m_selfCount,
                    totalDeepCount,
                    noSamples);

    for (FunctionGraph::NodeList::const_iterator it = funcNode.m_children.begin(), itEnd = funcNode.m_children.end(); it != itEnd; ++it)
    {
        const FunctionGraph::Node& childNode = **it;

        pMetadata = static_cast<const CssFunctionMetadata*>(childNode.m_val);

        if (displaySystemModules || nullptr == pMetadata->m_pModule || !pMetadata->m_pModule->isSystemModule())
        {
            AddTopLevelItem(*m_pChildrenTreeControl,
                            pMetadata->m_pModule,
                            &childNode,
                            pMetadata->m_funcName,
                            pMetadata->m_childPathCount,
                            totalDeepCount,
                            noSamples);
        }
        else
        {
            systemSamplesCount += pMetadata->m_childPathCount;
        }
    }

    if (0ULL != systemSamplesCount)
    {
        AddTopLevelItem(*m_pChildrenTreeControl, nullptr, nullptr, "[system modules]", systemSamplesCount, totalDeepCount, noSamples);
    }

    bool isSortIndShown = m_pChildrenTreeControl->header()->isSortIndicatorShown();
    // disable the sort indicator - for more accurate resizing (header size will not include sort indicator size)
    m_pChildrenTreeControl->header()->setSortIndicatorShown(false);

    m_pChildrenTreeControl->resizeColumnToContents(CALLGRAPH_BUTTERFLY_SAMPLES);
    // set back
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

            if (nullptr != pFuncListItem->m_pFuncNode)
            {
                emit functionSelected(*pFuncListItem->m_pFuncNode);
            }
        }
    }
}


// ****************************
// Class CallGraphTab
// ***************************


SessionCallGraphView::SessionCallGraphView(QWidget* pParent, CpuSessionWindow* pSessionWindow, afApplicationTreeItemData* pDisplayedSessionItemData) :
    DataTab(pParent, pSessionWindow),
    m_pButterfly(nullptr),
    m_pid(0U),
    m_pPidComboAction(nullptr),
    m_pFuncTable(nullptr),
    m_pPathFuncTable(nullptr),
    m_pCss(nullptr),
    m_pFuncNodeSelected(nullptr),
    m_pSplitter(nullptr),
    m_pHotSpotIndicatorComboBoxAction(nullptr)
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

}


SessionCallGraphView::~SessionCallGraphView()
{
    //since the CpuProfileCss isn't a QWidget, we need to free the memory
    if (nullptr != m_pCss)
    {
        delete m_pCss;
    }

    if (nullptr != m_pSplitter)
    {
        delete m_pSplitter;
    }
}

void SessionCallGraphView::FunctionListSelectionDone(const FunctionGraph::Node& funcNode)
{
    if (m_pFuncNodeSelected != &funcNode && nullptr != m_pCss)
    {
        m_pFuncNodeSelected = &funcNode;

        m_pCss->SetEventId(getFilterEvent());

        ShowPaths(*m_pCss, funcNode);
        ShowParentChild(*m_pCss, funcNode);
        emit functionSelected(funcNode);
    }
}

void SessionCallGraphView::editSource(const FunctionGraph::Node& funcNode)
{
    if (nullptr != m_pCss)
    {
        const CssFunctionMetadata* pMetadata = static_cast<const CssFunctionMetadata*>(funcNode.m_val);

        if (nullptr != pMetadata->m_pModule)
        {
            emit functionActivated(funcNode.m_key, m_pid, SHOW_ALL_TIDS, pMetadata->m_pModule);
        }
    }
}

// The display is for a particular PID
bool SessionCallGraphView::Display(const QString& caption, unsigned int pid)
{
    m_pid = 0U;
    m_indexOffset = CALLGRAPH_OFFSET_INDEX;
    m_sessionFile.setFile(caption);

    // Add all PIDs sampled for this module
    PidProcessMap* pProcMap = m_pProfileReader->getProcessMap();

    if (nullptr == pProcMap)
    {
        qApp->restoreOverrideCursor();
        acMessageBox::instance().critical(afGlobalVariablesManager::ProductNameA(), "Unable to get the css data");
        return false;
    }

    // Fill the process IDs combo box:
    fillPIDComb(pid);

    // Fill the hot spot indicator combo box:
    bool rcHotSpot = fillHotspotIndicatorCombo();
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
    Qt::SortOrder sortOrderOfFunctionTable = m_pFuncTable->functionsTreeControl()->header()->sortIndicatorOrder();
    int columnIndexOfFunctionTable = m_pFuncTable->functionsTreeControl()->header()->sortIndicatorSection();
    setEnableOnlySystemDllInfilterDlg(true);

    const bool displaySystemModules = CPUGlobalDisplayFilter::instance().m_displaySystemDLLs;
    OnDisplaySettingsClicked();

    if (displaySystemModules != CPUGlobalDisplayFilter::instance().m_displaySystemDLLs)
    {
        if (CPUGlobalDisplayFilter::instance().m_displaySystemDLLs)
        {
            OnIncludeSystemModules();
        }
        else
        {
            OnExcludeSystemModules();
        }
    }

    m_pFuncTable->sortByColumn(columnIndexOfFunctionTable, sortOrderOfFunctionTable);
}


// We need to update the display including system libraries
void SessionCallGraphView::OnIncludeSystemModules()
{
    GT_IF_WITH_ASSERT(m_pCss != nullptr)
    {
        qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

        CallGraphFuncListItem* pFuncListItem = nullptr;

        GT_IF_WITH_ASSERT(nullptr != m_pFuncTable)
        {
            pFuncListItem = m_pFuncTable->UnhideAllItems();
        }

        if (nullptr != pFuncListItem && nullptr != pFuncListItem->m_pFuncNode)
        {
            const FunctionGraph::Node& funcNode = *pFuncListItem->m_pFuncNode;

            emit functionSelected(funcNode);
            ShowPaths(*m_pCss, funcNode);
            ShowParentChild(*m_pCss, funcNode);
        }

        qApp->restoreOverrideCursor();
    }
}

// We need to update the display excluding system libraries
void SessionCallGraphView::OnExcludeSystemModules()
{
    GT_IF_WITH_ASSERT(m_pCss != nullptr)
    {
        qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

        GT_IF_WITH_ASSERT(nullptr != m_pFuncTable)
        {
            CallGraphFuncListItem* pFuncListItem = m_pFuncTable->HideSystemModules(*m_pCss);

            if (nullptr != pFuncListItem && nullptr != pFuncListItem->m_pFuncNode)
            {
                const FunctionGraph::Node& funcNode = *pFuncListItem->m_pFuncNode;

                emit functionSelected(funcNode);
                ShowPaths(*m_pCss, funcNode);
                ShowParentChild(*m_pCss, funcNode);
            }
            else
            {
                m_pPathFuncTable->clear();
                m_pButterfly->clear();
            }
        }

        qApp->restoreOverrideCursor();
    }
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
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pDisplayedSessionItemData != nullptr)
    {
        if (pid != m_pid || nullptr == m_pCss)
        {
            m_pid = pid;
            m_precision = afGlobalVariablesManager::instance().floatingPointPrecision();

            m_pFuncTable->clear();

            if (nullptr != m_pCss)
            {
                delete m_pCss;
            }

            SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(m_pDisplayedSessionItemData->extendedItemData());
            m_pCss = new CpuProfileCss(*m_pParentSessionWindow, *pSessionData);


            PROFILE_OPTIONS* pao = CpuProfilingOptions::instance().options();
            QString debugSearch = pao->debugSearchPaths;
            QString symDir;
            QString symList;

            if (false != pao->enableSymServer)
            {
                symDir = pao->symbolDownloadDir;
                gtUInt32 useSymbols = pao->useSymSrvMask;
                symList = pao->symSrvList;

                if (true == symList.isEmpty())
                {
                    QStringList servers = symList.split(';', QString::SkipEmptyParts);
                    symList.clear();

                    for (int i = 0; i < servers.size(); ++i)
                    {
                        if ((useSymbols & (1 << i)) > 0)
                        {
                            symList += servers.at(i) + ";";
                        }
                    }
                }
            }

            // The css file is located in the session directory
            if (m_pCss->Initialize(m_sessionFile.absolutePath(), pid))
            {
                m_pCss->SetEventId(getFilterEvent());
                CallGraphFuncListItem* pFuncListItem = m_pFuncTable->DisplayFunctions(*m_pCss, 1); // Requested by Doron (1 decimal of precision)

                if (nullptr != pFuncListItem && nullptr != pFuncListItem->m_pFuncNode)
                {
                    const FunctionGraph::Node& funcNode = *pFuncListItem->m_pFuncNode;

                    ShowPaths(*m_pCss, funcNode);
                    ShowParentChild(*m_pCss, funcNode);
                    emit functionSelected(funcNode);
                }
                else
                {
                    m_pPathFuncTable->clear();
                    m_pButterfly->clear();
                }
            }
            else
            {
                acMessageBox::instance().critical(CPU_PROF_MESSAGE, CP_callGraphViewFailedToReadCss);
                delete m_pCss;
                m_pCss = nullptr;
            }
        }
        else
        {
            const EventMaskType eventId = getFilterEvent();

            if (m_pCss->GetEventId() != eventId)
            {
                m_pFuncTable->clear();

                m_pCss->SetEventId(eventId);
                CallGraphFuncListItem* pFuncListItem = m_pFuncTable->DisplayFunctions(*m_pCss, 1); // Requested by Doron (1 decimal of precision)

                if (nullptr != m_pFuncNodeSelected)
                {
                    CallGraphFuncListItem* pSelectedItem = m_pFuncTable->FindTopLevelItem(*m_pFuncNodeSelected);

                    if (nullptr != pSelectedItem)
                    {
                        if (!pSelectedItem->isHidden())
                        {
                            pFuncListItem = pSelectedItem;
                        }
                    }
                }

                if (nullptr != pFuncListItem && nullptr != pFuncListItem->m_pFuncNode)
                {
                    const FunctionGraph::Node& funcNode = *pFuncListItem->m_pFuncNode;

                    m_pFuncNodeSelected = &funcNode;

                    ShowPaths(*m_pCss, funcNode);
                    ShowParentChild(*m_pCss, funcNode);
                    emit functionSelected(funcNode);
                }
                else
                {
                    m_pPathFuncTable->clear();
                    m_pButterfly->clear();
                }
            }
        }
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

void SessionCallGraphView::ShowParentChild(CpuProfileCss& css, const FunctionGraph::Node& funcNode)
{
    if (nullptr != m_pButterfly)
    {
        m_pButterfly->clear();

        const CssFunctionMetadata* pMetadata = static_cast<const CssFunctionMetadata*>(funcNode.m_val);

        m_pButterfly->setWindowTitle(tr("Immediate Parents and Children of Function: <b>") + pMetadata->m_funcName + "</b>");

        m_pButterfly->SetChildrenFunction(css, funcNode);
        m_pButterfly->SetParentsFunction(css, funcNode);
    }
}

void SessionCallGraphView::ShowPaths(CpuProfileCss& css, const FunctionGraph::Node& funcNode)
{
    if (nullptr != m_pPathFuncTable)
    {
        qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

        m_pPathFuncTable->SetFunctionPath(css, funcNode);

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
    (void)(index); // Unused

    // Sanity check:
    const QComboBox* pHotSpotIndicatorComboBox = TopToolbarComboBox(m_pHotSpotIndicatorComboBoxAction);
    GT_IF_WITH_ASSERT((nullptr != m_pParentSessionWindow) && (nullptr != m_pParentSessionWindow->sessionDisplaySettings()) && (pHotSpotIndicatorComboBox != nullptr))
    {
        // FIXME: Add hot spot based filter
        QString eventName = pHotSpotIndicatorComboBox->currentText();
        m_cuurentColSpec = m_pParentSessionWindow->sessionDisplaySettings()->getColumnSpecFromEventName(eventName);

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

bool SessionCallGraphView::fillHotspotIndicatorCombo()
{
    bool retVal = false;

    CpuProfileInfo* pProfileInfo = m_pProfileReader->getProfileInfo();
    GT_IF_WITH_ASSERT((m_pHotSpotIndicatorComboBoxAction != nullptr) && (m_pDisplayedSessionItemData != nullptr) && (m_pParentSessionWindow != nullptr)
                      && (m_pParentSessionWindow->sessionDisplaySettings() != nullptr) && (pProfileInfo != nullptr))
    {
        // Find the profile type:
        SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(m_pDisplayedSessionItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pSessionData != nullptr)
        {
            QStringList hotSpotColumns;

            QString filterName, origFilterName;

            if ((pSessionData->m_profileTypeStr == PM_profileTypeAssesPerformance) ||
                (pSessionData->m_profileTypeStr == PM_profileTypeTimeBased) ||
                (pSessionData->m_profileTypeStr == PM_profileTypeInstructionBasedSampling) ||
                (pSessionData->m_profileTypeStr == PM_profileTypeInvestigateBranching) ||
                (pSessionData->m_profileTypeStr == PM_profileTypeInvestigateDataAccess) ||
                (pSessionData->m_profileTypeStr == PM_profileTypeInvestigateInstructionAccess) ||
                (pSessionData->m_profileTypeStr == PM_profileTypeInvestigateInstructionL2CacheAccess) ||
                (pSessionData->m_profileTypeStr == PM_profileTypeCustomProfile))
            {
                filterName = "All Data";
            }

            GT_IF_WITH_ASSERT(!filterName.isEmpty())
            {
                SessionDisplaySettings* pSessionDisplaySettings = CurrentSessionDisplaySettings();
                GT_IF_WITH_ASSERT(pSessionDisplaySettings != nullptr)
                {
                    // get strings for the selection combo box - as the options in display settings page when selecting "all data", but dont change settings page
                    origFilterName = pSessionDisplaySettings->m_displayFilterName;
                    pSessionDisplaySettings->m_displayFilterName = filterName;
                    pSessionDisplaySettings->calculateDisplayedColumns(m_pProfileReader->getTopologyMap());

                    for (int i = 0; i < (int)pSessionDisplaySettings->m_availableDataColumnCaptions.size(); i++)
                    {
                        hotSpotColumns << pSessionDisplaySettings->m_availableDataColumnCaptions[i];
                    }

                    // set settings page back after getting the strings
                    pSessionDisplaySettings->m_displayFilterName = origFilterName;

                    retVal = true;
                }
            }

            filterMonitoredEvent(hotSpotColumns);

            if (!hotSpotColumns.empty())
            {
                m_pHotSpotIndicatorComboBoxAction->UpdateStringList(hotSpotColumns);
            }

            // Enable the combo if there are strings in the list:
            m_pHotSpotIndicatorComboBoxAction->UpdateEnabled(hotSpotColumns.size() > 1);
        }
    }

    OnSelectHotSpotIndicator(0);

    return retVal;
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
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pProfileReader != nullptr) && (m_pPidComboAction != nullptr) && (m_pTopToolbar != nullptr) && (m_pParentSessionWindow != nullptr))
    {
        const QComboBox* pPidCombo = TopToolbarComboBox(m_pPidComboAction);
        GT_IF_WITH_ASSERT(pPidCombo != nullptr)
        {
            int index = -1;

            // Get the map for the css collected processes for this session:
            const QMap<ProcessIdType, QString>& cssCollectedProcesses = m_pParentSessionWindow->CollectedProcessesMap();

            QStringList cssProcessesList;
            QMap<ProcessIdType, QString>::const_iterator iter = cssCollectedProcesses.begin(),
                                                         iterEnd = cssCollectedProcesses.end();

            for (; iter != iterEnd; ++iter)
            {
                QFileInfo processFileInfo(iter.value());
                unsigned int pidVal = iter.key();

                if (pidVal == pid)
                {
                    index = cssProcessesList.count();
                }

                QString pidEntry = QString("%1(%2)").arg(processFileInfo.fileName()).arg(pidVal);
                cssProcessesList << pidEntry;
            }

            // Update the combo box with the list of processes:
            m_pPidComboAction->UpdateStringList(cssProcessesList);

            if (-1 == index)
            {
                index = 0;

                CpuProfileInfo* pProfileInfo = m_pProfileReader->getProfileInfo();

                if (nullptr != pProfileInfo)
                {
                    QString strLaunchProcess = acGTStringToQString(pProfileInfo->m_targetPath);
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

void  SessionCallGraphView::filterMonitoredEvent(QStringList& eventNameList)
{
    if (TBPVER_BEFORE_RI >= m_pProfileReader->getProfileInfo()->m_tbpVersion)
    {
        //For tbp version before TBPVER_BEFORE_RI eventwise information is not available
        //So we only display "All data" instead of listing monitoed events.
        eventNameList.clear();
    }
    else
    {
        // Sanity check:
        GT_IF_WITH_ASSERT((m_pParentSessionWindow != nullptr) && (m_pParentSessionWindow->sessionDisplaySettings() != nullptr))
        {
            int count =  eventNameList.size();
            QStringList finalList;

            EventsFile* pEventsFile = CurrentSessionDisplaySettings()->getEventsFile();

            for (int i = 0; i < count; ++i)
            {
                QString eName = eventNameList.at(i);
                ColumnSpec cSpec = m_pParentSessionWindow->sessionDisplaySettings()->getColumnSpecFromEventName(eName);

                if (ColumnInvalid == cSpec.type)
                {
                    continue;
                }

                if ((pEventsFile == nullptr) && (IsTimerEvent(cSpec.dataSelectLeft.eventSelect)))
                {
                    finalList.append(eName);
                }
                else if ((pEventsFile == nullptr))
                {
                    continue;
                }
                else if (ColumnValue == cSpec.type)
                {
                    CpuEvent cpuevent;
                    pEventsFile->FindEventByValue(cSpec.dataSelectLeft.eventSelect, cpuevent);

                    if (IsTimerEvent(cpuevent.value) || IsPmcEvent(cpuevent.value))
                    {
                        finalList.append(eName);
                    }
                    else if ((IsIbsFetchEvent(cpuevent.value) && (GetIbsFetchEvent() == cpuevent.value)) ||
                             (IsIbsOpEvent(cpuevent.value) && (GetIbsOpEvent() == cpuevent.value)))
                    {
                        finalList.append(eName);
                    }
                }
            }

            eventNameList = finalList;
        }
    }
}

void SessionCallGraphView::UpdateTableDisplay(unsigned int updateType)
{
    if (updateType & UPDATE_TABLE_REBUILD)
    {
        if (CPUGlobalDisplayFilter::instance().m_displaySystemDLLs)
        {
            OnIncludeSystemModules();
        }
        else
        {
            OnExcludeSystemModules();
        }
    }
}

void SessionCallGraphView::selectFunction(const QString& functionName, ProcessIdType pid)
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
        QList<QTreeWidgetItem*> itemsList = m_pFuncTable->functionsTreeControl()->findItems(functionName, Qt::MatchExactly);
        GT_IF_WITH_ASSERT(1 <= itemsList.size())
        {
            CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(itemsList.first());

            if (nullptr != pFuncListItem)
            {
                if (nullptr != pFuncListItem->m_pFuncNode)
                {
                    const FunctionGraph::Node& funcNode = *pFuncListItem->m_pFuncNode;

                    ShowPaths(*m_pCss, funcNode);
                    ShowParentChild(*m_pCss, funcNode);
                    emit functionSelected(funcNode);
                }
            }
        }
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
        bool rc = connect(m_pFuncTable, SIGNAL(functionSelected(const FunctionGraph::Node&)), SLOT(FunctionListSelectionDone(const FunctionGraph::Node&)));
        GT_ASSERT(rc);

        rc = connect(this, SIGNAL(functionSelected(const FunctionGraph::Node&)), m_pFuncTable, SLOT(selectAFunction(const FunctionGraph::Node&)));
        GT_ASSERT(rc);

        rc = connect(m_pFuncTable->functionsTreeControl(), SIGNAL(editSourceFile(const FunctionGraph::Node&)), SLOT(editSource(const FunctionGraph::Node&)));
        GT_ASSERT(rc);

        //allocate the butterfly widget
        m_pButterfly = new CallGraphButterfly(this);


        rc = connect(m_pButterfly, SIGNAL(functionSelected(const FunctionGraph::Node&)), SLOT(FunctionListSelectionDone(const FunctionGraph::Node&)));
        GT_ASSERT(rc);

        m_pPathFuncTable = new CallGraphPathFuncList(this);

        m_pPathFuncTable->setWindowTitle("Paths");
        m_pPathFuncTable->initialize();

        rc = connect(m_pPathFuncTable, SIGNAL(functionSelected(const FunctionGraph::Node&)), SLOT(FunctionListSelectionDone(const FunctionGraph::Node&)));
        GT_ASSERT(rc);

        rc = connect(m_pPathFuncTable->functionsTreeControl(), SIGNAL(editSourceFile(const FunctionGraph::Node&)), SLOT(editSource(const FunctionGraph::Node&)));
        GT_ASSERT(rc);

        m_pButterfly->Initialize();

        rc = connect(m_pButterfly->parentsTreeControl(), SIGNAL(editSourceFile(const FunctionGraph::Node&)), SLOT(editSource(const FunctionGraph::Node&)));
        GT_ASSERT(rc);

        rc = connect(m_pButterfly->childrenTreeControl(), SIGNAL(editSourceFile(const FunctionGraph::Node&)), SLOT(editSource(const FunctionGraph::Node&)));
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

void SessionCallGraphView::displayInFunctionView(const QString& functionName)
{
    // Get the tree instance:
    afApplicationCommands* pCommands = afApplicationCommands::instance();
    SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();

    GT_IF_WITH_ASSERT((pCommands != nullptr) && (pSessionViewCreator != nullptr))
    {
        afApplicationTree* pApplicationTree = pCommands->applicationTree();
        GT_IF_WITH_ASSERT((m_pDisplayedSessionItemData != nullptr) && (pApplicationTree != nullptr) && (!functionName.isEmpty()))
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
                            pFunctionsView->selectFunction(functionName, m_pid);
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
            if (nullptr != pFuncListItem->m_pFuncNode)
            {
                const CssFunctionMetadata* pMetadata = static_cast<const CssFunctionMetadata*>(pFuncListItem->m_pFuncNode->m_val);
                enableFunctionsAction = (0ULL != pMetadata->m_selfCount);
                enableSourceCodeAction = enableFunctionsAction && (nullptr != pMetadata->m_pModule);
            }
        }

        // Enable / disable the actions:
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

    if (nullptr != pFuncListItem)
    {
        if (nullptr != pFuncListItem->m_pFuncNode)
        {
            emit editSourceFile(*pFuncListItem->m_pFuncNode);
        }
    }
}

void FunctionsTreeCtrl::GoToFunctionsView()
{
    GT_IF_WITH_ASSERT(nullptr != m_pCallGraphTab)
    {
        CallGraphFuncListItem* pFuncListItem = static_cast<CallGraphFuncListItem*>(currentItem());

        if (nullptr != pFuncListItem)
        {
            QString functionName = pFuncListItem->text(0);
            GT_IF_WITH_ASSERT(!functionName.isEmpty())
            {
                m_pCallGraphTab->displayInFunctionView(functionName);
            }
        }
    }
}

void FunctionsTreeCtrl::PathIndicatorToggled(int state)
{
    SetPathIndicator(Qt::Checked == state);
}
