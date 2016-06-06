//------------------------------ gpUIManager.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QDomDocument>


// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// Local:
#include <AMDTGpuProfiling/gpExecutionMode.h>
#include <AMDTGpuProfiling/gpUIManager.h>
#include <AMDTGpuProfiling/gpSessionView.h>
#include <AMDTGpuProfiling/gpStringConstants.h>
#include <AMDTGpuProfiling/gpTraceDataModel.h>
#include <AMDTGpuProfiling/gpViewsCreator.h>
#ifdef GP_OBJECT_VIEW_ENABLE
    #include <AMDTGpuProfiling/gpObjectDataModel.h>
#endif
#include <AMDTGpuProfiling/gpTreeHandler.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#include <AMDTGpuProfiling/Session.h>
#include <AMDTRemoteClient/Include/CXLDaemonClient.h>
#include <AMDTOSWrappers/Include/osProcess.h>

/// compare directory based on session index
/// \param sessionOneDir first directory
/// \param sessionTwoDir second directory
/// \return True if first one is having less valued index
static bool CompareFrameDirOnDate(QFileInfo dir1, QFileInfo dir2)
{
    return dir1.created() < dir2.created();
}

FrameIndex FrameIndexFromString(const QString& frameIndexStr)
{
    FrameIndex retVal(-1, -1);
    int separatorPos = frameIndexStr.indexOf('-');
    bool rc = false;
    if (separatorPos > 0)
    {
        retVal.first = frameIndexStr.mid(0, separatorPos).toInt(&rc);
        GT_IF_WITH_ASSERT(rc)
        {
            retVal.second = frameIndexStr.mid(separatorPos + 1, frameIndexStr.size() - 1).toInt(&rc);
        }
    }
    else
    {
        retVal.first = frameIndexStr.toInt(&rc);
        retVal.second = retVal.first;
    }
    return retVal;
}

QString FrameIndexToString(FrameIndex frameIndex)
{
    QString retVal;
    if (frameIndex.first == frameIndex.second)
    {
        retVal = QString("%1").arg(frameIndex.first);
    }
    else
    {
        retVal = QString("%1-%2").arg(frameIndex.first).arg(frameIndex.second);
    }
    return retVal;
}


gpUIManager::gpUIManager() : m_pCurrentlyRunningSessionData(nullptr), m_updaterThread(this)
{
    bool rc = connect(&m_updaterThread, SIGNAL(CapturedFrameUpdateReady(int, int)), this, SLOT(OnFrameCapture(int, int)), Qt::DirectConnection);
    GT_ASSERT(rc);

    // Set the dashed line pen properties
    m_dashedLinePen.setColor(acQAMD_GRAY1_COLOUR);
    m_dashedLinePen.setWidth(2);
    QVector<qreal> dashes;
    dashes << 2 << 3;
    m_dashedLinePen.setDashPattern(dashes);
}

gpUIManager::~gpUIManager()
{
    m_updaterThread.terminate();
}


bool gpUIManager::DoesSessionHasFrames(const QDir& sessionDir)
{
    bool retVal = false;

    QFileInfoList framesDirsList = sessionDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time | QDir::Reversed);

    foreach (QFileInfo currentFrameDir, framesDirsList)
    {
        QString dirName = currentFrameDir.baseName();

        if (dirName.contains(GPU_STR_FrameSubFolderNamePrefix))
        {
            retVal = true;
            break;
        }
    }

    return retVal;
}

bool gpUIManager::SaveCurrentSessionFrameDataOnDisk(const FrameInfo& frameInfo)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pCurrentlyRunningSessionData != nullptr) && (m_pCurrentlyRunningSessionData->m_pParentData != nullptr))
    {
        // Get the file paths for this frame
        osFilePath sessionFilePath = m_pCurrentlyRunningSessionData->m_pParentData->m_filePath;
        QDir frameDir;
        QString overviewFilePath, thumbnailFilePath;
        FrameIndex frameIndex(frameInfo.m_frameIndex, frameInfo.m_frameIndex + frameInfo.m_framesCount - 1);
        retVal = gpUIManager::GetPathsForFrame(sessionFilePath, frameIndex, frameDir, overviewFilePath, thumbnailFilePath);
        GT_IF_WITH_ASSERT(retVal)
        {
            // Write the XML string to the disk for offline session load
            QFile ovrFileHandle(overviewFilePath);
            QString infoAsXmlString;
            gpUIManager::FrameInfoToXML(frameInfo, infoAsXmlString);

            // Open the file for write:
            if (ovrFileHandle.open(QFile::WriteOnly | QFile::Truncate))
            {
                QTextStream streamData(&ovrFileHandle);
                streamData << infoAsXmlString;
                ovrFileHandle.close();
            }

            // Open the file for write. Write an empty file. Later, when we will try to open the file, we will get it from the server
            QString ltrFilePath = overviewFilePath;
            ltrFilePath.replace(GP_OVR_FileExtension, GP_LTR_FileExtension);
            QFile atrFileHandle(ltrFilePath);

            if (atrFileHandle.open(QFile::WriteOnly | QFile::Truncate))
            {
                atrFileHandle.close();
            }

#ifdef GP_OBJECT_VIEW_ENABLE
            // write the object file, empty file, "aor", need this to turn on the tree item
            ltrFilePath.replace(GP_LTR_FileExtension, GP_AOR_FileExtension);
            atrFileHandle.setFileName(ltrFilePath);

            if (atrFileHandle.open(QFile::WriteOnly | QFile::Truncate))
            {
                atrFileHandle.close();
            }

#endif
        }
    }

    return retVal;
}

