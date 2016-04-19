//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SessionManager.h $
/// \version $Revision: #17 $
/// \brief  This file contains SessionManager class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SessionManager.h#17 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#ifndef _SESSION_MANAGER_H_
#define _SESSION_MANAGER_H_

// Ignore warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qfileinfo.h>


#include <TSingleton.h>

#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>

#include "Session.h"
#include "GlobalSettings.h"

class ProfileApplicationTreeHandler;

class Sessions;

/// Singleton class that manages sessions
class AMDT_GPU_PROF_API SessionManager : public TSingleton<SessionManager>
{

public:
    /// TSingleton needs to be able to use our constructor.
    friend class TSingleton<SessionManager>;

    /// Add a new session to the session list.
    /// \param strName the session name
    /// \param strWorkingDirectory the project working directory
    /// \param strSessionOutputFile the session's main output file
    /// \param strProjName project name
    /// \param profileType the type of profile performed
    /// \param isImported flag indicating whether or not this session is imported
    /// \return Newly created session
    GPUSessionTreeItemData* AddSession(const QString& strName, const QString& strWorkingDirectory,
                                       const QString& strSessionOutputFile, const QString& strProjName, GPUProfileType profileType, bool isImported);

    /// Add a session from a file
    /// \param strSessionName the name of the session to add
    /// \param strProjName project name to add the session to
    /// \param profileType the type of the session being added.  If NA_PROFILE_TYPE, then the type will be gleaned from strFileName
    /// \param strFileName name of the session output file
    /// \param strError QString that contains an error message on failure
    /// \return newly created session, NULL if the session could not be added (see strError for reason)
    GPUSessionTreeItemData* AddSessionFromFile(const QString& strSessionName, const QString& strProjName, GPUProfileType profileType,
                                               const QString& strFileName, QString& strError);

    /// Remove a session
    /// \param pSession the session data of the item to remove
    bool RemoveSession(GPUSessionTreeItemData* pSession, bool deleteFilesFromDisk);

    /// Remove all sessions
    /// \param strError string that contains an error message on failure
    /// \return true if all sessions could be removed, false otherwise
    bool RemoveAllSessions(QString& strError);

    /// Load previous session
    /// \param strFullPath full path of the project's output file
    /// \return list of sessions loaded
    QString GetProjectNameFromFullName(const QString& strFullPath);

    /// Load previous session
    /// \param strErrMsg output error message
    /// \return list of sessions loaded
    QList<GPUSessionTreeItemData*> LoadProjectProfileSessions(QString& strErrMsg);

    /// Check and delete session files
    void CheckAndDeleteSessionFiles();

    /// Get profile type from file path
    /// \param strFileName full file path
    /// \return Profile type
    GPUProfileType GetProfileType(const QString& strFileName);

private:
    /// Initializes the singleton instance of the SessionManager class.
    SessionManager();

    /// Destroys the singleton instance of the SessionManager class.
    ~SessionManager();

    ///< Vector containing the sessions:
    gtVector<GPUSessionTreeItemData*> m_sessionsVector;
    ProfileApplicationTreeHandler* m_pProfileTreeHandler;

};

#endif // _SESSION_MANAGER_H_

