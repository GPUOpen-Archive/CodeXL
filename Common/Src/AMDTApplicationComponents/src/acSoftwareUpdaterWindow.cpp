//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acSoftwareUpdaterWindow.cpp
///
//==================================================================================

#include <AMDTApplicationComponents/Include/acSoftwareUpdaterWindow.h>

// Qt:
#include <QtWidgets>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkConfiguration>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFileLauncher.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acSoftwareUpdaterProxySetting.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>



// Display constants:
#define AF_SOFTWARE_UPDATER_BASE_WIDTH 600
#define AF_SOFTWARE_UPDATER_BASE_HEIGHT 480

acSoftwareUpdaterWindow::acSoftwareUpdaterWindow(const QString& productName, const acIconId& productIconId)
    : m_pCheckOnStartUpCheckBox(nullptr), m_pSkipBuildCheckBox(nullptr), m_pChkUpdateFrequencyComboBox(nullptr), m_pInstallButton(nullptr),
      m_pCancelButton(nullptr), m_pCloseButton(nullptr), m_pUpdateHeaderLabel(nullptr), m_pVersionDetailsWebView(nullptr),  m_pStatusLabel(nullptr),
      m_pProgressBar(nullptr), m_pDestinationFolderLabel(nullptr), m_pDestinationFolderLineEdit(nullptr), m_pDestinationFolderButton(nullptr), m_pCurrentInfo(nullptr),
      m_pLatestVersionInfo(nullptr), m_errorCode(-1), m_iSkippedBuild(-1), m_shouldSkipThisBuild(false), m_strDownloadPath(""), m_isAutoCheckEnabled(true),
      m_installedProdVersion(""), m_latestProdVersion(""), m_iNextCheckScheduleInDay(1), m_strUserProfile(""),
      m_strVersionInfoURL(""), m_isNewerVersionAvailable(false), m_isProcessingUpdate(false), m_isVersionDescriptionDisplayed(false), m_isUpdateNeeded(false),
      m_pNetworkManager(nullptr), m_pNetworkReply(nullptr), m_pUpdaterThreadForLatestInfo(nullptr), m_pUpdaterThreadToDownloadPackage(nullptr),
      m_bytesReceived(0), m_bytesTotal(0), m_isDownloading(false), m_isCheckingForNewUpdate(false), m_isForcingUpdateCheck(false), m_isDialogRaised(false),
      m_downloadFileType(AC_DOWNLOAD_EXE), m_ProductIconId(productIconId)
{
    m_productName = productName;
    m_title = QString(AC_STR_CheckForUpdatesWindowTitle).arg(m_productName);
    // Build the GUI layout:
    setDialogLayout();
}

void acSoftwareUpdaterWindow::postInit()
{
    // Connect the slots:
    connectSlots();

    // Create the network manager and threads:
    m_pNetworkManager = new QNetworkAccessManager(this);
    bool rc = connect(m_pNetworkManager, SIGNAL(finished(QNetworkReply*)), SLOT(onDownloadXMLFileFinish(QNetworkReply*)));
    GT_ASSERT(rc);

    rc = connect(&m_downloadManager, SIGNAL(finished(QNetworkReply*)), SLOT(onSetupDownloadComplete(QNetworkReply*)));
    GT_ASSERT(rc);

    rc = connect(this, SIGNAL(getLatestVersionInfo()), this, SLOT(onGetLatestVersionInfo()));
    GT_ASSERT(rc);

    // Initialize thread objects
    m_pUpdaterThreadToDownloadPackage = new acSoftwareUpdaterThread(acSoftwareUpdaterThread::AF_DOWNLOAD, this);
    m_pUpdaterThreadForLatestInfo = new acSoftwareUpdaterThread(acSoftwareUpdaterThread::AF_GET_LATEST_VERSION_INFO, this);

    // Create the proxy if a check for update is needed (this operation takes time):
    if (isCheckForUpdateNeeded())
    {
        acSoftwareUpdaterProxySetting::Instance();
    }

    // Perform the check for update if necessary:
    checkForUpdate();

    QNetworkConfiguration config;
    m_downloadManager.setConfiguration(config);
}

acSoftwareUpdaterWindow::~acSoftwareUpdaterWindow()
{
    if (m_pCurrentInfo != nullptr)
    {
        delete m_pCurrentInfo;
        m_pCurrentInfo = nullptr;
    }

    if (m_pLatestVersionInfo != nullptr)
    {
        delete m_pLatestVersionInfo;
        m_pLatestVersionInfo = nullptr;
    }

    if (m_pUpdaterThreadForLatestInfo != nullptr)
    {
        m_pUpdaterThreadForLatestInfo->terminate();
        m_pUpdaterThreadForLatestInfo->wait();
        delete m_pUpdaterThreadForLatestInfo;
        m_pUpdaterThreadForLatestInfo = nullptr;
    }

    if (m_pUpdaterThreadToDownloadPackage != nullptr)
    {
        m_pUpdaterThreadToDownloadPackage->terminate();
        m_pUpdaterThreadToDownloadPackage->wait();
        delete m_pUpdaterThreadToDownloadPackage;
        m_pUpdaterThreadToDownloadPackage = nullptr;
    }
}

void  acSoftwareUpdaterWindow::connectSlots()
{
    bool rc = connect(m_pChkUpdateFrequencyComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onUpdateCheckDaysChanged(const QString&)));
    GT_ASSERT(rc);

    rc = connect(m_pInstallButton, SIGNAL(clicked(bool)), this, SLOT(onInstallClick()));
    GT_ASSERT(rc);

    rc = connect(m_pSkipBuildCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onSkipBuildChanged(int)));
    GT_ASSERT(rc);

    rc = connect(m_pCancelButton, SIGNAL(clicked(bool)), this, SLOT(onCancelClick()));
    GT_ASSERT(rc);

    rc = connect(m_pCloseButton, SIGNAL(clicked(bool)), this, SLOT(onCloseClick()));
    GT_ASSERT(rc);

    rc = connect(m_pVersionDetailsWebView, SIGNAL(loadStarted()), SLOT(onVersionLoadStart()));
    GT_ASSERT(rc);

    rc = connect(m_pVersionDetailsWebView, SIGNAL(loadFinished(bool)), SLOT(onVersionLoadFinish(bool)));
    GT_ASSERT(rc);

    rc = connect(m_pDestinationFolderButton, SIGNAL(clicked()), this, SLOT(onDestinationFolderButtonClick()));
    GT_ASSERT(rc);

}


bool acSoftwareUpdaterWindow::setOnStartUpCheckBox(bool flag)
{
    GT_IF_WITH_ASSERT(m_pCheckOnStartUpCheckBox != nullptr)
    {
        m_pCheckOnStartUpCheckBox->setChecked(flag);
        m_pChkUpdateFrequencyComboBox->setDisabled(!flag);
        m_isAutoCheckEnabled = flag;
    }

    return true;
}

void acSoftwareUpdaterWindow::updateButtonsState()
{
    GT_IF_WITH_ASSERT((m_pSkipBuildCheckBox != nullptr) && (m_pInstallButton != nullptr) && (m_pStatusLabel != nullptr))
    {
        // Install button should be disabled while downloading:
        bool isSkipButtonChecked = m_pSkipBuildCheckBox->isChecked();
        bool shouldEnableInstallButton = !m_isDownloading && m_isUpdateNeeded && !isSkipButtonChecked;
        m_pInstallButton->setEnabled(shouldEnableInstallButton);

        m_pDestinationFolderButton->setEnabled(shouldEnableInstallButton);
        m_pDestinationFolderLineEdit->setEnabled(shouldEnableInstallButton);
        m_pDestinationFolderLabel->setEnabled(shouldEnableInstallButton);

        m_pDestinationFolderButton->setVisible(m_isUpdateNeeded);
        m_pDestinationFolderLineEdit->setVisible(m_isUpdateNeeded);
        m_pDestinationFolderLabel->setVisible(m_isUpdateNeeded);

        m_pSkipBuildCheckBox->setEnabled(!m_isDownloading && m_isUpdateNeeded);
        m_pChkUpdateFrequencyComboBox->setEnabled(!m_isDownloading);
        m_pCheckOnStartUpCheckBox->setEnabled(!m_isDownloading);
        m_pProgressBar->setVisible(m_isDownloading);
        m_pCancelButton->setVisible(m_isDownloading);
    }
}

