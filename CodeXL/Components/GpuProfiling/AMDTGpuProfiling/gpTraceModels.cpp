//------------------------------ gpTraceModels.cpp ------------------------------

#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTGpuProfiling/APIColorMap.h>
#include <AMDTGpuProfiling/gpTraceModels.h>
#include <AMDTGpuProfiling/gpTraceDataContainer.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

gpTableModelBase::gpTableModelBase(osThreadId threadID, gpTraceDataContainer* pDataContainer) : QAbstractItemModel(),
    m_pSessionDataContainer(pDataContainer),
    m_threadID(threadID)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        // Get the API type for the requested thread ID
        ProfileSessionDataItem::ProfileItemAPIType apiType = pDataContainer->APITypeForThread(threadID);

        ProfileSessionDataItem::GetListOfColumnIndices(apiType, m_columnIndicesVec);
    }
}


gpTableModelBase::gpTableModelBase(gpTraceDataContainer* pDataContainer, ProfileSessionDataItem::ProfileItemAPIType apiType) : QAbstractItemModel(),
m_pSessionDataContainer(pDataContainer)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        ProfileSessionDataItem::GetListOfColumnIndices(apiType, m_columnIndicesVec);
    }
}


gpTableModelBase::~gpTableModelBase()
{
}

int gpTableModelBase::rowCount(const QModelIndex& parent) const
{
    int retVal = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        if (parent.column() <= 0)
        {
            retVal = m_pSessionDataContainer->ThreadAPICount(m_threadID);
        }

    }
    return retVal;
}

int gpTableModelBase::columnCount(const QModelIndex& parent) const
{
    GT_UNREFERENCED_PARAMETER(parent);

    int retVal = m_columnIndicesVec.count();
    return retVal;
}

QVariant gpTableModelBase::data(const QModelIndex& index, int role) const
{
    QVariant retVal;

    if (index.isValid())
    {
        // Translate the column physical index to a logical column index
        ProfileSessionDataItem::ProfileSessionDataColumnIndex colIndex = ProfileSessionDataItem::SESSION_ITEM_UNKNOWN;
        GT_IF_WITH_ASSERT((index.column() < m_columnIndicesVec.size()) && index.column() >= 0)
        {
            colIndex = m_columnIndicesVec[index.column()];
        }

        if (role == Qt::TextAlignmentRole)
        {
            retVal = int(Qt::AlignLeft | Qt::AlignVCenter);
        }
        else if (role == Qt::DisplayRole || role == Qt::ForegroundRole || role == Qt::FontRole || role == Qt::UserRole || role == Qt::ToolTipRole || role == Qt::BackgroundColorRole)
        {
            ProfileSessionDataItem* pTableItem = GetItem(index);

            GT_IF_WITH_ASSERT(pTableItem != nullptr)
            {
                if (role == Qt::DisplayRole)
                {
                    retVal = pTableItem->GetColumnData(colIndex);
                }
                else if (role == Qt::ToolTipRole)
                {
                    retVal = pTableItem->GetColumnTooltip(colIndex);
                }
                else if (role == Qt::ForegroundRole)
                {
                    QColor color;

                    if ((colIndex == ProfileSessionDataItem::SESSION_ITEM_DEVICE_BLOCK_COLUMN) || (colIndex == ProfileSessionDataItem::SESSION_ITEM_OCCUPANCY_COLUMN))
                    {
                        color = m_linkColor;
                    }
                    else if (colIndex == ProfileSessionDataItem::SESSION_ITEM_INTERFACE_COLUMN || colIndex == ProfileSessionDataItem::SESSION_ITEM_CALL_COLUMN)
                    {
                        if (pTableItem->ItemType().m_itemMainType == ProfileSessionDataItem::PERF_MARKER_PROFILE_ITEM)
                        {
                            color = acQAMD_CYAN_OVERLAP_COLOUR;
                        }
                        else
                        {

                            unsigned int apiId;
                            pTableItem->GetAPIFunctionID(apiId);
                            color = APIColorMap::Instance()->GetAPIColor(pTableItem->ItemType(), apiId, m_defaultForegroundColor);
                        }
                    }

                    retVal = QVariant::fromValue(color);
                }
                else if (role == Qt::FontRole)
                {
                    if ((colIndex == ProfileSessionDataItem::SESSION_ITEM_DEVICE_BLOCK_COLUMN) || (colIndex == ProfileSessionDataItem::SESSION_ITEM_OCCUPANCY_COLUMN))
                    {
                        retVal = QVariant::fromValue(m_underlineFont);
                    }

                    retVal = QVariant::fromValue(m_font);
                }
                else if (role == Qt::TextAlignmentRole)
                {
                    retVal = QVariant(Qt::AlignLeft);
                }
                else if (role == Qt::BackgroundColorRole)
                {
                    QColor color = Qt::white;
                    ProfileSessionDataItem::ProfileItemType itemType = pTableItem->ItemType();

                    if ((itemType.m_itemMainType == ProfileSessionDataItem::DX12_API_PROFILE_ITEM) || (itemType.m_itemMainType == ProfileSessionDataItem::DX12_API_PROFILE_ITEM))
                    {
                        if ((itemType.m_itemSubType == FuncId_ID3D12GraphicsCommandList_BeginEvent) || (itemType.m_itemSubType == FuncId_ID3D12GraphicsCommandList_EndEvent) ||
                            (itemType.m_itemSubType == FuncId_ID3D12CommandQueue_BeginEvent) || (itemType.m_itemSubType == FuncId_ID3D12CommandQueue_EndEvent) ||
                            (itemType.m_itemSubType == FuncId_ID3D12GraphicsCommandList_SetMarker) || (itemType.m_itemSubType == FuncId_ID3D12CommandQueue_SetMarker))
                        {
                            color = Qt::yellow;
                        }
                    }

                    retVal = QVariant::fromValue(color);
                }
            }
        }

    }

    return retVal;
}

