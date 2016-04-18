//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afProjectManager.cpp
///
//==================================================================================
// Qt:
#include <QtWidgets>

// infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afRecentProjectsActionsExecutor.h>

// Static members initializations:
afProjectManager* afProjectManager::m_spMySingleInstance = nullptr;

// ---------------------------------------------------------------------------
// Name:        afProjectManager::afProjectManager
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        3/4/2012
// ---------------------------------------------------------------------------
afProjectManager::afProjectManager()
{
}

// ---------------------------------------------------------------------------
// Name:        afProjectManager::~afProjectManager
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        3/4/2012
// ---------------------------------------------------------------------------
afProjectManager::~afProjectManager()
{
    // There is an issue with deleting the settings pages, since they are also Qt windows.
    // Instead, just clear the vector:
    m_projectSettingsExtensions.clear();
}


// ---------------------------------------------------------------------------
// Name:        afProjectManager::instance
// Description: Get singleton instance
// Return Val:  afProjectManager&
// Author:      Sigal Algranaty
// Date:        3/4/2012
// ---------------------------------------------------------------------------
afProjectManager& afProjectManager::instance()
{
    // If my single instance was not created yet - create it:
    if (m_spMySingleInstance == nullptr)
    {
        m_spMySingleInstance = new afProjectManager;
        GT_ASSERT(m_spMySingleInstance);
    }

    return *m_spMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        afProjectManager::registerProjectSettingsExtension
// Description: Get the current project settings. By default, updates them from the
//              current implementation
// Author:      Uri Shomroni
// Date:        21/5/2012
// ---------------------------------------------------------------------------
const apProjectSettings& afProjectManager::currentProjectSettings() const
{
    return m_currentProject;
}

// ---------------------------------------------------------------------------
// Name:        afProjectManager::registerProjectSettingsExtension
// Description: Register an extension for project settings
// Arguments:   afProjectSettingsExtension* pProjectSettingsExtension
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
void afProjectManager::registerProjectSettingsExtension(afProjectSettingsExtension* pProjectSettingsExtension)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pProjectSettingsExtension != nullptr)
    {
        // Initialize the extension:
        pProjectSettingsExtension->Initialize();
        m_projectSettingsExtensions.push_back(pProjectSettingsExtension);
    }
}

