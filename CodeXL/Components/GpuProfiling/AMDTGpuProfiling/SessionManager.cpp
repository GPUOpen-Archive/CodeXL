//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SessionManager.cpp $
/// \version $Revision: #45 $
/// \brief :  This file contains SessionManager class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SessionManager.cpp#45 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>
#include <AMDTSharedProfiling/inc/SessionExplorerDefs.h>

// Local:
#include <AMDTGpuProfiling/gpUIManager.h>
#include <AMDTGpuProfiling/gpViewsCreator.h>
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/ListViewWindow.h>
#include <AMDTGpuProfiling/SessionManager.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#include <AMDTGpuProfiling/Util.h>


/// compare directory based on session index
/// \param sessionOneDir first directory
/// \param sessionTwoDir second directory
/// \return True if first one is having less valued index
bool CompareDirOnSessionIndex(QFileInfo sessionOneDir, QFileInfo sessionTwoDir)
{
    return sessionOneDir.created() < sessionTwoDir.created();
}

SessionManager::SessionManager()
{
    m_pProfileTreeHandler = ProfileApplicationTreeHandler::instance();
}

SessionManager::~SessionManager()
{
}

GPUSessionTreeItemData* SessionManager::AddSession(const QString& strSessionDisplayName, const QString& strWorkingDirectory, const QString& strSessionOutputFile,
                                                   const QString& strProjName, GPUProfileType profileType, bool isImported)
{
    GPUSessionTreeItemData* pRetVal = nullptr;
    GT_IF_WITH_ASSERT(m_pProfileTreeHandler != nullptr)
    {
        // Check if the session with this file path exist:
        osFilePath filePath(acQStringToGTString(strSessionOutputFile));
        afApplicationTreeItemData* pItemData = m_pProfileTreeHandler->FindItemByProfileFilePath(filePath);

        if (pItemData == nullptr)
        {
            if (profileType == API_TRACE)
            {
                pRetVal = new TraceSession(strSessionDisplayName, strWorkingDirectory, strSessionOutputFile, strProjName, isImported);
            }
            else if (profileType == PERFORMANCE)
            {
                pRetVal = new PerformanceCounterSession(strSessionDisplayName, strWorkingDirectory, strSessionOutputFile, strProjName, isImported);
            }
            else if (profileType == FRAME_ANALYSIS)
            {
                pRetVal = new gpSessionTreeNodeData(strSessionDisplayName, strWorkingDirectory, strSessionOutputFile, strProjName, isImported);
            }

            // Sanity check:
            GT_IF_WITH_ASSERT(pRetVal != nullptr)
            {
                pRetVal->m_pParentData = new afApplicationTreeItemData(true);
                pRetVal->m_pParentData->m_filePath = filePath;

                // If we are running from VS, create a temporary file
                gpViewsCreator::Instance()->CreateTempPCFile(pRetVal);

                // Set the profile type:
                pRetVal->m_profileTypeStr = Util::GetProfileTypeName(profileType);
                pRetVal->SearchForAdditionalFiles();
                m_sessionsVector.push_back(pRetVal);
            }
        }

    }

    return pRetVal;
}

GPUProfileType SessionManager::GetProfileType(const QString& strFileName)
{
    GPUProfileType profileType = NA_PROFILE_TYPE;

    if (strFileName.endsWith(".csv"))
    {
        profileType = PERFORMANCE;
    }
    else if (strFileName.endsWith(".atp"))
    {
        profileType = API_TRACE;
    }
    else if (strFileName.endsWith("." + acGTStringToQString(AF_STR_frameAnalysisArchivedFileExtension)))
    {
        profileType = FRAME_ANALYSIS;
    }
    else
    {
        // Warning : Header check may fail for any new header string added in backend and
        //           not got added into GeneratedFileHeader.
        QFile f(strFileName);

        if (f.open(QIODevice::ReadOnly))
        {
            int index = 0;
            QTextStream stream(&f);
            QString line = stream.readLine();

            if (line.contains(GeneratedFileHeader::Header[index++]))
            {
                profileType = PERFORMANCE;
            }
            else if (line.contains(GeneratedFileHeader::Header[index++]))
            {
                profileType = API_TRACE;
            }

            while (!stream.atEnd() && (GeneratedFileHeader::Header.count() > index))
            {
                line = stream.readLine();

                if (!line.contains(GeneratedFileHeader::Header[index]))
                {
                    if (!((profileType == PERFORMANCE) &&
                          line.contains(GeneratedFileHeader::Header[++index])))
                    {
                        profileType = NA_PROFILE_TYPE;
                        break;
                    }
                }

                index++;
            }

            f.close();
        }
        else
        {
            //FIXME: Report error
            profileType = NA_PROFILE_TYPE;
        }
    }

    return profileType;
}

