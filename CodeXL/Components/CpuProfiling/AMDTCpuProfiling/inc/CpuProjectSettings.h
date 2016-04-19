//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProjectSettings.h
/// \brief  The definition of the project settings dialog and framework extension
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/CpuProjectSettings.h#41 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
#ifndef _CPUPROJECTSETTINGS_H
#define _CPUPROJECTSETTINGS_H

// Qt:
#include <QtCore>
#include <QtWidgets>
#include <QWidget>

// Infra:
#include <AMDTApplicationFramework/Include/afProjectSettingsExtension.h>
#include <AMDTOSWrappers/Include/osMachine.h>

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

// ----------------------------------------------------------------------------------
// Class Name:           CpuProjectSettings : public afProjectSettingsExtension
// General Description:  This class is responsible for the project settings extension
// Author:  AMD Developer Tools Team
// Date:                 5/2/2012
// ----------------------------------------------------------------------------------
class CpuProjectSettings : public afProjectSettingsExtension
{
    Q_OBJECT

public:
    CpuProjectSettings();
    virtual ~CpuProjectSettings();

    // Overides afProjectSettingsExtension:
    virtual void Initialize();

    // Return the extension name:
    virtual gtString ExtensionXMLString();

    //Return the tab display
    virtual gtString ExtensionTreePathAsString();

    // Load / Save the project settings into a string:
    virtual bool GetXMLSettingsString(gtString& projectAsXMLString);
    virtual bool SetSettingsFromXMLString(const gtString& projectAsXMLString);
    virtual void RestoreDefaultProjectSettings();
    virtual bool AreSettingsValid(gtString& invalidMessageStr);

    /// Changes the GUI to reflect the current settings
    virtual bool RestoreCurrentSettings();

    // Get the data from the widget:
    virtual bool SaveCurrentSettings();

protected:

    // Builds the cores tree:
    void InitializeCoresTree();

    /// Set the core mask:
    /// \param afMask the mask
    void UpdateCoreTreeMask(gtUInt64 afMask);

    /// Update the check state of an item with the check state of its children:
    /// \param pTreeWidgetItem the item to update
    void UpdateTreeWidgetItemCheckState(QTreeWidgetItem* pTreeWidgetItem);
    void UpdateParentTreeWidgetItemCheckState(QTreeWidgetItem* pTreeWidgetItem);

    /// Check the current dialog settings, and see if CSS check box should be enabled:
    void EnableCSSCheckBox();

protected slots:

    /// Limits the affinity maximum
    void OnAffinityTextEdited(const QString& text);

    /// When executable path is changed:
    /// \param exePath the new executable file path
    /// \param isChangeFinal true iff the change is already set in the current project setting. Otherwise, it's only a GUI change that is not applied yet:
    void OnExecutableChanged(const QString& exePath, bool isChangeFinal, bool isUserModelId);

    /// When one the cores tree items is checked / unchecked:
    void OnCoresTreeItemChanged(QTreeWidgetItem* item, int column);

    /// Is handling the combo change of the profile type:
    void OnProfileTypeChanged(const QString& oldProfileType, const QString& newProfileType);

    /// Schedule radio buttons click:
    void OnScheduleRadioClick();

    /// When end data collection after check box is checked:
    void OnEndDataCollectionCheckBoxStateChanged();

    /// When the shared settings are updated:
    void OnSharedSettingsUpdated();

private:

    /// Persistent core count
    int m_coreCount;

    /// The maximum cores available for the processor group
    gtUInt64 m_maxCoreMask;

    // Profile schedule radios:
    QRadioButton* m_pEntireDurationRadio;
    QRadioButton* m_pProfilePausedRadio;
    QRadioButton* m_pProfileScheduledRadio;

    QLabel* m_pProfileType;
    QSpinBox* m_pStartAfterSpinBox;
    QSpinBox* m_pProfileDurationSpinBox;
    QLabel* m_pDurationLabel1;
    QLabel* m_pDurationLabel2;
    QCheckBox* m_pEndAfterCheckbox;
    QLabel* m_pDurationLabel4;
    QCheckBox* m_pTerminateAfterDataCollectionCheckBox;

    /// Widgets:
    QComboBox* m_pCodeExecInComboBox;
    QCheckBox* m_pCollectCSSCheckBox;
    QComboBox* m_pUnwindDepthComboBox;
    QComboBox* m_pOtherDepthComboBox;

    QSpinBox* m_pCollectionFrequencySpinBox;
    QLabel* m_pCSSLabel1;
    QLabel* m_pCSSLabel2;
    QLabel* m_pCSSLabel3;
    QLineEdit* m_pAffinityMaskText;
    QTreeWidget* m_pCoresTree;

    QLabel* m_pCSSScopeLabel;
    QCheckBox* m_pCSSFpoSupportCheckBox;
    QCheckBox* m_pCSSFpoOtherCheckBox;

    // widget will be a group for all pCssShiftRightLayout widgets
    QWidget* m_pCssRightLayoutWidget;

    bool m_isInitialized;

    /// String containing the latest user selection for profile type:
    QString m_currentProfileTypeStr;

    /// String containing the latest user selection for exe path:
    QString m_currentExePath;
};

class CPUTreeItem : public QTreeWidgetItem
{
public:
    CPUTreeItem() : QTreeWidgetItem() {};
    ~CPUTreeItem() {};

    gtUInt64 GetMask();
    void SetMask(gtUInt64 mask);
};

class CoreTreeItem : public QTreeWidgetItem
{
public:
    CoreTreeItem() : QTreeWidgetItem() {};
    ~CoreTreeItem() {};

    gtUInt64 m_mask;
};

#endif //_CPUPROJECTSETTINGS_H
