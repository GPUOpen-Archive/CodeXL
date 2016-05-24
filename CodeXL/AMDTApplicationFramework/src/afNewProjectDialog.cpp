//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afNewProjectDialog.cpp
///
//==================================================================================

// Warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>


// Infra:
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>
#include <AMDTApplicationComponents/Include/acValidators.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>

// For remote sessions.
#include <AMDTRemoteClient/Include/CXLDaemonClient.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afBrowseAction.h>
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afLineEdit.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afNewProjectDialog.h>
#include <AMDTApplicationFramework/Include/dialogs/afEditEnvironmentVariablesDialog.h>
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/dialogs/afMultipleDirectoriesBrowseDialog.h>
#include <AMDTApplicationFramework/Include/dialogs/afWindowsStoreAppsBrowserDialog.h>

// Static member initialization:
afNewProjectDialog* afNewProjectDialog::m_spMySingleInstance = nullptr;

// Dialog size:
#define AF_NEW_PROJECT_DIALOG_WIDTH 1024
#define AF_NEW_PROJECT_DIALOG_HEIGHT 500
#define AF_NEW_PROJECT_SETTINGS_TREE_WIDTH 150
#define AF_NEW_PROJECT_BUTTON_STANDARD_SIZE 21
#define AF_NEW_PROJECT_DIALOG_SCREEN_MARGIN 100

// Default port for daemon connection.
const gtUInt16 DAEMON_DEFAULT_PORT = 27015;

// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::afNewProjectDialog
// Description: Constructor
// Arguments:   QWidget* pParent
// Author:      Sigal Algranaty
// Date:        2/4/2012
// ---------------------------------------------------------------------------

afNewProjectDialog::afNewProjectDialog()
    : QDialog(afMainAppWindow::instance()),
      m_pApplicationCommands(nullptr), m_pSettingsTree(nullptr), m_pCurrentSettingsPageContainer(nullptr),
      m_pCurrentSettingsFrameLayout(nullptr), m_pGeneralPage(nullptr),
      m_pProjectNameTitle(nullptr), m_pProjectNameTextEdit(nullptr),
      m_pProgramExeRadioButton(nullptr), m_pProgramExeLabel(nullptr), m_pProgramExeTextEdit(nullptr), m_pBrowseForExeButton(nullptr), m_pBrowseForExeButtonAction(nullptr),
      m_pWinStoreAppRadioButton(nullptr), m_pRemoteHostRadioButton(nullptr), m_pLocalHostRadioButton(nullptr), m_pDummyRemoteWidgetA(nullptr),
      m_pDummyRemoteWidgetB(nullptr), m_pRemoteHostLayoutA(nullptr),  m_pRemoteHostLayoutB(nullptr), m_pRemoteHostIpLineEdit(nullptr),
      m_pRemoteHostPortLabel(nullptr), m_pRemoteHostPortLineEdit(nullptr), m_pRemoteHostAddressLabel(nullptr), m_pTestConnectionButton(nullptr),
      m_pWorkingFolderTitle(nullptr), m_pWorkingFolderTextEdit(nullptr), m_pWorkingFolderPathButton(nullptr), m_pBrowseForWorkingFolderAction(nullptr),
      m_pProgramArgsTitle(nullptr), m_pProgramArgs(nullptr),
      m_pEnvironmentVariablesTitle(nullptr), m_pEnvironmentVariablesTextEdit(nullptr), m_pEditEnvironmentVariables(nullptr),
      m_pSourceFilesDirectoryTitle(nullptr), m_pSourceFilesDirectoryTextEdit(nullptr), m_pSourceFilesDirectoryButton(nullptr),
      m_pSourceCodeRootDirectoryTitle(nullptr), m_pSourceCodeRootDirectoryTextEdit(nullptr), m_pSourceCodeRootDirectoryButton(nullptr),
      m_pSourceCodeRootDirectoryButtonAction(nullptr), m_initializingData(false), m_isRunningFromVS(false), m_selectedTreeFilePath(AF_globalSettingsGeneralHeader)
{
    // Set the VS flag:
    m_isRunningFromVS = afGlobalVariablesManager::instance().isRunningInsideVisualStudio();

    // Get the application commands instance:
    m_pApplicationCommands = afApplicationCommands::instance();
    GT_ASSERT(m_pApplicationCommands);

    // Set the dialog size:
    int dialogW = (int)acScalePixelSizeToDisplayDPI(AF_NEW_PROJECT_DIALOG_WIDTH);
    int dialogH = (int)acScalePixelSizeToDisplayDPI(AF_NEW_PROJECT_DIALOG_HEIGHT);
    resize(dialogW, dialogH);

    // Create the dialog layout:
    createDialogLayout();

    // Read current project settings:
    initDialogCurrentProjectSettings();

    // Set the dialog icon:
    afLoadTitleBarIcon(this);
}

// ---------------------------------------------------------------------------
// Name:        ~afNewProjectDialog
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        2/4/2012
// ---------------------------------------------------------------------------
afNewProjectDialog::~afNewProjectDialog()
{
}


// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::showDialog
// Description: Display the dialog with the current settings
// Author:      Sigal Algranaty
// Date:        10/4/2012
// ---------------------------------------------------------------------------
void afNewProjectDialog::ShowDialog(afDialogMode mode, const gtString& selectedTreeFilePath, const gtString& executablePath, afFocusArea focusArea)
{
    // Set the dialog mode:
    m_dialogMode = mode;

    // If the current project is empty, always open as new project dialog:
    if (afProjectManager::instance().currentProjectFilePath().isEmpty())
    {
        m_dialogMode = AF_DIALOG_NEW_PROJECT;
    }

    if (m_dialogMode == AF_DIALOG_NEW_PROJECT)
    {
        // Set the dialog for a new project
        setWindowTitle(AF_STR_newProjectWindowNewTitle);
    }
    else if (m_dialogMode == AF_DIALOG_EDIT_PROJECT)
    {
        // Set the window title:
        setWindowTitle(AF_STR_newProjectWindowEditTitle);
    }

    // Initialize the current project settings:
    initDialogCurrentProjectSettings();

    if (mode == AF_DIALOG_NEW_PROJECT)
    {
        // Restore default values:
        OnRestoreDefaultSettings(false);

        // Set the requested exe path:
        setRequestedExePath(executablePath);
    }

    // Select the first page:
    SelectTreeItemByTreePath(acGTStringToQString(selectedTreeFilePath));

    // Set the requested focused controls:
    SetFocusArea(focusArea);

    // Execute the dialog:
    afApplicationCommands::instance()->showModal(this);

}

// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::instance
// Description: Get singleton instance
// Return Val:  afProjectManager&
// Author:      Sigal Algranaty
// Date:        3/4/2012
// ---------------------------------------------------------------------------
afNewProjectDialog& afNewProjectDialog::instance()
{
    // If my single instance was not created yet - create it:
    if (m_spMySingleInstance == nullptr)
    {
        m_spMySingleInstance = new afNewProjectDialog;
        GT_ASSERT(m_spMySingleInstance);
    }

    return *m_spMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::OnClickBrowseForExe
// Description: Open the file dialog and browse for an executable
// Author:      Sigal Algranaty
// Date:        2/4/2012
// ---------------------------------------------------------------------------
void afNewProjectDialog::OnClickBrowseForExe()
{
    // Get the file selection:
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT((pApplicationCommands != nullptr) && (m_pProgramExeTextEdit != nullptr) && (m_pWorkingFolderTextEdit != nullptr) &&
                      (m_pProjectNameTextEdit != nullptr) && (m_pRemoteHostPortLineEdit != nullptr) && (m_pBrowseForExeButton != nullptr) && (m_pBrowseForExeButtonAction != nullptr))
    {
        // Get the executable file types:
        QString exeFileTypes = AF_STR_executableFilesDetails;

        // Get the current selected executable:
        QString currentExecutableFile = m_pProgramExeTextEdit->text();

        // Select the executable file:
        QString selectedExe = pApplicationCommands->ShowFileSelectionDialog(AF_STR_newProjecExecutableFileSelectionTitle, currentExecutableFile, exeFileTypes, m_pBrowseForExeButtonAction, false);

        if (!selectedExe.isEmpty())
        {
            // Set the file name:
            m_pProgramExeTextEdit->setText(selectedExe);

            osFilePath selectedExePath(acQStringToGTString(selectedExe));
            gtString projectName;
            selectedExePath.getFileName(projectName);

            if (!projectName.isEmpty())
            {
                QPalette pal;
                pal.setColor(QPalette::Text, acQGREY_TEXT_COLOUR);
                m_pProjectNameTextEdit->setPalette(pal);

                if (!m_pRemoteHostIpLineEdit->text().isEmpty() && m_pRemoteHostRadioButton->isChecked())
                {
                    projectName += L"@";
                    projectName += acQStringToGTString(m_pRemoteHostIpLineEdit->text());
                }

                m_pProjectNameTextEdit->setText(QString::fromWCharArray(projectName.asCharArray()));
            }

            osDirectory workingDirectory;
            bool rc = selectedExePath.getFileDirectory(workingDirectory);

            if (rc)
            {
                m_pWorkingFolderTextEdit->setText(QString::fromWCharArray(workingDirectory.directoryPath().asString().asCharArray()));
            }
        }
    }
}

void afNewProjectDialog::OnApplicationPathBrowse()
{
    GT_IF_WITH_ASSERT(m_pProgramExeRadioButton != nullptr)
    {
        bool shouldSelectExe = m_pProgramExeRadioButton->isChecked();

        if (shouldSelectExe)
        {
            OnClickBrowseForExe();
        }
        else
        {
            OnClickBrowseForWindowsStoreApp();
        }
    }
}

void afNewProjectDialog::OnClickBrowseForWindowsStoreApp()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pProgramExeTextEdit != nullptr)
    {
        afWindowsStoreAppsBrowserDialog storeAppsBrowser(m_pProgramExeTextEdit->text());

        if (storeAppsBrowser.exec() == QDialog::Accepted)
        {
            m_pProjectNameTextEdit->setText(storeAppsBrowser.GetAppName());
            m_pProgramExeTextEdit->setText(storeAppsBrowser.GetUserModelID());

            osFilePath packageDir(storeAppsBrowser.GetPackageDirectory().toStdWString().c_str());

            if (packageDir.exists())
            {
                m_pWorkingFolderTextEdit->setText(storeAppsBrowser.GetPackageDirectory());
            }
            else
            {
                m_pWorkingFolderTextEdit->clear();
            }
        }
    }
}



// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::OnExeEditingFinished
// Description: If the user entered a text into the executable path, update the
//              work folder
// Author:      Sigal Algranaty
// Date:        15/4/2012
// ---------------------------------------------------------------------------
void afNewProjectDialog::OnExeEditingFinished()
{
    // Sanity check
    GT_IF_WITH_ASSERT((m_pProgramExeTextEdit != nullptr) && (m_pWorkingFolderTextEdit != nullptr) && (m_pProjectNameTextEdit != nullptr))
    {
        // Build the executable path string:
        gtString exePathStr;

        acWideQStringToGTString(m_pProgramExeTextEdit->text(), exePathStr);
        osFilePath exePath(exePathStr);

        QPalette pal;
        pal.setColor(QPalette::Text, Qt::black);
        m_pProjectNameTextEdit->setPalette(pal);

        gtString projectName;
        exePath.getFileName(projectName);

        if (projectName.isEmpty())
        {
            // Since exe can be empty, make sure that the project name is always set:
            projectName = acQStringToGTString(afApplicationCommands::instance()->FindDefaultProjectName());
        }

        if (!m_pRemoteHostIpLineEdit->text().isEmpty() && m_pRemoteHostRadioButton->isChecked())
        {
            projectName += L"@";
            projectName += acQStringToGTString(m_pRemoteHostIpLineEdit->text());
        }

        m_pProjectNameTextEdit->setText(QString::fromWCharArray(projectName.asCharArray()));

        osDirectory workingDirectory;
        bool rc = exePath.getFileDirectory(workingDirectory);

        if (rc)
        {
            m_pWorkingFolderTextEdit->setText(QString::fromWCharArray(workingDirectory.directoryPath().asString().asCharArray()));
        }
    }
}