QVariant gpTableModelBase::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant retVal;

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        ProfileSessionDataItem::ProfileSessionDataColumnIndex colIndex = ProfileSessionDataItem::SESSION_ITEM_UNKNOWN;
        GT_IF_WITH_ASSERT((section < m_columnIndicesVec.size()) && section >= 0)
        {
            colIndex = m_columnIndicesVec[section];
        }
        retVal = QVariant(ProfileSessionDataItem::ColumnIndexTitle(colIndex));
    }
    else if (role == Qt::TextAlignmentRole)
    {
        retVal = QVariant(Qt::AlignLeft);
    }

    return retVal;
}

Qt::ItemFlags gpTableModelBase::flags(const QModelIndex& index) const
{
    Qt::ItemFlags retVal = 0;

    if (index.isValid())
    {
        retVal = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    return retVal;
}

QModelIndex gpTableModelBase::index(int row, int column, const QModelIndex& parent) const
{
    QModelIndex retVal;

    if (hasIndex(row, column, parent))
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
        {
            ProfileSessionDataItem* pParentItem = nullptr;

            if (!parent.isValid())
            {
                pParentItem = m_pSessionDataContainer->GetRootItem(m_threadID);
            }
            else
            {
                pParentItem = m_pSessionDataContainer->APIItem(m_threadID, parent.row());
            }

            ProfileSessionDataItem* pChildItem = pParentItem->GetChild(row);

            if (pChildItem != nullptr)
            {
                return createIndex(row, column, pChildItem);
            }
        }
    }

    return retVal;
}

QModelIndex gpTableModelBase::parent(const QModelIndex& index) const
{
    // There should be no parent, this is a table
    GT_UNREFERENCED_PARAMETER(index);
    return QModelIndex();

}


void gpTableModelBase::SetVisualProperties(const QColor& defaultForegroundColor, const QColor& linkColor, const QFont& font)
{
    m_defaultForegroundColor = defaultForegroundColor;
    m_linkColor = linkColor;
    m_font = font;
    m_underlineFont = m_font;
    m_underlineFont.setUnderline(true);
}

ProfileSessionDataItem* gpTableModelBase::GetItem(const QModelIndex& item) const
{
    ProfileSessionDataItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        pRetVal = m_pSessionDataContainer->APIItem(m_threadID, item.row());
    }

    return pRetVal;

}

SymbolInfo* gpTableModelBase::GetSymbolInfo(int callIndex)
{
    SymbolInfo* pRetVal = nullptr;

    // Sanity check
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        pRetVal = m_pSessionDataContainer->GetSymbolInfo(m_threadID, callIndex);
    }
    return pRetVal;
}

