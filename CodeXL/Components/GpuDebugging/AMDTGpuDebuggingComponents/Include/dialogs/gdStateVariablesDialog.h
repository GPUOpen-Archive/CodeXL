//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStateVariablesDialog.h
///
//==================================================================================

//------------------------------ gdStateVariablesDialog.h ------------------------------

#ifndef __GDSTATEVARIABLESDIALOG
#define __GDSTATEVARIABLESDIALOG

// Forward declarations:
class acListCtrl;
class acQTextFilterCtrl;

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          GD_API gdStateVariablesDialog : public QDialog
// General Description: State Variable selection dialog
//
// Author:              Yoni Rabin
// Creation Date:       22/4/2012
// ----------------------------------------------------------------------------------
class GD_API gdStateVariablesDialog : public QDialog
{
    Q_OBJECT
public:
    gdStateVariablesDialog(QWidget* pParent);
    ~gdStateVariablesDialog();

    const gtVector<gtString>& getSelectedStateVariables();
    static unsigned int getValidStateVariableTypesMask();
    void setActivatedStateVar(const QString& activatedStateVar);

protected:
    bool setDialogMonitoredVariables();
    bool setDialogActiveVariables();
    bool addChosenListItem(int stateVarID, const QString& varName);
    void colorActiveInLeftListCtrl();
    void doLayout();


protected slots:
    void onOk();
    void onAdd();
    void onRemove();
    void onRemoveAll();
    void onChosenListKeyPressed(QKeyEvent* pEvent);
    void onFilteredTextChanged(const QString& filterText);
    void onLeftListSelectionChanged();
    void onRightListSelectionChanged();
    void onFilterFocused(bool hasFocus);

private:
    void selectActivatedStateVar();
    bool addListItem(acListCtrl* pList, const gtString& name, int Id);
    void setButtonStates(bool hasLeftChanged);
    int varNameToListIndex(const QString& varName);

private:
    QHBoxLayout* m_pMainHorizontalLayout;
    QVBoxLayout* m_pMainVerticalLayout;
    QVBoxLayout* m_pButtonsVerticalLayout;
    QVBoxLayout* m_pListVerticalLayout;
    QVBoxLayout* m_pChosenListVerticalLayout;
    QGroupBox* m_pMainGroupBox;

    QPushButton* m_pAddButton;
    QPushButton* m_pRemoveButton;
    QPushButton* m_pRemoveAllButton;
    QDialogButtonBox* m_pButtonBox;

    acListCtrl* m_pList;
    acListCtrl* m_pChosenList;

    QLabel* m_pDescription;
    QLabel* m_pListText;
    QLabel* m_pChosenListText;
    acQTextFilterCtrl* m_pStatesFilter;

    QString m_activatedStateVar;

    gtVector<gtString> m_selectedList;
    bool m_changingList;
};

#endif  // __GDSTATEVARIABLESDIALOG
