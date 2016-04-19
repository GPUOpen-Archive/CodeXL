//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProfileSettingData.cpp $
/// \version $Revision: #8 $
/// \brief :  This file contains ProfileSettingData
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProfileSettingData.cpp#8 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>

#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>


#include "ProfileSettingData.h"
#include <AMDTGpuProfiling/Util.h>


ProfileSettingData::ProfileSettingData() :
    m_bMergeEnvironment(true)
{
}

ProfileSettingData::ProfileSettingData(const osFilePath& projectDir, const apProjectSettings& projectSettings)
{
    m_strPlatformConfig.clear(); // unused
    m_projectInfo.m_path = acGTStringToQString(projectDir.fileDirectoryAsString());
    m_projectInfo.m_name = acGTStringToQString(projectSettings.projectName());
    m_bMergeEnvironment = true;
    m_strApplicationPath = acGTStringToQString(projectSettings.executablePath().asString());
    m_strWorkingDirectory = acGTStringToQString(projectSettings.workDirectory().asString());
    m_strCommandlineArguments = acGTStringToQString(projectSettings.commandLineArguments());

    for (gtList<osEnvironmentVariable>::const_iterator i = projectSettings.environmentVariables().begin(); i != projectSettings.environmentVariables().end(); ++i)
    {
        gtString envVarDef = i->_name;

        if (!i->_value.isEmpty())
        {
            envVarDef.append(L"=").append(i->_value);
        }

        m_envVariableList.push_back(acGTStringToQString(envVarDef));
    }
}

void ProfileSettingData::Assign(const ProfileSettingData& otherProfileSettingData)
{
    m_strPlatformConfig = otherProfileSettingData.PlatformConfig();
    m_projectInfo.m_path = otherProfileSettingData.ProjectInfo().m_path;
    m_projectInfo.m_name = otherProfileSettingData.ProjectInfo().m_name;
    m_bMergeEnvironment = otherProfileSettingData.MergeEnvironment();
    m_strApplicationPath = otherProfileSettingData.ApplicationPath();
    m_strWorkingDirectory = otherProfileSettingData.WorkingDirectory();
    m_strCommandlineArguments = otherProfileSettingData.CommandlineArguments();

    m_envVariableList.clear();

    foreach (QString s, otherProfileSettingData.EnvVariableList())
    {
        m_envVariableList.append(s);
    }
}

void ProfileSettingData::SetEnvVariableList(const QStringList& value)
{
    m_envVariableList.clear();
    m_envVariableList.append(value);
}

void ProfileSettingData::SetMergeEnvironment(const bool value)
{
    m_bMergeEnvironment = value;
}

void ProfileSettingData::SetApplicationPath(const QString& value)
{
    m_strApplicationPath = value;
}

void ProfileSettingData::SetWorkingDirectory(const QString& value)
{
    m_strWorkingDirectory = value;
}

void ProfileSettingData::SetCommandlineArguments(const QString& value)
{
    m_strCommandlineArguments = value;
}

void ProfileSettingData::SetPlatformConfig(const QString& value)
{
    m_strPlatformConfig = value;
}

void ProfileSettingData::SetProjectInfoName(const QString& value)
{
    m_projectInfo.m_name = value;
}

void ProfileSettingData::SetProjectInfoPath(const QString& value)
{
    m_projectInfo.m_path = value;
}

void ProfileSettingData::AddEnvVariable(const QString& value)
{
    m_envVariableList << value;
}



