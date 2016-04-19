//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afGlobalSettingsPage.h
///
//==================================================================================

#ifndef __AFGLOBALSETTINGSPAGE_H
#define __AFGLOBALSETTINGSPAGE_H

// Qt:
#include <QGroupBox>

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

class TiXmlNode;

class AF_API afGlobalSettingsPage : public QGroupBox
{
    Q_OBJECT

public:
    afGlobalSettingsPage();
    virtual ~afGlobalSettingsPage();

    // Must be overridden by child classes:
    virtual void initialize() = 0;

    // Names for the extension:
    virtual gtString pageTitle() = 0; // Must be unique
    virtual gtString xmlSectionTitle() = 0; // Can be shared between different pages

    // Load / Save the settings into a string:
    virtual bool getXMLSettingsString(gtString& projectAsXMLString) = 0;
    virtual bool setSettingsFromXMLString(const gtString& projectAsXMLString) = 0;

    // Display the current settings in the page:
    virtual void loadCurrentSettings() = 0;

    // Restore the content to default settings:
    virtual void restoreDefaultSettings() = 0;

    // Save the data as it was changed in the widget to the specific settings manager (when "Ok" is pressed):
    virtual bool saveCurrentSettings() = 0;

    /// Check if the page has valid data or the user need to take farther action
    virtual bool IsPageDataValid() { return true; }
};

#endif //__AFGLOBALSETTINGSPAGE_H

