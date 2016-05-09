//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpDisplaySettingsDialog.cpp
///
//==================================================================================

//-------------------------- - tpDisplaySettingsDialog.cpp------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// Local:
#include <inc/tpDisplaySettingsDialog.h>
#include <inc/StringConstants.h>


#define DIALOG_WIDTH 220
#define DIALOG_HEIGHT 340
#define LABEL_HEIGHT 16
#define LAYOUT_MARGIN 5
#define TREE_BORDER_STYLE_SHEET "QTreeWidget { border: none; background-color: transparent;}"

tpDisplaySettingsDialog::tpDisplaySettingsDialog(QWidget* pParent, tpSessionData* pSessionData) :
    QDialog(pParent),
    m_pCoresLayout(nullptr),
    m_pProcessesLayout(nullptr),
    m_pProcessTree(nullptr),
    m_pSessionData(pSessionData)
{
    initializeLayout();

    QDialog::setWindowModality(Qt::ApplicationModal);
    afLoadTitleBarIcon(this);

    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    setWindowTitle(CP_STR_SettingsWindowTitle);


    SetCoresColumn();
    SetProcessesTree();


}

void tpDisplaySettingsDialog::initializeLayout()
{
    // create main layout
    QVBoxLayout* m_pMainLayout = new QVBoxLayout();
    m_pMainLayout->setMargin(LAYOUT_MARGIN);
    // m_pMainLayout->addStretch();

    // Create a scroll area
    QScrollArea* pScrollArea = new QScrollArea;
    pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QWidget* pScrollAreaWidget = new QWidget;
    pScrollArea->setWidget(pScrollAreaWidget);

    QHBoxLayout* m_pDataLayout = new QHBoxLayout();
    pScrollAreaWidget->setLayout(m_pDataLayout);
    m_pMainLayout->addWidget(pScrollAreaWidget, 1, Qt::AlignTop);
    m_pMainLayout->addStretch();

    // create cores layout and processes layout
    m_pCoresLayout = new QVBoxLayout();
    m_pProcessesLayout = new QVBoxLayout();
    m_pDataLayout->addLayout(m_pCoresLayout, 1);
    m_pDataLayout->addLayout(m_pProcessesLayout, 1);

    // add "cores" label
    QLabel* coresLabel = new QLabel;
    coresLabel->setTextFormat(Qt::RichText);
    coresLabel->setText(QString(CP_STR_SettingsTitle).arg(CP_STR_SettingsTitleCores));
    QFont font;
    font.setUnderline(true);
    coresLabel->setFont(font);
    coresLabel->setStyleSheet(CP_STR_SettingsLAbelStyleSheet);
    coresLabel->setFixedHeight(LABEL_HEIGHT);
    m_pCoresLayout->addWidget(coresLabel, Qt::AlignTop);

    // add "processes and threads" label
    QLabel* procslabel = new QLabel;
    procslabel->setTextFormat(Qt::RichText);
    procslabel->setText(QString(CP_STR_SettingsTitle).arg(CP_STR_SettingsTitleProcss));
    procslabel->setStyleSheet(CP_STR_SettingsLAbelStyleSheet);
    procslabel->setFont(font);
    procslabel->setFixedHeight(LABEL_HEIGHT);
    m_pProcessesLayout->addWidget(procslabel, Qt::AlignTop);

    // crate ok and cancel buttons
    QPushButton* m_pPushButtonOK = new QPushButton(CP_STR_SettingsOkButtonLabel);
    QPushButton* m_pPushButtonCancel = new QPushButton(CP_STR_SettingsCancelButtonLabel);

    bool rc = connect(m_pPushButtonOK, SIGNAL(clicked()), this, SLOT(OnClickOk()));
    GT_ASSERT(rc);
    rc = connect(m_pPushButtonCancel, SIGNAL(clicked()), this, SLOT(OnClickCancel()));
    GT_ASSERT(rc);

    QHBoxLayout* m_pButtonBox = new QHBoxLayout;

    m_pButtonBox->addStretch();
    m_pButtonBox->addWidget(m_pPushButtonOK);
    m_pButtonBox->addWidget(m_pPushButtonCancel);
    m_pMainLayout->addLayout(m_pButtonBox);

    setLayout(m_pMainLayout);
    setFixedSize(DIALOG_WIDTH, DIALOG_HEIGHT);
}

