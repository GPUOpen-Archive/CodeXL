//------------------------------ gpRemoteGraphicsBackendLauncher.cpp ------------------------------
// Qt.
#ifdef _WIN32
__pragma(warning(push))
__pragma(warning(disable: 4127))
#endif
#include <QtCore>

// Local:
#include <AMDTGpuProfiling/gpRemoteGraphicsBackendHandler.h>
#include <AMDTGpuProfiling/gpExecutionMode.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#ifdef _WIN32
    __pragma(warning(pop))
#endif

// Framework.
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Infra.
#include <AMDTOSWrappers/Include/osPortAddress.h>

// The timeout for the communication channel with the remote agent (in milliseconds).
const unsigned REMOTE_CLIENT_TIMEOUT_MS = 15000;

// The remote client is not used for the 64-bit Windows version of the process debugger:

#if !((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) && (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE))
    #include <AMDTRemoteClient/Include/CXLDaemonClient.h>
#endif

////////////////////// End Command helpers structs //////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        gpRemoteGraphicsBackendLauncher::gpRemoteGraphicsBackendLauncher
// Description: Constructor
// Author:
// Date:
// ---------------------------------------------------------------------------
gpRemoteGraphicsBackendHandler::gpRemoteGraphicsBackendHandler() : m_isHandshakeOnce(false)
{

}

// ---------------------------------------------------------------------------
// Name:        gpRemoteGraphicsBackendLauncher::~gpRemoteGraphicsBackendLauncher
// Description: Destructor
// Author:
// Date:
// ---------------------------------------------------------------------------
gpRemoteGraphicsBackendHandler::~gpRemoteGraphicsBackendHandler()
{

}

void gpRemoteGraphicsBackendHandler::Init(const osFilePath& serverPath, const gtString& commandArgs, const osDirectory& workDirectory)
{
    m_cmdLineArgsStr = commandArgs;
    m_serverPath = serverPath;
    m_workDirectory = workDirectory;
    m_isHandshakeOnce = false;
}

bool gpRemoteGraphicsBackendHandler::ExecuteRemoteGraphicsBackendServer(bool isRetryEnabled, gtString& strErrorMessageOut)
{
    LaunchGBEServerCommand launchGBackendServerCommnad(m_cmdLineArgsStr, m_serverPath, m_workDirectory);
    bool retVal = ExecuteRemoteCommand(isRetryEnabled, strErrorMessageOut, &launchGBackendServerCommnad);
    return retVal;
}

bool gpRemoteGraphicsBackendHandler::ExecuteRemoteCommand(bool isRetryEnabled, gtString& strErrorMessageOut, gpRemoteCommand* pCommand)
{
    bool retVal = false;

    // Lock the access to the remote agent
    osCriticalSectionLocker remoteAgentGuard(m_remoteAgentLock);

    if (afProjectManager::instance().currentProjectSettings().isRemoteTarget())
    {
        // Retrieve the daemon's address.
        m_dmnPort = afProjectManager::instance().currentProjectSettings().remoteTargetDaemonConnectionPort();
        m_dmnIp = afProjectManager::instance().currentProjectSettings().remoteTargetName();
    }
    else
    {
        gtVector<gtString> ipAddresses;
        osTCPSocket::getIpAddresses(ipAddresses);

        // the 127.0.0.1 is usually blocked by the firewall so we prefer using the local host name if we can find it
        m_dmnIp = GPU_STR_CodeXLAgentHomeIP;

        if (ipAddresses.size() > 0)
        {
            m_dmnIp = ipAddresses[0];
        }

        m_dmnPort = QString(AF_STR_newProject_remoteHostDefaultPort).toInt();
    }

    osPortAddress daemonAddr(m_dmnIp, m_dmnPort);

    // Initialize the daemon if required.
    retVal = CXLDaemonClient::IsInitialized(daemonAddr) || CXLDaemonClient::Init(daemonAddr, REMOTE_CLIENT_TIMEOUT_MS);
    GT_ASSERT_EX(retVal, GPU_STR_REMOTE_AGENT_INIT_FAILURE_WITH_CTX);

    // In recursive calls, make sure only one invocation handles the profile results.
    bool isHandleProfileResultRequired = true;

    CXLDaemonClient* pDmnClient = CXLDaemonClient::GetInstance();
    GT_IF_WITH_ASSERT(pDmnClient != NULL)
    {
        if (retVal)
        {
            // Connect to the daemon.
            osPortAddress addrBuffer;
            retVal = pDmnClient->ConnectToDaemon(addrBuffer);
            GT_IF_WITH_ASSERT(retVal)
            {
                bool isRetryRelevant = true;
                retVal = RemoteSessionHelper(pDmnClient, strErrorMessageOut, isRetryRelevant, pCommand);

                if (!retVal && isRetryEnabled && isRetryRelevant)
                {
                    // Give it another shot. This is for scenarios where the remote daemon was terminated and then re-launched.
                    strErrorMessageOut.assign(L"");
                    bool isTerminationSuccess = pDmnClient->TerminateWholeSession();
                    GT_ASSERT_EX(isTerminationSuccess, GPU_STR_REMOTE_SESSION_TERMINATION_FAILURE);
                    retVal = CXLDaemonClient::Init(daemonAddr, REMOTE_CLIENT_TIMEOUT_MS, true) && ExecuteRemoteGraphicsBackendServer(false, strErrorMessageOut);

                    // No need to handle profile result, since the recursive call will do it for us.
                    isHandleProfileResultRequired = false;
                }
                else if (!isRetryRelevant)
                {
                    // Probably a communication loss issue. Notify the user.
                    gtString errMsg = GPU_STR_REMOTE_COMMUNICATION_FAILURE;

                    if (!strErrorMessageOut.isEmpty())
                    {
                        // If we got a more specific error message, then we
                        // should use it instead of the default message.
                        errMsg = strErrorMessageOut;
                    }

                    OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
                    strErrorMessageOut = errMsg;
                }
            }
            else
            {
                gtString errMsg = afProjectManager::instance().currentProjectSettings().isRemoteTarget() ? GPU_STR_REMOTE_CONNECTION_FAILURE : GPU_STR_LOCAL_REMOTE_CONNECTION_FAILURE;
                OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
                strErrorMessageOut = errMsg;
            }
        }
    }
    else
    {
        gtString errMsg = GPU_STR_REMOTE_AGENT_INIT_FAILURE;
        OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
        strErrorMessageOut = errMsg;
    }

    return retVal;
}

