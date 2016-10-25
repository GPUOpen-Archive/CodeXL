//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/TraceTable.cpp $
/// \version $Revision: #53 $
/// \brief :  This file contains TraceTable
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/TraceTable.cpp#53 $
// Last checkin:   $DateTime: 2016/04/06 07:09:58 $
// Last edited by: $Author: rbober $
// Change list:    $Change: 567432 $
//=====================================================================

#include <qtIgnoreCompilerWarnings.h>

//Qt
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/src/afUtils.h>

#include "APIInfo.h"
#include "APITimelineItems.h"
#include "TraceTable.h"
#include "OccupancyInfo.h"
#include "APIColorMap.h"
#include <AMDTGpuProfiling/Util.h>
namespace boosticl = boost::icl;

TraceTableItem::TraceTableItem(const QString& strAPIPrefix, const QString& strApiName, APIInfo* pApiInfo, acTimelineItem* pTimelineItem, acTimelineItem* pDeviceBlock, OccupancyInfo* pOccupancyInfo) :
    m_parent(nullptr), m_startIndex(-1), m_endIndex(-1), m_pTimelineItem(pTimelineItem), m_pDeviceBlock(pDeviceBlock), m_pOccupancyInfo(pOccupancyInfo)
{
    // Fill the data structure with empty strings:
    m_data.reserve(TraceTableModel::TRACE_COLUMN_COUNT);

    for (int i = 0 ; i < TraceTableModel::TRACE_COLUMN_COUNT; i++)
    {
        m_data << "";
    }

    if (pApiInfo != nullptr)
    {
        if (pApiInfo->m_bHasDisplayableSeqId)
        {
            m_startIndex = (int)pApiInfo->m_uiDisplaySeqID;
        }

        m_data[TraceTableModel::TRACE_INTERFACE_COLUMN] = strApiName;
        m_data[TraceTableModel::TRACE_PARAMETERS_COLUMN] = QString::fromStdString(pApiInfo->m_ArgList);
        m_data[TraceTableModel::TRACE_RESULT_COLUMN] = QString::fromStdString(pApiInfo->m_strRet);

        QString strUniqueId = strAPIPrefix;
        strUniqueId.append('.').append(QString::number(pApiInfo->m_uiSeqID));
        m_uniqueId = QVariant::fromValue(strUniqueId);
    }
}

TraceTableItem::TraceTableItem(const QString& strAPIPrefix, const QString& strMarkerName, PerfMarkerEntry* pMarkerEntry, acTimelineItem* pTimelineItem) :
    m_parent(nullptr), m_startIndex(-1), m_endIndex(-1), m_pTimelineItem(pTimelineItem), m_pDeviceBlock(nullptr), m_pOccupancyInfo(nullptr)
{
    // Fill the data structure with empty strings:
    m_data.reserve(TraceTableModel::TRACE_COLUMN_COUNT);

    for (int i = 0 ; i < TraceTableModel::TRACE_COLUMN_COUNT; i++)
    {
        m_data << "";
    }

    if (pMarkerEntry != nullptr)
    {
        m_data[TraceTableModel::TRACE_INTERFACE_COLUMN] = strMarkerName;

        QString strUniqueId = strAPIPrefix;
        static int dummy = 0;
        dummy++;
        strUniqueId.append('.').append(QString::number(dummy));
        m_uniqueId = QVariant::fromValue(strUniqueId);
    }
}

TraceTableItem::~TraceTableItem()
{
    m_data.clear();
    qDeleteAll(m_children);
}

void TraceTableItem::AppendChild(TraceTableItem* child)
{
    TraceTableItem* curParent = child->GetParent();

    if (curParent != this)
    {
        if (curParent != nullptr)
        {
            curParent->m_children.removeAt(child->GetRow());
        }

        m_children.append(child);
        child->m_parent = this;
    }
}

void TraceTableItem::InsertChild(int index, TraceTableItem* child)
{
    TraceTableItem* curParent = child->GetParent();

    if (curParent != this)
    {
        if (curParent != nullptr)
        {
            curParent->m_children.removeAt(child->GetRow());
        }

        m_children.insert(index, child);
        child->m_parent = this;
    }
}

