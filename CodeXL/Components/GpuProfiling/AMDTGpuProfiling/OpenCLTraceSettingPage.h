//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/OpenCLTraceSettingPage.h $
/// \version $Revision: #23 $
/// \brief  This file contains OpenCL Trace settings by the GPU Profiler
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/OpenCLTraceSettingPage.h#23 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================
#ifndef _OPENCL_TRACE_SETTING_PAGE_H_
#define _OPENCL_TRACE_SETTING_PAGE_H_

#ifdef _WIN32
    #pragma warning(push, 1)
#endif
#include <QtCore>
#include <QtWidgets>
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Infra:
#include <AMDTApplicationFramework/Include/afProjectSettingsExtension.h>

#include <TSingleton.h>

#include "GPUProjectHandler.h"
#include "ProjectSettings.h"


/// Project Settings page for OpenCL Tracing options
class OpenCLTraceOptions : public afProjectSettingsExtension, public GPUProjectHandler
{
    Q_OBJECT

public:

    /// Destructor
    virtual ~OpenCLTraceOptions();

    /// singleton access
    /// \return the singleton value
    static OpenCLTraceOptions* Instance();

    /// Inherited function from framework to initialize page components
    virtual void Initialize();

    /// Gets the extension name
    /// \return the extension name
    virtual gtString ExtensionXMLString();

    /// Gets the extension display name (may contain spaces):
    /// \return the extension display name (may contain spaces):
    virtual gtString ExtensionTreePathAsString();

    /// Load the project settings into XML format into a string
    /// \param projectAsXMLString project setting string in XML format
    /// \return true on success, else false
    virtual bool GetXMLSettingsString(gtString& projectAsXMLString);

    /// Save the project settings
    /// \param projectAsXMLString project setting string in XML format
    /// \return true on success, else false
    virtual bool SetSettingsFromXMLString(const gtString& projectAsXMLString);

    /// Restore the control content to default settings
    virtual void RestoreDefaultProjectSettings();

    /// Set the current project data on the widgets
    /// \return true on success, else false.
    virtual bool RestoreCurrentSettings();

    /// Are current settings set in the widget valid?
    /// \param invalidMessageStr invalid string
    /// \return true on success, else false.
    virtual bool AreSettingsValid(gtString& invalidMessageStr);

    /// Get the data from the widget
    /// \returns true on success, else false.
    virtual bool SaveCurrentSettings();

    /// Create project setting string in XML format
    /// \param projectAsXMLString will contain final string
    /// \param projectPage  page name
    /// \return true on success, else false
    bool getProjectSettingsXML(gtString& projectAsXMLString, gtString& projectPage);

    /// Write project settings into XML string format
    /// \param projectAsXMLString will contain final string
    /// \param type name of the type
    void writeSession(gtString& projectAsXMLString, const gtString& type);

    /// Will update Trace options with user selected TraceOptions
    /// \param[out] apiTraceOptions update with user selected Trace options
    /// \return True on success, else false
    bool SetTraceOptions(APITraceOptions& apiTraceOptions);

    /// Save selected rules
    /// \param filePath Will contain selected rules
    /// \return True on success, else false
    bool GenerateRules(const QString& filePath);

private:
    /// hidden constructor to initialize the singleton object
    /// \param parent the parent widget
    OpenCLTraceOptions(QWidget* parent = 0);

    /// Will initialize Rules TreeView
    void InitializeRulesTreeView();

    /// Will initialize API filter TreeView
    void InitializeAPIFilterTreeView();

    /// Adds all the OpenCL function to the API tree:
    void AddCLAPIFunctionsToTree();

    /// Adds all the HSA function to the API tree:
    void AddHSAAPIFunctionsToTree();

    /// Update OpenCL API filter list
    void UpdateAPIFilterList();

    /// Update Rules list
    void UpdateRuleList();

    /// Update project settings
    void UpdateProjectSettings();

    /// Sets project setting available in XML format
    /// \param projectAsXMLString will contain final string
    /// \returns true on success, else false
    bool setProjectSettingsXML(const gtString& projectAsXMLString);

