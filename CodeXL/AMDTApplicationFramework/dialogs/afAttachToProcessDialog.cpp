//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afAttachToProcessDialog.cpp
///
//==================================================================================


//Qt
#include <QtWidgets>
#include <QtCore>

// Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osUser.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTApplicationFramework/Include/dialogs/afAttachToProcessDialog.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>

static QString GetProcessTypeString(osProcessId processId, osRuntimePlatform& platform);

afAttachToProcessDialog::afAttachToProcessDialog(bool isSysWideProf) : QDialog(afMainAppWindow::instance()),
    m_processId((osProcessId) - 1),
    m_processPlatform(OS_UNKNOWN_PLATFORM)
{
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setWindowTitle(AF_STR_winAttachToProcessTitle);
    resize(480, 400);

    createDialogLayout();

    // set system wide check box
    SetSystemWideCheckBox(isSysWideProf);

    m_pProcessesCtrl->blockSignals(true);
    fillProcessesList();
    m_pProcessesCtrl->blockSignals(false);

    m_pProcessesCtrl->setContextMenuPolicy(Qt::NoContextMenu);
}

afAttachToProcessDialog::~afAttachToProcessDialog()
{
}

void afAttachToProcessDialog::createDialogLayout()
{
    m_pProcessesCtrl = new acListCtrl(this);

    QStringList listHeaders;
    listHeaders << "Process";
    listHeaders << "ID";
    listHeaders << "Type";
    listHeaders << "User Name";
    m_pProcessesCtrl->initHeaders(listHeaders, false);
    m_pProcessesCtrl->setColumnWidth(0, 180);
    m_pProcessesCtrl->setColumnWidth(1, 60);
    m_pProcessesCtrl->setColumnWidth(2, 80);

    m_pProcessesCtrl->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_pProcessesCtrl, SIGNAL(itemSelectionChanged()), this, SLOT(onProcessSelected()));
    connect(m_pProcessesCtrl, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(onDoubleClicked(QTableWidgetItem*)));

    // Create the dialog buttons:
    QDialogButtonBox* pButtonBox = new QDialogButtonBox();

    GT_IF_WITH_ASSERT(nullptr != pButtonBox)
    {
        m_pAttachButton = new QPushButton(AF_STR_Attach_Button);
        QPushButton* pCancelButton = new QPushButton(AF_STR_Cancel_Button);
        QPushButton* pRefreshButton = new QPushButton(AF_STR_Refresh_Button);

        pButtonBox->addButton(m_pAttachButton, QDialogButtonBox::AcceptRole);
        pButtonBox->addButton(pCancelButton, QDialogButtonBox::RejectRole);
        pButtonBox->addButton(pRefreshButton, QDialogButtonBox::ResetRole);

        QSignalMapper* pSignalMapper = new QSignalMapper(this);
        connect(pRefreshButton, SIGNAL(clicked()), pSignalMapper, SLOT(map()));
        pSignalMapper->setMapping(pRefreshButton, QDialogButtonBox::ResetRole);
        connect(pSignalMapper, SIGNAL(mapped(int)), this, SLOT(onButtonClicked(int)));
    }

    // Connect the buttons:
    bool rc = connect(pButtonBox, SIGNAL(accepted()), this, SLOT(onAttachButton()));
    GT_ASSERT(rc);
    rc = connect(pButtonBox, SIGNAL(rejected()), this, SLOT(onCancelButton()));
    GT_ASSERT(rc);

    m_pCheckShowAll = new QCheckBox("Show non-attachable processes");
    rc = connect(m_pCheckShowAll, SIGNAL(stateChanged(int)), this, SLOT(onShowAllChanged(int)));
    GT_ASSERT(rc);

    // set system wide profiling check box
    m_pCheckSystemWide = new QCheckBox("System-Wide profiling");

    // Create the main layout:
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addWidget(m_pProcessesCtrl);
    pMainLayout->addWidget(m_pCheckShowAll);
    pMainLayout->addWidget(m_pCheckSystemWide);
    pMainLayout->addWidget(pButtonBox);

    setLayout(pMainLayout);

    QSortFilterProxyModel* pProxyModel = new QSortFilterProxyModel(m_pProcessesCtrl);
    pProxyModel->setSourceModel(m_pProcessesCtrl->model());
    m_pProcessesCtrl->setSortingEnabled(true);
    m_pProcessesCtrl->horizontalHeader()->setSortIndicatorShown(false);
}

void afAttachToProcessDialog::onAttachButton()
{
    if (OS_JAVA_PLATFORM == m_processPlatform || OS_DOT_NET_PLATFORM == m_processPlatform)
    {
        int reply = acMessageBox::instance().warning(AF_STR_WarningA,
                                                     "Attaching to a Java"
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                                                     "/.NET"
#endif
                                                     " application produces only native profiling,"
                                                     " i.e. no references to source code and managed modules."
                                                     " Do you wish to continue?",
                                                     QMessageBox::Yes | QMessageBox::No);

        if (QMessageBox::Yes == reply)
        {
            done(Accepted);
        }
    }
    else
    {
        done(Accepted);
    }
}

void afAttachToProcessDialog::onCancelButton()
{
    done(Rejected);
}

void afAttachToProcessDialog::onButtonClicked(int role)
{
    if (QDialogButtonBox::ResetRole == role)
    {
        m_pProcessesCtrl->blockSignals(true);

        m_pProcessesCtrl->setSortingEnabled(false);
        m_pProcessesCtrl->clearList();
        fillProcessesList();
        m_pProcessesCtrl->setSortingEnabled(true);
        m_pProcessesCtrl->horizontalHeader()->setSortIndicatorShown(false);

        m_pProcessesCtrl->blockSignals(false);
    }
}

