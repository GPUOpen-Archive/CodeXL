//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppCountersSelectionDialog.h
///
//==================================================================================

//------------------------------ ppCountersSelection.h ------------------------------

#ifndef __PPCOUNTERSSELECTION
#define __PPCOUNTERSSELECTION


// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTApplicationComponents/Include/acDialog.h>
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>

// local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTPowerProfilingMidTier/include/PowerProfilerCore.h>

// Need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

// Forward declarations:
class acListCtrl;


/// A dialog that displays all available performance counters
/// allow user to select counters for display
class PP_API ppCountersSelectionDialog : public acDialog
{
    Q_OBJECT

public:
    ppCountersSelectionDialog(QWidget* pParent);
    ~ppCountersSelectionDialog();

    /// Create a new counter selection dialog and displays it
    /// @param[in]  category - when opening dialog, focus available counters tree on category specified
    static bool OpenCountersSelectionDialog(AMDTPwrCategory category = AMDTPwrCategory(0));

    void SetIntialCounterCategory(AMDTPwrCategory category);


protected slots:
    /// OnOk: update the selected counters
    void OnOk();

    /// Add counters to selected counters tree ctrl
    void OnAdd();

    /// Remove counters from selected counters tree ctrl
    void OnRemove();

    /// Clear selected counters tree ctrl
    void OnRemoveAll();

    /// Update description for the counter focused and refresh buttons state
    void OnLTSelectionChanged();

    /// Refresh buttons state
    void OnRTSelectionChanged();

    /// Act like Add button only when on nodes
    void OnLTItemDoubleClick();

    /// Act like Remove button only when on nodes
    void OnRTItemDoubleClick();

private:

    /// Create the dialog and all its ctrls
    void SetDialogLayout();

    ///  loads both available counters and selected counters trees will all counters
    void PopulateCountersDisplay();

    /// enable / disable buttons
    void SetButtonStates();

    /// Go over selected counters tree, set category root to be visible when at list one counter is selected
    void SetSelectedCountersRootsVisibility();

    /// Set only selected counter to be visible
    void LoadSelectedCounters();

    /// add selected counter to selected counters tree
    /// @param[in]   counterID
    void AddToSelectedCounterTree(int counterId);

private:
    QGroupBox* m_pMainGroupBox;
    QVBoxLayout* m_pTopLayoutV;
    QHBoxLayout* m_pMainLayoutH;
    QVBoxLayout* m_pCenterButtonsLayoutV;
    QVBoxLayout* m_pRightLayoutV;
    QVBoxLayout* m_pLeftLayoutV;
    QHBoxLayout* m_pBottomButtonsLayoutH;

    QPushButton* m_pAddButton;
    QPushButton* m_pRemoveButton;
    QPushButton* m_pRemoveAllButton;

    acTreeCtrl* m_pAvailableCountersTW;
    acTreeCtrl* m_pSelectedCountersTW;

    QLabel*  m_pDialogDescription;
    QTextEdit*  m_pCounterDescription;
    QLabel*  m_pAvailableCountersText;
    QLabel*  m_pSelectedCountersText;

    /// Used to save APU Power counter id as we only know the counter by name, counter needs special handling - cannot be removed
    int m_apuPowerCounterId;

    /// Used to save dGPU Power first counter id as we only know the counter by name, counter needs special handling - cannot be removed
    int m_dgpuPowerCounterId;

    /// Contain true iff the previous and current selected counters list is different:
    bool m_wereSettingsChanged;

};

#endif  // __PPCOUNTERSSELECTION
