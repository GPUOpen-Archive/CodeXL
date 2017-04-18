//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SessionControl.h $
/// \version $Revision: #14 $
/// \brief  This file contains SessionControl class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SessionControl.h#14 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================
#ifndef _SESSION_CONTROL_H_
#define _SESSION_CONTROL_H_

#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable: 4127)
    #pragma warning(disable: 4512)
#endif
#include <QtCore>
#include <QtWidgets>
#ifdef _WIN32
    #pragma warning(pop)
#endif

#include "Session.h"
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include "CustomDataTypes.h"
#include "OccupancyInfo.h"


/// Performance counter widget class
class SessionControl : public QWidget
{
    Q_OBJECT

public :
    /// Performance counter widget class constructor.
    SessionControl(QWidget* parent = 0);

    /// Performance counter widget class destructor.
    ~SessionControl();

    /// To load current session.
    /// \param session current session
    /// \return True if succeed
    bool LoadSession(GPUSessionTreeItemData* session);

    /// To assign ToolTips of the header items of the table.
    /// \param tableView current session table view
    void AssignTooltips(QTableView* tableView);

    /// Set data grid visibility of the current session.
    void SetSessionDataGridVisibility();

    /// Show all columns of specified TableView object.
    /// \param tableView TableView of which wanted to see all columns
    void ShowAllColumns(QTableView* tableView);

    /// Set column visibility based on showZeroColumn.
    /// if showZeroColumn = true, display all columns of the specified Table
    /// if showZeroColumn = false, display only none zero columns
    /// \param showZeroColumn Flag to show none zero columns or all columns
    /// \param tableView To which visibility option will be applied
    void SetColumnVisibility(bool showZeroColumn, QTableView* tableView);

    /// To remove empty or zero columns from the table.
    /// \param tableView From which empty columns get removed
    void RemoveEmptyColumns(QTableView* tableView);

    /// Get TableView.
    /// \return TableView member of the class
    TableView* GetTableView() const { return m_sessionGridView; }

    /// Get zero column CheckBox.
    /// \return ZeroColumnCB member variable of the class
    QCheckBox* GetShowZeroColumnCB() const { return m_showZeroColumnCB; }

    /// Gets the occupancy info for the specified row
    /// \param rowIndex the row
    /// \param kernelName the kernel name for which to get the occupancy for
    /// \return the occupancy info for the specified row
    const IOccupancyInfoDataHandler* GetOccupancyForRow(int rowIndex, const QString& kernelName) const;

    /// Gets the column index of the occupancy column
    /// \return the column index of the occupancy column
    int KernelOccupancyColumnIndex() const { return m_iKernelOccupancyColumnIndex; }

    /// Gets the item model for the session
    /// \return the item model for the session
    QStandardItemModel* ItemModel() const { return m_standardModel; }

    /// Edit actions:
    virtual void onUpdateEdit_Copy(bool& isEnabled);
    virtual void onUpdateEdit_SelectAll(bool& isEnabled);

    /// Copies any selected text to the clipboard:
    virtual void OnEditCopy();
    virtual void OnEditSelectAll();


private:
    /// Populates the column headers
    /// \param headerItems the list of column headers to add
    /// \param includeOccupancyCol flag indicating if the Kernel Occupancy column should be included
    /// \return the number of columns added
    int PopulateColumnHeaders(const QStringList& headerItems, bool includeOccupancyCol);

    /// Loads profiler data into TableView.
    /// \param session the session containing the data to load
    /// \return True if succeed
    bool FillDataTableWithProfileData(GPUSessionTreeItemData* session);

    /// Initializes the Qt components
    void InitializeComponent();

    /// is session exist or already been deleted
    static bool IsControlExistInMap(QWidget* session);

    /// Removes the _pass_* from the input string if exists
    /// \param inputString input string
    /// \return QString after removeing _pass_* if it exists else return inputString
    QString RemovePassStringFromCounter(QString inputString);

    bool                      m_savedShowKernelDispatchCBState; ///< Save kernel dispatch CheckBox state.
    int                       m_iThreadColumnIndex;             ///< Represents Thread Column Index
    int                       m_iMethodColumnIndex;             ///< Represents Method Column Index
    int                       m_iKernelOccupancyColumnIndex;    ///< Represents Kernel Occupancy Column Index

    QStandardItemModel*       m_standardModel;                  ///< Represent current Model of the TableView
    TableView*                m_sessionGridView;                ///< Table view to show csv file data
    QToolBar*                 m_perfCounterToolBar;             ///< ToolBar to show performance counter options
    QCheckBox*                m_showZeroColumnCB;               ///< Shows Zero Column CheckBox

    QMap<int, const IOccupancyInfoDataHandler*> m_rowOccupancyInfoMap;            ///< Map of row to occupancy info

    static QMap<QWidget*, bool>      m_sessionControlsMap;      ///< Map of sessions
};

///QStandardItem descendant to provide numerical sorting of string data in the perf counter table
class PerfCounterItem : public QStandardItem
{
public:
    /// overridden < operator to do numerical conversion of string data
    /// \param other the QStandardItem being compared to this one
    /// \return true if this value is less than "other" value, false otherwise
    bool operator< (const QStandardItem& other) const;
};


#endif //_SESSION_CONTROL_H_
