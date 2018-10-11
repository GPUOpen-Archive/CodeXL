//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SharedProfileSettingPage.cpp
///
//==================================================================================

// TinyXml:
#include <tinyxml.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationFramework/Include/afNewProjectDialog.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// Local:
#include <AMDTSharedProfiling/inc/SharedProfileManager.h>
#include <AMDTSharedProfiling/inc/SharedProfileSettingPage.h>
#include <AMDTSharedProfiling/inc/StringConstants.h>

/// Static members initialization
SharedProfileSettingPage* SharedProfileSettingPage::m_psMySingleInstance = nullptr;
QList<gtString> SharedProfileSettingPage::m_profileSessionFileTypes;

/// compare directory based on session index
/// \param sessionOneDir first directory
/// \param sessionTwoDir second directory
/// \return True if first one is having less valued index
bool CompareDirOnSessionIndex(QFileInfo sessionOneDir, QFileInfo sessionTwoDir)
{
    return sessionOneDir.created() < sessionTwoDir.created();
}
SharedProfileSettingPage::SharedProfileSettingPage(): afProjectSettingsExtension(),
    m_pProfileTypeCombo(NULL),
    m_pProfileTypeDescription(NULL)
{
    m_profileTypeToDescriptionMap.insert(PM_profileTypePerformanceCountersPrefix, PM_profileTypeGPUPerformanceCountersDescription);
    m_profileTypeToDescriptionMap.insert(PM_profileTypeApplicationTracePrefix, PM_profileTypeApplicationTraceDescription);

    // Profile sessions
    m_profileSessionFileTypes.append(AF_STR_GpuProfileSessionFileExtension);
    m_profileSessionFileTypes.append(AF_STR_profileFileExtension4);
    m_profileSessionFileTypes.append(AF_STR_GpuProfileTraceFileExtension);

    // Frame analysis sessions
}

SharedProfileSettingPage::~SharedProfileSettingPage()
{
}

void SharedProfileSettingPage::Initialize()
{
    QGridLayout* pMainLayout = new QGridLayout;


    // Create the profile type group box:
    QLabel* pCaption1 = new QLabel(PM_STR_sharedProfileSettingsProfileType);

    pCaption1->setStyleSheet(AF_STR_captionLabelStyleSheetMain);

    int currentRow = 0;
    pMainLayout->addWidget(pCaption1, currentRow, 0, 1, 3);

    m_pProfileTypeCombo = new QComboBox;

    m_pProfileTypeCombo->setMaxVisibleItems(12);
    m_pProfileTypeCombo->setToolTip(PM_STR_sharedProfileSettingsProfileSessionTypeTooltip);

    QLabel* pLabel1 = new QLabel(PM_STR_sharedProfileSettingsProfileSessionType);

    QLabel* pSpacerLabel = new QLabel;

    pSpacerLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    currentRow++;
    pMainLayout->addWidget(pLabel1, currentRow, 0);
    pMainLayout->addWidget(m_pProfileTypeCombo, currentRow, 1, Qt::AlignLeft);
    pMainLayout->addWidget(pSpacerLabel, currentRow, 2, 2, 1);

    m_pProfileTypeDescription = new QLabel;
    m_pProfileTypeDescription->setWordWrap(true);

    currentRow ++;
    pMainLayout->addWidget(m_pProfileTypeDescription, currentRow, 1, 1, 2);

    QLabel* pStretchLabel = new QLabel;

    pStretchLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    currentRow++;
    pMainLayout->addWidget(pStretchLabel, currentRow, 0, 1, 2);

    setLayout(pMainLayout);

    // Connect widgets signals to slots:
    bool rc = connect(m_pProfileTypeCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(OnProfileTypeChanged(const QString&)));
    GT_ASSERT(rc);

    rc = connect(&SharedProfileManager::instance(), SIGNAL(profileSelectionChanged(const gtString&)), this, SLOT(OnProfileTypeChanged(const gtString&)));
    GT_ASSERT(rc);

    //
    RestoreCurrentSettings();
}

gtString SharedProfileSettingPage::ExtensionXMLString()
{
    return PM_STR_SharedProfileExtensionName;
}

gtString SharedProfileSettingPage::ExtensionTreePathAsString()
{
    return PM_STR_SharedProfileExtensionTreePathStr;
}

bool SharedProfileSettingPage::GetXMLSettingsString(gtString& projectAsXMLString)
{
    // NOTICE: The profile session types are only set after the plug ins are loaded.
    // This cannot be performed when this extension is initialized, so it is done here:

    GT_IF_WITH_ASSERT(m_pProfileTypeCombo != NULL)
    {
        // Select the current profile in the combo box:
        QString currentProfileType = acGTStringToQString(SharedProfileManager::instance().currentSelection());
        int index = m_pProfileTypeCombo->findText(currentProfileType);
        m_pProfileTypeCombo->setCurrentIndex(index);
    }

    bool retVal = m_currentSettings.ToXMLString(projectAsXMLString);
    return retVal;
}

bool SharedProfileSettingPage::SetSettingsFromXMLString(const gtString& projectAsXMLString)
{
    bool retVal = false;

    // Read the settings from the XML to the settings structure:
    retVal = m_currentSettings.InitFromXML(projectAsXMLString);

    // Set the settings to the widgets:
    RestoreCurrentSettings();

    emit SharedSettingsUpdated();

    return retVal;
}

void SharedProfileSettingPage::RestoreDefaultProjectSettings()
{
    SessionTreeNodeData restore;
    m_currentSettings = restore;

    RestoreCurrentSettings();
}


