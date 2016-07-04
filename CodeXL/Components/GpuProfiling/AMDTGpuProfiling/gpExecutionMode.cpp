//------------------------------ gpExecutionMode.cpp ------------------------------


#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt
#include <QtWidgets>

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <unistd.h>
#endif

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>


// graphics
#include <AMDTGraphicsServerInterface/Include/AMDTGraphicsServerInterface.h>

// Shared:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>
#include <AMDTSharedProfiling/inc/StringConstants.h>
#include <AMDTSharedProfiling/inc/SharedProfileSettingPage.h>

// Local:
#include <AMDTGpuProfiling/gpConnectDialog.h>
#include <AMDTGpuProfiling/gpExecutionMode.h>
#include <AMDTGpuProfiling/gpTreeHandler.h>
#include <AMDTGpuProfiling/gpUIManager.h>
#include <AMDTGpuProfiling/gpViewsCreator.h>
#include <AMDTGpuProfiling/ProfileProcessMonitor.h>
#include <AMDTGpuProfiling/gpStringConstants.h>
#include <AMDTGpuProfiling/Session.h>
#include <AMDTGpuProfiling/PerformanceCountersXMLSample.h>
#include <AMDTGpuProfiling/ProfileManager.h>

// include the PerfStudio server definitions:
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <../../Graphics/Server/WebServer/ServerDefinitions.h>
#else
    #include <AMDTGpuProfiling/../../Graphics/Server/WebServer/ServerDefinitions.h>
#endif

#define SERVER_WAIT_TIME 500
#define SERVER_CONNECTION_TRIES 20


gpExecutionMode::gpExecutionMode() :
    m_isFrameAnalysisRunning(false),
    m_pGraphicsServerCommunication(nullptr),
    m_serverProcessID(0),
    m_cxlAgentProcessID(0),
    m_isCapturing(false),
    m_isFrameAnalysisConnecting(false)
{
}

gpExecutionMode::~gpExecutionMode()
{

}

void gpExecutionMode::Initialize()
{
    m_settings.Initialize();
}

// ---------------------------------------------------------------------------
// Name:        gpExecutionMode::modeName
// Description: Mode name for identification
// Author:      Gilad Yarnitzky
// Date:        10/5/2012
// ---------------------------------------------------------------------------
gtString gpExecutionMode::modeName()
{
    return GPU_STR_executionMode;
}

gtString gpExecutionMode::modeActionString()
{
    return GPU_STR_executionModeAction;
}

gtString gpExecutionMode::modeVerbString()
{
    return GPU_STR_executionModeVerb;
}

gtString gpExecutionMode::modeDescription()
{
    return GPU_STR_executionModeDescription;
}

void gpExecutionMode::execute(afExecutionCommandId commandId)
{
    switch (commandId)
    {
        case AF_EXECUTION_ID_BUILD:
        case AF_EXECUTION_ID_CANCEL_BUILD:
        case AF_EXECUTION_ID_FRAME_STEP:
        case AF_EXECUTION_ID_DRAW_STEP:
        case AF_EXECUTION_ID_API_STEP:
        case AF_EXECUTION_ID_STEP_OVER:
        case AF_EXECUTION_ID_STEP_IN:
        case AF_EXECUTION_ID_STEP_OUT:
        case AF_EXECUTION_ID_BREAK:
        {
            break;
        }

        case AF_EXECUTION_ID_START:
        {
            OnStartFrameAnalysis();
        }
        break;

        case AF_EXECUTION_ID_CAPTURE:
        {
            OnFrameAnalysisCapture(gpExecutionMode::FrameAnalysisCaptureType_LinkedTrace);
        }
        break;
        case AF_EXECUTION_ID_CAPTURE_CPU:
        {
            OnFrameAnalysisCapture(gpExecutionMode::FrameAnalysisCaptureType_APITrace);
        }
        break;
        case AF_EXECUTION_ID_CAPTURE_GPU:
        {
            OnFrameAnalysisCapture(gpExecutionMode::FrameAnalysisCaptureType_GPUTrace);
        }
        break;
        case AF_EXECUTION_ID_STOP:
        {
            stopCurrentRun();
        }
        break;

        default: break;
    }
}


void gpExecutionMode::updateUI(afExecutionCommandId commandId, QAction* pAction)
{
    bool isActionEnabled = true;
    bool isActionVisible = true;

    GT_IF_WITH_ASSERT(NULL != pAction)
    {
        switch (commandId)
        {
            case AF_EXECUTION_ID_BUILD:
            case AF_EXECUTION_ID_CANCEL_BUILD:
            case AF_EXECUTION_ID_FRAME_STEP:
            case AF_EXECUTION_ID_DRAW_STEP:
            case AF_EXECUTION_ID_API_STEP:
            case AF_EXECUTION_ID_STEP_OVER:
            case AF_EXECUTION_ID_STEP_IN:
            case AF_EXECUTION_ID_STEP_OUT:
            case AF_EXECUTION_ID_BREAK:
            {
                isActionVisible = false;
                isActionEnabled = false;
                break;
            }

            case AF_EXECUTION_ID_START:
            {
                gtString text;
                isActionEnabled = IsStartEnabled(text);
            }
            break;

            case AF_EXECUTION_ID_CAPTURE:
            {
                isActionEnabled = IsCaptureEnabled();
                gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
                gpProjectSettings& settings = pFrameAnalysisManager->ProjectSettings();
                QString buttonText = QString(GPU_STR_dashboard_CaptureTooltip).arg(settings.m_numFramesToCapture);
                pAction->setText(buttonText);

            }
            break;
            case AF_EXECUTION_ID_CAPTURE_CPU:
            {
                isActionEnabled = IsCaptureEnabled();
                gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
                gpProjectSettings& settings = pFrameAnalysisManager->ProjectSettings();
                QString buttonText = QString(GPU_STR_dashboard_CaptureTooltip).arg(settings.m_numFramesToCapture);
                pAction->setText(buttonText);

            }
            break;
            case AF_EXECUTION_ID_CAPTURE_GPU:
            {
                isActionEnabled = IsCaptureEnabled();
                gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
                gpProjectSettings& settings = pFrameAnalysisManager->ProjectSettings();
                QString buttonText = QString(GPU_STR_dashboard_CaptureTooltip).arg(settings.m_numFramesToCapture);
                pAction->setText(buttonText);

            }
            break;

            case AF_EXECUTION_ID_STOP:
            {
                isActionEnabled = IsStopEnabled();
            }
            break;

            default: break;
        }

        pAction->setEnabled(isActionEnabled);
        pAction->setVisible(isActionVisible);
    }
}

