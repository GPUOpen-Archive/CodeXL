//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afGlobalSettingsDialog.cpp
///
//==================================================================================

// Warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apBasicParameters.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>

// Local:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/dialogs/afGlobalSettingsDialog.h>
#include <AMDTApplicationFramework/Include/afAidFunctions.h>

// Static member initialization:
afGlobalSettingsDialog* afGlobalSettingsDialog::m_spMySingleInstance = nullptr;

// Min Max and default values for the spin controls:
#define AF_OPTIONS_FLOATING_POINT_MIN_PRECISION 1
#define AF_OPTIONS_FLOATING_POINT_MAX_PRECISION 50
#define AF_OPTIONS_FLOATING_POINT_INIT_PRECISION_VALUE AP_DEFAULT_FLOATING_POINT_PRECISION


// Dialog size:
#define AF_GLOBAL_SETTINGS_DIALOG_WIDTH 600
#define AF_GLOBAL_SETTING_DIALOG_HEIGHT 400

// Default Values:
#define AF_PROXY_EXAMPLE_ADDRESS "www.myproxy.example"
#define AF_PROXY_EXAMPLE_PORT 8080

// ---------------------------------------------------------------------------
// Name:        afGlobalSettingsDialog::afGlobalSettingsDialog
// Description: Constructor
// Author:      Uri Shomroni
// Date:        19/4/2012
// ---------------------------------------------------------------------------
afGlobalSettingsDialog::afGlobalSettingsDialog()
    : QDialog(afMainAppWindow::instance()), m_pMainTabWidget(nullptr), m_pGeneralPage(nullptr), m_pFloatingPointPrecisionSpinBox(nullptr), m_pAlertMissingSourceFile(nullptr)
{
    // Create the dialog layout:
    createDialogLayout();

    // Read current settings:
    initDialogGlobalSettings();

    // Set my size:
    resize(AF_GLOBAL_SETTINGS_DIALOG_WIDTH, AF_GLOBAL_SETTING_DIALOG_HEIGHT);
}

// ---------------------------------------------------------------------------
// Name:        afGlobalSettingsDialog::~afGlobalSettingsDialog
// Description: Destructor
// Author:      Uri Shomroni
// Date:        19/4/2012
// ---------------------------------------------------------------------------
afGlobalSettingsDialog::~afGlobalSettingsDialog()
{

}