bool SharedProfileSettingPage::RestoreCurrentSettings()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pProfileTypeCombo != NULL)
    {
        retVal = true;

        // Set the current settings from the settings structure:
        QString currentProfileType = acGTStringToQString(SharedProfileManager::instance().currentSelection());
        int index = m_pProfileTypeCombo->findText(currentProfileType);

        if (index > 0)
        {
            m_pProfileTypeCombo->setCurrentIndex(index);
        }

        // Set the executable path:
        QString exePath = acGTStringToQString(afProjectManager::instance().currentProjectSettings().executablePath().asString());
    }

    return retVal;
}

bool SharedProfileSettingPage::AreSettingsValid(gtString& invalidMessageStr)
{
    (void)(invalidMessageStr); // unused
    SaveCurrentSettings();
    return true;
}

bool SharedProfileSettingPage::SaveCurrentSettings()
{
    // Set the selected profile type:
    GT_IF_WITH_ASSERT(m_pProfileTypeCombo != NULL)
    {
        gtString currentProfileType = acQStringToGTString(m_pProfileTypeCombo->currentText());
        bool rc = SharedProfileManager::instance().SelectProfileType(currentProfileType);
        GT_ASSERT(rc);

        // Save the properties to the settings structure:
        afNewProjectDialog::instance().setStoredProjectSessionType(currentProfileType);
    }

    RestoreCurrentSettings();

    // Set the current profile action tooltip:
    SharedProfileManager::instance().UpdateProfileMenuItemText();

    return true;
}

bool SharedProfileSettingPage::DoesProjectContainData(const gtString& projectName, gtString& typeOfProjectSavedData)
{
    bool retVal = false;

    typeOfProjectSavedData.makeEmpty();
    // Go over the profile output directory, and look for existing sessions

    // Get the project profile sessions directory:
    osFilePath projectFilePath;
    afGetUserDataFolderPath(projectFilePath);

    apProjectSettings projectSettings = afProjectManager::instance().currentProjectSettings();

    // Add the "ProjectName_ProfileOutput" to the folder:
    gtString projectProfilesLocation = projectName;
    projectProfilesLocation.append(AF_STR_ProfileDirExtension);
    projectFilePath.appendSubDirectory(projectProfilesLocation);

    gtString projectFolderString = projectFilePath.fileDirectoryAsString();
    projectFolderString.append(osFilePath::osPathSeparator);
    QDir projectProfilesQDir(acGTStringToQString(projectFolderString));
    QFileInfoList sessionDirs = projectProfilesQDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time | QDir::Reversed);

    // sort the directories by creation date, so the sessions appear chronologically
    qSort(sessionDirs.begin(), sessionDirs.end(), CompareDirOnSessionIndex);
    bool doesProfileSessionsExist = false;

    foreach (QFileInfo currentSessionDir, sessionDirs)
    {
        QString sessionDirName = currentSessionDir.fileName();

        if (currentSessionDir.isDir())
        {
            QString strOutputDirectory = currentSessionDir.absoluteFilePath();
            QDir dSession(strOutputDirectory);
            QFileInfoList sessionFiles = dSession.entryInfoList();

            foreach (QFileInfo fFile, sessionFiles)
            {
                osFilePath filePath(acQStringToGTString(fFile.fileName()));
                gtString extension;
                filePath.getFileExtension(extension);

                if (m_profileSessionFileTypes.contains(extension))
                {
                    doesProfileSessionsExist = true;
                }
            }
        }
    }

    // If sessions found, build the string describing the type of data saved for this project
    retVal = doesProfileSessionsExist;

    if (retVal)
    {
        if (doesProfileSessionsExist)
        {
            typeOfProjectSavedData.append(PM_profileSessions);
        }
    }

    return retVal;
}

void SharedProfileSettingPage::OnProfileTypeChanged(const QString& currentText)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pProfileTypeDescription != NULL)
    {

        // Emit a signal, so that other profile pages that are dependent on the profile type will handle the change:
        emit ProfileTypeChanged(m_previousProfileType, currentText);
        m_previousProfileType = currentText;

        // Update the description for the current profile session type:
        QString description = m_profileTypeToDescriptionMap[currentText];
        m_pProfileTypeDescription->setText(description);

        // Update the saved properties with the changed scope.
        // NOTICE: We cannot call SaveCurrentSettings here, since SaveCurrentSettings is calling afNewProjectDialog::instance, and this causes
        // a construction of the project dialog with only few of the extensions.
        // Once we create afNewProjectDialog not as a singleton (see http://ontrack-internal.amd.com/browse/CODEXL-1594), this patch can be removed, and replaced by
        // SaveCurrentSettings()
    }
}

void SharedProfileSettingPage::OnProfileTypeChanged(const gtString& profileType)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pProfileTypeCombo != NULL)
    {
        int index = m_pProfileTypeCombo->findText(acGTStringToQString(profileType));

        if (index >= 0)
        {
            m_pProfileTypeCombo->setCurrentIndex(index);
        }
    }
}

void SharedProfileSettingPage::AddProfileType(const QString& profileType)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pProfileTypeCombo != NULL)
    {
        // Add the profile type to the combo box:
        m_pProfileTypeCombo->addItem(profileType);
        m_pProfileTypeCombo->setCurrentIndex(0);
    }
}

SharedProfileSettingPage* SharedProfileSettingPage::Instance()
{
    if (m_psMySingleInstance == NULL)
    {
        m_psMySingleInstance = new SharedProfileSettingPage;
        GT_ASSERT(m_psMySingleInstance);

        afProjectManager::instance().registerProjectSettingsExtension(m_psMySingleInstance);
    }

    return m_psMySingleInstance;
}