void acSoftwareUpdaterWindow::setCheckUpdateFrequencyComboBox(int days)
{
    GT_IF_WITH_ASSERT(m_pChkUpdateFrequencyComboBox != nullptr)
    {
        switch (days)
        {

            case 1:
                m_pChkUpdateFrequencyComboBox->setCurrentIndex(0);
                break;

            case 7:
                m_pChkUpdateFrequencyComboBox->setCurrentIndex(1);
                break;

            case 30:
                m_pChkUpdateFrequencyComboBox->setCurrentIndex(2);
                break;

            default:
                GT_ASSERT(false);
                break;
        }
    }
}

void acSoftwareUpdaterWindow::updateWindowStatusLabels(const QString& title, const QString& message, const QString& status, const QString& header)
{
    GT_IF_WITH_ASSERT((m_pVersionDetailsWebView != nullptr) && (m_pUpdateHeaderLabel != nullptr))
    {
        QString htmlMessage = QString(AC_STR_CheckForUpdatesDescriptionPageHTMLFormat).arg(title).arg(message);
        m_pVersionDetailsWebView->setHtml(htmlMessage);

        if (!header.isEmpty())
        {
            m_pUpdateHeaderLabel->setText(header);
        }

        setStatusLabel(status);
    }
}


void acSoftwareUpdaterWindow::setVersionDescriptionURL(const QString& prodVersion, const QString& versionDescriptionURL)
{
    GT_IF_WITH_ASSERT((m_pVersionDetailsWebView != nullptr) && (m_pUpdateHeaderLabel != nullptr))
    {
        QString releaseInfo;
        releaseInfo.clear();

        if (!prodVersion.isEmpty())
        {
            m_latestProdVersion = prodVersion;
            releaseInfo.sprintf(AC_STR_CheckForUpdatesVersionIsAvailableNow, m_latestProdVersion.toLatin1().data());
        }

        m_pVersionDetailsWebView->load(versionDescriptionURL);

        QString strNewVersion = QString(AC_STR_CheckForUpdatesVersionIsAvailableTitle).arg(m_productName).arg(m_latestProdVersion).arg(m_installedProdVersion);
        m_pUpdateHeaderLabel->setText(strNewVersion);
    }
}

void acSoftwareUpdaterWindow::checkUpdateOnStartupStateChanged(int state)
{
    if (state == Qt::Checked)
    {
        m_isAutoCheckEnabled = true;
        m_pChkUpdateFrequencyComboBox->setDisabled(false);
    }
    else if (state == Qt::Unchecked)
    {
        m_isAutoCheckEnabled = false;
        m_pChkUpdateFrequencyComboBox->setDisabled(true);
    }

    if (state == Qt::Unchecked)
    {
        m_isAutoCheckEnabled = false;
    }
    else if (state == Qt::Checked)
    {
        m_isAutoCheckEnabled = true;
    }

    saveUserProfile();

}


bool acSoftwareUpdaterWindow::downloadAndInstall()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pLatestVersionInfo != nullptr) && (m_pUpdaterThreadToDownloadPackage != nullptr))
    {
        retVal = isNetworkConnected();

        if (retVal)
        {
            QString strUrl = FindDownloadPath();
            GT_IF_WITH_ASSERT(!strUrl.isEmpty())
            {
                gtString messageLog;
                messageLog.appendFormattedString(L"strUrl: %ls\n", acQStringToGTString(strUrl).asCharArray());
                OS_OUTPUT_DEBUG_LOG(messageLog.asCharArray(), OS_DEBUG_LOG_INFO);
                m_setUpUrl = QUrl::fromEncoded(strUrl.toLocal8Bit());
                GT_ASSERT(m_setUpUrl.isValid());

                messageLog.makeEmpty();
                messageLog.appendFormattedString(L"m_setUpUrl: %ls\n", acQStringToGTString(m_setUpUrl.toString()).asCharArray());
                OS_OUTPUT_DEBUG_LOG(messageLog.asCharArray(), OS_DEBUG_LOG_INFO);

                // Define a network request to get the file URL:
                QNetworkRequest request(m_setUpUrl);
                m_pNetworkReply = m_downloadManager.get(request);

                GT_IF_WITH_ASSERT(m_pNetworkReply != nullptr)
                {
                    // Connect the reply download progress:
                    bool rc = connect(m_pNetworkReply, SIGNAL(downloadProgress(qint64, qint64)), SLOT(onDownloadProgress(qint64, qint64)));
                    GT_ASSERT(rc);
                }

                messageLog.makeEmpty();
                messageLog.appendFormattedString(L"m_strDownloadPath: %ls\n", acQStringToGTString(m_strDownloadPath).asCharArray());
                OS_OUTPUT_DEBUG_LOG(messageLog.asCharArray(), OS_DEBUG_LOG_INFO);

                if (!m_strDownloadPath.isEmpty() && m_strDownloadPath[m_strDownloadPath.length() - 1] != '/')
                {
                    // We need to append / to the path
                    m_strDownloadPath.append("/");
                }

                messageLog.makeEmpty();
                messageLog.appendFormattedString(L"m_strDownloadPath: %ls\n", acQStringToGTString(m_strDownloadPath).asCharArray());
                OS_OUTPUT_DEBUG_LOG(messageLog.asCharArray(), OS_DEBUG_LOG_INFO);

                if (m_pUpdaterThreadToDownloadPackage->isRunning())
                {
                    m_pUpdaterThreadToDownloadPackage->exit();

                    if (!m_pUpdaterThreadToDownloadPackage->wait(1000))
                    {
                        OS_OUTPUT_DEBUG_LOG(L"Not able to terminate running thread", OS_DEBUG_LOG_ERROR);
                    }
                }

                // Start to download the file:
                m_pUpdaterThreadToDownloadPackage->start();

                // Initialize a status message, since the slots are called in a delay:
                setStatusLabel(AC_STR_CheckForUpdatesDownloading, 0);
            }
        }
        else
        {
            OS_OUTPUT_DEBUG_LOG(L"Network conenction error", OS_DEBUG_LOG_ERROR);
            m_isCheckingForNewUpdate = false;
            updateWindowStatusLabels(AC_STR_CheckForUpdatesConnectionError, AC_STR_CheckForUpdatesConnectionErrorText);
        }
    }

    return retVal;
}

// Installation:
// On windows, we have an exe installer. We download it and then execute it.
// On Linux, the user can select to download a tarball or an RPM file. For both
// files we only display in file system, we do not execute
void acSoftwareUpdaterWindow::onInstallClick()
{
    updateButtonsState();

    // download only when there is no download in progress:
    if (!m_isDownloading)
    {
        // First verify that the user selected a valid folder.
        GT_IF_WITH_ASSERT(m_pDestinationFolderLineEdit != nullptr)
        {
            m_strDownloadPath = m_pDestinationFolderLineEdit->text();
            osDirectory selectedDir;
            selectedDir.setDirectoryFullPathFromString(acQStringToGTString(m_strDownloadPath));
            GT_IF_WITH_ASSERT(selectedDir.exists())
            {
                setStatusLabel("Please wait...");
                m_isDownloading = true;
                m_downloadFileType = AC_DOWNLOAD_EXE;
                updateButtonsState();

                bool rc = downloadAndInstall();

                if (!rc)
                {
                    m_isDownloading = false;
                    updateButtonsState();
                }
            }
            else
            {
                // Display an error message to the user.
                QMessageBox msgBox;
                msgBox.setText(AC_STR_CheckForUpdatesInvalidDestinationFolderErrorMsg);
                // afMessageBox::instance().critical(AF_STR_CheckForUpdatesInvalidDestinationFolderErrorTitle, AF_STR_CheckForUpdatesInvalidDestinationFolderErrorMsg);
            }
        }
    }
}

