//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SharedProfileSettingPage.h
///
//==================================================================================


//------------------------------ SharedProfileSettingPage.h ------------------------------

#ifndef __AFMULTIPLEDIRECTORIESBROWSEDIALOG
#define __AFMULTIPLEDIRECTORIESBROWSEDIALOG

// Qt:
#include <QtCore>
#include <QtGui>

// Infra:
#include <AMDTApplicationFramework/Include/afProjectSettingsExtension.h>

#include <TSingleton.h>

// local
#include "LibExport.h"
#include <AMDTSharedProfiling/inc/SessionTreeNodeData.h>

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

/// Project Settings page for OpenCL Tracing options
class AMDTSHAREDPROFILING_API SharedProfileSettingPage : public afProjectSettingsExtension
{
    Q_OBJECT

public:

    /// Destructor
    virtual ~SharedProfileSettingPage();

    /// Inherited function from framework to initialize page components
    virtual void Initialize();

    static SharedProfileSettingPage* Instance();

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

    // The function should return true iff the project contains data like sessions or build outputs
    /// \param projectName the project for which the saved data on disk should be searched
    /// \param typeOfProjectSavedData[out] the type of data that is saved for the requested project name
    /// \return true iff the project contain data that is saved on disk
    virtual bool DoesProjectContainData(const gtString& projectName, gtString& typeOfProjectSavedData);

    /// Returns the current settings:
    SessionTreeNodeData& CurrentSharedProfileSettings() {return m_currentSettings;};

    /// Adds a profile type to the page:
    void AddProfileType(const QString& profileType);

    /// returns true is the System Wide radio button are set to system wide
    bool IsSystemWideRadioButtonChecked();

    /// return true if we in after project loading
    /// \returns the value of isInRestoreDefaultSettings
    bool GetAfetrProjectLoadedStatus() const { return m_isAfetrProjectLoaded; };

    /// sets the m_isAfetrProjectLoaded member
    /// \param isLoaded value to be set
    void SetAfterProjectLoadedStatus(bool isLoaded) { m_isAfetrProjectLoaded = isLoaded; };

public slots:

    void OnProfileTypeChanged(const gtString& profileType);

signals:

    /// Profile type had changed:
    void ProfileTypeChanged(const QString& oldProfileType, const QString& newProfileType);

    /// Is emited after the XML is loaded:
    void SharedSettingsUpdated();

protected slots:

    /// When executable is changed:
    void OnExecutableChanged(const QString& exePath, bool isChangeFinal, bool isUserModelId);
    void OnProfileTypeChanged(const QString& currentText);
private:

    /// hidden constructor to initialize the singleton object
    /// \param parent the parent widget
    SharedProfileSettingPage();

    static SharedProfileSettingPage* m_psMySingleInstance;

private:

    /// Contain the current shared profile settings:
    SessionTreeNodeData m_currentSettings;

    /// Widgets:
    QComboBox* m_pProfileTypeCombo;

    // Profile scope radio buttons:
    QRadioButton* m_pSingleApplicationRadioButton;
    QRadioButton* m_pSystemWideRadioButton;
    QRadioButton* m_pSystemWideWithFocusRadioButton;

    // Profile type description:
    QLabel* m_pProfileTypeDescription;
    QMap<QString, QString> m_profileTypeToDescriptionMap;

    /// Contain the previous profile type - is used by other profile pages:
    QString m_previousProfileType;

    /// is true if project loaded already
    bool m_isAfetrProjectLoaded;

    /// A list of extensions for profile sessions
    static QList<gtString> m_profileSessionFileTypes;

    /// A list of extensions for frame analyze sessions
    static QList<gtString> m_frameAnalyzeSessionFileTypes;
};


#endif //__AFMULTIPLEDIRECTORIESBROWSEDIALOG