void gpUIManager::PrepareTraceData(const osFilePath& traceFilePath, gpBaseSessionView* pTraceView, GPUSessionTreeItemData* pSessionData)
{
    if (traceFilePath.IsMatchingExtension(GP_LTR_FileExtensionW))
    {
        bool wasParseCanceled = false;
        gpTraceDataModel* pDataModel = nullptr;

        if (m_traceFilePathToDataModelMap.find(traceFilePath) != m_traceFilePathToDataModelMap.end())
        {
            pDataModel = m_traceFilePathToDataModelMap[traceFilePath];
        }

        if (pDataModel == nullptr)
        {
            // Create a new data model
            pDataModel = new gpTraceDataModel(pSessionData);

            // Insert the session window to the map
            m_traceFilePathToDataModelMap[traceFilePath] = pDataModel;

            // Load the session data
            gpTraceDataContainer* pTraceSessionDataContainer = nullptr;
            bool rc = pDataModel->LoadTraceFile(traceFilePath, wasParseCanceled, pTraceSessionDataContainer);
            GT_ASSERT(rc);

            if (wasParseCanceled)
            {
                // Close the file
                afApplicationCommands::instance()->closeFile(traceFilePath);
                afProgressBarWrapper::instance().hideProgressBar();
            }
            else
            {
                // Set the data container
                pTraceView->SetProfileDataModel(pDataModel);
            }
        }
    }
}

#ifdef GP_OBJECT_VIEW_ENABLE
void gpUIManager::PrepareObjectData(const osFilePath& objectFilePath, gpBaseSessionView* pObjectView, GPUSessionTreeItemData* pSessionData)
{
    GT_UNREFERENCED_PARAMETER(pSessionData);
    GT_UNREFERENCED_PARAMETER(pObjectView);

    if (objectFilePath.IsMatchingExtension(GP_AOR_FileExtensionW))
    {
        bool wasParseCanceled = false;

        gpObjectDataModel* pDataModel = nullptr;

        if (m_objectFilePathToDataModelMap.find(objectFilePath) != m_objectFilePathToDataModelMap.end())
        {
            pDataModel = m_objectFilePathToDataModelMap[objectFilePath];
        }

        if (pDataModel == nullptr)
        {
            // Create a new data model
            pDataModel = new gpObjectDataModel(pSessionData);

            // Insert the session window to the map
            m_objectFilePathToDataModelMap[objectFilePath] = pDataModel;

            // Load the session data
            gpObjectDataContainer* pObjectSessionDataContainer = nullptr;
            bool rc = pDataModel->LoadObjectFile(objectFilePath, wasParseCanceled, pObjectSessionDataContainer);
            GT_ASSERT(rc);

            if (wasParseCanceled)
            {
                // Close the file
                afApplicationCommands::instance()->closeFile(objectFilePath);
                afProgressBarWrapper::instance().hideProgressBar();
            }
            else
            {
                // Set the data container
                pObjectView->SetProfileObjectDataModel(pDataModel);
            }
        }
    }
}
#endif

void gpUIManager::GetListOfFrameFolders(const osFilePath& sessionFilePath, QList<FrameIndex>& frameIndicesList)
{
    gtString sessiongtDir = sessionFilePath.fileDirectoryAsString();
    sessiongtDir.append(osFilePath::osPathSeparator);

    QDir sessionQDir(acGTStringToQString(sessiongtDir));

    QFileInfoList frameDirs = sessionQDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time | QDir::Reversed);

    // sort the directories by creation date, so the sessions appear chronologically
    qSort(frameDirs.begin(), frameDirs.end(), CompareFrameDirOnDate);

    foreach (QFileInfo currentFrameDir, frameDirs)
    {
        QString currentFrameDirName = currentFrameDir.fileName();

        if (currentFrameDir.isDir())
        {
            // Extract the frame index from the frame folder name
            QString folderName = currentFrameDir.baseName();
            folderName.remove(GPU_STR_FrameSubFolderNamePrefix);

            FrameIndex frameIndex = FrameIndexFromString(folderName);

            if (frameIndex.first >= 0)
            {
                frameIndicesList << frameIndex;
            }
        }
    }

    // Sort the frame indices
    qSort(frameIndicesList.begin(), frameIndicesList.end());
}

int gpUIManager::GetNumberOfFrameFolders(const osFilePath& sessionFilePath)
{
    gtString sessiongtDir = sessionFilePath.fileDirectoryAsString();
    sessiongtDir.append(osFilePath::osPathSeparator);

    QDir sessionQDir(acGTStringToQString(sessiongtDir));

    QFileInfoList frameDirs = sessionQDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time | QDir::Reversed);

    return frameDirs.count();
}