void acSoftwareUpdaterWindow::OnDownloadRPM()
{
    // download only when there is no download in progress:
    if (!m_isDownloading)
    {
        // First verify that the user selected a valid folder.
        GT_IF_WITH_ASSERT(m_pDestinationFolderLineEdit != nullptr)
        {
            m_strDownloadPath = m_pDestinationFolderLineEdit->text();
            osDirectory selectedDir;
            selectedDir.setDirectoryFullPathFromString(acQStringToGTString(m_strDownloadPath));
            GT_IF_WITH_ASSERT(selectedDir.exists())
            {
                setStatusLabel("Please wait...");
                m_isDownloading = true;

                m_downloadFileType = AC_DOWNLOAD_RPM;
                updateButtonsState();

                bool rc = downloadAndInstall();

                if (!rc)
                {
                    m_isDownloading = false;
                    updateButtonsState();
                }
            }
        }
    }
}


void acSoftwareUpdaterWindow::OnDownloadDebian()
{
    // download only when there is no download in progress:
    if (!m_isDownloading)
    {
        // First verify that the user selected a valid folder.
        GT_IF_WITH_ASSERT(m_pDestinationFolderLineEdit != nullptr)
        {
            m_strDownloadPath = m_pDestinationFolderLineEdit->text();
            osDirectory selectedDir;
            selectedDir.setDirectoryFullPathFromString(acQStringToGTString(m_strDownloadPath));
            GT_IF_WITH_ASSERT(selectedDir.exists())
            {
                setStatusLabel("Please wait...");
                m_isDownloading = true;
                m_downloadFileType = AC_DOWNLOAD_DEB;
                updateButtonsState();

                bool rc = downloadAndInstall();

                if (!rc)
                {
                    m_isDownloading = false;
                    updateButtonsState();
                }
            }
        }
    }
}

void acSoftwareUpdaterWindow::OnDownloadTarball()
{
    // download only when there is no download in progress:
    if (!m_isDownloading)
    {
        // First verify that the user selected a valid folder.
        GT_IF_WITH_ASSERT(m_pDestinationFolderLineEdit != nullptr)
        {
            m_strDownloadPath = m_pDestinationFolderLineEdit->text();
            osDirectory selectedDir;
            selectedDir.setDirectoryFullPathFromString(acQStringToGTString(m_strDownloadPath));
            GT_IF_WITH_ASSERT(selectedDir.exists())
            {
                setStatusLabel("Please wait...");
                m_isDownloading = true;
                m_downloadFileType = AC_DOWNLOAD_TAR;
                updateButtonsState();

                bool rc = downloadAndInstall();

                if (!rc)
                {
                    m_isDownloading = false;
                    updateButtonsState();
                }
            }
        }
    }
}

void acSoftwareUpdaterWindow::onCancelClick()
{
    // Check if the updater thread is running:
    if (m_pNetworkReply != nullptr)
    {
        if (m_pNetworkReply->isRunning())
        {
            m_pNetworkReply->abort();
            m_pNetworkReply->close();
        }
    }

    m_isDownloading = false;
    m_bytesReceived = 0;
    m_bytesTotal = 0;

    // Make sure that buttons are updated with the current status of the updater:
    updateButtonsState();
}


void acSoftwareUpdaterWindow::onCloseClick()
{
    // Make sure that buttons are updated with the current status of the updater:
    updateButtonsState();

    // Close the dialog:
    close();
}

void acSoftwareUpdaterWindow::displayDialog(bool forceDialogDisplay)
{
    m_isForcingUpdateCheck = forceDialogDisplay;

    // Position the window in the center of the screen:
    QWidget* pDesktopWidget = QApplication::desktop();

    if (pDesktopWidget != nullptr)
    {
        // Center position calculation:
        int screenW = pDesktopWidget->width();
        int screenH = pDesktopWidget->height();
        int windowW = width();
        int windowH = height();
        int centerW = (screenW / 2) - (windowW / 2);
        int centerH = (screenH / 2) - (windowH / 2);
        move(centerW, centerH);
    }

    // Will only go and check for new Update if there is not new update check going on and
    // there is not download in progress.
    if ((!m_isCheckingForNewUpdate) && (!m_isDownloading))
    {
        m_isCheckingForNewUpdate = true;
        m_isUpdateNeeded = true;

        // Initialize the proxy network:
        // It will only initialize once, next time onwards it will get return early.
        acSoftwareUpdaterProxySetting::Instance();

        // Clear
        setStatusLabel();

        // Force check for update:
        checkForUpdate();

        // Display dialog only if user called check for updates, or if update is needed:
        if (m_isForcingUpdateCheck)
        {
            m_isDialogRaised = true;
            raise();
            exec();
        }

    }
}

void acSoftwareUpdaterWindow::setNewAvailableVersionDetails(bool isUpdateNeeded)
{
    m_isUpdateNeeded = isUpdateNeeded;

    if (isUpdateNeeded)
    {
        int majorVersion = m_pLatestVersionInfo->m_iProgramVersionMajor;
        int minorVersion = m_pLatestVersionInfo->m_iProgramVersionMinor;
        int buildVersion = m_pLatestVersionInfo->m_iProgramVersionBuild;

        QString strProdVersion = QString::number(majorVersion) + ".";
        strProdVersion += QString::number(minorVersion) + ".";
        strProdVersion += QString::number(buildVersion);

        setVersionDescriptionURL(strProdVersion, m_pLatestVersionInfo->m_versionDescriptionURL);
    }
    else
    {
        m_pUpdateHeaderLabel->clear();
        QString text = QString("%1 is up to date (%2)").arg(m_productName).arg(m_installedProdVersion);
        m_pUpdateHeaderLabel->setText(text);
        setStatusLabel();

        QString productUptodate1 = QString(AC_STR_CheckForUpdatesDescriptionProductUpToDate1).arg(m_productName);
        QString productUptodate2 = QString(AC_STR_CheckForUpdatesDescriptionProductUpToDate2).arg(m_productName);
        QString htmlMessage = QString(AC_STR_CheckForUpdatesDescriptionPageHTMLFormat).arg(productUptodate1).arg(productUptodate2);
        m_pVersionDetailsWebView->setHtml(htmlMessage);
    }

    m_isCheckingForNewUpdate = false;
    updateButtonsState();

}


void acSoftwareUpdaterWindow::onUpdateCheckDaysChanged(const QString& text)
{
    if (text == "Week")
    {
        m_iNextCheckScheduleInDay = 7;
    }
    else if (text == "Day")
    {
        m_iNextCheckScheduleInDay = 1;
    }
    else if (text == "Month")
    {
        m_iNextCheckScheduleInDay = 30;
    }
}

void acSoftwareUpdaterWindow::setStatusLabel(const QString& message, int progress)
{
    GT_IF_WITH_ASSERT((m_pStatusLabel != nullptr) && (m_pProgressBar != nullptr))
    {
        m_pStatusLabel->setText(message);

        if (progress >= 0)
        {
            m_pProgressBar->setValue(progress);
        }

        m_pProgressBar->setVisible(progress > 0);
    }
}

void acSoftwareUpdaterWindow::onVersionLoadStart()
{
    m_isVersionDescriptionDisplayed = false;
    QString loadMessage = QString(AC_STR_CheckForUpdatesLoadingMessage).arg(m_productName);
    setStatusLabel(loadMessage);
}

void acSoftwareUpdaterWindow::onVersionLoadFinish(bool status)
{
    if (status)
    {
        setStatusLabel();
    }
    else
    {
        // For some reason when we get here we could not load the description URL.
        // Instead, we set the version number:
        QString versionDescription = QString(AC_STR_CheckForUpdatesDescriptionText).arg(m_productName).arg(m_productName).arg(m_productName);
        updateWindowStatusLabels("New version found", versionDescription);
    }

    m_isVersionDescriptionDisplayed = true;
}

