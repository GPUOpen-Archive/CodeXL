//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afProjectSettingsExtension.h
///
//==================================================================================

#ifndef __AFPROJECTSETTINGSEXTENSION_H
#define __AFPROJECTSETTINGSEXTENSION_H

// Qt:
#include <QGroupBox>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>

// Infra:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          AF_API afProjectSettingsExtension
// General Description: abstract class for an object that extends the framework settings
// Author:              Sigal Algranaty
// Creation Date:       4/4/2012
// ----------------------------------------------------------------------------------
class AF_API afProjectSettingsExtension : public QGroupBox
{
    Q_OBJECT

public:

    afProjectSettingsExtension();
    virtual ~afProjectSettingsExtension();

    // Initialize the creator:
    // This function must be implemented, and it should initialize the widget:
    virtual void Initialize() = 0;

    // Return the extension name:
    virtual gtString ExtensionXMLString() = 0;

    // Return the extension display name (may contain spaces):
    virtual gtString ExtensionTreePathAsString() = 0;

    // Load / Save the project settings into a string:
    virtual bool GetXMLSettingsString(gtString& projectAsXMLString) = 0;
    virtual bool SetSettingsFromXMLString(const gtString& projectAsXMLString) = 0;

    // Restore the control content to default settings:
    virtual void RestoreDefaultProjectSettings() = 0;

    // Are current settings set in the widget are valid?
    virtual bool AreSettingsValid(gtString& invalidMessageStr) = 0;

    // Get the data from the widget:
    virtual bool SaveCurrentSettings() = 0;

    // Set the current extension settings to the controls:
    // This function is called after the cancel button is clicked, and it is used for canceling the user operations:
    virtual bool RestoreCurrentSettings() = 0;

    // should the QGroupBox be added to the setting dialog (in case it is not empty box)
    virtual bool ShouldAddToProjectSettingDialog() { return true; }

    // The function should return true iff the project contains data like sessions or build outputs
    /// \param projectName the project for which the saved data on disk should be searched
    /// \param typeOfProjectSavedData[out] the type of data that is saved for the requested project name
    /// \return true iff the project contain data that is saved on disk
    virtual bool DoesProjectContainData(const gtString& projectName, gtString& typeOfProjectSavedData) { GT_UNREFERENCED_PARAMETER(projectName); GT_UNREFERENCED_PARAMETER(typeOfProjectSavedData); return false; }
};

#endif //__AFPROJECTSETTINGSEXTENSION_H

