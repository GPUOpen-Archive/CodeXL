//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppProjectSettingsExtension.h
///
//==================================================================================

//------------------------------ ppProjectSettingsExtension.h ------------------------------

#ifndef __PPPROJECTSETTINGSEXTENSION_H
#define __PPPROJECTSETTINGSEXTENSION_H

// Qt:
#include <QtWidgets>

#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTApplicationFramework/Include/afProjectSettingsExtension.h>


// ----------------------------------------------------------------------------------
// Class Name:           ppProjectSettingsExtension : public afProjectSettingsExtension
// General Description:  This class is used for handling the project settings
// Author:               Gilad Yarnitzky
// Date:                 25/8/2014
// ----------------------------------------------------------------------------------
class PP_API ppProjectSettingsExtension : public afProjectSettingsExtension
{
    Q_OBJECT

public:
    ppProjectSettingsExtension();
    virtual ~ppProjectSettingsExtension();

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

    // Project settings widgets:
    QSpinBox* m_pSamplingIntervalSpinBox;

};

#endif //__ppProjectSettingsExtension_H