void acSoftwareUpdaterWindow::setDialogLayout()
{
    // Adjust size to screen scaling:
    int windowWidth = acScalePixelSizeToDisplayDPI(AF_SOFTWARE_UPDATER_BASE_WIDTH);
    int windowHeight = acScalePixelSizeToDisplayDPI(AF_SOFTWARE_UPDATER_BASE_HEIGHT);

    // Will fix the window size
    setFixedSize(windowWidth, windowHeight);
    setWindowFlags(Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);

    setWindowTitle(m_title);

    // Initialize window components
    m_pUpdateHeaderLabel = new QLabel;
    QFrame* pFrame = new QFrame;
    m_pVersionDetailsWebView = new QWebView;
    QHBoxLayout* pFrameLayout = new QHBoxLayout;
    pFrameLayout->addWidget(m_pVersionDetailsWebView);

    pFrame->setLayout(pFrameLayout);
    pFrame->setFrameShadow(QFrame::Plain);
    pFrame->setFrameShape(QFrame::StyledPanel);

    m_pCheckOnStartUpCheckBox = new QCheckBox("Automatically check for updates every");
    m_pSkipBuildCheckBox = new QCheckBox("Skip this build");
    m_pChkUpdateFrequencyComboBox = new QComboBox();

    m_pChkUpdateFrequencyComboBox->addItem("Day");
    m_pChkUpdateFrequencyComboBox->addItem("Week");
    m_pChkUpdateFrequencyComboBox->addItem("Month");

    m_pChkUpdateFrequencyComboBox->setDisabled(true);
    m_pChkUpdateFrequencyComboBox->setEditable(false);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    QString downloadText = "&Install";
#else
    QString downloadText = "&Download";
#endif
    m_pInstallButton = new QPushButton(downloadText);

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    QMenu* pInstallActionMenu = new QMenu;
    pInstallActionMenu->addAction(AC_STR_CheckForUpdatesDownloadTarball, this, SLOT(OnDownloadTarball()));
    pInstallActionMenu->addAction(AC_STR_CheckForUpdatesDownloadRPM, this, SLOT(OnDownloadRPM()));

    gtString linuxVariant;
    osGetLinuxVariantName(linuxVariant);

    if (linuxVariant == OS_STR_linuxVariantUbuntu)
    {
        pInstallActionMenu->addAction(AC_STR_CheckForUpdatesDownloadDebianPackage, this, SLOT(OnDownloadDebian()));
    }

    m_pInstallButton->setMenu(pInstallActionMenu);
#endif

    m_pCloseButton = new QPushButton("C&lose");
    QVBoxLayout* pCheckBoxesLayout = new QVBoxLayout;
    QHBoxLayout* pUpdaterFrequencyLayout = new QHBoxLayout();

    pUpdaterFrequencyLayout->addWidget(m_pCheckOnStartUpCheckBox);
    pUpdaterFrequencyLayout->addWidget(m_pChkUpdateFrequencyComboBox);
    pUpdaterFrequencyLayout->addStretch();

    pCheckBoxesLayout->addLayout(pUpdaterFrequencyLayout);
    pCheckBoxesLayout->addWidget(m_pSkipBuildCheckBox);

    QHBoxLayout* pButtonsLayout = new QHBoxLayout;

    pButtonsLayout->addStretch(50);
    pButtonsLayout->addWidget(m_pInstallButton, Qt::AlignRight);
    pButtonsLayout->addWidget(m_pCloseButton, Qt::AlignRight);

    // QGridLayout* pStatusLayout = new QGridLayout;
    QHBoxLayout* pStatusLayout = new QHBoxLayout;
    m_pStatusLabel = new QLabel;
    m_pProgressBar = new QProgressBar;

    int progressBarWidth = acScalePixelSizeToDisplayDPI(200);
    m_pProgressBar->setMinimumWidth(progressBarWidth);
    m_pProgressBar->setTextVisible(true);
    m_pProgressBar->setTextDirection(QProgressBar::BottomToTop);
    m_pProgressBar->setMaximumHeight(16);

    m_pCancelButton = new QPushButton("Cancel");

    // Add the progress bar to the status bar: (permanent is on the right side)
    m_pProgressBar->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));

    pStatusLayout->addWidget(m_pStatusLabel, Qt::AlignLeft);
    pStatusLayout->addWidget(m_pProgressBar, Qt::AlignRight);
    pStatusLayout->addWidget(m_pCancelButton, Qt::AlignRight);
    pStatusLayout->setContentsMargins(0, 0, 0, 0);

    // Set the default minimum + maximum values:
    m_pProgressBar->setMaximum(100);
    m_pProgressBar->setMinimum(0);

    // Initialize the destination folder widgets.
    m_pDestinationFolderLabel = new QLabel;
    m_pDestinationFolderLineEdit = new QLineEdit;

    // Create the button for displaying the folder selection dialog.
    m_pDestinationFolderButton = new QToolButton;
    m_pDestinationFolderButton->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    m_pDestinationFolderButton->setContentsMargins(0, 0, 0, 0);

    // Set the destination folder label text and max width.
    m_pDestinationFolderLabel->setText(AC_STR_CheckForUpdatesTempFolderLabelText);

    // Set the default destination folder.
    osFilePath folder(osFilePath::OS_USER_DOWNLOADS);
    m_pDestinationFolderLineEdit->setText(acGTStringToQString(folder.asString()));
    m_pDestinationFolderLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Arrange the destination folder layout.
    QHBoxLayout* pDstFolderLayout = new QHBoxLayout;
    pDstFolderLayout->addWidget(m_pDestinationFolderLabel, 0, Qt::AlignLeft);
    pDstFolderLayout->addWidget(m_pDestinationFolderLineEdit, 1, 0);
    pDstFolderLayout->addWidget(m_pDestinationFolderButton, Qt::AlignRight);

    QVBoxLayout* pMainLayout = new QVBoxLayout;

    pMainLayout->addWidget(m_pUpdateHeaderLabel);
    pMainLayout->addWidget(pFrame);
    pMainLayout->addLayout(pCheckBoxesLayout, Qt::AlignLeft);
    pMainLayout->addLayout(pDstFolderLayout);
    pMainLayout->addLayout(pStatusLayout);
    pMainLayout->addLayout(pButtonsLayout, Qt::AlignRight);

    setLayout(pMainLayout);
    m_pStatusLabel->setMinimumWidth(450);

    QPixmap icon64;
    acSetIconInPixmap(icon64, m_ProductIconId, AC_64x64_ICON);
    setWindowIcon(icon64);

    // By default will be disabled. Will enable only if new update is available:
    updateButtonsState();
}