gtString gpExecutionMode::sessionTypeName(int sessionTypeIndex)
{
    gtString sessionName;

    GT_IF_WITH_ASSERT(sessionTypeIndex >= 0 && sessionTypeIndex < 1)
    {
        sessionName = GPU_STR_executionSesionType;
    }

    return sessionName;

}

QPixmap* gpExecutionMode::sessionTypeIcon(int sessionTypeIndex)
{
    QPixmap* pPixmap = NULL;

    GT_IF_WITH_ASSERT(sessionTypeIndex >= 0 && sessionTypeIndex < 1)
    {
        pPixmap = new QPixmap;
        acSetIconInPixmap(*pPixmap, AC_ICON_FRAME_ANALYSIS_MODE);
    }

    return pPixmap;
}

bool gpExecutionMode::ExecuteStartupAction(afStartupAction action)
{
    bool retVal = false;

    if (action == AF_NO_PROJECT_USER_ACTION_CREATE_NEW_PROJECT_FRAME_ANALYZE)
    {
        afApplicationCommands::instance()->OnFileNewProject();
        retVal = true;
    }

    return retVal;
}

bool gpExecutionMode::IsStartupActionSupported(afStartupAction action)
{
    bool retVal = false;

    if (action == AF_NO_PROJECT_USER_ACTION_CREATE_NEW_PROJECT_FRAME_ANALYZE)
    {
        retVal = true;
    }

    return retVal;
}

bool gpExecutionMode::IsRemoteEnabledForSessionType(const gtString& sessionType)
{
    GT_UNREFERENCED_PARAMETER(sessionType);

    // We only support remote debugging in standalone:
    bool retVal = !afGlobalVariablesManager::instance().isRunningInsideVisualStudio();
    return retVal;
}

bool gpExecutionMode::isModeEnabled()
{
    return true;
}

gtString gpExecutionMode::HowToStartModeExecutionMessage()
{
    gtString retStr = GPU_STR_PropertiesExecutionInformationSA;

    if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        retStr = GPU_STR_PropertiesExecutionInformationVS;
    }

    return retStr;
}


afRunModes gpExecutionMode::getCurrentRunModeMask()
{
    afRunModes retVal = 0;

    if (m_isFrameAnalysisRunning)
    {
        retVal |= AF_DEBUGGED_PROCESS_EXISTS;
        retVal |= AF_DEBUGGED_PROCESS_RUNNING;
    }

    if (SharedProfileManager::instance().isExportIsRunning())
    {
        retVal |= AF_FRAME_ANALYZE_CURRENTLY_EXPORTING;
    }

    if (SharedProfileManager::instance().isImportIsRunning())
    {
        retVal |= AF_FRAME_ANALYZE_CURRENTLY_IMPORTING;
    }

    if (m_isFrameAnalysisConnecting)
    {
        retVal |= AF_FRAME_ANALYZE_CONNECTING;
    }

    return retVal;
}

bool gpExecutionMode::canStopCurrentRun()
{
    return m_isFrameAnalysisRunning;
}


bool gpExecutionMode::getExceptionEventDetails(const apExceptionEvent& exceptionEve,
                                               osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded,
                                               bool& kernelDebuggingEnteredAtLeastOnce)
{
    (void)(exceptionEve); // unused
    (void)(exceptionCallStack); // unused
    (void)(openCLEnglineLoaded); // unused
    (void)(openGLEnglineLoaded); // unused
    (void)(kernelDebuggingEnteredAtLeastOnce); // unused
    //Do we need this if not debugging?
    return true;
}

bool gpExecutionMode::stopCurrentRun()
{
    // Shut down the server
    ShutServerDown();

    // Get the captured frame from the server. Some of the frames are captured from the application UI, so we need to pick it up from the server
    gtString capturedFramesAsXML, errorMessage;
    bool isRetryEnabled = true;
    bool rc = m_remoteGraphicsBackendServerLauncher.GetCapturedFrames(afProjectManager::instance().currentProjectSettings().projectName(), capturedFramesAsXML, isRetryEnabled, errorMessage);
    GT_ASSERT(rc);

    ApplicationEnded();

    gpUIManager::Instance()->OnApplicationEnded();

    afApplicationCommands::instance()->updateToolbarCommands();

    // Update the application title with the status of the frame analysis
    UpdateApplicationTitle();

    // Notify the current opened view that a frame capture is processing
    gpUIManager::Instance()->UpdateUI();

    return true;
}

bool gpExecutionMode::IsStartEnabled(gtString& startActionText)
{
    gtString fileName;
    afProjectManager::instance().currentProjectSettings().executablePath().getFileNameAndExtension(fileName);

    bool retVal = afExecutionModeManager::instance().isActiveMode(GPU_STR_executionMode) && (!m_isFrameAnalysisRunning);
    startActionText = GPU_STR_executionModeStart;

    if (!fileName.isEmpty())
    {
        startActionText += AF_STR_Space;
        startActionText += AF_STR_LeftParenthesis;
        startActionText += fileName;
        startActionText += AF_STR_RightParenthesis;
    }


    bool isInExport = (afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_FRAME_ANALYZE_CURRENTLY_EXPORTING);
    bool isInImport = (afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_FRAME_ANALYZE_CURRENTLY_IMPORTING);

    retVal &= (!isInExport && !isInImport);

    return retVal;
}

bool gpExecutionMode::IsStopEnabled()
{
    return m_isFrameAnalysisRunning;
}

bool gpExecutionMode::IsCaptureEnabled()
{
    bool retVal = false;
    // Pause is enable when application is running and not paused
    bool isProcessRunning = (afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_DEBUGGED_PROCESS_EXISTS);
    bool isPaused = (afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_DEBUGGED_PROCESS_PAUSED);
    retVal = isProcessRunning && !isPaused && !m_isCapturing;

    return retVal;
}

bool gpExecutionMode::GetCapturedFrameData(FrameInfo& frameInfo)
{
    bool retVal = true;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    // Sanity check:
    GT_IF_WITH_ASSERT((gpUIManager::Instance() != nullptr) && (gpUIManager::Instance()->CurrentlyRunningSessionData() != nullptr))
    {
        // Get the frame data from the remote agent
        gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();
        gtString sessionName = acQStringToGTString(gpUIManager::Instance()->CurrentlyRunningSessionData()->m_displayName);
        bool isRetryEnabled = true;
        gtString strErrorMessageOut;
        retVal = m_remoteGraphicsBackendServerLauncher.GetFrameData(projectName, sessionName, frameInfo.m_frameIndex, frameInfo, isRetryEnabled, strErrorMessageOut);
        GT_IF_WITH_ASSERT(retVal)
        {
            // Translate the frame info XML into the structure
            retVal = gpUIManager::Instance()->FrameInfoFromXML(frameInfo.m_frameInfoXML, frameInfo);
        }

        // Save the captured frame data on disk
        gpUIManager::Instance()->SaveCurrentSessionFrameDataOnDisk(frameInfo);
    }

    return retVal;
}