void tpDisplaySettingsDialog::OnClickOk()
{
    // copy the last saved tree state before cancel to current tree
    CopyCheckStatesFromTree(m_pProcessTree, m_lastProcessesTree);

    // copy last state of cores selection before cancel
    UpdateLastCoresSelection();

    accept();
}

void tpDisplaySettingsDialog::OnClickCancel()
{
    // copy processes tree from last tree
    CopyCheckStatesFromTree(m_lastProcessesTree, m_pProcessTree);

    // copy from last cores state list
    UpdateFromLastCoresSelection();

    reject();
}

void tpDisplaySettingsDialog::GetTpSettingsData(QString& selectedCoresStr, int& selectedProcsNum, int& selectedThreadsNum)
{
    SelectedCoresString(selectedCoresStr);

    selectedProcsNum = SelectedProcessesCount();
    selectedThreadsNum = SelectedThreadsCount();
}


void tpDisplaySettingsDialog::SetCoresColumn()
{
    GT_IF_WITH_ASSERT(m_pCoresLayout != nullptr)
    {
        QCheckBox* pcheckBox = new QCheckBox(CP_STR_SettingsAllCoresCbText);
        m_pCoresLayout->addWidget(pcheckBox);
        m_coresCBsList.append(pcheckBox);
        bool rc = connect(pcheckBox, SIGNAL(clicked(bool)), this, SLOT(AllCBCheckStateChanged(bool)));
        GT_ASSERT(rc);

        // get cores names and add to list and layouts check-boxes
        QStringList coresNames = m_pSessionData->CoresList();

        // for each core in cores list
        foreach (QString name, coresNames)
        {
            // create check-box + add to list + add to layout
            QCheckBox* pCoreCheckBox = new QCheckBox(name);
            m_pCoresLayout->addWidget(pCoreCheckBox);
            m_coresCBsList.append(pCoreCheckBox);

            // connect CoreCheckStateChanged to checked event
            rc = connect(pCoreCheckBox, SIGNAL(clicked(bool)), this, SLOT(CoreCheckStateChanged(bool)));
            GT_ASSERT(rc);
        }

        foreach (QCheckBox* cb, m_coresCBsList)
        {
            m_lastCoresStatesList.append(cb->checkState());
        }

        m_pCoresLayout->addStretch();
    }
}