bool acSoftwareUpdaterWindow::loadUserProfile()
{
    int year, day, month;
    bool retVal = true;

    // Initialize local variables with valid data.
    day = 1;
    month = 1;
    year = 1900;

    bool wasFileFound = false;

    if (m_pCurrentInfo != nullptr)
    {
        // Make sure that the user file exists:
        if (QFile::exists(m_strUserProfile))
        {
            // Extract the file and read data:
            QFile file(m_strUserProfile);

            QDomDocument doc;
            doc.setContent(&(file));
            QDomElement root = doc.documentElement();

            GT_IF_WITH_ASSERT(!root.isNull())
            {
                QDomNode node = root.firstChild();

                bool isCurrentFieldValid = true;

                while (!node.isNull())
                {
                    QDomNode childNode = node.firstChild();

                    if (node.toElement().tagName() == "Program_Name")
                    {
                        m_pCurrentInfo->m_strProgramName = childNode.nodeValue();
                    }
                    else if (node.toElement().tagName() == "Enable_Auto_Update")
                    {
                        m_isAutoCheckEnabled = (childNode.nodeValue() == "1");
                    }
                    else if (node.toElement().tagName() == "Last_Check_Date_Year")
                    {
                        isCurrentFieldValid = false;
                        year = childNode.nodeValue().toInt(&isCurrentFieldValid);
                        retVal &= isCurrentFieldValid;
                    }
                    else if (node.toElement().tagName() == "Last_Check_Date_Month")
                    {
                        isCurrentFieldValid = false;
                        month = childNode.nodeValue().toInt(&isCurrentFieldValid);
                        retVal &= isCurrentFieldValid;
                    }
                    else if (node.toElement().tagName() == "Last_Check_Date_Day")
                    {
                        isCurrentFieldValid = false;
                        day = childNode.nodeValue().toInt(&isCurrentFieldValid);
                        retVal &= isCurrentFieldValid;
                    }
                    else if (node.toElement().tagName() == "Next_Check")
                    {
                        isCurrentFieldValid = false;
                        m_iNextCheckScheduleInDay = childNode.nodeValue().toInt(&isCurrentFieldValid);
                        retVal &= isCurrentFieldValid;
                    }
                    else if (node.toElement().tagName() == "Skipped_Build")
                    {
                        isCurrentFieldValid = false;
                        m_iSkippedBuild = childNode.nodeValue().toInt(&isCurrentFieldValid);
                        retVal &= isCurrentFieldValid;
                    }

                    node = node.nextSibling();
                }

                m_lastCheckDate.setDate(year, month, day);

                // Set the frequency:
                setCheckUpdateFrequencyComboBox(m_iNextCheckScheduleInDay);
                setOnStartUpCheckBox(m_isAutoCheckEnabled);

                wasFileFound = true;
            }
        }
    }

    if (!wasFileFound)
    {
        if (m_pLatestVersionInfo != nullptr)
        {
            // No user XML - fill the data with default data:
            m_pCurrentInfo->m_strProgramName = m_pLatestVersionInfo->m_strProgramName;
            m_pCurrentInfo->m_iReleaseDay = m_pLatestVersionInfo->m_iReleaseDay;
            m_pCurrentInfo->m_iReleaseMonth = m_pLatestVersionInfo->m_iReleaseMonth;
            m_pCurrentInfo->m_iReleaseYear = m_pLatestVersionInfo->m_iReleaseYear;
        }
    }

    return retVal;
}


bool acSoftwareUpdaterWindow::isCheckForUpdateNeeded()
{
    bool retVal = false;

    if (m_isAutoCheckEnabled || m_isForcingUpdateCheck)
    {
        QDate scheduledCheckDate = m_lastCheckDate.addDays(m_iNextCheckScheduleInDay);

        if (QDate::currentDate() < scheduledCheckDate)
        {
            retVal =  false;
        }
        else
        {
            retVal = true;
        }
    }

    return retVal;
}

bool acSoftwareUpdaterWindow::checkForUpdate()
{
    if (!m_isProcessingUpdate)
    {
        // Make sure proxy is set:
        acSoftwareUpdaterProxySetting::Instance();

        m_isProcessingUpdate = true;

        if (m_isAutoCheckEnabled || m_isForcingUpdateCheck)
        {
            // Looking at last checked date
            if (isCheckForUpdateNeeded() || m_isForcingUpdateCheck)
            {
                setStatusLabel("Checking for latest version...");

                if (m_pUpdaterThreadForLatestInfo->isRunning())
                {
                    m_pUpdaterThreadForLatestInfo->exit();

                    if (m_pUpdaterThreadForLatestInfo->wait(1000) == false)
                    {
                        OS_OUTPUT_DEBUG_LOG(L"Not able to terminate running thread", OS_DEBUG_LOG_ERROR);
                    }
                }

                m_pUpdaterThreadForLatestInfo->start();
            }
            else
            {
                setNewAvailableVersionDetails(false);
                m_isProcessingUpdate = false;
            }
        }
        else
        {
            // Auto update has been turned off
            setNewAvailableVersionDetails(false);
            m_isProcessingUpdate = false;
        }
    }
    else
    {
        setStatusLabel("Checking for latest version...");
    }

    return true;
}

QDomElement acSoftwareUpdaterWindow::addXMLElement(QDomDocument& doc, QDomNode& node, const QString& tag, const QString& value)
{
    QDomElement el = doc.createElement(tag);
    node.appendChild(el);

    if (!value.isNull())
    {
        QDomText txt = doc.createTextNode(value);
        el.appendChild(txt);
    }

    return el;
}

void acSoftwareUpdaterWindow::saveUserProfile()
{
    QDomDocument doc;

    GT_IF_WITH_ASSERT((m_pCurrentInfo != nullptr) && (m_pLatestVersionInfo != nullptr))
    {

        // first we should confirm that path exists.
        // If not will create it.
        QFileInfo fileInfo(m_strUserProfile);
        QString fileName = fileInfo.fileName();
        QString filePath = fileInfo.filePath();
        int index = filePath.indexOf(fileName);
        filePath = filePath.left(index);
        QDir dir;
        dir.mkpath(filePath);

        QDomProcessingInstruction instr = doc.createProcessingInstruction(
                                              "xml", "version='1.0' encoding='UTF-8'");
        doc.appendChild(instr);

        QDomElement h = addXMLElement(doc, doc, "Program_Info");
        addXMLElement(doc, h, "Program_Name", m_pCurrentInfo->m_strProgramName);
        addXMLElement(doc, h, "Enable_Auto_Update", QString::number(m_isAutoCheckEnabled));
        addXMLElement(doc, h, "Last_Check_Date_Year", QString::number(m_lastCheckDate.year()));
        addXMLElement(doc, h, "Last_Check_Date_Month", QString::number(m_lastCheckDate.month()));
        addXMLElement(doc, h, "Last_Check_Date_Day", QString::number(m_lastCheckDate.day()));
        addXMLElement(doc, h, "Next_Check", QString::number(m_iNextCheckScheduleInDay));
        addXMLElement(doc, h, "Skipped_Build", QString::number(m_iSkippedBuild));

        // Write into the file:
        if (m_strUserProfile.length() == 0)
        {
            OS_OUTPUT_DEBUG_LOG(L"Config file path is empty", OS_DEBUG_LOG_ERROR);
            return;
        }

        try
        {
            QFile configFile(m_strUserProfile);
            configFile.open(QIODevice::WriteOnly | QIODevice::Text);
            QTextStream out(&configFile);
            out << doc.toString();
            configFile.close();
        }
        catch (...)
        {
            OS_OUTPUT_DEBUG_LOG(L"Encountered problem in file handling", OS_DEBUG_LOG_ERROR);
        }
    }
}

QUrl RedirectedTo(const QUrl& possibleRedirect, const QUrl& oldURL)
{
    QUrl redirect;

    if (!possibleRedirect.isEmpty() && possibleRedirect != oldURL)
    {
        redirect = possibleRedirect;
    }

    return redirect;
}

bool acSoftwareUpdaterWindow::writeByteFile(const QString& fileNameUrl, QIODevice* dataIn)
{
    bool retVal = false;

    QString fileName = QFileInfo(fileNameUrl).fileName();

    if (fileName.isEmpty())
    {
        fileName = "SetUp.exe";
    }
    else
    {
        // Find a name for the downloaded file. If the file exists, change the file name:
        m_strDownloadPath.append(fileName);

        if (QFile::exists(m_strDownloadPath))
        {
            // File already exists, Will create new name
            int i = 0;
            fileName += '.';

            while (QFile::exists(fileName + QString::number(i)))
            {
                ++i;
            }

            fileName += QString::number(i);
        }

        if (fileName.isEmpty())
        {
            fileName = "SetUp.exe";
        }
    }

    // Full path is already got created.
    QFile file1(m_strDownloadPath);

    if (!file1.open(QIODevice::WriteOnly))
    {
        gtString msg;
        msg.appendFormattedString(L"Not able to open file %ls for writing. Downloading into current directory", m_strDownloadPath.toStdWString().data());
        OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_ERROR);

        QFile file2(fileName);

        if (!file2.open(QIODevice::WriteOnly))
        {
            OS_OUTPUT_DEBUG_LOG(L"Even second attempt failed", OS_DEBUG_LOG_ERROR);
            return false;
        }

        file2.write(dataIn->readAll());
        file2.close();
    }
    else
    {
        retVal = true;
        file1.write(dataIn->readAll());
        file1.close();
    }

    return retVal;
}

