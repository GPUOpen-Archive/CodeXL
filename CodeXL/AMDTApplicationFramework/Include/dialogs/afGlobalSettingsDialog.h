//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afGlobalSettingsDialog.h
///
//==================================================================================

#ifndef __AFGLOBALSETTINGSDIALOG_H
#define __AFGLOBALSETTINGSDIALOG_H

// Qt:
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTabWidget>
#include <QToolButton>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          AF_API afGlobalSettingsDialog : public QDialog
// General Description: A dialog used to set the global settings per extension and the
//                      general global settings
// Author:              Uri Shomroni
// Creation Date:       19/4/2012
// ----------------------------------------------------------------------------------
class AF_API afGlobalSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    static afGlobalSettingsDialog& instance();
    virtual ~afGlobalSettingsDialog();

    void showDialog(const gtString& extensionActivePageName = L"");

protected slots:
    void onOkButton();
    void onRestoreDefaultSettings();

    // General settings page actions:
    void onBrowseForLogFilesButton();
    void onProxyCheckBox(int newState);
    void onFPPrecisionChange(int value);

private:
    // Only the instance method is allowed to create me:
    afGlobalSettingsDialog();

    void createDialogLayout();
    void createGeneralPage();
    void fillGeneralPageData();
    void initDialogGlobalSettings();
    void setActivePage(const gtString& extensionName);
    void createRemoteControls(QGroupBox* pRemoteSettingsGroupBox);

private:
    // Only afSingletonsDelete can delete my instance:
    friend class afSingletonsDelete;

private:
    static afGlobalSettingsDialog* m_spMySingleInstance;

    // Dialog parts:
    QTabWidget* m_pMainTabWidget;
    QGroupBox* m_pGeneralPage;

    // Global settings page:
    QComboBox* m_pLogLevelComboBox;
    QLineEdit* m_pLogFilesPathLineEdit;
    QToolButton* m_pLogFilesPathButton;
    QCheckBox* m_pUsingProxyCheckBox;
    QLineEdit* m_pProxyAddress;
    QSpinBox* m_pProxyPort;
    QSpinBox* m_pFloatingPointPrecisionSpinBox;

    // Remote GPU debugging ports.
    QSpinBox* m_pSpyAPIPortSpinBox;
    QSpinBox* m_pSpyEventsPortSpinBox;
    QSpinBox* m_pRDSPortSpinBox;
    QSpinBox* m_pRDSEventsPortSpinBox;

    // Source files:
    QCheckBox* m_pAlertMissingSourceFile;
};

#endif //__AFGLOBALSETTINGSDIALOG_H

