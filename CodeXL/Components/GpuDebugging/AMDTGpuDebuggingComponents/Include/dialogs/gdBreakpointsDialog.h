//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdBreakpointsDialog.h
///
//==================================================================================

//------------------------------ gdBreakpointsDialog.h ------------------------------

#ifndef __GDBREAKPOINTSDIALOG
#define __GDBREAKPOINTSDIALOG

// Forward declarations:
class acListCtrl;
class acQTextFilterCtrl;

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apGenericBreakpoint.h>
#include <AMDTApplicationComponents/Include/acDialog.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>
#include <AMDTGpuDebuggingComponents/Include/gdBreakpointsItemData.h>

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

class acListCtrl;
class acQTextFilterCtrl;

// ----------------------------------------------------------------------------------
// Class Name:          GD_API gdBreakpointsDialog : public QDialog
// General Description: This is the coolest class ever written!
//
// Author:              Yoni Rabin
// Creation Date:       13/6/2012
// ----------------------------------------------------------------------------------
//class GD_API gdBreakpointsDialog : public acDialog
class GD_API gdBreakpointsDialog : public acDialog
{
    Q_OBJECT

public:
    gdBreakpointsDialog(QWidget* pParent);
    ~gdBreakpointsDialog();

protected slots:
    void onOk();
    void onAdd();
    void onChosenBreakpointDoubleClicked(QTableWidgetItem* pItem);
    void onRemove();
    void onRemoveAll();
    void onSelectDeselectAll(bool checked);
    void onChosenListKeyDown(QKeyEvent* pEvent);
    void onBreakpointsFilterTextChanged(const QString& filterText);
    void onFunctionsFilterFocused(bool hasFocus);
    void onAfterRemoveRow(int row);
    void onChosenItemChanged(QTableWidgetItem* pItem);
    void onLeftListSelectionChanged();
    void onRightListSelectionChanged();
    void onTabChanged(int index);
    void onFocusChange(QWidget* pOldItem, QWidget* pNewItem);

protected:
    void onListSelectionChanged(acListCtrl* pCallingList);
    bool initAPIFunctionsList();
    bool initGenericBreakpointsList();

    void setDialogActiveBreakpoints();
    void colorActiveBreakpointsInLeftListCtrl();
    void addActiveBreakpointsToRightListCtrl();
    void removeSelectedBreakpoint(QTableWidgetItem* pItem);
    void setDialogLayout();
    void setInitialValues();
    void buildFinalLayout();
    void verifyChosenListLastRow();

    // Internal utilities:
    unsigned int getMonitoredFunctionsFilterByCurrentProjectType();
    bool isBreakpointChosen(const gdBreakpointsItemData* pBrekapointItemData, QTableWidgetItem*& pRetItem);
    void addBreakpointToRightListCtrl(const gdBreakpointsItemData* pBreakpointItemData, gtString& breakpointName, bool checkStatus);
    void updateSelectAllCheckBoxStatus(bool ignoreLastItem = true);
    gdBreakpointsItemData* findBreakpointMatchingItemData(gtAutoPtr<apBreakPoint>& aptrBreakpoint);
    QTableWidgetItem* findBreakpointMatchingItem(gtAutoPtr<apBreakPoint>& aptrBreakpoint);
    void colorActiveKernelInLeftListCtrl();
    gdBreakpointsItemData* getBreakpointData(QTableWidgetItem* pItem);
    void getBreakpointTypeString(gdBreakpointsItemData* pNewBreapointData, QString& typeString);

private:
    void resetListColor(acListCtrl* pList, QColor col = Qt::black);
    QTableWidgetItem* addListItem(acListCtrl* pList, const gtString& name, gdBreakpointsItemData* pItemData);
    QTableWidgetItem* addChosenListItem(const QString& name, const QString& type, gdBreakpointsItemData* pItemData, bool checked = false);
    acListCtrl* getActiveList();
    void setButtonStates();

private:
    QGroupBox* m_pMainGroupBox;
    QVBoxLayout* m_pTopLayoutV;
    QHBoxLayout* m_pMainLayoutH;
    QVBoxLayout* m_pCenterButtonsLayoutV;
    QVBoxLayout* m_pRightLayoutV;
    QVBoxLayout* m_pAPILayoutV;
    QVBoxLayout* m_pBreakpointsLayoutV;
    QHBoxLayout* m_pBottomButtonsLayoutH;

    QPushButton* m_pAddButton;
    QPushButton* m_pRemoveButton;
    QPushButton* m_pRemoveAllButton;

    QCheckBox* m_pCheckBox;

    // A list containing all possible breakpoints (left list):
    acListCtrl* m_pAPIList;
    // List of all generic breakpoints:
    acListCtrl* m_pGenericBreakpointsList;
    // The right list:
    acListCtrl* m_pChosenList;
    // Displays the Functions text filter:
    acQTextFilterCtrl* m_pFunctionsFilter;
    acQTextFilterCtrl* m_pKernelFilter;
    // Text Labels:
    QLabel*  m_pDescription;
    QLabel*  m_pChosenListText;
    // Tabs:
    QTabWidget* m_pTabs;
    QWidget* m_pAPITab;
    QWidget* m_pBreakpointsTab;
    // A pointer to the last row:
    QTableWidgetItem* m_pLastChosenRow;
    // Last row on m_pChosenList is in edit mode
    bool m_LastChosenRowOnEdit;

    // Vector of breakpoints data to delete:
    gtPtrVector<gdBreakpointsItemData*> m_rightListBreakpointsDataVector;
    // Lock while updating check boxes:
    bool m_updatingCheckStatus;
    // Helper vars:
    int m_amountOfMonitoredFunctions;
};

#endif  // __GDBREAKPOINTSDIALOG
