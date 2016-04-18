//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionSourceCodeView.h
///
//==================================================================================

#ifndef __SESSIONSOURCECODEVIEW_H
#define __SESSIONSOURCECODEVIEW_H

#include <list>
#include <assert.h>
#include <math.h>

// Qt:
#include <QtWidgets>
#include <QtCore>
#include <QItemDelegate>
#include <QLabel>
#include <QTextEdit>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTExecutableFormat/inc/ExecutableFile.h>

#include <inc/DataTab.h>
#include <inc/SourceViewTreeItem.h>
#include <inc/SourceCodeViewUtils.h>

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

class acTreeItemDeletate;
class SourceCodeTreeModel;
/// -----------------------------------------------------------------------------------------------
/// \class Name: SessionSourceCodeView : public DataTab
/// \brief Description: Is used to display source code for cpu profile sessions
/// -----------------------------------------------------------------------------------------------
class SessionSourceCodeView : public DataTab
{
    Q_OBJECT

public:

    SessionSourceCodeView(QWidget* pParent, CpuSessionWindow* pSessionWindow, const QString& sessionDir);
    virtual ~SessionSourceCodeView();

    bool DisplayModule(const CpuProfileModule* pModDetail);

    /// Add a source code item to the CodeXL explorer. This function is called when the source view is opened for
    /// a requested module
    void AddSourceCodeItemToExplorer();

    void DisplayAddress(gtVAddr addr, ProcessIdType pid, ThreadIdType tid);

    // Overrides DataTab:
    /// Updates the current tables display according to the needed update type:
    /// \param updateType the type of update needs to be performed (see SettingsDifference for details):
    virtual void UpdateTableDisplay(unsigned int updateType);

    virtual void applyUserDisplayInformation();


    void RefreshView();

public slots:
    void HighlightMatchingSourceLine(gtVAddr addr);
    void onViewChanged() {};
    virtual void onEditCopy();
    virtual void onEditSelectAll();
    virtual void onFindClick();
    virtual void onFindNext();

protected slots:

    void OnFunctionsComboChange(int fnIndex);
    void OnPIDComboChange(int index);
    void OnTIDComboChange(int index);
    void OnTreeItemDoubleClick(const QModelIndex& index);
    void OnTreeItemClick(const QModelIndex& index);
    void OnTreeVerticalScrollPositionChange(int value, bool isFromKeyboard = false);
    void OnItemExpanded(const QModelIndex& index);
    void OnItemCollapsed(const QModelIndex& index);
    void OnExpandAll();
    void OnCollapseAll();
    void OnShowCodeBytes();
    void OnShowNoteWindow();
    void OnShowAddress();
    void OnHotSpotComboChanged(const QString& text);
    void OnSelectAll();
    void OnItemSelectChanged(const QModelIndex& current, const QModelIndex& previous);

protected:

    // Overrides.
    virtual void keyPressEvent(QKeyEvent* pKeyEvent);

    bool CreateViewLayout();

    bool InitSourceCodeTreeView();

    void SetItemsDelegate();

    void ExtendTreeContextMenu();
    void CreateTopLayout();
    bool FillHotspotIndicatorCombo();
    bool GetActualSourceFile(const QString& targetFile, QString& tryFile);

private:
    // Helper function to setup GUI stuff
    void OnFetchInstrucionsRequest(SourceViewTreeItem* pSrcItem);
    void CreateFunctionsComboBox();
    void CreatePidTidComboBoxes();
    void CreateDisplayFilterLinkLabel();

    void CreateHotSpotIndicatorComboBox();

    bool IsAddressInCurrentFunction(gtVAddr addr);

    bool ReadPE();

    bool UpdateDisplay();

    void UpdateColumnWidths();

    void UpdatePercentDelegate();

    bool CreateModelData();

    void UpdateWithNewSymbol();

    // Code cache stuff
    bool CacheSourceLinesFromFile();

    QString FindSourceFile(QString fileName);

    /// Finds the requested function in the displayed module:
    /// \param functionIndex the function within the functions combo box
    /// \return true iff the function was found successfully
    bool FindRequestedFunctionInModule(int functionIndex);

    void HideFilteredColumns();

    /// Select the requested tree item in the tree. Select and ensure visible the matching item in the table
    void SetTreeSelection(SourceViewTreeItem* pItemToSelect);

protected:

    // GUI Elements:
    QWidget* m_pWidget;
    QVBoxLayout* m_pMainVBoxLayout;
    QLabel* m_pModuleLocationInfoLabel;

    QAction* m_pExpandAllAction;
    QAction* m_pCollapseAllAction;

    // Top toolbar actions:
    acWidgetAction* m_pFunctionsComboBoxAction;
    acWidgetAction* m_pPIDComboBoxAction;
    acWidgetAction* m_pTIDComboBoxAction;
    acWidgetAction* m_pHotSpotIndicatorComboBoxAction;
    acWidgetAction* m_pPIDLabelAction;
    acWidgetAction* m_pTIDLabelAction;

    QAction* m_pShowCodeBytesAction;
    QAction* m_pShowAddressAction;
    QAction* m_pShowNoteAction;

    SourceCodeTreeView* m_pSourceCodeTree;
    SourceCodeTreeModel* m_pTreeViewModel;

    // Delegate:
    acTreeItemDeletate* m_pTreeItemDelegate;

    // Selection:
    bool m_CLUNoteShown;
    bool m_ignoreVerticalScroll;
};


#endif //__SESSIONSOURCECODEVIEW_H