void gpUIManager::OnWindowClose(SharedSessionWindow* pClosedSessionWindow)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pClosedSessionWindow != nullptr)
    {
        // Remove the session window from the models map
        if (m_traceFilePathToDataModelMap.contains(pClosedSessionWindow->SessionFilePath()))
        {
            gpTraceDataModel* pDataModel = m_traceFilePathToDataModelMap[pClosedSessionWindow->SessionFilePath()];
            delete pDataModel;

            m_traceFilePathToDataModelMap.remove(pClosedSessionWindow->SessionFilePath());
        }
    }
}

void gpUIManager::OnFrameCapture(int firstFrameIndex, int lastFrameIndex)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pCurrentlyRunningSessionData != nullptr)
    {
        FrameIndex frameIndex(firstFrameIndex, lastFrameIndex);
        // Add the currently captured frame index to the vector of captured frames
        m_pCurrentlyRunningSessionData->m_capturedFramesIndices << frameIndex;

        emit CapturedFrameUpdateReady(firstFrameIndex, lastFrameIndex);
    }
}

void gpUIManager::OnApplicationEnded()
{
    // Add each of the captured frames to the tree:
    gpSessionTreeNodeData* pRunningSessionData = CurrentlyRunningSessionData();
    GT_IF_WITH_ASSERT(pRunningSessionData != nullptr)
    {
        foreach (FrameIndex frameIndex, pRunningSessionData->m_capturedFramesIndices)
        {
            // Add the captured frames to the tree and expand only the last one
            bool shouldExpand = (frameIndex == pRunningSessionData->m_capturedFramesIndices.last());
            gpTreeHandler::Instance().AddCapturedFrameToTree(frameIndex, shouldExpand);
        }

        pRunningSessionData->m_capturedFramesIndices.clear();

        // Notify users that the application had ended
        emit ApplicationRunEnded(pRunningSessionData);
    }

    // check if m_pCurrentlyRunningSessionData is still valid (not null and was not removed from the tree)
    if (IsSessionDataValid())
    {
        m_pCurrentlyRunningSessionData->m_isSessionRunning = false;
    }

    m_pCurrentlyRunningSessionData = nullptr;

}

void gpUIManager::SetCurrentlyRunningSessionData(gpSessionTreeNodeData* pSessionData)
{
    if (pSessionData != nullptr)
    {
        pSessionData->m_isSessionRunning = true;
    }
    else if (m_pCurrentlyRunningSessionData != nullptr)
    {
        // Un-setting the running data - set running flag on the current data to false
        m_pCurrentlyRunningSessionData->m_isSessionRunning = false;
    }

    // Set the session data
    m_pCurrentlyRunningSessionData = pSessionData;
}

bool gpUIManager::FrameInfoFromXML(const gtASCIIString& frameInfoXML, FrameInfo& frameInfo)
{
    bool retVal = true;
    QDomDocument frameInfoDoc;
    frameInfoDoc.setContent(acGTASCIIStringToQString(frameInfoXML));

    bool rc = false;
    QDomNodeList elementsList = frameInfoDoc.elementsByTagName(GPU_STR_frameInfoXMLFrameNumber);

    if (elementsList.size() == 1)
    {
        frameInfo.m_frameIndex = elementsList.at(0).toElement().text().toInt(&rc);
    }

    retVal = retVal && rc;

    elementsList = frameInfoDoc.elementsByTagName(GPU_STR_frameInfoXMLServerXMLFullPath);

    if (elementsList.size() == 1)
    {
        frameInfo.m_frameInfoXML = elementsList.at(0).toElement().text().toStdString().c_str();
    }

    retVal = retVal && rc;

    elementsList = frameInfoDoc.elementsByTagName(GPU_STR_frameInfoXMLFrameFPS);

    if (elementsList.size() == 1)
    {
        frameInfo.m_fps = (int) elementsList.at(0).toElement().text().toDouble(&rc);
    }

    retVal = retVal && rc;

    elementsList = frameInfoDoc.elementsByTagName(GPU_STR_frameInfoXMLFrameAPICallCount);

    if (elementsList.size() == 1)
    {
        frameInfo.m_apiCalls = elementsList.at(0).toElement().text().toInt(&rc);
    }

    retVal = retVal && rc;

    elementsList = frameInfoDoc.elementsByTagName(GPU_STR_frameInfoXMLFrameDrawCallCount);

    if (elementsList.size() == 1)
    {
        frameInfo.m_drawCalls = elementsList.at(0).toElement().text().toInt(&rc);
    }

    elementsList = frameInfoDoc.elementsByTagName(GPU_STR_frameInfoXMLFrameTracedFramesCount);

    if (elementsList.size() == 1)
    {
        frameInfo.m_framesCount = elementsList.at(0).toElement().text().toInt(&rc);
    }

    retVal = retVal && rc;

    elementsList = frameInfoDoc.elementsByTagName(GPU_STR_frameInfoXMLFrameCPUFrameDuration);

    if (elementsList.size() == 1)
    {
        frameInfo.m_frameDuration = elementsList.at(0).toElement().text().toDouble(&rc);
    }

    elementsList = frameInfoDoc.elementsByTagName(GPU_STR_frameInfoXMLFrameElapsedTime);

    if (elementsList.size() == 1)
    {
        frameInfo.m_elapsedTimeMS = elementsList.at(0).toElement().text().toDouble(&rc);
    }

    retVal = retVal && rc;

#ifdef GP_OBJECT_VIEW_ENABLE
    // store frame storage location
    elementsList = frameInfoDoc.elementsByTagName(GPU_STR_frameInfoXMLLocation);
    GT_IF_WITH_ASSERT(elementsList.size() == 1)
    {
        frameInfo.m_serverFolderPath.assign(elementsList.at(0).toElement().text().toStdWString().c_str());
    }
    retVal = retVal && rc;
#endif

    return retVal;
}