QVariant TraceTableItem::GetColumnData(int columnIndex) const
{
    TraceTableModel::TraceTableColIndex columnIndexId = (TraceTableModel::TraceTableColIndex)columnIndex;

    switch (columnIndexId)
    {
        case TraceTableModel::TRACE_INTERFACE_COLUMN:
        case TraceTableModel::TRACE_PARAMETERS_COLUMN:
        case TraceTableModel::TRACE_RESULT_COLUMN:
        {
            if (m_data.count() > columnIndex)
            {
                return m_data[columnIndex];
            }

            return QVariant();
        }

        case TraceTableModel::TRACE_CPU_TIME_COLUMN:
        case TraceTableModel::TRACE_DEVICE_TIME_COLUMN:
        {
            QString retVal;

            if (m_data.count() > columnIndex)
            {
                bool isFloat = false;
                float floatVal = m_data[columnIndex].toFloat(&isFloat);

                if (isFloat)
                {
                    // Do not show values less then 0.0001:
                    if (floatVal > 0.0001)
                    {
                        int precision = 4;

                        if (fmod(floatVal, (float)1.0) == 0.0)
                        {
                            precision = 0;
                        }

                        retVal = QLocale(QLocale::English).toString(floatVal, 'f', precision);
                    }
                }
            }

            return retVal;
        }

        case TraceTableModel::TRACE_INDEX_COLUMN:
        {
            if (m_startIndex >= 0)
            {
                QString retVal = QString::number(m_startIndex);

                if (m_endIndex >= 0)
                {
                    retVal.sprintf("%d-%d", m_startIndex, m_endIndex);
                }

                return retVal;
            }

            return QVariant();

        }

        case TraceTableModel::TRACE_DEVICE_BLOCK_COLUMN:
        {
            if (m_pDeviceBlock != nullptr)
            {
                return m_pDeviceBlock->text();
            }

            return QVariant();

        }

        case TraceTableModel::TRACE_OCCUPANCY_COLUMN:
        {
            if (m_pOccupancyInfo != nullptr && m_pOccupancyInfo->GetOccupancy() >= 0)
            {
                return Util::RemoveTrailingZero(QString("%1").number(m_pOccupancyInfo->GetOccupancy(), 'f', 2)).append('%');
            }

            return QVariant();

        }

        default:
        {
            return QVariant();
        }
    }
}


void TraceTableItem::SetColumnData(int columnIndex, QVariant data)
{
    TraceTableModel::TraceTableColIndex columnIndexId = (TraceTableModel::TraceTableColIndex)columnIndex;

    switch (columnIndexId)
    {
        case TraceTableModel::TRACE_INDEX_COLUMN:
        case TraceTableModel::TRACE_INTERFACE_COLUMN:
        case TraceTableModel::TRACE_PARAMETERS_COLUMN:
        case TraceTableModel::TRACE_RESULT_COLUMN:
        case TraceTableModel::TRACE_CPU_TIME_COLUMN:
        case TraceTableModel::TRACE_DEVICE_TIME_COLUMN:
        {
            if (m_data.count() > columnIndex)
            {
                m_data[columnIndex] = data;
            }

            break;
        }

        default:
        {
            GT_ASSERT_EX(false, L"Invalid data column index");
            break;
        }
    }
}


int TraceTableItem::GetRow() const
{
    int retVal = 0;

    if (m_parent != nullptr)
    {
        retVal = m_parent->m_children.indexOf(const_cast<TraceTableItem*>(this));
    }

    return retVal;
}

void TraceTableItem::UpdateIndices(int childStartIndex, int childEndIndex)
{
    if ((childEndIndex < 0) && (childStartIndex >= 0))
    {
        if (m_startIndex < 0)
        {
            m_startIndex = childStartIndex;
        }
        else
        {
            if (m_startIndex > childStartIndex)
            {
                m_startIndex = childStartIndex;
            }
        }

        if (m_endIndex < 0)
        {
            m_endIndex = childStartIndex;
        }
        else
        {
            if (m_endIndex < childStartIndex)
            {
                m_endIndex = childStartIndex;
            }
        }
    }

    // Go through the parents, and update its indices, until you get to the root item
    TraceTableItem* pParent = m_parent;

    while (pParent != nullptr)
    {
        if (pParent->m_startIndex < 0)
        {
            pParent->m_startIndex = m_startIndex;
        }

        if (pParent->m_endIndex < 0)
        {
            pParent->m_endIndex = m_endIndex;
        }
        else
        {
            if (pParent->m_endIndex < m_endIndex)
            {
                pParent->m_endIndex = m_endIndex;
            }
        }

        pParent = pParent->m_parent;
    }
}