// ---------------------------------------------------------------------------
// Name:        afProjectManager::registerToListenExeChanged
// Description: Register an extension for project settings
//              that is informed whenever executble is changed
// Return Val:  void
// Author:      Bhattacharyya Koushik
// Date:        5/10/2013
// ---------------------------------------------------------------------------
void afProjectManager::registerToListenExeChanged(afProjectSettingsExtension* pProjectSettingsExtension)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pProjectSettingsExtension != nullptr)
    {
        QObject::connect(this, SIGNAL(ExecutableChanged(const QString&, bool, bool)), pProjectSettingsExtension, SLOT(OnExecutableChanged(const QString&, bool, bool)));
    }
}
// ---------------------------------------------------------------------------
// Name:        afProjectManager::getExtensionSettingsWidget
// Description: Return the widget handling the project settings for extension with the requested index
// Arguments:   int extensionIndex
// Return Val:  QWidget*
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
QWidget* afProjectManager::getExtensionSettingsWidget(int extensionIndex, gtString& extensionDisplayName)
{
    QWidget* pRetVal = nullptr;

    GT_IF_WITH_ASSERT((extensionIndex >= 0) && (extensionIndex < (int)m_projectSettingsExtensions.size()))
    {
        // Get the extension:
        afProjectSettingsExtension* pExtension = m_projectSettingsExtensions[extensionIndex];
        GT_IF_WITH_ASSERT(pExtension != nullptr)
        {
            pRetVal = pExtension;
            extensionDisplayName = pExtension->ExtensionTreePathAsString();
        }
    }
    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        afProjectManager::saveCurrentProjectData
// Description: Save the project data for the requested extension
// Arguments:   QWidget* pWidget
//              int extensionIndex
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
bool afProjectManager::saveCurrentProjectData(int extensionIndex)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((extensionIndex >= 0) && (extensionIndex < (int)m_projectSettingsExtensions.size()))
    {
        // Get the extension object:
        afProjectSettingsExtension* pExtension = m_projectSettingsExtensions[extensionIndex];
        GT_IF_WITH_ASSERT(pExtension != nullptr)
        {
            retVal = pExtension->SaveCurrentSettings();
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afProjectManager::currentProjectDataAsXMLString
// Description: Return the requested extension settings string as XML
// Arguments:   int extensionIndex
//              const gtString& settingsAsXML
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        5/4/2012
// ---------------------------------------------------------------------------
bool afProjectManager::currentProjectDataAsXMLString(int extensionIndex, gtString& settingsAsXML)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((extensionIndex >= 0) && (extensionIndex < (int)m_projectSettingsExtensions.size()))
    {
        // Get the extension object:
        afProjectSettingsExtension* pExtension = m_projectSettingsExtensions[extensionIndex];
        GT_IF_WITH_ASSERT(pExtension != nullptr)
        {
            retVal = pExtension->GetXMLSettingsString(settingsAsXML);
        }
    }

    return retVal;

}

// ---------------------------------------------------------------------------
// Name:        afProjectManager::setCurrentProjectDataFromXMLString
// Description: Save the current settings for the current extension
// Arguments:   const gtString& extensionName
//              const gtString& settingsAsXML
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/4/2012
// ---------------------------------------------------------------------------
bool afProjectManager::setCurrentProjectDataFromXMLString(const gtString& extensionName, const gtString& settingsAsXML, bool& wasProjectFound)
{
    bool retVal = false;
    wasProjectFound = false;

    // Look for the extension with this extension name:
    for (int i = 0 ; i < (int)m_projectSettingsExtensions.size(); i++)
    {
        // Get the current extension:
        afProjectSettingsExtension* pCurrentExtension = m_projectSettingsExtensions[i];
        GT_IF_WITH_ASSERT(pCurrentExtension != nullptr)
        {
            if (pCurrentExtension->ExtensionXMLString() == extensionName)
            {
                retVal = pCurrentExtension->SetSettingsFromXMLString(settingsAsXML);
                wasProjectFound = true;
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afProjectManager::restoreDefaultExtensionsProjectSettings
// Description: Restore each of the extensions default settings
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/4/2012
// ---------------------------------------------------------------------------
void afProjectManager::restoreDefaultExtensionsProjectSettings()
{
    // Restore each of the extensions settings:
    for (int i = 0 ; i < (int)m_projectSettingsExtensions.size(); i++)
    {
        // Get the current extension:
        afProjectSettingsExtension* pCurrentExtension = m_projectSettingsExtensions[i];
        GT_IF_WITH_ASSERT(pCurrentExtension != nullptr)
        {
            pCurrentExtension->RestoreDefaultProjectSettings();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afProjectManager::setCurrentProject
// Description: Set the current project
// Arguments:   const apProjectSettings& projectSettings
// Author:      Sigal Algranaty
// Date:        8/4/2012
// ---------------------------------------------------------------------------
void afProjectManager::setCurrentProject(const apProjectSettings& projectSettings)
{
    // Save the file:
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {
        // Check if this is another project:
        bool isProjectNameChanged = projectSettings.projectName() != m_currentProject.projectName();

        // Set th project settings:
        m_currentProject = projectSettings;

        // Set the current project file path:
        osFilePath newProjectFilePath;

        if (!m_currentProject.projectName().isEmpty())
        {
            afGetUserDataFolderPath(newProjectFilePath);
            newProjectFilePath.setFileName(m_currentProject.projectName());
            newProjectFilePath.setFileExtension(AF_STR_projectFileExtension);

            // Set the current project:
            setCurrentProjectFilePath(newProjectFilePath);

            // If the project opened is a sample saved in CodeXL older version, fix the sample path to the new samples location
            FixSamplesPath();
        }
        else
        {
            m_currentProjectFilePath.clear();
        }

        // Throw an event if this is a new project:
        if (isProjectNameChanged)
        {
            // Create a global variable changed event:
            afGlobalVariableChangedEvent eve(afGlobalVariableChangedEvent::CURRENT_PROJECT);

            // Trigger variable change event:
            apEventsHandler::instance().handleDebugEvent(eve);
        }

        if (!projectSettings.projectName().isEmpty())
        {
            // Send an execution mode changed event from the manager since the project is new and still does not have last active settings:
            gtString lastSessionType = projectSettings.lastActiveSessionType();
            gtString modeName = projectSettings.lastActiveMode();

            if (modeName.isEmpty())
            {
                modeName = afExecutionModeManager::instance().activeMode()->modeName();
                lastSessionType = afExecutionModeManager::instance().activeMode()->selectedSessionTypeName();
            }

            apExecutionModeChangedEvent executionModeEvent(modeName, lastSessionType);
            apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);
        }


        // If this is an empty project, restore to default settings in extensions:
        if (m_currentProject.projectName().isEmpty())
        {
            restoreDefaultExtensionsProjectSettings();

            afApplicationCommands::instance()->applicationTree()->clearTreeItems(false);
        }

        // Save the XML file:
        pApplicationCommands->OnFileSaveProject();

    }
}


// ---------------------------------------------------------------------------
// Name:        afProjectManager::setCurrentProjectFilePath
// Description: Set the current project file path
// Arguments:   const osFilePath& filePath
// Author:      Sigal Algranaty
// Date:        8/4/2012
// ---------------------------------------------------------------------------
void afProjectManager::setCurrentProjectFilePath(const osFilePath& filePath)
{
    // Set the file path:
    m_currentProjectFilePath = filePath;
}

// ---------------------------------------------------------------------------
// Name:        afProjectManager::setOriginalProjectFilePath
// Description: Set the original project file path
// Arguments:   const osFilePath& filePath
// Author:      Bhattacharyya Koushik
// Date:        8/7/2012
// ---------------------------------------------------------------------------
void afProjectManager::setOriginalProjectFilePath(const osFilePath& filePath)
{
    // Set the file path:
    m_originalProjectFilePath = filePath;
}


// ---------------------------------------------------------------------------
// Name:        afProjectManager::UpdateRecentlyUsedProjects
// Description: Update the recently used project names
// Author:      Sigal Algranaty
// Date:        10/4/2012
// ---------------------------------------------------------------------------
bool afProjectManager::UpdateRecentlyUsedProjects()
{
    bool retVal = false;

    // Sanity check
    GT_IF_WITH_ASSERT(m_pRecentlyUsedProjectsManager != nullptr)
    {
        retVal = m_pRecentlyUsedProjectsManager->UpdateRecentlyUsedProjects();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afProjectManager::areSettingsValid
// Description: Check if current settings are valid
// Arguments:   gtString& invalidMessageStr
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/4/2012
// ---------------------------------------------------------------------------
bool afProjectManager::areSettingsValid(gtString& invalidMessageStr, gtString& invalidExtensionTreePath)
{
    bool retVal = true;

    // Restore each of the extensions settings:
    for (int i = 0 ; i < (int)m_projectSettingsExtensions.size(); i++)
    {
        // Get the current extension:
        afProjectSettingsExtension* pCurrentExtension = m_projectSettingsExtensions[i];
        GT_IF_WITH_ASSERT(pCurrentExtension != nullptr)
        {
            retVal = pCurrentExtension->AreSettingsValid(invalidMessageStr);

            if (!retVal)
            {
                // Break on the first error:
                invalidExtensionTreePath = pCurrentExtension->ExtensionTreePathAsString();
                break;
            }
        }
    }

    return retVal;
}


bool afProjectManager::DoesProjectContainData(const gtString& projectName, gtString& typeOfProjectSavedData)
{
    bool retVal = false;

    typeOfProjectSavedData.makeEmpty();

    // Restore each of the extensions settings:
    for (int i = 0; i < (int)m_projectSettingsExtensions.size(); i++)
    {
        // Get the current extension:
        afProjectSettingsExtension* pCurrentExtension = m_projectSettingsExtensions[i];
        GT_IF_WITH_ASSERT(pCurrentExtension != nullptr)
        {
            gtString dataTypeStr;
            bool rc = pCurrentExtension->DoesProjectContainData(projectName, dataTypeStr);

            if (rc)
            {
                retVal = true;

                if (!typeOfProjectSavedData.isEmpty())
                {
                    // Append the lost data type to the list of data types with a ","
                    typeOfProjectSavedData.append(AF_STR_Comma);
                    typeOfProjectSavedData.append(AF_STR_Space);
                }

                typeOfProjectSavedData.append(dataTypeStr);
            }
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        afProjectManager::restoreCurrentExtensionSettings
// Description: Restores each of the extensions' GUI to reflect the current
//              project settings
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/6/2012
// ---------------------------------------------------------------------------
bool afProjectManager::restoreCurrentExtensionSettings()
{
    bool retVal = true;

    // Restore each of the extensions settings:
    for (int i = 0 ; i < (int)m_projectSettingsExtensions.size(); i++)
    {
        // Get the current extension:
        afProjectSettingsExtension* pCurrentExtension = m_projectSettingsExtensions[i];
        GT_IF_WITH_ASSERT(pCurrentExtension != nullptr)
        {
            bool rc = pCurrentExtension->RestoreCurrentSettings();
            retVal = retVal && rc;
        }
    }

    return retVal;
}

void afProjectManager::EmitExecutableChanged(const QString& exeName, bool isChangeFinal, bool isUserModelId)
{
    emit ExecutableChanged(exeName, isChangeFinal, isUserModelId);
}

void afProjectManager::emitGuiChangeRequiredForRemoteSession(bool isRemoteSession)
{
    // Emit the signal.
    emit OnRemoteHostCheckBoxCheckChange(isRemoteSession);
}

QString afProjectManager::GetProjectNameWithRemoteHost(const QString& origProjectName, const QString& hostID)
{
    // Get the project name with no host:
    QString retVal = GetProjectNameWithLocalHost(origProjectName);

    if (hostID != AF_STR_modeToolbarHostLocal)
    {
        // Append the requested host to the project name:
        if (!hostID.contains(AF_STR_Shtrudel))
        {
            retVal.append(AF_STR_Shtrudel);
        }

        retVal.append(hostID);
    }

    return retVal;
}

QString afProjectManager::GetProjectNameWithLocalHost(const QString& origProjectName)
{
    QString retVal = origProjectName;

    // Find '@' in project name:
    int shtrudelPos = origProjectName.indexOf(AF_STR_Shtrudel);

    if (shtrudelPos >= 0)
    {
        // Chop the host ID:
        retVal.chop(retVal.length() - shtrudelPos);
    }

    return retVal;
}

QString afProjectManager::GetHostFromProjectName(const QString& projectName)
{
    QString retVal = projectName;

    // Find '@' in project name:
    int shtrudelPos = projectName.indexOf(AF_STR_Shtrudel);

    if (shtrudelPos >= 0)
    {
        // Get the string from the shtrudel ahead:
        retVal = retVal.mid(shtrudelPos);
    }
    else
    {
        retVal = AF_STR_modeToolbarHostLocal;
    }

    return retVal;
}

void afProjectManager::FixSamplesPath()
{
    // If the exe name is set, and doesn't exist, we want to fix it, in case of one of our samples
    if (!m_currentProject.executablePath().isEmpty() && !m_currentProject.executablePath().exists())
    {
        // Try to see if the exe name is one of our samples exe names
        afCodeXLSampleID currentSampleID = AF_SAMPLE_NONE;
        gtString exeName;
        m_currentProject.executablePath().getFileName(exeName);

        if (exeName.find(AF_STR_CodeXLTeapotExampleBinaryName) >= 0)
        {
            currentSampleID = AF_TEAPOT_SAMPLE;
        }
        else if (exeName.find(AF_STR_CodeXLMatMulExampleBinaryName) >= 0)
        {
            currentSampleID = AF_MATMUL_SAMPLE;
        }

        if (currentSampleID != AF_SAMPLE_NONE)
        {
            // Ask the user permission to fix the paths
            int userAnswer = acMessageBox::instance().question(afGlobalVariablesManager::instance().ProductNameA(), AF_STR_OldSampleProjectQuestion, QMessageBox::Yes | QMessageBox::No);

            if (userAnswer == QMessageBox::Yes)
            {
                // Get the sample properties
                gtString sampleName, sampleMode, sampleSessionType, sampleDirName, sampleBinaryName, sampleProjectName, buildOptions;
                afApplicationCommands::instance()->GetSampleProperties(currentSampleID, sampleName, sampleMode, sampleSessionType, sampleDirName, sampleBinaryName, sampleProjectName, buildOptions);

                // Build the exe path, working folder and source code directories
                osFilePath samplePath;
                bool rc = samplePath.SetInstallRelatedPath(osFilePath::OS_CODEXL_EXAMPLES_PATH, false);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Find the exe path and res path (in Linux it is different then windows and the string const reflects that):
                    samplePath.appendSubDirectory(sampleDirName);
                    samplePath.appendSubDirectory(AF_STR_CODEXLExampleReleaseDirName);


                    osDirectory clFilesDirectory;
                    osFilePath clFilesPath = samplePath;
                    clFilesPath.appendSubDirectory(AF_STR_CodeXLSampleResourcesDirName);

                    samplePath.setFileName(sampleBinaryName);
                    samplePath.setFileExtension(AF_STR_CodeXLSampleBinaryExtension);

                    // Set the exe path
                    m_currentProject.setExecutablePath(samplePath);
                    m_currentProject.setWorkDirectoryFromString(samplePath.fileDirectoryAsString());
                    m_currentProject.SetSourceFilesDirectories(clFilesPath.fileDirectoryAsString());
                }

                // Build the KA extension strings, with the builds options, compiler type and cl files list:
                // Save the project after fixing the paths
                afApplicationCommands::instance()->OnFileSaveProject();
            }
        }
    }
}