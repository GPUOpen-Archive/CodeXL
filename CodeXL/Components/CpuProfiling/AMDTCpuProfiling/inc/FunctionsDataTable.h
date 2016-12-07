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
};

enum FunctionTabCol
{
    AMDT_FUNC_TAB_FUNC_ID_COL = 0,
    AMDT_FUNC_TAB_FUNC_NAME_COL,
    AMDT_FUNC_TAB_MOD_NAME_COL,
    AMDT_FUNC_TAB_SAMPLE_START_COL,
    AMDT_FUNC_TAB_TBP_SAMPLE_COL = AMDT_FUNC_TAB_SAMPLE_START_COL,
    AMDT_FUNC_TAB_TBP_PER_SAMPLE_COL,
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
    virtual bool HandleHotSpotIndicatorSet() { return true; };
    bool AddRowToTable(const gtVector<AMDTProfileData>& allModuleData);

    /// Contain the indices for the function and module name columns:
    int m_functionIdColumn = -1;
    int m_functionNameColumn = -1;
    int m_moduleNameColumn = -1;

    bool m_popupToBrowseMissingFiles = true;

    CpuSessionWindow* m_pParentSessionWindow = nullptr;
};


#endif //__FunctionsDataTable_H

