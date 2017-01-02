//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ModuleFilterDialog.cpp
///
//==================================================================================

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>
#include <QFile>
#include <QDir>
#include <QMessageBox>

// Infra:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>

// Local:
#include <inc/ModuleFilterDialog.h>
#include <inc/CpuProjectHandler.h>
#include <inc/Auxil.h>

#define str_Modules "Modules"
#define str_SelectAllModules "Select all modules"
#define str_DisplaySystemDLLs "Display System modules in Modules Filter"
#define str_DialogCaption "CodeXL Modules Filter"
#define str_TableColumnHeadingModule "Modules"
#define str_TableColumnHeadingPath "Path"

#define CP_CPU_TABLE_ROW_HEIGHT 18

ModuleFilterDialog::ModuleFilterDialog(std::shared_ptr<cxlProfileDataReader> pProfDataRdr,
                                       std::shared_ptr<DisplayFilter> pDisplayFilter,
                                       TableDisplaySettings* pDisplaySettings,
                                       CPUSessionTreeItemData* pSessionData,
                                       QWidget* pParent): acDialog(pParent),
    m_pProfDataRdr(pProfDataRdr),
    m_pDisplayFilter(pDisplayFilter),
    m_pTableDisplaySettings(pDisplaySettings),
    m_pSessionData(pSessionData)
{
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    intializeLayout();
    intializeData();

    QObject::connect(m_pPbOk, SIGNAL(clicked()), this, SLOT(onClickOk()));
    QObject::connect(m_pPbCancel, SIGNAL(clicked()), this, SLOT(onClickCancel()));
    QObject::connect(m_pSelectAllModules, SIGNAL(stateChanged(int)), this, SLOT(onCheckSelectALL(int)));
    QObject::connect(m_pDisplaySystemModule, SIGNAL(stateChanged(int)), this, SLOT(onCheckSystemDLL(int)));
    resize(QSize(600, 350));

    setWindowTitle(str_DialogCaption);
    afLoadTitleBarIcon(this);
}

ModuleFilterDialog::~ModuleFilterDialog()
{
}

void ModuleFilterDialog::intializeLayout()
{
    m_pPbOk                 = new QPushButton("OK", this);
    m_pPbCancel             = new QPushButton("Cancel", this);
    m_pSelectAllModules     = new QCheckBox(str_SelectAllModules, this);
    m_pDisplaySystemModule = new QCheckBox(str_DisplaySystemDLLs, this);
    m_pProcessDescriptor    = new QLabel(str_Modules, this);

    QString exeName;
    QString PID;

    if (1 == m_pTableDisplaySettings->m_filterByPIDsList.size())
    {
        AMDTProcessId onePID = m_pTableDisplaySettings->m_filterByPIDsList.at(0);
        PID.setNum(onePID);

        gtVector<AMDTProfileProcessInfo> procInfo;
        bool ret = m_pProfDataRdr->GetProcessInfo(AMDT_PROFILE_ALL_PROCESSES, procInfo);
        GT_ASSERT(ret);

        for (const auto& process : procInfo)
        {
            if (process.m_pid == onePID)
            {
                exeName = acGTStringToQString(process.m_name);
                break;
            }
            else
            {
                exeName = "Process";
            }
        }
    }
    else
    {
        //bool isSystemWide = false;
        exeName = "System-wide Profiling";

        if ((m_pSessionData != nullptr) && (m_pSessionData->m_profileScope != PM_PROFILE_SCOPE_SINGLE_EXE))
        {
            // isSystemWide = true;
            exeName = "Multiple Processes";
        }

        if (m_pTableDisplaySettings->m_filterByPIDsList.isEmpty())
        {
            PID = "All";
        }
        else
        {
            PID.setNum(m_pTableDisplaySettings->m_filterByPIDsList.size());
        }

        PID.append(" Processes");
    }

    m_pProcessDescriptor->setText(QString("%1 of %2(%3):")
                                  .arg(str_Modules)
                                  .arg(exeName)
                                  .arg(PID));

    m_pModuleTree = new acListCtrl(this, CP_CPU_TABLE_ROW_HEIGHT);

    m_pModuleTree->verticalHeader()->setVisible(false);
    m_pModuleTree->horizontalHeader()->setStretchLastSection(true);
    m_pModuleTree->setShowGrid(false);
    m_pModuleTree->setContextMenuPolicy(Qt::NoContextMenu);
    QHBoxLayout* pButtonBox = new QHBoxLayout;
    QVBoxLayout* pFullLayout = new QVBoxLayout;

    pButtonBox->addStretch();
    pButtonBox->addWidget(m_pPbOk);
    pButtonBox->addSpacing(6);
    pButtonBox->addWidget(m_pPbCancel);

    pFullLayout->addWidget(m_pProcessDescriptor);
    pFullLayout->addWidget(m_pModuleTree);
    pFullLayout->addWidget(m_pDisplaySystemModule);
    pFullLayout->addWidget(m_pSelectAllModules);

    pFullLayout->addItem(pButtonBox);

    setLayout(pFullLayout);
}

