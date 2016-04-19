//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SessionWindow.cpp $
/// \version $Revision: #46 $
/// \brief  This file contains GPUSessionWindow class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SessionWindow.cpp#46 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================
#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>

// QScintilla
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexercpp.h>

#include <AMDTBaseTools/Include/gtAssert.h>

// AMDTApplicationFramework:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afMessageBox.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>

// Local:
#include <AMDTGpuProfiling/SessionWindow.h>
#include <AMDTGpuProfiling/CodeViewerWindow.h>
#include <AMDTGpuProfiling/KernelOccupancyWindow.h>
#include <AMDTGpuProfiling/gpViewsCreator.h>
#include <AMDTGpuProfiling/ProfileManager.h>

const QStringList IGNORE_COLUMNS_EMPTY_CELLS { "Method", "ExecutionOrder", "ThreadID", "CallIndex", "GlobalWorkSize", "WorkGroupSize", "Time", "LocalMemSize", "VGPRs", "SGPRs", "ScratchRegs", "FCStacks", "KernelOccupancy", "ThreadGroup" };


float GPUSessionWindow::ms_widthFactor = 1.07f;
QString GPUSessionWindow::ms_strShowRowMenuText = "Show Row ";
QString GPUSessionWindow::ms_strShowAllRowsMenuText = "Show All Hidden Rows";
QString GPUSessionWindow::ms_strNoHiddenRowsMenuText = "There are no hidden rows (double-click the first column to hide a row)";
QString GPUSessionWindow::ms_strNoPreformanceDataHeading = "No data was collected for some kernels";
QString GPUSessionWindow::ms_strNoPreformanceDataText = "Some kernels do not have any Performance Counters data.\nKernels that use Shared Virtual Memory or pipe arguments are ignored if multiple passes are configured for data collection.\nUse the CodeXL Project Settings to configure the number of collection passes the profiler will perform.";



GPUSessionWindow::GPUSessionWindow(QWidget* parent) : gpBaseSessionView(parent),
    m_pCurrentSession(NULL),
    m_pControl(NULL),
    m_pGroupActions(NULL)
{
    m_visibleRows.clear();
    m_pControl = new SessionControl(this);

    m_pGroupActions = new QActionGroup(this);


    bool status = false;
    status =  connect(m_pControl->GetShowZeroColumnCB(), SIGNAL(stateChanged(int)), this, SLOT(ChangeStateInZeroColumnCB(int))) && status;
    status = connect(m_pControl->GetTableView(), SIGNAL(mouseLeftClicked(QModelIndex)), this, SLOT(CellContentClicked(const QModelIndex&))) && status;
    status = connect(m_pControl->GetTableView(), SIGNAL(entered(const QModelIndex&)), this, SLOT(MouseEnterInACell(const QModelIndex&))) && status;

    // To hide rows
    m_pControl->GetTableView()->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    status = connect(m_pControl->GetTableView()->verticalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(HideSelectedRow(int))) && status;
    status = connect(m_pControl->GetTableView()->verticalHeader(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ContextMenuForHiddenRows(const QPoint&))) && status;

    if (status == false)
    {
        // TODO : Need to add error handling
    }

    m_pSessionTabWidget->setTabsClosable(true);

    QHBoxLayout* layout = new QHBoxLayout(this);
    m_pSessionTabWidget->addTab(m_pControl, "Performance Counters");
    layout->addWidget(m_pSessionTabWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
}

GPUSessionWindow::~GPUSessionWindow()
{
    // Remove me from the list of session windows in the session view creator:
    gpViewsCreator::Instance()->OnWindowClose(this);
}


bool GPUSessionWindow::DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage, QString& errorMessage)
{
    // Call the base class implementation
    bool retVal = SharedSessionWindow::DisplaySession(sessionFilePath, sessionInnerPage, errorMessage);

    QString sessionFile;
    bool rc = gpViewsCreator::GetSessionFileFromTempPCFile(sessionFilePath, sessionFile);
    GT_IF_WITH_ASSERT(rc)
    {
        // Initialize the session file path:
        m_sessionFilePath = acQStringToGTString(sessionFile);

        GPUSessionTreeItemData* pSession = qobject_cast<GPUSessionTreeItemData*>(m_pSessionData);

        // Sanity check:
        GT_IF_WITH_ASSERT(pSession != nullptr)
        {
            bool shouldReportEmptyCells = false;

            if (m_pCurrentSession != pSession)
            {
                m_pCurrentSession = pSession;
                Clear();
                m_visibleRows.clear();

                if (m_pCurrentSession != NULL)
                {
                    m_pCurrentSession->FlushData();
                }

                retVal = m_pControl->LoadSession(pSession);

                if (retVal)
                {
                    QStandardItemModel* pItemModel = m_pControl->ItemModel();
                    bool isEmptyCellFound = false;
                    bool isEmptyCellRelevant = false;

                    for (int row = 0; row < pItemModel->rowCount(); row++)
                    {
                        // Initializing list
                        m_visibleRows.push_back(true);

                        DetectEmptyCells(pItemModel, row, isEmptyCellFound, isEmptyCellRelevant);
                        shouldReportEmptyCells |= isEmptyCellRelevant;
                    }

                    if (m_pControl->GetShowZeroColumnCB())
                    {
                        ChangeStateInZeroColumnCB(m_pControl->GetShowZeroColumnCB()->checkState());
                    }

                    AdjustColumnWidth();
                }
            }

            if (shouldReportEmptyCells && (pSession->GetAPIToTrace() != APIToTrace_HSA))
            {
                afMessageBox::instance().warning(ms_strNoPreformanceDataHeading, ms_strNoPreformanceDataText);
            }
        }
    }

    return retVal;
}