void gpUIManager::FrameInfoToXML(const FrameInfo& frameInfo, QString& xmlString)
{
    QString serverXMLFile = acGTStringToQString(frameInfo.m_descriptionFileRemotePath);
    QString serverLocation = acGTStringToQString(frameInfo.m_serverFolderPath);
    QString traceFilePath = acGTStringToQString(frameInfo.m_traceFileRemotePath);
    xmlString = QString(GPU_STR_frameInfoXMLFileFormat)
                .arg(serverLocation)
                .arg(frameInfo.m_frameIndex)
                .arg(traceFilePath)
                .arg(serverXMLFile)
                .arg(frameInfo.m_elapsedTimeMS)
                .arg(frameInfo.m_fps)
                .arg(frameInfo.m_frameDuration)
                .arg(frameInfo.m_apiCalls)
                .arg(frameInfo.m_drawCalls)
                .arg(frameInfo.m_framesCount);
}

bool gpUIManager::GetFrameInfo(const osFilePath& sessionFilePath, FrameInfo& frameInfo)
{
    bool retVal = false;

    QDir frameDir;
    QString overviewFilePath, thumbnailFilePath;
    gtASCIIString infoXML;
    FrameIndex frameIndex(frameInfo.m_frameIndex, frameInfo.m_frameIndex + frameInfo.m_framesCount - 1);
    retVal = GetPathsForFrame(sessionFilePath, frameIndex, frameDir, overviewFilePath, thumbnailFilePath);
    GT_IF_WITH_ASSERT(retVal)
    {
        QFile fileHandle(overviewFilePath);
        QString infoAsXmlString;

        // Open the file for read
        if (fileHandle.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream in(&fileHandle);
            infoAsXmlString = in.readAll();
            infoXML = infoAsXmlString.toStdString().c_str();
        }

        fileHandle.close();

        // Open the file for write. Write an empty file. Later, when we will try to open the file, we will get it from the server
        QString ltrFilePath = overviewFilePath;
        ltrFilePath.replace(GP_OVR_FileExtension, GP_LTR_FileExtension);
        QFile atrFileHandle(ltrFilePath);

        if (!atrFileHandle.exists())
        {
            if (atrFileHandle.open(QFile::WriteOnly | QFile::Truncate))
            {
                atrFileHandle.close();
            }
        }
    }

    // Extract the frame info from the XML string
    retVal = gpUIManager::FrameInfoFromXML(infoXML, frameInfo);

    frameInfo.m_pImageBuffer = nullptr;
    osFilePath imageFilePath(acQStringToGTString(thumbnailFilePath));

    // load the image and return it in the buffer:
    if (imageFilePath.exists())
    {
        osFile imageFile(imageFilePath);
        retVal = imageFile.getSize(frameInfo.m_imageSize);
        GT_IF_WITH_ASSERT(retVal)
        {
            retVal = imageFile.open(osChannel::OS_BINARY_CHANNEL);
            GT_IF_WITH_ASSERT(retVal)
            {
                frameInfo.m_pImageBuffer = new unsigned char[frameInfo.m_imageSize];
                retVal = imageFile.read((char*)frameInfo.m_pImageBuffer, frameInfo.m_imageSize);
                imageFile.close();
            }
        }
    }

    return retVal;
}

