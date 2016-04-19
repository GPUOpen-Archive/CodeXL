//------------------------------ gpTraceTree.cpp ------------------------------

// Infra:
#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineItem.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SessionTreeNodeData.h>

// Local:
#include <AMDTGpuProfiling/gpTraceTree.h>
#include <AMDTGpuProfiling/gpTraceModels.h>
#include <AMDTGpuProfiling/gpTraceView.h>
#include <AMDTGpuProfiling/ProfileSessionDataItem.h>
#include <AMDTGpuProfiling/gpTraceDataContainer.h>
#include <AMDTGpuProfiling/APIColorMap.h>
#include <AMDTGpuProfiling/gpStringConstants.h>
#include <AMDTGpuProfiling/Util.h>
#include <AMDTGpuProfiling/SymbolInfo.h>

#define GP_DEFAULT_ROW_HEIGHT 20
gpTraceTree::gpTraceTree(osThreadId threadID, gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView) :
    m_pDataModel(nullptr),
    m_pSessionView(pSessionView),
    m_pContextMenu(nullptr),
    m_pGotoSourceAction(nullptr),
    m_pZoomInTimelineAction(nullptr),
    m_pExpandAllAction(nullptr),
    m_pCollapseAllAction(nullptr),
    m_pCopyAction(nullptr),
    m_pSelectAllAction(nullptr),
    m_pExportToCSVAction(nullptr),
    m_pCahcedSymbolInfo(nullptr)
{
    // Create the model for this table
    m_pDataModel = new DXAPITreeModel(threadID, pDataContainer);
    setModel(m_pDataModel);

    // Set the link color
    m_pDataModel->SetVisualProperties(palette().color(QPalette::Text), palette().color(QPalette::Link), font());

    // Set the graphic attributes
    setSortingEnabled(false);
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setMouseTracking(true);

    // Prepare the context menu
    BuildContextMenu();
}


gpTraceTree::~gpTraceTree()
{
}

ProfileSessionDataItem* gpTraceTree::GetItem(const QModelIndex& item)
{
    ProfileSessionDataItem* pRetVal = nullptr;

    DXAPITreeModel* pModel = qobject_cast<DXAPITreeModel*>(model());

    // Sanity check:
    GT_IF_WITH_ASSERT(pModel != nullptr)
    {
        pRetVal = pModel->GetItem(item);
    }

    return pRetVal;
}


void gpTraceTree::resizeEvent(QResizeEvent* event)
{
    QTreeView::resizeEvent(event);
#pragma message ("TODO: FA:")
    /*int newWidth = event->size().width();

    if (event->oldSize().width() != newWidth)
    {
        QHeaderView* horzHeader = horizontalHeader();

        for (int i = 0; i < horzHeader->count(); i++)
        {
            int newSectionWidth = int(newWidth * GetSectionFillWeight(i));
            horzHeader->resizeSection(i, newSectionWidth);
        }
    }*/
}

float gpTraceTree::GetSectionFillWeight(int sectionIndex)
{
    float retVal = .05f;

    // column widths as a percent of the total width (these values look best)
    static float s_columnWeightsPercentage[ProfileSessionDataItem::SESSION_ITEM_COLUMN_COUNT];
    s_columnWeightsPercentage[ProfileSessionDataItem::SESSION_ITEM_INDEX_COLUMN] = 5;
    s_columnWeightsPercentage[ProfileSessionDataItem::SESSION_ITEM_INTERFACE_COLUMN] = 14;
    s_columnWeightsPercentage[ProfileSessionDataItem::SESSION_ITEM_CALL_COLUMN] = 14;
    s_columnWeightsPercentage[ProfileSessionDataItem::SESSION_ITEM_PARAMETERS_COLUMN] = 40;
    s_columnWeightsPercentage[ProfileSessionDataItem::SESSION_ITEM_RESULT_COLUMN] = 9;
    s_columnWeightsPercentage[ProfileSessionDataItem::SESSION_ITEM_DEVICE_BLOCK_COLUMN] = 9;
    s_columnWeightsPercentage[ProfileSessionDataItem::SESSION_ITEM_OCCUPANCY_COLUMN] = 9;
    s_columnWeightsPercentage[ProfileSessionDataItem::SESSION_ITEM_CPU_TIME_COLUMN] = 5;

    GT_IF_WITH_ASSERT((sectionIndex < ProfileSessionDataItem::SESSION_ITEM_COLUMN_COUNT) && (sectionIndex >= 0))
    {
        retVal = s_columnWeightsPercentage[sectionIndex] / 100.0;
    }
    return retVal;
}

