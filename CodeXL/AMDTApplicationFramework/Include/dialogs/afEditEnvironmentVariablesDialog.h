//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afEditEnvironmentVariablesDialog.h
///
//==================================================================================

#ifndef __AFEDITENVIRONMENTVARIABLESDIALOG
#define __AFEDITENVIRONMENTVARIABLESDIALOG

// Qt:
#include <QDialog>

// Infra:
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTBaseTools/Include/gtList.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          AF_API afEditEnvironmentVariablesDialog : public QDialog
// General Description: Displays the Environment Variables as a two-column list (using a grid),
//
// Author:              Yoni Rabin
// Creation Date:       11/4/2012
// ----------------------------------------------------------------------------------
class AF_API afEditEnvironmentVariablesDialog : public QDialog
{
    Q_OBJECT

public:
    afEditEnvironmentVariablesDialog(QWidget* pParent, gtList<osEnvironmentVariable>& envVars);
    virtual ~afEditEnvironmentVariablesDialog();

    // Get the result string:
    const gtString& environmentVariablesAsString() const {return m_envVarsString;};
protected:

    void initLayout();
    void addEndRow();
    void setItemEditable(int row, int col, bool editable);
    void removeRow(int row);
    void addEditableRow(const gtString& name, const gtString& val);
    void centerOnScreen();
    bool rowExists(int row, bool searchAll = false);
protected slots:

    void accept();
    void onCellEntered(int row, int column);
    void onCellChanged(int row, int column);
    void onCurrentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

protected:
    QLabel* m_pEditVarsDialogDescription;
    acListCtrl* m_pEnvironmentVariablesGrid;

    // Semicolon delimited string containing the environment variables:
    gtString m_envVarsString;
    gtList<osEnvironmentVariable>& m_envVars;

    // True if and only if new cells are being inserted (which triggers several events):
    bool m_isDuringCellInsertion;

    // QT:
    QVBoxLayout* m_pLayout;

};

#endif  // __afEditEnvironmentVariablesDialog