void GPUSessionWindow::AdjustColumnWidth()
{
    QStandardItemModel* itemModel = m_pControl->ItemModel();
    QFontMetrics fontMetrics(m_pControl->GetTableView()->fontMetrics());

    int paddingSize = fontMetrics.width("    "); // pad with room for four spaces
    int maxWidth;
    int currentSize;
    QString cellValue;

    // Only resize the columns to fit the first 200 (or 220 if there are less than 220) rows.
    // This method is too slow if we try to resize the columns for all rows (if there are a lot of rows)
    // So the user will have to adjust column widths manually if they have a lot of rows and there is wider data in rows above 200
    int rowsToCheck = itemModel->rowCount();

    if (rowsToCheck > 220)
    {
        rowsToCheck = 200;
    }

    for (int col = 0; col < itemModel->columnCount(); col++)
    {
        cellValue = itemModel->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString();
        maxWidth = ComputeStringWidth(cellValue, fontMetrics);

        for (int row = 0; row < rowsToCheck; row++)
        {
            QModelIndex modelIndex = itemModel->index(row, col);
            QStandardItem* item = itemModel->itemFromIndex(modelIndex);

            if (item != NULL)
            {
                cellValue = modelIndex.data().toString();
                currentSize = ComputeStringWidth(cellValue, QFontMetrics(item->font()));

                if (maxWidth < currentSize)
                {
                    maxWidth = currentSize;
                }
            }
        }

        m_pControl->GetTableView()->setColumnWidth(col, maxWidth + paddingSize);
    }
}

int GPUSessionWindow::ComputeStringWidth(const QString& str, const QFontMetrics& fm)
{
    AGP_TODO("check and remove the variable ""ms_widthFactor""");
    return (int)(ceil(fm.boundingRect(str).width() * ms_widthFactor));
}

void GPUSessionWindow::Clear()
{
    //TODO: Need to clear data in member variables.
    //Control->Clear();
    //CurrentSession = NULL;
    //this.Caption = Resources.SessionWindowTitle;
}