bool gpUIManager::GetPathsForFrame(const osFilePath& sessionFilePath, FrameIndex frameIndex, QDir& frameDir, QString& overviewFilePath, QString& thumbFilePath, bool shouldCreateFrameDir)
{
    bool retVal = false;

    // Calculate the frame directory
    osDirectory frameOSDir;
    retVal = sessionFilePath.getFileDirectory(frameOSDir);
    GT_IF_WITH_ASSERT(retVal)
    {
        osFilePath frameDirPath = frameOSDir.directoryPath();
        gtString frameDirStr, fileName;
        sessionFilePath.getFileName(fileName);

        QString sessionDisplayName = acGTStringToQString(fileName);
        QString overviewFileName, thumbnailFileName;
        if (frameIndex.first == frameIndex.second)
        {
            // Single frame file
            frameDirStr.appendFormattedString(GPU_STR_FrameSubFolderNameSingleFormat, frameIndex.first);

            // Build the overview & thumbnail file names
            overviewFileName = QString(GPU_STR_FrameFileNameFormat).arg(sessionDisplayName).arg(frameIndex.first).arg(GP_Overview_FileExtension);
            thumbnailFileName = QString(GPU_STR_FrameFileNameFormat).arg(sessionDisplayName).arg(frameIndex.first).arg(GP_ThumbnailFileExtension);
        }
        else
        {
            // Multiple frames file
            frameDirStr.appendFormattedString(GPU_STR_FrameSubFolderNameMultipleFormat, frameIndex.first, frameIndex.second);

            // Build the overview & thumbnail file names
            overviewFileName = QString(GPU_STR_FrameMultipleFileNameFormat).arg(sessionDisplayName).arg(frameIndex.first).arg(frameIndex.second).arg(GP_Overview_FileExtension);
            thumbnailFileName = QString(GPU_STR_FrameMultipleFileNameFormat).arg(sessionDisplayName).arg(frameIndex.first).arg(frameIndex.second).arg(GP_ThumbnailFileExtension);
        }
        frameDirPath.appendSubDirectory(frameDirStr);
        frameDirStr = frameDirPath.fileDirectoryAsString();
        frameDirStr.append(osFilePath::osPathSeparator);
        QString frameDirQStr = acGTStringToQString(frameDirStr);
        frameDir = frameDirQStr;

        QFileInfo overviewPath(frameDir, overviewFileName);
        QFileInfo thumbPath(frameDir, thumbnailFileName);

        overviewFilePath = overviewPath.absoluteFilePath();
        thumbFilePath = thumbPath.absoluteFilePath();

        // Create the directory for the frame
        if (shouldCreateFrameDir)
        {
            if (!frameDir.exists())
            {
                retVal = frameDir.mkdir(frameDir.path());
            }
        }
        else
        {
            retVal = true;
        }
    }

    return retVal;
}

bool gpUIManager::GetFrameImageAndInfo(const QString& overviewFilePath, FrameInfo& frameInfo)
{
    bool retVal = false;

    // Get the frame info from the overview XML file path
    QFile fileHandle(overviewFilePath);
    QString infoAsXmlString;

    // Open the file for write
    bool rcOverview = fileHandle.open(QFile::ReadOnly | QFile::Text);

    if (rcOverview)
    {
        QTextStream in(&fileHandle);
        infoAsXmlString = in.readAll();
        gtASCIIString frameInfoXml = infoAsXmlString.toStdString().c_str();
        retVal = gpUIManager::FrameInfoFromXML(frameInfoXml, frameInfo);
        fileHandle.close();
    }

    QString thumbnailFilePath = overviewFilePath;
    thumbnailFilePath.replace(GP_Overview_FileExtension, GP_ThumbnailFileExtension);


    osFile imageFile(acQStringToGTString(thumbnailFilePath));
    bool rcThumb = imageFile.getSize(frameInfo.m_imageSize);

    if (rcThumb)
    {
        rcThumb = imageFile.open(osChannel::OS_BINARY_CHANNEL);

        if (rcThumb)
        {
            frameInfo.m_pImageBuffer = new unsigned char[frameInfo.m_imageSize];
            rcThumb = imageFile.read((char*)frameInfo.m_pImageBuffer, frameInfo.m_imageSize);
            imageFile.close();
        }
    }

    retVal = rcThumb && rcOverview;
    return retVal;
}


