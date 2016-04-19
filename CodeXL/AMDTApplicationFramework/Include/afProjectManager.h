//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afProjectManager.h
///
//==================================================================================

#ifndef __AFPROJECTMANAGER_H
#define __AFPROJECTMANAGER_H

#include <QtWidgets>

// Forward declaration:
class afRecentProjectsActionsExecutor;

// Infra:
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTAPIClasses/Include/apProjectSettings.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/afProjectSettingsExtension.h>

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

// ----------------------------------------------------------------------------------
// Class Name:          afProjectManager
// General Description: A singleton that manages the tool projects
// Author:              Sigal Algranaty
// Creation Date:       3/4/2012
// ----------------------------------------------------------------------------------
class AF_API afProjectManager : public QObject
{
    Q_OBJECT
public:

    static afProjectManager& instance();

    // Current project settings:
    const apProjectSettings& currentProjectSettings() const;
    void setCurrentProject(const apProjectSettings& projectSettings);
    void setProjectSettingsWithoutEvent(const apProjectSettings& projectSettings) { m_currentProject = projectSettings; }

    void setLastActiveMode(const gtString& mode) { m_currentProject.setLastActiveMode(mode); }
    void setLastActiveSessionType(const gtString& sessionType) { m_currentProject.setLastActiveSessionType(sessionType); }
    void setWindowsStoreAppUserModelID(const gtString& userModelId) { m_currentProject.setWindowsStoreAppUserModelID(userModelId); }
    void SetSourceFilesDirectories(const gtString& sourceDirs) {m_currentProject.SetSourceFilesDirectories(sourceDirs);};

    // Project extensions:
    void registerProjectSettingsExtension(afProjectSettingsExtension* pProjectSettingsExtension);
    void registerToListenExeChanged(afProjectSettingsExtension* pProjectSettingsExtension);
    int amountOfProjectExtensions() const { return (int)m_projectSettingsExtensions.size(); }
    QWidget* getExtensionSettingsWidget(int extensionIndex, gtString& extensionDisplayName);
    bool saveCurrentProjectData(int extensionIndex);
    bool currentProjectDataAsXMLString(int extensionIndex, gtString& settingsAsXML);
    bool setCurrentProjectDataFromXMLString(const gtString& extensionName, const gtString& settingsAsXML, bool& wasProjectFound);
    void setRecentlyUsedActionsExecutor(afRecentProjectsActionsExecutor* pRecentlyUsedProjectsManager) { m_pRecentlyUsedProjectsManager = pRecentlyUsedProjectsManager; }
    void restoreDefaultExtensionsProjectSettings();
    bool UpdateRecentlyUsedProjects();
    bool restoreCurrentExtensionSettings();

    /// Checks if the current settings are valid, and
    bool areSettingsValid(gtString& invalidMessageStr, gtString& invalidExtensionTreePath);

    /// Does a project contain data. The function calls the extensions, and check if there is a
    /// saved data on the disk, that is driven from the project name.
    /// The function is used to warn the user for lost data when renaming a project
    /// \param projectName the project for which the saved data on disk should be searched
    /// \param typeOfProjectSavedData[out] the type of data that is saved for the requested project name
    /// \return true iff the project contain data that is saved on disk
    bool DoesProjectContainData(const gtString& projectName, gtString& typeOfProjectSavedData);

    // Project file path:
    void setCurrentProjectFilePath(const osFilePath& filePath);
    const osFilePath& currentProjectFilePath() const { return m_currentProjectFilePath; }

    // Project file path:
    void setOriginalProjectFilePath(const osFilePath& filePath);
    const osFilePath& originalProjectFilePath() const { return m_originalProjectFilePath; }

    /// Emit an executable changed signal, stating that the executable file has been changed:
    /// \param exePath the new executable file path
    /// \param isChangeFinal true iff the change is already set in the current project setting. Otherwise, it's only a GUI change that is not applied yet:
    void EmitExecutableChanged(const QString& exePath, bool isChangeFinal, bool isUserModelId);

    void emitGuiChangeRequiredForRemoteSession(bool isRemoteSession);

    void EmitSourcePathChanged(gtString oldSourcePath, gtString newSourcePath) { emit SourcePathChanged(oldSourcePath, newSourcePath); };

    /// Emit a signal stating that the current project settings should be cleared:
    void EmitClearCurretProjectSettings() { emit OnClearCurrentProjectSettings();}

    /// Remote & local project name utilities:

    /// Add /change the host id in the input project name:
    /// \param origProjectName the original project name
    /// \param hostID the requested host id (can start with '@'. If the '@' is not there, the function will add it)
    /// \return QString with the new project name containing the host ID
    static QString GetProjectNameWithRemoteHost(const QString& origProjectName, const QString& hostID);

    /// Add /change the host id in the input project name:
    /// \param origProjectName the original project name
    /// \param hostID the requested host id
    /// \return QString with the new project name containing the host ID
    static QString GetProjectNameWithLocalHost(const QString& origProjectName);

    /// Add /change the host id in the input project name:
    /// \param projectName the project name to extract the remote host from
    /// \return QString with the project host name
    static QString GetHostFromProjectName(const QString& projectName);

signals:
    // Indicates that the remote host check box has changed
    // its state (checked/unchecked).
    // \param isChecked indicates whether the checkbox's current
    // state is checked or unchecked.
    void OnRemoteHostCheckBoxCheckChange(bool isChecked);

    /// Is emitted when a new project is created, and the current settings should be cleared:
    void OnClearCurrentProjectSettings();

protected:

    /// Fix the samples path to the new CodeXL samples folder
    void FixSamplesPath();


private:
    // Only afSingletonsDelete can delete my instance:
    friend class afSingletonsDelete;

protected:

    static afProjectManager* m_spMySingleInstance;

    afProjectManager();
    ~afProjectManager();

    // The current project:
    apProjectSettings m_currentProject;

    // The current project file path:
    osFilePath m_currentProjectFilePath;

    // The original project file path:
    osFilePath m_originalProjectFilePath;

    // Holds the list of project settings extensions:
    gtPtrVector<afProjectSettingsExtension*> m_projectSettingsExtensions;

    // Recently used projects actions manager:
    afRecentProjectsActionsExecutor* m_pRecentlyUsedProjectsManager;

signals:

    /// Signals that the exe path has change in the current project:
    /// \param exePath the new executable file path
    /// \param isChangeFinal true iff the change is already set in the current project setting. Otherwise, it's only a GUI change that is not applied yet:
    /// \param isWindowsStoreApp true iff the exePath is actually a User Model ID of a Windows Store App.
    void ExecutableChanged(const QString& exePath, bool isChangeFinal, bool isUserModelId);

    /// Signals that the files source path has change in the current project:
    /// \param oldSourcePath the old files source path
    /// \param newSourcePath the new files source path
    void SourcePathChanged(gtString oldSourcePath, gtString newSourcePath);
};

#endif // __AFPROJECTMANAGER_H