TraceTableModel::TraceTableModel(QObject* parent) : QAbstractItemModel(parent), m_isInitialized(false), m_shouldExpandBeEnabled(false)
{
    m_pRootItem = new TraceTableItem("", "", nullptr, nullptr, nullptr, nullptr);

}

TraceTableModel::~TraceTableModel()
{
    SAFE_DELETE(m_pRootItem);
}

void TraceTableModel::SetVisualProperties(const QColor& defaultForegroundColor, const QColor& linkColor, const QFont& font)
{
    // Build the list of headers:
    BuildHeaderData();


    m_defaultForegroundColor = defaultForegroundColor;
    m_linkColor = linkColor;
    m_font = font;
    m_underlineFont = m_font;
    m_underlineFont.setUnderline(true);
}

TraceTableItem* TraceTableModel::AddTraceItem(const QString& strAPIPrefix, const QString& strApiName, APIInfo* pApiInfo, acTimelineItem* pTimelineItem, acTimelineItem* pDeviceBlock, OccupancyInfo* pOccupancyInfo)
{
    TraceTableItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT((pApiInfo != nullptr) && (pTimelineItem != nullptr))
    {
        TraceTableItem* pParent = nullptr;

        pRetVal = new TraceTableItem(strAPIPrefix, strApiName, pApiInfo, pTimelineItem, pDeviceBlock, pOccupancyInfo);


        // Get the start and end time for the current pTableItem:
        quint64 startTime = pApiInfo->m_ullStart;
        quint64 endTime = pApiInfo->m_ullEnd;

        float cpuTimeSec = (float)(endTime - startTime) / 1000000;
        QVariant cpuTimeVar;
        cpuTimeVar.setValue(cpuTimeSec);
        pRetVal->SetColumnData(TRACE_CPU_TIME_COLUMN, cpuTimeVar);

        if (pDeviceBlock != nullptr)
        {
            float gpuTimeSec = (float)(pDeviceBlock->endTime() - pDeviceBlock->startTime()) / 1000000;
            QVariant gpuTimeVar;
            gpuTimeVar.setValue(gpuTimeSec);
            pRetVal->SetColumnData(TRACE_DEVICE_TIME_COLUMN, gpuTimeVar);
        }

        // Go through the existing API items, and look for an appropriate parent:
        auto iter = m_apiCallsTraceItemsMap.lower_bound(endTime);

        if (iter != m_apiCallsTraceItemsMap.end() && iter != m_apiCallsTraceItemsMap.begin())
        {
            auto iterPrev = std::prev(iter);

            quint64 prevTime = iterPrev->second->GetTimelineItem()->startTime();

            if (prevTime < startTime)
            {
                pParent = iterPrev->second;
            }
        }

        if (pParent != nullptr)
        {
            if (pParent != m_pRootItem)
            {
                m_shouldExpandBeEnabled = true;
            }

            pParent->AppendChild(pRetVal);
        }
        else
        {
            // Add the pTableItem to the map:
            m_apiCallsTraceItemsMap[endTime] = pRetVal;
        }
    }

    return pRetVal;
}


TraceTableItem* TraceTableModel::AddTopLevelTraceItem(const QString& strAPIPrefix, const QString& strApiName, APIInfo* pApiInfo, acTimelineItem* pTimelineItem, acTimelineItem* pDeviceBlock, OccupancyInfo* pOccupancyInfo)
{
    TraceTableItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT((pApiInfo != nullptr) && (pTimelineItem != nullptr))
    {
        pRetVal = new TraceTableItem(strAPIPrefix, strApiName, pApiInfo, pTimelineItem, pDeviceBlock, pOccupancyInfo);


        // Get the start and end time for the current pTableItem:
        quint64 startTime = pApiInfo->m_ullStart;
        quint64 endTime = pApiInfo->m_ullEnd;

        float cpuTimeSec = (float)(endTime - startTime) / 1000000;
        QVariant cpuTimeVar;
        cpuTimeVar.setValue(cpuTimeSec);
        pRetVal->SetColumnData(TRACE_CPU_TIME_COLUMN, cpuTimeVar);

        if (pDeviceBlock != nullptr)
        {
            float gpuTimeSec = (float)(pDeviceBlock->endTime() - pDeviceBlock->startTime()) / 1000000;
            QVariant gpuTimeVar;
            gpuTimeVar.setValue(gpuTimeSec);
            pRetVal->SetColumnData(TRACE_DEVICE_TIME_COLUMN, gpuTimeVar);
        }

        // Add the pTableItem to the map:
        m_apiCallsTraceItemsMap[endTime] = pRetVal;
    }

    return pRetVal;
}