bool GPUSessionWindow::GetKernelNameFromTV(QString& strKernelName, const QModelIndex index)
{
    // kernel name resides on column 0
    if ((index.column() != 0) || (index.row() < 0))
    {
        strKernelName = QString();
        return false;
    }

    QString strCell = Util::ToString(m_pControl->ItemModel(), index);

    // check the cell content whether it is a kernel name
    if (!Util::IsKernelName(strCell))
    {
        strKernelName = QString();
        return false;
    }

    strKernelName = strCell.trimmed();
    return true;
}

bool GPUSessionWindow::CreateGroupActions(int rowIndex)
{
    QList<QString> actionText;
    QString actionStr = ms_strShowRowMenuText;
    int rowCount = m_pControl->GetTableView()->model()->rowCount();

    delete m_pGroupActions;

    if (rowIndex >= rowCount)
    {
        return false;
    }

    for (int i = rowIndex - 1; i >= 0 ; i--)
    {
        if (m_visibleRows[i] == false)
        {
            actionText.push_front(actionStr + QString::number(i + 1));
        }
        else
        {
            break;
        }
    }

    for (int i = rowIndex + 1; i < rowCount; i++)
    {
        if (m_visibleRows[i] == false)
        {
            actionText.push_back(actionStr + QString::number(i + 1));
        }
        else
        {
            break;
        }
    }

    m_pGroupActions = new QActionGroup(this);

    for (int i = 0; i < actionText.count(); i++)
    {
        m_pGroupActions->addAction(actionText[i]);
    }

    if (actionText.isEmpty())
    {
        int index;

        for (index = 0; index < rowCount; index++)
        {
            if (m_visibleRows[index] == false)
            {
                m_pGroupActions->addAction(ms_strShowAllRowsMenuText);
                break;
            }
        }

        if (index == rowCount)
        {
            QAction* noHiddenRowsAction = m_pGroupActions->addAction(ms_strNoHiddenRowsMenuText);
            noHiddenRowsAction->setEnabled(false);
        }
    }
    else
    {
        m_pGroupActions->addAction(ms_strShowAllRowsMenuText);
    }

    connect(m_pGroupActions, SIGNAL(triggered(QAction*)), this, SLOT(ShowHiddenRows(QAction*)));

    return true;
}

// Private Slots