void tpDisplaySettingsDialog::SetProcessesTree()
{
    // get processes and threads map
    QMap<AMDTProcessId, QVector<AMDTThreadId> >& procsAndThreadsMap = m_pSessionData->ProcessesAndThreadsMap();

    // create procs/threads tree
    m_pProcessTree = new QTreeWidget();

    GT_IF_WITH_ASSERT(m_pProcessesLayout != nullptr)
    {
        m_pProcessesLayout->addWidget(m_pProcessTree);

        m_pProcessTree->setColumnCount(1);
        m_pProcessTree->setHeaderHidden(true);
        //m_pProcessTree->setItemDelegate(new acItemDelegate);
        m_pProcessTree->setStyleSheet(TREE_BORDER_STYLE_SHEET);

        QTreeWidgetItem* processItem;
        QTreeWidgetItem* threadItem;

        QMap<AMDTProcessId, QVector<AMDTThreadId> >::iterator it = procsAndThreadsMap.begin();

        for (; it != procsAndThreadsMap.end(); it++)
        {
            // for each process - add top level item to tree
            AMDTProcessId processId = it.key();
            processItem = new QTreeWidgetItem();
            processItem->setFlags(processItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            QString processName = m_pSessionData->GetProcessNameById(processId);
            processName.append(QString("(%1)").arg(processId));
            processItem->setText(0, processName);
            processItem->setCheckState(0, Qt::Unchecked);
            m_pProcessTree->addTopLevelItem(processItem);
            m_pProcessTree->expandItem(processItem);

            foreach (AMDTThreadId threadId, procsAndThreadsMap[processId])
            {
                // for each thread - add child item to tree
                threadItem = new QTreeWidgetItem();
                threadItem->setFlags(threadItem->flags() | Qt::ItemIsUserCheckable);
                QString threadName = m_pSessionData->GetThreadNameById(threadId);
                threadName.append(QString("(%1)").arg(threadId));
                threadItem->setText(0, threadName);
                threadItem->setCheckState(0, Qt::Unchecked);
                processItem->addChild(threadItem);
            }
        }

        m_pProcessesLayout->addStretch();
        //Note: tree have 2 levels - processes(top level) and threads(first child level)

        // connected to tree item checked state changed
        connect(m_pProcessTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(ProcessesTreeItemChanged(QTreeWidgetItem*, int)));

        CreateCopiedTree();
    }
}

void tpDisplaySettingsDialog::UpdateTreeWidgetItemCheckState(QTreeWidgetItem* treeWidgetItem)
{
    if (treeWidgetItem != nullptr)
    {
        Qt::CheckState checkState = treeWidgetItem->checkState(0);

        // if parent - set all children to have the same state as this node
        for (int i = 0; i < treeWidgetItem->childCount(); i++)
        {
            treeWidgetItem->child(i)->setCheckState(0, checkState);
        }

        QTreeWidgetItem* parentItem = treeWidgetItem->parent();

        // check the what should be the new parent state
        if (parentItem != nullptr)
        {
            int childrenCount = parentItem->childCount();
            int checkCount = 0;
            int uncheckCount = 0;

            for (int i = 0; i < childrenCount; i++)
            {
                checkCount += (parentItem->child(i)->checkState(0) == Qt::Checked);
                uncheckCount += (parentItem->child(i)->checkState(0) == Qt::Unchecked);
            }

            if (checkCount == childrenCount)
            {
                parentItem->setCheckState(0, Qt::Checked);
            }
            else if (uncheckCount == childrenCount)
            {
                parentItem->setCheckState(0, Qt::Unchecked);
            }
            else
            {
                parentItem->setCheckState(0, Qt::PartiallyChecked);
            }
        }
    }
}

void tpDisplaySettingsDialog::ProcessesTreeItemChanged(QTreeWidgetItem* pItem, int index)
{
    GT_UNREFERENCED_PARAMETER(index);
    GT_UNREFERENCED_PARAMETER(pItem);

    GT_IF_WITH_ASSERT(m_pProcessTree != nullptr)
    {
        // when one of the tree items check state changed - update tree items check state accordingly
        // disconnect events to update the tree and connect at the end
        disconnect(m_pProcessTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(ProcessesTreeItemChanged(QTreeWidgetItem*, int)));
        UpdateTreeWidgetItemCheckState(pItem);
        connect(m_pProcessTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(ProcessesTreeItemChanged(QTreeWidgetItem*, int)));
    }
}

int tpDisplaySettingsDialog::SelectedCoresCount()
{
    int retVal = 0;

    int count = m_coresCBsList.count();

    // if all selected - get number of check-boxes and reduce "all"
    if (m_coresCBsList[0]->checkState() == Qt::Checked)
    {
        retVal = count - 1;
    }
    else
    {
        // don't count "aLL"
        for (int i = 1; i < count; i++)
        {
            if (m_coresCBsList[i]->checkState() == Qt::Checked)
            {
                retVal++;
            }
        }
    }

    return retVal;
}

void tpDisplaySettingsDialog::SelectedCoresString(QString& str)
{
    int count = m_coresCBsList.count();

    // if all selected - get number of check-boxes and reduce "all"
    if (m_coresCBsList[0]->checkState() == Qt::Checked)
    {
        str = CP_STR_SettingsAllCoresStr;
    }
    else
    {
        // don't count "aLL"
        for (int i = 1; i < count; i++)
        {
            if (m_coresCBsList[i]->checkState() == Qt::Checked)
            {
                if (i > 1)
                {
                    str.append(", ");
                }

                str.append(m_coresCBsList[i]->text());
            }
        }
    }

    if (str.isEmpty())
    {
        str = CP_STR_SettingsNoCoresStr;
    }
}

int tpDisplaySettingsDialog::SelectedProcessesCount()
{
    int retVal = 0;

    GT_IF_WITH_ASSERT(m_pProcessTree != nullptr)
    {
        int count = m_pProcessTree->topLevelItemCount();

        // count only selected top level items
        for (int i = 0; i < count; i++)
        {
            if (m_pProcessTree->topLevelItem(i)->checkState(0) != Qt::Unchecked)
            {
                retVal++;
            }
        }
    }

    return retVal;
}

int tpDisplaySettingsDialog::SelectedThreadsCount()
{
    int retVal = 0;

    GT_IF_WITH_ASSERT(m_pProcessTree != nullptr)
    {
        int count = m_pProcessTree->topLevelItemCount();

        for (int i = 0; i < count; i++)
        {
            QTreeWidgetItem* parentItem = m_pProcessTree->topLevelItem(i);
            GT_IF_WITH_ASSERT(parentItem != nullptr)
            {
                if (parentItem->checkState(0) != Qt::Unchecked)
                {
                    int children = parentItem->childCount();

                    // get children
                    for (int j = 0; j < children; j++)
                    {
                        // count only selected child level items
                        if (parentItem->child(j)->checkState(0) == Qt::Checked)
                        {
                            retVal++;
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

void tpDisplaySettingsDialog::SelectedProcessesAndThreadsMap(QMap<AMDTProcessId, QVector<AMDTThreadId> >& procsThreadsMap)
{
    GT_IF_WITH_ASSERT(m_pProcessTree != nullptr)
    {
        procsThreadsMap.clear();

        int procsCount = m_pProcessTree->topLevelItemCount();

        // count only selected top level items
        for (int i = 0; i < procsCount; i++)
        {
            QTreeWidgetItem* topItem = m_pProcessTree->topLevelItem(i);

            if (topItem->checkState(0) != Qt::Unchecked)
            {
                // get process id by name
                QString processName = topItem->text(0);
                AMDTProcessId procID = GetIdFromTreeItem(processName);
                QVector<AMDTThreadId> threadsList;

                int threadsCount = topItem->childCount();

                for (int j = 0; j < threadsCount; j++)
                {
                    // get thread id by name and add to list
                    QTreeWidgetItem* childItem = topItem->child(j);

                    if (childItem->checkState(0) == Qt::Checked)
                    {
                        QString threadName = childItem->text(0);
                        AMDTThreadId ThreadId = GetIdFromTreeItem(threadName);
                        threadsList.append(ThreadId);
                    }
                }

                // add selected process and its threads to map
                if (threadsList.count() > 0)
                {
                    procsThreadsMap[procID] = threadsList;
                }
            }
        }
    }
}

void tpDisplaySettingsDialog::SelectedCoresList(QVector<QString>& coresList)
{
    foreach (QCheckBox* cb, m_coresCBsList)
    {
        QString threadName = cb->text();
        coresList.append(threadName);
    }
}

AMDTProcessId tpDisplaySettingsDialog::GetIdFromTreeItem(const QString& name)
{
    AMDTProcessId retVal = 0;

    QStringList list = name.split('(');

    if (list.count() == 2)
    {
        QString idStr = list[1].remove(")");
        retVal = idStr.toUInt();
    }

    return retVal;
}

void tpDisplaySettingsDialog::CoreCheckStateChanged(bool checked)
{
    GT_UNREFERENCED_PARAMETER(checked);

    // check the check state of all Cores and update the all check-box
    int count = m_coresCBsList.count();
    int numChecked = 0;

    // skip "all" check-box
    for (int i = 1; i < count; i++)
    {
        if (m_coresCBsList[i]->checkState() == Qt::Checked)
        {
            numChecked++;
        }
    }

    // if all cores selected - update th "All" check-box to checked
    if (numChecked == count - 1)
    {
        m_coresCBsList[0]->setCheckState(Qt::Checked);
    }
    else
    {
        m_coresCBsList[0]->setCheckState(Qt::Unchecked);
    }


}

void tpDisplaySettingsDialog::AllCBCheckStateChanged(bool checked)
{
    Qt::CheckState checkSatte = checked ? Qt::Checked : Qt::Unchecked;

    // if "All" check-box was checked/unchecked update all the other cores check-boxes accordingly
    for (int i = 1; i < m_coresCBsList.count(); i++)
    {
        m_coresCBsList[i]->setCheckState(checkSatte);
    }
}

void tpDisplaySettingsDialog::CreateCopiedTree()
{
    GT_IF_WITH_ASSERT(m_pProcessTree != nullptr)
    {
        m_lastProcessesTree = new QTreeWidget();
        int topLevelCount = m_pProcessTree->topLevelItemCount();

        QTreeWidgetItem* origTopItem = nullptr;
        QTreeWidgetItem* newTopItem = nullptr;
        QTreeWidgetItem* childItem = nullptr;

        // go over all top level items in tree
        for (int i = 0; i < topLevelCount; i++)
        {
            // for each top level item create new tree item and copy it's text
            origTopItem = m_pProcessTree->topLevelItem(i);
            newTopItem = new QTreeWidgetItem();
            newTopItem->setText(0, origTopItem->text(0));

            m_lastProcessesTree->addTopLevelItem(newTopItem);
            int childrenCount = origTopItem->childCount();

            // for each child level item create new tree item and copy it's text
            for (int j = 0; j < childrenCount; j++)
            {
                childItem = new QTreeWidgetItem();
                childItem->setText(0, origTopItem->child(j)->text(0));
                newTopItem->addChild(childItem);
            }
        }
    }
}

void tpDisplaySettingsDialog::CopyCheckStatesFromTree(QTreeWidget* srcTree, QTreeWidget* dstTree)
{
    GT_IF_WITH_ASSERT(srcTree != nullptr && dstTree != nullptr)
    {
        int srcTreeTopItemsCount = srcTree->topLevelItemCount();
        int dstTreeTopItemsCount = dstTree->topLevelItemCount();

        // sanity check
        GT_IF_WITH_ASSERT(srcTreeTopItemsCount == dstTreeTopItemsCount)
        {
            QTreeWidgetItem* srcTopItem = nullptr;
            QTreeWidgetItem* dstTopItem = nullptr;

            for (int i = 0; i < srcTreeTopItemsCount; i++)
            {
                // get parallel top level items in both trees
                srcTopItem = srcTree->topLevelItem(i);
                dstTopItem = dstTree->topLevelItem(i);

                GT_IF_WITH_ASSERT(srcTopItem != nullptr && dstTopItem != nullptr)
                {
                    // copy source item state to destination item
                    dstTopItem->setCheckState(0, srcTopItem->checkState(0));
                    int childrenCount = srcTopItem->childCount();

                    for (int j = 0; j < childrenCount; j++)
                    {
                        // get parallel child level items in both trees
                        Qt::CheckState state = srcTopItem->child(j)->checkState(0);

                        GT_IF_WITH_ASSERT(dstTopItem->child(j) != nullptr)
                        {
                            // copy source item state to destination item
                            dstTopItem->child(j)->setCheckState(0, state);
                        }
                    }
                }
            }
        }
    }
}

void tpDisplaySettingsDialog::UpdateLastCoresSelection()
{
    int count = m_coresCBsList.count();
    GT_IF_WITH_ASSERT(count == m_lastCoresStatesList.count())
    {
        // for each item in cores list copy it's state to backup list
        for (int i = 0; i < count; i++)
        {
            m_lastCoresStatesList[i] = (m_coresCBsList[i]->checkState());
        }
    }
}

void tpDisplaySettingsDialog::UpdateFromLastCoresSelection()
{
    int count = m_coresCBsList.count();
    GT_IF_WITH_ASSERT(count == m_lastCoresStatesList.count())
    {
        // for each item in cores list copy it's state from backup list
        for (int i = 0; i < count; i++)
        {
            m_coresCBsList[i]->setCheckState(m_lastCoresStatesList[i]);
        }
    }
}