TraceTableItem* TraceTableModel::AddTraceItem(const QString& strAPIPrefix, const QString& strMarkerName, PerfMarkerEntry* pMarkerEntry)
{
    TraceTableItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(pMarkerEntry != nullptr)
    {
        TraceTableItem* pParent = nullptr;

        // Create the new table pTableItem:
        pRetVal = new TraceTableItem(strAPIPrefix, strMarkerName, pMarkerEntry, nullptr);

        if (!m_openedPerfMarkerItemsStack.isEmpty())
        {
            pParent = m_openedPerfMarkerItemsStack.top();
        }

        if (pParent != nullptr)
        {
            pParent->InsertChild(0, pRetVal);
        }

        m_openedPerfMarkerItemsStack.push_back(pRetVal);
    }

    return pRetVal;
}

TraceTableItem* TraceTableModel::CloseLastOpenedPerfMarker(acTimelineItem* pTimelineItem)
{
    TraceTableItem* pOpenedItem = m_openedPerfMarkerItemsStack.pop();
    GT_IF_WITH_ASSERT(pOpenedItem != nullptr)
    {
        // Set the timeline pTableItem:
        pOpenedItem->SetTimelineItem(pTimelineItem);

        // Calculate the CPU time:
        quint64 cpuTime = pTimelineItem->endTime() - pTimelineItem->startTime();
        float cpuTimeSec = (float)cpuTime / 1000000;
        QVariant cpuTimeVar;
        cpuTimeVar.setValue(cpuTimeSec);
        pOpenedItem->SetColumnData(TRACE_CPU_TIME_COLUMN, cpuTimeVar);

        QPair<quint64, quint64> range;
        range.first = pTimelineItem->startTime();
        range.second = pTimelineItem->endTime();
        m_perfMarkersTraceItemsMap[range] = pOpenedItem;
        m_markerIntervals += make_pair(boosticl::interval<quint64>::closed(range.first, range.second), pOpenedItem);
    }

    return pOpenedItem;
}


int TraceTableModel::rowCount(const QModelIndex& parent) const
{
    int retVal = 0;

    TraceTableItem* parentItem;

    if (parent.column() <= 0)
    {

        if (!parent.isValid())
        {
            parentItem = m_pRootItem;
        }
        else
        {
            parentItem = static_cast<TraceTableItem*>(parent.internalPointer());
        }

        retVal = parentItem->GetChildCount();
    }

    return retVal;
}

int TraceTableModel::columnCount(const QModelIndex& /*parent*/) const
{
    // revisit if we ever want different number of columns for different rows
    int retVal = m_headerData.count();
    return retVal;
}