void afNewProjectDialog::OnRemoteHostTextChanged(const QString& text)
{
    // Sanity check
    GT_IF_WITH_ASSERT((m_pProjectNameTextEdit != nullptr) && (m_pRemoteHostIpLineEdit != nullptr))
    {
        // In VS do not update the project name:
        if (!m_isRunningFromVS)
        {
            QPalette pal;
            pal.setColor(QPalette::Text, Qt::black);
            m_pProjectNameTextEdit->setPalette(pal);

            QString projectName = m_pProjectNameTextEdit->text();

            // if project name is empty (the default name was deleted) - re-add the default project name
            if (projectName.isEmpty())
            {
                projectName = afApplicationCommands::instance()->FindDefaultProjectName();
            }

            int pos = projectName.indexOf('@');

            if (pos >= 0)
            {
                projectName.chop(projectName.size() - pos);
            }

            if (!text.isEmpty())
            {
                projectName += "@";
                projectName += text;
            }

            m_pProjectNameTextEdit->setText(projectName);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::onExeChange
// Description: If the user entered a text into the executable path, update the
//              observers
// Author:      Bhattacharyya Koushik
// Date:        15/4/2012
// ---------------------------------------------------------------------------
void afNewProjectDialog::onExeChanged(const QString& strExe)
{
    GT_UNREFERENCED_PARAMETER(strExe);

    // Sanity check
    GT_IF_WITH_ASSERT(m_pProgramExeTextEdit != nullptr)
    {
        bool isUserModelId = ((nullptr != m_pWinStoreAppRadioButton) && m_pWinStoreAppRadioButton->isChecked());
        QString strExeInner = m_pProgramExeTextEdit->text();
        afProjectManager::instance().EmitExecutableChanged(strExeInner, false, isUserModelId);
    }
}

void afNewProjectDialog::OnAppTypeRadioButtonSelect()
{
    // Sanity check
    GT_IF_WITH_ASSERT(m_pProgramExeTextEdit != nullptr && m_pWinStoreAppRadioButton != nullptr)
    {
        if (m_pWinStoreAppRadioButton->isEnabled())
        {
            m_pWinStoreAppRadioButton->setChecked(false);
            m_pProgramExeTextEdit->setEnabled(!m_isRunningFromVS);
            m_pBrowseForExeButton->setEnabled(!m_isRunningFromVS);
            m_pRemoteHostRadioButton->setEnabled(true);
        }
        else
        {
            m_pProgramExeRadioButton->setChecked(true);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::OnClickBrowseForPath
// Description: Browse for the project working folder
// Author:      Sigal Algranaty
// Date:        2/4/2012
// ---------------------------------------------------------------------------
void afNewProjectDialog::OnClickBrowseForPath()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pWorkingFolderTextEdit != nullptr) && (m_pBrowseForWorkingFolderAction != nullptr))
    {
        // Get the file selection:
        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT((pApplicationCommands != nullptr) && (m_pWorkingFolderTextEdit != nullptr) && (m_pBrowseForWorkingFolderAction != nullptr))
        {
            QString selectedFolder;
            QString currentFolder = m_pWorkingFolderTextEdit->text();
            selectedFolder = pApplicationCommands->ShowFolderSelectionDialog(AF_STR_newProjectWorkingFolderSelectionTitle, currentFolder, m_pBrowseForWorkingFolderAction);

            if (!selectedFolder.isEmpty())
            {
                m_pWorkingFolderTextEdit->setText(selectedFolder);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::OnEditEnvironmentVariables
// Description: Edit environment variables
// Author:      Sigal Algranaty
// Date:        11/4/2012
// ---------------------------------------------------------------------------
void afNewProjectDialog::OnEditEnvironmentVariables()
{
    // Define an edit environment variables dialog:
    gtList<osEnvironmentVariable> envList;

    // Add the environment variables:
    gtList<osEnvironmentVariable>::const_iterator iter = m_projectSettings.environmentVariables().begin();
    gtList<osEnvironmentVariable>::const_iterator endIter = m_projectSettings.environmentVariables().end();

    while (iter != endIter)
    {
        envList.push_back(*iter);
        iter++;
    }

    // Open the dialog:
    afEditEnvironmentVariablesDialog dlg(this, envList);
    int rc = dlg.exec();

    if (rc == QDialog::Accepted)
    {
        // Get the environments as string:
        const gtString& envVarsStr = dlg.environmentVariablesAsString();

        // Set the environment string:
        GT_IF_WITH_ASSERT(m_pEnvironmentVariablesTextEdit != nullptr)
        {
            m_pEnvironmentVariablesTextEdit->setText(QString::fromWCharArray(envVarsStr.asCharArray()));
        }

        // Clear the environment variables:
        m_projectSettings.clearEnvironmentVariables();

        // Add the environment variables:
        iter = envList.begin();
        endIter = envList.end();

        while (iter != endIter)
        {
            m_projectSettings.addEnvironmentVariable(*iter);
            iter++;
        }
    }
}

int afNewProjectDialog::CalculateTreeWidth()
{
    int retVal = (int)acScalePixelSizeToDisplayDPI(AF_NEW_PROJECT_SETTINGS_TREE_WIDTH);

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSettingsTree != nullptr)
    {
        QString text = "ABC Profile ABC GPU Profile: Perf. Counters";
        QFont font = m_pSettingsTree->font();
        QRect treeTextBoundingRect = QFontMetrics(font).boundingRect(text);
        retVal = treeTextBoundingRect.width();

        // Add margin:
        retVal += 6;
    }

    return retVal;
}

void afNewProjectDialog::createDialogLayout()
{
    // Create the tab widget:
    m_pSettingsTree = new acTreeCtrl(this, 1, false);
    m_pSettingsTree->blockSignals(true);
    m_pSettingsTree->header()->hide();
    m_pSettingsTree->setContextMenuPolicy(Qt::NoContextMenu);
    int treeWidth = CalculateTreeWidth();
    m_pSettingsTree->setFixedWidth(treeWidth);
    m_pSettingsTree->setItemDelegate(new acItemDelegate);

    // Disable multi-line selection:
    m_pSettingsTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pSettingsTree->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_pCurrentSettingsFrameLayout = new QStackedLayout;

    // Connect the restore default to slot:
    bool rc = connect(m_pSettingsTree, SIGNAL(itemSelectionChanged()), this, SLOT(OnSettingsTreeItemChanged()));
    GT_ASSERT(rc);

    rc = connect(m_pSettingsTree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnSettingsTreeItemClicked()));
    GT_ASSERT(rc);

    // Create the "General" page:
    createGeneralPage();

    // Get the project manager instance:
    afProjectManager& projectsManager = afProjectManager::instance();
    int amountOfExtensions = projectsManager.amountOfProjectExtensions();

    for (int i = 0; i < amountOfExtensions; i++)
    {
        gtString extensionName;
        QWidget* pCurrentWidget = projectsManager.getExtensionSettingsWidget(i, extensionName);
        GT_IF_WITH_ASSERT(pCurrentWidget != nullptr)
        {
            afProjectSettingsExtension* pCurrentExtension = qobject_cast<afProjectSettingsExtension*>(pCurrentWidget);

            if ((pCurrentExtension != nullptr) && pCurrentExtension->ShouldAddToProjectSettingDialog())
            {
                AddExtensionSettingsPage(extensionName, pCurrentWidget);
            }
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
    rc = connect(pBox, SIGNAL(accepted()), this, SLOT(OnOkButton()));
    GT_ASSERT(rc);
    rc = connect(pBox, SIGNAL(rejected()), this, SLOT(OnCancelButton()));
    GT_ASSERT(rc);

    // Add "Restore Default Settings" button:
    QPushButton* pRestoreDefaults = new QPushButton(AF_STR_newProjectRestoreDefaultSettings);

    // Connect the restore default to slot:
    rc = connect(pRestoreDefaults, SIGNAL(clicked()), this, SLOT(OnRestoreDefaultSettings()));
    GT_ASSERT(rc);

    m_pCurrentSettingsPageContainer = new QFrame;
    m_pCurrentSettingsPageContainer->setFrameStyle(QFrame::StyledPanel);
    m_pCurrentSettingsPageContainer->setStyleSheet("QFrame { background-color: white;}");
    m_pCurrentSettingsPageContainer->setLayout(m_pCurrentSettingsFrameLayout);
    m_pCurrentSettingsFrameLayout->setContentsMargins(0, 0, 0, 0);
    m_pCurrentSettingsFrameLayout->setSpacing(0);
    m_pCurrentSettingsPageContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Create the main layout:
    QGridLayout* pMainLayout = new QGridLayout;
    pMainLayout->addWidget(m_pSettingsTree, 0, 0);
    pMainLayout->addWidget(m_pCurrentSettingsPageContainer, 0, 1);
    pMainLayout->addWidget(pRestoreDefaults, 1, 0, Qt::AlignLeft);
    pMainLayout->addWidget(pBox, 1, 1, Qt::AlignRight);

    pMainLayout->setColumnStretch(0, 0);
    pMainLayout->setColumnStretch(1, 1);
    setLayout(pMainLayout);

    // Set my caption:
    setWindowTitle(AF_STR_newProjectWindowEditTitle);

    // Set window flags (minimize / maximize / close buttons):
    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    m_pSettingsTree->blockSignals(false);

    // Expand all top level items of the tree:
    m_pSettingsTree->collapseAll();

    for (int i = 0 ; i < m_pSettingsTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* pItem = m_pSettingsTree->topLevelItem(i);

        if (pItem != nullptr)
        {
            pItem->setExpanded(true);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::initDialogCurrentProjectSettings
// Description: Read current project settings
// Author:      Sigal Algranaty
// Date:        3/4/2012
// ---------------------------------------------------------------------------
void afNewProjectDialog::initDialogCurrentProjectSettings()
{
    m_initializingData = true;

    // Get current the project settings:
    m_projectSettings = afProjectManager::instance().currentProjectSettings();

    // Fill the general page data:
    fillGeneralPageData();

    m_initializingData = false;
}

// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::OnOkButton
// Description: Is handling the user ok click
// Author:      Sigal Algranaty
// Date:        3/4/2012
// ---------------------------------------------------------------------------
void afNewProjectDialog::OnOkButton()
{
    // Check if the settings are valid:
    gtString invalidMessage, invalidExtensionTreePath, prevSourcePaths, newSourcePaths;
    bool isValid = AreSettingsValid(invalidMessage, invalidExtensionTreePath);

    if (isValid)
    {
        // Sanity check:
        GT_IF_WITH_ASSERT((m_pProjectNameTextEdit != nullptr) &&
                          (m_pProgramExeRadioButton != nullptr) &&
                          (m_pProgramExeTextEdit != nullptr) &&
                          (m_pWinStoreAppRadioButton != nullptr) &&
                          (m_pWorkingFolderTextEdit != nullptr) &&
                          (m_pProgramArgs != nullptr) &&
                          (m_pEnvironmentVariablesTextEdit != nullptr))
        {
            // Set the project settings:
            gtString filePath;

            // Set executable and user model ID:
            osFilePath exePath;
            gtString userModelId;

            if (m_pWinStoreAppRadioButton->isChecked())
            {
                acWideQStringToGTString(m_pProgramExeTextEdit->text(), userModelId);
            }
            else
            {
                acQStringToOSFilePath(m_pProgramExeTextEdit->text().trimmed(), exePath);
            }

            m_projectSettings.setWindowsStoreAppUserModelID(userModelId);
            m_projectSettings.setExecutablePath(exePath);

            osFilePath workPath;
            osDirectory dir;
            dir.setDirectoryFullPathFromString(acQStringToGTString(m_pWorkingFolderTextEdit->text().trimmed()));
            m_projectSettings.setWorkDirectory(dir.directoryPath());

            gtString args;
            acWideQStringToGTString(m_pProgramArgs->text(), args);
            m_projectSettings.setCommandLineArguments(args);

            gtString projectName;
            acWideQStringToGTString(m_pProjectNameTextEdit->text(), projectName);
            m_projectSettings.setProjectName(projectName);

            // Clear the environment variables:
            gtString envStr;
            m_projectSettings.clearEnvironmentVariables();
            acWideQStringToGTString(m_pEnvironmentVariablesTextEdit->text(), envStr);
            m_projectSettings.addEnvironmentVariablesString(envStr, AF_STR_newProjectEnvironmentVariablesDelimiter);
        }

        GT_IF_WITH_ASSERT(m_pSourceFilesDirectoryTextEdit != nullptr)
        {
            prevSourcePaths = m_projectSettings.SourceFilesDirectories();
            gtString sourceCodeDirs = acQStringToGTString(m_pSourceFilesDirectoryTextEdit->text());

            if (sourceCodeDirs.isEmpty())
            {
                // By default sources are located in the application path:
                sourceCodeDirs = m_projectSettings.workDirectory().asString();
            }

            newSourcePaths = sourceCodeDirs;

            m_projectSettings.SetSourceFilesDirectories(sourceCodeDirs);

            m_projectSettings.SetSourceCodeRootLocation(acQStringToGTString(m_pSourceCodeRootDirectoryTextEdit->text()));
            QStringList sourceFolders = m_pSourceFilesDirectoryTextEdit->text().split(";");

            if (!sourceFolders.isEmpty())
            {
                QString lastBrowsedFolder = sourceFolders.at(sourceFolders.size() - 1);
                afGlobalVariablesManager::instance().SetLastBrowseLocation(AF_Str_NewProjectBrowseForSourceCodeFolder, lastBrowsedFolder);
            }
        }

        // For remote sessions.
        GT_IF_WITH_ASSERT(m_pRemoteHostRadioButton != nullptr  && m_pLocalHostRadioButton != nullptr  &&
                          m_pRemoteHostIpLineEdit != nullptr && m_pRemoteHostPortLineEdit != nullptr)
        {
            // Check if this is a remote session.
            bool isRemoteSession = m_pRemoteHostRadioButton->isChecked();

            // Update the global context to indicate whether this is a remote session.
            // Extract connection parameters.
            QString qstrHostIp = m_pRemoteHostIpLineEdit->text().trimmed();
            QString qstrHostPort = m_pRemoteHostPortLineEdit->text().trimmed();
            gtString hostIp = acQStringToGTString(qstrHostIp);
            gtString hostPort = acQStringToGTString(qstrHostPort);

            unsigned int hostPortNumber = 0;
            hostPort.toUnsignedIntNumber(hostPortNumber);

            if (isRemoteSession)
            {
                // Verify that the port number is valid.
                if (!(hostPortNumber > 0 && hostPortNumber < 65536))
                {
                    hostPortNumber = DAEMON_DEFAULT_PORT;
                }

                m_projectSettings.setRemoteDebugging(hostIp, static_cast<gtUInt16>(hostPortNumber));
            }
            else
            {
                m_projectSettings.setLocalDebugging();
                m_projectSettings.SetRemoteTargetHostname(hostIp);
                m_projectSettings.setRemoteTargetDaemonPort(static_cast<gtUInt16>(hostPortNumber));
            }

            isValid = true;


            GT_ASSERT_EX(isValid, L"Failed to extract CodeXL Daemon connection parameters.");
        }

        // Store the global setting
        afGlobalVariablesManager::instance().saveGlobalSettingsToXMLFile();

        // Accept:
        accept();

        // Store the current project settings:
        bool rc = storeProjectSettings();
        GT_ASSERT(rc);

        // Trigger event for source dir changes, only after project setting is saved so new files can be added to project
        if (prevSourcePaths.compareNoCase(newSourcePaths) < 0 || (m_dialogMode == AF_DIALOG_NEW_PROJECT))
        {
            if (m_dialogMode == AF_DIALOG_NEW_PROJECT)
            {
                prevSourcePaths.makeEmpty();
            }

            afProjectManager::instance().EmitSourcePathChanged(prevSourcePaths, newSourcePaths);
        }

        emit OkButtonClicked();
    }
    else
    {
        if (!invalidMessage.isEmpty())
        {
            // Output a message to the user with the invalid project settings details:
            acMessageBox::instance().critical(AF_STR_ErrorA, acGTStringToQString(invalidMessage));
        }

        // Select the extension with the problem:
        SelectTreeItemByTreePath(acGTStringToQString(invalidExtensionTreePath));
    }
}


// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::closeEvent
// Description: Implementing the close event to catch the reject operation
// Arguments:   QCloseEvent * pEvent
// Author:      Sigal Algranaty
// Date:        6/6/2012
// ---------------------------------------------------------------------------
void afNewProjectDialog::closeEvent(QCloseEvent* pEvent)
{
    // Call cancel button execution:
    OnCancelButton();

    // Call the base class implementation:
    QDialog::closeEvent(pEvent);
}

void afNewProjectDialog::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        OnCancelButton();
    }
}


// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::OnCancelButton
// Description: Is handling the user cancel click
// Author:      Sigal Algranaty
// Date:        4/6/2012
// ---------------------------------------------------------------------------
void afNewProjectDialog::OnCancelButton()
{
    // Restore the settings to the current settings:
    initDialogCurrentProjectSettings();

    // Restore each of the extensions settings:
    afProjectManager::instance().restoreCurrentExtensionSettings();

    // Cancel the dialog and exit:
    reject();
}

// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::createGeneralPage
// Description: Create the layout for the general page
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
void afNewProjectDialog::createGeneralPage()
{
    // Create a widget for the "General" page:
    m_pGeneralPage = new QGroupBox;

    // Create the main grid layout:
    QGridLayout* pLayout = new QGridLayout;

    // Line 1: Project name:
    m_pProjectNameTitle = new QLabel;
    m_pProjectNameTitle->setText(AF_STR_newProjecProjectName);
    m_pProjectNameTextEdit = new QLineEdit;
    m_pProjectNameTextEdit->clear();

    // Set a validator to the project name text edit box:
    QValidator* pValidator = new acFileNameValidator;
    m_pProjectNameTextEdit->setValidator(pValidator);

    // Connect the project name to an edit slot:
    bool rc = connect(m_pProjectNameTextEdit, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(OnProjectNameEdit(int, int)));
    GT_ASSERT(rc);

    QLabel* pGeneralCaptionLabel = new QLabel(AF_STR_newProjectGeneralTabName);
    pGeneralCaptionLabel->setStyleSheet(AF_STR_captionLabelStyleSheetMain);

    int currentGridRow = 0;
    pLayout->addWidget(pGeneralCaptionLabel, currentGridRow, 0, 1, 3);

    // Next line - project name:
    currentGridRow++;
    pLayout->addWidget(m_pProjectNameTitle, currentGridRow, 0);
    pLayout->addWidget(m_pProjectNameTextEdit, currentGridRow, 1, 1, 2);


    // Remote host:
    m_pRemoteHostRadioButton = new QRadioButton(AF_STR_newProject_remoteHost);
    m_pLocalHostRadioButton = new QRadioButton(AF_STR_newProject_localHost);

    m_pRemoteHostRadioButton->setToolTip(AF_STR_newProject_remoteHostTooltip);
    m_pLocalHostRadioButton->setToolTip(AF_STR_newProject_localHostTooltip);

    m_pDummyRemoteWidgetA = new QWidget;
    m_pRemoteHostLayoutA = new QHBoxLayout;
    m_pRemoteHostLayoutB = new QHBoxLayout;
    m_pRemoteHostIpLineEdit = new afLineEdit(AF_Str_NewProjectRemoteHostIPLineEdit);
    m_pRemoteHostPortLabel = new QLabel;
    m_pRemoteHostPortLineEdit = new QLineEdit;
    m_pTestConnectionButton = new QPushButton(AF_STR_newProject_remoteHostTestConnection);
    m_pTestConnectionButton->setToolTip(AF_STR_newProject_remoteHostTestConnectionTooltip);

    m_pRemoteHostLayoutB->setContentsMargins(0, 0, 0, 0);
    m_pRemoteHostLayoutB->addWidget(m_pLocalHostRadioButton, 0, Qt::AlignLeft);
    m_pRemoteHostLayoutB->addWidget(m_pRemoteHostRadioButton, 0, Qt::AlignLeft);
    m_pRemoteHostLayoutB->addStretch();


    // Next line - remote:
    currentGridRow++;
    pLayout->addWidget(new QLabel(AF_STR_newProject_targetHost), currentGridRow, 0);
    pLayout->addLayout(m_pRemoteHostLayoutB, currentGridRow, 1);

    int portWidth = QFontMetrics(m_pRemoteHostPortLineEdit->font()).boundingRect("9999999").width();
    m_pRemoteHostPortLineEdit->setMaximumWidth(portWidth);
    m_pRemoteHostLayoutA->setContentsMargins(0, 0, 0, 0);
    m_pRemoteHostLayoutA->addWidget(m_pRemoteHostIpLineEdit, 1);
    m_pRemoteHostLayoutA->addWidget(m_pRemoteHostPortLabel);
    m_pRemoteHostLayoutA->addWidget(m_pRemoteHostPortLineEdit);
    m_pRemoteHostLayoutA->addWidget(m_pTestConnectionButton);
    currentGridRow++;
    m_pRemoteHostAddressLabel = new QLabel(AF_STR_newProject_remoteHostAddress);
    pLayout->addWidget(m_pRemoteHostAddressLabel, currentGridRow, 0);
    pLayout->addLayout(m_pRemoteHostLayoutA, currentGridRow, 1, 1, 2);

    m_pLocalHostRadioButton->setChecked(true);
    m_pRemoteHostPortLabel->setText(AF_STR_newProject_remoteHostPort);
    m_pRemoteHostPortLineEdit->setText(AF_STR_newProject_remoteHostDefaultPort);
    m_pRemoteHostPortLineEdit->setEnabled(false);
    m_pRemoteHostAddressLabel->setEnabled(false);


    // Assign the event handlers.
    rc = connect(m_pRemoteHostRadioButton, SIGNAL(toggled(bool)), this, SLOT(OnHostRadioButtonSelection(bool)));
    GT_ASSERT(rc);

    rc = connect(m_pLocalHostRadioButton, SIGNAL(toggled(bool)), this, SLOT(OnHostRadioButtonSelection(bool)));
    GT_ASSERT(rc);

    rc = connect(m_pTestConnectionButton, SIGNAL(clicked()), this, SLOT(OnTestConnection()));
    GT_ASSERT(rc);

    QString targetAppCaption = m_isRunningFromVS ? AF_STR_projectSettingsTargetApplicationVS : AF_STR_projectSettingsTargetApplication;
    QLabel* pTargetAppCaptionLabel = new QLabel(targetAppCaption);
    pTargetAppCaptionLabel->setStyleSheet(AF_STR_captionLabelStyleSheet);
    currentGridRow++;
    pLayout->addWidget(pTargetAppCaptionLabel, currentGridRow, 0, 1, 3);

    // Exe / User ID radio:
    m_pProgramExeRadioButton = new QRadioButton(AF_STR_newProjectDesktopApplication);
    m_pProgramExeRadioButton->setToolTip(AF_STR_newProjectDesktopApplicationTooltip);
    m_pWinStoreAppRadioButton = new QRadioButton(AF_STR_newProjectWindowsStoreApp);
    m_pWinStoreAppRadioButton->setToolTip(AF_STR_newProjectWindowsStoreAppTooltip);

    QButtonGroup* pGroup = new QButtonGroup;
    pGroup->addButton(m_pProgramExeRadioButton);
    pGroup->addButton(m_pWinStoreAppRadioButton);

    m_pProgramExeLabel = new QLabel(AF_STR_newProjectProjectExePath);

    m_pProgramExeTextEdit = new QLineEdit;
    m_pProgramExeTextEdit->clear();
    m_pBrowseForExeButton = new QToolButton();
    m_pBrowseForExeButton->setContentsMargins(0, 0, 0, 0);
    m_pBrowseForExeButtonAction = new afBrowseAction(AF_Str_NewProjectBrowseForEXE);
    m_pBrowseForExeButton->setDefaultAction(m_pBrowseForExeButtonAction);
    m_pBrowseForExeButton->setToolTip(AF_STR_newProjectProjectBrowseForExePathTooltipApp);
    m_pBrowseForExeButtonAction->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    m_pBrowseForExeButtonAction->setToolTip(AF_STR_newProjectProjectBrowseForExePathTooltipApp);

    QHBoxLayout* pHLayout = new QHBoxLayout;
    pHLayout->addWidget(m_pProgramExeRadioButton, 0, Qt::AlignLeft);
    pHLayout->addWidget(m_pWinStoreAppRadioButton, 0, Qt::AlignLeft);
    pHLayout->addStretch();

    // The application type line will only be available on windows:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    QLabel* pLabelapplicationType = new QLabel(AF_STR_newProjectProjectApplicationType);
    pLabelapplicationType->setEnabled(!m_isRunningFromVS);
    currentGridRow++;
    pLayout->addWidget(pLabelapplicationType, currentGridRow, 0);
    pLayout->addLayout(pHLayout, currentGridRow, 1, 1, 2);
#endif

    currentGridRow++;
    pLayout->addWidget(m_pProgramExeLabel, currentGridRow, 0);
    pLayout->addWidget(m_pProgramExeTextEdit, currentGridRow, 1);
    pLayout->addWidget(m_pBrowseForExeButton, currentGridRow, 2);

    rc = connect(m_pProgramExeRadioButton, SIGNAL(clicked()), this, SLOT(OnAppTypeRadioButtonSelect()));
    GT_ASSERT(rc);

    // Connect the browse button to its slot:
    rc = connect(m_pBrowseForExeButtonAction, SIGNAL(triggered()), this, SLOT(OnApplicationPathBrowse()));
    GT_ASSERT(rc);

    // Connect the browse button to its slot:
    rc = connect(m_pProgramExeTextEdit, SIGNAL(editingFinished()), this, SLOT(OnExeEditingFinished()));
    GT_ASSERT(rc);

    // Connect the browse button to its slot:
    rc = connect(m_pRemoteHostIpLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(OnRemoteHostTextChanged(const QString&)));
    GT_ASSERT(rc);

    rc = connect(m_pProgramExeTextEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onExeChanged(const QString&)));
    GT_ASSERT(rc);

    rc = connect(m_pWinStoreAppRadioButton, SIGNAL(clicked()), this, SLOT(OnApplicationTypeRadioButtonSelect()));
    GT_ASSERT(rc);

    rc = connect(m_pProgramExeRadioButton, SIGNAL(clicked()), this, SLOT(OnApplicationTypeRadioButtonSelect()));
    GT_ASSERT(rc);

    if (!osSupportWindowsStoreApps())
    {
        m_pWinStoreAppRadioButton->setEnabled(false);
    }

    // Working folder:
    m_pWorkingFolderTitle = new QLabel;
    m_pWorkingFolderTitle->setText(AF_STR_newProjectWorkingFolder);
    m_pWorkingFolderTextEdit = new QLineEdit;
    m_pWorkingFolderPathButton = new QToolButton;
    m_pWorkingFolderPathButton->setContentsMargins(0, 0, 0, 0);
    m_pWorkingFolderPathButton->setToolTip(AF_STR_newProjectWorkingFolderTooltip);

    m_pBrowseForWorkingFolderAction = new afBrowseAction(AF_Str_NewProjectBrowseForWorkingFolder);
    m_pWorkingFolderPathButton->setDefaultAction(m_pBrowseForWorkingFolderAction);
    m_pBrowseForWorkingFolderAction->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    m_pBrowseForWorkingFolderAction->setToolTip(AF_STR_newProjectWorkingFolderTooltip);

    // Get the size for a button with standard icon:
    int buttonDim = (int)acScalePixelSizeToDisplayDPI(AF_NEW_PROJECT_BUTTON_STANDARD_SIZE);
    QSize buttonSize(buttonDim, buttonDim);

    currentGridRow++;
    pLayout->addWidget(m_pWorkingFolderTitle, currentGridRow, 0);
    pLayout->addWidget(m_pWorkingFolderTextEdit, currentGridRow, 1);
    pLayout->addWidget(m_pWorkingFolderPathButton, currentGridRow, 2);

    // Connect the browse button to its slot:
    rc = connect(m_pBrowseForWorkingFolderAction, SIGNAL(triggered()), this, SLOT(OnClickBrowseForPath()));
    GT_ASSERT(rc);

    // Command line arguments:
    m_pProgramArgsTitle = new QLabel;
    m_pProgramArgsTitle->setText(AF_STR_newProjectProgramArguments);
    m_pProgramArgs = new QLineEdit;

    currentGridRow++;
    pLayout->addWidget(m_pProgramArgsTitle, currentGridRow, 0);
    pLayout->addWidget(m_pProgramArgs, currentGridRow, 1, 1, 1);
    m_pProgramArgs->setToolTip(AF_STR_newProjectProgramArgumentsTooltip);

    // Environment variables:
    m_pEnvironmentVariablesTitle = new QLabel;
    m_pEnvironmentVariablesTitle->setText(AF_STR_newProjectEnvironmentVariables);
    m_pEnvironmentVariablesTextEdit = new QLineEdit;
    m_pEnvironmentVariablesTextEdit->setEnabled(false);

    m_pEditEnvironmentVariables = new QToolButton;
    m_pEditEnvironmentVariables->setText(QString::fromWCharArray(AF_STR_newProjectEditEnvVars));
    m_pEditEnvironmentVariables->setContentsMargins(0, 0, 0, 0);
    m_pEditEnvironmentVariables->setMaximumSize(buttonSize);
    m_pEditEnvironmentVariables->setMinimumSize(buttonSize);
    m_pEditEnvironmentVariables->setToolTip(AF_STR_newProjectEditEnvVarsTooltip);

    currentGridRow++;
    pLayout->addWidget(m_pEnvironmentVariablesTitle, currentGridRow, 0);
    pLayout->addWidget(m_pEnvironmentVariablesTextEdit, currentGridRow, 1);
    pLayout->addWidget(m_pEditEnvironmentVariables, currentGridRow, 2);

    rc = connect(m_pEditEnvironmentVariables, SIGNAL(clicked()), this, SLOT(OnEditEnvironmentVariables()));
    GT_ASSERT(rc);

    // Source file sections:
    QLabel* pSourceFilesLabel = new QLabel(AF_STR_projectSettingsSourceFiles);
    pSourceFilesLabel->setStyleSheet(AF_STR_captionLabelStyleSheet);
    currentGridRow++;
    pLayout->addWidget(pSourceFilesLabel, currentGridRow, 0, 1, 3);

    // Source files directory:
    m_pSourceFilesDirectoryTitle = new QLabel(AF_STR_projectSettingsSourceFilesDirectory);
    m_pSourceFilesDirectoryTextEdit = new QLineEdit;

    m_pSourceFilesDirectoryButton = new QToolButton;
    m_pSourceFilesDirectoryButton->setText(QString::fromWCharArray(AF_STR_newProjectEditEnvVars));
    m_pSourceFilesDirectoryButton->setContentsMargins(0, 0, 0, 0);
    m_pSourceFilesDirectoryButton->setContentsMargins(0, 0, 0, 0);
    m_pSourceFilesDirectoryButton->setMaximumSize(buttonSize);
    m_pSourceFilesDirectoryButton->setMinimumSize(buttonSize);
    m_pSourceFilesDirectoryButton->setToolTip(AF_STR_projectSettingsSourceFilesDirectoryTooltip);

    // Connect the browse button to its slot:
    rc = connect(m_pSourceFilesDirectoryButton, SIGNAL(clicked()), this, SLOT(OnClickBrowseForSourceFilesDirectory()));
    GT_ASSERT(rc);

    currentGridRow++;
    pLayout->addWidget(m_pSourceFilesDirectoryTitle, currentGridRow, 0);
    pLayout->addWidget(m_pSourceFilesDirectoryTextEdit, currentGridRow, 1);
    pLayout->addWidget(m_pSourceFilesDirectoryButton, currentGridRow, 2);

    // Source file root directory:
    m_pSourceCodeRootDirectoryTitle = new QLabel(AF_STR_projectSettingsSourceFilesResolutionDirectory);
    m_pSourceCodeRootDirectoryTextEdit = new QLineEdit;

    m_pSourceCodeRootDirectoryButton = new QToolButton;
    m_pSourceCodeRootDirectoryButtonAction = new afBrowseAction(AF_Str_NewProjectBrowseForSourceCodeRootFolder);
    m_pSourceCodeRootDirectoryButtonAction->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    m_pSourceCodeRootDirectoryButton->setContentsMargins(0, 0, 0, 0);

    // Connect the browse button to its slot:
    rc = connect(m_pSourceCodeRootDirectoryButtonAction, SIGNAL(triggered()), this, SLOT(OnClickBrowseForSourceRootDirectory()));
    GT_ASSERT(rc);

    currentGridRow++;
    pLayout->addWidget(m_pSourceCodeRootDirectoryTitle, currentGridRow, 0);
    pLayout->addWidget(m_pSourceCodeRootDirectoryTextEdit, currentGridRow, 1);
    pLayout->addWidget(m_pSourceCodeRootDirectoryButton, currentGridRow, 2);


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    m_pSourceCodeRootDirectoryTitle->setEnabled(false);
    m_pSourceCodeRootDirectoryTextEdit->setEnabled(false);
    m_pSourceCodeRootDirectoryButton->setEnabled(false);
#endif

    // Add a dummy label for stretch:
    currentGridRow++;
    pLayout->addWidget(new QLabel(), currentGridRow, 2, 1, 1, Qt::AlignLeft);
    pLayout->setRowStretch(currentGridRow, 100);

    pLayout->setColumnStretch(0, 10);
    pLayout->setColumnStretch(1, 80);
    pLayout->setColumnStretch(2, 10);
    m_pGeneralPage->setLayout(pLayout);

    // Set the Program Executable radio button to be the default:
    m_pProgramExeRadioButton->setChecked(true);
    OnAppTypeRadioButtonSelect();

    // Add the general page:
    QStringList generalList;
    generalList << AF_STR_newProjectGeneralTabName;
    QTreeWidgetItem* pGeneralTreeItem = m_pSettingsTree->addItem(generalList, m_pGeneralPage);
    m_treePathToTreeItemMap[AF_STR_newProjectGeneralTabName] = pGeneralTreeItem;
    m_pCurrentSettingsFrameLayout->addWidget(m_pGeneralPage);
    // Fit the GUI for VS:
    FitToVisualStudio();

}

// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::storeProjectSettings
// Description: Store the current project settings
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
bool afNewProjectDialog::storeProjectSettings()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSettingsTree != nullptr) && (m_pApplicationCommands != nullptr))
    {
        // Get the project manager instance:
        afProjectManager& projectManager = afProjectManager::instance();

        // If a new project was created, and there is an existing project, close it:
        if ((m_dialogMode == AF_DIALOG_NEW_PROJECT) && (!projectManager.currentProjectSettings().projectName().isEmpty()))
        {
            m_pApplicationCommands->OnFileCloseProject(false);
        }

        // Set the current settings:
        projectManager.setCurrentProject(m_projectSettings);

        // Get the amount of extension pages and amount of extensions:
        int amountOfExtensions = projectManager.amountOfProjectExtensions();

        retVal = true;

        for (int i = 0; i < amountOfExtensions; i++)
        {
            // Save the project settings for each of the extensions:
            bool rc = projectManager.saveCurrentProjectData(i);
            retVal = retVal && rc;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::fillGeneralPageData
// Description: Fill the data in the general page with the current settings
// Author:      Sigal Algranaty
// Date:        10/4/2012
// ---------------------------------------------------------------------------
void afNewProjectDialog::fillGeneralPageData()
{
    // Set the project name text:
    GT_IF_WITH_ASSERT(m_pProjectNameTextEdit != nullptr)
    {
        m_pProjectNameTextEdit->setText(QString::fromWCharArray(m_projectSettings.projectName().asCharArray()));

        QPalette pal;
        pal.setColor(QPalette::Text, Qt::black);
        m_pProjectNameTextEdit->setPalette(pal);
    }

    // Restore daemon configurations.
    bool isRemoteSession = m_projectSettings.isRemoteTarget();
    GT_IF_WITH_ASSERT(m_pRemoteHostRadioButton != nullptr  && m_pLocalHostRadioButton != nullptr &&
                      m_pRemoteHostPortLineEdit != nullptr && m_pRemoteHostIpLineEdit != nullptr)
    {
        m_pRemoteHostRadioButton->setChecked(isRemoteSession);
        m_pLocalHostRadioButton->setChecked(!isRemoteSession);
        adjustGuiToHostChange(isRemoteSession);

        gtString remoteHostIp = m_projectSettings.remoteTargetName();
        QString remoteHostIpQstr = acGTStringToQString(remoteHostIp);
        m_pRemoteHostIpLineEdit->setText(remoteHostIpQstr);

        gtUInt16 remoteHostPort = m_projectSettings.remoteTargetDaemonConnectionPort();

        if (remoteHostPort == 0)
        {
            remoteHostPort = DAEMON_DEFAULT_PORT;
        }

        gtString gtstrRemoteHostPort = L"";
        gtstrRemoteHostPort.appendFormattedString(L"%d", remoteHostPort);
        QString remoteHostPortQstr = acGTStringToQString(gtstrRemoteHostPort);
        m_pRemoteHostPortLineEdit->setText(remoteHostPortQstr);
    }

    // Set the program executable text:
    GT_IF_WITH_ASSERT(m_pProgramExeRadioButton != nullptr &&
                      m_pWinStoreAppRadioButton != nullptr &&
                      m_pProgramExeTextEdit != nullptr)
    {
        if (m_projectSettings.windowsStoreAppUserModelID().isEmpty() || isRemoteSession)
        {
            m_pProgramExeRadioButton->setChecked(true);
            m_pProgramExeTextEdit->setText(QString::fromWCharArray(m_projectSettings.executablePath().asString().asCharArray()));
        }
        else
        {
            m_pWinStoreAppRadioButton->setChecked(true);
            m_pProgramExeTextEdit->setText(QString::fromWCharArray(m_projectSettings.windowsStoreAppUserModelID().asCharArray()));
        }

        OnAppTypeRadioButtonSelect();
    }

    // Set the working folder text:
    GT_IF_WITH_ASSERT(m_pWorkingFolderTextEdit != nullptr)
    {
        m_pWorkingFolderTextEdit->setText(QString::fromWCharArray(m_projectSettings.workDirectory().asString().asCharArray()));
    }

    // Set the program arguments text:
    GT_IF_WITH_ASSERT(m_pProgramArgs != nullptr)
    {
        m_pProgramArgs->setText(QString::fromWCharArray(m_projectSettings.commandLineArguments().asCharArray()));
    }

    GT_IF_WITH_ASSERT(m_pSourceFilesDirectoryTextEdit != nullptr)
    {
        // Set the kernel source code directory:
        m_pSourceFilesDirectoryTextEdit->setText(acGTStringToQString(m_projectSettings.SourceFilesDirectories()));
    }

    GT_IF_WITH_ASSERT(m_pSourceCodeRootDirectoryTextEdit != nullptr)
    {
        // Set the kernel source code directory:
        m_pSourceCodeRootDirectoryTextEdit->setText(acGTStringToQString(m_projectSettings.SourceCodeRootLocation()));
    }

    // Set the environmental variables text:
    GT_IF_WITH_ASSERT(m_pEnvironmentVariablesTextEdit != nullptr)
    {
        gtString environmentVariableAsString;
        m_projectSettings.environmentVariablesAsString(environmentVariableAsString);
        m_pEnvironmentVariablesTextEdit->setText(QString::fromWCharArray(environmentVariableAsString.asCharArray()));
    }
}

// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::OnRestoreDefaultSettings
// Description: Restore default settings for each of the extensions
// Author:      Sigal Algranaty
// Date:        11/4/2012
// ---------------------------------------------------------------------------
void afNewProjectDialog::OnRestoreDefaultSettings(bool keepProjectUnchanged)
{
    m_initializingData = true;

    // Sanity check
    GT_IF_WITH_ASSERT(m_pSettingsTree != nullptr)
    {
        // Call the project manager to restore all the extensions default settings:
        afProjectManager::instance().restoreDefaultExtensionsProjectSettings();

        // Sanity check:
        GT_IF_WITH_ASSERT((m_pProjectNameTextEdit != nullptr) &&
                          (m_pProgramExeTextEdit != nullptr) && (m_pProgramExeRadioButton != nullptr) &&
                          (m_pWorkingFolderTextEdit != nullptr) && (m_pProgramArgs != nullptr) &&
                          (m_pEnvironmentVariablesTextEdit != nullptr) &&
                          (m_pSourceFilesDirectoryTextEdit != nullptr) && (m_pSourceCodeRootDirectoryTextEdit != nullptr) &&
                          (m_pSettingsTree != nullptr) && (m_pRemoteHostRadioButton != nullptr) && (m_pLocalHostRadioButton != nullptr))
        {
            if (keepProjectUnchanged)
            {
                OnExeEditingFinished();
            }
            else
            {
                // Set text default values:
                QString defaultProjectName = afApplicationCommands::instance()->FindDefaultProjectName();
                m_pProjectNameTextEdit->setText(defaultProjectName);
                QPalette pal;
                pal.setColor(QPalette::Text, acQGREY_TEXT_COLOUR);
                m_pProjectNameTextEdit->setPalette(pal);
                m_pProgramExeTextEdit->setText("");
                m_pWorkingFolderTextEdit->setText("");

                // Local session is the default.
                m_pRemoteHostRadioButton->setChecked(false);
                m_pLocalHostRadioButton->setChecked(true);

                m_pProgramExeRadioButton->setChecked(true);
                OnAppTypeRadioButtonSelect();
            }

            m_pProgramArgs->clear();
            m_pEnvironmentVariablesTextEdit->clear();
            m_pSourceFilesDirectoryTextEdit->clear();
            m_pSourceCodeRootDirectoryTextEdit->clear();

            // Select the general page:
            SelectTreeItemByTreePath(AF_STR_newProjectGeneralTabName);

            // Restore default daemon port.
            GT_IF_WITH_ASSERT(m_pRemoteHostPortLineEdit != nullptr &&
                              m_pRemoteHostIpLineEdit != nullptr &&
                              m_pRemoteHostRadioButton != nullptr  &&
                              m_pLocalHostRadioButton != nullptr)
            {
                m_pRemoteHostPortLineEdit->setText(AF_STR_newProject_remoteHostDefaultPort);
                m_pRemoteHostIpLineEdit->setText("");
                m_pLocalHostRadioButton->setChecked(true);
                m_pRemoteHostPortLineEdit->setEnabled(false);
                m_pRemoteHostAddressLabel->setEnabled(false);
                adjustGuiToHostChange(false);
            }
        }
    }

    m_initializingData = false;
}


// ---------------------------------------------------------------------------
// Name:        isValidPortNumber
// Description: Checks if the remote daemon configurations are valid (auxiliary function)
// Arguments:   invalidMessageStr - a buffer to fill with the error message in case of invalid settings.
// Return Val:  bool - Valid / Invalid
// Author:      Amit Ben-Moshe
// Date:        26/08/2013
// ---------------------------------------------------------------------------
bool afNewProjectDialog::isValidRemoteSettings(gtString& invalidMessageStr) const
{
    bool ret = false;

    GT_IF_WITH_ASSERT(m_pRemoteHostIpLineEdit != nullptr &&
                      m_pRemoteHostPortLineEdit != nullptr)
    {
        QString qstrHostIp = m_pRemoteHostIpLineEdit->text().trimmed();
        gtString hostIp = acQStringToGTString(qstrHostIp);

        if (hostIp.isEmpty())
        {
            invalidMessageStr = AF_STR_newProject_remoteHostInvalidAddress;
        }
        else
        {
            QString qstrDmnPort = m_pRemoteHostPortLineEdit->text().trimmed();
            gtString dmnPort = acQStringToGTString(qstrDmnPort);

            if (!dmnPort.isIntegerNumber())
            {
                invalidMessageStr = AF_STR_newProject_remoteHostInvalidPort;
            }
            else
            {
                unsigned int portNum = 0;
                dmnPort.toUnsignedIntNumber(portNum);

                if (!(portNum > 0 && portNum < 65536))
                {
                    invalidMessageStr = AF_STR_newProject_remoteHostInvalidPort;
                }
                else
                {
                    ret = true;
                }
            }
        }
    }
    return ret;
}

bool afNewProjectDialog::AreSettingsValid(gtString& invalidMessageStr, gtString& invalidExtensionTreePath) const
{
    bool retVal = true;

    if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        // If this is the standalone version, read the settings from the general page:
        GT_IF_WITH_ASSERT((m_pProjectNameTextEdit != nullptr) &&
                          (m_pProgramExeRadioButton != nullptr) &&
                          (m_pProgramExeTextEdit != nullptr) &&
                          (m_pWinStoreAppRadioButton != nullptr) &&
                          (m_pWorkingFolderTextEdit != nullptr) &&
                          (m_pRemoteHostRadioButton != nullptr) &&
                          (m_pLocalHostRadioButton != nullptr))
        {
            // Verify remote configurations (if relevant).
            const bool isRemoteSession = m_pRemoteHostRadioButton->isChecked();
            retVal = (!isRemoteSession || isValidRemoteSettings(invalidMessageStr));
            GT_IF_WITH_ASSERT(retVal)
            {
                bool isAppValid = true;
                bool isWorkingFolderValid = true;
                IsApplicationPathsValid(isAppValid, isWorkingFolderValid);


                if (!isAppValid)
                {
                    retVal = false;

                    if (m_pWinStoreAppRadioButton->isChecked())
                    {
                        invalidMessageStr = AF_STR_newProjectWindowsStoreAppInvalid;
                    }
                    else
                    {
                        invalidMessageStr = AF_STR_newProjectExeDoesNotExistOrInvalid;
                    }
                }
                else if (!isWorkingFolderValid)
                {
                    retVal = false;
                    invalidMessageStr = AF_STR_newProjectWorkingFolderDoesNotExist;
                }
                else if ((m_pProjectNameTextEdit->text().isEmpty()) || (m_pProjectNameTextEdit->text() == AF_STR_newProjectEnterName))
                {
                    retVal = false;
                    invalidMessageStr = AF_STR_newProjectInvalidName;
                }

                int pos = 0;
                QString projectNameText = m_pProjectNameTextEdit->text();

                if (m_pProjectNameTextEdit->validator()->validate(projectNameText, pos) == QValidator::Invalid)
                {
                    retVal = false;
                    invalidMessageStr = AF_STR_newProjectInvalidNameInvalidChars;
                }
                else if (m_pProjectNameTextEdit->text().isEmpty())
                {
                    retVal = false;
                    invalidMessageStr = AF_STR_newProjectInvalidName;
                }

                // If the project name had changed, warn the user that data that belong to this project will not be visible in this project
                else if ((m_pProjectNameTextEdit->text() != acGTStringToQString(m_projectSettings.projectName())) && !m_projectSettings.projectName().isEmpty())
                {
                    if (m_dialogMode == AF_DIALOG_EDIT_PROJECT)
                    {
                        // Check if the project contain an existing data on disk.
                        gtString existingDataTypes;
                        bool doesProjectContainData = afProjectManager::instance().DoesProjectContainData(m_projectSettings.projectName(), existingDataTypes);

                        if (doesProjectContainData)
                        {
                            // Build the message to the user that contain the existing data types for this project,
                            QString message = QString(AF_STR_newProjectProjectNameChanged).arg(acGTStringToQString(existingDataTypes));
                            int userAnswer = acMessageBox::instance().warning(afGlobalVariablesManager::ProductNameA(), message, QMessageBox::Yes | QMessageBox::No);

                            if (userAnswer == QMessageBox::No)
                            {
                                retVal = false;
                            }
                        }
                    }
                }
            }
        }
    }

    if (retVal)
    {
        // Invalidate in extensions:
        gtString invalidPageTreePath;
        retVal = afProjectManager::instance().areSettingsValid(invalidMessageStr, invalidExtensionTreePath);
    }

    return retVal;
}


void afNewProjectDialog::IsApplicationPathsValid(bool& isAppValid, bool& isWorkingFolderValid) const
{
    isAppValid = true;
    const bool isWInStoreAppRadioButtonChecked = m_pWinStoreAppRadioButton->isChecked();
    const QString workingFolderPath = m_pWorkingFolderTextEdit->text();
    const QString appFilePath = m_pProgramExeTextEdit->text();
    const bool isRemoteSession = m_pRemoteHostRadioButton->isChecked();
    if (isRemoteSession)
    {
        osPortAddress dmnAddress;
        GT_IF_WITH_ASSERT(GetRemotePortAddress(dmnAddress))
        {
            bool isExecutionSuccessfull = CXLDaemonClient::ValidateAppPaths(dmnAddress, acQStringToGTString(appFilePath), acQStringToGTString(workingFolderPath), isAppValid, isWorkingFolderValid);
            GT_ASSERT_EX(isExecutionSuccessfull, L"Failed to check application path for validity");
        }
    }
    else
    {
        if (isWInStoreAppRadioButtonChecked)
        {
            isAppValid = !appFilePath.isEmpty();
        }
        else if (!appFilePath.isEmpty())
        {
            QFile file(appFilePath);
            //for local files: if windows store application just check if file exists, for regular binaries check if it's an executable
            isAppValid = isWInStoreAppRadioButtonChecked? file.exists() : QFileInfo(file).isExecutable();
        }

        QDir dir(workingFolderPath);
        isWorkingFolderValid = dir.exists() || workingFolderPath.isEmpty();
    }
    
}


bool afNewProjectDialog::GetRemotePortAddress(osPortAddress& dmnAddress) const
{
    bool result = false;
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pRemoteHostIpLineEdit != nullptr) && (m_pRemoteHostPortLineEdit != nullptr))
    {
        // Get the host name and port number from widgets:
        QString qstrHostIp = m_pRemoteHostIpLineEdit->text().trimmed();
        QString qstrHostPort = m_pRemoteHostPortLineEdit->text().trimmed();
        gtString hostIp = acQStringToGTString(qstrHostIp);
        gtString hostPort = acQStringToGTString(qstrHostPort);

        // Convert the port to a number:
        unsigned int hostPortNumber = 0;
        hostPort.toUnsignedIntNumber(hostPortNumber);
        dmnAddress.setAsRemotePortAddress(hostIp, hostPortNumber);
        result = true;
    }
    
    return result;
}

// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::OnProjectNameEdit
// Description: When the user starts editing, clear the text
// Author:      Sigal Algranaty
// Date:        17/4/2012
// ---------------------------------------------------------------------------
void afNewProjectDialog::OnProjectNameEdit(int oldCursor, int newCursor)
{
    GT_UNREFERENCED_PARAMETER(oldCursor);
    GT_UNREFERENCED_PARAMETER(newCursor);

    if (!m_initializingData)
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(m_pProjectNameTextEdit != nullptr)
        {
            if (m_pProjectNameTextEdit->text() == AF_STR_newProjectEnterName)
            {
                m_pProjectNameTextEdit->clear();
                QPalette pal;
                pal.setColor(QPalette::Text, Qt::black);
                m_pProjectNameTextEdit->setPalette(pal);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::OnClickBrowseForSourceFilesDirectory
// Description: Browse for kernel source code folder
// Author:      Sigal Algranaty
// Date:        12/4/2012
// ---------------------------------------------------------------------------
void afNewProjectDialog::OnClickBrowseForSourceFilesDirectory()
{
    GT_IF_WITH_ASSERT(m_pSourceFilesDirectoryTextEdit != nullptr)
    {
        afMultipleDirectoriesBrowseDialog dlg(this);
        dlg.SetDirectoriesList(m_pSourceFilesDirectoryTextEdit->text());

        if (QDialog::Accepted == dlg.exec())
        {
            m_pSourceFilesDirectoryTextEdit->setText(dlg.GetDirectoriesList());
        }
    }
}

void afNewProjectDialog::OnClickBrowseForSourceRootDirectory()
{
    // Get the file selection:
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT((pApplicationCommands != nullptr) && (m_pSourceCodeRootDirectoryTextEdit != nullptr) && (m_pSourceCodeRootDirectoryButtonAction != nullptr))
    {
        // Select the source code root folder:
        QString curreotRootFolder = m_pSourceCodeRootDirectoryTextEdit->text();

        QString sourceCodeRootFolder = pApplicationCommands->ShowFolderSelectionDialog(AF_STR_projectSettingsSelectKernelSourceFilesFolderTitle, curreotRootFolder, m_pSourceCodeRootDirectoryButtonAction);

        // Set the folder in the text widget:
        m_pSourceCodeRootDirectoryTextEdit->setText(sourceCodeRootFolder);
    }
}

/// ------------------------------------------------------------------------------------------------
/// \brief Name:        setRequestedExePath
/// \brief Description: Sets the requested executable path
/// \param[in]          const osFilePath& executablePath
/// \return void
/// ------------------------------------------------------------------------------------------------
void afNewProjectDialog::setRequestedExePath(const gtString& executablePath)
{
    if (!executablePath.isEmpty())
    {
        osFilePath exeFilePath(executablePath);
        GT_IF_WITH_ASSERT((m_pSourceFilesDirectoryTextEdit != nullptr) && (m_pProgramExeTextEdit != nullptr) && (m_pWorkingFolderTextEdit != nullptr) && (m_pProjectNameTextEdit != nullptr))
        {
            // Set the file name:
            m_pProgramExeTextEdit->setText(QString::fromWCharArray(executablePath.asCharArray()));

            QPalette pal;
            pal.setColor(QPalette::Text, acQGREY_TEXT_COLOUR);
            m_pProjectNameTextEdit->setPalette(pal);

            gtString projectName;
            exeFilePath.getFileName(projectName);

            m_pProjectNameTextEdit->setText(QString::fromWCharArray(projectName.asCharArray()));

            osDirectory workingDirectory;
            bool rc = exeFilePath.getFileDirectory(workingDirectory);

            if (rc)
            {
                m_pWorkingFolderTextEdit->setText(QString::fromWCharArray(workingDirectory.directoryPath().asString().asCharArray()));
                m_pSourceFilesDirectoryTextEdit->setText(QString::fromWCharArray(workingDirectory.directoryPath().asString().asCharArray()));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::adjustGuiToHostChange
// Description: Helper function that adjusts the GUI controls to the relevant state: remote session, or local session.
// Arguments:   bool isRemote, indicates whether the session is remote (true) or local (false).
// Author:      Amit Ben-Moshe
// Date:        29/09/2013
// ---------------------------------------------------------------------------
void afNewProjectDialog::adjustGuiToHostChange(bool isRemote)
{
    GT_IF_WITH_ASSERT((m_pRemoteHostIpLineEdit != nullptr) && (m_pRemoteHostPortLabel != nullptr) && (m_pTestConnectionButton != nullptr) &&
                      (m_pRemoteHostPortLineEdit != nullptr) && (m_pRemoteHostAddressLabel != nullptr))
    {
        m_pRemoteHostIpLineEdit->setEnabled(isRemote);
        m_pRemoteHostPortLabel->setEnabled(isRemote);
        m_pRemoteHostPortLineEdit->setEnabled(isRemote);
        m_pRemoteHostAddressLabel->setEnabled(isRemote);
        m_pTestConnectionButton->setEnabled(isRemote);
    }

    if (isRemote)
    {
        // Enable the standard executable selection controls.
        GT_IF_WITH_ASSERT(m_pProgramExeRadioButton != nullptr)
        {
            m_pProgramExeRadioButton->setChecked(true);
        }

        GT_IF_WITH_ASSERT(m_pProgramExeTextEdit != nullptr)
        {
            m_pProgramExeTextEdit->setEnabled(!m_isRunningFromVS);
        }

        // Disable the App Store controls.
        GT_IF_WITH_ASSERT(m_pBrowseForExeButton != nullptr)
        {
            m_pBrowseForExeButton->setEnabled(!m_isRunningFromVS);
        }

        GT_IF_WITH_ASSERT(m_pRemoteHostIpLineEdit != nullptr)
        {
            OnRemoteHostTextChanged(m_pRemoteHostIpLineEdit->text());
        }
    }
    else
    {
        OnRemoteHostTextChanged("");
    }

    // Trigger the signal for other parallel GUI components to adjusts their state.
    afProjectManager::instance().emitGuiChangeRequiredForRemoteSession(isRemote);
}

// ---------------------------------------------------------------------------
// Name:        afNewProjectDialog::OnHostRadioButtonSelection
// Description: Event handler to handle selection of the host radio button
// Author:      Amit Ben-Moshe
// Date:        29/09/2013
// ---------------------------------------------------------------------------
void afNewProjectDialog::OnHostRadioButtonSelection(bool isSelected)
{
    // Check if remote host was selected:
    bool isRemoteSelected = (((sender() == m_pRemoteHostRadioButton) && isSelected) || ((sender() == m_pLocalHostRadioButton) && !isSelected));

    // Adjust the GUI to the host selection:
    adjustGuiToHostChange(isRemoteSelected);
}

void afNewProjectDialog::OnApplicationTypeRadioButtonSelect()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pWinStoreAppRadioButton != nullptr) && (m_pLocalHostRadioButton != nullptr) &&
                      (m_pProgramExeRadioButton != nullptr) && (m_pProgramExeLabel != nullptr) && 
                      (m_pProgramExeTextEdit != nullptr) && (m_pBrowseForExeButton != nullptr))
    {
        bool isExe = m_pProgramExeRadioButton->isChecked();

        // Adjust the Remote Host controls:
        if (!isExe)
        {
            m_pLocalHostRadioButton->setChecked(true);
        }

        // Set the labels and tooltip of the relevant widgets for exe / windows store app selection
        QString tooltip = isExe ? AF_STR_newProjectProjectBrowseForExePathTooltipApp : AF_STR_newProjectProjectBrowseForExePathTooltipWinStoreApp;
        QString editBoxTooltip = isExe ? AF_STR_newProjectProjectEXEEditLineTooltipApp : AF_STR_newProjectProjectStoreAppEditLineTooltipApp;
        QString label = isExe ? AF_STR_newProjectProjectExePath : AF_STR_newProjectWindowsStoreAppUserModelID;

        m_pBrowseForExeButton->setToolTip(tooltip);
        m_pBrowseForExeButtonAction->setToolTip(tooltip);

        m_pProgramExeTextEdit->setToolTip(editBoxTooltip);

        m_pProgramExeLabel->setText(label);

        adjustGuiToHostChange(false);
    }
}

void afNewProjectDialog::SelectTreeItemByTreePath(const QString& selectedTreeFilePath)
{
    // Set the selected tree file path:
    if (!selectedTreeFilePath.isEmpty())
    {
        m_selectedTreeFilePath = selectedTreeFilePath;
    }

    // Get the tree item for this path:
    QTreeWidgetItem* pTreeItem = m_treePathToTreeItemMap[m_selectedTreeFilePath];

    if (pTreeItem == nullptr)
    {
        pTreeItem = m_treePathToTreeItemMap[AF_STR_newProjectGeneralTabName];
    }

    GT_IF_WITH_ASSERT(pTreeItem != nullptr)
    {
        m_pSettingsTree->setCurrentItem(pTreeItem);
    }
}

void afNewProjectDialog::OnSettingsTreeItemChanged()
{
    GT_IF_WITH_ASSERT(m_pSettingsTree != nullptr)
    {
        if (!m_pSettingsTree->selectedItems().isEmpty())
        {
            QTreeWidgetItem* pSelectedItem = m_pSettingsTree->selectedItems().at(0);

            if (pSelectedItem != nullptr)
            {
                QWidget* pWidgetForTreeItem = nullptr;
                QVariant itemData = pSelectedItem->data(0, Qt::UserRole);
                pWidgetForTreeItem = (QWidget*)itemData.value<void*>();
                GT_IF_WITH_ASSERT((pWidgetForTreeItem != nullptr) && (m_pCurrentSettingsFrameLayout != nullptr))
                {
                    m_pCurrentSettingsFrameLayout->setCurrentWidget(pWidgetForTreeItem);
                }

                // Get the tree item for this path:
                for (auto iter = m_treePathToTreeItemMap.begin(); iter != m_treePathToTreeItemMap.end(); iter++)
                {
                    if (iter.value() == pSelectedItem)
                    {
                        m_selectedTreeFilePath = iter.key();
                        break;
                    }
                }
            }
        }
    }
}

void afNewProjectDialog::AddExtensionSettingsPage(const gtString& extensionName, QWidget* pExtensionWidget)
{
    GT_IF_WITH_ASSERT(m_pSettingsTree != nullptr)
    {
        QString pathAsString = acGTStringToQString(extensionName);
        QStringList pathAsList = pathAsString.split(",");
        QTreeWidgetItem* pCurrentParent = nullptr;

        foreach (QString strNodeName, pathAsList)
        {
            QTreeWidgetItem* pNewItem = nullptr;

            if (pCurrentParent == nullptr)
            {
                for (int i = 0; i < m_pSettingsTree->topLevelItemCount(); i++)
                {
                    QTreeWidgetItem* pCurrentItem = m_pSettingsTree->topLevelItem(i);

                    if (pCurrentItem != nullptr)
                    {
                        if (pCurrentItem->text(0) == strNodeName)
                        {
                            pNewItem = pCurrentItem;
                            break;
                        }
                    }
                }
            }
            else
            {
                for (int i = 0; i < pCurrentParent->childCount(); i++)
                {
                    QTreeWidgetItem* pCurrentItem = pCurrentParent->child(i);

                    if (pCurrentItem != nullptr)
                    {
                        if (pCurrentItem->text(0) == strNodeName)
                        {
                            pNewItem = pCurrentItem;
                            break;
                        }
                    }
                }
            }

            if (pNewItem == nullptr)
            {
                QStringList list;
                list << strNodeName;
                pNewItem = m_pSettingsTree->addItem(list, pExtensionWidget, pCurrentParent);
                m_treePathToTreeItemMap[pathAsString] = pNewItem;
            }

            // Set the new parent, so that next items in the list will use it as parent:
            pCurrentParent = pNewItem;
        }
    }

    GT_IF_WITH_ASSERT(m_pCurrentSettingsFrameLayout != nullptr)
    {
        m_pCurrentSettingsFrameLayout->addWidget(pExtensionWidget);
        pExtensionWidget->setStyleSheet(AF_STR_groupBoxWhiteBG);
    }
}

void afNewProjectDialog::OnSettingsTreeItemClicked()
{
    emit SettingsTreeSelectionAboutToChange();
}

void afNewProjectDialog::SetFocusArea(afFocusArea focusArea)
{
    if (focusArea == AF_FOCUS_REMOTE_HOST)
    {
        GT_IF_WITH_ASSERT((m_pRemoteHostRadioButton != nullptr) && (m_pRemoteHostPortLineEdit != nullptr))
        {
            // Check the remote host radio button:
            m_pRemoteHostRadioButton->setChecked(true);
            m_pLocalHostRadioButton->setChecked(false);

            // Select the text in the remote host, to make it focused:
            m_pRemoteHostIpLineEdit->setFocus();
            m_pRemoteHostIpLineEdit->selectAll();
        }
    }
}

void afNewProjectDialog::FitToVisualStudio()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pProjectNameTextEdit != nullptr) && (m_pProgramExeRadioButton != nullptr) && (m_pProgramExeLabel != nullptr) &&
                      (m_pProgramExeTextEdit != nullptr) && (m_pBrowseForExeButton != nullptr) && (m_pWinStoreAppRadioButton != nullptr) && (m_pWorkingFolderTitle != nullptr) && (m_pWorkingFolderTextEdit != nullptr) &&
                      (m_pWorkingFolderPathButton != nullptr) && (m_pProgramArgsTitle != nullptr) && (m_pProgramArgs != nullptr) && (m_pEnvironmentVariablesTitle != nullptr) && (m_pEnvironmentVariablesTextEdit != nullptr) &&
                      (m_pEditEnvironmentVariables != nullptr) && (m_pSourceCodeRootDirectoryTitle != nullptr) && (m_pSourceCodeRootDirectoryTextEdit != nullptr) && (m_pSourceCodeRootDirectoryButton != nullptr))
    {
        if (m_isRunningFromVS)
        {
            m_pProjectNameTextEdit->setEnabled(false);
            m_pProgramExeRadioButton->setEnabled(false);
            m_pProgramExeLabel->setEnabled(false);
            m_pProgramExeTextEdit->setEnabled(false);
            m_pBrowseForExeButton->setEnabled(false);
            m_pWinStoreAppRadioButton->setEnabled(false);
            m_pWorkingFolderTitle->setEnabled(false);
            m_pWorkingFolderTextEdit->setEnabled(false);
            m_pWorkingFolderPathButton->setEnabled(false);
            m_pProgramArgsTitle->setEnabled(false);
            m_pProgramArgs->setEnabled(false);
            m_pEnvironmentVariablesTitle->setEnabled(false);
            m_pEnvironmentVariablesTextEdit->setEnabled(false);
            m_pEditEnvironmentVariables->setEnabled(false);
            m_pSourceCodeRootDirectoryTitle->setEnabled(false);
            m_pSourceCodeRootDirectoryTextEdit->setEnabled(false);
            m_pSourceCodeRootDirectoryButton->setEnabled(false);
        }
    }
}

void afNewProjectDialog::OnTestConnection()
{

    osPortAddress dmnAddress;
    GT_IF_WITH_ASSERT(GetRemotePortAddress(dmnAddress))
    {
        // Perform the connection test:
        bool isExecutionSuccessfull = false;
        bool isConnectivityValid = false;
        isExecutionSuccessfull = CXLDaemonClient::ValidateConnectivity(dmnAddress, isConnectivityValid);


        // Build an output string to the user according to the test connection results:
        QString connectionTestResults = AF_STR_newProject_remoteHostTestConnectionSucceededMessage;

        if (!isExecutionSuccessfull || !isConnectivityValid)
        {
            connectionTestResults = QString(AF_STR_newProject_remoteHostTestConnectionFailedMessage).arg(dmnAddress.hostName().asASCIICharArray()).arg(dmnAddress.portNumber());
        }

        // Popup a message box with the connection test results:
        acMessageBox::instance().information(afGlobalVariablesManager::ProductNameA(), connectionTestResults);
    }
}

void afNewProjectDialog::resizeEvent(QResizeEvent* pResizeEvent)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(QApplication::desktop() != nullptr)
    {
        // Check the screen size, and if the screen is not sufficient for the dimensions that Qt is defining, we should set a smaller size:
        int screenH = QApplication::desktop()->screenGeometry().height();

        if (pResizeEvent->size().height() > screenH - AF_NEW_PROJECT_DIALOG_SCREEN_MARGIN)
        {
            setFixedHeight(screenH - AF_NEW_PROJECT_DIALOG_SCREEN_MARGIN);
        }

        int screenW = QApplication::desktop()->screenGeometry().width();

        if (pResizeEvent->size().width() > screenW - AF_NEW_PROJECT_DIALOG_SCREEN_MARGIN)
        {
            setFixedWidth(screenW - AF_NEW_PROJECT_DIALOG_SCREEN_MARGIN);
        }
    }

    // Call the base class implementation:
    QDialog::resizeEvent(pResizeEvent);
}
