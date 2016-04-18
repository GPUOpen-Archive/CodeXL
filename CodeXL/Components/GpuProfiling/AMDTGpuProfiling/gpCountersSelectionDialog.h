//------------------------------ gpCountersSelectionDialog.h ------------------------------

#ifndef __GPCOUNTERSSELECTIONDIALOG_H
#define __GPCOUNTERSSELECTIONDIALOG_H


// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTApplicationComponents/Include/acDialog.h>
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>

// local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/gpTraceDataModel.h>
// Need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

// Forward declarations:
class acListCtrl;

/// A dialog that displays all available performance counters
/// allow user to select counters for display
class AMDT_GPU_PROF_API gpCountersSelectionDialog : public acDialog
{
    Q_OBJECT

public:
    gpCountersSelectionDialog(QString& selectedPreset);
    ~gpCountersSelectionDialog();

    QString NewPresetName() { return m_newPresetName; }

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
    void AddAvailableCounters(acTreeCtrl* pTreeCtrl);

    /// Add the selected preset to the initial selected counters
    void SetInitialSelectedCounters(QString& selectedPreset);

    /// add selected counter to selected counters tree
    /// @param[in]   counterID
    void AddToSelectedCounterTree(void* gpCounterData);

private:
    QPushButton* m_pAddButton;
    QPushButton* m_pRemoveButton;
    QPushButton* m_pRemoveAllButton;

    acTreeCtrl* m_pAvailableCounters;
    acTreeCtrl* m_pSelectedCounters;

    QLabel*     m_pDialogDescription;
    QTextEdit*  m_pCounterDescription;
    QLabel*     m_pAvailableCountersText;
    QLabel*     m_pSelectedCountersText;
    QTextEdit*  m_pNewPresetName;

    /// Contain true iff the previous and current selected counters list is different:
    bool m_wereSettingsChanged;

    // the added preset name
    QString m_newPresetName;
};

#endif  // __GPCOUNTERSSELECTIONDIALOG_H
