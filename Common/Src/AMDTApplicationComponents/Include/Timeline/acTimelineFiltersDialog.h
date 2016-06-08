//==================================================================================
// Copyright (c) 2011 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acTimelineFiltersDialog.h
///
//==================================================================================

//------------------------------ acTimelineFiltersDialog.h ------------------------------

#ifndef __ACTIMELINEFILTERSDIALOG_H
#define __ACTIMELINEFILTERSDIALOG_H


// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <QDialog>

#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

class acTimelineBranch;

class AC_API acTimelineFiltersDialog : public QDialog
{
    Q_OBJECT
public:
    /// constructor
    acTimelineFiltersDialog(QWidget* pParent = nullptr);

    void DisplayDialog(QList<acTimelineBranch*>& m_subBranches);
    bool IsAllItemsChecked()const { return m_isAllItemsChecked; }
    int GetCPUThreadCount()const { return m_CPUThreadCount; }
    int GetVisibleCPUThreadCount()const { return m_visibleCPUThreadCount; }
    QMap<QString, bool>& getThreadVisibilityMap() { return m_threadNameVisibilityMap; }

private slots:
    /// function called on Ok button clicked
    void OnClickOk();

    /// function called on cancel button clicked
    void OnClickCancel();

    /// function called on one of the tree items check state changed
    /// \params not used
    void OnTreeItemChanged(QTreeWidgetItem*, int);


private:
    /// initialize dialog window layouts
    void InitializeLayout();

    void BuildItemTree();

    void AddItemToTree(acTimelineBranch* pBranch, QTreeWidgetItem* pTopItem);

    /// updates tree items when one of the tree items check state was changed
    /// if the changed check state is of a top level item - updates all it's child items
    /// if the changed item is a child item - update it's parent item by going over the parents children list and deciding the new check state of the parent
    /// \param treeWidgetItem - the changed tree item
    void UpdateTreeWidgetItemCheckState(QTreeWidgetItem* treeWidgetItem);

    void UpdateCheckStatesFromTree();

    bool IsCPUTRootNode(acTimelineBranch* pBranch)const;

    void UpdateCheckStateFromItem(QTreeWidgetItem* pItem, acTimelineBranch* pBranch, bool isCPUItem, bool isRootItem);


    /// vertical layout UI items
    QVBoxLayout* m_pVLayout;

    /// threads tree
    QTreeWidget* m_pItemsTree;

    QList<acTimelineBranch*>* m_pSubBranches;

    bool m_isAllItemsChecked;
    int m_CPUThreadCount;
    int m_visibleCPUThreadCount;

    QMap<QString, bool> m_threadNameVisibilityMap;
};

#endif //__ACTIMELINEFILTERSDIALOG_H
