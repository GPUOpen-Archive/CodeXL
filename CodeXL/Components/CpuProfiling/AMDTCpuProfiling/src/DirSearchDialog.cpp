//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DirSearchDialog.cpp
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/DirSearchDialog.cpp#13 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

//QT
#include <QtCore>
#include <QtWidgets>
#include <QFileDialog>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <inc/DirSearchDialog.h>

#include <AMDTBaseTools/Include/gtAssert.h>


DirSearchDialog::DirSearchDialog(QWidget* parent, Qt::WindowFlags fl)
    : QDialog(parent, fl)
{
    setWindowModality(Qt::WindowModal);
    //set up the ui
    setupUi(this);

    m_pNewLine->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    m_pRemoveLine->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    m_pUp->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
    m_pDown->setIcon(style()->standardIcon(QStyle::SP_ArrowDown));

    m_pDirTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(m_pNewLine, SIGNAL(clicked()), SLOT(onAdd()));
    connect(m_pRemoveLine, SIGNAL(clicked()), SLOT(onRemove()));
    connect(m_pUp, SIGNAL(clicked()), SLOT(onUp()));
    connect(m_pDown, SIGNAL(clicked()), SLOT(onDown()));
    connect(m_pDirTable, SIGNAL(cellDoubleClicked(int, int)),
            SLOT(onDoubleClick(int)));
    connect(m_pDirTable, SIGNAL(currentCellChanged(int, int, int, int)),
            SLOT(onCurrentChanged(int)));
}

DirSearchDialog::~DirSearchDialog()
{
}

void DirSearchDialog::setDirList(QString dirList)
{
    QStringList dirs = dirList.split(";");
    m_pDirTable->setRowCount(dirs.size());

    for (int i = 0; i < dirs.size(); i++)
    {
        m_pDirTable->setItem(i, 0, new QTableWidgetItem(dirs.at(i)));
    }
}

QString DirSearchDialog::getDirList()
{
    QString dirList;

    for (int i = 0; i < m_pDirTable->rowCount(); i++)
    {
        dirList += m_pDirTable->item(i, 0)->text() + ";";
    }

    return dirList;
}

void DirSearchDialog::setCurrentRow(int row)
{
    m_pDirTable->clearSelection();
    m_pDirTable->selectRow(row);
    m_pDirTable->setCurrentCell(row, 0);
    m_pDirTable->update();
}

void DirSearchDialog::onAdd()
{
    m_pDirTable->insertRow(0);
    m_pDirTable->setItem(0, 0, new QTableWidgetItem(""));
    setCurrentRow(0);
}

void DirSearchDialog::onRemove()
{
    int row = m_pDirTable->currentRow();
    bool lastRow = ((m_pDirTable->rowCount() - 1) == row);
    m_pDirTable->removeRow(row);

    if (lastRow)
    {
        row--;
    }

    setCurrentRow(row);

    if ((0 == row) || ((m_pDirTable->rowCount() - 1) == row))
    {
        onCurrentChanged(row);
    }
}

void DirSearchDialog::onUp()
{
    int row = m_pDirTable->currentRow();

    //shouldn't happen, but just in case, don't crash
    if (0 == row)
    {
        onCurrentChanged(0);
        return;
    }

    QString temp = m_pDirTable->item(row, 0)->text();
    m_pDirTable->item(row, 0)->setText(m_pDirTable->item((row - 1), 0)->text());
    m_pDirTable->item((row - 1), 0)->setText(temp);
    setCurrentRow(row - 1);
}

void DirSearchDialog::onDown()
{
    int row = m_pDirTable->currentRow();

    //shouldn't happen, but just in case, don't crash
    if ((m_pDirTable->rowCount() - 1) == row)
    {
        onCurrentChanged((m_pDirTable->rowCount() - 1));
        return;
    }

    QString temp = m_pDirTable->item(row, 0)->text();
    m_pDirTable->item(row, 0)->setText(m_pDirTable->item((row + 1), 0)->text());
    m_pDirTable->item((row + 1), 0)->setText(temp);
    setCurrentRow(row + 1);
}

void DirSearchDialog::onDoubleClick(int row)
{
    QString path = QFileDialog::getExistingDirectory(m_pDirTable,
                                                     "Locate additional directory", m_pDirTable->item(row, 0)->text());

    if (!path.isNull())
    {
        osFilePath filePath;
        filePath.setFullPathFromString(acQStringToGTString(path), true);
        m_pDirTable->item(row, 0)->setText(acGTStringToQString(filePath.asString()));
    }
}

void DirSearchDialog::onCurrentChanged(int row)
{
    m_pRemoveLine->setEnabled(true);
    m_pUp->setEnabled(true);
    m_pDown->setEnabled(true);

    if (0 == row)
    {
        m_pUp->setEnabled(false);
    }

    if ((m_pDirTable->rowCount() - 1) == row)
    {
        m_pDown->setEnabled(false);
    }

    if (0 > row)
    {
        m_pUp->setEnabled(false);
        m_pDown->setEnabled(false);
        m_pRemoveLine->setEnabled(false);
    }
} //DirSearchDialog::onCurrentChanged
