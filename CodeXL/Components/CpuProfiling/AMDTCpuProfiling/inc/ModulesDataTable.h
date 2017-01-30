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

enum ModuleSummaryCol
{
    //CXL_MOD_SUMMARY_MOD_ID_COL = 0,
    CXL_MOD_SUMMARY_MOD_NAME_COL = 0,
    CXL_MOD_SUMMARY_SAMPLE_COL,
    CXL_MOD_SUMMARY_SAMPLE_PER_COL,
};

enum ModuleTabCol
{
    CXL_MOD_TAB_MOD_ID_COL = 0,
    CXL_MOD_TAB_MOD_NAME_COL,
    CXL_MOD_TAB_SYM_LOADED_COL,
    CXL_MOD_TAB_SAMPLE_START_COL,
    CXL_MOD_TAB_TBP_SAMPLE_COL = CXL_MOD_TAB_SAMPLE_START_COL,
    CXL_MOD_TAB_TBP_SAMPLE_PER_COL,
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

    int getEmptyMsgItemColIndex() const;

    // returns modules table type
    TableType GetTableType() const;

    bool findModuleId(int rowIndex, AMDTModuleId& modId);

public slots:

    void onAboutToShowContextMenu() override;

protected:

    bool fillSummaryTable(int counterIdx) override;
    bool fillTableData(AMDTProcessId procId, AMDTModuleId modId, std::vector<AMDTUInt64> modIdVec = {}) override;

private:

    bool AddRowToTable(const gtVector<AMDTProfileData>& allProcessData);
    void mergeProfileModuleData(gtVector<AMDTProfileData>& data) const;

    int m_moduleIdColumn = -1;
    int m_moduleNameColumn = -1;

    CpuSessionWindow* m_pParentSessionWindow = nullptr;
};


#endif //__MODULESDATATABLE_H