void acSoftwareUpdaterWindow::onSetupDownloadComplete(QNetworkReply* pReply)
{
    QNetworkReply::NetworkError replyError = QNetworkReply::NoError;
    bool wasErrorFound = true;

    if (pReply != nullptr)
    {
        replyError = pReply->error();

        if (replyError == QNetworkReply::NoError)
        {
            wasErrorFound = false;

            QVariant possibleRedirectURL = pReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
            m_setUpUrl = RedirectedTo(possibleRedirectURL.toUrl(), m_setUpUrl);

            QString strUrl = m_setUpUrl.toEncoded();

            if (!strUrl.isEmpty())
            {
                QString tempPackageName = QFileInfo(strUrl).fileName();
            }

            if (!m_setUpUrl.isEmpty())
            {
                m_downloadManager.get(QNetworkRequest(m_setUpUrl));
                pReply->deleteLater();
            }
            else
            {
                QString filePath = FindDownloadPath();
                bool rc = writeByteFile(filePath, pReply);
                GT_IF_WITH_ASSERT(rc)
                {
                    launchInstalledFile(filePath);
                }
            }

            m_bytesReceived = 0;
            m_bytesTotal = 0;
        }
    }

    if (wasErrorFound)
    {
        gtString debugMessage;

        if (pReply != nullptr)
        {
            debugMessage.fromASCIIString(pReply->errorString().toLatin1().data());
        }

        debugMessage.prependFormattedString(L"QNetworkReply Error:%d ", replyError);
        OS_OUTPUT_DEBUG_LOG(debugMessage.asCharArray(), OS_DEBUG_LOG_ERROR);
        m_isCheckingForNewUpdate = false;

        if (replyError != QNetworkReply::OperationCanceledError)
        {
            // Do not publish error message for user cancellation:
            QString errorText = AC_STR_CheckForUpdatesDownloadErrorText;
            errorText.replace(AC_STR_CheckForUpdatesPRODUCTNAMEConst, m_productName);

            QString errorText2 = AC_STR_CheckForUpdatesFailureTitle;
            errorText2.replace(AC_STR_CheckForUpdatesPRODUCTNAMEConst, m_productName);
            updateWindowStatusLabels(errorText, AC_STR_CheckForUpdatesDownloadErrorText, "", errorText2);
        }
        else
        {
            setStatusLabel();
        }
    }
}

void acSoftwareUpdaterWindow::launchInstalledFile(const QString& filePath)
{
    GT_UNREFERENCED_PARAMETER(filePath);

    // Will get notified about download completion.
    m_isDownloading = false;
    updateButtonsState();

    if (m_downloadFileType == AC_DOWNLOAD_EXE)
    {
        // Make sure that the downloaded file exists:
        GT_IF_WITH_ASSERT(QFile::exists(m_strDownloadPath))
        {
            // Run the installer:
            close();
            osFileLauncher installLauncher(acQStringToGTString(m_strDownloadPath));
            installLauncher.launchFile();
        }
    }
    else
    {
        // Linux installation: only show the file in explorer:
        QString path = QDir::toNativeSeparators(m_strDownloadPath);
        osFilePath filePath1(acQStringToGTString(m_strDownloadPath));
        osDirectory fileDir;
        filePath1.getFileDirectory(fileDir);
        QString fileDirStr = QString("file:///%1").arg(acGTStringToQString(fileDir.directoryPath().asString()));
        QDesktopServices::openUrl(QUrl(fileDirStr));
        QString message;

        if (m_downloadFileType == AC_DOWNLOAD_TAR)
        {
            message.sprintf(AC_STR_CheckForUpdatesDownloadCompleteTar, m_strDownloadPath.toLatin1().data());
        }
        else if (m_downloadFileType == AC_DOWNLOAD_RPM)
        {
            message.sprintf(AC_STR_CheckForUpdatesDownloadCompleteRPM, m_strDownloadPath.toLatin1().data());
        }
        else if (m_downloadFileType == AC_DOWNLOAD_DEB)
        {
            message.sprintf(AC_STR_CheckForUpdatesDownloadCompleteDebian, m_strDownloadPath.toLatin1().data());
        }

        updateWindowStatusLabels(AC_STR_CheckForUpdatesDownloadComplete, message);
    }
}

void acSoftwareUpdaterWindow::onSkipBuildChanged(int state)
{
    m_shouldSkipThisBuild = (state == Qt::Checked);

    if (m_shouldSkipThisBuild)
    {
        if (m_pLatestVersionInfo)
        {
            m_iSkippedBuild = m_pLatestVersionInfo->m_iProgramVersionBuild;
        }
    }

    saveUserProfile();
    updateButtonsState();
}

void acSoftwareUpdaterWindow::onDownloadXMLFileFinish(QNetworkReply* pReply)
{
    bool rc = extractXMLContentFromNetworkReply(pReply);

    if (rc)
    {
        QDate curDate;
        m_isUpdateNeeded = false;

        if (isCheckForUpdateNeeded() || m_isForcingUpdateCheck)
        {
            m_isUpdateNeeded = true;
            curDate = QDate::currentDate();
            m_lastCheckDate.setDate(curDate.year(), curDate.month(), curDate.day());
        }

        // Save the user profile with the current details:
        saveUserProfile();

        // Sanity check:
        GT_IF_WITH_ASSERT((m_pLatestVersionInfo != nullptr) && (m_pCurrentInfo != nullptr))
        {
            bool isNewVersionOnWeb = (*m_pLatestVersionInfo > *m_pCurrentInfo);

            if (isNewVersionOnWeb)
            {
                if (((m_shouldSkipThisBuild) || (m_iSkippedBuild == m_pLatestVersionInfo->m_iProgramVersionBuild)) && (!m_isForcingUpdateCheck))
                {
                    m_shouldSkipThisBuild = true;
                    m_isNewerVersionAvailable = false;
                    setNewAvailableVersionDetails(false);
                }
                else
                {
                    m_isForcingUpdateCheck = true;
                    m_isNewerVersionAvailable = true;
                    setNewAvailableVersionDetails(true);
                }
            }
            else
            {
                m_isNewerVersionAvailable = false;
                setNewAvailableVersionDetails(false);
            }
        }

        // Clear the status label:
        if (m_isVersionDescriptionDisplayed)
        {
            setStatusLabel();
        }
    }
    else
    {
        m_isCheckingForNewUpdate = false;
        QString message;
        QString error2 = AC_STR_CheckForUpdatesConnectionErrorText2;
        error2.replace(AC_STR_CheckForUpdatesPRODUCTNAMEConst, m_productName);
        message.append(AC_STR_CheckForUpdatesConnectionErrorText);
        message.append(error2);
        updateWindowStatusLabels(AC_STR_CheckForUpdatesConnectionError, message, "", AC_STR_CheckForUpdatesFailureTitle);
        OS_OUTPUT_DEBUG_LOG(L"Network connection error", OS_DEBUG_LOG_ERROR);
    }

    m_isProcessingUpdate = false;

    // Display dialog if an update is needed:
    if (m_isUpdateNeeded && !m_isDialogRaised && rc)
    {
        raise();
        exec();
    }
}


