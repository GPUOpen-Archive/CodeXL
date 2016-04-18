//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afMultipleDirectoriesBrowseDialog.cpp
///
//==================================================================================

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>
#include <QFileDialog>

// Infra:
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// Local:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/dialogs/afMultipleDirectoriesBrowseDialog.h>

#define AF_MULTIPLE_DIRECTORIES_DIALOG_WIDTH 650
#define AF_MULTIPLE_DIRECTORIES_DIALOG_HEIGHT 300


afMultipleDirectoriesBrowseDialog::afMultipleDirectoriesBrowseDialog(QWidget* pParent)
    : acDialog(pParent), m_pLastClickedItem(nullptr)
{
    // Window modality:
    setWindowModality(Qt::WindowModal);

    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    setWindowTitle(AF_STR_MultiDirDialogCaption);

    // Set up the layout:
    SetDialogLayout();

    // Connect widgets to the slots:
    bool rc = connect(m_pNewDirectoryButton, SIGNAL(clicked()), SLOT(OnAddDirectory()));
    GT_ASSERT(rc);

    rc = connect(m_pRemoveDirectoryButton, SIGNAL(clicked()), SLOT(OnRemoveDirectory()));
    GT_ASSERT(rc);
    rc = connect(m_pUpButton, SIGNAL(clicked()), SLOT(OnUp()));
    GT_ASSERT(rc);
    rc = connect(m_pDownButton, SIGNAL(clicked()), SLOT(OnDown()));
    GT_ASSERT(rc);

    rc = connect(m_pDirTable, SIGNAL(cellDoubleClicked(int, int)), SLOT(OnDoubleClick(int)));
    GT_ASSERT(rc);

    rc = connect(m_pDirTable, SIGNAL(itemSelectionChanged()), SLOT(OnCurrentChanged()));
    GT_ASSERT(rc);

    OnCurrentChanged();

}

afMultipleDirectoriesBrowseDialog::~afMultipleDirectoriesBrowseDialog()
{
}

void afMultipleDirectoriesBrowseDialog::SetDirectoriesList(const QString& dirList)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pDirTable != nullptr)
    {
        QStringList dirs = dirList.split(";");

        foreach (QString dir, dirs)
        {
            if (!dir.isEmpty())
            {
                m_pDirTable->addRow(dir);
                QTableWidgetItem* pLastAddedItem = m_pDirTable->item(m_pDirTable->rowCount() - 1, 0);
                GT_IF_WITH_ASSERT(pLastAddedItem != nullptr)
                {
                    pLastAddedItem->setToolTip(dir);
                }
            }
        }

        for (int i = 0 ; i < m_pDirTable->rowCount(); i++)
        {
            m_pDirTable->enableRowEditing(i, true);
            QTableWidgetItem* pItem = m_pDirTable->item(i, 0);

            if (pItem != nullptr)
            {
                pItem->setFlags(pItem->flags() | Qt::ItemIsEditable | Qt::ItemIsEnabled);
            }
        }
    }
}

QString afMultipleDirectoriesBrowseDialog::GetDirectoriesList()
{
    QString dirList;

    for (int i = 0; i < m_pDirTable->rowCount(); i++)
    {
        if (i > 0)
        {
            // Add ";" to separate previous item:
            dirList += ";";
        }

        // Add the current item:
        dirList += m_pDirTable->item(i, 0)->text();
    }

    return dirList;
}

void afMultipleDirectoriesBrowseDialog::SetCurrentRow(int row)
{
    m_pDirTable->clearSelection();
    m_pDirTable->selectRow(row);
    m_pDirTable->setCurrentCell(row, 0);
    m_pDirTable->update();
}