bool gpExecutionMode::GetCapturedFrameData(const gtString& projectName, const gtString& sessionName, FrameInfo& frameInfo)
{
    bool retVal = true;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    // Sanity check:
    GT_IF_WITH_ASSERT(gpUIManager::Instance() != nullptr)
    {
        // Get the frame data from the remote agent
        bool isRetryEnabled = true;
        gtString strErrorMessageOut;
        retVal = m_remoteGraphicsBackendServerLauncher.GetFrameData(projectName, sessionName, frameInfo.m_frameIndex, frameInfo, isRetryEnabled, strErrorMessageOut);
    }

    return retVal;
}


bool gpExecutionMode::RefreshLoadedProjectSessionsFromServer()
{
    bool retVal = true;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    // Make sure that the remote server launcher is initialized with the current server settings
    const gtString& projectName = afProjectManager::instance().currentProjectSettings().projectName();
    const osDirectory& workDir = afProjectManager::instance().currentProjectSettings().workDirectory();

    if (!projectName.isEmpty())
    {
        m_remoteGraphicsBackendServerLauncher.Init(osFilePath(), L"", workDir);
        retVal = true;
    }

    GT_IF_WITH_ASSERT(retVal)
    {
        // Get the captured frame from the server. Some of the frames are captured from the application UI, so we need to pick it up from the server
        gtString capturedFramesAsXML, errorMessage;
        bool isRetryEnabled = true;
        retVal = m_remoteGraphicsBackendServerLauncher.GetCapturedFrames(projectName, capturedFramesAsXML, isRetryEnabled, errorMessage);
        GT_IF_WITH_ASSERT(retVal)
        {
            // Before opening the project, make sure that the local disk contain all files for the capture frames for all the project sessions
            retVal = gpUIManager::Instance()->CacheProjectSessionsList(capturedFramesAsXML);
        }
    }

    return retVal;
}

void gpExecutionMode::TerminateRemoteAgent()
{
    OS_DEBUG_LOG_TRACER;
    gtVector<gtString> fileNamesToTerminate{ GPU_STR_CodeXLAgent };
    osProcessId currentProcessId = osGetCurrentProcessId();

    if (CXL_DAEMON_CLIENT != nullptr)
    {
        CXL_DAEMON_CLIENT->TerminateWholeSession();
    }

    osTerminateProcessesByName(fileNamesToTerminate, currentProcessId, false);
}

void gpExecutionMode::OnStartFrameAnalysis()
{
    bool rc = CheckProjectSettings();

    if (rc)
    {
        // Make sure that the raptr exe is not running on the remote agent machine
        HandleSpecialRunningProcesses();

        // Launch the graphic server and send the requested application
        bool messageShown(false);

        if (rc)
        {
            rc = LaunchServer(messageShown);
        }

        if (rc)
        {
            gpConnectDialog connectionDialog(m_pGraphicsServerCommunication, afMainAppWindow::instance());
            bool shouldConnect = connectionDialog.Connect();
            QString processID = connectionDialog.PIDToConnectTo();

            if (shouldConnect && !processID.isEmpty())
            {
                m_isFrameAnalysisConnecting = true;
                // connect the process and pass the pid:
                rc = m_pGraphicsServerCommunication->ConnectProcess(processID.toStdString().c_str(), connectionDialog.APIToConnectTo().toStdString().c_str());

                GT_IF_WITH_ASSERT(rc)
                {
                    // Create the session node
                    CreateNewSession();

                    // Create the dashboard file for this session
                    gpUIManager::Instance()->WriteDashboardFile();

                    // Open the new created session
                    afApplicationCommands::instance()->OpenFileAtLine(gpUIManager::Instance()->CurrentlyRunningSessionData()->m_pParentData->m_filePath, 0);

                    m_isFrameAnalysisRunning = true;

                    // Start the timer that updates the captured frames from server
                    m_capturedFramesUpdateTime.setFromCurrentTime();

                    // Update the application title with the status of the frame analysis
                    UpdateApplicationTitle();
                }
                else
                {
                    // Build a message according to the API type
                    QString apiStr = connectionDialog.APIToConnectTo();
                    if (apiStr.isEmpty())
                    {
                        apiStr = QString("%1 or %2").arg(GPU_STR_DX12Api).arg(GPU_STR_VulkanApi);
                    }
                    QString userMessage = QString(GPU_STR_dashboard_failedToConnectError).arg(apiStr);
                    acMessageBox::instance().information(AF_STR_InformationA, userMessage);
                }

                m_isFrameAnalysisConnecting = false;
            }
            else
            {
                rc = false;
            }
        }
        else
        {
            if (!messageShown)
            {
                acMessageBox::instance().information(AF_STR_InformationA, GPU_STR_dashboard_failedToLaunchError);
            }
        }

        if (!rc)
        {
            // If something failed in the connection or the server launch, we should shut down.
            // Otherwise, future executions will be blocked
            stopCurrentRun();
        }
    }
}

bool gpExecutionMode::CheckProjectSettings()
{
    bool retVal = false;

    const apProjectSettings& projectSettings = afProjectManager::instance().currentProjectSettings();

    // Get the debugged process executable path:
    const osFilePath& exePath = projectSettings.executablePath();
    bool exeFileExist = exePath.isExecutable();
    bool isRemoteTarget = projectSettings.isRemoteTarget();
    bool isProjectLoaded = !projectSettings.projectName().isEmpty();

    if (!isProjectLoaded)
    {
        // Display the startup dialog:
        afExecutionModeManager::instance().DisplayStartupDialog();
    }

    else if ((!isRemoteTarget && !exeFileExist) || (isRemoteTarget && exePath.isEmpty()))
    {
        bool isProjectLoaded = !afProjectManager::instance().currentProjectSettings().projectName().isEmpty();
        QString userMessage = isProjectLoaded ? AF_STR_NoExeIsSelected : AF_STR_NoProjectIsLoaded;

        // The exe file does not exist on disk - display an appropriate message:
        QMessageBox::StandardButton userAnswer = acMessageBox::instance().information(AF_STR_ErrorA, userMessage, QMessageBox::Ok | QMessageBox::Cancel);

        if (userAnswer == QMessageBox::Ok)
        {
            afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
            {
                {
                    // Open the debug settings dialog:
                    pApplicationCommands->OnProjectSettings(AF_globalSettingsGeneralHeaderUnicode);
                    const apProjectSettings& updatedProjectSettings = afProjectManager::instance().currentProjectSettings();

                    // Get the debugged process executable path:
                    const osFilePath& newExePath = updatedProjectSettings.executablePath();

                    if (newExePath.isExecutable())
                    {
                        retVal = true;
                    }
                }
            }
        }
        else
        {
            retVal = false;
        }
    }
    else
    {
        retVal = true;
    }

    return retVal;
}