void ModuleFilterDialog::intializeData()
{
    GT_IF_WITH_ASSERT(m_pTableDisplaySettings != nullptr)
    {
        // Set the "Select all modules" check state:
        if (0 == m_pTableDisplaySettings->m_filterByModulePathsList.size())
        {
            m_pSelectAllModules->setCheckState(Qt::Checked);
        }
        else if (m_pTableDisplaySettings->m_filterByModulePathsList.size() < m_pTableDisplaySettings->m_allModulesFullPathsList.size())
        {
            m_pSelectAllModules->setCheckState(Qt::PartiallyChecked);
        }
        else
        {
            m_pSelectAllModules->setCheckState(Qt::Unchecked);
        }

        bool sysModuleEn = m_pTableDisplaySettings->m_shouldDisplaySystemModuleInModulesDlg;
        m_pDisplaySystemModule->setChecked(sysModuleEn);
        m_pDisplaySystemModule->setEnabled(sysModuleEn);

        GT_IF_WITH_ASSERT(m_pTableDisplaySettings->m_allModulesFullPathsList.size() == m_pTableDisplaySettings->m_isModule32BitList.size())
        {
            m_pModuleTree->setColumnCount(2);
            m_pModuleTree->setRowCount(m_pTableDisplaySettings->m_allModulesFullPathsList.size());

            m_pModuleTree->setHorizontalHeaderItem(0, new QTableWidgetItem(str_TableColumnHeadingModule));
            m_pModuleTree->setHorizontalHeaderItem(1, new QTableWidgetItem(str_TableColumnHeadingPath));

            for (int i = 0; i < m_pTableDisplaySettings->m_allModulesFullPathsList.size(); ++i)
            {
                // Check if this module is included in the filtered by modules list:
                QString qFilePath = m_pTableDisplaySettings->m_allModulesFullPathsList.at(i);

                QFileInfo fInfo = QFileInfo(qFilePath);

                // Look for icon:
                osFilePath modulePath;
                modulePath.setFullPathFromString(acQStringToGTString(qFilePath));

                // Add a new table item for this module:
                QTableWidgetItem* pItemName = new QTableWidgetItem(*CPUProfileDataTable::moduleIcon(modulePath,
                                                                   m_pTableDisplaySettings->m_isModule32BitList.at(i)),
                                                                   fInfo.fileName());


                // Check if this module is a system module:
                m_pTableDisplaySettings->m_isSystemModuleList[i] = osIsSystemModule(modulePath.asString());

                pItemName->setFlags(pItemName->flags() ^ Qt::ItemIsEditable);
                pItemName->data(Qt::CheckStateRole);

                // Check if this file path is selected:
                bool isSelected = true;

                if (m_pTableDisplaySettings->m_filterByModulePathsList.size() > 0)
                {
                    isSelected = m_pTableDisplaySettings->m_filterByModulePathsList.contains(qFilePath);
                }

                if (isSelected)
                {
                    pItemName->setCheckState(Qt::Checked);
                }
                else
                {
                    pItemName->setCheckState(Qt::Unchecked);
                }

                m_pModuleTree->setItem(i, 0, pItemName);

                QString path = qFilePath.mid(0, qFilePath.length() - fInfo.fileName().length()) ;
                QTableWidgetItem* pItemPath = new QTableWidgetItem(path);
                pItemPath->setFlags(pItemPath->flags() ^ Qt::ItemIsEditable);
                m_pModuleTree->setItem(i, 1, pItemPath);
            }

            m_pModuleTree->resizeColumnsToContents();
            m_pModuleTree->setRowHeight(CP_CPU_TABLE_ROW_HEIGHT);
            m_pModuleTree->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
            m_pModuleTree->setSortingEnabled(true);
            onCheckSystemModule(m_pTableDisplaySettings->m_shouldDisplaySystemModuleInModulesDlg);
            QObject::connect(m_pModuleTree, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onClickModuleItem(QTableWidgetItem*)));
        }
    }
}