void afMultipleDirectoriesBrowseDialog::OnAddDirectory()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pDirTable != nullptr)
    {
        // Check if there is already new directory:
        bool newItemExists = false;
        QTableWidgetItem* pFirstItem = m_pDirTable->item(0, 0);

        if (pFirstItem != nullptr)
        {
            newItemExists = pFirstItem->text().isEmpty();
        }

        if (!newItemExists)
        {
            m_pDirTable->addEmptyRows(0, 1);
            pFirstItem = m_pDirTable->item(0, 0);
        }

        GT_IF_WITH_ASSERT(pFirstItem != nullptr)
        {
            pFirstItem->setFlags(pFirstItem->flags() | Qt::ItemIsEditable | Qt::ItemIsEnabled);

            // Set the new row as current:
            SetCurrentRow(0);

            // Update the tooltips:
            pFirstItem->setToolTip("Double-click to browse for a source directory...");

            for (int i = 1; i < m_pDirTable->rowCount(); i++)
            {
                QTableWidgetItem* pCurrent = m_pDirTable->item(i, 0);

                if (pCurrent != nullptr)
                {
                    pCurrent->setToolTip(pCurrent->text());
                }
            }
        }
    }
}

void afMultipleDirectoriesBrowseDialog::OnRemoveDirectory()
{
    int row = m_pDirTable->currentRow();
    bool lastRow = ((m_pDirTable->rowCount() - 1) == row);
    m_pDirTable->removeRow(row);

    if (lastRow)
    {
        row--;
    }

    SetCurrentRow(row);
}

void afMultipleDirectoriesBrowseDialog::OnUp()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pDirTable != nullptr)
    {
        int row = m_pDirTable->currentRow();

        int prevRow = row - 1;

        if (prevRow >= 0)
        {
            QString temp = m_pDirTable->item(row, 0)->text();
            m_pDirTable->item(row, 0)->setText(m_pDirTable->item((prevRow), 0)->text());
            m_pDirTable->item((prevRow), 0)->setText(temp);
            SetCurrentRow(prevRow);
        }
    }
}

void afMultipleDirectoriesBrowseDialog::OnDown()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pDirTable != nullptr)
    {
        // Get the current row:
        int row = m_pDirTable->currentRow();

        if (row >= 0)
        {
            int nextRow = row + 1;

            // Only if the next row is valid, do the operation:
            if (nextRow < m_pDirTable->rowCount())
            {
                QString temp = m_pDirTable->item(row, 0)->text();
                m_pDirTable->item(row, 0)->setText(m_pDirTable->item((nextRow), 0)->text());
                m_pDirTable->item((nextRow), 0)->setText(temp);
                SetCurrentRow(nextRow);
            }
        }
    }
}

void afMultipleDirectoriesBrowseDialog::OnDoubleClick(int row)
{
    QString text;

    QTableWidgetItem* pItem = nullptr;
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pDirTable != nullptr)
    {
        pItem = m_pDirTable->item(row, 0);

        if (pItem != nullptr)
        {
            text = pItem->text();
        }
    }

    QString path = QFileDialog::getExistingDirectory(m_pDirTable, "Locate additional directory", text);

    if (!path.isNull())
    {
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
        path.replace('\\', '/');
#elif AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        path.replace('/', '\\');
#endif

        if (pItem == nullptr)
        {
            m_pDirTable->addRow("");
            pItem = m_pDirTable->item(row, 0);
        }

        GT_IF_WITH_ASSERT(pItem != nullptr)
        {
            pItem->setText(path);
        }
    }
}

void afMultipleDirectoriesBrowseDialog::OnCurrentChanged()
{
    // Check if there are selected items in the table:
    bool isSelected = (m_pDirTable->selectedItems().count() > 0);

    m_pRemoveDirectoryButton->setEnabled(isSelected);
    m_pUpButton->setEnabled(isSelected);
    m_pDownButton->setEnabled(isSelected);

    if (isSelected)
    {
        int row = m_pDirTable->selectedItems().first()->row();

        if (0 == row)
        {
            m_pUpButton->setEnabled(false);
        }

        if ((m_pDirTable->rowCount() - 1) == row)
        {
            m_pDownButton->setEnabled(false);
        }

        QTableWidgetItem* pItem = m_pDirTable->item(row, 0);

        if (pItem != nullptr)
        {
            if (pItem->text().isEmpty())
            {
                m_pDownButton->setEnabled(false);
                m_pUpButton->setEnabled(false);
                m_pRemoveDirectoryButton->setEnabled(false);
            }
        }

    }
}