bool gpRemoteGraphicsBackendHandler::RemoteSessionHelper(CXLDaemonClient* pDmnClient, gtString& strErrorMessageOut, bool& isRetryRelevant, gpRemoteCommand* pCommand)
{
    bool retVal = m_isHandshakeOnce;

    if (!retVal)
    {
        GT_IF_WITH_ASSERT(pDmnClient != nullptr)
        {
            // Verify the daemon's version.
            gtString handshakeErrorStrBuffer;
            retVal = m_isHandshakeOnce = pDmnClient->PerformHandshake(m_isHandshakeOnce, handshakeErrorStrBuffer);
            GT_ASSERT_EX(retVal, GPU_STR_REMOTE_HANDSHAKE_FALIURE);

            // Basically, retry should be helpful only if the first connection failed.
            isRetryRelevant = !retVal;
            m_isHandshakeOnce = retVal;

            // If handshake failed, output an error message
            if (!retVal)
            {
                if (handshakeErrorStrBuffer.isEmpty())
                {
                    handshakeErrorStrBuffer = GPU_STR_REMOTE_HANDSHAKE_UNKNOWN_FAILURE;
                }

                OS_OUTPUT_DEBUG_LOG(handshakeErrorStrBuffer.asCharArray(), OS_DEBUG_LOG_ERROR);
                strErrorMessageOut = handshakeErrorStrBuffer;
            }
        }
    }

    if (retVal)
    {
        GT_IF_WITH_ASSERT(pCommand != nullptr)
        {
            // Execute the command
            RemoteClientError errorCode = rceUnknown;
            retVal = pCommand->ExecuteCommand(pDmnClient, errorCode);

            if (!retVal)
            {
                if (errorCode == rceTargetAppNotFound)
                {
                    // Set the relevant error message.
                    strErrorMessageOut = acQStringToGTString(GPU_STR_REMOTE_TARGET_APP_NOT_FOUND);
                }
                else  if (errorCode == rcePortUnavailable)
                {
                    // Set the relevant error message.
                    strErrorMessageOut = GPU_STR_REMOTE_TARGET_APP_PORT_OCCUPIED;
                    unsigned int port = ProfileManager::Instance()->GetFrameAnalysisModeManager()->ProjectSettings().m_serverConnectionPort;
                    strErrorMessageOut.replace(GPU_STR_PORT_NNN, gtString(to_wstring(port).c_str()));

                }
                else  if (errorCode == rceTargetAppIsAlreadyRunning)
                {
                    // Set the relevant error message.
                    strErrorMessageOut = acQStringToGTString(GPU_STR_REMOTE_TARGET_APP_ALREADY_RUNNING);
                }
                else
                {
                    gtString errMsg = GPU_STR_REMOTE_AGENT_VERSION_NOT_RETRIEVED;
                    OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
                    strErrorMessageOut = errMsg;
                }

            }
        }
    }

    return retVal;
}