bool gpUIManager::CacheProjectSessionsList(const gtString& capturedFramesAsXML)
{
    bool retVal = false;

    gtMap<gtString, gtVector<FrameIndex>> sessionFramesMap;
    retVal = ProjectSessionsMapFromXML(capturedFramesAsXML, sessionFramesMap);
    GT_IF_WITH_ASSERT(retVal)
    {
        // Find the current project profiles folder (CodeXLProjectsFolder/"ProjectName_ProfileOutput"):
        osFilePath projectFilePath;
        afGetUserDataFolderPath(projectFilePath);
        gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();
        gtString projectProfilesLocation = projectName;
        QString projName = acGTStringToQString(projectProfilesLocation);
        projectProfilesLocation += AF_STR_ProfileDirExtension;
        projectFilePath.appendSubDirectory(projectProfilesLocation);

        // Show a progress dialog
        afProgressBarWrapper::instance().ShowProgressDialog(GPU_STR_sessionInfoXMLLoadProgressMessage, sessionFramesMap.size());
        auto iter = sessionFramesMap.begin();
        auto iterEnd = sessionFramesMap.end();

        for (; iter != iterEnd; iter++)
        {
            // Get the session name and list of frames, and make sure that files exist for the session and each of its frames
            gtString sessionName = (*iter).first;

            // Get the list of frames for this session
            const gtVector<FrameIndex>& framesList = (*iter).second;

            if (!framesList.empty())
            {
                // Get the session file path
                osFilePath sessionFilePath = projectFilePath;
                sessionFilePath.appendSubDirectory(sessionName);
                sessionFilePath.setFileName(sessionName);
                sessionFilePath.setFileExtension(GP_Dashbord_FileExtensionW);
                osDirectory sessionDir;
                sessionFilePath.getFileDirectory(sessionDir);

                if (!sessionDir.exists())
                {
                    sessionDir.create();
                }

                if (!sessionFilePath.exists())
                {
                    // If the session file doesn't exist (it means that the session was created by another client), create it
                    osFile sessionFile;
                    bool rc = sessionFile.open(sessionFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

                    if (rc)
                    {
                        sessionFile.writeString(GPU_STR_TreeNodeDashboard);
                        sessionFile.close();
                    }
                }

                // Increment the progress dialog
                afProgressBarWrapper::instance().incrementProgressBar();

                for (int i = 0; i < (int)framesList.size(); i++)
                {
                    FrameIndex currentFrameIndex = framesList[i];
                    QDir frameDir;
                    QString ovrFilePath, thumbFilePath;
                    bool rc = GetPathsForFrame(sessionFilePath, currentFrameIndex, frameDir, ovrFilePath, thumbFilePath);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        // If the overview file doesn't exist (it means that the session was created by another client), create it
                        osFilePath overviewFilePath(acQStringToGTString(ovrFilePath));

                        if (!overviewFilePath.exists())
                        {
                            // Sanity check
                            GT_IF_WITH_ASSERT((ProfileManager::Instance() != nullptr) && (ProfileManager::Instance()->GetFrameAnalysisModeManager() != nullptr))
                            {
                                // Go get the XML description from the server
                                gtString frameAsXML;
                                FrameInfo frameInfo;
                                frameInfo.m_frameIndex = currentFrameIndex.first;
                                frameInfo.m_framesCount = currentFrameIndex.second - currentFrameIndex.first + 1;
                                bool rc = ProfileManager::Instance()->GetFrameAnalysisModeManager()->GetCapturedFrameData(projectName, sessionName, frameInfo);
                                GT_IF_WITH_ASSERT(rc)
                                {
                                    osFile frameFile;
                                    rc = frameFile.open(overviewFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

                                    if (rc)
                                    {
                                        frameFile.writeString(frameInfo.m_frameInfoXML);
                                        frameFile.close();
                                    }

                                    // Open the file for write. Write an empty file. Later, when we will try to open the file, we will get it from the server
                                    osFilePath ltrFilePath = overviewFilePath;
                                    ltrFilePath.setFileExtension(GP_LTR_FileExtensionW);
                                    osFile atrFile;
                                    rc = atrFile.open(ltrFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

                                    if (rc)
                                    {
                                        atrFile.close();
                                    }
                                }
                            }
                        }
                    }
                }

                const apProjectSettings& projectSettings = afProjectManager::instance().currentProjectSettings();
                QString sessionQtName = acGTStringToQString(sessionName);
                QString workingDirectory = acGTStringToQString(projectSettings.workDirectory().asString());
                QString sessionPath = acGTStringToQString(sessionFilePath.asString());

                // Create the session item data from the session file path
                GPUSessionTreeItemData* pSession = SessionManager::Instance()->AddSession(sessionQtName, workingDirectory, sessionPath, sessionQtName, FRAME_ANALYSIS, false);
                GT_IF_WITH_ASSERT(pSession != nullptr)
                {
                    // Fill in the session data from the project settings
                    pSession->m_commandArguments = acGTStringToQString(projectSettings.commandLineArguments());
                    pSession->m_exeFullPath = acGTStringToQString(projectSettings.executablePath().asString());
                    const gtList<osEnvironmentVariable>& envList = projectSettings.environmentVariables();
                    auto iter = envList.begin();

                    for (; iter != envList.end(); iter++)
                    {
                        pSession->m_envVariables.appendFormattedString(L"%ls=%ls;", (*iter)._name.asCharArray(), (*iter)._value.asCharArray());
                    }

                    ProfileManager::Instance()->AddSessionToExplorer(pSession, false, false);
                }
            }
        }

        afProgressBarWrapper::instance().hideProgressBar();

    }
    return retVal;
}

bool gpUIManager::ProjectSessionsMapFromXML(const gtString& capturedFramesAsXML, gtMap<gtString, gtVector<FrameIndex>>& sessionFramesMap)
{
    bool retVal = false;

    QString frameXML = acGTStringToQString(capturedFramesAsXML);
    QDomDocument framesInfoDoc;
    framesInfoDoc.setContent(frameXML);

    QDomNodeList sessionItemsList = framesInfoDoc.elementsByTagName(GPU_STR_sessionInfoXMLSessions);
    GT_IF_WITH_ASSERT(sessionItemsList.size() == 1)
    {
        retVal = true;

        QDomElement allSessionsNode = sessionItemsList.at(0).toElement();
        QDomNodeList sessionsList = allSessionsNode.elementsByTagName(GPU_STR_sessionInfoXMLSession);

        for (int sIndex = 0; sIndex < (int)sessionsList.size(); sIndex++)
        {
            QDomElement sessionElem = sessionsList.at(sIndex).toElement();
            QString sessionName = sessionElem.attribute("name");
            gtString gtSessionName = acQStringToGTString(sessionName);
            gtVector<FrameIndex> framesList;
            QDomNodeList sessionFramesList = sessionElem.elementsByTagName(GPU_STR_sessionInfoXMLFrame);
            int framesCount = (int)sessionFramesList.size();

            for (int fIndex = 0; fIndex < (int)framesCount; fIndex++)
            {
                QDomElement frameElem = sessionFramesList.at(fIndex).toElement();
                QString frameStr = QString::fromStdString(frameElem.attribute(GPU_STR_sessionInfoXMLIndex).toStdString());
                FrameIndex frameIndex = FrameIndexFromString(frameStr);
                
                GT_IF_WITH_ASSERT(frameIndex.first > 0)
                {
                    framesList.push_back(frameIndex);
                    retVal = retVal;
                }
            }

            sessionFramesMap[gtSessionName] = framesList;
        }
    }

    return retVal;
}

void gpUIManager::AddCapturedFramesFromServer(const gtString& capturedFramesAsXML)
{
    gtMap<gtString, gtVector<FrameIndex>> sessionFramesMap;

    bool rc = ProjectSessionsMapFromXML(capturedFramesAsXML, sessionFramesMap);
    GT_IF_WITH_ASSERT(rc && m_pCurrentlyRunningSessionData != nullptr)
    {
        gtString sessionName = acQStringToGTString(m_pCurrentlyRunningSessionData->m_displayName);
        const gtVector<FrameIndex>& frames = sessionFramesMap[sessionName];

        for (auto iter = frames.begin(); iter != frames.end(); iter++)
        {
            if (!m_pCurrentlyRunningSessionData->m_capturedFramesIndices.contains(*iter))
            {
                // Add this captured frame to the list of frames
                FrameIndex index = *iter;
                OnFrameCapture(index.first, index.second);

                // Notify users that a frame update is ready
                emit CapturedFrameUpdateReady(index.first, index.second);
            }
        }
    }
}

void gpUIManager::OnBeforeActiveSubWindowClose(const osFilePath& sessionFilePath, bool& shouldClose)
{
    // If this is the running session:
    if (sessionFilePath == CurrentExecutingFrameAnalysisPath())
    {
        // Ask the user if he wants to close the profile session:
        shouldClose = true;
        int userAnswer = acMessageBox::instance().question(AF_STR_QuestionA, GPU_STR_SessionStopConfirm, QMessageBox::Yes | QMessageBox::No);

        if (userAnswer == QMessageBox::No)
        {
            shouldClose = false;
        }
    }
}

osFilePath gpUIManager::CurrentExecutingFrameAnalysisPath()
{
    osFilePath retPath;

    if (m_pCurrentlyRunningSessionData != nullptr && m_pCurrentlyRunningSessionData->m_pParentData != nullptr)
    {
        // If this is the running session:
        retPath = m_pCurrentlyRunningSessionData->m_pParentData->m_filePath;
    }

    return retPath;
}

bool gpUIManager::IsSessionDataValid()
{
    bool retVal = false;

    if (m_pCurrentlyRunningSessionData != nullptr)
    {
        if (ProfileApplicationTreeHandler::instance()->FindSessionDataByProfileFilePath(m_pCurrentlyRunningSessionData->m_pParentData->m_filePath) == m_pCurrentlyRunningSessionData)
        {
            retVal = true;
        }
    }

    return retVal;
}

void gpUIManager::CloseAllSessionWindows(GPUSessionTreeItemData* pSession)
{
    if ((pSession != nullptr) && (pSession->m_pParentData != nullptr))
    {
        // Close all frame related to this session
        gpSessionTreeNodeData* pGPData = qobject_cast<gpSessionTreeNodeData*>(pSession);

        if (pGPData != nullptr)
        {
            // Get all frame indices for this session
            QList<FrameIndex> frameIndices;
            GetListOfFrameFolders(pSession->m_pParentData->m_filePath, frameIndices);

            foreach (FrameIndex frameIndex, frameIndices)
            {
                // Get the frame file path for this frame
                QDir frameDir;
                QString overviewFileStr, thumbFileStr, ltrFileStr;
                GetPathsForFrame(pSession->m_pParentData->m_filePath, frameIndex, frameDir, overviewFileStr, thumbFileStr, false);

                ltrFileStr = overviewFileStr;
                ltrFileStr.replace(GP_OVR_FileExtension, GP_LTR_FileExtension);

                osFilePath ltrFilePath(acQStringToGTString(ltrFileStr));
                osFilePath thumbFilePath(acQStringToGTString(thumbFileStr));

                // Hide the thumbnail file and the frame file
                gpViewsCreator::Instance()->HideSession(ltrFilePath);
                gpViewsCreator::Instance()->HideSession(thumbFilePath);

            }
        }
    }

}

void gpUIManager::UpdateUI()
{
    afApplicationCommands::instance()->updateToolbarCommands();
    emit UIUpdated();
}

void gpUIManager::WriteDashboardFile()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pCurrentlyRunningSessionData != nullptr)
    {
        // Create the dashboard file
        osFilePath dashBoardFilePath;
        dashBoardFilePath.setFileDirectory(m_pCurrentlyRunningSessionData->SessionDir());
        dashBoardFilePath.setFileName(acQStringToGTString(m_pCurrentlyRunningSessionData->m_displayName));
        dashBoardFilePath.setFileExtension(AF_STR_frameAnalysisDashboardFileExtension);
        gpFASessionData data;
        data.Write(dashBoardFilePath);
    }
}
bool gpUIManager::ReadDashboardFile(gpFASessionData& sessionData, const osFilePath& dashBoardFilePath)
{
    bool rc = false;
    sessionData.m_workingFolder = L"";
    sessionData.m_exePath = L"";
    sessionData.m_cmdArgs = L"";
    sessionData.m_envVars = L"";
    sessionData.m_isRemoteHost = false;
    sessionData.m_hostIP = L"";
    rc = sessionData.Read(dashBoardFilePath);
    return rc;
}
////////////////////////////////////////////////////