void gpExecutionMode::OnFrameAnalysisCapture(gpExecutionMode::FrameAnalysisCaptureType captureType)
{

    OS_DEBUG_LOG_TRACER;
    gtASCIIString strWebRtn;

    // Sanity check:
    GT_IF_WITH_ASSERT(gpUIManager::Instance()->CurrentlyRunningSessionData() != nullptr)
    {
        // If the session is not opened, open it
        afApplicationCommands::instance()->OpenFileAtLine(gpUIManager::Instance()->CurrentlyRunningSessionData()->m_pParentData->m_filePath, 0);

        CaptureFrame(captureType);
    }
}

bool gpExecutionMode::GetFrameTraceFromServer(const osFilePath& sessionFilePath, FrameIndex frameIndex, osFilePath& traceFilePath)
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    // When the trace file is missing from the local disk, or when it is an empty file,
    // this means that we should get the file from the server
    bool shouldGetFileFromServer = !traceFilePath.exists();

    if (traceFilePath.exists())
    {
        osFile fileHandle(traceFilePath);
        fileHandle.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_READ);
        unsigned long fileSize = 0;
        fileHandle.getSize(fileSize);

        if (fileSize == 0)
        {
            shouldGetFileFromServer = true;
        }

        fileHandle.close();
    }

    if (shouldGetFileFromServer)
    {
        // Launch the server with CapturePlayer
        FrameInfo frameInfo;
        frameInfo.m_frameIndex = frameIndex.first;
        frameInfo.m_framesCount = frameIndex.second - frameIndex.first + 1;
        retVal = gpUIManager::Instance()->GetFrameInfo(sessionFilePath, frameInfo);
        GT_IF_WITH_ASSERT(retVal)
        {
            // Launch the server in a "CapturePlayer" mode, to replay the frame and get the trace
            bool messageShown(false);
            retVal = LaunchServer(messageShown, frameInfo.m_frameInfoXML);
            GT_IF_WITH_ASSERT(retVal)
            {
                gpConnectDialog connectionDialog(m_pGraphicsServerCommunication, nullptr, true);
                bool shouldConnect = connectionDialog.Connect();
                QString processID = connectionDialog.PIDToConnectTo();

                if (shouldConnect && !processID.isEmpty())
                {
                    // connect the process and pass the pid:
                    retVal = m_pGraphicsServerCommunication->ConnectProcess(processID.toStdString().c_str(), connectionDialog.APIToConnectTo().toStdString().c_str());

                    gtASCIIString traceAsText;

                    GT_IF_WITH_ASSERT(m_pGraphicsServerCommunication != nullptr)
                    {
                        m_pGraphicsServerCommunication->GetLinkedTrace(traceAsText);
                    }

                    retVal = !traceAsText.isEmpty();
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        osFile traceFileHandle(traceFilePath);
                        retVal = traceFileHandle.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
                        GT_IF_WITH_ASSERT(retVal)
                        {
                            gtString dataAsStr;
                            dataAsStr.fromASCIIString(traceAsText.asCharArray());

                            traceFileHandle.writeString(dataAsStr);
                            traceFileHandle.close();
                        }
                    }

                    // Shut down the server and kill the server communication object
                    ShutServerDown();

                    retVal = true;
                }
                else if (processID.isEmpty())
                {
                    retVal = false;
                }
            }
        }
    }
    else
    {
        retVal = true;
    }

    return retVal;
}

// Get object tree and database from server
bool gpExecutionMode::GetFrameObject(const osFilePath& sessionFilePath, int frameIndex, osFilePath& objectFilePath)
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    gtString strObjTree, strObjDBase;
    osFilePath objFileStorage;

    // need to handle two files here, one for the tree and one for the Database
    strObjTree.makeEmpty();
    strObjTree.append(objectFilePath.asString());

    strObjDBase.makeEmpty();
    strObjDBase.append(objectFilePath.fileDirectoryAsString());
    strObjDBase.append(GPU_STR_FullObjectDatabase);

    // When the object file is missing from the local disk, or when it is an empty file,
    // this means that we should get the file from the server
    bool shouldGetFileFromServer = !objectFilePath.exists();

    if (objectFilePath.exists())
    {
        osFile fileHandle(objectFilePath);
        fileHandle.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_READ);
        unsigned long fileSize = 0;
        fileHandle.getSize(fileSize);

        if (fileSize == 0)
        {
            shouldGetFileFromServer = true;
        }

        fileHandle.close();
    }

    if (shouldGetFileFromServer)
    {
        // Launch the server with CapturePlayer
        FrameInfo frameInfo;
        frameInfo.m_frameIndex = frameIndex;
        retVal = gpUIManager::Instance()->GetFrameInfo(sessionFilePath, frameInfo);
        GT_IF_WITH_ASSERT(retVal)
        {
            // Launch the server in a "CapturePlayer" mode, to replay the frame and get the object
            bool messageShown(false);
            retVal = LaunchServer(messageShown, frameInfo.m_frameInfoXML);
            GT_IF_WITH_ASSERT(retVal)
            {
                gpConnectDialog connectionDialog(m_pGraphicsServerCommunication, nullptr, true);
                bool shouldConnect = connectionDialog.Connect();
                QString processID = connectionDialog.PIDToConnectTo();

                if (shouldConnect && !processID.isEmpty())
                {
                    // connect the process and pass the pid:
                    retVal = m_pGraphicsServerCommunication->ConnectProcess(processID.toStdString().c_str(), connectionDialog.APIToConnectTo().toStdString().c_str());

                    gtASCIIString objectInfoAsXML;

                    // Getting tree
                    GT_IF_WITH_ASSERT(m_pGraphicsServerCommunication != nullptr)
                    {
                        objectInfoAsXML.makeEmpty();
                        m_pGraphicsServerCommunication->GetObjectTree(objectInfoAsXML);
                    }

                    retVal = !objectInfoAsXML.isEmpty();
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        objFileStorage.setFullPathFromString(strObjTree);
                        osFile objectFileHandle(objFileStorage);
                        retVal = objectFileHandle.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
                        GT_IF_WITH_ASSERT(retVal)
                        {
                            gtString dataAsStr;
                            dataAsStr.fromASCIIString(objectInfoAsXML.asCharArray());

                            objectFileHandle.writeString(dataAsStr);
                            objectFileHandle.close();
                        }
                    }

                    // Getting database
                    GT_IF_WITH_ASSERT(m_pGraphicsServerCommunication != nullptr)
                    {
                        objectInfoAsXML.makeEmpty();
                        m_pGraphicsServerCommunication->GetObjectDatabase(objectInfoAsXML);
                    }

                    retVal = !objectInfoAsXML.isEmpty();
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        objFileStorage.setFullPathFromString(strObjDBase);
                        osFile objectFileHandle(objFileStorage);
                        retVal = objectFileHandle.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
                        GT_IF_WITH_ASSERT(retVal)
                        {
                            gtString dataAsStr;
                            dataAsStr.fromASCIIString(objectInfoAsXML.asCharArray());

                            objectFileHandle.writeString(dataAsStr);
                            objectFileHandle.close();
                        }
                    }

                    // Shut down the server and kill the server communication object
                    ShutServerDown();

                    return true;
                }
            }
        }
    }
    else
    {
        retVal = true;
    }

    return retVal;
}

