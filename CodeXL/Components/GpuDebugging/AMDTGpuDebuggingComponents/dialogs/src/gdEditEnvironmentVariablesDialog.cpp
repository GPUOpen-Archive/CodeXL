//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdEditEnvironmentVariablesDialog.cpp
///
//==================================================================================

//------------------------------ gdEditEnvironmentVariablesDialog.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// For compilers that support pre-compilation, includes "wx/wx.h".


// wxWindows
#include "wx/statline.h"

// Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acWXMessageDialog.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

// Local
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdCodeXLGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdEditEnvironmentVariablesDialog.h>

BEGIN_EVENT_TABLE(gdEditEnvironmentVariablesDialog, acDialog)
    EVT_GRID_CMD_EDITOR_SHOWN(ID_EDIT_ENVIRONMENT_VARS_DIALOG_GRID, gdEditEnvironmentVariablesDialog::onGridCellEditorShowen)
    EVT_GRID_CMD_EDITOR_HIDDEN(ID_EDIT_ENVIRONMENT_VARS_DIALOG_GRID, gdEditEnvironmentVariablesDialog::onGridCellEditorHidden)
END_EVENT_TABLE()

// ---------------------------------------------------------------------------
// Name:        gdEditEnvironmentVariablesDialog::gdEditEnvironmentVariablesDialog
// Description: constructor
// Author:      Guy Ilany
// Date:        27/11/2007
// ---------------------------------------------------------------------------
gdEditEnvironmentVariablesDialog::gdEditEnvironmentVariablesDialog(wxWindow* parent, gtString& settingsDialogEditString, gtList<osEnvironmentVariable>& envVars)
    : acDialog(parent, ID_EDIT_ENVIRONMENT_VARS_DIALOG, _(GD_STR_EditVariablesDialogTitle), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
      _dialogEditVarsString(settingsDialogEditString),
      _envVars(envVars),
      _isDuringCellInsertion(false)
{
    // Set the Layout components
    setLayout();

    // Set values
    bool rc1 = setEnvironmentVariablesValues();
    GT_ASSERT(rc1);

    // activate
    SetSizer(_pSizer);
    SetAutoLayout(true);
    _pSizer->SetSizeHints(this);
    _pSizer->Fit(this);
    Centre(wxBOTH | wxCENTRE_ON_SCREEN);
}


// ---------------------------------------------------------------------------
// Name:        gdEditEnvironmentVariablesDialog::~gdEditEnvironmentVariablesDialog
// Description: Destructor
// Author:      Avi Shapira
// Date:        17/5/2006
// ---------------------------------------------------------------------------
gdEditEnvironmentVariablesDialog::~gdEditEnvironmentVariablesDialog()
{
}


// ---------------------------------------------------------------------------
// Name:        gdEditEnvironmentVariablesDialog::ShowModal
// Description: Overrides the wxDialog ShowModal
// Return Val:  0 - failure
//              wxID_OK - success
// Author:      Guy Ilany
// Date:        27/11/2007
// ---------------------------------------------------------------------------
int gdEditEnvironmentVariablesDialog::ShowModal()
{
    _pEnvironmentVariablesGrid->SetFocus();
    int retVal = wxDialog::ShowModal();
    gtString newEnvVarsStr; // Will hold the new Environment variables (in ' ; ' '=' separation format)

    // Add the dialog chosen Environment Variables to the Debug Settings Dialog's EnvString
    if (retVal == wxID_OK)
    {
        int rowCount = _pEnvironmentVariablesGrid->GetNumberRows();
        gtString currentVarName;
        gtString currentValue;
        bool isFirstVar = true;

        // Iterate over the Grid and update the newEnvVarsStr accordingly
        for (int i = 0; i < rowCount; i++)
        {
            currentVarName = _pEnvironmentVariablesGrid->GetCellValue(i, 0).c_str();
            currentValue = _pEnvironmentVariablesGrid->GetCellValue(i, 1).c_str();

            // If current var has a non empty name (value can be empty)- update the newEnvVarsStr.
            // Else, inform the user.
            if (!(currentVarName.isEmpty() || 0 == currentVarName.compareNoCase(GD_STR_EditVariablesDialogAddEnvironmentVariable)))
            {
                // Append the current var name to the newEnvVarsStr

                // Append ';' only if this is not the first var in newEnvVarsStr
                if (!isFirstVar)
                {
                    newEnvVarsStr.append(L" ; ");
                }

                newEnvVarsStr.append(currentVarName);
                newEnvVarsStr.append(L"=");
                newEnvVarsStr.append(currentValue);
                isFirstVar = false;
            }
            else if (currentValue.isEmpty()) // var's name and value are empty
            {
                // Empty Var name and Value is empty - Do not add to newEnvVarsStr - erase the line
                _pEnvironmentVariablesGrid->DeleteRows(i);
                i--;
                rowCount--;
                continue;
            }
            else
            {
                // Generate a proper message
                gtString errorInRow;
                errorInRow.appendFormattedString(GD_STR_EditVariablesDialogErrorMsg, i + 1);
                acWXMessageBox(errorInRow.asCharArray(), GD_STR_EditVariablesDialogErrorMsgTitle, wxICON_ERROR | wxOK);
                retVal = 0;
                break;
            }
        }

        // Update the Environment Variables String in case of valid variables and values
        if (retVal == wxID_OK)
        {
            _dialogEditVarsString = newEnvVarsStr;
            _envVars.clear();
            osEnvironmentVariable currVar;

            for (int i = 0; i < rowCount; i++)
            {
                currVar._name = _pEnvironmentVariablesGrid->GetCellValue(i, 0).c_str();
                currVar._value = _pEnvironmentVariablesGrid->GetCellValue(i, 1).c_str();
                _envVars.push_back(currVar);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdEditEnvironmentVariablesDialog::setLayout
// Description: Set the dialog Layout
// Return Val:  bool - Success / failure.
// Author:      Guy Ilany
// Date:        27/11/2007
// ---------------------------------------------------------------------------
bool gdEditEnvironmentVariablesDialog::setLayout()
{
    bool rc1 = true;

    // StaticText for the description of the dialog
    _pEditVarsDialogDescription = new wxStaticText;
    _pEditVarsDialogDescription->Create(this, -1, AF_STR_Empty, wxDefaultPosition, wxDefaultSize, wxTE_RICH | wxTE_MULTILINE);
    _pEditVarsDialogDescription->SetLabel(GD_STR_EditVariablesDialogDescription);

    // Creating the Grid, with one 'insert new var here' row
    _pEnvironmentVariablesGrid = new wxGrid(this, ID_EDIT_ENVIRONMENT_VARS_DIALOG_GRID, wxDefaultPosition, wxSize(400, 150));

    // Set the initial value of the dialog
    bool gridrc = setGridLayout();
    GT_ASSERT(gridrc);

    // Get the product title:
    gtString title = afGlobalVariablesManager::instance().productTitleAsString();
    // Add the specific dialog caption
    title.append(GD_STR_EditVariablesDialogTitle);

    // Set the title:
    this->SetTitle(title.asCharArray());

    //Add the Icon to the dialog
    afLoadCodeXLTitleBarIcon(*this);

    // Creating the main Sizer
    _pSizer = new wxBoxSizer(wxVERTICAL);

    // Add the dialog's description to the Sizer
    wxSizerItem*  rc2 = _pSizer->Add(_pEditVarsDialogDescription, 0, wxALL, 10);

    if (rc2 == NULL)
    {
        rc1 = false;
    }

    // Add the Grid
    rc2 = _pSizer->Add(_pEnvironmentVariablesGrid, 0, wxALL, 10);

    if (rc2 == NULL)
    {
        rc1 = false;
    }

    // Add OK + Cancel Buttons
    rc2 = _pSizer->Add(CreateButtonSizer(wxCANCEL | wxOK), 0, wxCENTRE | wxALL, 10);

    if (rc2 == NULL)
    {
        rc1 = false;
    }

    GT_RETURN_WITH_ASSERT(rc1);
}


// ---------------------------------------------------------------------------
// Name:        gdEditEnvironmentVariablesDialog::setGridLayout
// Description: Sets up the Grid's Layout (the Row and Column Labels are of size 0)
// Author:      Guy Ilany
// Date:        28/11/2007
// ---------------------------------------------------------------------------
bool gdEditEnvironmentVariablesDialog::setGridLayout()
{
    bool rc1 = true;

    _pEnvironmentVariablesGrid->CreateGrid(1, 2);

    // Disabling the row Label
    _pEnvironmentVariablesGrid->SetRowLabelSize(0);

    // We can set the sizes of individual rows and columns
    // in pixels
    int columnWidth = afGetClientNetoWidth(_pEnvironmentVariablesGrid->GetClientSize().GetX()) / 2;
    _pEnvironmentVariablesGrid->SetColSize(0, columnWidth);
    _pEnvironmentVariablesGrid->SetColSize(1, columnWidth);
    _pEnvironmentVariablesGrid->SetDefaultColSize(columnWidth);
    _pEnvironmentVariablesGrid->SetColMinimalWidth(0, columnWidth);
    _pEnvironmentVariablesGrid->SetColMinimalWidth(1, columnWidth);
    _pEnvironmentVariablesGrid->SetRowMinimalHeight(0, 10);

    // Set Column Labels
    _pEnvironmentVariablesGrid->SetColLabelValue(0, GD_STR_EditVariablesDialogVariableNameColumnTitle);
    _pEnvironmentVariablesGrid->SetColLabelValue(1, AF_STR_Value);
    _pEnvironmentVariablesGrid->SetColLabelSize(_pEnvironmentVariablesGrid->GetColLabelSize() * 3 / 4);

    wxFont font = _pEnvironmentVariablesGrid->GetDefaultCellFont();

    // Using uninitialized fonts can cause crashes in Linux
    if (!font.Ok())
    {
        font = *wxNORMAL_FONT;
    }

    _pEnvironmentVariablesGrid->SetLabelFont(font);

    // Set the first row to be 'enter new var name here...' text
    _pEnvironmentVariablesGrid->SetCellValue(0, 0, GD_STR_EditVariablesDialogAddEnvironmentVariable);
    _pEnvironmentVariablesGrid->SetCellTextColour(0, 0, acWX_GREY_TEXT_COLOUR);
    _pEnvironmentVariablesGrid->SetReadOnly(0, 1, true);

    return rc1;
}


// ---------------------------------------------------------------------------
// Name:        gdEditEnvironmentVariablesDialog::setEnvironmentVariablesValues
// Description: Set the grid to contain the var's name and var's value,
//              according to the Debug Settings Dialog
// Return Val:  bool - Success / failure.
// Author:      Guy Ilany
// Date:        27/11/2007
// ---------------------------------------------------------------------------
bool gdEditEnvironmentVariablesDialog::setEnvironmentVariablesValues()
{
    bool retVal = true;
    gtList<osEnvironmentVariable>::const_iterator iter = _envVars.begin();
    gtList<osEnvironmentVariable>::const_iterator endIter = _envVars.end();

    while (iter != endIter)
    {
        retVal = retVal && addVarNameAndValue(*iter);
        iter++;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdEditEnvironmentVariablesDialog::addVarNameAndValue
// Description: Adds a coupling of var name and value to the _pEnvironmentVariablesGrid.
// Arguments:
//          varName - the var's name
//          varValue - it's corresponding value (as string)
// Return Val: bool  - Success / failure.
// Author:      Guy Ilany
// Date:        27/11/2007
// ---------------------------------------------------------------------------
bool gdEditEnvironmentVariablesDialog::addVarNameAndValue(osEnvironmentVariable var)
{
    bool rc = true;
    gtString varName = var._name;
    gtString varValue = var._value;
    int newRowIndex = _pEnvironmentVariablesGrid->GetNumberRows() - 1;
    _pEnvironmentVariablesGrid->InsertRows(newRowIndex);

    _pEnvironmentVariablesGrid->SetCellValue(newRowIndex, 0, varName.asCharArray());
    _pEnvironmentVariablesGrid->SetCellValue(newRowIndex, 1, varValue.asCharArray());
    _pEnvironmentVariablesGrid->SetRowMinimalHeight(newRowIndex, 10);


    GT_RETURN_WITH_ASSERT(rc);
}
// ---------------------------------------------------------------------------
// Name:        void gdEditEnvironmentVariablesDialog::onGridCellEditorHidden
// Description: Called when a cell's focus is out. We use this only to set the 'Add...' text on the last row.
// Author:      Guy Ilany
// Date:        2007/11/29
// ---------------------------------------------------------------------------
void gdEditEnvironmentVariablesDialog::onGridCellEditorHidden(wxGridEvent& event)
{
    int eveRow = event.GetRow();
    int eveCol = event.GetCol();
    int rows = _pEnvironmentVariablesGrid->GetNumberRows();

    if (!_isDuringCellInsertion)
    {
        // If we are editing the last environment variable NAME Column:
        if ((0 == eveCol) && ((rows - 1) == eveRow))
        {
            // Get the cell text editor
            wxGridCellEditor* pCellEditor = _pEnvironmentVariablesGrid->GetDefaultEditorForCell(eveRow, eveCol);

            if (pCellEditor != NULL)
            {
                // Get the cell value:
                const wxString& cellNewContent = pCellEditor->GetValue();

                // Check if the var name cell is empty
                if (cellNewContent.IsEmpty())
                {
                    // Fill the cell with the 'enter new var name here' text
                    _pEnvironmentVariablesGrid->SetCellValue(eveRow, 0, GD_STR_EditVariablesDialogAddEnvironmentVariable);
                    _pEnvironmentVariablesGrid->SetCellTextColour(eveRow, 0, acWX_GREY_TEXT_COLOUR);
                    _pEnvironmentVariablesGrid->SetReadOnly(eveRow, 1, true);
                }
                else if (cellNewContent != GD_STR_EditVariablesDialogAddEnvironmentVariable)
                {
                    // var name column is not empty:
                    _pEnvironmentVariablesGrid->SetCellTextColour(eveRow, eveCol, *wxBLACK);

                    // Make the current line variable name cell NOT read only
                    _pEnvironmentVariablesGrid->SetReadOnly(eveRow, 1, false);

                    // Insert new row at the end
                    _isDuringCellInsertion = true;

                    bool rc1 = _pEnvironmentVariablesGrid->InsertRows(rows, 1);
                    GT_IF_WITH_ASSERT(rc1)
                    {
                        _pEnvironmentVariablesGrid->SetCellValue(rows, 0, GD_STR_EditVariablesDialogAddEnvironmentVariable);
                        _pEnvironmentVariablesGrid->SetCellTextColour(rows, 0, acWX_GREY_TEXT_COLOUR);
                        // Make the inserted row variable name cell read only
                        _pEnvironmentVariablesGrid->SetReadOnly(rows, 1, true);
                    }
                    _isDuringCellInsertion = false;
                }

                // Bug 8285: We need to release the cell editor (there is an IncRef when we get it)
                pCellEditor->DecRef();
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        void gdEditEnvironmentVariablesDialog::onSelectCell
// Description: Is called when a cell is selected. Basically, the grid does all the job by default,
//              But we use this function to erase the 'Add...' text when the cell is chosen
// Author:      Guy Ilany
// Date:        2007/12/02
// ---------------------------------------------------------------------------
void gdEditEnvironmentVariablesDialog::onGridCellEditorShowen(wxGridEvent& event)
{
    int eveRow = event.GetRow();
    int eveCol = event.GetCol();
    int rows = _pEnvironmentVariablesGrid->GetNumberRows();

    // If we are editing the last environment variable NAME Column:
    if ((0 == eveCol) && ((rows - 1) == eveRow))
    {
        // Check if the var name cell is empty
        gtString cellText(_pEnvironmentVariablesGrid->GetCellValue(eveRow, eveCol).c_str());

        if ((cellText.compareNoCase(GD_STR_EditVariablesDialogAddEnvironmentVariable) == 0))
        {
            _pEnvironmentVariablesGrid->SetCellValue(AF_STR_Empty, eveRow, 0);
            _pEnvironmentVariablesGrid->SetCellTextColour(eveRow, eveCol, *wxBLACK);
        }
    }
}
