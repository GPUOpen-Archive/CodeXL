//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file FunctionsDataTable.h
///
//==================================================================================

#ifndef __FunctionsDataTable_H
#define __FunctionsDataTable_H

// Qt:
#include <QtCore>
#include <QtWidgets>

// Local:
#include <inc/CPUProfileDataTable.h>

class CpuSessionWindow;

enum FunctionSummaryCol
{
    AMDT_FUNC_SUMMMARY_FUNC_ID_COL = 0,
    AMDT_FUNC_SUMMMARY_FUNC_NAME_COL,
    AMDT_FUNC_SUMMMARY_FUNC_SAMPLE_COL,
    AMDT_FUNC_SUMMMARY_FUNC_PER_SAMPLE_COL,
    AMDT_FUNC_SUMMMARY_FUNC_MODULE_COL,
    AMDT_FUNC_FUNC_NAME_COL = AMDT_FUNC_SUMMMARY_FUNC_NAME_COL,
    AMDT_FUNC_FUNC_MODULE_COL,
    AMDT_FUNC_START_SAMPLE,
    AMDT_FUNC_TBP_PER_COL = AMDT_FUNC_START_SAMPLE + 1
};

/// -----------------------------------------------------------------------------------------------
/// \class Name: FunctionsDataTable : public CPUProfileDataTable
/// \brief Description:  This class will be used to display functions in data table.
///                      The class will be used in the CPU profiling views
/// -----------------------------------------------------------------------------------------------
class FunctionsDataTable : public CPUProfileDataTable
{
    Q_OBJECT

public:

    FunctionsDataTable(QWidget* pParent,
                       const gtVector<TableContextMenuActionType>& additionalContextMenuActions,
                       SessionTreeNodeData* pSessionData,
                       CpuSessionWindow* pSessionWindow);
    virtual ~FunctionsDataTable();

    // TODO: Unused ??
    int getFunctionNameColumnIndex() const { return m_functionNameColIndex; }

    QString getFunctionName(int rowIndex) const;
    QString getModuleName(int rowIndex) const;
    QString getFunctionId(int rowIndex) const;

    /// Set the popup for missing files attribute:
    void setPopupToBrowseMissingFiles(bool popup) {m_popupToBrowseMissingFiles = popup;}

    // returns functions table type
    TableType GetTableType() const;

public slots:

    /// Overriding CPUProfileDataTable:
    virtual void onAboutToShowContextMenu();

protected:

    virtual bool fillSummaryTables(int counterIdx);
    virtual bool fillTableData(AMDTProcessId procId, AMDTModuleId modId, std::vector<AMDTUInt64> modIdVec = {});
    virtual bool HandleHotSpotIndicatorSet();

    bool setModuleIcon(int row, const AMDTProfileModuleInfo& moduleInfo);
    bool AddRowToTable(const gtVector<AMDTProfileData>& allModuleData);

    /// Contain the indices for the function and module name columns:
    int m_functionNameColIndex;
    int m_moduleNameColIndex;
    bool m_popupToBrowseMissingFiles;

    CpuSessionWindow* m_pParentSessionWindow;
};


#endif //__FunctionsDataTable_H

