//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SessionWindow.h $
/// \version $Revision: #22 $
/// \brief  This file contains GPUSessionWindow class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SessionWindow.h#22 $
// Last checkin:   $DateTime: 2016/03/17 05:20:00 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 564197 $
//=====================================================================
#ifndef _SESSIONWINDOW_H_
#define _SESSIONWINDOW_H_

#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SharedSessionWindow.h>

#include <AMDTGpuProfiling/SessionControl.h>
#include <AMDTGpuProfiling/Util.h>
#include <AMDTGpuProfiling/Session.h>
#include <AMDTGpuProfiling/OccupancyInfo.h>
#include <AMDTGpuProfiling/SessionViewTabWidget.h>

#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/gpBaseSessionView.h>

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool


//forward declarations
class CodeViewerWindow;
class KernelOccupancyWindow;

/// UI for Perf Counter GPUSessionTreeItemData window
class GPUSessionWindow : public gpBaseSessionView
{
    Q_OBJECT
public:
    /// Constructor
    /// \param parent the parent widget
    GPUSessionWindow(QWidget* parent = 0);

    /// Destructor
    ~GPUSessionWindow();

    /// Display the session file. This function should be implemented for session views with multiple children
    /// \param sessionFilePath the file path for the requested session
    /// \param sessionInnerPage the item type describing the inner view to open, or AF_TREE_ITEM_ITEM_NONE when the root is supposed to open
    /// \param [out] errorMessage when the display fails, errorMessage should contain a message for the user
    virtual bool DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage, QString& errorMessage);

    /// Clear data table
    void Clear();

    /// Gets current session control
    /// \return pointer of the current session control
    SessionControl* GetControl() const { return m_pControl; }

    /// Queries the window to see if it's current session is the specified session
    /// \param pSession the session to check for
    /// \return true if the specified session is the view's current session, false otherwise
    bool IsCurrentSession(GPUSessionTreeItemData* pSession) const { return pSession == m_pCurrentSession; };

    /// Edit actions:
    virtual void onUpdateEdit_Copy(bool& isEnabled);
    virtual void onUpdateEdit_SelectAll(bool& isEnabled);

    /// Copies any selected text to the clipboard:
    virtual void OnEditCopy();
    virtual void OnEditSelectAll();

    /// handle the find & find next functions
    virtual void onFindClick();
    virtual void onEditFindNext();
    virtual void onUpdateEdit_Find(bool& isEnabled);
    virtual void onUpdateEdit_FindNext(bool& isEnabled);

private slots:
    /// Called to hide row
    void HideSelectedRow(int rowIndex);

    /// Called to show context menu containing nearby hidden rows of the selected row.
    void ContextMenuForHiddenRows(const QPoint& p);

    /// Called to unhide row(s)
    void ShowHiddenRows(QAction*);

    /// Called when show Zero Column CheckBox gets checked/unchecked.
    /// \param state the state(checked/unchecked) of CheckBox
    void ChangeStateInZeroColumnCB(int state);

    /// Called when the mouse enters into a cell -- shows the hand cursor if the cell contains a kernel name or occupancy info
    /// \param index of the cell
    void MouseEnterInACell(const QModelIndex& index);

    /// Called when the cell data is clicked.
    /// \param index of the cell
    void CellContentClicked(const QModelIndex& index);


private:
    static float   ms_widthFactor;             ///< extra space factor for cell width
    static QString ms_strShowRowMenuText;      ///< menu text for "Show Row" menus
    static QString ms_strShowAllRowsMenuText;  ///< menu text for "Show all hidden rows" menu item
    static QString ms_strNoHiddenRowsMenuText; ///< menu text for "No Hidden rows" menu item
    static QString ms_strNoPreformanceDataText;///< user information message regarding kernels w no performance counters data
    static QString ms_strNoPreformanceDataHeading; /// < user information message title

    /// Called to create Group of actions of nearby hidden rows.
    /// \param rowIndex will indicate selected row number.
    /// \return true for successful execution else false
    bool CreateGroupActions(int rowIndex);

    /// Parse the cell value in the session data grid and check whether it contains a kernel name.
    /// \param strKernelName the output kernel name
    /// \param index the Model index to the session data cell
    /// \return true if the cell contains a kernel name, false otherwise
    bool GetKernelNameFromTV(QString& strKernelName, const QModelIndex index);

    /// Compute the width of the given string
    /// \param str the string whose width to be computed
    /// \param fm the font metrics for computing width
    /// \return width of the string
    int ComputeStringWidth(const QString& str, const QFontMetrics& fm);

    /// Adjust the width of columns so that most of the contents are visible
    void AdjustColumnWidth();

    /// Counts number of visible rows
    int VisibleRows();

    /// Helper function to show a hidden row
    /// \param rowIndex the index of the row to show
    void ShowHiddenRow(int rowIndex);

    /// Helper function to check if the passed header column
    /// of empty cell is relevant to display a message
    /// \param headerText of current column
    bool IsEmptyCellRelevant(const QString& headerText)const;

    /// Helper function to detect empty cells in the current row
    /// and if there relevant empty cells to display a message
    /// \param pItemModel a model pointer to be able to access row cells
    /// \param row - current row
    /// \param isEmptyCellFound - indicates that empty cells are found
    /// \param isEmptyCellRelevant - indicates that found empty cells are relevant and a message should be displayed
    /// \param shouldIgnoreCPURows - indicates that a row with first cell containing "CPU" word should be ignored ( true by default)
    void DetectEmptyCells(QStandardItemModel* pItemModel, const int& row, bool& isEmptyCellFound, bool& isEmptyCellRelevant, bool shouldIgnoreCPURows = true)const;

    GPUSessionTreeItemData*               m_pCurrentSession;         ///< the current loaded session
    SessionControl*        m_pControl;                ///< pointer to current session control.
    QActionGroup*          m_pGroupActions;           ///< Group of actions for hidden rows
    QList<bool>            m_visibleRows;             ///< List of visible rows
};

#endif