void gpTraceTree::BuildContextMenu()
{
    // Add the actions to the table:
    m_pContextMenu = new QMenu(this);
    m_pCopyAction = m_pContextMenu->addAction(AF_STR_CopyA, this, SLOT(OnEditCopy()));
    m_pSelectAllAction = m_pContextMenu->addAction(AF_STR_SelectAllA, this, SLOT(OnEditSelectAll()));
    m_pContextMenu->addSeparator();

    m_pExpandAllAction = m_pContextMenu->addAction(GPU_STR_TraceViewExpandAll, this, SLOT(OnExpandAll()));
    m_pCollapseAllAction = m_pContextMenu->addAction(GPU_STR_TraceViewCollapseAll, this, SLOT(OnCollapseAll()));
    m_pContextMenu->addSeparator();

    m_pGotoSourceAction = m_pContextMenu->addAction(GPU_STR_TraceViewGoToSource, this, SLOT(OnGotoSource()));
    m_pZoomInTimelineAction = m_pContextMenu->addAction(GPU_STR_TraceViewZoomTimeline, this, SLOT(OnZoomItemInTimeline()));
    m_pContextMenu->addSeparator();

    m_pExportToCSVAction = m_pContextMenu->addAction(AF_STR_ExportToCSV, this, SLOT(OnExportToCSV()));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnContextMenu(const QPoint&)));
}


void gpTraceTree::OnContextMenu(const QPoint& point)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pDataModel != nullptr)
    {
        QString strMenuText(GPU_STR_TraceViewGoToSource);
        bool isSourceCodeEnabled = false;
        bool isCopyEnabled = false;
        bool isSelectAllEnabled = false;

        QModelIndex index = indexAt(point);
        ProfileSessionDataItem* pSessionDataItem = m_pDataModel->GetItem(index);

        // Sanity check:
        if (pSessionDataItem != nullptr)
        {
            // Check if copy and select all actions are enabled:
            isCopyEnabled = !selectionModel()->selectedIndexes().isEmpty();
            isSelectAllEnabled = (m_pDataModel->rowCount(QModelIndex()) > 0);

#pragma message ("TODO: FA: this is not correct. In order to get the real row, we should implement the unique id implemented in the trace view")
            int row = index.row();
            m_pCahcedSymbolInfo = m_pDataModel->GetSymbolInfo(row);

            if ((row >= 0) && (m_pCahcedSymbolInfo != nullptr))
            {
                isSourceCodeEnabled = true;
            }

            // change the caption if the option to allow source code navigation is not enabled (gives a hint to the user)
            if (!m_pDataModel->SessionHasSymbolInformation())
            {
                strMenuText.append(QString(" (Enable navigation to source code on the \"%1\" project setting page.)").arg(Util::ms_APP_TRACE_OPTIONS_PAGE));
            }

            // disable context menu actions on selection of more then 1 row (not copy and select all)
            bool isMultiRowSelection = (selectionModel()->selectedRows().count() > 1);

            m_pGotoSourceAction->setText(strMenuText);
            m_pGotoSourceAction->setEnabled(isSourceCodeEnabled && !isMultiRowSelection);
            m_pExpandAllAction->setEnabled(true);
            m_pCollapseAllAction->setEnabled(true);
            m_pZoomInTimelineAction->setEnabled(isCopyEnabled && !isMultiRowSelection);
            m_pCopyAction->setEnabled(isCopyEnabled);
            m_pSelectAllAction->setEnabled(isSelectAllEnabled);
            m_pContextMenu->exec(acMapToGlobal(this, point));
        }
    }
}

void gpTraceTree::OnGotoSource()
{
    if (m_pCahcedSymbolInfo != nullptr)
    {
        osFilePath filePath(acQStringToGTString(m_pCahcedSymbolInfo->FileName()));

        if (filePath.isEmpty())
        {
            QString strExtraInfo;

            strExtraInfo = GP_Str_InvalidDebugInfo;

            // if file name, api name, symbol name, and line number are all empty, then that is an indication that there is no entry in the .st file for the selected API
            if (m_pCahcedSymbolInfo->ApiName().isEmpty() && m_pCahcedSymbolInfo->SymbolName().isEmpty() && m_pCahcedSymbolInfo->LineNumber() == 0)
            {
                strExtraInfo += QString(GP_Str_AppTerminatedUnexpectedly).arg(Util::ms_ENABLE_TIMEOUT_OPTION);
            }

            Util::ShowWarningBox(QString(GP_Str_NoSourceInfoForSelectedAPI).arg(strExtraInfo));
        }
        else if (filePath.exists() && filePath.isRegularFile())
        {
            afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
            {
                pApplicationCommands->OpenFileAtLine(filePath, m_pCahcedSymbolInfo->LineNumber(), -1);
            }
        }
        else
        {
            // check to see if this file is a file from an installed sample file (in this case, the path in the debug info might not match the path to the file on disk)
            filePath = Util::GetInstalledPathForSampleFile(filePath);

            if (filePath.exists() && filePath.isRegularFile())
            {
                afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
                GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
                {
                    pApplicationCommands->OpenFileAtLine(filePath, m_pCahcedSymbolInfo->LineNumber(), -1);
                }
            }
            else
            {
                Util::ShowWarningBox(QString(GP_Str_CantAccessSourceForSelectedAPI).arg(acGTStringToQString(filePath.asString())));
            }
        }

        m_pCahcedSymbolInfo = nullptr;
    }
}