void GPUSessionWindow::CellContentClicked(const QModelIndex& index)
{
    static QString strCodeViewerTabText = "Code Viewer (%1)";

    // show the kernel code if we click on the content
    QString strKernelName;
    QString errMsg;

    if (GetKernelNameFromTV(strKernelName, index))
    {
        errMsg = QString(GP_Str_ErrorUnableToLoad).arg(GP_Str_OccupancyCodeViewerName);

        // load the various kernel files
        if (Util::IsCodeAvailable(m_pCurrentSession, strKernelName))
        {
            if (m_pCodeViewerWindow == NULL)
            {
                m_pCodeViewerWindow = new(nothrow) CodeViewerWindow(this);

                if (!m_pCodeViewerWindow)
                {
                    errMsg += "\n";
                    errMsg += GP_Str_ErrorInsufficientMemory;
                    Util::ShowErrorBox(errMsg);
                    return;
                }

                //
                m_codeViewerTabIndex = m_pSessionTabWidget->addTab(m_pCodeViewerWindow, "");

            }

            m_pCodeViewerWindow->LoadKernelCodeFiles(m_pCurrentSession, strKernelName);
            GT_ASSERT(m_codeViewerTabIndex != -1)
            m_pSessionTabWidget->setTabText(m_codeViewerTabIndex, strCodeViewerTabText.arg(strKernelName));
            m_pSessionTabWidget->setCurrentIndex(m_codeViewerTabIndex);
        }
    }
    else
    {
        int kernelOccColIndex = m_pControl->KernelOccupancyColumnIndex();

        if (index.row() >= 0 && kernelOccColIndex != -1 && index.column() == kernelOccColIndex)
        {
            // Get the kernel name for this row:
            QString kernelName;
            QModelIndex kernelColIndex = m_pControl->ItemModel()->index(index.row(), 0);
            bool rc = GetKernelNameFromTV(kernelName, kernelColIndex);
            GT_ASSERT(rc);
            OccupancyInfo* occupancyInfo = m_pControl->GetOccupancyForRow(index.row(), kernelName);

            if (!occupancyInfo)
            {
                return;
            }

            m_currentDisplayedOccupancyKernel = occupancyInfo->GetKernelName();
            QTableView* pPcTable = m_pControl->GetTableView();
            int columnCount = pPcTable->model()->columnCount();
            int callIndexColIndex = -1;
            int executionOrderColIndex = -1;

            for (int col = 0; col < columnCount; col++)
            {
                if (pPcTable->model()->headerData(col, Qt::Horizontal).toString().trimmed() == "CallIndex")
                {
                    callIndexColIndex = col;
                    break;
                }

                // for HSA sessions, there is no Callindex column (no dispatch API), so we have to use ExecutionOrder instead
                if (pPcTable->model()->headerData(col, Qt::Horizontal).toString().trimmed() == "ExecutionOrder")
                {
                    executionOrderColIndex = col;
                }
            }

            if (-1 == callIndexColIndex && -1 != executionOrderColIndex)
            {
                callIndexColIndex = executionOrderColIndex;
            }

            if (-1 == callIndexColIndex)
            {
                return;
            }

            QModelIndex callIndexModelIndex = pPcTable->model()->index(index.row(), callIndexColIndex);
            bool ok = false;
            int callIndex = pPcTable->model()->data(callIndexModelIndex).toInt(&ok);

            if (!ok)
            {
                return;
            }

            QString strErrorMessageOut;
            connect(ProfileManager::Instance(), SIGNAL(OccupancyFileGenerationFinished(bool, const QString&, const QString&)), this, SLOT(OnOccupancyFileGenerationFinish(bool, const QString&, const QString&)));
            // Generate occupancy page
            bool retVal = ProfileManager::Instance()->GenerateOccupancyPage(m_pCurrentSession, occupancyInfo, callIndex, strErrorMessageOut);

            if (!retVal)
            {
                Util::ShowErrorBox(strErrorMessageOut);
            }
        }
    }
}

void GPUSessionWindow::MouseEnterInACell(const QModelIndex& index)
{
    QCursor cursor = Qt::ArrowCursor;

    if (index.column() >= 0 && index.row() >= 0)
    {
        QStandardItemModel* itemModel = m_pControl->ItemModel();

        if (index.column() == 0)
        {
            QString strCell = Util::ToString(itemModel, index).trimmed();

            if (Util::IsKernelName(strCell) && Util::IsCodeAvailable(m_pCurrentSession, strCell))
            {
                cursor = Qt::PointingHandCursor;
            }
        }
        else
        {
            int kernelOccColIndex = m_pControl->KernelOccupancyColumnIndex();

            if (kernelOccColIndex != -1 && index.column() == kernelOccColIndex)
            {
                QString strCell = itemModel->data(index).toString();

                if (strCell != QString())
                {
                    cursor = Qt::PointingHandCursor;
                }
            }
        }
    }

    m_pControl->GetTableView()->setCursor(cursor);
}

void GPUSessionWindow::ChangeStateInZeroColumnCB(int state)
{
    if (state == Qt::Unchecked)
    {
        QTableView* tempTV = m_pControl->GetTableView();
        m_pControl->RemoveEmptyColumns(tempTV);
        m_pControl->show();
    }
    else if (state == Qt::Checked)
    {
        QTableView* tempTV = m_pControl->GetTableView();
        m_pControl->ShowAllColumns(tempTV);
        m_pControl->show();
    }
}

