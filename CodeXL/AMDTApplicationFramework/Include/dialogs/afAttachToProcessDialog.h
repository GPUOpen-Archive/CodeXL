//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afAttachToProcessDialog.h
///
//==================================================================================

#ifndef __AFATTACHTOPROCESSDIALOG_H
#define __AFATTACHTOPROCESSDIALOG_H

// Qt:
#include <QDialog>

// infra:
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>


class AF_API afAttachToProcessDialog : public QDialog
{
    Q_OBJECT

public:
    afAttachToProcessDialog(bool isSysWideProf);
    virtual ~afAttachToProcessDialog();

    osProcessId GetProcessID() const { return m_processId; }
    osRuntimePlatform GetProcessPlatform() const { return m_processPlatform; }

    /// returns true if the "system wide" checkbox is checked
    bool IsSystemWideProfiling();

protected slots:
    void onAttachButton();
    void onCancelButton();
    void onButtonClicked(int role);
    void onProcessSelected();
    void onDoubleClicked(QTableWidgetItem* pItem);
    void onShowAllChanged(int state);

private:
    void createDialogLayout();
    void fillProcessesList();

    /// sets the "system wide" checkbox
    /// \param set is the checkbox status to be set
    void SetSystemWideCheckBox(bool set);

    acListCtrl* m_pProcessesCtrl;
    QPushButton* m_pAttachButton;
    QCheckBox* m_pCheckShowAll;
    QCheckBox* m_pCheckSystemWide;
    osProcessId m_processId;
    osRuntimePlatform m_processPlatform;
};

#endif //__AFATTACHTOPROCESSDIALOG_H

