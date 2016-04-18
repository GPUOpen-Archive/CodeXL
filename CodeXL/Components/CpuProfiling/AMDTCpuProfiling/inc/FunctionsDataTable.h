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

// Executable:
#include <AMDTExecutableFormat/inc/ExecutableFile.h>

// Local:
#include <inc/CPUProfileDataTable.h>

class CpuSessionWindow;

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

    /// Returns the amount of monitored threads in the profile reader:
    int amountOfThreads() const { return (int)m_threads.size(); }

    int getFunctionNameColumnIndex() const { return m_functionNameColIndex; }

    QString getFunctionName(int rowIndex) const;

    /// Get the function address for the function in row row index
    gtVAddr getFunctionAddress(int rowIndex, const CpuProfileModule*& pModule) const;

    const QList<ProcessIdType>* getFunctionPidList(int rowIndex) const;

    /// Set the popup for missing files attribute:
    void setPopupToBrowseMissingFiles(bool popup) {m_popupToBrowseMissingFiles = popup;}

    /// Finds the modules handler for the requested module file path
    /// \param filePath - the modules file path
    /// \return CpuProfileModule - the output module handler
    const CpuProfileModule* findModuleHandler(const osFilePath& filePath) const;

    // returns functions table type
    TableType GetTableType() const;

public slots:

    /// Overriding CPUProfileDataTable:
    virtual void onAboutToShowContextMenu();

protected:

    class FunctionData
    {
    public:
        QString m_functionName;
        gtVAddr m_functionAddress;
        gtVector<float> m_dataVector;
        QList<ProcessIdType> m_pidList;
        const CpuProfileModule* m_pModule;
    };

    /// Fill the list data according to the requested item:
    bool fillListData();

    /// Build the map of the current hot spot values.
    /// The values are taken from m_functionsInfosVec in order to save the
    /// calculation of the whole table once changing the hot spot indicator
    /// \return true / false is succeeded or failed
    virtual bool buildHotSpotIndicatorMap();

    /// handles event of changing the hotspot indicator combobox
    /// \return true / false is succeeded or failed
    virtual bool HandleHotSpotIndicatorSet();

    /// Add all the function items for the module "pid"
    /// \pModule - the module structure
    /// \moduleFilePath - module file path
    bool addFunctionsForModule(const CpuProfileModule* pModule, const QString& moduleFilePath, AggregatedSample& parentSamples);

    /// Add the requested function to the table
    /// \param rowIndex - the row index
    /// \param functionData - the function information (name + address)
    /// \param functionDataVector - the data vector for the function
    /// \param moduleFilePath - the function module path
    /// \param is32BitModule is the function module a 32bit module
    bool addFunctionItem(int rowIndex, const FunctionData& functionData, const QString& moduleFilePath, bool is32BitModule);

    /// Check if the input module path should be filtered:
    bool shouldModuleBeDisplayed(const CpuProfileModule& module);

    const FunctionData* getFunctionData(int rowIndex) const;


protected:

    // Testing performance:
    bool fillListData1();
    bool fillListData2();

protected:

    /// Member variables that are used for the calculations of functions:
    gtVAddr m_lastParentAddr;

    gtVector<FunctionData> m_functionsInfosVec;

    /// Contain the currently displayed session amount of thread:
    QList<int> m_threads;

    /// True iff the user should receive a popup for missing files:
    bool m_popupToBrowseMissingFiles;

    /// Contain the indices for the function and module name columns:
    int m_functionNameColIndex;
    int m_moduleNameColIndex;

    CpuSessionWindow* m_pParentSessionWindow;
};


#endif //__FunctionsDataTable_H