    /// Will handle change event for TreeWidget item
    /// \param topOfTree Top level tree widget
    /// \param item changed item
    /// \param column column of the tree
    /// \param topOfTreeString top of the tree string.
    void HandleTreeWidgetItemChangeEvent(QTreeWidget* topOfTree, QTreeWidgetItem* item, int column, QString topOfTreeString);

    /// Restore the API and rules trees items check state from saved settings
    void RestoreTreesDataFromSettings();


    /// fix tree items top levels state
    /// \param the tree to be fixed
    void UpdateTreeCheckState(QTreeWidget* tree);

    /// Fix recursively sub tree state
    /// \param root item for sub tree
    /// \returns Check State for sub tree parent
    Qt::CheckState GetSubTreeCheckState(QTreeWidgetItem* parent);

    /// Show / hide the relevant tree items for the selected API type:
    /// \param apiType the user selected api type
    void FitTreesToAPIType(APIToTrace api);

    QRadioButton*              m_pOpenCLRadioButton;                ///< Radiobutton for OpenCL applications
    QRadioButton*              m_pHSARadioButton;                 ///< Radiobutton for HSA applications
    QCheckBox*                 m_pShowAPIErrorCodeCB;               ///< CheckBox for "Always show API error code"
    QCheckBox*                 m_pCollapseCallsCB;                  ///< CheckBox for collapse calls
    QCheckBox*                 m_pEnableNavigationCB;               ///< CheckBox for navigate to source code
    QTreeWidget*               m_pAPIRulesTW;                       ///< TreeWidget to show rules
    QTreeWidget*               m_pAPIsToTraceTW;                    ///< TreeWidget to show list of OpenCL APIs
    QRadioButton*              m_pAPIsToTraceRB;                    ///< Radio button for filter APIs
    QRadioButton*              m_pGenerateSummaryPageRB;            ///< RadioButton for generate summary pages
    QLabel*                    m_pMaxNumberOfAPIsLB;                ///< Label for maximum number of APIs
    QSpinBox*                  m_pMaxNumberOfAPIsSB;                ///< SpinBox for maximum number of APIs to be traced
    QCheckBox*                 m_pWriteDataTimeOutCB;               ///< CheckBox for write data TimeOut (shown on Windows only)
    QLabel*                    m_pTimeoutIntervalLB;                ///< Label for timeout interval spinbox (shown on Linux only)
    QSpinBox*                  m_pTimeOutIntervalSB;                ///< SpinBox for timeout interval
    QCheckBox*                 m_pGenerateOccupancyCB;              ///< CheckBox for generate occupancy data

    QTreeWidgetItem* m_pOpenCLRootItem;                             ///< The top level item representing the CL API
    QTreeWidgetItem* m_pHSARootItem;                                ///< The top level item representing the HSA API

    APITraceOptions m_currentSettings;                              ///< Resource class for OpenCL trace page

    QVector<QTreeWidgetItem*> m_hsaRulesTreeWidgetItems;            ///< Holds a vector of tree items to show when the HSA api is selected

    static OpenCLTraceOptions* m_spInstance;                       /// static member singleton. Not using SINGLETON template since should not automatically delete at the end

private slots:
    /// Called when item changed in RuleTreeWidget.
    /// \param item TreeWidget item
    /// \param column of the item
    void RulesTreeItemChanged(QTreeWidgetItem* item, int column);

    /// Called when item changed in OpenCL APIsTreeWidget.
    /// \param item TreeWidget item
    /// \param column of the item
    void APIsTreeItemChanged(QTreeWidgetItem* item, int column);

    /// Called when radiobutton will get toggled.
    /// \param isChecked will have radiobutton state
    void RadioButtonToggled(bool isChecked);

    /// Called when the HSA / OpenCL radiobutton will get toggled.
    void OnAPITypeRadioButtonToggled();

    /// Called when timeout checkbox state changes
    /// \param state will indicate the current state of the checkbox
    void WriteTimeOutStateChanged(int state);

};


#endif //_OPENCL_TRACE_SETTING_PAGE_H_