bool gpRemoteGraphicsBackendHandler::TerminateRemoteGraphicsBeckendServer()
{
    osPortAddress daemonAddr(m_dmnIp, m_dmnPort);

    // Initialize the daemon if required.
    bool retVal = CXLDaemonClient::IsInitialized(daemonAddr) || CXLDaemonClient::Init(daemonAddr, REMOTE_CLIENT_TIMEOUT_MS);
    GT_ASSERT_EX(retVal, GPU_STR_REMOTE_AGENT_INIT_FAILURE_WITH_CTX);

    CXLDaemonClient* pDmnClient = CXLDaemonClient::GetInstance();
    GT_IF_WITH_ASSERT(pDmnClient != NULL)
    {
        if (retVal)
        {
            // Connect to the daemon.
            osPortAddress addrBuffer;
            retVal = pDmnClient->ConnectToDaemon(addrBuffer);
            GT_IF_WITH_ASSERT(retVal)
            {
                retVal = pDmnClient->TerminateRemoteProcess(romGRAPHICS);
            }
            pDmnClient->Close();
        }
    }
    return retVal;
}

bool gpRemoteGraphicsBackendHandler::GetCapturedFrames(const gtString& projectName,
                                                       gtString& capturedFramesXmlStr,
                                                       bool&     isRetryEnabled,
                                                       gtString& strErrorMessageOut)
{
    GetCapturedFramesCommand capturedFramesCommnad(projectName, capturedFramesXmlStr);
    bool retVal = ExecuteRemoteCommand(isRetryEnabled, strErrorMessageOut, &capturedFramesCommnad);

    return retVal;

}
bool gpRemoteGraphicsBackendHandler::GetCapturedFrames(const gtString& projectName,
                                                       const gtString& sessionName,
                                                       const osTime* pCapturedMinimalTime,
                                                       gtString& capturedFramesXmlStr,
                                                       bool& isRetryEnabled,
                                                       gtString& strErrorMessageOut)
{
    GT_ASSERT(pCapturedMinimalTime != nullptr);
    GetCapturedFramesCommandByTime command(projectName, sessionName, *pCapturedMinimalTime, capturedFramesXmlStr);
    bool retVal = ExecuteRemoteCommand(isRetryEnabled, strErrorMessageOut, &command);
    return retVal;
}

bool gpRemoteGraphicsBackendHandler::IsProcessRunning(const gtString& processName, bool& isProcessRunning, gtString& strErrorMessageOut)
{
    IsProcessRunningCommand isProcessRunningCommand(processName, isProcessRunning);
    bool isRetryEnabled = false;
    bool retVal = ExecuteRemoteCommand(isRetryEnabled, strErrorMessageOut, &isProcessRunningCommand);

    return retVal;
}

bool gpRemoteGraphicsBackendHandler::KillRunningProcess(const gtString& processName, gtString& strErrorMessageOut)
{
    KillRunningProcessCommand killRunningProcessCommand(processName);
    bool isRetryEnabled = false;
    bool retVal = ExecuteRemoteCommand(isRetryEnabled, strErrorMessageOut, &killRunningProcessCommand);

    return retVal;
}

bool gpRemoteGraphicsBackendHandler::DeleteRemoteSession(const gtString& projectName, const gtString& sessionName, gtString& strErrorMessageOut)
{
    DeleteRemoteSessionCommand deleteRemoteSessionCommand(projectName, sessionName);
    bool isRetryEnabled = false;
    bool retVal = ExecuteRemoteCommand(isRetryEnabled, strErrorMessageOut, &deleteRemoteSessionCommand);

    return retVal;
}

bool gpRemoteGraphicsBackendHandler::GetFrameData(const gtString& projectName,
                                                  const gtString& sessionName,
                                                  int frameIndex, FrameInfo& frameData,
                                                  bool& isRetryEnabled,
                                                  gtString& strErrorMessageOut)
{
    GetCapturedFrameDataCommand capturedFrameDataCommnad(projectName, sessionName, frameIndex, frameData);
    bool retVal = ExecuteRemoteCommand(isRetryEnabled, strErrorMessageOut, &capturedFrameDataCommnad);

    return retVal;
}