void gpExecutionMode::ShutServerDown()
{
    OS_DEBUG_LOG_TRACER;

    // End the session in the log file:
    osDebugLog::instance().EndSession();

    // terminate the connection to the server
    if (m_pGraphicsServerCommunication != nullptr)
    {
        // Prevent endless resends of messages to the server by setting the GraphicsServerInterface object's StopSignal flag
        m_pGraphicsServerCommunication->SetStopSignal();

        // end the executed application
        bool rc = false;
        const int numOfShutdownAttempts = 3;

        for (int i = 0; false == rc && i < numOfShutdownAttempts; ++i)
        {
            rc = m_pGraphicsServerCommunication->ShutDown();

            if (!rc)
            {
                m_pGraphicsServerCommunication->sleepBetweenResendAttempts();
            }
        }

        rc = m_pGraphicsServerCommunication->Disconnect();
        GT_ASSERT(rc);
    }

    // Terminate the remote graphics server
    m_remoteGraphicsBackendServerLauncher.TerminateRemoteGraphicsBeckendServer();
}

bool gpExecutionMode::CaptureFrame(FrameAnalysisCaptureType captureType)
{
    OS_DEBUG_LOG_TRACER;
    gpUIManager::Instance()->UIUpdaterThread().SendCaptureFrameRequest(captureType);
    m_isCapturing = true;

    // Notify the current opened view that a frame capture is processing
    gpUIManager::Instance()->UpdateUI();
    return true;
}

bool gpExecutionMode::CapturePerformanceCounters(FrameIndex frameIndex, bool shouldOpenFile)
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pGraphicsServerCommunication != nullptr) && (gpUIManager::Instance()->CurrentlyRunningSessionData() != nullptr))
    {
        // Get the linked trace from the server
        gtASCIIString strWebRtn;
        retVal = m_pGraphicsServerCommunication->PushLogger(strWebRtn);

        if (retVal)
        {
#pragma message ("TODO: FA: uncomment when the server performance counters are implemented")
            // retVal = m_pGraphicsServerCommunication->RunProfile(strWebRtn);
            retVal = true;

            if (retVal)
            {
#pragma message ("TODO: FA: uncomment when the server performance counters are implemented")
                // m_pGraphicsServerCommunication->PopLayer();
                strWebRtn = GP_Str_ProfileExampleData;

                // Create the performance counters file and write it
                osFilePath countersFilePath = gpUIManager::Instance()->CurrentlyRunningSessionData()->SessionDir().directoryPath();
                int currentHour = qrand() % 24;
                int currentMinute = qrand() % 60;
                int currentSec = qrand() % 60;
                QString fileNameStr;
                if (frameIndex.first == frameIndex.second)
                {
                    fileNameStr = QString(GPU_STR_FramePerfCountersFileNameSingleFormat).arg(gpUIManager::Instance()->CurrentlyRunningSessionData()->m_displayName).arg(frameIndex.first).arg(currentHour).arg(currentMinute).arg(currentSec);
                }
                else
                {
                    fileNameStr = QString(GPU_STR_FramePerfCountersFileNameMultiFormat).arg(gpUIManager::Instance()->CurrentlyRunningSessionData()->m_displayName).arg(frameIndex.first).arg(frameIndex.second).arg(currentHour).arg(currentMinute).arg(currentSec);
                }
                gtString fileName = acQStringToGTString(fileNameStr);
                countersFilePath.setFileName(fileName);
                gtString frameFolderName;
                if (frameIndex.first == frameIndex.second)
                {
                    frameFolderName.appendFormattedString(GPU_STR_FrameSubFolderNameSingleFormat, frameIndex.first);
                }
                else
                {
                    frameFolderName.appendFormattedString(GPU_STR_FrameSubFolderNameMultipleFormat, frameIndex.first, frameIndex.second);
                }
                countersFilePath.appendSubDirectory(frameFolderName);
                countersFilePath.setFileExtension(AF_STR_profileFileExtension9);
                osFile countersFile;
                osDirectory frameDir;
                countersFilePath.getFileDirectory(frameDir);

                if (!frameDir.exists())
                {
                    frameDir.create();
                }

                bool rcFile = countersFile.open(countersFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

                if (rcFile)
                {
                    gtString dataAsStr;
                    dataAsStr.fromASCIIString(strWebRtn.asCharArray());
                    countersFile.writeString(dataAsStr);
                    countersFile.close();

                    // Add a trace node to the session tree
                    gpTreeHandler::Instance().AddCountersDataFileToSession(countersFilePath, frameIndex, shouldOpenFile);
                }
            }
        }
    }
    return retVal;
}

