//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SessionControl.cpp $
/// \version $Revision: #42 $
/// \brief  This file contains SessionControl class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SessionControl.cpp#42 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#include <qtIgnoreCompilerWarnings.h>

#include <cmath>
#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include <QtCore>
#include <QtWidgets>

#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

#include "SessionControl.h"

#include <AMDTGpuProfiling/Util.h>
#include "OccupancyInfo.h"
#include "CounterManager.h"

static const gtString s_LOADING_PERFCOUNTER_DATA_PROGRESS = L"Loading Performance Counter Data..."; ///< string shown when loading session data

QMap<QWidget*, bool> SessionControl::m_sessionControlsMap;

SessionControl::SessionControl(QWidget* parent) :
    QWidget(parent),
    m_savedShowKernelDispatchCBState(true),
    m_iThreadColumnIndex(-1),
    m_iMethodColumnIndex(-1),
    m_iKernelOccupancyColumnIndex(-1),
    m_standardModel(nullptr)
{
    InitializeComponent();

    m_sessionControlsMap[this] = true;
}

SessionControl::~SessionControl()
{
    m_sessionControlsMap.remove(this);
}

void SessionControl::InitializeComponent()
{
    m_perfCounterToolBar = new QToolBar(this);

    m_showZeroColumnCB = new QCheckBox(tr("Show Zero Columns"), this);
    m_showZeroColumnCB->setChecked(true);
    m_showZeroColumnCB->setToolTip(tr("Show or hide columns with all zero or empty values"));

    m_perfCounterToolBar->addWidget(m_showZeroColumnCB);

    m_sessionGridView = new TableView(this);
    // To enable mouse tracking.
    m_sessionGridView->setMouseTracking(true);
    // To color alternate rows
    m_sessionGridView->setAlternatingRowColors(true);

    // Enable multi-line selection:
    m_sessionGridView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_sessionGridView->setSelectionBehavior(QAbstractItemView::SelectRows);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_perfCounterToolBar);
    mainLayout->addWidget(m_sessionGridView);

    setLayout(mainLayout);
}

bool SessionControl::LoadSession(GPUSessionTreeItemData* session)
{
    bool retVal = false;

    // Reset the occupancy flag load:
    session->ResetOccupancyFileLoad();

    if (session != nullptr && FillDataTableWithProfileData(session) && (m_standardModel != nullptr))
    {
        SetSessionDataGridVisibility();
        QString colHeader = m_standardModel->headerData(0, Qt::Horizontal).toString();
        AssignTooltips(m_sessionGridView);
        retVal = true;
    }

    return retVal;
}

int SessionControl::PopulateColumnHeaders(const QStringList& headerItems, bool includeOccupancyCol)
{
    int retVal = headerItems.size();
    int stacksColIndex = -1;
    int sgprsColIndex = -1;

    for (int i = 0; i < retVal; i++)
    {
        QString headerName = headerItems.value(i).trimmed();

        if (m_iThreadColumnIndex == -1 && headerName == "ThreadID")
        {
            m_iThreadColumnIndex = i;
        }
        else if (m_iMethodColumnIndex == -1 && headerName == "Method")
        {
            m_iMethodColumnIndex = i;
        }
        else if (stacksColIndex == -1 && headerName == "FCStacks")
        {
            stacksColIndex = i;
        }
        else if (sgprsColIndex == -1 && headerName == "SGPRs")
        {
            sgprsColIndex = i;
        }

        bool counterFound = false;
        bool isPercentage = false;
        QString strCounterDesc;

        if (!counterFound && CounterManager::Instance()->IsHardwareFamilySupported(VOLCANIC_ISLANDS_FAMILY))
        {
            counterFound = CounterManager::Instance()->IsCounterTypePercentage(VOLCANIC_ISLANDS_FAMILY, headerName, isPercentage);
        }

        if (!counterFound && CounterManager::Instance()->IsHardwareFamilySupported(SEA_ISLANDS_FAMILY))
        {
            counterFound = CounterManager::Instance()->IsCounterTypePercentage(SEA_ISLANDS_FAMILY, headerName, isPercentage);
        }

        if (!counterFound && CounterManager::Instance()->IsHardwareFamilySupported(SOUTHERN_ISLANDS_FAMILY))
        {
            counterFound = CounterManager::Instance()->IsCounterTypePercentage(SOUTHERN_ISLANDS_FAMILY, headerName, isPercentage);
        }

        // check non-hardware counters (SC stats, etc.)
        if (!counterFound)
        {
            counterFound = CounterManager::Instance()->IsCounterTypePercentage(NA_HARDWARE_FAMILY, headerName, isPercentage);
        }

        if (counterFound && isPercentage)
        {
            headerName.append(" (%)");
        }

        m_standardModel->setHorizontalHeaderItem(i, new QStandardItem(headerName));
    }

    if (((m_iThreadColumnIndex != -1) || (m_iMethodColumnIndex != -1)) && includeOccupancyCol)
    {
        retVal++;

        if (stacksColIndex != -1)
        {
            m_iKernelOccupancyColumnIndex = stacksColIndex + 1;
        }
        else if (sgprsColIndex != -1)
        {
            m_iKernelOccupancyColumnIndex = sgprsColIndex + 1;
        }
        else
        {
            m_iKernelOccupancyColumnIndex = 0;
        }

        m_standardModel->insertColumn(m_iKernelOccupancyColumnIndex);
        m_standardModel->setHeaderData(m_iKernelOccupancyColumnIndex, Qt::Horizontal, "KernelOccupancy");
    }

    return retVal;
}

