//==================================================================================
// Copyright (c) 2011 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acTimelineFiltersDialog.cpp
///
//==================================================================================

//-------------------------- - acTimelineFiltersDialog.cpp------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <Include/Timeline/acTimelineFiltersDialog.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineBranch.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>


#define LABEL_HEIGHT 16
#define LAYOUT_MARGIN 5
#define BACKGROUND_COLOR "background-color: white;"
#define TREE_BORDER_STYLE_SHEET "QTreeWidget { border: 1px solid gray; border-radius: 2px; padding: 2px; " BACKGROUND_COLOR "}"
#define LABEL_STYLE_SHEET "QLabel { " BACKGROUND_COLOR " }"
#define DIALOG_STYLE_SHEET "QDialog { " BACKGROUND_COLOR " }"
#define WIDGET_STYLE_SHEET "QWidget { " BACKGROUND_COLOR " }"
#define CaptionStyleSheet "QLabel { background-color: rgb(236, 236, 236); }"

#define AC_TIMELINE_FILTERS_WIDTH 220
#define AC_TIMELINE_FILTERS_HEIGHT 200

acTimelineFiltersDialog::acTimelineFiltersDialog(QWidget* pParent) :
    QDialog(pParent),
    m_pVLayout(nullptr),
    m_pItemsTree(nullptr),
    m_isAllItemsChecked(false),
    m_CPUThreadCount(0),
    m_visibleCPUThreadCount(0)
{
    InitializeLayout();

    QDialog::setWindowModality(Qt::ApplicationModal);
    //afLoadTitleBarIcon(this);

    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    setWindowTitle(AC_STR_TimelineFilterDlgCaption);
    setStyleSheet(DIALOG_STYLE_SHEET);
}

void acTimelineFiltersDialog::InitializeLayout()
{
    // create main layout
    QVBoxLayout* m_pMainLayout = new QVBoxLayout();
    m_pMainLayout->setMargin(LAYOUT_MARGIN);

    // Create a scroll area
    QScrollArea* pScrollArea = new QScrollArea;
    pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QWidget* pScrollAreaWidget = new QWidget;
    pScrollArea->setWidget(pScrollAreaWidget);
    pScrollAreaWidget->setStyleSheet(WIDGET_STYLE_SHEET);

    QHBoxLayout* m_pDataLayout = new QHBoxLayout();
    pScrollAreaWidget->setLayout(m_pDataLayout);
    m_pMainLayout->addWidget(pScrollAreaWidget, 1, Qt::AlignTop);
    m_pMainLayout->addStretch();

    // create layout
    m_pVLayout = new QVBoxLayout();
    m_pDataLayout->addLayout(m_pVLayout, 1);

    QFont fnt;
    fnt.setBold(true);
    QLabel* pCaption = new QLabel;
    pCaption->setText("Threads");
    pCaption->setStyleSheet(CaptionStyleSheet);
    pCaption->setFixedHeight(LABEL_HEIGHT);
    pCaption->setFont(fnt);
    m_pVLayout->addWidget(pCaption, Qt::AlignTop);

    // create ok and cancel buttons
    QDialogButtonBox* pBox = new QDialogButtonBox();
    QPushButton* m_pPushButtonOK = new QPushButton(AC_STR_DefaultOKButton);
    QPushButton* m_pPushButtonCancel = new QPushButton(AC_STR_Cancel_Button);

    pBox->addButton(m_pPushButtonOK, QDialogButtonBox::AcceptRole);
    pBox->addButton(m_pPushButtonCancel, QDialogButtonBox::RejectRole);

    bool rc = connect(pBox, SIGNAL(accepted()), this, SLOT(OnClickOk()));
    GT_ASSERT(rc);
    rc = connect(pBox, SIGNAL(rejected()), this, SLOT(OnClickCancel()));
    GT_ASSERT(rc);

    QHBoxLayout* m_pButtonBox = new QHBoxLayout;

    m_pButtonBox->addStretch();
    m_pButtonBox->addWidget(m_pPushButtonOK);
    m_pButtonBox->addWidget(m_pPushButtonCancel);
    m_pMainLayout->addLayout(m_pButtonBox);

    setLayout(m_pMainLayout);
    setMaximumWidth(AC_TIMELINE_FILTERS_WIDTH);
    setMaximumHeight(AC_TIMELINE_FILTERS_HEIGHT);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
}

void acTimelineFiltersDialog::DisplayDialog(QList<acTimelineBranch*>& subBranches)
{
    m_pSubBranches = &subBranches;
    BuildItemTree();
}

void acTimelineFiltersDialog::OnClickOk()
{
    UpdateCheckStatesFromTree();
    accept();
}

void acTimelineFiltersDialog::OnClickCancel()
{
    reject();
}


