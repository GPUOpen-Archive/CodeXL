//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CounterSelectionSettingPage.h $
/// \version $Revision: #39 $
/// \brief  This file contains classes related to the counter selection options
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CounterSelectionSettingPage.h#39 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================
#ifndef _COUNTER_SELECTION_SETTING_PAGE_H
#define _COUNTER_SELECTION_SETTING_PAGE_H

#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include <QtCore>
#include <QtWidgets>
#include <AMDTBaseTools/Include/gtString.h>

#include <AMDTGpuProfiling/GPUProjectHandler.h>
#include <AMDTGpuProfiling/CounterManager.h>
#include <AMDTGpuProfiling/ProjectSettings.h>

#include <TSingleton.h>

// Infra:
#include <AMDTApplicationFramework/Include/afProjectSettingsExtension.h>

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool


/// the counter selection page
class CounterSelectionSettingWindow : public afProjectSettingsExtension, public GPUProjectHandler
{
    Q_OBJECT

public:

    /// singleton access
    /// \return the singleton value
    static CounterSelectionSettingWindow* Instance();

    /// destructor
    virtual ~CounterSelectionSettingWindow();

    /// Inherited function from framework to initialize page components
    virtual void Initialize();

    /// Gets the extension name:
    /// \return the extension name:
    virtual gtString ExtensionXMLString();

    /// Gets the extension display name (may contain spaces):
    /// \return the extension display name (may contain spaces):
    virtual gtString ExtensionTreePathAsString();

    /// Load the project settings into XML format into a string
    /// \param projectAsXMLString project setting string in XML format
    /// \returns true on success, else false
    virtual bool GetXMLSettingsString(gtString& projectAsXMLString);

    /// Save the project settings
    /// \param projectAsXMLString project setting string in XML format
    /// \returns true on success, else false
    virtual bool SetSettingsFromXMLString(const gtString& projectAsXMLString);

    /// Restore the control content to default settings:
    virtual void RestoreDefaultProjectSettings();

    /// Are current settings set in the widget valid?
    /// \param invalidMessageStr invalid string
    /// \return true on success, else false.
    virtual bool AreSettingsValid(gtString& invalidMessageStr);

    /// Get the data from the widget
    /// \returns true on success, else false.
    virtual bool SaveCurrentSettings();

    /// Set the current project data on the widgets
    /// \returns true on success, else false.
    virtual bool RestoreCurrentSettings();

    /// Create project setting string in XML format
    /// \param projectAsXMLString will contain final string
    /// \param projectPage  page name
    /// \returns true on success, else false
    bool getProjectSettingsXML(gtString& projectAsXMLString, gtString& projectPage);

    /// Save the project settings
    /// \param projectAsXMLString project setting string in XML format
    /// \returns true on success, else false
    bool setProjectSettingsXML(const gtString& projectAsXMLString);

    /// Write project settings into XML string format
    /// \param projectAsXMLString will contain final string
    /// \param type name of the type
    void writeSession(gtString& projectAsXMLString, const gtString& type);

    /// To update counter capture list
    void UpdateCounterCapture();

    /// The function checks if number of passes is 1
    /// \returns true if number of passes is 1
    bool IsSinglePassChecked();

    /// The function checks the state of GPU time collect checkbox
    /// \returns true if GpuTime collecting checkbox checked
    bool IsGpuTimeCollected();

    /// Returns true if at least one counter is selected.
    /// \returns true if at least one counter is selected, else false
    bool IsCountersSelected();

protected:
    virtual void showEvent(QShowEvent* pEvent);
    virtual void hideEvent(QHideEvent* pEvent);

private slots:

    /// Called when item changed in m_pCounterListTW.
    /// \param item TreeWidget item
    /// \param column of the item
    void TreeWidgetItem_Changed(QTreeWidgetItem* item, int column);

    /// Gets called on click of "Load Selection" button
    void LoadSelectionButtonClicked();

    /// Gets called on click of "Save Selection" button
    void SaveSelectionButtonClicked();

    /// Adjusts the HW counters UI to a remote session / non-remote session scenario.
    /// \param isRemoteSession indicates whether we need to adjust the UI to remote or
    /// non remote scenario.
    void RemoteSessionStatusChangedHandler(bool isRemoteSession);

    /// this function called on Gpu time collecting checkbox checked
    void GpuTimeCollectChecked();

    /// Called when the HSA / OpenCL radiobutton will get toggled.
    void OnAPITypeRadioButtonToggled();

private:

    CounterSelectionSettingWindow();
    void FillTreeWidgetStringList(QTreeWidget* tree, QStringList& strList);

    /// Select the maximal counters combination, to make it possible for HSA (single pass):
    void DisableCountersForSinglePass();

    /// Disable default copy constructor
    CounterSelectionSettingWindow(const CounterSelectionSettingWindow&);

    /// Disable assignment operator
    CounterSelectionSettingWindow& operator = (CounterSelectionSettingWindow&);

    /// Load counter names to the selected counters's list box.
    void LoadCaptureSettings();

