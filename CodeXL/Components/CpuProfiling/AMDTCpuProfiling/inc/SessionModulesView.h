//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionModulesView.h
///
//==================================================================================

#ifndef __SESSIONMODULESVIEW_H
#define __SESSIONMODULESVIEW_H

// Qt:
#include <QtCore>
#include <QtWidgets>
#include <QLabel>
#include <QTextEdit>
// Infra:
#include <AMDTBaseTools/Include/gtMap.h>

// Local:
#include <inc/DataTab.h>
#include <inc/CPUProfileDataTable.h>
#include <inc/DisplayFilter.h>

class afApplicationTreeItemData;
class ProcessesDataTable;
class ModulesDataTable;

enum LAST_SELECTED_TABLE
{
    TOP_PROCESS_TABLE,
    TOP_MODULE_TABLE,
    BOTTOM_MODULE_TABLE,
    BOTTOM_PROCESS_TABLE,
    TOTAL_TABLE_COUNT
};

class SessionModulesView : public DataTab
{
    Q_OBJECT

public:
    SessionModulesView(QWidget* pParent, CpuSessionWindow* pSessionWindow);
    ~SessionModulesView();

    bool display(afApplicationTreeItemData* pItemData);

    /// Select and set the focus on the selected module
    /// \param - moduleName the module name
    void selectModule(const QString& moduleName);

    /// Select and set the focus on the selected process
    /// \param - processName the process name
    void selectProcess(const QString& processName);

    /// Updates the current tables display according to the needed update type:
    /// \param updateType the type of update needs to be performed (see SettingsDifference for details):
    virtual void UpdateTableDisplay(unsigned int updateType);

    /// Display the view by processes:
    void displayByProcesses();

    ///Local CLU status
    bool m_CLUNoteShown;

public slots:

    void onViewChanged() {};
    virtual void onEditCopy();
    virtual void onEditSelectAll();
    virtual void onFindClick();
    virtual void onFindNext();

private slots:

    /// Is handling the change of the display by combo:
    void onDisplayByChanged(int index);

    /// Is handling selection changed for the processes and modules top table:
    void onProcessesTableCellChanged();
    void onModulesTableCellChanged();

    /// Item activation and context menu slots:
    void onTableItemActivated(QTableWidgetItem* pActivateItem);
    void onTableContextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType actionType, QTableWidgetItem* pTableItem);

    void onBottomProcessesTableCellChanged();
    void onBottomModulesTableCellChanged();
    //void onTableSelectBottomProcess();
    //void onTableSelectBottomModule();

private:

    bool displaySessionDataTables();

    void setSessionWindowLayout();

    /// Create the top toolbar for this view:
    acToolBar* createViewTopToolbar();

    /// \brief Name:        initDisplayFilters
    /// \brief Description: Initializes the display filters for each of the tables
    /// \return void
    void initDisplayFilters();

    /// Activate a table item:
    void activateTableItem(QTableWidgetItem* pActivateItem, CPUProfileDataTable* pDataTable);

private:

    // Widgets:
    ProcessesDataTable* m_pTopProcessesTable;
    ProcessesDataTable* m_pBottomProcessesTable;
    ModulesDataTable* m_pTopModulesTable;
    ModulesDataTable* m_pBottomModulesTable;

    // Display filters for the tables:
    TableDisplaySettings m_modulesTableFilter;
    TableDisplaySettings m_processesTableFilter;

    QSplitter* m_pSplitterCentralWidget;
    acWidgetAction* m_pComboBoxDisplayByAction;

    QWidget* m_pProcessWidget;
    QWidget* m_pModulesWidget;
    QWidget* m_pHintFrame;

    acWidgetAction* m_pTopLabelAction;
    QLabel* m_pBottomLabel;

    bool m_isProcessesUp;
    enum LAST_SELECTED_TABLE m_latestSelectedTable;
};


#endif //__SESSIONMODULESVIEW_H

