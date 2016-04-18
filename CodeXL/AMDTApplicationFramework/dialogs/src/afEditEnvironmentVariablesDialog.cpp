//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afEditEnvironmentVariablesDialog.cpp
///
//==================================================================================

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTApplicationFramework/Include/dialogs/afEditEnvironmentVariablesDialog.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

#include <QtWidgets/QHeaderView>

#define AF_ENV_VAR_DLG_NAME_COL     0
#define AF_ENV_VAR_DLG_VALUE_COL    1

// ---------------------------------------------------------------------------
// Name:        afEditEnvironmentVariablesDialog::afEditEnvironmentVariablesDialog
// Description: Constructor
// Arguments:   QWidget *pParent
//              gtList<osEnvironmentVariable>& envVars
// Author:      Yoni Rabin
// Date:        11/4/2012
// ---------------------------------------------------------------------------
afEditEnvironmentVariablesDialog::afEditEnvironmentVariablesDialog(QWidget* pParent, gtList<osEnvironmentVariable>& envVars)
    : QDialog(pParent, Qt::WindowCloseButtonHint),
      m_envVarsString(L""),
      m_envVars(envVars),
      m_isDuringCellInsertion(false)
{
    // Init and Set the Layout components:
    initLayout();

    // Fill the input environment variables into the list:
    for (gtList<osEnvironmentVariable>::const_iterator it = envVars.begin(); it != envVars.end(); ++it)
    {
        addEditableRow(it->_name, it->_value);
    }

    // Add the last row:
    if (envVars.length() == 0)
    {
        addEditableRow(L"", L"");
    }

    addEndRow();

    // Connecting the events- this must come after grid initialization to avoid unnecessary calls:
    connect(m_pEnvironmentVariablesGrid, SIGNAL(cellEntered(int, int)), this, SLOT(onCellEntered(int, int)));
    connect(m_pEnvironmentVariablesGrid, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(onCurrentCellChanged(int, int, int, int)));
    connect(m_pEnvironmentVariablesGrid, SIGNAL(cellChanged(int, int)), this, SLOT(onCellChanged(int, int)));

    // Centers the dialog in the middle of the screen:
    centerOnScreen();
}

// ---------------------------------------------------------------------------
// Name:        afEditEnvironmentVariablesDialog::~afEditEnvironmentVariablesDialog
// Description: destructor
// Author:      Yoni Rabin
// Date:        12/4/2012
// ---------------------------------------------------------------------------
afEditEnvironmentVariablesDialog::~afEditEnvironmentVariablesDialog()
{}

