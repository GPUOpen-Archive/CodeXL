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

    ModuleFilterDialog(shared_ptr<cxlProfileDataReader> m_pProfDataRdr,
                        TableDisplaySettings* pDisplaySettings,
                        CPUSessionTreeItemData* pSessionData,
                        bool isDisplaySysModEn,
                        QWidget* pParent = nullptr);
    virtual ~ModuleFilterDialog();

private:
    QPushButton* m_pPbOk = nullptr;
    QPushButton* m_pPbCancel = nullptr;
    QCheckBox* m_pSelectAllModules = nullptr;
    QCheckBox* m_pDisplaySystemDLL = nullptr;
    QLabel* m_pProcessDescriptor = nullptr;
    acListCtrl* m_pModuleTree = nullptr;

    shared_ptr<cxlProfileDataReader> m_pProfDataRdr = nullptr;
    TableDisplaySettings* m_pTableDisplaySettings = nullptr;
    CPUSessionTreeItemData* m_pSessionData = nullptr;
    bool m_isDisplaySysModEn = false;

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
