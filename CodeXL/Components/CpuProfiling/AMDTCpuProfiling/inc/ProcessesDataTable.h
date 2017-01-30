//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ProcessesDataTable.h
///
//==================================================================================

#ifndef __ProcessesDataTable_H
#define __ProcessesDataTable_H

#include <inc/CPUProfileDataTable.h>


enum ProcessSummaryCol
{
    CXL_PROC_SUMMARY_PROC_NAME_COL = 0,
    CXL_PROC_SUMMARY_PROC_ID_COL,
    CXL_PROC_SUMMARY_SAMPLE_COL,
    CXL_PROC_SUMMARY_SAMPLE_PER_COL,
};

enum ProcessTabCol
{
    CXL_PROC_TAB_PROC_NAME_COL = 0,
    CXL_PROC_TAB_PROC_ID_COL,
    CXL_PROC_TAB_SAMPLE_START_COL,
    CXL_PROC_TAB_TBP_SAMPLE_COL = CXL_PROC_TAB_SAMPLE_START_COL,
    CXL_PROC_TAB_TBP_SAMPLE_PER_COL,
};

/// -----------------------------------------------------------------------------------------------
/// \class Name: ProcessesDataTable : public CPUProfileDataTable
/// \brief Description:  This class will be used to display processes in data table.
///                      The class will be used in the CPU profiling views
/// -----------------------------------------------------------------------------------------------
class ProcessesDataTable : public CPUProfileDataTable
{
    Q_OBJECT

public:

    ProcessesDataTable(QWidget* pParent, const gtVector<TableContextMenuActionType>& additionalContextMenuActions, SessionTreeNodeData* pSessionData);

    virtual ~ProcessesDataTable();

    /// Finds details for the process in the requested row index
    /// \param rowIndex - the process row index
    /// \param pid[out] - the process file name
    /// \param processFileName[out] - the process id
    /// \return true on success false on failure
    bool findProcessDetails(int rowIndex, AMDTProcessId& pid, QString& processFileName);

    int getEmptyMsgItemColIndex() const;

    // returns processes table type
    TableType GetTableType() const;

protected:

    bool fillSummaryTable(int counterIdx) override;
    bool fillTableData(AMDTProcessId procId, AMDTModuleId modId, std::vector<AMDTUInt64> modIdVec = {}) override;

private:

    bool AddRowToTable(const gtVector<AMDTProfileData>& allProcessData);
    void mergeProfileProcessData(gtVector<AMDTProfileData>& data) const;

    int m_processIdColumn = -1;
    int m_processNameColumn = -1;
};


#endif //__ProcessesDataTable_H