void GPUSessionWindow::HideSelectedRow(int rowIndex)
{
    if ((VisibleRows() > 1) && (0 <= rowIndex))
    {
        m_visibleRows[rowIndex] = false;
        m_pControl->GetTableView()->hideRow(rowIndex);
    }
}

void GPUSessionWindow::ContextMenuForHiddenRows(const QPoint& point)
{
    QMenu* menu = new QMenu();

    bool status = CreateGroupActions(m_pControl->GetTableView()->rowAt(point.y()));

    if (status == true)
    {
        menu->addActions(m_pGroupActions->actions());
        menu->exec(acMapToGlobal(m_pControl->GetTableView(), point));
    }
}

void GPUSessionWindow::ShowHiddenRow(int rowIndex)
{
    if (rowIndex > 0 && rowIndex < m_visibleRows.count())
    {
        m_visibleRows[rowIndex] = true;
        m_pControl->GetTableView()->showRow(rowIndex);
    }
}

void GPUSessionWindow::ShowHiddenRows(QAction* act)
{
    QString actionStr = act->text();

    if (actionStr.startsWith(ms_strShowRowMenuText))
    {
        actionStr = actionStr.right(actionStr.count() - ms_strShowRowMenuText.length());
        bool status = false;
        int rowIndex = actionStr.toInt(&status);
        GT_ASSERT(status);
        ShowHiddenRow(rowIndex - 1);
    }
    else if (actionStr.startsWith(ms_strShowAllRowsMenuText))
    {
        for (int i = 0; i < m_visibleRows.count(); i++)
        {
            ShowHiddenRow(i);
        }
    }
}

int GPUSessionWindow::VisibleRows()
{
    int visibleRowCount = 0;

    for (int index = 0; index < m_visibleRows.count(); index++)
    {
        if (m_visibleRows[index] == true)
        {
            visibleRowCount++;
        }
    }

    return visibleRowCount;
}

void GPUSessionWindow::onUpdateEdit_Copy(bool& isEnabled)
{
    bool bRes = false;
    QWidget* pCurrentWidget = m_pSessionTabWidget->currentWidget();

    if (NULL != qobject_cast<SessionControl*>(pCurrentWidget))
    {
        (qobject_cast<SessionControl*>(pCurrentWidget))->onUpdateEdit_Copy(isEnabled);
        bRes = true;
    }
    else if (NULL != qobject_cast<CodeViewerWindow*>(pCurrentWidget))
    {
        (qobject_cast<CodeViewerWindow*>(pCurrentWidget))->onUpdateEdit_Copy(isEnabled);
        bRes = true;
    }
    else if (NULL != dynamic_cast<KernelOccupancyWindow*>(pCurrentWidget)) // YRshtunique: dynamic_cast used as qobject_cast requires Q_OBJECT macro in KernelOccupancyWindow
    {
        bRes = true;
    }

    GT_ASSERT(bRes);
}

void GPUSessionWindow::onUpdateEdit_SelectAll(bool& isEnabled)
{
    bool bRes = false;
    QWidget* pCurrentWidget = m_pSessionTabWidget->currentWidget();

    if (NULL != qobject_cast<SessionControl*>(pCurrentWidget))
    {
        (qobject_cast<SessionControl*>(pCurrentWidget))->onUpdateEdit_SelectAll(isEnabled);
        bRes = true;
    }
    else if (NULL != qobject_cast<CodeViewerWindow*>(pCurrentWidget))
    {
        (qobject_cast<CodeViewerWindow*>(pCurrentWidget))->onUpdateEdit_SelectAll(isEnabled);
        bRes = true;
    }
    else if (NULL != dynamic_cast<KernelOccupancyWindow*>(pCurrentWidget))
    {
        bRes = true;
    }

    GT_ASSERT(bRes);
}