QVariant TraceTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
    {
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    }
    else if (role == Qt::DisplayRole || role == Qt::ForegroundRole || role == Qt::FontRole || role == Qt::UserRole)
    {
        TraceTableItem* pTableItem = static_cast<TraceTableItem*>(index.internalPointer());

        GT_IF_WITH_ASSERT(pTableItem != nullptr)
        {
            if (role == Qt::DisplayRole)
            {
                return pTableItem->GetColumnData(index.column());
            }
            else if (role == Qt::ForegroundRole)
            {
                QColor retVal;

                if ((index.column() == TRACE_DEVICE_BLOCK_COLUMN && pTableItem->GetDeviceBlock() != nullptr) || (index.column() == TRACE_OCCUPANCY_COLUMN && pTableItem->GetOccupancyInfo() != nullptr))
                {
                    retVal = m_linkColor;
                }
                else if (index.column() == TRACE_INTERFACE_COLUMN)
                {
                    PerfMarkerTimelineItem* pMarker = qobject_cast<PerfMarkerTimelineItem*>(pTableItem->GetTimelineItem());

                    if (pMarker != nullptr)
                    {
                        retVal = APIColorMap::Instance()->GetPerfMarkersColor();
                    }
                    else
                    {
                        retVal = APIColorMap::Instance()->GetAPIColor(pTableItem->GetColumnData(TRACE_INTERFACE_COLUMN).toString(), m_defaultForegroundColor);
                    }
                }

                return QVariant::fromValue(retVal);
            }
            else if (role == Qt::FontRole)
            {
                if ((index.column() == TRACE_DEVICE_BLOCK_COLUMN && pTableItem->GetDeviceBlock() != nullptr) || (index.column() == TRACE_OCCUPANCY_COLUMN && pTableItem->GetOccupancyInfo() != nullptr))
                {
                    return QVariant::fromValue(m_underlineFont);
                }

                return QVariant::fromValue(m_font);
            }
            else if (role == Qt::TextAlignmentRole)
            {
                return QVariant(Qt::AlignLeft);
            }
            else if (role == Qt::UserRole)
            {
                return pTableItem->GetUniqueId();
            }
        }
    }

    return QVariant();
}

QVariant TraceTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section < m_headerData.count())
    {
        return QVariant(m_headerData[section]);
    }

    return QVariant();
}

Qt::ItemFlags TraceTableModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags retVal = 0;

    if (index.isValid())
    {
        retVal = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    return retVal;
}

QModelIndex TraceTableModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    TraceTableItem* parentItem;

    if (!parent.isValid())
    {
        parentItem = m_pRootItem;
    }
    else
    {
        parentItem = static_cast<TraceTableItem*>(parent.internalPointer());
    }

    TraceTableItem* childItem = parentItem->GetChild(row);

    if (childItem != nullptr)
    {
        return createIndex(row, column, childItem);
    }
    else
    {
        return QModelIndex();
    }
}

QModelIndex TraceTableModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    TraceTableItem* childItem = static_cast<TraceTableItem*>(index.internalPointer());
    TraceTableItem* parentItem = childItem->GetParent();

    if (parentItem == m_pRootItem)
    {
        return QModelIndex();
    }

    return createIndex(parentItem->GetRow(), 0, parentItem);
}

acTimelineItem* TraceTableModel::GetDeviceBlock(const QModelIndex& index)
{
    acTimelineItem* retVal = nullptr;

    if (index.isValid())
    {
        TraceTableItem* childItem = static_cast<TraceTableItem*>(index.internalPointer());
        retVal = childItem->GetDeviceBlock();
    }

    return retVal;
}

OccupancyInfo* TraceTableModel::GetOccupancyItem(const QModelIndex& index)
{
    OccupancyInfo* retVal = nullptr;

    if (index.isValid())
    {
        TraceTableItem* childItem = static_cast<TraceTableItem*>(index.internalPointer());
        retVal = childItem->GetOccupancyInfo();
    }

    return retVal;
}

