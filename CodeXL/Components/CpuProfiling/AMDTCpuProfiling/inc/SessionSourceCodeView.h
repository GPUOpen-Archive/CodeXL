//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionSourceCodeView.h
///
//==================================================================================

#ifndef __SESSIONSOURCECODEVIEW_H
#define __SESSIONSOURCECODEVIEW_H

// Qt:
#include <QtWidgets>
#include <QtCore>
#include <QItemDelegate>
#include <QLabel>
#include <QTextEdit>

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

    //bool DisplayModule(const CpuProfileModule* pModDetail);
    bool DisplayViewModule(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32> funcModInfo);

    /// Add a source code item to the CodeXL explorer. This function is called when the source view is opened for
    /// a requested module
    void AddSourceCodeItemToExplorer();

    void DisplayAddress(gtVAddr addr, AMDTProcessId pid, AMDTThreadId tid);

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
    void CreateFunctionsComboBox();
    void CreatePidTidComboBoxes();
    void CreateDisplayFilterLinkLabel();
    void CreateHotSpotIndicatorComboBox();
    bool UpdateDisplay();
    void UpdateColumnWidths();
    void UpdatePercentDelegate();
    bool CreateModelData();
    void UpdateWithNewSymbol();

    // Code cache stuff
    bool CacheSourceLinesFromFile();

    QString FindSourceFile(QString fileName);
    void HideFilteredColumns();

    /// Select the requested tree item in the tree. Select and ensure visible the matching item in the table
    void SetTreeSelection(SourceViewTreeItem* pItemToSelect);

protected:
    // GUI Elements:
    QWidget* m_pWidget = nullptr;
    QVBoxLayout* m_pMainVBoxLayout = nullptr;
    QLabel* m_pModuleLocationInfoLabel = nullptr;

    QAction* m_pExpandAllAction = nullptr;
    QAction* m_pCollapseAllAction = nullptr;

    // Top toolbar actions:
    acWidgetAction* m_pFunctionsComboBoxAction = nullptr;
    acWidgetAction* m_pPIDComboBoxAction = nullptr;
    acWidgetAction* m_pTIDComboBoxAction = nullptr;
    acWidgetAction* m_pHotSpotIndicatorComboBoxAction = nullptr;
    acWidgetAction* m_pPIDLabelAction = nullptr;
    acWidgetAction* m_pTIDLabelAction = nullptr;

    QAction* m_pShowCodeBytesAction = nullptr;
    QAction* m_pShowAddressAction = nullptr;
    QAction* m_pShowNoteAction = nullptr;

    SourceCodeTreeView* m_pSourceCodeTree = nullptr;
    SourceCodeTreeModel* m_pTreeViewModel = nullptr;

    // Delegate:
    acTreeItemDeletate* m_pTreeItemDelegate = nullptr;

    // Selection:
    bool m_CLUNoteShown = false;
    bool m_ignoreVerticalScroll = false;

    AMDTUInt32  m_moduleId      = AMDT_PROFILE_ALL_MODULES;
    AMDTUInt32  m_functionId    = AMDT_PROFILE_ALL_FUNCTIONS;
    AMDTUInt32  m_processId     = AMDT_PROFILE_ALL_PROCESSES;
    std::vector<AMDTUInt32> m_functionIdVec;
    gtString m_srcFilePath;
    std::vector<gtString> m_supportedCounterList;
    gtMap<AMDTProcessId, gtVector<AMDTThreadId>> m_pidTidMap;
};

#endif //__SESSIONSOURCECODEVIEW_H