void ModuleFilterDialog::onCheckSystemModule(int state)
{
    int rowCount = m_pModuleTree->rowCount();

    for (int i = 0; i < rowCount; ++i)
    {
        if ((nullptr == m_pModuleTree) || (nullptr == m_pModuleTree->item(i, 1)))
        {
            return;
        }

        QString name = m_pModuleTree->item(i, 1)->text();
        name.append(m_pModuleTree->item(i, 0)->text());

        if (osIsSystemModule(acQStringToGTString(name)))
        {
            m_pModuleTree->setRowHidden(i, (state == Qt::Unchecked));
        }
    }

    //Update select all check box if needed
    int shownCount = 0;
    int selectedCount = 0;

    for (int i = 0; i < rowCount; ++i)
    {
        if (!m_pModuleTree->isRowHidden(i))
        {
            shownCount++;

            if (Qt::Checked == m_pModuleTree->item(i, 0)->checkState())
            {
                selectedCount++;
            }
        }
    }

    QObject::disconnect(m_pSelectAllModules, SIGNAL(stateChanged(int)), this, SLOT(onCheckSelectALL(int)));

    if (0 == selectedCount)
    {
        m_pSelectAllModules->setCheckState(Qt::Unchecked);
    }
    else if (selectedCount == shownCount)
    {
        m_pSelectAllModules->setCheckState(Qt::Checked);
    }
    else if (selectedCount < shownCount)
    {
        m_pSelectAllModules->setCheckState(Qt::PartiallyChecked);
    }

    QObject::connect(m_pSelectAllModules, SIGNAL(stateChanged(int)), this, SLOT(onCheckSelectALL(int)));
}

void ModuleFilterDialog::onCheckSelectALL(int state)
{
    int rowCount = m_pModuleTree->rowCount();
    QObject::disconnect(m_pModuleTree, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onClickModuleItem(QTableWidgetItem*)));

    for (int i = 0; i < rowCount; ++i)
    {
        if ((nullptr == m_pModuleTree) || (nullptr == m_pModuleTree->item(i, 1)))
        {
            break;
        }

        if (!m_pModuleTree->isRowHidden(i))
        {
            m_pModuleTree->item(i, 0)->setCheckState((state == Qt::Unchecked) ? Qt::Unchecked : Qt::Checked);
        }
    }

    QObject::connect(m_pModuleTree, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onClickModuleItem(QTableWidgetItem*)));
}