bool TraceTableModel::InitializeModel()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_pRootItem != nullptr)
    {
        // Go through the marker items, and the API items in parallel, and add it to the table:
        auto apiIter = m_apiCallsTraceItemsMap.begin();
        auto apiIterEnd = m_apiCallsTraceItemsMap.end();

        auto markersIter = m_perfMarkersTraceItemsMap.begin();
        auto markersIterEnd = m_perfMarkersTraceItemsMap.end();

        int itemCount = m_apiCallsTraceItemsMap.size() + m_perfMarkersTraceItemsMap.size();

        if (m_perfMarkersTraceItemsMap.isEmpty())
        {
            m_pRootItem->ReserveChildrenCount(itemCount);
        }

        afProgressBarWrapper::instance().setProgressText(GPU_STR_TraceViewLoadingTraceTableItemsProgress);

        // Go over the API items and the markers item, and add them to the table:
        while ((apiIter != apiIterEnd) || (markersIter != markersIterEnd))
        {
            /// calculate next item and advance iterator
            TraceTableItem* pNextItemToAdd = CalculateNextTraceTableItem(markersIter, markersIterEnd, apiIter, apiIterEnd);
            TraceTableItem* pParent = GetNextItemParent(pNextItemToAdd);
            GT_IF_WITH_ASSERT(pParent != nullptr)
            {
                // Add the child to the appropriate parent:
                pParent->AppendChild(pNextItemToAdd);

                if (pParent != m_pRootItem)
                {
                    pParent->UpdateIndices(pNextItemToAdd->GetStartIndex(), pNextItemToAdd->GetEndIndex());
                }

                afProgressBarWrapper::instance().incrementProgressBar();
            }
        }

        m_apiCallsTraceItemsMap.clear();
        m_perfMarkersTraceItemsMap.clear();
        m_markerIntervals.clear();
        retVal = true;
    }

    m_isInitialized = retVal;
    return retVal;
}
/// returns items parent , by default it's a root item,unless overlapping marker item is found
TraceTableItem* TraceTableModel::GetNextItemParent(const TraceTableItem* pNextItemToAdd) const
{
    //// Look for the right parent for this item:
    /// By default all items are immediate children of the root node
    TraceTableItem* pParent = m_pRootItem;
    GT_IF_WITH_ASSERT(pNextItemToAdd != nullptr)
    {
        const quint64 nextItemStartTime = pNextItemToAdd->GetTimelineItem()->startTime();
        const quint64 nextItemEndTime = pNextItemToAdd->GetTimelineItem()->endTime();
        const auto nextItemTimeInterval = boosticl::interval<quint64>::right_open(nextItemStartTime, nextItemEndTime);

        auto itr = m_markerIntervals.find(nextItemTimeInterval);

        if (itr != m_markerIntervals.end())
        {
            auto pMarker = itr->second;

            //check if next item we're adding overlapped by the found marker , if so make it next item parent
            if (pMarker != pNextItemToAdd &&
                pMarker->GetTimelineItem() != nullptr &&
                pMarker->GetTimelineItem()->startTime() <= nextItemStartTime &&
                pMarker->GetTimelineItem()->endTime() >= nextItemEndTime)
            {
                pParent = pMarker;
            }
        }
    }
    return pParent;
}

/// calculates next item and advance one of iterators - marker or api iterator
TraceTableItem* TraceTableModel::CalculateNextTraceTableItem(PerfMarkersMapItr& markersIter, const PerfMarkersMapItr& markersIterEnd,
                                                             ApiCallsTraceMapItr& apiIter, const ApiCallsTraceMapItr& apiIterEnd) const
{
    TraceTableItem* pNextItemToAdd = nullptr;
    TraceTableItem* pCurrentAPI = (apiIter != apiIterEnd) ? apiIter->second : nullptr;
    TraceTableItem* pCurrentMarker = (markersIter != markersIterEnd) ? markersIter.value() : nullptr;

    //no more markers --> api is next item
    if ((pCurrentAPI != nullptr) && (pCurrentMarker == nullptr))
    {
        pNextItemToAdd = pCurrentAPI;
        apiIter++;
    }
    // no more api's --> marker is the next item
    else if ((pCurrentAPI == nullptr) && (pCurrentMarker != nullptr))
    {
        pNextItemToAdd = pCurrentMarker;
        markersIter++;
    }
    //choose by start time
    else
    {
        // We have both next API and markers. Compare the start time to decide which of them should be added next:
        quint64 apiStart = pCurrentAPI->GetTimelineItem()->startTime();
        quint64 markerStart = markersIter.key().first;

        // Next item should be an API item:
        if (apiStart <= markerStart)
        {
            pNextItemToAdd = pCurrentAPI;
            apiIter++;
        }

        // Next one should be a marker item:
        else
        {
            pNextItemToAdd = pCurrentMarker;
            markersIter++;
        }
    }

    return pNextItemToAdd;
}


void TraceTableModel::SetAPICallsNumber(unsigned int apiNum)
{
    // set reserved items - for checking before tab creation
    m_reservedApiCallsTraceItems = apiNum;
}