void gpExecutionMode::CreateNewSession()
{
    OS_DEBUG_LOG_TRACER;
    const apProjectSettings& projectSettings = afProjectManager::instance().currentProjectSettings();
    gtString projectName = projectSettings.projectName();
    gtString projectDir = afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString();

    gtString strSessionDisplayName;
    osDirectory sessionOsDir;
    osFilePath projectDirPath(projectDir);

    // Get the place where the file needs to be written to:
    // get the next session name and dir from the session naming helper (and clean the dir if there is any old cruft)
    ProfileApplicationTreeHandler::instance()->GetNextSessionNameAndDir(projectName, projectDirPath, strSessionDisplayName, sessionOsDir);

    gpSessionTreeNodeData* pSessionData = CreateSession(projectName, strSessionDisplayName, sessionOsDir);
    GT_IF_WITH_ASSERT(pSessionData != nullptr)
    {
        // Add the tree nodes:
        // Add the main session node which will add the other needed nodes:
        ProfileApplicationTreeHandler::instance()->AddSession(pSessionData, true);

        gpUIManager::Instance()->SetCurrentlyRunningSessionData(pSessionData);

        if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
        {
            static bool sMainWindowInitialized = false;

            if (!sMainWindowInitialized)
            {
                if (afMainAppWindow::instance() != nullptr)
                {
                    bool rc = afMainAppWindow::instance()->connect(afMainAppWindow::instance(), SIGNAL(SubWindowAboutToClose(const osFilePath&, bool&)), gpUIManager::Instance(), SLOT(OnBeforeActiveSubWindowClose(const osFilePath&, bool&)));
                    GT_ASSERT(rc);

                    sMainWindowInitialized = true;
                }
            }
        }
    }
}


void gpExecutionMode::ApplicationEnded()
{
    m_isFrameAnalysisRunning = false;
}

gpSessionTreeNodeData* gpExecutionMode::CreateSession(gtString& projectName, gtString& strSessionDisplayName, osDirectory& sessionOsDir)
{
    OS_DEBUG_LOG_TRACER;
    gpSessionTreeNodeData* pRetVal = nullptr;

    // Create the session data:
    gtString projectDir = afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString();

    const apProjectSettings& projectSettings = afProjectManager::instance().currentProjectSettings();
    gtString workDirAsStr = projectSettings.workDirectory().asString();

    osFilePath outputFilePath(sessionOsDir.directoryPath());
    outputFilePath.setFileName(strSessionDisplayName);
    outputFilePath.setFileExtension(GP_Dashbord_FileExtensionW);

    pRetVal = new gpSessionTreeNodeData(acGTStringToQString(strSessionDisplayName), acGTStringToQString(workDirAsStr), acGTStringToQString(outputFilePath.asString()), acGTStringToQString(projectName), false);

    pRetVal->m_pParentData = new afApplicationTreeItemData;
    pRetVal->m_pParentData->m_filePath = outputFilePath;
    pRetVal->m_pParentData->m_itemType = AF_TREE_ITEM_PROFILE_SESSION;

    pRetVal->m_profileTypeStr = PM_profileTypeFrameAnalysis;
    pRetVal->m_commandArguments.clear();

    gtString exeName;
    projectSettings.executablePath().getFileNameAndExtension(exeName);
    pRetVal->m_exeName = acGTStringToQString(exeName);
    gtString executablePathAsStr = projectSettings.executablePath().asString();
    pRetVal->m_exeFullPath = acGTStringToQString(executablePathAsStr);

    if (pRetVal->m_exeFullPath.isEmpty())
    {
        pRetVal->m_exeFullPath = acGTStringToQString(projectSettings.windowsStoreAppUserModelID());
    }

    // Frame analysis supports only single exe scope and an entire duration profiling.
    // These variables should not be taken from the settings, it's a constant
    pRetVal->m_profileScope = PM_PROFILE_SCOPE_SINGLE_EXE;
    pRetVal->m_shouldProfileEntireDuration = true;

    return pRetVal;
}

bool gpExecutionMode::GetFrameAnalisysServerPaths(gtString& capturePlayerPathAsStr, osFilePath& serverPath) const
{
    OS_DEBUG_LOG_TRACER;
    // Get the server location: assume it is where CodeXL is located
    serverPath = osFilePath(osFilePath::OS_CODEXL_BINARIES_PATH);
    gtString serverPathAsStr = serverPath.asString();
    serverPathAsStr.append(osFilePath::osPathSeparator);

    capturePlayerPathAsStr = AC_STR_PathReplacementString;

    // load the correct server based on the bitness of the application:
    gtString perfStudioServerName(GPU_STR_perfStudioServer64);

    serverPathAsStr.append(perfStudioServerName);

    capturePlayerPathAsStr.append(GPU_STR_GraphicsCapturePlayer64);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    serverPathAsStr.append(L".");
    serverPathAsStr.append(AF_STR_exeFileExtension);
    capturePlayerPathAsStr.append(L".");
    capturePlayerPathAsStr.append(AF_STR_exeFileExtension);
#endif
    serverPath.setFullPathFromString(serverPathAsStr);

    return true;
}

void gpExecutionMode::HandleSpecialRunningProcesses()
{
    OS_DEBUG_LOG_TRACER;
    InitializeCodeXLRemoteAgent();

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // Check if the raptr.exe or fraps.exe is running on the remote agent
    bool isRaptrRunning = false;
    bool isFrapsRunning = false;
    gtString errorMessage;
    bool rc1 = m_remoteGraphicsBackendServerLauncher.IsProcessRunning(GPU_STR_RaptrExeName, isRaptrRunning, errorMessage);
    bool rc2 = m_remoteGraphicsBackendServerLauncher.IsProcessRunning(GPU_STR_FrapsExeName, isFrapsRunning, errorMessage);

    GT_IF_WITH_ASSERT(rc1 && rc2)
    {
        // If the raptr.exe or fraps.exe process is running, warn the user for possible conflicts with the server
        if (isRaptrRunning || isFrapsRunning)
        {
            auto message = GetRaptErrMessage(isRaptrRunning, isFrapsRunning);

            int userAction = acMessageBox::instance().warning(afGlobalVariablesManager::instance().ProductNameA(), message, QMessageBox::Yes | QMessageBox::No);

            if (userAction == QMessageBox::Yes)
            {
                if (isRaptrRunning)
                {
                    rc1 = m_remoteGraphicsBackendServerLauncher.KillRunningProcess(GPU_STR_RaptrExeName, errorMessage);
                    GT_ASSERT(rc1);
                }

                if (isFrapsRunning)
                {
                    rc1 = m_remoteGraphicsBackendServerLauncher.KillRunningProcess(GPU_STR_FrapsExeName, errorMessage);
                    GT_ASSERT(rc1);
                }
            }
        }
    }
#endif
    HandleRunningProjectProcess();

}