    /// Load a counter file to the UI.
    /// \param strCounterFile The input counter filename
    /// \returns true if successful, false otherwise
    bool LoadCounterFile(const QString& strCounterFile);

    /// Save the current counter selections to a file.
    /// \param strCounterFile The counter filename
    /// \returns true if successful, false otherwise
    bool SaveCounterFile(QString strCounterFile);

    /// Sets the specified counter name to the specified check state
    /// \param strCounterName the counter name whose check state should be set
    /// \param checkState the state of the counter's check box (true==checked, false==unchecked)
    void SetCounterCheckState(const QString& strCounterName, bool checkState);

    /// Initializes the counter tree view -- occurs once per session
    void InitializeCounterTreeView(bool isRemoteSession);

    /// Update label to show the total selected counters
    void UpdateLabel();

    /// Update project settings with current settings
    void UpdateProjectSettings();

    /// Ensures that tree item widgets that have the same text as the specified are checked/unchecked
    /// \param item the item whose text to use when looking for identical items
    /// \param checked flag indicating whether to check/uncheck identical items
    void UpdateIdenticalCounters(QTreeWidgetItem* item, bool checked);

    /// Adds the counters for the specifed family to the tree control
    /// \param hardwareFamily the hardware family whose counter should be added to the tree
    void AddFamilyToTree(HardwareFamily hardwareFamily);

    /// Calls UpdateTreeTopLevelCheckState with m_pCounterListTW if tree has changed
    void UpdateCountersTreeCheckState();

    /// updates tree top level utem by the check state of the children
    /// \param the tree to be updated
    void UpdateTreeTopLevelCheckState(QTreeWidget* pTree);

    /// gets the sub tree check state
    /// \param parent item
    /// \returns the subtree check state
    Qt::CheckState GetItemCheckStateByChildrenState(QTreeWidgetItem* pParent);

    /// Validate text entered to m_pSpecificKernelsEdit
    /// Kernel name should consist of characters, digits and underscore but cannot start with a digit
    bool ValidateSpecificKernelsText();

    QRadioButton*                    m_pOpenCLRadioButton;                ///< Radiobutton for OpenCL applications
    QRadioButton*                    m_pHSARadioButton;                 ///< Radiobutton for HSA applications
    QLabel*                          m_pPerfCounterNotAvailableLabel;    /// GPU performance counters not available label
    QLabel*                          m_pNumOfCounterSelectedLB;          ///< Label to indicate number of counters selected
    QCheckBox*                       m_pGenerateOccupancyCB;             ///< CheckBox to generate kernel occupancy
    QCheckBox*                       m_pGpuTimeCollectCB;                ///< check box for GPU time collecting
    QTreeWidget*                     m_pCounterListTW;                   ///< TreeWidget have list of performance counters available
    QTreeWidget*                     m_pLastValidListTW;
    QPushButton*                     m_pLoadSelectionPB;                 ///< PushButton to load selected counters
    QPushButton*                     m_pSaveSelectionPB;                 ///< PushButton to save selected counters
    QGroupBox*                       m_pCounterLayoutGroup;              /// widgets group of tree + combo-boxes + load/save buttons
    QLabel*                          m_pProfileSpecificKernelLabel;
    QLabel*                          m_pProfileSpecificKernelDesc;
    QLineEdit*                       m_pSpecificKernelsEdit;
    QLabel*                          m_pNoteLabel;                        /// note label


#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    QCheckBox*                       m_pAdvancedInitXThread;             ///< check box for advanced option for XInitThread workaround
#endif

    QMap<QString, int>               m_numOfCountersSelected;            ///< Contains number of counters selected per top-level node (node by name) //is it correct to use node name here?
    QMap<HardwareFamily, QString>    m_hardwareFamilyTreeNodeMap;        ///< Map from HardwareFamily to tree-node label for that family's counters
    typedef QMap<QString, QTreeWidgetItem*> CounterMap;                  ///< Typedef used to map from counter name to tree node
    QMap<HardwareFamily, CounterMap> m_treeNodeMap;                      ///< Map from Hardware Family to a map of counter name to tree node for that family's counters
    bool                             m_singleHardwareFamily;             ///< Flag indicating whether or not there is one or more than on hardware family
    bool                             m_isRemoteSession;                  ///< Flag indicating whether or not the CounterSelectionSettingPage is in remote state
    CounterSettingOption             m_currentSettings;                  ///< Resources of counter selection setting page.
    bool                             m_isSinglePassCollect;                /// is single pass check
    unsigned int                     m_currentPassesCount;                /// current passes according to selected counters
    bool                             m_isPageOnDisplay;                  /// Flag is on only when counter selection setting page is on Show
    bool                             m_isCounterTreeUpdateNeeded;        /// Flag gets set when m_pCounterListTW is changed programmatically (not by user)

    static CounterSelectionSettingWindow* m_spInstance;                  /// static member singleton. Not using SINGLETON template since should not automatically delete at the end
    QStringList m_fullCounterNamesList;                                  /// a list of all existing counters
};

#endif //_COUNTER_SELECTION_SETTING_WINDOW_H
