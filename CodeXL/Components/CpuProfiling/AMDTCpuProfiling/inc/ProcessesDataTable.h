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

enum ProcessTableCol
{
    PROCESS_NAME_COL = 0,
    PROCESS_ID_COL,
    PROCESS_SAMPLE_COL,
    PROCESS_TBP_PER_COL = PROCESS_SAMPLE_COL + 1
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
    bool findProcessDetails(int rowIndex, ProcessIdType& pid, QString& processFileName);

    // returns processes table type
    TableType GetTableType() const;

protected:

    virtual bool fillSummaryTables(int counterIdx);
    virtual bool fillTableData(AMDTProcessId procId, AMDTModuleId modId, std::vector<AMDTUInt64> modIdVec = {});

private:
    bool AddRowToTable(const gtVector<AMDTProfileData>& allProcessData);
};


#endif //__ProcessesDataTable_H

