//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdEditEnvironmentVariablesDialog.h
///
//==================================================================================

//------------------------------ gdEditEnvironmentVariablesDialog.h ------------------------------

#ifndef __GDEDITENVIRONMENTVARIABLESDIALOG
#define __GDEDITENVIRONMENTVARIABLESDIALOG


// wxWindows:
#include "wx/grid.h"

// ----------------------------------------------------------------------------------
// Class Name:           gdEditEnvironmentVariablesDialog : public acDialog
// General Description: Displays the Environment Variables as a two-column list (using a grid),
//                      according to the string at the gdDebuggerSettingsDialog.
// Author:               Guy Ilany
// Creation Date:        27/11/2007
// ----------------------------------------------------------------------------------
class GD_API gdEditEnvironmentVariablesDialog : public acDialog
{

public:
    gdEditEnvironmentVariablesDialog(wxWindow* parent, gtString& settingsDialogEditString, gtList<osEnvironmentVariable>& envVars);
    virtual ~gdEditEnvironmentVariablesDialog();

private:
    DECLARE_EVENT_TABLE()

private:
    bool setGridLayout();
    bool setLayout();
    bool setEnvironmentVariablesValues();
    bool addVarNameAndValue(osEnvironmentVariable var);

    // Handling events
    void onGridCellEditorHidden(wxGridEvent& event);
    void onGridCellEditorShowen(wxGridEvent& event);

public:
    // Overrides wxDialog
    virtual int ShowModal();

private:
    wxStaticText* _pEditVarsDialogDescription;
    wxSizer* _pSizer;
    wxGrid* _pEnvironmentVariablesGrid;
    gtString& _dialogEditVarsString;
    gtList<osEnvironmentVariable>& _envVars;

    // Contains true iff we are during new cells inserting, done by this class (which triggers all kinds of events):
    bool _isDuringCellInsertion;
};

#endif  // __GDEDITENVIRONMENTVARIABLESDIALOG