bool SessionControl::FillDataTableWithProfileData(GPUSessionTreeItemData* session)
{
    bool retVal = false;
    bool wasBreaked = false;

    GT_IF_WITH_ASSERT((m_sessionGridView != nullptr) && (session->m_pParentData != nullptr))
    {
        osFilePath sessionCSVFile;
        session->GetSessionCSVFile(sessionCSVFile);
        QString outputFileName = acGTStringToQString(sessionCSVFile.asString());

        if (QFile::exists(outputFileName))
        {
            QFile sessionFile(outputFileName);

            if (sessionFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                const QMap<uint, QList<OccupancyInfo*> >& occTable = session->LoadAndGetOccupancyTable();

                afProgressBarWrapper::instance().ShowProgressBar(s_LOADING_PERFCOUNTER_DATA_PROGRESS, 100);
                float percentDone = 0;
                qint64 fileSize = sessionFile.size();

                // skip over previously read properties section
                // this is not optimized -- see if we can cache the QFile object so that when it reads the properties in LoadProperties, it is left at the position we need it to be here
                for (int i = 0; i < session->GetPropertyCount(); i++)
                {
                    sessionFile.readLine();
                }

                char listSeparator = ',';
                QString strListSeparatorVal;

                if (session->GetProperty("ListSeparator", strListSeparatorVal))
                {
                    QByteArray bytes = strListSeparatorVal.trimmed().toLatin1();

                    if (bytes.length() > 0)
                    {
                        listSeparator = bytes[0];
                    }
                }

                m_standardModel = new QStandardItemModel(this);


                // populate header
                QStringList list = QString::fromLatin1(sessionFile.readLine()).trimmed().split(listSeparator);
                int headerCount = PopulateColumnHeaders(list, !occTable.isEmpty());

                QString currentItemText;
                QMap<uint, int> threadOccupancyCounts;

                // populate table
                int row = 0;

                while (!sessionFile.atEnd() && !wasBreaked)
                {
                    qApp->processEvents();  // this keeps the UI responsive while loading a large file

                    if (!IsControlExistInMap(this))
                    {
                        wasBreaked = true;
                        break;
                    }
                    else
                    {
                        if ((sessionFile.pos() / (float)(fileSize)) > (percentDone + 0.01f))
                        {
                            percentDone += 0.01f;
                            afProgressBarWrapper::instance().setProgressText(s_LOADING_PERFCOUNTER_DATA_PROGRESS);
                            afProgressBarWrapper::instance().updateProgressBar((int)(percentDone * 100));
                        }

                        QStringList sessionFileStrList = QString::fromLatin1(sessionFile.readLine()).trimmed().split(listSeparator);

                        // avoid going out of bounds
                        int upperLimit = m_iKernelOccupancyColumnIndex;

                        if (upperLimit < 0 || upperLimit >= sessionFileStrList.size())
                        {
                            upperLimit = sessionFileStrList.size();
                        }

                        // only add data for columns that have headers
                        if (upperLimit > headerCount)
                        {
                            upperLimit = headerCount;
                        }

                        // add items before the occupancy column
                        for (int column = 0; column < upperLimit; column++)
                        {
                            QStandardItem* item = new PerfCounterItem();

                            item->setEditable(false);
                            currentItemText = Util::RemoveTrailingZero(sessionFileStrList.value(column).trimmed());
                            item->setData(currentItemText, Qt::DisplayRole);
                            QFont cellFont = item->font();

                            if (column == 0 && Util::IsCodeAvailable(session, currentItemText))
                            {
                                item->setForeground(palette().link());
                                cellFont.setUnderline(true);
                                item->setFont(cellFont);
                            }

                            m_standardModel->setItem(row, column, item);
                        }

                        if (m_iKernelOccupancyColumnIndex != -1)
                        {
                            // add the occupancy column
                            QString strThreadId = sessionFileStrList.value(m_iThreadColumnIndex).trimmed();
                            bool status;
                            uint threadId = strThreadId.toUInt(&status);

                            if (status)
                            {
                                int curIndex = 0;

                                // map the occupancy data to kernel, based on sequential dispatches per thread
                                if (threadOccupancyCounts.contains(threadId))
                                {
                                    curIndex = threadOccupancyCounts[threadId] + 1;
                                }

                                if ((occTable.contains(threadId)) && ((curIndex) < occTable[threadId].count()))
                                {
                                    OccupancyInfo* curKernelOccupancyInfo = occTable[threadId][curIndex];

                                    QString strMethod = sessionFileStrList.value(m_iMethodColumnIndex).trimmed();

                                    if (!strMethod.isEmpty())
                                    {
                                        strMethod = strMethod.right(strMethod.length() - strMethod.lastIndexOf('_') - 1);

                                        if (strMethod.startsWith(curKernelOccupancyInfo->GetDeviceName()))
                                        {
                                            if (threadOccupancyCounts.contains(threadId))
                                            {
                                                threadOccupancyCounts[threadId] = curIndex;
                                            }
                                            else
                                            {
                                                threadOccupancyCounts.insert(threadId, curIndex);
                                            }

                                            if (curKernelOccupancyInfo->GetOccupancy() >= 0)
                                            {
                                                QStandardItem* item = new PerfCounterItem();

                                                item->setEditable(false);
                                                item->setData(Util::RemoveTrailingZero(QString::number(curKernelOccupancyInfo->GetOccupancy())), Qt::DisplayRole);
                                                item->setForeground(palette().link());
                                                QFont cellFont = item->font();
                                                cellFont.setUnderline(true);
                                                item->setFont(cellFont);
                                                m_standardModel->setItem(row, m_iKernelOccupancyColumnIndex, item);
                                            }

                                            m_rowOccupancyInfoMap.insert(row, curKernelOccupancyInfo);
                                        }
                                    }
                                }
                            }

                            // add items after the occupancy column
                            // only add data for columns that have headers
                            upperLimit = sessionFileStrList.size();

                            if (upperLimit > headerCount - 1)
                            {
                                upperLimit = headerCount - 1;
                            }

                            for (int column = m_iKernelOccupancyColumnIndex; column < upperLimit; column++)
                            {
                                QStandardItem* item = new PerfCounterItem();

                                item->setEditable(false);
                                item->setData(Util::RemoveTrailingZero(sessionFileStrList.value(column).trimmed()), Qt::DisplayRole);
                                m_standardModel->setItem(row, column + 1, item);
                            }
                        }

                        row++;
                    }
                }

                sessionFile.close();
                afProgressBarWrapper::instance().hideProgressBar();
            }
        }

        if (!wasBreaked)
        {

            m_sessionGridView->horizontalHeader()->setSectionsMovable(true);

            m_sessionGridView->setSortingEnabled(true);
            m_sessionGridView->setModel(m_standardModel);
            retVal = true;
        }
        else
        {
            retVal = false;
        }
    }

    return retVal;
}