void acTimelineFiltersDialog::BuildItemTree()
{
    // create threads tree
    if (m_pItemsTree != nullptr)
    {
        delete m_pItemsTree;
        m_pItemsTree = nullptr;
    }

    m_pItemsTree = new QTreeWidget();
    m_threadNameVisibilityMap.clear();

    GT_IF_WITH_ASSERT(m_pVLayout != nullptr)
    {
        m_pVLayout->addWidget(m_pItemsTree);

        m_pItemsTree->setColumnCount(1);
        m_pItemsTree->setHeaderHidden(true);
        m_pItemsTree->setStyleSheet(TREE_BORDER_STYLE_SHEET);

        for (QList<acTimelineBranch*>::const_iterator iter = m_pSubBranches->begin(); iter != m_pSubBranches->end(); ++iter)
        {
            acTimelineBranch* pBranch = (*iter);
            bool isCPUItem = IsCPUTRootNode(pBranch);

            if (isCPUItem == true)
            {
                QTreeWidgetItem* pTopItem = new QTreeWidgetItem();

                m_pItemsTree->addTopLevelItem(pTopItem);
                m_pItemsTree->expandItem(pTopItem);
                AddItemToTree(pBranch, pTopItem);

                m_pVLayout->addStretch();

                // connected to tree item checked state changed
                connect(m_pItemsTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(OnTreeItemChanged(QTreeWidgetItem*, int)));
            }
        }
    }
}

void acTimelineFiltersDialog::AddItemToTree(acTimelineBranch* pBranch, QTreeWidgetItem* pTreeItem)
{
    if (pBranch != nullptr && pTreeItem != nullptr)
    {
        pTreeItem->setFlags(pTreeItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        pTreeItem->setText(0, pBranch->text());


        if (IsCPUTRootNode(pBranch) == false)
        {
            m_threadNameVisibilityMap.insert(pBranch->text(), pBranch->IsVisible());
        }

        int visibleSubBranchesCount = 0;
        int subBranchesCount = pBranch->subBranchCount();

        for (int i = 0; i < subBranchesCount; i++)
        {
            acTimelineBranch* pSubBranch = pBranch->getSubBranch(i);
            QTreeWidgetItem* threadItem = new QTreeWidgetItem();
            AddItemToTree(pSubBranch, threadItem);
            pTreeItem->addChild(threadItem);

            if (pSubBranch->IsVisible() == true)
            {
                visibleSubBranchesCount++;
            }
        }

        // set visibility check considering children
        Qt::CheckState state = Qt::Unchecked;

        if (pBranch->IsVisible() == true)
        {
            if (visibleSubBranchesCount == subBranchesCount && pBranch->IsVisible() == true)
            {
                state = Qt::Checked;
            }
            else
            {
                state = Qt::PartiallyChecked;
            }
        }

        pTreeItem->setCheckState(0, state);
    }
}

void acTimelineFiltersDialog::OnTreeItemChanged(QTreeWidgetItem* pItem, int index)
{
    GT_UNREFERENCED_PARAMETER(index);
    GT_UNREFERENCED_PARAMETER(pItem);

    GT_IF_WITH_ASSERT(m_pItemsTree != nullptr)
    {
        // when one of the tree items check state changed - update tree items check state accordingly
        // disconnect events to update the tree and connect at the end
        disconnect(m_pItemsTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(OnTreeItemChanged(QTreeWidgetItem*, int)));
        UpdateTreeWidgetItemCheckState(pItem);
        connect(m_pItemsTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(OnTreeItemChanged(QTreeWidgetItem*, int)));
    }
}

void acTimelineFiltersDialog::UpdateTreeWidgetItemCheckState(QTreeWidgetItem* treeWidgetItem)
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

            QString str = QString(AC_STR_TimelineFilterCPUThreads).arg(checkCount).arg(childrenCount);
            parentItem->setText(0, str);
        }
    }
}

void acTimelineFiltersDialog::UpdateCheckStatesFromTree()
{
    GT_IF_WITH_ASSERT(m_pItemsTree != nullptr)
    {
        m_CPUThreadCount = m_visibleCPUThreadCount = 0;
        m_isAllItemsChecked = true;

        int i = 0;

        for (QList<acTimelineBranch*>::iterator iter = m_pSubBranches->begin(); iter != m_pSubBranches->end(); ++iter)
        {
            acTimelineBranch* pBranch = (*iter);
            bool isCPUItem = IsCPUTRootNode(pBranch);

            // get top level item
            QTreeWidgetItem* pTopItem = m_pItemsTree->topLevelItem(i);

            UpdateCheckStateFromItem(pTopItem, pBranch, isCPUItem, true);
            i++;
        }
    }
}

bool acTimelineFiltersDialog::IsCPUTRootNode(acTimelineBranch* pBranch)const
{
    GT_IF_WITH_ASSERT(pBranch != nullptr)
    {
        return pBranch->text().contains("CPU");
    }
    return false;
}

void acTimelineFiltersDialog::UpdateCheckStateFromItem(QTreeWidgetItem* pItem, acTimelineBranch* pBranch, bool isCPUItem, bool isRootItem)
{
    if (pItem != nullptr && pBranch != nullptr)
    {
        Qt::CheckState state = pItem->checkState(0);
        m_isAllItemsChecked &= (state != Qt::Unchecked);

        if (isRootItem == false)
        {
            if (isCPUItem == true)
            {
                m_CPUThreadCount++;

                if (state != Qt::Unchecked)
                {
                    m_visibleCPUThreadCount++;
                }
            }

            pBranch->setVisibility(state != Qt::Unchecked);
            m_threadNameVisibilityMap[pBranch->text()] = state != Qt::Unchecked;

        }

        int subBranchesCount = pBranch->subBranchCount();

        for (int j = 0; j < subBranchesCount; j++)
        {
            QTreeWidgetItem* pSubItem = pItem->child(j);
            acTimelineBranch* pSubBranch = pBranch->getSubBranch(j);
            UpdateCheckStateFromItem(pSubItem, pSubBranch, isCPUItem, false);
        }
    }
}