void gpExecutionMode::HandleRunningProjectProcess()
{
    const apProjectSettings& currentSettings = afProjectManager::instance().currentProjectSettings();
    gtString errorMessage;
    gtString  projectExecutableFileName;
    currentSettings.executablePath().getFileNameAndExtension(projectExecutableFileName);
    bool isProjectExecutableAlreadyRunning = false;

    if (projectExecutableFileName.isEmpty() == false)
    {
        GT_IF_WITH_ASSERT(true == m_remoteGraphicsBackendServerLauncher.IsProcessRunning(projectExecutableFileName, isProjectExecutableAlreadyRunning, errorMessage))
        {
            if (isProjectExecutableAlreadyRunning)
            {
                QString message = QString(GP_Str_WarningProjectProcessIsRunning).arg(acGTStringToQString(projectExecutableFileName));
                int userAction = acMessageBox::instance().warning(afGlobalVariablesManager::instance().ProductNameA(), message, QMessageBox::Yes | QMessageBox::No);

                if (userAction == QMessageBox::Yes)
                {
                    bool rc3 = m_remoteGraphicsBackendServerLauncher.KillRunningProcess(projectExecutableFileName, errorMessage);
                    GT_ASSERT(rc3);
                }
            }
        }
    }
}

QString gpExecutionMode::GetRaptErrMessage(const bool isRaptrRunning, const bool isFrapsRunning) const
{
    QString runningExe = isRaptrRunning ? GPU_STR_RaptrExeNameA : "";

    if (isFrapsRunning)
    {
        if (isRaptrRunning)
        {
            runningExe.append(" and ");
            runningExe.append(GPU_STR_FrapsExeNameA);
        }
    }

    QString message = QString(GP_Str_WarningRaptrProcessIsRunning).arg(runningExe);
    return message;
}

void gpExecutionMode::HandleSessionDeletion(GPUSessionTreeItemData* pSessionData)
{
    OS_DEBUG_LOG_TRACER;
    // Sanity check:
    GT_IF_WITH_ASSERT(pSessionData != nullptr)
    {
        // Make sure that the session is deleted on the server as well
        gtString errorMessage;
        gtString sessionName = acQStringToGTString(pSessionData->m_displayName);
        gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();
        bool rcDelete = m_remoteGraphicsBackendServerLauncher.DeleteRemoteSession(projectName, sessionName, errorMessage);
        GT_ASSERT(rcDelete);
    }
}

void gpExecutionMode::onExecutionModeChanged()
{
    OS_DEBUG_LOG_TRACER;
    
    

    if (afExecutionModeManager::instance().isActiveMode(PM_STR_FrameAnalysisMode))
    {
        InitializeCodeXLRemoteAgent();
    }
    else  if (afProjectManager::instance().currentProjectSettings().isRemoteTarget() == false)
    {
        TerminateRemoteAgent();
    }
}

void gpExecutionMode::UpdateApplicationTitle()
{
    // only update the application title in standalone (doesn't make sense in VS - vspApplicationCommands::setApplicationCaption will assert)
    if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
        {
            // Set the application caption:
            gtString titleBarString;
            afCalculateCodeXLTitleBarString(titleBarString);
            afApplicationCommands::instance()->setApplicationCaption(titleBarString);

            // Update the tree root text:
            afApplicationTree* pApplicationTree = pApplicationCommands->applicationTree();
            GT_IF_WITH_ASSERT(pApplicationTree != NULL)
            {
                pApplicationTree->updateTreeRootText();
            }
        }
    }
}

bool gpExecutionMode::LaunchServer(bool& messageShown, const gtASCIIString& frameXMLFullPath)
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    messageShown = false;
    // launch the server:
    osFilePath serverPath;
    gtString capturePlayerPathAsStr;
    bool rc = GetFrameAnalisysServerPaths(capturePlayerPathAsStr, serverPath);
    GT_IF_WITH_ASSERT(true == rc)
    {
        gtString commandArgs;
        const apProjectSettings& currentSettings = afProjectManager::instance().currentProjectSettings();
        gtString exePath = frameXMLFullPath.isEmpty() ? currentSettings.executablePath().asString() : capturePlayerPathAsStr;
        exePath.replace(L"\\", L"\\\\");
        commandArgs.appendFormattedString(L"\"%ls\"", exePath.asCharArray());

        if (frameXMLFullPath.isEmpty())
        {
            // Add command line arguments only to the server
            if (!currentSettings.commandLineArguments().isEmpty())
            {
                gtString cmdArgs = currentSettings.commandLineArguments();
                cmdArgs.replace(L"\\", L"\\\\");

                commandArgs.appendFormattedString(L" --appargs \"%ls\"", cmdArgs.asCharArray());
            }
        }

        if (!frameXMLFullPath.isEmpty())
        {
            gtString xmlAsArg;
            xmlAsArg.fromASCIIString(frameXMLFullPath.asCharArray());
            xmlAsArg.replace(L"\\", L"\\\\");

            commandArgs.appendFormattedString(L" --appargs \"%ls\"", xmlAsArg.asCharArray());
        }

        if (!currentSettings.workDirectory().asString().isEmpty())
        {
            gtString workDir = currentSettings.workDirectory().asString().asCharArray();
            workDir.append(osFilePath::osPathSeparator);
            workDir.replace(L"\\", L"\\\\");

            commandArgs.appendFormattedString(L" --appworkingdir \"%ls\"", workDir.asCharArray());
        }

        // Add the port number to the command line arguments
        commandArgs.appendFormattedString(L" --port \"%d\"", m_settings.m_serverConnectionPort);

        // Initialize the CodeXL remote agent
        retVal = InitializeCodeXLRemoteAgent();

        m_remoteGraphicsBackendServerLauncher.Init(serverPath, commandArgs, currentSettings.workDirectory());
        gtString errorMsg;
        retVal = m_remoteGraphicsBackendServerLauncher.ExecuteRemoteGraphicsBackendServer(true, errorMsg);

        if (retVal == false)
        {
            acMessageBox::instance().warning(afGlobalVariablesManager::ProductNameA(), acGTStringToQString(errorMsg));
            messageShown = true;
        }
    }

    if (retVal)
    {
        // Connect to the graphics server
        rc = ConnectToServer();
    }

    return retVal;
}

bool gpExecutionMode::InitializeCodeXLRemoteAgent()
{
    bool retVal = true;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);


    // initiate the remote agent
    if (0 == m_cxlAgentProcessID || osIsProcessAlive(GPU_STR_CodeXLAgent) == false)
    {
        retVal = false;
        osFilePath codeXLAgent(osFilePath::OS_CODEXL_BINARIES_PATH);
        gtString codeXLAgentAsStr = codeXLAgent.asString();
        codeXLAgentAsStr.append(osFilePath::osPathSeparator);

        codeXLAgentAsStr.append(GPU_STR_CodeXLAgent);
        codeXLAgent.setFullPathFromString(codeXLAgentAsStr);

        osProcessHandle processHandle;
        osThreadHandle threadHandle;

        bool shouldDisplayServerWindow = false;
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
        // When debugging CodeXL, open the server window to detect connection problems
        shouldDisplayServerWindow = true;
#endif
        osFilePath workDir = codeXLAgent.fileDirectoryAsString();

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
         const auto cxlAgentArgs = L"--ip " GPU_STR_CodeXLAgentHomeIP;
#else
     const auto cxlAgentArgs = L"";
#endif    
        if (osLaunchSuspendedProcess(codeXLAgent, cxlAgentArgs, workDir, m_cxlAgentProcessID, processHandle, threadHandle, shouldDisplayServerWindow))
        {
            osResumeSuspendedProcess(m_cxlAgentProcessID, processHandle, threadHandle, true);
            retVal = true;
        }
    }

    return retVal;
}