// ---------------------------------------------------------------------------
// Name:        afEditEnvironmentVariablesDialog::addEditableRow
// Description: function to add a new row
// Arguments:   const gtString & name
//              const gtString & val
// Author:      Yoni Rabin
// Date:        15/4/2012
// ---------------------------------------------------------------------------
void afEditEnvironmentVariablesDialog::addEditableRow(const gtString& name, const gtString& val)
{
    // Avoid entering here when we add an item:
    if (!m_isDuringCellInsertion)
    {
        m_isDuringCellInsertion = true;

        // Get the index of the row about to be added:
        int newRowNumber = m_pEnvironmentVariablesGrid->rowCount();

        // Create an empty line:
        QStringList row;
        row << acGTStringToQString(name) << acGTStringToQString(val);
        m_pEnvironmentVariablesGrid->addRow(row, nullptr);

        // Get the left-most item of the new line:
        QTableWidgetItem* pFirstItemInNewLine = m_pEnvironmentVariablesGrid->item(newRowNumber, AF_ENV_VAR_DLG_NAME_COL);
        GT_IF_WITH_ASSERT(pFirstItemInNewLine != nullptr)
        {
            // Make it editable:
            pFirstItemInNewLine->setFlags(pFirstItemInNewLine->flags() | Qt::ItemIsEditable | Qt::ItemIsSelectable);
            pFirstItemInNewLine->setTextColor(Qt::black);
        }

        QTableWidgetItem* pSecondItemInNewLine = m_pEnvironmentVariablesGrid->item(newRowNumber, AF_ENV_VAR_DLG_VALUE_COL);
        GT_IF_WITH_ASSERT(pSecondItemInNewLine != nullptr)
        {
            // Make it editable:
            pSecondItemInNewLine->setFlags(pSecondItemInNewLine->flags() | Qt::ItemIsEditable | Qt::ItemIsSelectable);
            pSecondItemInNewLine->setTextColor(Qt::black);
        }
        m_isDuringCellInsertion = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        afEditEnvironmentVariablesDialog::removeRow
// Description: utility function to remove a row
// Arguments:   int row
// Author:      Yoni Rabin
// Date:        15/4/2012
// ---------------------------------------------------------------------------
void afEditEnvironmentVariablesDialog::removeRow(int row)
{
    if (row < m_pEnvironmentVariablesGrid->rowCount() - 1)
    {
        m_pEnvironmentVariablesGrid->removeRow(row);
        m_pEnvironmentVariablesGrid->setEnabled(false);
        m_pEnvironmentVariablesGrid->setEnabled(true);

    }
}

// ---------------------------------------------------------------------------
// Name:        afEditEnvironmentVariablesDialog::setEditable
// Description: set the cell to be editable/non editable
// Arguments:   int row
//              int col
//              bool editable
// Return Val:  void
// Author:      Yoni Rabin
// Date:        15/4/2012
// ---------------------------------------------------------------------------
void afEditEnvironmentVariablesDialog::setItemEditable(int row, int col, bool editable)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pEnvironmentVariablesGrid != nullptr)
    {
        QTableWidgetItem* pCell = m_pEnvironmentVariablesGrid->item(row, col);
        GT_IF_WITH_ASSERT(pCell != nullptr)
        {
            if (editable)
            {
                pCell->setFlags(pCell->flags() | Qt::ItemIsEditable  | Qt::ItemIsSelectable);
            }
            else
            {
                pCell->setFlags(pCell->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afEditEnvironmentVariablesDialog::addEndRow
// Description: Add a new row at the end of the grid
// Author:      Yoni Rabin
// Date:        15/4/2012
// ---------------------------------------------------------------------------
void afEditEnvironmentVariablesDialog::addEndRow()
{
    QStringList row;
    row << AF_STR_EditVariablesDialogAddEnvironmentVariable << "";
    m_pEnvironmentVariablesGrid->addRow(row, nullptr);

    // Get the added item:
    QTableWidgetItem* pCurrentItem = m_pEnvironmentVariablesGrid->item(m_pEnvironmentVariablesGrid->rowCount() - 1, AF_ENV_VAR_DLG_NAME_COL);
    GT_IF_WITH_ASSERT(pCurrentItem != nullptr)
    {
        // Gray out text:
        pCurrentItem->setTextColor(Qt::gray);
        pCurrentItem->setText(AF_STR_EditVariablesDialogAddEnvironmentVariable);

        // Name is editable:
        pCurrentItem->setFlags(pCurrentItem->flags() | Qt::ItemIsEditable);

        // Value is not editable:
        pCurrentItem = m_pEnvironmentVariablesGrid->item(m_pEnvironmentVariablesGrid->rowCount() - 1, AF_ENV_VAR_DLG_VALUE_COL);
        pCurrentItem->setTextColor(Qt::black);
        pCurrentItem->setFlags(pCurrentItem->flags() & ~Qt::ItemIsEditable);
    }
}

// ---------------------------------------------------------------------------
// Name:        afEditEnvironmentVariablesDialog::accept
// Description: load the env variable strings and make sure they are correct
// Author:      Yoni Rabin
// Date:        12/4/2012
// ---------------------------------------------------------------------------
void afEditEnvironmentVariablesDialog::accept()
{
    // Read env vars from grid:
    m_envVarsString.makeEmpty();
    m_envVars.clear();

    gtString name;
    gtString val;

    // This triggers onEndEdit event to get the last edited cell in case we are still editing it:
    m_pEnvironmentVariablesGrid->setEnabled(false);
    m_pEnvironmentVariablesGrid->setEnabled(true);

    // Assume that last row is always empty:
    int numRows = m_pEnvironmentVariablesGrid->rowCount() - 1;

    for (int i = 0; i < numRows; ++i)
    {
        m_pEnvironmentVariablesGrid->getItemText(i, AF_ENV_VAR_DLG_NAME_COL, name);
        m_pEnvironmentVariablesGrid->getItemText(i, AF_ENV_VAR_DLG_VALUE_COL, val);

        //If this row is not empty:
        if (!name.isEmpty() && !rowExists(i))
        {
            // Add this environment variable to the string:
            osEnvironmentVariable row;
            row._name = name;
            row._value = val;
            m_envVars.push_back(row);
            m_envVarsString.append(name).append('=').append(val).append(AF_STR_newProjectEnvironmentVariablesDelimiter);
        }
    }

    m_envVarsString.removeTrailing(L';');

    // Call the dialog accept:
    QDialog::accept();
}

// ---------------------------------------------------------------------------
// Name:        afEditEnvironmentVariablesDialog::initLayout
// Description: Initialize the dialog layout
// Author:      Yoni Rabin
// Date:        11/4/2012
// ---------------------------------------------------------------------------
void afEditEnvironmentVariablesDialog::initLayout()
{
    // Set the title:
    gtString title = afGlobalVariablesManager::instance().ProductName();
    setWindowTitle(AF_STR_EditVariablesDialogTitle);

    // Add the dialog's description to the layout:
    m_pEditVarsDialogDescription = new QLabel(AF_STR_EditVariablesDialogDescription);

    // Add the grid:
    m_pEnvironmentVariablesGrid = new acListCtrl(this, AC_DEFAULT_LINE_HEIGHT, true);
    m_pEnvironmentVariablesGrid->setContextMenuPolicy(Qt::NoContextMenu);
    m_pEnvironmentVariablesGrid->setEnablePaste(true);

    // Set the column headers:
    QStringList columnCaptions;
    columnCaptions << AF_STR_EditVariablesDialogVariableNameColumnTitle;
    columnCaptions << AF_STR_ValueA;
    gtVector<float> colVec;
    colVec.push_back(0.5);
    colVec.push_back(0.5);
    m_pEnvironmentVariablesGrid->initHeaders(columnCaptions, colVec, false);
    m_pEnvironmentVariablesGrid->setShowGrid(true);

    m_pEnvironmentVariablesGrid->horizontalHeader()->setSectionsClickable(false);

    m_pEnvironmentVariablesGrid->setSelectionBehavior(QAbstractItemView::SelectItems);
    //m_pEnvironmentVariablesGrid->setFocusPolicy(Qt::StrongFocus);
    m_pEnvironmentVariablesGrid->setEditTriggers(QAbstractItemView::AllEditTriggers);

    // Add the OK + Cancel buttons:
    QDialogButtonBox* pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    pButtonBox->setCenterButtons(true);

    // Connect the buttons to slots:
    bool rc = connect(pButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
    GT_ASSERT(rc);

    rc = connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
    GT_ASSERT(rc);

    // Creating the main layout:
    m_pLayout = new QVBoxLayout();

    m_pLayout->addWidget(m_pEditVarsDialogDescription);
    m_pLayout->addSpacing(5);
    m_pLayout->addWidget(m_pEnvironmentVariablesGrid);
    m_pLayout->addWidget(pButtonBox);
    m_pLayout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(m_pLayout);
}

void afEditEnvironmentVariablesDialog::onCellChanged(int row, int column)
{
    GT_UNREFERENCED_PARAMETER(row);
    GT_UNREFERENCED_PARAMETER(column);

    m_pEnvironmentVariablesGrid->clearFocus();
    m_pEnvironmentVariablesGrid->clearSelection();
}


// ---------------------------------------------------------------------------
// Name:        afEditEnvironmentVariablesDialog::onCellEntered
// Description: onCellEntered event
// Arguments:   QTableWidgetItem* pItem
// Author:      Yoni Rabin
// Date:        15/4/2012
// ---------------------------------------------------------------------------
void afEditEnvironmentVariablesDialog::onCellEntered(int row, int column)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pEnvironmentVariablesGrid != nullptr)
    {
        QTableWidgetItem* pItem = m_pEnvironmentVariablesGrid->item(row, column);
        GT_IF_WITH_ASSERT(pItem != nullptr)
        {
            if (column == 0 && row == m_pEnvironmentVariablesGrid->rowCount() - 1)
            {
                // Turn this row to normal row:
                pItem->setTextColor(Qt::black);
                pItem->setText("");
                setItemEditable(row, AF_ENV_VAR_DLG_VALUE_COL, true);
            }

            //m_pEnvironmentVariablesGrid->editItempItem->setSelected(true);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afEditEnvironmentVariablesDialog::onCurrentCellChanged
// Description: onCurrentCellChanged event
// Arguments:   int currentRow
//              int currentColumn
//              int previousRow
//              int previousColumn
// Author:      Yoni Rabin
// Date:        16/4/2012
// ---------------------------------------------------------------------------
void afEditEnvironmentVariablesDialog::onCurrentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pEnvironmentVariablesGrid != nullptr)
    {
        QTableWidgetItem* pItem = m_pEnvironmentVariablesGrid->item(currentRow, currentColumn);
        GT_IF_WITH_ASSERT(pItem != nullptr)
        {
            // If the selected row is the last row, and this is the left column, set to black, editable and clear the default text
            if (currentRow == m_pEnvironmentVariablesGrid->rowCount() - 1 && currentColumn == AF_ENV_VAR_DLG_NAME_COL)
            {
                // Turn this row into a normal row:
                pItem->setTextColor(Qt::black);
                pItem->setText("");
                setItemEditable(currentRow, AF_ENV_VAR_DLG_VALUE_COL, true);
            }

            pItem->setSelected(true);
        }

        // Ignore first entrance
        if (previousRow >= 0)
        {
            QTableWidgetItem* pPrevItem = m_pEnvironmentVariablesGrid->item(previousRow, previousColumn);
            GT_IF_WITH_ASSERT(pPrevItem != nullptr)
            {
                // If this is the Name column:
                if (previousColumn == AF_ENV_VAR_DLG_NAME_COL)
                {
                    // If this is the last row:
                    if (previousRow == m_pEnvironmentVariablesGrid->rowCount() - 1)
                    {
                        // Only add an end row if we put a value in the previous row:
                        if (!pPrevItem->text().trimmed().isEmpty())
                        {
                            addEndRow();
                        }
                        else
                        {
                            // Reset last row to be the end row:
                            pPrevItem->setTextColor(Qt::gray);
                            pPrevItem->setText(AF_STR_EditVariablesDialogAddEnvironmentVariable);
                            setItemEditable(previousRow, AF_ENV_VAR_DLG_VALUE_COL, false);

                            pItem = m_pEnvironmentVariablesGrid->item(previousRow, AF_ENV_VAR_DLG_VALUE_COL);
                            // to make disabling the value cell effective - make sure it's not open for edit
                            GT_IF_WITH_ASSERT(nullptr != pItem)
                            {
                                m_pEnvironmentVariablesGrid->closePersistentEditor(pItem);
                            }
                        }
                    }

                    // If this is a duplicate row or we deleted the name remove the row:
                    if (rowExists(previousRow, true) || pPrevItem->text().isEmpty())
                    {
                        removeRow(previousRow);
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afEditEnvironmentVariablesDialog::centerOnScreen
// Description: Places the QDialog in the center of the screen
// Author:      Yoni Rabin
// Date:        11/4/2012
// ---------------------------------------------------------------------------
void afEditEnvironmentVariablesDialog::centerOnScreen()
{
    setGeometry(0, 0, this->width(), this->height());

    //Put the dialog in the screen center:
    const QRect screen = QApplication::desktop()->screenGeometry();
    move(screen.center() - rect().center());
}

// ---------------------------------------------------------------------------
// Name:        afEditEnvironmentVariablesDialog::rowExists
// Description: Check whether the name exists in the grid,
//              if searchAll is specified will search after row as well
// Arguments:   int row
//              bool searchAll
// Return Val:  bool - Success / failure.
// Author:      Yoni Rabin
// Date:        17/4/2012
// ---------------------------------------------------------------------------
bool afEditEnvironmentVariablesDialog::rowExists(int row, bool searchAll)
{
    bool retVal = false;
    // Sanity:
    GT_ASSERT(row < m_pEnvironmentVariablesGrid->rowCount());

    // Init variables:
    gtString name, current;
    int upperBound;

    // Whether to run on all elements or just until the current row
    if (searchAll)
    {
        upperBound = m_pEnvironmentVariablesGrid->rowCount() - 1 ;
    }
    else
    {
        upperBound = row;
    }

    m_pEnvironmentVariablesGrid->getItemText(row, AF_ENV_VAR_DLG_NAME_COL, name);

    // Loop over the grid and look for the element:
    for (int i = 0; i < upperBound; ++i)
    {
        // Do not check against same element:
        if (i == row)
        {
            continue;
        }

        m_pEnvironmentVariablesGrid->getItemText(i, AF_ENV_VAR_DLG_NAME_COL, current);

        // if we have found an element with the same name:
        if (name == current)
        {
            // return true:
            retVal = true;
            break;
        }
    }

    return retVal;
}

