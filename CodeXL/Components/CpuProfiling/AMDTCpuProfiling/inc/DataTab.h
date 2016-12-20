//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DataTab.h
/// \brief  Interface for the DataTab class.
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/DataTab.h#56 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

#ifndef DATA_TAB_H
#define DATA_TAB_H

// Infrastructure
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTBaseTools/Include/gtString.h>

// QT
#include <QtCore>
#include <QtWidgets>
#include <QMainWindow>
#include <QMenu>
#include <QMutex>
#include <QPointer>
#include <QTreeWidget>

// Local
#include <inc/StdAfx.h>

#include <AMDTCpuProfilingRawData/inc/CpuProfileReader.h>

#include <inc/DisplayFilter.h>

// Threshold for CLU
#define THRESHOLD_CLU_UTIL_LOW  1
#define THRESHOLD_CLU_UTIL_MED  15
#define THRESHOLD_CLU_SPAN      1

enum CLU_NOTE
{
    CLU_NOTE_SPAN,
    CLU_NOTE_SPAN_OVER_THRESHOLD,
    CLU_NOTE_LOW_UTILIZATION,
    CLU_NOTE_MEDIUM_UTILIZATION,
    CLU_NOTE_LOW_CLU_HIGH_ACCESS_RATE,
    CLU_NOTE_MED_CLU_HIGH_ACCESS_RATE,
    CLU_NOTE_COMPULSORY,
    CLU_NOTE_BAD_DISASM,
    CLU_NOTE_COUNT
};

class TableDisplaySettings;
class CPUProfileDataTable;
class CpuSessionWindow;
class afApplicationTreeItemData;
class ViewCollection;
class acToolBar;
class acWidgetAction;


/// -----------------------------------------------------------------------------------------------
/// \class Name: ProfileViewDisplayInformation
/// \brief Description:  This class is used to save and restore the user display settings for the
///                      CPU profile GUI. The view display information should be restore after changing
///                      the display settings for instance, or selecting another function.
/// -----------------------------------------------------------------------------------------------
class ProfileViewDisplayInformation
{
public:
    ProfileViewDisplayInformation();
    void clear();

    /// Returns true iff the current position is the bottom row.
    bool isAtBottom() const { return (m_maxVerticalScrollPosition == m_verticalScrollPosition);}

    int m_selectedTopLevelItemIndex;
    int m_selectedTopLevelChildItemIndex;
    int m_sortByColumn;
    Qt::SortOrder m_sortOrder;
    bool m_shouldExpandSelected;
    int m_maxVerticalScrollPosition;
    int m_verticalScrollPosition;
    QList<int> m_expandedTreeItems;
    QString m_selectedHotSpot;

};

// All different tabs showing data about a session should be DataTabs.
// They account for the different data shown in a view, a changed viewed, and
// exporting the data via the main window to a csv file
class DataTab : public QMainWindow, public apIEventsObserver
{
    Q_OBJECT;


public:
    DataTab(QWidget* pParent, CpuSessionWindow* pParentSessionWindow, const QString& sessionDir = QString());
    virtual ~DataTab();
    virtual QString GetExportString() {return m_exportString;};
    virtual acTreeCtrl* GetExportList() {return m_pList;};
    virtual void stopToolbarMovement() {return;};

    const afApplicationTreeItemData* currentlyDisplayedItemData() const {return m_pDisplayedSessionItemData;};
    void setDisplayedItemData(afApplicationTreeItemData* pData) {m_pDisplayedSessionItemData = pData;};

    void showInformationPanel(bool show);

    /// Updates the current tables display according to the needed update type:
    /// \param updateType the type of update needs to be performed (see SettingsDifference for details):
    virtual void UpdateTableDisplay(unsigned int updateType) {(void)(updateType); };

    /// Should be override by inherit classes. Is used to set the sort / selection in table as it was before the change:
    virtual void applyUserDisplayInformation() {};

    const CpuSessionWindow* parentSessionWindow() const {return m_pParentSessionWindow;}


    /// Filter information function:
    void setFilter(TableDisplaySettings* ipFilter) { m_pDisplaySettings = ipFilter; }
    virtual QString displayFilterString();
    void updateDisplaySettingsString();

    void setEnableOnlySystemDllInfilterDlg(bool enableOnly) { m_enableOnlySystemDllInDisplaySettings = enableOnly; }

    /// \brief Name:        openCallGraphViewForFunction
    /// \brief Description: Open the call graph view for the activated function
    /// \param[in]          funcName - the name of the function
    /// \param[in]          pid the process ID of the function
    //void openCallGraphViewForFunction(const QString& funcName, ProcessIdType pid);
    void openCallGraphViewForFunction(AMDTFunctionId funcId, ProcessIdType pid);