GPUSessionTreeItemData* SessionManager::AddSessionFromFile(const QString& strSessionName, const QString& strProjName, GPUProfileType profileType, const QString& strFileName, QString& strError)
{

    GPUSessionTreeItemData* pRetVal = nullptr;

    if (profileType == NA_PROFILE_TYPE)
    {
        profileType = GetProfileType(strFileName);
    }

    if (profileType != NA_PROFILE_TYPE)
    {
        GT_IF_WITH_ASSERT(m_pProfileTreeHandler != nullptr)
        {
            // Check if the session with this file path exist:
            osFilePath filePath(acQStringToGTString(strFileName));
            afApplicationTreeItemData* pItemData = m_pProfileTreeHandler->FindItemByProfileFilePath(filePath);

            if (pItemData != nullptr)
            {
                pRetVal = qobject_cast<GPUSessionTreeItemData*>(pItemData->extendedItemData());
            }
            else
            {
                QFileInfo f(strFileName);
                QString strOutputDirectory = f.path();

                if (!strOutputDirectory.endsWith(QDir::separator()))
                {
                    strOutputDirectory.append(QDir::separator());
                }

                // Add the session:
                pRetVal = AddSession(strSessionName, QString(), strFileName, strProjName, profileType, false);
            }
        }

    }
    else
    {
        strError = "The specified file does not appear to be a valid profile result file";
    }

    return pRetVal;

}

bool SessionManager::RemoveSession(GPUSessionTreeItemData* pSession, bool deleteFilesFromDisk)
{
    bool retVal = true;

    if (deleteFilesFromDisk)
    {
        // Remove gpsession files:
        osFilePath gpSessionFilePath;
        QString stemp;

        if (gpViewsCreator::GetTempPCFile(pSession->m_projectName, pSession->m_displayName, gpSessionFilePath))
        {
            bool rc = Util::RemoveFileOrDirectory(acGTStringToQString(gpSessionFilePath.asString()), stemp);
            retVal = retVal && rc;
        }

        // Delete additional files:
        QFileInfo fileInfo;

        foreach (QString file, pSession->GetAdditionalFiles())
        {
            fileInfo.setFile(file);

            if (!fileInfo.exists())
            {
                continue;
            }

            bool rc = Util::RemoveFileOrDirectory(file, stemp);
            GT_ASSERT(rc);
        }
    }

    // Look for the matching session data:
    int removedItemIndex = -1;

    for (int i = 0 ; i < (int)m_sessionsVector.size(); i++)
    {
        if (pSession == m_sessionsVector[i])
        {
            removedItemIndex = i;
            break;
        }
    }

    if (removedItemIndex >= 0)
    {
        retVal = true;

        // Remove the session data:
        m_sessionsVector.removeItem(removedItemIndex);
    }
    else
    {
        retVal = false;
    }

    return retVal;
}

bool SessionManager::RemoveAllSessions(QString& strError)
{
    bool retVal = true;
    strError.clear();
    QString tmpError;
    osDirectory sessionParentDir;
    gtList<osFilePath> dirEntries;

    for (int i = 0; i < (int)m_sessionsVector.size(); ++i)
    {
        GPUSessionTreeItemData* pSession = m_sessionsVector[i];

        if (pSession != nullptr)
        {
            // Delete the session files
            bool rc = pSession->DeleteSessionFilesFromDisk(tmpError);
            retVal = retVal && rc;
        }
    }

    m_sessionsVector.clear();
    return retVal;
}

QString SessionManager::GetProjectNameFromFullName(const QString& strFullPath)
{
    QFileInfo f(strFullPath);
    return f.baseName();
}

