//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/GeneralSettingWindow.h $
/// \version $Revision: #8 $
/// \brief :  This file contains General settings class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/GeneralSettingWindow.h#8 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#ifndef _GENERAL_SETTING_WINDOW_H_
#define _GENERAL_SETTING_WINDOW_H_

#ifdef _WIN32
    #pragma warning(push, 1)
#endif
#include <QRadioButton>
#include <QCheckBox>
#include <QtXml/qdom.h>
#ifdef _WIN32
    #pragma warning(pop)
#endif

#include <TSingleton.h>

// Infra:
#include <AMDTApplicationFramework/Include/afGlobalSettingsPage.h>


/// class for global settings (currently unused in CodeXL)
class GeneralSettingComponents
{
public:
    /// Constructor
    GeneralSettingComponents();

    bool m_alwaysDeleteSessionFile;  ///< Always delete session file
    bool m_neverDeleteSessionFile;   ///< Never delete session file
    bool m_askUserBeforeDelete;      ///< Ask before deletion of session file
    bool m_showDetailsOfDeletion;    ///< Show details of deleted files
};

/// UI for Global Settings page (currently unused in CodeXL)
class GeneralSettingWindow : public afGlobalSettingsPage, public TSingleton<GeneralSettingWindow>
{
    Q_OBJECT

    /// provide access to private members of the class
    friend class TSingleton<GeneralSettingWindow>;

public:
    /// Destructor
    virtual ~GeneralSettingWindow();

    /// Initialize the creator:
    /// This function must be implemented, and it should initialize the widget:
    virtual void initialize();

    /// Gets the Page Title for the extension
    /// \return the Page Title for the extension
    virtual gtString pageTitle();

    /// Gets the XML Section Title for the extension
    /// \return the XML Section Title for the extension
    virtual gtString xmlSectionTitle();

    /// Gets the XML Settings string for the extension
    /// \param[out] projectAsXMLString the XML Settings string for the extension
    /// \return true if successful, false otherwise
    virtual bool getXMLSettingsString(gtASCIIString& projectAsXMLString);

    /// Sets the XML Settings string for the extension
    /// \param[in] projectAsXMLString the XML Settings string for the extension
    /// \return true if successful, false otherwise
    virtual bool setSettingsFromXMLString(const gtASCIIString& projectAsXMLString);

    /// To load Current settings
    virtual void loadCurrentSettings();

    /// Restore the content to default settings:
    virtual void restoreDefaultSettings();

    /// Set the current project data on the widgets
    /// \return true if successful, false otherwise
    virtual bool restoreCurrentSettings();

    /// Save the data as it was changed in the widget to the specific settings manager (when "Ok" is pressed):
    /// \return true if successful, false otherwise
    virtual bool saveCurrentSettings();

private:
    /// hide constructor
    GeneralSettingWindow(QWidget* parent = 0);

    /// disable default copy ctor
    GeneralSettingWindow& operator = (const GeneralSettingWindow&);

    /// disable default assignment operator
    GeneralSettingWindow(const GeneralSettingWindow&);

    /// Creates project setting string in XML format
    /// \param projectAsXMLString will contain final string
    /// \param projectPage  page name
    /// \return true if successful, false otherwise
    bool getProjectSettingsXML(gtASCIIString& projectAsXMLString, gtString& projectPage);

    /// Sets project setting available in XML format
    /// \param projectAsXMLString will contain final string
    /// \return true if successful, false otherwise
    bool setProjectSettingsXML(const gtASCIIString& projectAsXMLString);

    /// Write project settings into XML string format
    /// \param projectAsXMLString will contain final string
    /// \param type name of the type
    void writeSession(gtASCIIString& projectAsXMLString, const gtASCIIString type);

    /// Write bool value in XML format
    /// \param projectAsXMLString Will have XML formate string
    /// \param key XML key
    /// \param value Boolean value of the XML key
    void writeBool(gtASCIIString& projectAsXMLString, const gtASCIIString& key, const bool value);

    /// Write value in XML format
    /// \param projectAsXMLString Will have XML formate string
    /// \param key XML key
    /// \param value Value of the XML key
    void writeValue(gtASCIIString& projectAsXMLString, const gtASCIIString& key, const gtASCIIString& value);

    /// Update global settings
    void UpdateGlobalSettings();

    GeneralSettingComponents m_currentSettings;  ///< Contains current setting information of the window

    const gtASCIIString m_constStrAlwaysDeleteSessionFile;  ///< Constant string for always delete session files
    const gtASCIIString m_constStrNeverDeleteSessionFile;   ///< Constant string for never delete session files
    const gtASCIIString m_constStrAskUserBeforeDelete;      ///< Constant string for ask user before delete
    const gtASCIIString m_constStrShowDetailsOfDeletion;    ///< Constant string to show details of deletion

    QRadioButton* m_pAlwaysDeleteSessionFileRB; ///< RadioButton to delete session files
    QRadioButton* m_pNeverDeleteSessionFileRB;  ///< RadioButton to never delete session files
    QRadioButton* m_pAskUserBeforeDeleteRB;     ///< RadioButton to ask user before deletion
    QCheckBox*    m_pShowDetailsOfDeletionCB;   ///< CheckBox to show details of deleted files

private slots:
    /// Gets called once RadioButton gets toggled
    /// \param isChecked flag indicating whether or not the button is checked
    void RadioButtonToggled(bool isChecked);
};


#endif//_GENERAL_SETTING_WINDOW_H_