bool acSoftwareUpdaterWindow::extractXMLContentFromNetworkReply(QNetworkReply* pReply)
{
    bool retVal = false;

    QNetworkReply::NetworkError replyError = QNetworkReply::NoError;
    bool wasErrorFound = true;

    if (pReply != nullptr)
    {
        replyError = pReply->error();

        if (replyError == QNetworkReply::NoError)
        {
            wasErrorFound = false;
            QDomDocument doc;
            GT_IF_WITH_ASSERT(doc.setContent(pReply))
            {
                QDomElement root = doc.documentElement();
                GT_IF_WITH_ASSERT(!root.isNull())
                {
                    retVal = true;
                    QDomNode node = root.firstChild();

                    while (!node.isNull())
                    {
                        QDomNode childNode = node.firstChild();

                        // Flag that is checking the current field:
                        bool isCurrentFieldValid = true;

                        if (node.toElement().tagName() == "Program_Version_Major")
                        {
                            m_pLatestVersionInfo->m_iProgramVersionMajor = childNode.nodeValue().toInt(&isCurrentFieldValid);
                        }
                        else if (node.toElement().tagName() == "Program_Version_Minor")
                        {
                            m_pLatestVersionInfo->m_iProgramVersionMinor = childNode.nodeValue().toInt(&isCurrentFieldValid);
                        }
                        else if (node.toElement().tagName() == "Program_Version_Build")
                        {
                            m_pLatestVersionInfo->m_iProgramVersionBuild = childNode.nodeValue().toInt(&isCurrentFieldValid);
                        }
                        else if (node.toElement().tagName() == "Program_Release_Year")
                        {
                            m_pLatestVersionInfo->m_iReleaseYear = childNode.nodeValue().toInt(&isCurrentFieldValid);
                        }
                        else if (node.toElement().tagName() == "Program_Release_Month")
                        {
                            m_pLatestVersionInfo->m_iReleaseMonth = childNode.nodeValue().toInt(&isCurrentFieldValid);
                        }
                        else if (node.toElement().tagName() == "Program_Release_Day")
                        {
                            m_pLatestVersionInfo->m_iReleaseDay = childNode.nodeValue().toInt(&isCurrentFieldValid);
                        }
                        else if (node.toElement().tagName() == "Program_Name")
                        {
                            m_pLatestVersionInfo->m_strProgramName = childNode.nodeValue();
                        }
                        else if (node.toElement().tagName() == "Program_File")
                        {
                            m_pLatestVersionInfo->m_strProgramFile = childNode.nodeValue();
                        }
                        else if (node.toElement().tagName() == "Program_File2")
                        {
                            m_pLatestVersionInfo->m_strProgramFile2 = childNode.nodeValue();
                        }
                        else if (node.toElement().tagName() == "Program_File3")
                        {
                            m_pLatestVersionInfo->m_strProgramFile3 = childNode.nodeValue();
                        }
                        else if (node.toElement().tagName() == "Version_Description_URL")
                        {
                            m_pLatestVersionInfo->m_versionDescriptionURL = childNode.nodeValue();
                        }

                        retVal = retVal && isCurrentFieldValid;

                        if (!isCurrentFieldValid)
                        {
                            gtString messageLog;
                            messageLog.appendFormattedString(L"Invalid XML Field: %ls\n", acQStringToGTString(node.toElement().tagName()).asCharArray());
                            OS_OUTPUT_DEBUG_LOG(messageLog.asCharArray(), OS_DEBUG_LOG_ERROR);
                        }

                        node = node.nextSibling();
                    }
                }
            }
        }

        if (wasErrorFound)
        {
            gtString debugMessage;

            if (pReply != nullptr)
            {
                debugMessage.fromASCIIString(pReply->errorString().toLatin1().data());
            }

            debugMessage.prependFormattedString(L" QNetworkReply Error:%d", replyError);
            OS_OUTPUT_DEBUG_LOG(debugMessage.asCharArray(), OS_DEBUG_LOG_ERROR);
        }
    }

    return retVal;
}

bool acSoftwareUpdaterWindow::onGetLatestVersionInfo()
{
    if (!isNetworkConnected())
    {
        m_isProcessingUpdate = false;
        OS_OUTPUT_DEBUG_LOG(L"Network connection error", OS_DEBUG_LOG_ERROR);
        m_isCheckingForNewUpdate = false;
        QString errorText = QString(AC_STR_CheckForUpdatesConnectionErrorText3).arg(m_productName);
        updateWindowStatusLabels(AC_STR_CheckForUpdatesConnectionError, AC_STR_CheckForUpdatesConnectionErrorText, "", errorText);
        return false;
    }

    if (theNetworkProxyChecker.IsChecking())
    {
        connect(&theNetworkProxyChecker, SIGNAL(Finished()), this, SLOT(onLazyGetLatestVersionInfo()));
        setStatusLabel("Checking for latest version...");
        return true;
    }

    QString strUrl = m_strVersionInfoURL;
    QUrl url = QUrl::fromEncoded(strUrl.toLocal8Bit());

    QNetworkRequest request(url);

    m_pNetworkManager->get(request);

    return true;
}

bool acSoftwareUpdaterWindow::onLazyGetLatestVersionInfo()
{
    if (!isNetworkConnected())
    {
        m_isProcessingUpdate = false;
        OS_OUTPUT_DEBUG_LOG(L"Network connection error", OS_DEBUG_LOG_ERROR);
        m_isCheckingForNewUpdate = false;
        updateWindowStatusLabels(AC_STR_CheckForUpdatesConnectionError, AC_STR_CheckForUpdatesConnectionErrorText);
        setStatusLabel("");
        return false;
    }

    QString strUrl = m_strVersionInfoURL;
    QUrl url = QUrl::fromEncoded(strUrl.toLocal8Bit());

    QNetworkRequest request(url);

    m_pNetworkManager->get(request);

    disconnect(&theNetworkProxyChecker, SIGNAL(Finished()), this, SLOT(onLazyGetLatestVersionInfo()));
    // TO_DO: Should reduce the singleton reference count:
    // delete(g_CheckNwtProxy);
    // g_CheckNwtProxy = nullptr;

    return true;
}

bool acSoftwareUpdaterWindow::isNetworkConnected()
{
    bool retVal = true;

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    retVal = false;
    // NOTICE: This test is only working properly on window. On linux the test has bugs,
    // specially on Ubuntu. Therefore, we decided to remove it on linux, and try to
    // get the network reply anyway. If we will fail later, we will test it and output for network connection problems:
    QList<QNetworkInterface> networkInterfaces = QNetworkInterface::allInterfaces();

    for (int i = 0; i < networkInterfaces.count(); i++)
    {
        QNetworkInterface ntwInterface = networkInterfaces.at(i);

        if (ntwInterface.flags().testFlag(QNetworkInterface::IsUp) &&
            !ntwInterface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            if (ntwInterface.addressEntries().count() > 0)
            {
                retVal = true;
            }
        }

        if (retVal)
        {
            break;
        }
    }

#endif

    return retVal;
}

void acSoftwareUpdaterWindow::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    GT_IF_WITH_ASSERT(m_pLatestVersionInfo != nullptr)
    {

        m_bytesReceived = bytesReceived;
        m_bytesTotal = bytesTotal;

        // Notice: On Linux sometimes the bytes total are negative. Use the buffer size for the calculation:
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)

        if (m_bytesTotal < 0)
        {
            m_bytesTotal = m_pNetworkReply->size();
        }

#endif
        // Build the progress text:
        gtASCIIString message;
        int progress = -1;

        if ((m_bytesReceived < m_bytesTotal) && (m_bytesTotal > 0))
        {
            // Calculate the progress values:
            float recievedMB = m_bytesReceived / 1024 / 1024;
            float totalMB = m_bytesTotal / 1024  / 1024;
            float percentF = recievedMB / totalMB * 100;
            int percent = (int)percentF;

            // Make sure that the progress string is valid:
            if ((recievedMB > 0) && (totalMB > 0) && (percentF > 0))
            {
                // Get the name of the downloaded version:
                message.appendFormattedString("Updating... %1.fMB of %1.fMB downloaded", recievedMB, totalMB, percent);
                float dev = (float)m_bytesReceived / (float)m_bytesTotal;
                progress = dev * 100;

                setStatusLabel(message.asCharArray(), progress);

            }
        }


        // Update the GUI:
        updateButtonsState();

        m_isCheckingForNewUpdate = false;
    }
}