// ---------------------------------------------------------------------------
// Name:        afGlobalSettingsDialog::instance
// Description: Returns the singleton instance of this class
// Author:      Uri Shomroni
// Date:        22/4/2012
// ---------------------------------------------------------------------------
afGlobalSettingsDialog& afGlobalSettingsDialog::instance()
{
    if (nullptr == m_spMySingleInstance)
    {
        m_spMySingleInstance = new afGlobalSettingsDialog;
    }

    return *m_spMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        afGlobalSettingsDialog::showDialog
// Description: Show the dialog
// Author:      Uri Shomroni
// Date:        19/4/2012
// ---------------------------------------------------------------------------
void afGlobalSettingsDialog::showDialog(const gtString& extensionActivePageName)
{
    initDialogGlobalSettings();
    // Select the General page:
    m_pMainTabWidget->setCurrentIndex(0);

    setActivePage(extensionActivePageName);

    afApplicationCommands::instance()->showModal(this);
}

// ---------------------------------------------------------------------------
// Name:        afGlobalSettingsDialog::onOkButton
// Description: Called when the user presses the "Ok" button
// Author:      Uri Shomroni
// Date:        23/4/2012
// ---------------------------------------------------------------------------
void afGlobalSettingsDialog::onOkButton()
{
    // Set the global page settings:
    GT_IF_WITH_ASSERT((nullptr != m_pLogFilesPathLineEdit) && (nullptr != m_pLogLevelComboBox) && (nullptr != m_pLogFilesPathLineEdit) &&
                      (nullptr != m_pProxyAddress) && (nullptr != m_pProxyPort) && (m_pFloatingPointPrecisionSpinBox != nullptr) &&
                      (m_pSpyAPIPortSpinBox != nullptr) && (m_pSpyEventsPortSpinBox != nullptr) &&
                      (m_pRDSPortSpinBox != nullptr) && (m_pRDSEventsPortSpinBox != nullptr) && (m_pAlertMissingSourceFile != nullptr))
    {
        afGlobalVariablesManager& theGlobalVariablesManager = afGlobalVariablesManager::instance();

        // Set the log level:
        osDebugLogSeverity newLogSeverity = (osDebugLogSeverity)m_pLogLevelComboBox->currentIndex();
        osDebugLog::instance().setLoggedSeverity(newLogSeverity);

        // Get the float precision value:
        int fpPrecision = m_pFloatingPointPrecisionSpinBox->value();
        theGlobalVariablesManager.setFloatingPointPrecision(fpPrecision);
        apSetFloatParamsDisplayPrecision(m_pFloatingPointPrecisionSpinBox->value());

        // Set missing source alert:
        theGlobalVariablesManager.SetShouldAlertMissingSourceFile(m_pAlertMissingSourceFile->isChecked());

        // Get the current log directory:
        gtString newLogFilesPath;
        newLogFilesPath.fromASCIIString(m_pLogFilesPathLineEdit->text().toLatin1());

        // Make sure the path is treated as a directory and not last word as a file name:
        newLogFilesPath += osFilePath::osPathSeparator;

        // Verify that the directory can be accessed
        osDirectory accessPermissionTrialDir(newLogFilesPath);
        bool isLogPathAccessible = accessPermissionTrialDir.isWriteAccessible();
        bool isLogPathEmpty = m_pLogFilesPathLineEdit->text().isEmpty();

        if (isLogPathAccessible && !isLogPathEmpty)
        {
            theGlobalVariablesManager.setLogFilesDirectoryPath(newLogFilesPath);
        }
        else
        {
            // Oops! No access to the log path. Notify the user and revert to the current log path.
            QString strMessage;

            if (!isLogPathAccessible)
            {
                strMessage = QString(AF_STR_globalSettingsInaccessiblePathErrorMsg).arg(QString::fromWCharArray(newLogFilesPath.asCharArray()));
            }
            else if (isLogPathEmpty)
            {
                strMessage = AF_STR_globalSettingsEmptyPathErrorMsg;
            }

            acMessageBox::instance().warning(afGlobalVariablesManager::ProductNameA(), strMessage, QMessageBox::Ok);
        }

        // Get the current proxy settings:
        bool isUsingProxy = m_pUsingProxyCheckBox->isChecked();
        osPortAddress proxyAddress;

        if (isUsingProxy)
        {
            gtString newProxyAddress;
            newProxyAddress.fromASCIIString(m_pProxyAddress->text().toLatin1());
            proxyAddress.setAsRemotePortAddress(newProxyAddress, (unsigned short)m_pProxyPort->value());
        }

        theGlobalVariablesManager.setProxyInformation(isUsingProxy, proxyAddress);

        // Set the remote debugging ports:
        int spyPort = m_pSpyAPIPortSpinBox->value();
        int spyEventsPort = m_pSpyEventsPortSpinBox->value();
        int rdsPort = m_pRDSPortSpinBox->value();
        int rdsEventsPort = m_pRDSEventsPortSpinBox->value();
        theGlobalVariablesManager.setRemoteDebuggingPorts(spyPort, spyEventsPort, rdsPort, rdsEventsPort);
    }

    // Set each tab page's settings:
    afGlobalVariablesManager& theGlobalVariablesManager = afGlobalVariablesManager::instance();
    int numberOfSettingsPages = theGlobalVariablesManager.amountOfGlobalSettingPages();

    bool validPages = true;

    for (int i = 0; i < numberOfSettingsPages; i++)
    {
        afGlobalSettingsPage* pCurrentPage = theGlobalVariablesManager.getGlobalSettingPage(i);
        bool rcPg = pCurrentPage->saveCurrentSettings();
        GT_ASSERT(rcPg);

        validPages &= pCurrentPage->IsPageDataValid();
    }

    // Save the settings file:
    theGlobalVariablesManager.saveGlobalSettingsToXMLFile();

    if (validPages)
    {
        accept();
    }
}

// ---------------------------------------------------------------------------
// Name:        afGlobalSettingsDialog::onRestoreDefaultSettings
// Description: Called when the user presses the "Restore Defaults" button
// Author:      Uri Shomroni
// Date:        23/4/2012
// ---------------------------------------------------------------------------
void afGlobalSettingsDialog::onRestoreDefaultSettings()
{
    GT_IF_WITH_ASSERT((nullptr != m_pLogFilesPathLineEdit) && (nullptr != m_pLogLevelComboBox) && (nullptr != m_pLogFilesPathLineEdit) &&
                      (nullptr != m_pProxyAddress) && (nullptr != m_pProxyPort) && (m_pFloatingPointPrecisionSpinBox != nullptr) &&
                      (m_pRDSPortSpinBox != nullptr) && (m_pRDSEventsPortSpinBox != nullptr) &&
                      (m_pSpyAPIPortSpinBox != nullptr) && (m_pSpyEventsPortSpinBox != nullptr))
    {
        // Restore the general page default settings:
        osFilePath logFilesDefaultPath(osFilePath::OS_TEMP_DIRECTORY);
        m_pLogFilesPathLineEdit->setText(acGTStringToQString(logFilesDefaultPath.asString()));
        m_pLogLevelComboBox->setCurrentIndex((int)OS_DEBUG_LOG_INFO);
        m_pProxyAddress->setText(AF_PROXY_EXAMPLE_ADDRESS);
        m_pProxyPort->setValue(AF_PROXY_EXAMPLE_PORT);
        m_pUsingProxyCheckBox->setChecked(false);
        m_pFloatingPointPrecisionSpinBox->setValue(AF_OPTIONS_FLOATING_POINT_INIT_PRECISION_VALUE);

        onProxyCheckBox(m_pUsingProxyCheckBox->checkState());

        // Set the default remote debugging ports:
        m_pRDSPortSpinBox->setValue(AP_REMOTE_TARGET_CONNECTION_DEFAULT_CONNECTION_PORT);
        m_pRDSEventsPortSpinBox->setValue(AP_REMOTE_TARGET_CONNECTION_DEFAULT_EVENTS_PORT);
        m_pSpyAPIPortSpinBox->setValue(AP_REMOTE_TARGET_CONNECTION_DEFAULT_SPY_API_PORT);
        m_pSpyEventsPortSpinBox->setValue(AP_REMOTE_TARGET_CONNECTION_DEFAULT_SPY_EVENTS_PORT);
    }

    // Set each tab page's settings:
    afGlobalVariablesManager& theGlobalVariablesManager = afGlobalVariablesManager::instance();
    int numberOfSettingsPages = theGlobalVariablesManager.amountOfGlobalSettingPages();

    for (int i = 0; i < numberOfSettingsPages; i++)
    {
        afGlobalSettingsPage* pCurrentPage = theGlobalVariablesManager.getGlobalSettingPage(i);
        pCurrentPage->restoreDefaultSettings();
    }
}

// ---------------------------------------------------------------------------
// Name:        afGlobalSettingsDialog::onBrowseForLogFilesButton
// Description: Called when the user presses the "browse for log files" button
// Author:      Uri Shomroni
// Date:        25/4/2012
// ---------------------------------------------------------------------------
void afGlobalSettingsDialog::onBrowseForLogFilesButton()
{
    // Get the file selection:
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT((nullptr != pApplicationCommands) && (nullptr != m_pLogFilesPathLineEdit))
    {
        // Select the executable file:
        QString defaultFolder;
        QString logFolder = pApplicationCommands->ShowFolderSelectionDialog(AF_STR_globalSettingsSelectLogFilesFolderTitle, defaultFolder);

        if (!logFolder.isEmpty())
        {
            // Set the folder path:
            m_pLogFilesPathLineEdit->setText(logFolder);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afGlobalSettingsDialog::onProxyCheckBox
// Description: Called when the status of the proxy checkbox changes
// Author:      Uri Shomroni
// Date:        25/4/2012
// ---------------------------------------------------------------------------
void afGlobalSettingsDialog::onProxyCheckBox(int newState)
{
    bool enableSettings = (Qt::Checked == newState);
    m_pProxyAddress->setEnabled(enableSettings);
    m_pProxyPort->setEnabled(enableSettings);
}

// ---------------------------------------------------------------------------
// Name:        afGlobalSettingsDialog::createDialogLayout
// Description: Creates the dialog layout
// Author:      Uri Shomroni
// Date:        19/4/2012
// ---------------------------------------------------------------------------
void afGlobalSettingsDialog::createDialogLayout()
{
    // Create the tab widget:
    m_pMainTabWidget = new QTabWidget;

    // Create the "General" page:
    createGeneralPage();

    GT_IF_WITH_ASSERT(m_pGeneralPage != nullptr)
    {
        // Add the general page:
        m_pMainTabWidget->addTab(m_pGeneralPage, AF_STR_globalSettingsGeneralTabName);
    }

    // Get the project manager instance:
    afGlobalVariablesManager& theGlobalVariablesManager = afGlobalVariablesManager::instance();
    int amountOfExtensions = theGlobalVariablesManager.amountOfGlobalSettingPages();

    for (int i = 0; i < amountOfExtensions; i++)
    {
        gtString pageTitle;
        QWidget* pCurrentWidget = theGlobalVariablesManager.getGlobalSettingPage(i, &pageTitle);
        GT_IF_WITH_ASSERT(pCurrentWidget != nullptr)
        {
            // Add the general page:
            m_pMainTabWidget->addTab(pCurrentWidget, acGTStringToQString(pageTitle));
        }
    }

    // Create the dialog buttons:
    QDialogButtonBox* pBox = new QDialogButtonBox();
    GT_IF_WITH_ASSERT(nullptr != pBox)
    {
        QPushButton* pOkButton = new QPushButton(AF_STR_OK_Button);
        QPushButton* pCancelButton = new QPushButton(AF_STR_Cancel_Button);
        pBox->addButton(pOkButton, QDialogButtonBox::AcceptRole);
        pBox->addButton(pCancelButton, QDialogButtonBox::RejectRole);
    }

    // Connect the Ok button:
    bool rc = connect(pBox, SIGNAL(accepted()), this, SLOT(onOkButton()));
    GT_ASSERT(rc);
    rc = connect(pBox, SIGNAL(rejected()), this, SLOT(reject()));
    GT_ASSERT(rc);

    // Add "Restore Default Settings" button:
    QPushButton* pRestoreDefaults = new QPushButton(AF_STR_globalSettingsRestoreDefaultSettings);

    // Connect the restore default to slot:
    rc = connect(pRestoreDefaults, SIGNAL(clicked()), this, SLOT(onRestoreDefaultSettings()));
    GT_ASSERT(rc);

    // Create the main layout:
    QGridLayout* pMainLayout = new QGridLayout;
    pMainLayout->addWidget(m_pMainTabWidget, 0, 0, 1, 2);
    pMainLayout->addWidget(pRestoreDefaults, 2, 0);
    pMainLayout->addWidget(pBox, 2, 1);

    setLayout(pMainLayout);

    // Set the dialog title:
    setWindowTitle(AF_STR_globalSettingsWindowTitle);

    // Set the dialog icon:
    afLoadTitleBarIcon(this);

    // Set window flags (minimize / maximize / close buttons):
    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);
}

// ---------------------------------------------------------------------------
// Name:        afGlobalSettingsDialog::createGeneralPage
// Description: Creates the general settings page
// Author:      Uri Shomroni
// Date:        19/4/2012
// ---------------------------------------------------------------------------
void afGlobalSettingsDialog::createGeneralPage()
{
    // Create a widget for the "General" page:
    m_pGeneralPage = new QGroupBox;

    // Create the main grid layout:
    QVBoxLayout* pLayout = new QVBoxLayout;

    // Group 1: Logging
    QGroupBox* pLoggingGroupBox = new QGroupBox(AF_STR_globalSettingsLoggingGroupTitle);
    QVBoxLayout* pLoggingLayout = new QVBoxLayout;

    QHBoxLayout* pLogLevelLayout = new QHBoxLayout;
    QLabel* pLogLevelLabel = new QLabel(AF_STR_globalSettingsDebugLogLevelLabel);
    m_pLogLevelComboBox = new QComboBox;
    QStringList logLevels;
    gtString currentLogLevelAsString;

    for (int i = 0; i <= (int)OS_DEBUG_LOG_EXTENSIVE; i++)
    {
        currentLogLevelAsString = osDebugLogSeverityToString((osDebugLogSeverity)i);
        logLevels.append(acGTStringToQString(currentLogLevelAsString));
    }

    m_pLogLevelComboBox->addItems(logLevels);
    pLogLevelLayout->addWidget(pLogLevelLabel, 1, Qt::AlignVCenter | Qt::AlignLeft);
    pLogLevelLayout->addWidget(m_pLogLevelComboBox, 0, Qt::AlignVCenter | Qt::AlignRight);
    pLoggingLayout->addLayout(pLogLevelLayout);

    QLabel* pLogFilesPathLabel = new QLabel(AF_STR_globalSettingsLogFilesFolderLabel);
    QHBoxLayout* pLogFilesPathLayout = new QHBoxLayout;
    m_pLogFilesPathLineEdit = new QLineEdit;
    m_pLogFilesPathButton = new QToolButton;
    m_pLogFilesPathButton->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    m_pLogFilesPathButton->setContentsMargins(0, 0, 0, 0);
    pLogFilesPathLayout->addWidget(m_pLogFilesPathLineEdit, 1, 0);
    pLogFilesPathLayout->addWidget(m_pLogFilesPathButton, 0, Qt::AlignVCenter | Qt::AlignRight);
    pLoggingLayout->addWidget(pLogFilesPathLabel, 0, 0);
    pLoggingLayout->addLayout(pLogFilesPathLayout);

    pLoggingGroupBox->setLayout(pLoggingLayout);
    pLayout->addWidget(pLoggingGroupBox, 0, 0);

    // Group 2: Connection:
    QGroupBox* pConnectionGroupBox = new QGroupBox(AF_STR_globalSettingsConnectionGroupTitle);
    QVBoxLayout* pConnectionLayout = new QVBoxLayout;

    m_pUsingProxyCheckBox = new QCheckBox(AF_STR_globalSettingsUsingProxyCheckBox);
    pConnectionLayout->addWidget(m_pUsingProxyCheckBox);

    QHBoxLayout* pProxySettingsLayout = new QHBoxLayout;
    QLabel* pAddressLabel = new QLabel(AF_STR_globalSettingsProxyAddressLabel);
    m_pProxyAddress = new QLineEdit;
    QLabel* pPortLabel = new QLabel(AF_STR_globalSettingsProxyPortLabel);
    m_pProxyPort = new QSpinBox;
    m_pProxyPort->setRange(0, 0xffff);
    pProxySettingsLayout->addWidget(pAddressLabel, 5);
    pProxySettingsLayout->addWidget(m_pProxyAddress, 140);
    pProxySettingsLayout->addWidget(pPortLabel, 5);
    pProxySettingsLayout->addWidget(m_pProxyPort, 12);
    pConnectionLayout->addLayout(pProxySettingsLayout);


    // Create the remote settings group box.
    // Remote ports headline label.
    QLabel* pPortsHeadlineLabel = new QLabel(AF_STR_globalRemoteSettingsPortsHeadline);
    pConnectionLayout->addSpacing(5);

    // Port spin boxes.
    QHBoxLayout* pRemoteDebuggingPortsLayout = new QHBoxLayout;
    m_pSpyAPIPortSpinBox = new QSpinBox;
    m_pSpyAPIPortSpinBox->setRange(0, 0xffff);
    m_pSpyEventsPortSpinBox = new QSpinBox;
    m_pSpyEventsPortSpinBox->setRange(0, 0xffff);
    m_pRDSPortSpinBox = new QSpinBox;
    m_pRDSPortSpinBox->setRange(0, 0xffff);
    m_pRDSEventsPortSpinBox = new QSpinBox;
    m_pRDSEventsPortSpinBox->setRange(0, 0xffff);

    // Adding the widgets.
    pRemoteDebuggingPortsLayout->addWidget(pPortsHeadlineLabel, 178);
    pRemoteDebuggingPortsLayout->addWidget(m_pRDSPortSpinBox, 19);
    pRemoteDebuggingPortsLayout->addWidget(m_pRDSEventsPortSpinBox, 19);
    pRemoteDebuggingPortsLayout->addWidget(m_pSpyAPIPortSpinBox, 19);
    pRemoteDebuggingPortsLayout->addWidget(m_pSpyEventsPortSpinBox, 19);

    pConnectionLayout->addLayout(pRemoteDebuggingPortsLayout);
    pConnectionGroupBox->setLayout(pConnectionLayout);
    pLayout->addWidget(pConnectionGroupBox, 0, 0);

    m_pFloatingPointPrecisionSpinBox = new QSpinBox;
    m_pFloatingPointPrecisionSpinBox->setRange(AF_OPTIONS_FLOATING_POINT_MIN_PRECISION, AF_OPTIONS_FLOATING_POINT_MAX_PRECISION);
    connect(m_pFloatingPointPrecisionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onFPPrecisionChange(int)));

    QLabel* pFloatPrecisionLabel = new QLabel(AF_STR_globalSettingsFloatingPointPrecision);

    // Group 4: Advanced:
    QGroupBox* pAdvancedGroupBox = new QGroupBox(AF_STR_globalSettingsAdvancedGroupTitle);
    QHBoxLayout* pFloatPrecisionLayout = new QHBoxLayout;
    pFloatPrecisionLayout->addWidget(pFloatPrecisionLabel, 1);
    pFloatPrecisionLayout->addWidget(m_pFloatingPointPrecisionSpinBox);

    pAdvancedGroupBox->setLayout(pFloatPrecisionLayout);

    pLayout->addWidget(pAdvancedGroupBox);
    m_pGeneralPage->setLayout(pLayout);

    QGroupBox* pSourceFileGroup = new QGroupBox(AF_STR_projectSettingsSourceFiles);
    QVBoxLayout* pSourceLayout = new QVBoxLayout;
    m_pAlertMissingSourceFile = new QCheckBox(AF_STR_globalSettingsAlertMissingSourceFiles);
    pSourceLayout->addWidget(m_pAlertMissingSourceFile);
    pSourceFileGroup->setLayout(pSourceLayout);

    pLayout->addWidget(pSourceFileGroup);

    pLayout->addStretch(1);

    // Connect the general page signals:
    bool rc = connect(m_pLogFilesPathButton, SIGNAL(clicked()), this, SLOT(onBrowseForLogFilesButton()));
    GT_ASSERT(rc);

    rc = connect(m_pUsingProxyCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onProxyCheckBox(int)));
    GT_ASSERT(rc);

    onProxyCheckBox(m_pUsingProxyCheckBox->checkState());
}

// ---------------------------------------------------------------------------
// Name:        afGlobalSettingsDialog::fillGeneralPageData
// Description: Fills the general page from the global settings
// Author:      Uri Shomroni
// Date:        19/4/2012
// ---------------------------------------------------------------------------
void afGlobalSettingsDialog::fillGeneralPageData()
{
    afGlobalVariablesManager& theGlobalVariablesManager =  afGlobalVariablesManager::instance();

    GT_IF_WITH_ASSERT((nullptr != m_pLogFilesPathLineEdit) && (nullptr != m_pLogLevelComboBox) && (nullptr != m_pUsingProxyCheckBox) &&
                      (nullptr != m_pProxyAddress) && (nullptr != m_pProxyPort) && (m_pFloatingPointPrecisionSpinBox != nullptr))
    {
        // Get the current log level:
        osDebugLogSeverity currentLogSeverity = osDebugLog::instance().loggedSeverity();
        m_pLogLevelComboBox->setCurrentIndex((int)currentLogSeverity);

        // Get the current log directory:
        osFilePath currentLogFilesPath = theGlobalVariablesManager.logFilesDirectoryPath();
        m_pLogFilesPathLineEdit->setText(acGTStringToQString(currentLogFilesPath.asString()));

        // Get the current proxy settings:
        bool isUsingProxy = false;
        osPortAddress proxyAddress;
        theGlobalVariablesManager.getProxyInformation(isUsingProxy, proxyAddress);
        m_pUsingProxyCheckBox->setChecked(isUsingProxy);

        if (isUsingProxy)
        {
            m_pProxyAddress->setText(acGTStringToQString(proxyAddress.hostName()));
            m_pProxyPort->setValue(proxyAddress.portNumber());
        }
        else
        {
            m_pProxyAddress->setText(AF_PROXY_EXAMPLE_ADDRESS);
            m_pProxyPort->setValue(AF_PROXY_EXAMPLE_PORT);
        }

        // Get the remote debugging ports and set them to the widgets:
        int spyPort         = static_cast<int>(theGlobalVariablesManager.getSpyPort());
        int spyEventsPort   = static_cast<int>(theGlobalVariablesManager.getSpyEventsPort());
        int rdsPort         = static_cast<int>(theGlobalVariablesManager.getRdsPort());
        int rdsEventsPort   = static_cast<int>(theGlobalVariablesManager.getRdsEventsPort());
        m_pRDSEventsPortSpinBox->setValue(rdsEventsPort);
        m_pSpyAPIPortSpinBox->setValue(spyPort);
        m_pSpyEventsPortSpinBox->setValue(spyEventsPort);
        m_pRDSPortSpinBox->setValue(rdsPort);

        m_pFloatingPointPrecisionSpinBox->setValue((int)theGlobalVariablesManager.floatingPointPrecision());

        onProxyCheckBox(m_pUsingProxyCheckBox->checkState());
    }
}

// ---------------------------------------------------------------------------
// Name:        afGlobalSettingsDialog::initDialogGlobalSettings
// Description: Loads the dialog's pages from the current values
// Author:      Uri Shomroni
// Date:        23/4/2012
// ---------------------------------------------------------------------------
void afGlobalSettingsDialog::initDialogGlobalSettings()
{
    // Load the general page settings:
    fillGeneralPageData();

    // Loading each page's settings should be done by the pages themselves:
    afGlobalVariablesManager& theGlobalVariablesManager = afGlobalVariablesManager::instance();
    int numberOfGlobalSettingsPages = theGlobalVariablesManager.amountOfGlobalSettingPages();

    for (int i = 0; i < numberOfGlobalSettingsPages; i++)
    {
        afGlobalSettingsPage* pCurrentPage = theGlobalVariablesManager.getGlobalSettingPage(i);
        GT_IF_WITH_ASSERT(nullptr != pCurrentPage)
        {
            pCurrentPage->loadCurrentSettings();
        }
    }
}


void afGlobalSettingsDialog::onFPPrecisionChange(int value)
{
    GT_IF_WITH_ASSERT(m_pFloatingPointPrecisionSpinBox != nullptr)
    {
        QString strMessage;

        int newValue = value;

        if (value < AF_OPTIONS_FLOATING_POINT_MIN_PRECISION)
        {
            newValue = AF_OPTIONS_FLOATING_POINT_MIN_PRECISION;
            strMessage = QString(AF_STR_globalSettingsMinBoundErrorMsg).arg(newValue);
        }
        else if (value > AF_OPTIONS_FLOATING_POINT_MAX_PRECISION)
        {
            newValue = AF_OPTIONS_FLOATING_POINT_MAX_PRECISION;
            strMessage = QString(AF_STR_globalSettingsMinBoundErrorMsg).arg(newValue);
        }

        if (newValue != value)
        {
            acMessageBox::instance().warning(afGlobalVariablesManager::ProductNameA(), strMessage, QMessageBox::Ok);
            m_pFloatingPointPrecisionSpinBox->setValue(newValue);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afGlobalSettingsDialog::setActivePage
// Description: Set the requested extension page as active
// Arguments:   const gtString& extensionName
// Author:      GiladYarnitzky
// Date:        12/8/2013
// ---------------------------------------------------------------------------
void afGlobalSettingsDialog::setActivePage(const gtString& extensionName)
{
    GT_IF_WITH_ASSERT(m_pMainTabWidget != nullptr)
    {
        // Set the general page as active by default:
        m_pMainTabWidget->setCurrentIndex(0);

        // Get the amount of pages:
        int amountOfPages = m_pMainTabWidget->count();

        // Iterate the pages and look for the relevant page:
        for (int i = 0 ; i < amountOfPages; i++)
        {
            QString currentTabText = m_pMainTabWidget->tabText(i);

            // Remove accelerator from tab name:
            currentTabText.remove("&");

            if (currentTabText == acGTStringToQString(extensionName))
            {
                m_pMainTabWidget->setCurrentIndex(i);
                m_pMainTabWidget->currentWidget()->setFocus();
                break;
            }
        }
    }
}

