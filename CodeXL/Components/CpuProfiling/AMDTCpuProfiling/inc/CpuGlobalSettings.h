//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuGlobalSettings.h
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/CpuGlobalSettings.h#12 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
#ifndef _CPUGLOBALSETTINGS_H
#define _CPUGLOBALSETTINGS_H

// Qt:
#include <QtCore>
#include <QtWidgets>
#include <QWidget>

// Infra:
#include <AMDTApplicationFramework/Include/afGlobalSettingsPage.h>

// Generated:
#include <tmp/ui/ui_iCpuGlobalSettings.h>

class afBrowseAction;

class CpuGlobalSettings : public afGlobalSettingsPage,
    public Ui_iCpuGlobalSettings
{
    Q_OBJECT

public:
    CpuGlobalSettings();
    virtual ~CpuGlobalSettings();

    // Inherited from afGlobalSettingsPage
    // Must be overridden by child classes:
    virtual void initialize();

    // Names for the extension:
    virtual gtString pageTitle(); // Must be unique
    virtual gtString xmlSectionTitle(); // Can be shared between different pages

    // Load / Save the settings into a string:
    virtual bool getXMLSettingsString(gtString& projectAsXMLString);
    virtual bool setSettingsFromXMLString(const gtString& projectAsXMLString);

    // Display the current settings in the page:
    virtual void loadCurrentSettings();

    // Restore the content to default settings:
    virtual void restoreDefaultSettings();

    // Save the data as it was changed in the widget to the specific settings manager (when "Ok" is pressed):
    virtual bool saveCurrentSettings();

protected slots:

    void onNewSymDir();
    void onRemoveSymDir();
    void onSymDirUp();
    void onSymDirDown();

    void onSearchSymDown();
    void onBrowseDebugDirs();
    void onSymServeItemChange(QListWidgetItem* pCurrent);

protected:
    void updateGui(bool bReset = false);

protected:

    /// Adding action for the symbol directory action, to make sure that last browsed folder is saved:
    afBrowseAction* m_pBrowseSymbolDirAction;
};

#endif //_CPUGLOBALSETTINGS_H