bool TraceTableModel::ExportToCSV(const QString& outputFilePath)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pRootItem != nullptr)
    {
        // Define a file handle:
        QFile fileHandle(outputFilePath);

        // Open the file for write:
        bool rcOpen = fileHandle.open(QFile::WriteOnly | QFile::Truncate);

        if (rcOpen)
        {
            QTextStream data(&fileHandle);
            QStringList strList;


            // Get the row count:
            int rowCount = m_pRootItem->GetChildCount();

            // Write the headers:
            for (int i = 0; i < TRACE_COLUMN_COUNT; ++i)
            {
                QString headerString = headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
                strList << AC_STR_QuotSpace + headerString + AC_STR_QuotSpace;
            }

            // Write a new line:
            data << strList.join(AC_STR_Tab) << AC_STR_NewLineA;
            strList.clear();

            // Go through the rows and export each of them:
            for (int row = 0; row < rowCount; ++row)
            {
                QString rowStr;
                TraceTableItem* pRowItem = m_pRootItem->GetChild(row);

                for (int column = 0; column < TRACE_COLUMN_COUNT; ++column)
                {
                    QString colString = pRowItem->GetColumnData(column).toString();
                    strList << AC_STR_QuotSpace + colString + AC_STR_QuotSpace;
                }

                data << strList.join(AC_STR_Tab) << AC_STR_NewLineA;
                strList.clear();
            }

            // Close the file:
            fileHandle.close();

            retVal = true;
        }
    }

    return retVal;
}


void TraceTableModel::BuildHeaderData()
{
    m_headerData.clear();

    // Add all the columns that are not filtered:
    m_headerData.insert(TRACE_INDEX_COLUMN, tr("Call Index"));
    m_headerData.insert(TRACE_INTERFACE_COLUMN, tr("Interface"));
    m_headerData.insert(TRACE_PARAMETERS_COLUMN, tr("Parameters"));
    m_headerData.insert(TRACE_RESULT_COLUMN, tr("Result"));
    m_headerData.insert(TRACE_DEVICE_BLOCK_COLUMN, tr("Device Block"));
    m_headerData.insert(TRACE_OCCUPANCY_COLUMN, tr("Kernel Occupancy"));
    m_headerData.insert(TRACE_CPU_TIME_COLUMN, tr("CPU Time"));
    m_headerData.insert(TRACE_DEVICE_TIME_COLUMN, tr("Device Time"));
}

TraceTable::TraceTable(QWidget* parent, unsigned int threadId) : QTreeView(parent), m_threadId(threadId)
{
    setSortingEnabled(false);
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setMouseTracking(true);

    // show highlighted API even when the view is inactive
    QPalette myPalette = palette();
    myPalette.setColor(QPalette::Inactive, QPalette::Highlight, myPalette.color(QPalette::Active, QPalette::Highlight));
    myPalette.setColor(QPalette::Inactive, QPalette::HighlightedText, myPalette.color(QPalette::Active, QPalette::HighlightedText));
    setPalette(myPalette);

    QHeaderView* horzHeader = header();

    horzHeader->setSectionResizeMode(QHeaderView::Interactive);

    horzHeader->setHighlightSections(false);
    setUniformRowHeights(true);
}

TraceTable::~TraceTable()
{
}

void TraceTable::resizeEvent(QResizeEvent* event)
{
    QTreeView::resizeEvent(event);
    int newWidth = event->size().width();

    if (event->oldSize().width() != newWidth)
    {
        QHeaderView* horzHeader = header();

        for (int i = 0; i < horzHeader->count(); i++)
        {
            int newSectionWidth = int(newWidth * GetSectionFillWeight(i));
            horzHeader->resizeSection(i, newSectionWidth);
        }
    }
}