bool gpExecutionMode::ConnectToServer()
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    // Get the server IP and port
    const apProjectSettings& projectSettings = afProjectManager::instance().currentProjectSettings();
    gtASCIIString serverIpAddressOrName;

    if (projectSettings.isRemoteTarget())
    {
        serverIpAddressOrName = projectSettings.remoteTargetName().asASCIICharArray();
    }
    else
    {
        serverIpAddressOrName = "127.0.0.1";
    }

    // Delete old instance of the graphics server
    if (m_pGraphicsServerCommunication != nullptr)
    {
        delete m_pGraphicsServerCommunication;
    }

    // Allocate a graphic server communication
    m_pGraphicsServerCommunication = new GraphicsServerCommunication(serverIpAddressOrName, m_settings.m_serverConnectionPort);

    // Write a message to the debug log - trying to connect to the server
    gtString logMsg;
    logMsg.appendFormattedString(GPU_STR_Attempting_To_Connect_To_Frame_Analysis_Server, projectSettings.remoteTargetName().asCharArray(), m_settings.m_serverConnectionPort);
    OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);

    // Attempt to connect to the server
    retVal = m_pGraphicsServerCommunication->ConnectServer("");

    return retVal;
}

void gpExecutionMode::UpdateCapturedFrameFromServer()
{
    OS_DEBUG_LOG_TRACER;
    GT_IF_WITH_ASSERT(gpUIManager::Instance()->CurrentlyRunningSessionData() != nullptr)
    {
        gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();
        gtString sessionName = acQStringToGTString(gpUIManager::Instance()->CurrentlyRunningSessionData()->m_displayName);
        gtString capturedFramesAsXML, errorMessage;
        bool isRetryEnabled = true;
        bool rc = m_remoteGraphicsBackendServerLauncher.GetCapturedFrames(projectName, sessionName, &m_capturedFramesUpdateTime, capturedFramesAsXML, isRetryEnabled, errorMessage);
        GT_IF_WITH_ASSERT(rc)
        {
            gpUIManager::Instance()->AddCapturedFramesFromServer(capturedFramesAsXML);
        }

        m_capturedFramesUpdateTime.setFromCurrentTime();
    }
}

void gpExecutionMode::Terminate()
{
    gtVector<gtString> fileNamesToTerminate{ GPU_STR_CodeXLAgent, GPU_STR_perfStudioServer64, GPU_STR_GraphicsCapturePlayer64 };
    osProcessId currentProcessId = osGetCurrentProcessId();
    osTerminateProcessesByName(fileNamesToTerminate, currentProcessId, false);
}

void gpExecutionMode::GetToolbarStartButtonText(gtString& buttonText, bool fullString /*= true*/)
{
    gtString exeFileName;
    afProjectManager::instance().currentProjectSettings().executablePath().getFileNameAndExtension(exeFileName);

    buttonText = GPU_STR_executionStartButton;

    if (!exeFileName.isEmpty() && fullString)
    {
        buttonText.appendFormattedString(AF_STR_playButtonExeNameOnly, exeFileName.asCharArray());
    }
}

bool gpExecutionMode::PrepareTraceFile(const osFilePath& sessionFile, FrameIndex frameIndex, SessionTreeNodeData* pTreeNodeData, gpBaseSessionView* pTraceView, bool prepareTraceData)
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    if (frameIndex.first < 0)
    {
        // Extract the frame index from the file path
        gtString fileName;
        sessionFile.getFileName(fileName);
        int pos = fileName.findLastOf(L"_");

        GT_IF_WITH_ASSERT(pos >= 0)
        {
            // Get the frame/s string
            gtString frameIndexStr;
            fileName.getSubString(pos + 1, fileName.length(), frameIndexStr);

            // Check if this is a single / multi frame file
            int separatorPos = frameIndexStr.findFirstOf(L"-");
            if (separatorPos < 0)
            {
                retVal = frameIndexStr.toIntNumber(frameIndex.first);
                frameIndex.second = frameIndex.first;
            }
            else
            {
                gtString frameIndexStr1, frameIndexStr2;
                frameIndexStr.getSubString(0, separatorPos - 1, frameIndexStr1);
                frameIndexStr.getSubString(separatorPos+1, frameIndexStr.length() - 1, frameIndexStr2);
                retVal = frameIndexStr1.toIntNumber(frameIndex.first);
                retVal = retVal && frameIndexStr2.toIntNumber(frameIndex.second);
            }
            GT_ASSERT(retVal);
        }
    }


    GT_IF_WITH_ASSERT(frameIndex.first > 0)
    {
        // Make sure that the frame trace is written to the trace file
        gpExecutionMode* pModeManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
        GT_IF_WITH_ASSERT(pModeManager != nullptr)
        {
            // We should look for the frame owner session file path (needed to calculate the frame file paths)
            afApplicationTreeItemData* pSessionData = ProfileApplicationTreeHandler::instance()->FindParentSessionItemData(pTreeNodeData->m_pParentData);
            GT_IF_WITH_ASSERT(pSessionData != nullptr)
            {
                osFilePath sessionFilePath = pSessionData->m_filePath;
                gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();
                gtString sessionName = acQStringToGTString(pTreeNodeData->m_displayName);

                gtASCIIString traceAsText;
                // Make sure that the frame trace is written to the trace file
                gpExecutionMode* pModeManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
                GT_IF_WITH_ASSERT(pModeManager != nullptr)
                {
                    osFilePath traceFilePath = gpTreeHandler::Instance().GetFrameChildFilePath(sessionFile, frameIndex, AF_TREE_ITEM_GP_FRAME_TIMELINE);

                    retVal = pModeManager->GetFrameTraceFromServer(sessionFilePath, frameIndex, traceFilePath);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        if (prepareTraceData == true)
                        {
                            gpUIManager::Instance()->PrepareTraceData(sessionFile, pTraceView, qobject_cast<GPUSessionTreeItemData*>(pTreeNodeData));
                        }
                    }
                }
            }
        }
    }

    return retVal;
}