void ModuleFilterDialog::onClickModuleItem(QTableWidgetItem* item)
{
    if (nullptr != item)
    {
        Qt::CheckState currentCheckState = item->checkState();

        QObject::disconnect(m_pModuleTree, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onClickModuleItem(QTableWidgetItem*)));
        QObject::disconnect(m_pSelectAllModules, SIGNAL(stateChanged(int)), this, SLOT(onCheckSelectALL(int)));
        QModelIndexList list = m_pModuleTree->selectionModel()->selectedIndexes();

        foreach (QModelIndex index, list)
        {
            m_pModuleTree->item(index.row(), 0)->setCheckState((currentCheckState == Qt::Unchecked) ? Qt::Unchecked : Qt::Checked);
        }

        int shownCount = 0;
        int selectedCount = 0;

        for (int i = 0; i < m_pModuleTree->rowCount(); ++i)
        {
            if ((nullptr == m_pModuleTree) || (nullptr == m_pModuleTree->item(i, 1)))
            {
                break;
            }

            if (!m_pModuleTree->isRowHidden(i))
            {
                shownCount++;

                if (Qt::Checked == m_pModuleTree->item(i, 0)->checkState())
                {
                    selectedCount++;
                    QTableWidgetItem* itemWid = m_pModuleTree->item(i, 0);
                    std::string str = itemWid->text().toStdString();
                }
            }
        }

        if (0 == selectedCount)
        {
            m_pSelectAllModules->setCheckState(Qt::Unchecked);
        }
        else if (selectedCount == shownCount)
        {
            m_pSelectAllModules->setCheckState(Qt::Checked);
        }
        else if (selectedCount < shownCount)
        {
            m_pSelectAllModules->setCheckState(Qt::PartiallyChecked);
        }

        QObject::connect(m_pSelectAllModules, SIGNAL(stateChanged(int)), this, SLOT(onCheckSelectALL(int)));
        QObject::connect(m_pModuleTree, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onClickModuleItem(QTableWidgetItem*)));
    }

}

void ModuleFilterDialog::onClickOk()
{
    if (m_pDisplaySystemModule->isEnabled())
    {
        bool isSystemModuleChecked = m_pDisplaySystemModule->isChecked();
        m_pTableDisplaySettings->m_shouldDisplaySystemModuleInModulesDlg = isSystemModuleChecked;
        m_pDisplayFilter->SetIgnoreSystemModule(!isSystemModuleChecked);
    }

    m_pTableDisplaySettings->m_filterByModulePathsList.clear();
    int rowCount = m_pModuleTree->rowCount();

    for (int i = 0; i < rowCount; ++i)
    {
        if ((nullptr == m_pModuleTree) || (nullptr == m_pModuleTree->item(i, 1)))
        {
            break;
        }

        if (!m_pModuleTree->isRowHidden(i))
        {
            if (m_pModuleTree->item(i, 0)->checkState() == Qt::Checked)
            {
                QString moduleNameWithPath = m_pModuleTree->item(i, 1)->text();
                moduleNameWithPath += m_pModuleTree->item(i, 0)->text();
                m_pTableDisplaySettings->m_filterByModulePathsList << moduleNameWithPath;
            }
        }
    }

    if (0 == m_pTableDisplaySettings->m_filterByModulePathsList.size())
    {
        QMessageBox::information(this, "CodeXL Error", "At least one module must be selected.");
        int iRowChecked =  -1;

        for (int i = 0; i < rowCount; ++i)
        {
            if ((nullptr == m_pModuleTree) || (nullptr == m_pModuleTree->item(i, 1)))
            {
                break;
            }

            if (!m_pModuleTree->isRowHidden(i))
            {
                m_pModuleTree->item(i, 0)->setCheckState(Qt::Checked);
                iRowChecked = i;
                break;
            }
        }

        // Make sure this item is visible
        m_pModuleTree->scrollToItem(m_pModuleTree->item(iRowChecked, 0), QAbstractItemView::EnsureVisible);
        return;
    }

    accept();
}

void ModuleFilterDialog::onClickCancel()
{
    reject();
}

bool ModuleFilterDialog::isThisPidListed(gtUInt64 pid)
{
    if (0 == m_pTableDisplaySettings->m_filterByPIDsList.size())
    {
        return false;
    }
    else
    {
        return m_pTableDisplaySettings->m_filterByPIDsList.contains(pid);
    }
}



