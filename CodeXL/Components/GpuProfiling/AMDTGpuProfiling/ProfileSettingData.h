//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProfileSettingData.h $
/// \version $Revision: #5 $
/// \brief :  This file contains ProfileSettingData parameters
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProfileSettingData.h#5 $
// Last checkin:   $DateTime: 2015/11/18 08:31:35 $
// Last edited by: $Author: gyarnitz $
// Change list:    $Change: 548908 $
//=====================================================================

#ifndef _PROFILE_SETTING_DATA_H_
#define _PROFILE_SETTING_DATA_H_

#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include <QtCore>
#include <QtWidgets>
#include <AMDTAPIClasses/Include/apProjectSettings.h>

#include "Project.h"


/// data for profile setting dialog (currently unused in CodeXL)
class ProfileSettingData
{
public:
    /// Initializes a new instance of the ProfileSettingData class.
    ProfileSettingData();

    /// Initializes a new instance of the ProfileSettingData class from CodeXL project params.
    /// \param projectDir the directory where the project lives
    /// \param projectSettings the project settings for the project
    ProfileSettingData(const osFilePath& projectDir, const apProjectSettings& projectSettings);

    /// Assign one ProfileSettingData to other
    /// \param otherProfileSettingData source data
    void Assign(const ProfileSettingData& otherProfileSettingData);

    /// Gets the env variable list
    /// \return the env variable list
    QStringList EnvVariableList() const { return m_envVariableList; }

    /// Gets the merge environment flag
    /// \return the merge environment flag
    bool MergeEnvironment() const { return m_bMergeEnvironment; }

    /// Gets the application path
    /// \return the application path
    QString ApplicationPath() const { return m_strApplicationPath; }

    /// Gets the application path
    /// \return the application path
    QString WorkingDirectory() const { return m_strWorkingDirectory; }

    /// Gets the command line arguments
    /// \return the command line arguments
    QString CommandlineArguments() const { return m_strCommandlineArguments; }

    /// Gets the platform configuration (in VS plugin)
    /// \return the platform configuration (in VS plugin)
    QString PlatformConfig() const { return m_strPlatformConfig; }

    /// Gets the project info
    /// \return the project info
    gpProject ProjectInfo() const { return m_projectInfo; }

    /// Sets the env variable list
    /// \param value the env variable list
    void SetEnvVariableList(const QStringList& value);

    /// Sets the merge environment flag
    /// \param value the merge environment flag
    void SetMergeEnvironment(const bool value);

    /// Sets the application path
    /// \param value the application path
    void SetApplicationPath(const QString& value);

    /// Sets the application path
    /// \param value the application path
    void SetWorkingDirectory(const QString& value);

    /// Sets the command line arguments
    /// \param value the command line arguments
    void SetCommandlineArguments(const QString& value);

    /// Sets the platform configuration (in VS plugin)
    /// \param value the platform configuration (in VS plugin)
    void SetPlatformConfig(const QString& value);

    /// Gets the project info name
    /// \param value the project info name
    void SetProjectInfoName(const QString& value);

    /// Sets the project info path
    /// \param value the project info path
    void SetProjectInfoPath(const QString& value);

    /// Adds the specified env variable to the env var list
    /// \param value the env var to add
    void AddEnvVariable(const QString& value);

private:
    QStringList m_envVariableList;     ///< environment variable list
    bool m_bMergeEnvironment;          ///< merge environment
    QString m_strApplicationPath;      ///< application path
    QString m_strWorkingDirectory;     ///< working directory
    QString m_strCommandlineArguments; ///< command line arguments
    QString m_strPlatformConfig;       ///< platform config (used in VS plugin only?)
    gpProject m_projectInfo;             ///< project info (name and path)
};

#endif // _PROFILE_SETTING_DATA_H_


