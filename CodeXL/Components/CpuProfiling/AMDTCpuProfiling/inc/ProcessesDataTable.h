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

    /// Fill the list data according to the requested item:
    bool fillListData();

    /// Add a module item to the table:
    /// \param process - the process to add
    /// \param pid - the process id
    /// \return true on success false on failure
    bool addProcessItem(ProcessIdType pid, const CpuProfileProcess& process);

    /// The module data cells had already added before. This function collects the data itself after reading it from
    /// the module samples
    // \param process - the process to add
    /// \param pid - the process id
    /// \param rowIndex - the row index of the process
    /// \param processDataVector - the values for the requested module
    /// \return true on success false on failure
    bool collectProcessDisplayedDataColumnValues(ProcessIdType pid, const CpuProfileProcess& process, int rowIndex, gtVector<float>& processDataVector);

    /// Calculate the process item sample count string:
    /// \param pid - the process id
    /// \sampleCountStr[out] - the sample count as string
    /// \sampleCount[out] - the sample count as gtUInt32
    /// \return true on success false on failure
    bool calculateProcessSamplesCount(ProcessIdType pid, QString& sampleCountStr, gtUInt32& moduleSampleCount);

    /// Check if according to current filter settings, the process should be displayed
    /// \param pid - the process id
    bool shouldPIDBeDisplayed(ProcessIdType pid);

    typedef struct
    {
        gtVector<float> procDataArray;
    } ProgramItemValues;

    typedef gtMap<gtString, ProgramItemValues> PathToValuesMap;
    PathToValuesMap m_pathToValuesMap;

};


#endif //__ProcessesDataTable_H