LaunchGBEServerCommand::LaunchGBEServerCommand(const gtString& cmdLineArgsStr, const osFilePath& serverPath, const osDirectory& workDirectory) :
    m_cmdLineArgsStr(cmdLineArgsStr),
    m_serverPath(serverPath),
    m_workDirectory(workDirectory)
{

}

bool LaunchGBEServerCommand::ExecuteCommand(CXLDaemonClient* pDmnClient, RemoteClientError& errorCode) const
{
    // Launch CodeXLGpuProfiler on the remote machine, and wait for it to finish.
    bool retVal = false;
    GT_IF_WITH_ASSERT(pDmnClient != nullptr)
    {
        retVal = pDmnClient->LaunchGraphicsBeckendServer(m_serverPath, m_cmdLineArgsStr, m_workDirectory, errorCode);
    }
    return retVal;
}

GetCapturedFramesCommand::GetCapturedFramesCommand(const gtString& projectName, gtString& capturedFramesXmlStr) :
    m_projectName(projectName),
    m_capturedFramesXmlStr(capturedFramesXmlStr)
{

}

bool GetCapturedFramesCommand::ExecuteCommand(CXLDaemonClient* pDmnClient, RemoteClientError& errorCode) const
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(pDmnClient != nullptr)
    {
        retVal = pDmnClient->GetCapturedFrames(m_projectName, m_capturedFramesXmlStr, errorCode);
    }
    return retVal;
}

GetCapturedFramesCommandByTime::GetCapturedFramesCommandByTime(const gtString& projectName, const gtString& session, const osTime& capturedMinimalTime, gtString& capturedFramesXmlStr) :
    GetCapturedFramesCommand(projectName, capturedFramesXmlStr),
    m_session(session),
    m_capturedMinimalTime(capturedMinimalTime)
{

}

bool GetCapturedFramesCommandByTime::ExecuteCommand(CXLDaemonClient* pDmnClient, RemoteClientError& errorCode) const
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(pDmnClient != nullptr)
    {
        retVal = pDmnClient->GetCapturedFrames(m_projectName, m_session, &m_capturedMinimalTime, m_capturedFramesXmlStr, errorCode);
    }
    return retVal;
}

GetCapturedFrameDataCommand::GetCapturedFrameDataCommand(const gtString& projectName, const gtString& sessionName, const int frameIx, FrameInfo& frameData) :
    m_projectName(projectName),
    m_sessionName(sessionName),
    m_frameIx(frameIx),
    m_frameData(frameData)
{

}

bool GetCapturedFrameDataCommand::ExecuteCommand(CXLDaemonClient* pDmnClient, RemoteClientError& errorCode) const
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(pDmnClient != nullptr)
    {
        retVal = pDmnClient->GetCapturedFrameData(m_projectName, m_sessionName, m_frameIx, m_frameData, errorCode);
    }
    return retVal;
}

IsProcessRunningCommand::IsProcessRunningCommand(const gtString& processName, bool& isProcessRunning) : m_processName(processName), m_isProcessRunning(isProcessRunning)
{

}

bool IsProcessRunningCommand::ExecuteCommand(CXLDaemonClient* pDmnClient, RemoteClientError& errorCode) const
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(pDmnClient != nullptr)
    {
        // Check if the requested process is running on the server
        m_isProcessRunning = false;
        retVal = pDmnClient->IsProcessRunning(m_processName, m_isProcessRunning, errorCode);
    }

    return retVal;
}

KillRunningProcessCommand::KillRunningProcessCommand(const gtString& processName) : m_processName(processName)
{

}

bool KillRunningProcessCommand::ExecuteCommand(CXLDaemonClient* pDmnClient, RemoteClientError& errorCode) const
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(pDmnClient != nullptr)
    {
        // Delete the runningRaptr.exe if it is running on the server
        retVal = pDmnClient->KillRunningProcess(m_processName, errorCode);
    }
    return retVal;
}

DeleteRemoteSessionCommand::DeleteRemoteSessionCommand(const gtString& projectName, const gtString& sessionName) : m_projectName(projectName), m_sessionName(sessionName)
{

}

bool DeleteRemoteSessionCommand::ExecuteCommand(CXLDaemonClient* pDmnClient, RemoteClientError& errorCode) const
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(pDmnClient != nullptr)
    {
        // Delete the requested session on the remote agent machine
        retVal = pDmnClient->DeleteFrameAnalysisSession(m_projectName, m_sessionName, errorCode);
    }
    return retVal;
}