void GPUSessionWindow::OnEditCopy()
{
    bool bRes = false;
    QWidget* pCurrentWidget = m_pSessionTabWidget->currentWidget();

    if (NULL != qobject_cast<SessionControl*>(pCurrentWidget))
    {
        (qobject_cast<SessionControl*>(pCurrentWidget))->OnEditCopy();
        bRes = true;
    }
    else if (NULL != qobject_cast<CodeViewerWindow*>(pCurrentWidget))
    {
        (qobject_cast<CodeViewerWindow*>(pCurrentWidget))->OnEditCopy();
        bRes = true;
    }
    else if (NULL != dynamic_cast<KernelOccupancyWindow*>(pCurrentWidget))
    {
        bRes = true;
    }

    GT_ASSERT(bRes);
}

void GPUSessionWindow::OnEditSelectAll()
{
    bool bRes = false;
    QWidget* pCurrentWidget = m_pSessionTabWidget->currentWidget();

    if (NULL != qobject_cast<SessionControl*>(pCurrentWidget))
    {
        (qobject_cast<SessionControl*>(pCurrentWidget))->OnEditSelectAll();
        bRes = true;
    }
    else if (NULL != qobject_cast<CodeViewerWindow*>(pCurrentWidget))
    {
        (qobject_cast<CodeViewerWindow*>(pCurrentWidget))->OnEditSelectAll();
        bRes = true;
    }
    else if (NULL != dynamic_cast<KernelOccupancyWindow*>(pCurrentWidget))
    {
        bRes = true;
    }

    GT_ASSERT(bRes);
}

//-----------------------------------------------------------------
void GPUSessionWindow::onFindClick()
{
    if (m_pSessionTabWidget->currentIndex() == m_codeViewerTabIndex)
    {
        m_pCodeViewerWindow->onFindClick();
    }
}

//-----------------------------------------------------------------
void GPUSessionWindow::onEditFindNext()
{
    if (m_pSessionTabWidget->currentIndex() == m_codeViewerTabIndex)
    {
        m_pCodeViewerWindow->onEditFindNext();
    }
}

//-----------------------------------------------------------------
void GPUSessionWindow::onUpdateEdit_Find(bool& isEnabled)
{
    isEnabled = false;

    if (m_pSessionTabWidget->currentIndex() == m_codeViewerTabIndex)
    {
        isEnabled = true;
    }
}

//-----------------------------------------------------------------
void GPUSessionWindow::onUpdateEdit_FindNext(bool& isEnabled)
{
    isEnabled = false;

    if (m_pSessionTabWidget->currentIndex() == m_codeViewerTabIndex)
    {
        isEnabled = true;
    }
}


bool GPUSessionWindow::IsEmptyCellRelevant(const QString& currentColumnHeaderText)const
{
    bool res = true;

    if (IGNORE_COLUMNS_EMPTY_CELLS.contains(currentColumnHeaderText))
    {
        res = false;
    }

    return res;
}

void GPUSessionWindow::DetectEmptyCells(QStandardItemModel* pItemModel, const int& row, bool& isEmptyCellFound, bool& isEmptyCellRelevant, bool shouldIgnoreCPURows)const
{
    bool shouldIgnoreRow = pItemModel->index(row, 0).data().toString().contains("CPU") && shouldIgnoreCPURows;

    for (int col = 0; col < pItemModel->columnCount() && !shouldIgnoreRow; col++)
    {
        // Going over first row to check if we have empty cells, other rows expected to have similar behavior
        QModelIndex modelIndex = pItemModel->index(row, col);
        QString cellValue = modelIndex.data().toString();

        if (cellValue.isEmpty())
        {
            isEmptyCellFound |= true;
            QStandardItem* pHorizontalHeaderItem = pItemModel->horizontalHeaderItem(col);

            if (NULL != pHorizontalHeaderItem)
            {
                QString headerText = pHorizontalHeaderItem->text();
                isEmptyCellRelevant |= IsEmptyCellRelevant(headerText);
            }
        }
    }
}