void acSoftwareUpdaterWindow::initVersionDetails(const osFilePath& userConfigFilePath, const gtString& imagesPath)
{
    // Set the xml URL:
    gtString versionURL = AC_STR_CheckForUpdatesServerXMLFileName;

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    int OS_BITNESS = 0;

    if (osGetOSAddressSpace(OS_BITNESS))
    {
        if (OS_BITNESS == AMDT_64_BIT_ADDRESS_SPACE)
        {
            versionURL = AC_STR_CheckForUpdatesServerXMLFileNameFor64Bits;
        }
    }

#endif

    versionURL.replace(AC_STR_CheckForUpdatesPRODUCTNAMEConstW, acQStringToGTString(m_productName));
    gtString versionURLOverride;
    gtString envName;
    envName.appendFormattedString(AC_STR_CheckForUpdatesEnvVarName, acQStringToGTString(m_productName).toUpperCase().asCharArray());
    bool rc = osGetCurrentProcessEnvVariableValue(envName, versionURLOverride);

    if (rc)
    {
        versionURL = versionURLOverride;
        gtString message;
        message.appendFormattedString(L"Overriding the updater XML: %ls", versionURLOverride.asCharArray());
        OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_INFO);
    }

    // Get the version build, major and minor version:
    int buildVersion = -1, majorVersion = -1, minorVersion = -1, year = -1, month = -1, day = -1;
    rc = acGetVersionDetails(buildVersion, majorVersion, minorVersion, year, month, day);
    GT_ASSERT(rc);


    // Create the information structures:
    m_pCurrentInfo = new(std::nothrow) acSoftwareUpdateInfo();

    m_pLatestVersionInfo = new(std::nothrow) acSoftwareUpdateInfo();


    // Override the config file information
    m_pCurrentInfo->m_iProgramVersionBuild = buildVersion;
    m_pCurrentInfo->m_iProgramVersionMajor = majorVersion;
    m_pCurrentInfo->m_iProgramVersionMinor = minorVersion;

    m_pCurrentInfo->m_iReleaseYear = year;
    m_pCurrentInfo->m_iReleaseMonth = month;
    m_pCurrentInfo->m_iReleaseDay = day;

    m_isForcingUpdateCheck = false;
    m_isAutoCheckEnabled = true;
    m_iSkippedBuild = -1;
    m_shouldSkipThisBuild = false;
    m_isNewerVersionAvailable = false;
    m_iNextCheckScheduleInDay = 1;
    m_isProcessingUpdate = false;
    m_isUpdateNeeded = false;

    // Initializing with valid date
    m_lastCheckDate.setDate(1900, 1, 2);

    m_strUserProfile = acGTStringToQString(userConfigFilePath.asString());
    m_strVersionInfoURL = acGTStringToQString(versionURL);

    // Create the updater window:
    osProductVersion appVersion;
    osGetApplicationVersion(appVersion);
    gtString versionStr = appVersion.toString(false);

    m_installedProdVersion = acGTStringToQString(versionStr);
    QString statusText = QString(AC_STR_CheckForUpdatesStatusText).arg(m_productName).arg(m_installedProdVersion);
    GT_IF_WITH_ASSERT(m_pUpdateHeaderLabel != nullptr)
    {
        m_pUpdateHeaderLabel->setText(statusText);
    }
    m_imagesPath = imagesPath;

    // Load the user saved settings from local XML file:
    rc = loadUserProfile();
    GT_ASSERT(rc);

    // save to config file
    saveUserProfile();

    postInit();
}

void acSoftwareUpdaterWindow::performAutoCheckForUpdate()
{
    if (isCheckForUpdateNeeded())
    {
        // Perform forced update:
        displayDialog(false);
    }
}

void acSoftwareUpdaterWindow::closeEvent(QCloseEvent* pEvent)
{
    GT_UNREFERENCED_PARAMETER(pEvent);

    m_isForcingUpdateCheck = false;
    m_isDialogRaised = false;

    GT_IF_WITH_ASSERT(m_pVersionDetailsWebView != nullptr)
    {
        m_pVersionDetailsWebView->setPage(nullptr);
    }
}

void acSoftwareUpdaterWindow::showEvent(QShowEvent* pEvent)
{
    GT_UNREFERENCED_PARAMETER(pEvent);

    GT_IF_WITH_ASSERT(m_pVersionDetailsWebView != nullptr)
    {
        if (m_pLatestVersionInfo != nullptr)
        {
            QString descriptionURL = m_pLatestVersionInfo->m_versionDescriptionURL;

            if (!descriptionURL.isEmpty())
            {
                m_pVersionDetailsWebView->load(descriptionURL);
            }
        }
    }
}

void acSoftwareUpdaterWindow::onDestinationFolderButtonClick()
{
    GT_IF_WITH_ASSERT(m_pDestinationFolderButton != nullptr)
    {
        // Set the currently selected.
        QString currentlySelectedFolder = m_pDestinationFolderLineEdit->text();
        osDirectory defaultFolder;
        defaultFolder.setDirectoryFullPathFromString(acQStringToGTString(currentlySelectedFolder));

        if (!defaultFolder.exists())
        {
            // If the currently selected folder does not exist, take the default one.
            defaultFolder.setDirectoryFullPathFromString(osFilePath::OS_TEMP_DIRECTORY);
            currentlySelectedFolder = acGTStringToQString(defaultFolder.directoryPath().asString());
        }

        // Display the folder selection dialog.
        QString selectedFolder = ShowFolderSelectionDialog(AC_STR_CheckForUpdatesSelectDestinationFolder, currentlySelectedFolder);

        if (!selectedFolder.isEmpty())
        {
            // If the user selected a new folder, set is as the current folder.
            m_strDownloadPath = selectedFolder;
            m_pDestinationFolderLineEdit->setText(m_strDownloadPath);
        }

        // Make sure that we go back to the update window.
        this->raise();
    }
}

QString acSoftwareUpdaterWindow::FindDownloadPath() const
{
    QString retVal;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pLatestVersionInfo != nullptr)
    {
        switch (m_downloadFileType)
        {
            case AC_DOWNLOAD_EXE:
            case AC_DOWNLOAD_RPM:
            {
                retVal = m_pLatestVersionInfo->m_strProgramFile;
            }
            break;

            case AC_DOWNLOAD_TAR:
            {
                retVal = m_pLatestVersionInfo->m_strProgramFile2;
            }
            break;

            case AC_DOWNLOAD_DEB:
            {
                retVal = m_pLatestVersionInfo->m_strProgramFile3;
            }
            break;

            default:
                break;
        }
    }

    return retVal;
}

acSoftwareUpdaterThread::acSoftwareUpdaterThread(acUpdaterThreadOperationType opType, acSoftwareUpdaterWindow* pDlg) :
    m_threadOperationType(opType), m_pDialog(pDlg)
{
    setTerminationEnabled(true);
}

acSoftwareUpdaterThread::~acSoftwareUpdaterThread()
{
    m_pDialog = nullptr;
}

void acSoftwareUpdaterThread::run()
{
    switch (m_threadOperationType)
    {
        case AF_DOWNLOAD:
        {
            m_pDialog->download();
            break;
        }

        case AF_GET_LATEST_VERSION_INFO:
        {

            m_pDialog->getLatestVersionInfo();
            break;
        }

        default:
            break;
    }

    exec();
}

QString acSoftwareUpdaterWindow::ShowFolderSelectionDialog(const QString& dialogCaption, QString& defaultFolder)
{
    QString retVal;

    // Call the QFileDialog:
    QFileDialog::Options options;
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    QFileDialog dialog(this, dialogCaption, defaultFolder);

    options = QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly;
    dialog.setOptions(options);

    acPrepareDialog(dialog, m_imagesPath);

    dialog.setFileMode(QFileDialog::Directory);

    if (QDialog::Accepted == dialog.exec())
    {
        retVal = dialog.selectedFiles().value(0);
    }


#elif AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    retVal = QFileDialog::getExistingDirectory(this, dialogCaption, defaultFolder);
#endif // AMDT_BUILD_TARGET

    return retVal;
}