bool gpTableModelBase::SessionHasSymbolInformation()
{
    bool retVal = nullptr;

    // Sanity check
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        retVal = !m_pSessionDataContainer->SymbolTableMap().empty();
    }
    return retVal;
}


bool gpTableModelBase::ExportToCSV(const QString& outputFilePath)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        int threadCount = m_pSessionDataContainer->ThreadsCount();

        for (int i = 0; i < threadCount; i++)
        {
            osThreadId threadID = m_pSessionDataContainer->ThreadID(i);

            // Sanity check:
            GT_IF_WITH_ASSERT(m_pSessionDataContainer->GetRootItem(threadID) != nullptr)
            {
                QString threadFile = outputFilePath;
                //QString tableCaption = QString(GPU_STR_timeline_CPU_ThreadBranchName).arg(threadID);

                threadFile.append(osFilePath::osPathSeparator + QString(AC_STR_ExportThreadToCSVFileName).arg(QString::number(threadID)) + QString(GP_CSV_FileExtension));
                // Define a file handle:
                QFile fileHandle(threadFile);

                // Open the file for write:
                bool rcOpen = fileHandle.open(QFile::WriteOnly | QFile::Truncate);

                if (rcOpen)
                {
                    QTextStream data(&fileHandle);
                    QStringList strList;


                    // Get the row count:
                    int rowCount = m_pSessionDataContainer->ThreadAPICount(threadID);
                    int colCount = m_columnIndicesVec.count();

                    // Write the headers:
                    for (int i = 0; i < colCount; ++i)
                    {
                        QString headerString = headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
                        strList << AC_STR_QuotSpace + headerString + AC_STR_QuotSpace + AC_STR_CommaA;
                    }

                    // Write a new line:
                    data << strList.join(AC_STR_Tab) << AC_STR_NewLineA;
                    strList.clear();

                    // Go through the rows and export each of them:
                    for (int row = 0; row < rowCount; ++row)
                    {
                        QString rowStr;
                        ProfileSessionDataItem* pRowItem = m_pSessionDataContainer->APIItem(threadID, row);

                        // Sanity check:
                        GT_IF_WITH_ASSERT(pRowItem != nullptr)
                        {
                            for (auto column : m_columnIndicesVec)
                            {
                                QString colString = pRowItem->GetColumnData(column).toString();
                                // remove commas between values in same column, as they define a new column in Ecxel
                                colString.replace(AC_STR_CommaA, AC_STR_SpaceA);
                                strList << AC_STR_QuotSpace + colString + AC_STR_QuotSpace + AC_STR_CommaA;
                            }

                            data << strList.join(AC_STR_Tab) << AC_STR_NewLineA;
                            strList.clear();
                        }
                    }

                    // Close the file:
                    fileHandle.close();

                    retVal = true;
                }
            }
        }
    }
    return retVal;
}

ProfileSessionDataItem::ProfileSessionDataColumnIndex gpTableModelBase::TableIndexToColumnIndex(int tableColumnIndex)
{
    ProfileSessionDataItem::ProfileSessionDataColumnIndex retVal = ProfileSessionDataItem::SESSION_ITEM_UNKNOWN;

    // Sanity check:
    GT_IF_WITH_ASSERT((tableColumnIndex >= 0) && (tableColumnIndex < m_columnIndicesVec.size()))
    {
        retVal = m_columnIndicesVec[tableColumnIndex];
    }
    return retVal;
}


GPUTableModel::GPUTableModel(const QString& queueName, gpTraceDataContainer* pDataContainer) :
    gpTableModelBase(0, pDataContainer), m_queueName(queueName)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        // Get the columns vector
        ProfileSessionDataItem::ProfileItemAPIType gpuItemType = ProfileSessionDataItem::DX12_GPU_PROFILE_ITEM;
        if (m_pSessionDataContainer->SessionAPIType() == ProfileSessionDataItem::VK_API_PROFILE_ITEM)
        {
            gpuItemType = ProfileSessionDataItem::VK_GPU_PROFILE_ITEM;
        }
        else if (m_pSessionDataContainer->SessionAPIType() == ProfileSessionDataItem::CL_API_PROFILE_ITEM)
        {
            gpuItemType = ProfileSessionDataItem::CL_GPU_PROFILE_ITEM;
        }
        ProfileSessionDataItem::GetListOfColumnIndices(gpuItemType, m_columnIndicesVec);
    }
}