void afMultipleDirectoriesBrowseDialog::SetDialogLayout()
{
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    QToolBar* pToolbar = new QToolBar;
    pToolbar->setContentsMargins(0, 0, 0, 0);

    // Do not allow the toolbar to float:
    pToolbar->setFloatable(false);
    pToolbar->setMovable(false);
    pToolbar->setStyleSheet("QToolBar { border-style: none; margin-left:5; margin-right: 5; padding: 0}");

    m_pNewDirectoryButton = new QPushButton;
    m_pRemoveDirectoryButton = new QPushButton;
    m_pUpButton = new QPushButton;
    m_pDownButton = new QPushButton;

    // Add a spacer to the toolbar so the widgets are aligned to right:
    QWidget* pSpacer = new QWidget;
    pSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pToolbar->addWidget(pSpacer);

    pToolbar->addWidget(m_pNewDirectoryButton);
    pToolbar->addWidget(m_pRemoveDirectoryButton);
    pToolbar->addWidget(m_pUpButton);
    pToolbar->addWidget(m_pDownButton);

    m_pNewDirectoryButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    m_pRemoveDirectoryButton->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    m_pUpButton->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
    m_pDownButton->setIcon(style()->standardIcon(QStyle::SP_ArrowDown));

    m_pNewDirectoryButton->setToolTip(AF_STR_MultiDirNewTooltip);
    m_pRemoveDirectoryButton->setToolTip(AF_STR_MultiDirDeleteTooltip);
    m_pUpButton->setToolTip(AF_STR_MultiDirUpTooltip);
    m_pDownButton->setToolTip(AF_STR_MultiDirDownTooltip);

    // Create the dialog buttons:
    QDialogButtonBox* pBox = new QDialogButtonBox();

    GT_IF_WITH_ASSERT(nullptr != pBox)
    {
        QPushButton* pOkButton = new QPushButton(AF_STR_OK_Button);

        QPushButton* pCancelButton = new QPushButton(AF_STR_Cancel_Button);

        pBox->addButton(pOkButton, QDialogButtonBox::AcceptRole);
        pBox->addButton(pCancelButton, QDialogButtonBox::RejectRole);

        // Connect the Ok button:
        bool rc = connect(pBox, SIGNAL(accepted()), this, SLOT(accept()));
        GT_ASSERT(rc);
        rc = connect(pBox, SIGNAL(rejected()), this, SLOT(reject()));
        GT_ASSERT(rc);
    }

    QHBoxLayout* pLayout = new QHBoxLayout;

    QLabel* pDescriptionLabel = new QLabel(AF_Str_ProjectSettingsSoureFilesDirectoriesLabel);
    pDescriptionLabel->setWordWrap(true);
    // Force this label to be stretched longer than it would by default
    const int stretchValue = 1;
    pLayout->addWidget(pDescriptionLabel, stretchValue);

    pLayout->addWidget(pToolbar);

    pMainLayout->addLayout(pLayout);
    pMainLayout->addWidget(pDescriptionLabel);

    m_pDirTable = new acListCtrl(nullptr, AC_DEFAULT_LINE_HEIGHT, true);
    m_pDirTable->EnableEditableRowDeletion();
    m_pDirTable->setItemDelegate(new acItemDelegate);
    m_pDirTable->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);

    pMainLayout->addWidget(m_pDirTable);
    pMainLayout->addWidget(pBox, Qt::AlignRight);

    m_pDirTable->SetSelectionMode(QAbstractItemView::SingleSelection);
    m_pDirTable->setShowGrid(false);
    m_pDirTable->setColumnCount(1);

    QStringList list;
    list << AF_STR_MultiDirDialogTableHeader;
    m_pDirTable->setHorizontalHeaderLabels(list);
    QTableWidgetItem* pHeaderItem = m_pDirTable->horizontalHeaderItem(0);
    GT_IF_WITH_ASSERT(pHeaderItem != nullptr)
    {
        pHeaderItem->setTextAlignment(Qt::AlignLeft);
    }

    m_pDirTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    setLayout(pMainLayout);

    resize(AF_MULTIPLE_DIRECTORIES_DIALOG_WIDTH, AF_MULTIPLE_DIRECTORIES_DIALOG_HEIGHT);
}