float TraceTable::GetSectionFillWeight(int sectionIndex)
{
    // column widths as a percent of the total width (these values look best)
    static float IndexColumnFillWeight = .05f;
    static float InterfaceColumnFillWeight = .14f;
    static float ParametersColumnFillWeight = .40f;
    static float ResultColumnFillWeight = .09f;
    static float DeviceBlockColumnFillWeight = .09f;
    static float KernelOccupancyBlockColumnFillWeight = .09f;
    static float timeColumnFillWeight = .05f;

    TraceTableModel::TraceTableColIndex colIndex = (TraceTableModel::TraceTableColIndex)sectionIndex;

    switch (colIndex)
    {
        case TraceTableModel::TRACE_INDEX_COLUMN:
            return IndexColumnFillWeight;

        case TraceTableModel::TRACE_INTERFACE_COLUMN:
            return InterfaceColumnFillWeight;

        case TraceTableModel::TRACE_PARAMETERS_COLUMN:
            return ParametersColumnFillWeight;

        case TraceTableModel::TRACE_RESULT_COLUMN:
            return ResultColumnFillWeight;

        case TraceTableModel::TRACE_DEVICE_BLOCK_COLUMN:
            return DeviceBlockColumnFillWeight;

        case TraceTableModel::TRACE_OCCUPANCY_COLUMN:
            return KernelOccupancyBlockColumnFillWeight;

        case TraceTableModel::TRACE_CPU_TIME_COLUMN:
            return timeColumnFillWeight;

        case TraceTableModel::TRACE_DEVICE_TIME_COLUMN:
            return timeColumnFillWeight;

        default:
            return 0;
    }
}

void TraceTable::OnEditCopy()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(qApp != nullptr)
    {
        // Get the clipboard from the application:
        QClipboard* pClipboard = qApp->clipboard();
        GT_IF_WITH_ASSERT(pClipboard != nullptr)
        {
            // Get the selected items list:
            QModelIndexList selectedRowsList = selectionModel()->selectedRows();
            QModelIndexList selectedItemsList = selectionModel()->selectedIndexes();

            QString selectedText;

            if (!selectedRowsList.isEmpty())
            {
                for (int i = 0 ; i < model()->columnCount(); i++)
                {
                    if (!isColumnHidden(i))
                    {
                        selectedText.append(model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
                    }

                    if (i == model()->columnCount() - 1)
                    {
                        // Last column - add new line:
                        selectedText.append("\n");
                    }
                    else
                    {
                        selectedText.append(", ");
                    }
                }
            }

            QList<int> copiedLinesList;

            foreach (QModelIndex index, selectedItemsList)
            {
                // Get the current row:
                int currentRow = index.row();

                // If we did not copy this line context yet:
                if (!copiedLinesList.contains(currentRow))
                {
                    // We want to copy the text in full lines, so we search for all indices containing this row:
                    QModelIndexList currentRowIndexList;

                    foreach (QModelIndex tempIndex, selectedItemsList)
                    {
                        if (tempIndex.row() == currentRow)
                        {
                            if (!isColumnHidden(tempIndex.column()))
                            {
                                // Get the text for the current column:
                                QString currentColText = model()->data(tempIndex, Qt::DisplayRole).toString();

                                // Add the text:
                                selectedText.append(currentColText);

                                if (index.column() != model()->columnCount() - 1)
                                {
                                    selectedText.append(", ");
                                }
                            }
                        }
                    }

                    copiedLinesList << currentRow;

                    selectedText.append("\n");
                }
            }

            // Set the copied text to the clipboard:
            pClipboard->setText(selectedText);
        }
    }
}

void TraceTable::OnEditSelectAll()
{
    selectAll();
}

void TraceTable::onUpdateEdit_Copy(bool& isEnabled)
{
    isEnabled = (selectedIndexes().size() >= 1);
}

bool TraceTable::ShouldExpandBeEnabled() const
{
    bool retVal = false;
    TraceTableModel* pModel = qobject_cast<TraceTableModel*>(model());
    GT_IF_WITH_ASSERT(pModel != nullptr)
    {
        retVal = pModel->ShouldExpandBeEnabled();
    }

    return retVal;
}

int TraceTable::NumOfSelectedRows() const
{
    return selectionModel()->selectedRows().count();
}

bool TraceTable::ExportToCSV(const QString& outputFilePath)
{
    bool retVal = false;
    TraceTableModel* pModel = qobject_cast<TraceTableModel*>(model());
    GT_IF_WITH_ASSERT(pModel != nullptr)
    {
        retVal = pModel->ExportToCSV(outputFilePath);
    }

    return retVal;
}

void TraceTable::SetHiddenColumnsList(const QList<TraceTableModel::TraceTableColIndex>& hiddenColumnsList)
{
    if (qobject_cast<TraceTableModel*>(model()) != nullptr)
    {
        foreach (TraceTableModel::TraceTableColIndex index, hiddenColumnsList)
        {
            setColumnHidden(index, true);
        }
    }
}