QList<GPUSessionTreeItemData*> SessionManager::LoadProjectProfileSessions(QString& strErrMsg)
{
    QList<GPUSessionTreeItemData*> sessionList;

    // Get the project profile sessions directory:
    osFilePath projectFilePath;
    afGetUserDataFolderPath(projectFilePath);

    apProjectSettings projectSettings = afProjectManager::instance().currentProjectSettings();

    // Add the "ProjectName_ProfileOutput" to the folder:
    gtString projectProfilesLocation = projectSettings.projectName();
    QString projName = acGTStringToQString(projectProfilesLocation);
    projectProfilesLocation += AF_STR_ProfileDirExtension;
    projectFilePath.appendSubDirectory(projectProfilesLocation);

    QString workingDirectory = acGTStringToQString(projectSettings.workDirectory().asString());

    gtString projectFolderString = projectFilePath.fileDirectoryAsString();
    projectFolderString.append(osFilePath::osPathSeparator);
    QDir projectProfilesQDir(acGTStringToQString(projectFolderString));;
    QFileInfoList sessionDirs = projectProfilesQDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time | QDir::Reversed);
    // sort the directories by creation date, so the sessions appear chronologically
    qSort(sessionDirs.begin(), sessionDirs.end(), CompareDirOnSessionIndex);

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
                bool isSessionFile = false;
                GPUProfileType profileType = NA_PROFILE_TYPE;

                if (fFile.fileName().endsWith(GP_CSV_FileExtension))
                {
                    isSessionFile = true;
                    profileType = PERFORMANCE;
                }
                else if (fFile.fileName().endsWith(GP_ATP_FileExtension))
                {
                    isSessionFile = true;
                    profileType = API_TRACE;
                }

                else if (fFile.fileName().endsWith(GP_Dashbord_FileExtension))
                {
                    // For frame analysis we show only session with frames
                    bool doesSessionContainFrames = gpUIManager::Instance()->DoesSessionHasFrames(dSession);
                    isSessionFile = doesSessionContainFrames;
                    profileType = FRAME_ANALYSIS;
                }

                if (isSessionFile)
                {
                    // use the dir name as the default session name (same behavior as APP Profiler)
                    QString strSessionName = dSession.dirName();
                    bool isImported = strSessionName.endsWith("_Imported");

                    if (isImported)
                    {
                        strSessionName.replace("_Imported", "");
                    }

                    QString strOutputFile = fFile.filePath();

                    if (!strOutputDirectory.endsWith(QDir::separator()))
                    {
                        strOutputDirectory.append(QDir::separator());
                    }

                    QString projNameToUse = projName;
                    QDir outputDir(strOutputDirectory);

                    // Create the session item data from the session file path
                    GPUSessionTreeItemData* pSession = AddSession(strSessionName, workingDirectory, strOutputFile, projNameToUse, profileType, isImported);

                    // Fill in the session data from the project settings
                    pSession->m_commandArguments = acGTStringToQString(projectSettings.commandLineArguments());
                    pSession->m_exeFullPath = acGTStringToQString(projectSettings.executablePath().asString());
                    const gtList<osEnvironmentVariable>& envList = projectSettings.environmentVariables();
                    auto iter = envList.begin();

                    for (; iter != envList.end(); iter++)
                    {
                        pSession->m_envVariables.appendFormattedString(L"%ls=%ls;", (*iter)._name.asCharArray(), (*iter)._value.asCharArray());
                    }

                    GT_IF_WITH_ASSERT(pSession != nullptr)
                    {
                        sessionList.append(pSession);
                    }
                    else
                    {
                        QString s = QString("Could not load\"%1\" of \"%2\"\n") .arg(sessionDirName) .arg(strOutputFile);
                        strErrMsg.append(s);
                    }

                    break;
                }
            }

        }
    }

    return sessionList;
}

void SessionManager::CheckAndDeleteSessionFiles()
{
    QMessageBox::StandardButton dlgResult = QMessageBox::No; // default

    if (m_sessionsVector.empty())
    {
        return;
    }

    if (GlobalSettings::Instance()->m_generalOpt.m_delOption == ASK)
    {
        AGP_TODO("Check for CA, since session files means all the files of all the sessions, to user.");
        dlgResult = acMessageBox::instance().question(QString("Delete %1 GPUSessionTreeItemData Files").arg(afGlobalVariablesManager::ProductNameA()),
                                                      "Do you want to delete the session files?",
                                                      QMessageBox::Yes | QMessageBox::No,
                                                      QMessageBox::Yes);
    }

    if ((GlobalSettings::Instance()->m_generalOpt.m_delOption == ALWAYS) ||
        (dlgResult == QMessageBox::Yes))
    {
        QString strError;
        RemoveAllSessions(strError);

        if (GlobalSettings::Instance()->m_generalOpt.m_showDetailDeletion)
        {
            QStringList strList = strError.split(QString("\n"), QString::SkipEmptyParts);

            if (strList.count())
            {
                ListViewWindow ld;
                ld.SetDataAndConfig(
                    "Deleted Profiler GPUSessionTreeItemData Files",
                    "List of deleted files/folders",
                    strList,
                    GlobalSettings::Instance()->m_generalOpt.m_showDetailDeletion);
                ld.exec();
            }
            else
            {
                Util::ShowWarningBox("No session file found for deletion");
            }
        }
    }
}