    /// Sets the flag stating which change should be performed once the view is displayed:
    /// The type of change that should be applied
    void SetUpdateType(unsigned int changeType);

    void UpdateTableDisplaySettings();

    void  IntializeCLUNoteString();
signals:
    void exportData(QTreeWidget* pList);

public slots:

    virtual void onViewChanged() = 0;
    virtual void onEditCopy();
    virtual void onEditSelectAll() {};
    virtual void onFindClick() {};
    virtual void onFindNext() {};
    virtual void onExport() {emit exportData(m_pList);};
    virtual void OnApplicationFocusChanged(QWidget* pOld, QWidget* pNew);


protected:

    void ResizeColumnsToContents();

    /// Creates a frame that contain an hint icon:
    QFrame* createHintLabelFrame();

    /// Create the CLU notes pane:
    void createCLUNotesFrame(QLayout* pLayout);

    /// \brief Name:        selectTableItem
    /// \brief Description: Selects and sets the focus on the item with the requested string in the input table
    /// \param[in]          pProfileDataTable - the table to select the item at
    /// \param[in]          itemName - the item name
    /// \param[in]          column - the table column to look for the item at
    void selectTableItem(CPUProfileDataTable* pProfileDataTable, const QString& itemName, int column);

    /// \brief Name:        updateHint
    /// \brief Description: Updates the window hint with the requested text
    /// \param[in]          hint - the hint
    /// \return void
    void updateHint(const QString& hint);

    /// Process name -> PID:
    bool ProcessNameToPID(const QString& processName, ProcessIdType& pid);

    /// Update CLU note window content
    void UpdateNoteWindowContent(gtVector<float>& cluDataVector);
    QString GetNoteString(gtVector<float>& cluDataVector);
    void GetNoteList(gtVector<float>& cluDataVector, QList<CLU_NOTE>& noteList);

    /// Return the combo box for the action:
    /// \param pAction the action related to the combo box
    const QComboBox* TopToolbarComboBox(acWidgetAction* pAction);

    /// Return the label for the action:
    /// \param pAction the action related to the label
    const QLabel* TopToolbarLabel(acWidgetAction* pAction);

    /// update the display in a protected way that prevent closing the mdi
    /// \param updateType the type of update needs to be performed (see SettingsDifference for details):
    void ProtectedUpdateTableDisplay(unsigned int changeType);

protected slots:

    void OnDisplaySettingsClicked();
    void openCallGraphView();


protected:

    /// Tree item data:
    afApplicationTreeItemData* m_pDisplayedSessionItemData;

    QString m_exportString;
    acTreeCtrl* m_pList;

    /// m_indexOffset shows how many columns are reserved for the tab-specific data
    int m_indexOffset;

    QMenu* m_pMenu;
    QMenu* m_pColumnMenu;
    QVector<QPointer<QAction> > m_pColumnAction;
    QString m_selectText;
    int m_precision;    /// Should be read inside the onViewChanged function
    QString m_sessionDir;
    gtUInt64 m_shownTotal;
    QMutex m_displayMutex;


    /// Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);

    /// Overrides apIEventsObserver:
    virtual const wchar_t* eventObserverName() const { return L"DataTab"; };

    /// Information label & frame:
    QLabel* m_pHintLabel;
    QFrame* m_pHintFrame;

    /// Filter used for display:
    TableDisplaySettings* m_pDisplaySettings;

    /// Display settings link:
    acWidgetAction* m_pDisplaySettingsAction;
    acToolBar* m_pTopToolbar;
    CpuSessionWindow* m_pParentSessionWindow;

    std::shared_ptr<DisplayFilter>  m_pDisplayFilter = nullptr ;

    bool m_enableOnlySystemDllInDisplaySettings;

    /// This flag keeps the type of change needs to be performed once the view is displayed.
    /// See SettingsDifference for the change bitwise flag:
    unsigned int m_updateChangeType;

    /// CLU:
    bool m_isProfiledClu;
    QTextEdit* m_pNoteWidget;
    QLabel* m_pNoteHeader;
    QStringList m_CLUNoteStringList;

    /// View display information (selected line / table sort information):
    ProfileViewDisplayInformation m_userDisplayInformation;

    /// Stores the last focused widget, if it is related to the view:
    QWidget* m_pLastFocusedWidget;

    /// Holds the list of widgets that are relevant for edit actions:
    QList<QWidget*> m_editActionsWidgetsList;

    /// True iff we're in the update function (avoid multiple updates)
    bool m_isUpdatingData;

    // Profile Data Accessor
    std::shared_ptr<cxlProfileDataReader> m_pProfDataRdr = nullptr;
    std::map<gtString, AMDTUInt32> m_CounterIdxMap;
    bool m_isCLU = false;
};

#endif