void afAttachToProcessDialog::onDoubleClicked(QTableWidgetItem* pItem)
{
    GT_IF_WITH_ASSERT(nullptr != pItem)
    {
        if (pItem->textColor() != Qt::gray)
        {
            onAttachButton();
        }
    }
}

void afAttachToProcessDialog::onShowAllChanged(int state)
{
    GT_UNREFERENCED_PARAMETER(state);

    onButtonClicked(QDialogButtonBox::ResetRole);
}

void afAttachToProcessDialog::onProcessSelected()
{
    QList<QTableWidgetItem*> items = m_pProcessesCtrl->selectedItems();

    if (!items.isEmpty())
    {
        QTableWidgetItem* pItem = items.first();
        bool enable = (pItem->textColor() != Qt::gray);
        m_pAttachButton->setEnabled(enable);

        if (enable)
        {
            QTableWidgetItem* pRowItem = m_pProcessesCtrl->item(pItem->row(), 1);
            QVariant dataInner = pRowItem->data(Qt::DisplayRole);
            m_processId = qvariant_cast<osProcessId>(dataInner);

            pRowItem = m_pProcessesCtrl->item(pItem->row(), 2);
            QString type = pRowItem->text();

            m_processPlatform = OS_NATIVE_PLATFORM;

            if (3 < type.count())
            {
                if ('J' == type.at(0))
                {
                    m_processPlatform = OS_JAVA_PLATFORM;
                }
                else if ('.' == type.at(0))
                {
                    m_processPlatform = OS_DOT_NET_PLATFORM;
                }
            }
        }
    }
}

void afAttachToProcessDialog::fillProcessesList()
{
    int row = 0;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    void* tokenHandle = nullptr;

    osSetPrivilege(tokenHandle, SE_DEBUG_NAME, true, false);
#endif

    const bool showAll = m_pCheckShowAll->isChecked();

    osProcessesEnumerator processEnum;

    if (processEnum.initialize())
    {
        QTableWidgetItem* pSelectedItem = nullptr;

        osProcessId processId;
        gtString executableName;

        while (processEnum.next(processId, &executableName))
        {
            if (((osProcessId)0) == processId)
            {
                continue;
            }

            bool isAttachable = osIsProcessAttachable(processId);

            QString processType;

            if (showAll || isAttachable)
            {
                osRuntimePlatform platform = OS_UNKNOWN_PLATFORM;
                processType = GetProcessTypeString(processId, platform);

                if (OS_JAVA_PLATFORM == platform || OS_DOT_NET_PLATFORM == platform)
                {
                    isAttachable = false;
                }
            }

            if (!(showAll || isAttachable))
            {
                continue;
            }

            gtString userName;
            osGetProcessUserName(processId, userName);

            QStringList rowTexts;

            rowTexts << acGTStringToQString(executableName);
            rowTexts << "";
            rowTexts << processType;
            rowTexts << acGTStringToQString(userName);

            if (m_pProcessesCtrl->addRow(rowTexts, static_cast<QPixmap*>(nullptr), Qt::AlignVCenter | Qt::AlignLeft))
            {
                QTableWidgetItem* pRowItem = m_pProcessesCtrl->item(row, 1);

                QVariant dataInner = qVariantFromValue(processId);
                pRowItem->setData(Qt::DisplayRole, dataInner);

                if (!isAttachable)
                {
                    m_pProcessesCtrl->item(row, 0)->setTextColor(Qt::gray);
                    pRowItem->setTextColor(Qt::gray);
                    m_pProcessesCtrl->item(row, 2)->setTextColor(Qt::gray);
                    m_pProcessesCtrl->item(row, 3)->setTextColor(Qt::gray);
                }

                row++;
            }
        }

        if (0 < m_pProcessesCtrl->rowCount())
        {
            m_pProcessesCtrl->sortItems(0);
            m_pProcessesCtrl->resizeColumnToContents(3);

            pSelectedItem = m_pProcessesCtrl->item(0, 0);

            if (nullptr != pSelectedItem)
            {
                m_pProcessesCtrl->setCurrentItem(pSelectedItem);
                onProcessSelected();
            }
        }
    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    if (nullptr != tokenHandle)
    {
        osSetPrivilege(tokenHandle, SE_DEBUG_NAME, false, true);
    }

#endif
}

void afAttachToProcessDialog::SetSystemWideCheckBox(bool set)
{
    GT_IF_WITH_ASSERT(m_pCheckSystemWide != nullptr)
    {
        // set system wide check box (of this dialog window)
        m_pCheckSystemWide->setChecked(set);
    }
}


bool afAttachToProcessDialog::IsSystemWideProfiling()
{
    bool retval = false;
    GT_IF_WITH_ASSERT(m_pCheckSystemWide != nullptr)
    {
        // return checked status of system wide check box (of this dialog window)
        retval = m_pCheckSystemWide->isChecked();
    }
    return retval;
}

static QString GetProcessTypeString(osProcessId processId, osRuntimePlatform& platform)
{
    QString strType;

    osModuleArchitecture arch;

    if (osGetProcessType(processId, arch, platform, false))
    {
        if (OS_JAVA_PLATFORM == platform)
        {
            strType = "Java, ";
        }
        else if (OS_DOT_NET_PLATFORM == platform)
        {
            strType = ".NET, ";
        }


        if (OS_X86_64_ARCHITECTURE == arch)
        {
            strType += "x64";
        }
        else
        {
            strType += "x86";
        }
    }

    return strType;
}

