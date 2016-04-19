//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdGlobalDebugSettingsPage.h
///
//==================================================================================

//------------------------------ gdGlobalDebugSettingsPage.h ------------------------------

#ifndef __GDGLOBALDEBUGSETTINGSPAGE_H
#define __GDGLOBALDEBUGSETTINGSPAGE_H

#include<QCheckBox>
#include<QLineEdit>
#include<QToolButton>
#include<QComboBox>
#include<QSpinBox>

// Infra:
#include <AMDTApplicationFramework/Include/afGlobalSettingsPage.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>

class GD_API gdGlobalDebugSettingsPage : public afGlobalSettingsPage
{
    Q_OBJECT

public:
    gdGlobalDebugSettingsPage();
    virtual ~gdGlobalDebugSettingsPage();

    // Must be overridden by child classes:
    virtual void initialize();

    // Names for the extension:
    virtual gtString pageTitle(); // Must be unique
    virtual gtString xmlSectionTitle(); // Can be shared between different pages

    // Load / Save the settings into a string:
    virtual bool getXMLSettingsString(gtString& projectAsXMLString);
    virtual bool setSettingsFromXMLString(const gtString& projectAsXMLString);

    virtual void loadCurrentSettings();

    // Restore the content to default settings:
    virtual void restoreDefaultSettings();

    // Save the data as it was changed in the widget to the specific settings manager (when "Ok" is pressed):
    virtual bool saveCurrentSettings();

protected slots:
    void handleValueChanged(int value);

private:

    // Call stack group:
    QCheckBox* m_pCollectAllocatedObjectsCreationCallStacks;

    // HTML Log File group:
    QCheckBox* m_pSaveTexturesToLogFileCheckBox;
    QComboBox* m_pLogFileTextureFileFormatComboBox;

    // API Logging group:
    QSpinBox* m_pOpenGLLoggedCallsMaxSpinBox;
    QSpinBox* m_pOpenCLLoggedCallsMaxSpinBox;
    QSpinBox* m_pMaxTreeItemsPerTypeSpinBox;

    // Advanced group:
    QCheckBox* m_pFlushLogAfterEachFunctionCheckBox;
};

#endif // __GDGLOBALDEBUGSETTINGSPAGE_H

