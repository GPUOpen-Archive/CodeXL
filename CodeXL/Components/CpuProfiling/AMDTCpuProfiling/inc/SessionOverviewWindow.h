//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionOverviewWindow.h
/// \brief  The interface for the SessionOverviewWindow
///
//==================================================================================
// $Id: //devtools/branch/CPUProfileGUI/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/SessionOverviewWindow.h#1 $
// Last checkin:   $DateTime: 2013/02/04 06:21:40 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 463503 $
//=============================================================

#ifndef __SESSIONOVERVIEWWINDOW_H_
#define __SESSIONOVERVIEWWINDOW_H_

// Qt:
#include <QtCore>
#include <QtWidgets>
#include <QLabel>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afHTMLContent.h>

// Local:
#include <inc/DataTab.h>
#include <inc/CPUProfileDataTable.h>
#include <inc/DisplayFilter.h>

class ProcessesDataTable;
class ModulesDataTable;
class FunctionsDataTable;
class acQHTMLWindow;
class afApplicationTreeItemData;
class ViewCollection;
class CPUSessionTreeItemData;

class SessionOverviewWindow : public DataTab
{
    Q_OBJECT

public:
    SessionOverviewWindow(QWidget* pParent, CpuSessionWindow* pSessionWindow);
    ~SessionOverviewWindow();

    bool display(afApplicationTreeItemData* pItemData);

    bool openFunctionSourceCode(gtVAddr functionAddress, const CpuProfileModule* pModule);
	bool openSourceCodeView(QTableWidgetItem* pTableItem);

    /// \brief Name:        findModuleHandler
    /// \brief Description: Finds the modules handler for the requested module file path
    /// \param filePath - the modules file path
    /// \return CpuProfileModule - the output module handler
    const CpuProfileModule* findModuleHandler(const osFilePath& filePath) const;

    /// Updates the current tables display according to the needed update type:
    /// \param updateType the type of update needs to be performed (see SettingsDifference for details):
    virtual void UpdateTableDisplay(unsigned int updateType);

public slots:

    void onViewChanged() {};
    virtual void onEditCopy();
    virtual void onEditSelectAll();
    virtual void onFindClick();
    virtual void onFindNext();

signals:
    void functionActivated(gtVAddr functionAddress, ProcessIdType pid, ThreadIdType tid, const CpuProfileModule* pModule);
	void opensourceCodeViewSig(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32> funcModInfo);
    void hotspotIndicatorChanged(const QString& text);

protected slots:

    void openProcessesView();
    void openModulesView();
    void openFunctionsView();
    void onAfterHotSpotComboChanged(const QString& text);
    void onHotSpotComboChanged(const QString& text);
    void onTableItemActivated(QTableWidgetItem* pActivateItem);
    void onTableContextMenuActionTriggered(CPUProfileDataTable::TableContextMenuActionType actionType, QTableWidgetItem* pTableItem);


private:

    bool displaySession();

    void activateTableItem(QTableWidgetItem* pActivateItem, CPUProfileDataTable* pDataTable);

    bool displaySessionProperties();
    bool displaySessionDataTables();
    bool updateTablesHotspotIndicator();
    bool fillHotspotIndicatorCombo();

    /// \brief Name:        setSessionWindowLayout
    /// \brief Description: Set the window layout
    /// \return void
    void setSessionWindowLayout();

    /// \brief Name:        initDisplayFilters
    /// \brief Description: Initializes the display filters for each of the tables
    /// \return void
    void initDisplayFilters();

    /// \brief Name:        buildEventsStringsVector
    /// \brief Description: Builds a vector of the monitored events for this session
    /// \param[out]          eventsStrVector - the vector of strings describing the events
    /// \return True :
    /// \return False:
    bool buildEventsStringsVector(gtVector<gtString>& eventsStrVector, gtString& eventsAsHTMLTable);

    /// \brief Name:        openFunctionViewForFunction
    /// \brief Description: Open the functions view for the activated function
    /// \param[in]          pTableItem - the item activated in functions table
    void openFunctionViewForFunction(QTableWidgetItem* pTableItem);

    /// \brief Name:        openFunctionViewForModule
    /// \brief Description: Open the functions view for the activated module
    /// \param[in]          pTableItem - the item activated in functions table
    void openFunctionViewForModule(QTableWidgetItem* pTableItem);

    /// \brief Name:        openModulesViewForModule
    /// \brief Description: Open the modules view for the activated module
    /// \param[in]          pTableItem - the item activated in modules table
    void openModulesViewForModule(QTableWidgetItem* pTableItem);

    /// \brief Name:        openModulesViewForProcess
    /// \brief Description: Open the modules view for the activated process
    /// \param[in]          pTableItem - the item activated in processes table
    void openModulesViewForProcess(QTableWidgetItem* pTableItem);

    /// \brief Name:        openFunctionViewForProcess
    /// \brief Description: Open the functions view for the activated process
    /// \param[in]          pTableItem - the item activated in functions table
    void openFunctionViewForProcess(QTableWidgetItem* pTableItem);

private:

    // Widgets:
    ProcessesDataTable* m_pProcessesTable = nullptr;
    ModulesDataTable* m_pModulesTable = nullptr;
    FunctionsDataTable* m_pFunctionsTable = nullptr;
    QSplitter* m_pSplitterCentralWidget = nullptr;
    acQHTMLWindow* m_pPropertiewView = nullptr;
    QLabel* m_pProcessesHeader = nullptr;
    acWidgetAction* m_pHotSpotIndicatorComboBoxAction = nullptr;

    // Display filters for the tables:
    TableDisplaySettings m_processesTablesFilter;
    TableDisplaySettings m_modulesTablesFilter;
    TableDisplaySettings m_functionsTablesFilter;

    /// The overview window should have dedicated settings, since it is always displaying "All Data" display filter name:
    SessionDisplaySettings m_hotspotSessionSettings;

    // Does the current displayed session have multiple sessions:
    bool m_isMultiProcesses;

	int m_counterIdx = 0;
};

#endif // __SessionOverviewWindow_H_
