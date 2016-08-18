//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionFunctionView.h
///
//==================================================================================

#ifndef __SESSIONFUNCTIONVIEW_H
#define __SESSIONFUNCTIONVIEW_H

// Qt:
#include <QtCore>
#include <QtWidgets>
#include <QLabel>
#include <QMainWindow>
#include <QTextEdit>
// Infra:
#include <AMDTBaseTools/Include/gtMap.h>

// Local:
#include <inc/DataTab.h>
#include <inc/DisplayFilter.h>
#include <inc/ModuleFilterDialog.h>
#include <inc/FunctionsDataTable.h>

class afApplicationTreeItemData;


class SessionFunctionView : public DataTab
{
    Q_OBJECT

public:
    SessionFunctionView(QWidget* pParent, CpuSessionWindow* pSessionWindow);
    SessionFunctionView();
    ~SessionFunctionView();

    bool display(afApplicationTreeItemData* pItemData);

    /// Select and set the focus on the selected function
    /// \param - functionName the function name
    /// \param - pid the process ID
    //void selectFunction(const QString& functionName, ProcessIdType pid);
    void selectFunction(const QString& funcId);


    /// Display the functions for the requested module
    /// \param - moduleFullPath the module full path
    void displayModule(const QString& moduleFullPath);

    /// Filter functions by a PID
    /// \param pid - the filter pid
    void filterByPID(int pid);

    /// Filter functions by a the process file path
    /// \param moduleFilePaths - the list of modules file paths
    void FilterByModuleFilePaths(AMDTModuleId moduleId);

    /// Updates the current tables display according to the needed update type:
    /// \param updateType the type of update needs to be performed (see SettingsDifference for details):
    virtual void UpdateTableDisplay(unsigned int updateType);

    ///Local CLU status
    bool m_CLUNoteShown;
signals:
    void functionActivated(gtVAddr functionAddress, ProcessIdType pid, ThreadIdType tid, const CpuProfileModule* pModule);
    void opensourceCodeViewSig(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32> funcModInfo);

public slots:

    void onViewChanged() {};
    void onOpenModuleSelector(const QString& link);
    void onCellChanged();

    virtual void onEditCopy();
    virtual void onEditSelectAll();
    virtual void onFindClick();
    virtual void onFindNext();

protected slots:

    void onDisplayFilterChanged(int index);
    void onSelectPid(int index);
    void onTableItemActivated(QTableWidgetItem* pActivateItem);
    void onTableContextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType actionType, QTableWidgetItem* pTableItem);
    void onOpenDisplayFilterDialog();
protected:

    /// \brief Name:        displaySessionDataTables
    /// \brief Description: Display the session data tables with the current demands
    bool displaySessionDataTables();

    /// \brief Name:        setSessionWindowLayout
    /// \brief Description: Builds the window GUI layout
    void setSessionWindowLayout();

    /// \brief Name:        CreateToolbar
    /// \brief Description: Create the toolbar for this view
    void CreateToolbar();

    /// \brief Name:        initDisplayFilters
    /// \brief Description: Initializes the display filters for each of the tables
    /// \return void
    void initDisplayFilters();

    /// \brief Name:        updateModulesFilterLinkString
    /// \brief Description: Get a formatted html string that describes the module selection
    /// \return QString
    QString updateModulesFilterLinkString();

    /// \brief Name:        fillPIDComb
    /// \brief Description: Fill th elist of process ID
    /// \return void
    void fillPIDComb();

private:
    // Widgets:
    FunctionsDataTable* m_pFunctionTable = nullptr;

    // Display filters for the tables:
    TableDisplaySettings    m_functionsTablesFilter;
    acWidgetAction* m_pLabelModuleSelectedAction = nullptr;
    acWidgetAction* m_pPIDComboBoxAction         = nullptr;

    //void addModulesForPID(uint pid);
    void updateDataFromPidComboBox();
    ProcessIdType getCurrentPid();
    bool m_updateData;
    AMDTModuleId m_moduleId = AMDT_PROFILE_ALL_MODULES;

    std::map<gtString, AMDTUInt64> m_moduleNameIdMap;

};

#endif //__SESSIONFUNCTIONVIEW_H