void SessionControl::ShowAllColumns(QTableView* tblView)
{
    GT_IF_WITH_ASSERT(tblView != nullptr && tblView->model() != nullptr)
    {
        // set the column's visibility
        for (int column = 0; column < tblView->model()->columnCount(); column++)
        {
            tblView->setColumnHidden(column, false);
        }
    }
}

void SessionControl::RemoveEmptyColumns(QTableView* tblView)
{
    GT_IF_WITH_ASSERT(tblView != nullptr && tblView->model() != nullptr)
    {
        bool hide;

        QModelIndex itemIndex;
        QString dataValue;

        for (int col = 0; col < tblView->model()->columnCount(); col++)
        {
            hide = true;

            for (int row = 0; row < tblView->model()->rowCount(); row++)
            {
                itemIndex = m_standardModel->index(row, col);

                if (itemIndex.isValid())
                {
                    dataValue = m_standardModel->data(itemIndex).toString();
                }

                if (!dataValue.isEmpty() && !Util::IsZeroValue(dataValue))
                {
                    hide = false;
                    break;  // once we've decided not to hide this column, no need to check more rows
                }
            }

            tblView->setColumnHidden(col, hide);
        }
    }
}

void SessionControl::SetColumnVisibility(bool showZeroColumn, QTableView* tblView)
{
    if (showZeroColumn)
    {
        ShowAllColumns(tblView);
    }
    else
    {
        RemoveEmptyColumns(tblView);
    }
}