GPUTableModel::~GPUTableModel()
{

}

int GPUTableModel::rowCount(const QModelIndex& parent) const
{
    int retVal = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        if (parent.column() <= 0)
        {
            retVal = m_pSessionDataContainer->GPUItemsCount(m_queueName);
        }

    }
    return retVal;
}

QModelIndex GPUTableModel::index(int row, int column, const QModelIndex& parent) const
{
    QModelIndex retVal;

    if (hasIndex(row, column, parent))
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
        {
            ProfileSessionDataItem* pItem = m_pSessionDataContainer->GPUItem(m_queueName, row);

            if (pItem != nullptr)
            {
                retVal = createIndex(row, column, pItem);
            }
        }
    }

    return retVal;
}

ProfileSessionDataItem* GPUTableModel::GetItem(const QModelIndex& item) const
{
    ProfileSessionDataItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        pRetVal = m_pSessionDataContainer->GPUItem(m_queueName, item.row());
    }

    return pRetVal;
}

bool GPUTableModel::ExportToCSV(const QString& outputFilePath)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {

        int queueCount = m_pSessionDataContainer->GPUCallsContainersCount();

        for (int i = 0; i < queueCount; i++)
        {
            QString queueName = m_pSessionDataContainer->GPUObjectName(i);
            QString queueFile = outputFilePath;

            QString queueTypeStr;
            int queueType = m_pSessionDataContainer->GPUObjectType(queueName);
            queueTypeStr = gpTraceDataContainer::CommandListTypeAsString(queueType);

            // Create the queue table name
            QString queueDisplayName = ProfileSessionDataItem::QueueDisplayName(queueName);
            QString tableFileName = QString(GPU_STR_timeline_QueueBranchName).arg(queueTypeStr).arg(queueDisplayName);

            queueFile.append(osFilePath::osPathSeparator + tableFileName + QString(GP_CSV_FileExtension));


            // Define a file handle:
            QFile fileHandle(queueFile);

            // Open the file for write:
            bool rcOpen = fileHandle.open(QFile::WriteOnly | QFile::Truncate);

            if (rcOpen)
            {
                QTextStream data(&fileHandle);
                QStringList strList;


                // Get the row count:
                int rowCount = m_pSessionDataContainer->GPUItemsCount(queueName);
                int colCount = m_columnIndicesVec.count();

                // Write the headers:
                for (int i = 0; i < colCount; ++i)
                {
                    QString headerString = headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
                    strList << AC_STR_QuotSpace + headerString + AC_STR_QuotSpace + AC_STR_CommaA;
                }

                // Write a new line:
                data << strList.join(AC_STR_Tab) << AC_STR_NewLineA;
                strList.clear();

                // Go through the rows and export each of them:
                for (int row = 0; row < rowCount; ++row)
                {
                    QString rowStr;
                    ProfileSessionDataItem* pRowItem = m_pSessionDataContainer->GPUItem(queueName, row);

                    // Sanity check:
                    GT_IF_WITH_ASSERT(pRowItem != nullptr)
                    {
                        for (auto column : m_columnIndicesVec)
                        {
                            QString colString = pRowItem->GetColumnData(column).toString();
                            // remove commas between values in same column, as they define a new column in Ecxel
                            colString.replace(AC_STR_CommaA, AC_STR_SpaceA);
                            strList << AC_STR_QuotSpace + colString + AC_STR_QuotSpace + AC_STR_CommaA;
                        }

                        data << strList.join(AC_STR_Tab) << AC_STR_NewLineA;
                        strList.clear();
                    }
                }

                // Close the file:
                fileHandle.close();

                retVal = true;
            }
        }
    }

    return retVal;


}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DXAPITreeModel::DXAPITreeModel(osThreadId threadID, gpTraceDataContainer* pDataContainer) : gpTableModelBase(threadID, pDataContainer),
    m_pRootItem(nullptr)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        // Get the API type for the requested thread ID
        ProfileSessionDataItem::ProfileItemAPIType apiType = m_pSessionDataContainer->APITypeForThread(threadID);

        ProfileSessionDataItem::GetListOfColumnIndices(apiType, m_columnIndicesVec);

        // Initialize the root item
        m_pRootItem = m_pSessionDataContainer->GetRootItem(m_threadID);
    }
}