void gpTraceTree::OnCollapseAll()
{
    collapseAll();
}

void gpTraceTree::OnExpandAll()
{
    expandAll();
}

void gpTraceTree::OnZoomItemInTimeline()
{
    GT_IF_WITH_ASSERT((!selectionModel()->selectedIndexes().isEmpty()) && (m_pDataModel != nullptr))
    {
        QModelIndex firstSelected = selectionModel()->selectedIndexes().first();
        ProfileSessionDataItem* pItem = m_pDataModel->GetItem(firstSelected);

        // Sanity check:
        GT_IF_WITH_ASSERT((pItem != nullptr) && (m_pSessionView != nullptr))
        {
            // Get the timeline item related to this profile session item
            m_pSessionView->ZoomToItem(pItem);
        }
    }
}

void gpTraceTree::OnExportToCSV()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pDataModel != nullptr) && (m_pSessionView != nullptr))
    {
        // The file path for the saved CSV file:
        QString csvFilePathStr;

        // Get the tree data from the session view
        SessionTreeNodeData* pSessionData = m_pSessionView->SessionTreeData();

        // Sanity check:
        GT_IF_WITH_ASSERT(pSessionData != nullptr)
        {
            // Build the CSV default file name:
            QString fileName = QString(GPU_CSV_FileNameFormat).arg(pSessionData->m_displayName).arg(GPU_CSV_FileNameTraceView);
            bool rc = afApplicationCommands::instance()->ShowQTSaveCSVFileDialog(csvFilePathStr, fileName, this);
            GT_IF_WITH_ASSERT(rc)
            {
                // Export the web view table to a CSV file:
                rc = m_pDataModel->ExportToCSV(csvFilePathStr);
                GT_ASSERT(rc);
            }
        }
    }
}

void gpTraceTree::OnEditCopy()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(qApp != NULL)
    {
        // Get the clipboard from the application:
        QClipboard* pClipboard = qApp->clipboard();
        GT_IF_WITH_ASSERT(pClipboard != NULL)
        {
            // Get the selected items list:
            QModelIndexList selectedRowsList = selectionModel()->selectedRows();
            QModelIndexList selectedItemsList = selectionModel()->selectedIndexes();

            QString selectedText;

            if (!selectedRowsList.isEmpty())
            {
                for (int i = 0; i < model()->columnCount(); i++)
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

                    for (QModelIndex tempIndex : selectedItemsList)
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

void gpTraceTree::OnEditSelectAll()
{
    selectAll();
}

ProfileSessionDataItem::ProfileSessionDataColumnIndex gpTraceTree::TableIndexToColumnIndex(int tableColumnIndex)
{
    ProfileSessionDataItem::ProfileSessionDataColumnIndex retVal = ProfileSessionDataItem::SESSION_ITEM_UNKNOWN;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pDataModel != nullptr)
    {
        retVal = m_pDataModel->TableIndexToColumnIndex(tableColumnIndex);
    }
    return retVal;
}


void gpTraceTree::SelectRowBySampleId(ProfileSessionDataItem* pItem)
{
    QModelIndex firstSelected = selectionModel()->selectedIndexes().first();
    ProfileSessionDataItem* pSelectedItem = m_pDataModel->GetItem(firstSelected);
    GT_IF_WITH_ASSERT((pItem != nullptr) && (pSelectedItem != nullptr))
    {
        QModelIndex next_index = currentIndex().child(1, 0);

        //QModelIndex next_index = myTreeView->model()->index(1, 0);
        //QModelIndex next_index =idx.child(0,0);
        //qDebug() << "next_index" << next_index.row() << next_index.column();
        //myTreeView->setCurrentIndex(next_index);

        selectionModel()->select(next_index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
}