void gpFASessionData::Init()
{
    // Fill sessionData with data from the current project settings
    const apProjectSettings& projectSettings = afProjectManager::instance().currentProjectSettings();
    m_workingFolder = projectSettings.workDirectory().asString();
    m_exePath = projectSettings.executablePath().asString();
    m_cmdArgs = projectSettings.commandLineArguments();
    projectSettings.environmentVariablesAsString(m_envVars);
    m_isRemoteHost = projectSettings.isRemoteTarget();

    if (m_isRemoteHost == true)
    {
        m_hostIP = projectSettings.remoteTargetName();
    }
    else
    {
        if (CXL_DAEMON_CLIENT != nullptr)
        {
            osPortAddress ipaddress;
            bool rc = CXL_DAEMON_CLIENT->GetDaemonAddress(ipaddress);
            GT_ASSERT(rc);
            ipaddress.toString(m_hostIP);
        }

    }
}

bool gpFASessionData::Write(const osFilePath& dashboardFilePath)
{
    Init();
    QFile dashBoardFile(acGTStringToQString(dashboardFilePath.asString()));
    bool rc = dashBoardFile.open(QIODevice::WriteOnly | QIODevice::Text);

    if (rc)
    {
        QTextStream out(&dashBoardFile);
        out << acGTStringToQString(m_workingFolder) << endl;
        out << acGTStringToQString(m_exePath) << endl;
        out << acGTStringToQString(m_cmdArgs) << endl;
        out << acGTStringToQString(m_envVars) << endl;
        out << acGTStringToQString(m_hostIP) << endl;

        dashBoardFile.close();
    }

    return rc;
}