DXAPITreeModel::~DXAPITreeModel()
{
}

int DXAPITreeModel::rowCount(const QModelIndex& parent) const
{
    int retVal = 0;

    ProfileSessionDataItem* pParentItem = nullptr;

    if (parent.column() <= 0)
    {
        if (!parent.isValid())
        {
            pParentItem = m_pRootItem;
        }
        else
        {
            pParentItem = static_cast<ProfileSessionDataItem*>(parent.internalPointer());
        }

        // Sanity check:
        GT_IF_WITH_ASSERT(pParentItem != nullptr)
        {
            retVal = pParentItem->GetChildCount();
        }
    }

    return retVal;
}

QModelIndex DXAPITreeModel::index(int row, int column, const QModelIndex& parent) const
{
    QModelIndex retVal;

    if (hasIndex(row, column, parent))
    {
        ProfileSessionDataItem* pParentItem = m_pRootItem;

        if (parent.isValid())
        {
            pParentItem = static_cast<ProfileSessionDataItem*>(parent.internalPointer());
        }

        // Sanity check:
        GT_IF_WITH_ASSERT(pParentItem != nullptr)
        {
            ProfileSessionDataItem* pChildItem = pParentItem->GetChild(row);

            if (pChildItem != nullptr)
            {
                return createIndex(row, column, pChildItem);
            }
        }
    }

    return retVal;
}

QModelIndex DXAPITreeModel::parent(const QModelIndex& index) const
{
    QModelIndex retVal;

    if (index.isValid())
    {

        ProfileSessionDataItem* pChildItem = static_cast<ProfileSessionDataItem*>(index.internalPointer());

        // Sanity check:
        GT_IF_WITH_ASSERT(pChildItem != nullptr)
        {
            ProfileSessionDataItem* pParentItem = pChildItem->GetParent();

            if ((pParentItem != m_pRootItem) && (pParentItem != nullptr))
            {
                retVal = createIndex(pParentItem->GetRow(), 0, pParentItem);
            }
        }
    }

    return retVal;
}

ProfileSessionDataItem* DXAPITreeModel::GetItem(const QModelIndex& item) const
{
    ProfileSessionDataItem* pRetVal = static_cast<ProfileSessionDataItem*>(item.internalPointer());
    return pRetVal;

}


bool DXAPITreeModel::ExportToCSV(const QString& outputFilePath)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSessionDataContainer != nullptr) && (m_pRootItem != nullptr))
    {
        int threadCount = m_pSessionDataContainer->ThreadsCount();

        for (int i = 0; i < threadCount; i++)
        {
            osThreadId threadID = m_pSessionDataContainer->ThreadID(i);

            // Sanity check:
            GT_IF_WITH_ASSERT((m_pSessionDataContainer != nullptr) && (m_pSessionDataContainer->GetRootItem(threadID) != nullptr))
            {
                QString threadFile = outputFilePath;
                //QString tableCaption = QString(GPU_STR_timeline_CPU_ThreadBranchName).arg(threadID);

                threadFile.append(osFilePath::osPathSeparator + QString(AC_STR_ExportThreadToCSVFileName).arg(QString::number(threadID)) + QString(GP_CSV_FileExtension));
                // Define a file handle:
                QFile fileHandle(threadFile);


                // Open the file for write:
                bool rcOpen = fileHandle.open(QFile::WriteOnly | QFile::Truncate);

                if (rcOpen)
                {
                    QTextStream data(&fileHandle);
                    QStringList strList;

                    // Sanity check:
                    GT_IF_WITH_ASSERT(m_pRootItem != nullptr)
                    {
                        int rowCount = m_pRootItem->GetChildCount();

                        // Write the headers:
                        for (int i = 0; i < ProfileSessionDataItem::SESSION_ITEM_COLUMN_COUNT; ++i)
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
                            ProfileSessionDataItem* pRowItem = m_pRootItem->GetChild(row);

                            for (int column = 0; column < ProfileSessionDataItem::SESSION_ITEM_COLUMN_COUNT; ++column)
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
            }
        }
    }
    return retVal;
}
