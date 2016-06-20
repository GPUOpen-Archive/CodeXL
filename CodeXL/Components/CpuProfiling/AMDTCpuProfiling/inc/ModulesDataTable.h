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

    /// Checks if a module symbols are loaded for the module displayed in the table row:
    /// \param int moduleRowIndex the module row
    /// \return true iff the module symbols are loaded
    bool AreModuleSymbolsLoaded(int moduleRowIndex);

    // returns modules table type
    TableType GetTableType() const;

	bool findModueId(int rowIndex, AMDTModuleId & modId);

public slots:

    /// Overrides CPUProfileDataTable:
    virtual void onAboutToShowContextMenu();

protected:

    /// Fill the list data according to the requested item:
    bool fillListData();

	virtual bool fillSummaryTables(int counterIdx); 
	virtual bool fillTableData(AMDTProcessId procId, AMDTModuleId modId, std::vector<AMDTUInt64> modIdVec = {});

    /// Check if the module should be displayed or filtered:
    /// \pModule - the requested module
    /// \return true if the modules should be displayed, false otherwise
    bool shouldModuleBeDisplayed(const CpuProfileModule* pModule);
private:

    /// Add a module item to the table:
    /// \param pModule - the module to add
    /// \return true on success false on failure
    bool addModuleItem(const CpuProfileModule* pModulemoduleDataVector);

    /// Calculate the module item sample count string:
    /// \param pModule - the module
    /// \sampleCountStr[out] - the sample count as string
    /// \sampleCount[out] - the sample count as gtUInt32
    /// \return true on success false on failure
    bool calculateModuleSamplesCount(const CpuProfileModule* pModule, QString& sampleCountStr, gtUInt32& sampleCount);

    /// The module data cells had already added before. This function collects the data itself after reading it from
    /// the module samples
    /// \param pModule - the module to calculate the data for
    /// \param moduleRowIndex - the module row index
    /// \param moduleDataVector - the values for the requested module
    /// \return true on success false on failure
    bool collectModuleDisplayedDataColumnValues(const CpuProfileModule* pModule, gtVector<float>& moduleDataVector);


    CpuSessionWindow* m_pParentSessionWindow;
};


#endif //__MODULESDATATABLE_H

