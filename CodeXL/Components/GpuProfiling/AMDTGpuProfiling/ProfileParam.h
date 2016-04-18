//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProfileParam.h $
/// \version $Revision: #5 $
/// \brief :  This file contains Profile parameters
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProfileParam.h#5 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================

#ifndef _PROFILE_PARAM_H_
#define _PROFILE_PARAM_H_

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>

#include "ProfileSettingData.h"
#include <AMDTGpuProfiling/Util.h>


using namespace std;

/// Profile parameter
class ProfileParam
{
public:

    /// Initializes a new instance of the ProfileParam class
    ProfileParam();

    /// Destructor
    ~ProfileParam();

    /// Gets the project name
    /// \return the project name
    QString ProjectName() { return m_strProjectName; }

    /// Gets the session directory
    /// \return the session directory
    QString SessionDir() { return m_strSessionDir; }

    /// Gets the output directory
    /// \return the output directory
    QString OutputDirectory() { return m_strOutputDirectory; }

    /// Gets the session file name
    /// \return the session file name
    QString SessionFile() { return m_strSessionFile; }

    /// Gets the session name
    /// \return the session name
    QString SessionName() { return m_strSessionName; }

    /// Gets the profile type
    /// \return the profile type
    GPUProfileType ProfileTypeValue() { return m_profileTypeValue; }

    /// Gets the current profile setting data
    /// \return the current profile setting data
    ProfileSettingData* CurrentProfileData() { return m_currentProfileData; }

    /// Sets the project name
    /// \param strValue the new project name
    void SetProjectName(const QString& strValue) { m_strProjectName = strValue; }

    /// Sets the session directory
    /// \param strValue the new session directory
    void SetSessionDir(const QString& strValue) { m_strSessionDir = strValue; }

    /// Sets the output directory
    /// \param strValue the new output directory
    void SetOutputDirectory(const QString& strValue) { m_strOutputDirectory = strValue; }

    /// Sets the session file name
    /// \param strValue the new session file name
    void SetSessionFile(const QString& strValue) { m_strSessionFile = strValue; }

    /// Sets the session name
    /// \param strValue the new session name
    void SetSessionName(const QString& strValue) { m_strSessionName = strValue; }

    /// Sets the profile type
    /// \param value the new profile type
    void SetProfileTypeValue(const GPUProfileType& value) { m_profileTypeValue = value; }

private:
    /// Disabled copy constructor
    ProfileParam(const ProfileParam&);

    /// Disabled assignment operator
    ProfileParam& operator=(const ProfileParam&);

    QString m_strProjectName;                 ///< Project name
    QString m_strSessionDir;                  ///< Directory of the session
    QString m_strOutputDirectory;             ///< Output of the session
    QString m_strSessionFile;                 ///< Output file of the session
    QString m_strSessionName;                 ///< Name of the session
    GPUProfileType m_profileTypeValue;           ///< Profile type
    ProfileSettingData* m_currentProfileData; ///< Profile data
};


#endif // _PROFILE_PARAM_H_
