//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ModulesDataTable.h
///
//==================================================================================

#ifndef __MODULESDATATABLE_H
#define __MODULESDATATABLE_H

#include <inc/CPUProfileDataTable.h>

class CpuSessionWindow;

enum ModuleTableCol
{
    AMDT_MOD_TABLE_SUMMARY_MOD_NAME = 0,
    AMDT_MOD_TABLE_SUMMARY_SAMPLE,
    AMDT_MOD_TABLE_SUMMARY_SAMPLE_PER,
    AMDT_MOD_TABLE_MOD_ID = 0,
    AMDT_MOD_TABLE_MOD_NAME,
    AMDT_MOD_TABLE_SYMBOL_LOADED,
    AMDT_MOD_TABLE_CLU_HS_COL = 1
};
/// -----------------------------------------------------------------------------------------------
/// \class Name: ModulesDataTable : public CPUProfileDataTable
/// \brief Description:  This class will be used to display modules in data table.
///                      The class will be used in the CPU profiling views
/// -----------------------------------------------------------------------------------------------
class ModulesDataTable : public CPUProfileDataTable
{
    Q_OBJECT

public:

    ModulesDataTable(QWidget* pParent,
                     const gtVector<TableContextMenuActionType>& additionalContextMenuActions,
                     SessionTreeNodeData* pSessionData,
                     CpuSessionWindow* pSessionWindow);

    virtual ~ModulesDataTable();
    /// Find the module file name for the module in the specified row:
    /// \param moduleRowIndex - the requested module row index
    /// \param[out] moduleFilePath - the requested module file path
    /// \return true on success false on failure
    bool findModuleFilePath(int moduleRowIndex, QString& moduleFilePath);
    // returns modules table type
    TableType GetTableType() const;

    bool findModueId(int rowIndex, AMDTModuleId& modId);

public slots:

    /// Overrides CPUProfileDataTable:
    virtual void onAboutToShowContextMenu();

protected:

    virtual bool fillSummaryTables(int counterIdx);
    virtual bool fillTableData(AMDTProcessId procId, AMDTModuleId modId, std::vector<AMDTUInt64> modIdVec = {});

private:
    bool AddRowToTable(const gtVector<AMDTProfileData>& allProcessData);

    CpuSessionWindow* m_pParentSessionWindow;
};


#endif //__MODULESDATATABLE_H

