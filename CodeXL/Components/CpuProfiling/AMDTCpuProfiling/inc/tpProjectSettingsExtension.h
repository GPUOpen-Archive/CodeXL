//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpProjectSettingsExtension.h
///
//==================================================================================

//------------------------------ tpProjectSettingsExtension.h ------------------------------

#ifndef __tpProjectSettingsExtension_H
#define __tpProjectSettingsExtension_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTApplicationFramework/Include/afProjectSettingsExtension.h>

// ----------------------------------------------------------------------------------
// Class Name:           tpProjectSettings
// General Description:  This class contain the data of the thread profile project settings
// ----------------------------------------------------------------------------------
class tpProjectSettings
{
public:
    QString m_dummyString;
};


// ----------------------------------------------------------------------------------
// Class Name:           tpProjectSettingsExtension : public afProjectSettingsExtension
// General Description:  This class is used for handling the thread profiling project settings
// ----------------------------------------------------------------------------------
class tpProjectSettingsExtension : public afProjectSettingsExtension
{
    Q_OBJECT

public:
    tpProjectSettingsExtension();
    virtual ~tpProjectSettingsExtension();

    /// Initialize the widget:
    virtual void Initialize();

    /// Return the extension name:
    virtual gtString ExtensionXMLString();

    /// Return the extension page title:
    virtual gtString ExtensionTreePathAsString();

    /// Load / Save the project settings into a string:
    virtual bool GetXMLSettingsString(gtString& projectAsXMLString);
    virtual bool SetSettingsFromXMLString(const gtString& projectAsXMLString);
    virtual void RestoreDefaultProjectSettings();
    virtual bool AreSettingsValid(gtString& invalidMessageStr);

    virtual bool RestoreCurrentSettings();

    /// Get the data from the widget:
    virtual bool SaveCurrentSettings();

private:

    /// Project settings widgets:
    QLineEdit* m_pDummyLineEdit;

    /// Current project settings:
    tpProjectSettings m_settings;

};

#endif //__tpProjectSettingsExtension_H