void SessionControl::SetSessionDataGridVisibility()
{
    SetColumnVisibility(GetShowZeroColumnCB()->isEnabled(), m_sessionGridView);
}

void SessionControl::AssignTooltips(QTableView* tableView)
{
    int ColumnCount = tableView->model()->columnCount();

    for (int col = 0; col < ColumnCount; col++)
    {
        QString header = tableView->model()->headerData(col, Qt::Horizontal).toString().trimmed();

        if (header.indexOf(" (%)") != -1)
        {
            header = header.left(header.indexOf(" (%)"));
        }

        bool descriptionFound = false;
        QString strCounterDesc;

        if (!descriptionFound && CounterManager::Instance()->IsHardwareFamilySupported(VOLCANIC_ISLANDS_FAMILY))
        {
            descriptionFound = CounterManager::Instance()->GetCounterDesc(VOLCANIC_ISLANDS_FAMILY, header, strCounterDesc);
        }

        if (!descriptionFound && CounterManager::Instance()->IsHardwareFamilySupported(SEA_ISLANDS_FAMILY))
        {
            descriptionFound = CounterManager::Instance()->GetCounterDesc(SEA_ISLANDS_FAMILY, header, strCounterDesc);
        }

        if (!descriptionFound && CounterManager::Instance()->IsHardwareFamilySupported(SOUTHERN_ISLANDS_FAMILY))
        {
            descriptionFound = CounterManager::Instance()->GetCounterDesc(SOUTHERN_ISLANDS_FAMILY, header, strCounterDesc);
        }

        // check non-hardware counters (SC stats, etc.)
        if (!descriptionFound)
        {
            descriptionFound = CounterManager::Instance()->GetCounterDesc(NA_HARDWARE_FAMILY, header, strCounterDesc);
        }

        if (descriptionFound)
        {
            m_standardModel->horizontalHeaderItem(col)->setToolTip(strCounterDesc);
        }
    }
}

OccupancyInfo* SessionControl::GetOccupancyForRow(int rowIndex, const QString& kernelName) const
{
    OccupancyInfo* retVal = nullptr;

    if (m_rowOccupancyInfoMap.contains(rowIndex))
    {
        retVal = m_rowOccupancyInfoMap[rowIndex];

        if (retVal != nullptr)
        {
            // If the occupancy info doesn't match the one in the map, this means that the user sorted the table,
            // and the information in the map is no longer reflecting the correct line numbers:
            if (!kernelName.startsWith(retVal->GetKernelName()))
            {
                QMap<int, OccupancyInfo*>::const_iterator iter = m_rowOccupancyInfoMap.begin();

                for (; iter != m_rowOccupancyInfoMap.end(); iter++)
                {
                    if ((*iter) != nullptr)
                    {
                        if (kernelName.startsWith((*iter)->GetKernelName()))
                        {
                            retVal = (*iter);
                            break;
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

void SessionControl::onUpdateEdit_Copy(bool& isEnabled)
{
    if (m_sessionGridView != nullptr)
    {
        isEnabled = !m_sessionGridView->selectedIndexes().isEmpty();
    }
}

void SessionControl::onUpdateEdit_SelectAll(bool& isEnabled)
{
    if (m_sessionGridView != nullptr)
    {
        isEnabled = true;
    }
}
void SessionControl::OnEditCopy()
{
    GT_IF_WITH_ASSERT(m_sessionGridView != nullptr)
    {
        m_sessionGridView->onEditCopy();
    }
}

void SessionControl::OnEditSelectAll()
{
    GT_IF_WITH_ASSERT(m_sessionGridView != nullptr)
    {
        m_sessionGridView->onEditSelectAll();
    }
}


bool PerfCounterItem::operator< (const QStandardItem& other) const
{
    bool ok = false;
    int int1, int2;
    QString thisText = text();
    QString otherText = other.text();

    // first check if int conversion works
    int1 = thisText.toInt(&ok);

    if (ok)
    {
        int2 = otherText.toInt(&ok);

        if (ok)
        {
            return int1 < int2;
        }
    }

    // then try double conversion
    double d1, d2;
    d1 = thisText.toDouble(&ok);

    if (ok)
    {
        d2 = otherText.toDouble(&ok);

        if (ok)
        {
            return d1 < d2;
        }
    }

    // if both of the above fail, then fall back to alphabetical comparison
    return QStandardItem::operator<(other);
}

bool SessionControl::IsControlExistInMap(QWidget* wid)
{
    return m_sessionControlsMap.contains(wid);
}

