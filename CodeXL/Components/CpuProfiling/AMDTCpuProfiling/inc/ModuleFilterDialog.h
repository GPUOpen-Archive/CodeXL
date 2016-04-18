//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ModuleFilterDialog.h
///
//==================================================================================

#ifndef __MODULEFILTERDLG_H
#define __MODULEFILTERDLG_H

#include <QtCore>
#include <QtWidgets>
#include <QDialog>
#include <QStandardItemModel>
#include <QTreeView>
#include <QTableView>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QBoxLayout>
#include <QFileInfo>

#include <qheaderview.h>

#include <AMDTCpuProfilingRawData/inc/CpuProfileReader.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/Include/acDialog.h>
#include <inc/CPUProfileDataTable.h>

class CPUSessionTreeItemData;

class ModuleFilterDialog : public acDialog
{
    Q_OBJECT

public:

    ModuleFilterDialog(CpuProfileReader* pProfileReader,
                       TableDisplaySettings* pDisplaySettings,
                       CPUSessionTreeItemData* pSessionData,
                       QWidget* pParent = nullptr);
    virtual ~ModuleFilterDialog();

private:
    QPushButton* m_pPbOk;
    QPushButton* m_pPbCancel;
    QCheckBox* m_pSelectAllModules;
    QCheckBox* m_pDisplaySystemDLL;
    QLabel* m_pProcessDescriptor;
    acListCtrl* m_pModuleTree;
    CpuProfileReader* m_pProfileReader;
    TableDisplaySettings* m_pTableDisplaySettings;
    CPUSessionTreeItemData* m_pSessionData;

    void intializeLayout();
    void intializeData();
    bool isThisPidListed(gtUInt64 pid);
private slots:
    void onCheckSystemDLL(int state);
    void onCheckSelectALL(int state);
    void onClickModuleItem(QTableWidgetItem* item);
    void onClickOk();
    void onClickCancel();

};


#endif //__MODULEFILTERDLG_H