bool gpFASessionData::Read(const osFilePath& dashboardFilePath)
{
	bool rc = false;

	if (dashboardFilePath.exists())
	{
		QFile dashBoardFile(acGTStringToQString(dashboardFilePath.asString()));
		rc = dashBoardFile.open(QIODevice::ReadOnly | QIODevice::Text);
		GT_IF_WITH_ASSERT(rc)
		{
			QTextStream in(&dashBoardFile);
            QString workingFolder = in.readLine();
 			m_workingFolder = acQStringToGTString(workingFolder);
            QString exePath = in.readLine();
			m_exePath = acQStringToGTString(exePath);
            QString cmdArgs = in.readLine();
			m_cmdArgs = acQStringToGTString(cmdArgs);
            QString envVars = in.readLine();
			m_envVars = acQStringToGTString(envVars);
            QString hostIP = in.readLine();
			m_hostIP = acQStringToGTString(hostIP);

			dashBoardFile.close();

			if (IsRemoteHost() == false)
			{
				m_hostIP = GPU_STR_dashboard_HTMLHostLocal;
			}

		}
	}
    return rc;
}

bool gpFASessionData::IsRemoteHost()
{
    bool rc = true;
    gtString _hostName;
    gtVector<gtString> ipAddresses;
    bool foundValid = false;
    bool rcAddrs = osTCPSocket::getIpAddresses(ipAddresses);
    GT_ASSERT(rcAddrs);

    if (rcAddrs && !ipAddresses.empty())
    {
        bool foundPreferred = false;
        gtString s_localhostName1 = L"127.0.0.1";
        gtString s_localhostName2 = L"localhost";
        gtString s_hostnameEnvVarName = L"HOSTNAME";
        gtString hostnameEnvVarValue;
        osGetCurrentProcessEnvVariableValue(s_hostnameEnvVarName, hostnameEnvVarValue);
        // Take the first address which is not the localhost. If HOSTNAME is set to something else,prefer that one.
        int numberOfAddresses = (int)ipAddresses.size();

        for (int i = 0; (i < numberOfAddresses) && (!foundPreferred); i++)
        {
            // Ignore the localhost addresses:
            const gtString& currentAddr = ipAddresses[i];

            if ((currentAddr != s_localhostName1) && (currentAddr != s_localhostName2))
            {
                // Is this an IPv4 Address?
                static const gtString s_allowedIPv4Chars = L"0123456789.";

                if ((3 == currentAddr.count('.')) && (currentAddr.onlyContainsCharacters(s_allowedIPv4Chars)))
                {
                    // Select the first IPv4 address we find (but continue looking for the preferred address):
                    if (!foundValid)
                    {
                        _hostName = currentAddr;
                        foundValid = true;
                    }
                }

                if (m_hostIP.find(currentAddr) != -1)
                {
                    rc = false;
                }
            }
        }
    }

    return rc;
}
