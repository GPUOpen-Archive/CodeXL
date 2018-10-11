// Qt.
#include <QtCore>
#include <QtWidgets>

// Local.
#include "AsyncRemoteGpuProfilingTask.h"
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>

// Infra.
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTBaseTools//Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osApplication.h>

// Framework.
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apProfileProcessTerminatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apProfileProgressEvent.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Remote client.
#include <AMDTRemoteClient/Include/RemoteClientDataTypes.h>

// The timeout for the communication channel with the remote agent (in milliseconds).
const unsigned g_REMOTE_CLIENT_TIMEOUT_MS = 1500;

// This class will handle the progress bar animation.
class AsyncProgressUpdater :
    public osThread
{
public:

    AsyncProgressUpdater() : osThread(GPU_STR_REMOTE_PROFILING_TASK_THREAD_NAME), mIsInProgress(true)
    {
        gtString progBarMsg = GPU_STR_REMOTE_PROFILE_IN_PROGRESS_PREFIX;
        progBarMsg.append(afProjectManager::instance().currentProjectSettings().remoteTargetName());
    }

    virtual int entryPoint()
    {
        unsigned char counter = 1;
        gtString progBarMsg = GPU_STR_REMOTE_PROFILE_IN_PROGRESS_PREFIX;
        progBarMsg.append(afProjectManager::instance().currentProjectSettings().remoteTargetName());

        while (mIsInProgress)
        {
            unsigned char val = counter++;

            if (counter > 3)
            {
                counter = 1;
            }

            gtString strToShow = progBarMsg;

            switch (val)
            {
                case 1:
                    strToShow.append(L".");
                    break;

                case 2:
                    strToShow.append(L"..");
                    break;

                case 3:
                    strToShow.append(L"...");
                    break;

                default:
                    strToShow.append(L".");
                    break;
            }

            // Notify.
            static const gtString profileName(GPU_STR_GENERAL_SETTINGS);
            apProfileProgressEvent eve(profileName, strToShow, 0);
            apEventsHandler::instance().registerPendingDebugEvent(eve);
            osSleep(500);
        }

        return 0;
    }

    void Stop()
    {
        mIsInProgress = false;
    }

private:
    bool mIsInProgress;

};




AsyncRemoteGpuProfilingTask::AsyncRemoteGpuProfilingTask(bool isCounterFileRequired, const gtString& strCounterFile,
                                                         bool isEnvVarFileRequired, bool isApiRulesFileRequired, bool isApiFilterFileRequired, const gtString& apiRulesFile, const gtString& strArguments,
                                                         const gtString& tempEnvVarFileName, const gtString& apiFilterFileName, const gtString& profileOutputDir, bool isKernelSpecific, const gtString& specificKernels) :
    osThread(L"AsyncRemoteGpuProfilingTask"),
    mCounterFileName(strCounterFile),
    mApiRulesFileName(apiRulesFile),
    mTempEnvVarFileName(tempEnvVarFileName),
    mApiFilterFileName(apiFilterFileName),
    mProfileOutputDir(profileOutputDir),
    mCmdLineArgsStr(strArguments),
    mSpecificKernels(specificKernels),
    mIsCounterFileRequired(isCounterFileRequired),
    mIsEnvVarFileRequired(isEnvVarFileRequired),
    mIsApiRulesFileRequired(isApiRulesFileRequired),
    mIsApiFilterFileRequired(isApiFilterFileRequired),
    mIsKernelSpecific(isKernelSpecific),
    mErrorObservers()
{

}


AsyncRemoteGpuProfilingTask::~AsyncRemoteGpuProfilingTask(void)
{
}

int AsyncRemoteGpuProfilingTask::entryPoint()
{
    // First, let the user know that we are attempting to start a remote GPU profiling session.
    static const gtString profileName(GPU_STR_GENERAL_SETTINGS);
    apProfileProgressEvent eve(profileName, GPU_STR_STARTING_REMOTE_SESSION, 0);
    apEventsHandler::instance().registerPendingDebugEvent(eve);

    QString errMsg;
    int rc = (ExecuteRemoteGpuProfiling(true, errMsg) ? 0 : -1);

    // Notify the observers.
    std::list<IAsyncErrorMessageConsumer*>::iterator iter;

    for (iter = mErrorObservers.begin(); iter != mErrorObservers.end(); ++iter)
    {
        if (*iter != NULL)
        {
            (*iter)->ConsumeErrorMessage(errMsg);
        }
    }

    return rc;
}

bool AsyncRemoteGpuProfilingTask::RemoteGpuProfilingHelper(CXLDaemonClient* pDmnClient, QString& strErrorMessageOut, bool& isRetryRelevant)
{
    // Verify the daemon's version.
    gtString handshakeErrorStrBuffer;
    bool isHandshakeMatch = false;
    bool retVal = pDmnClient->PerformHandshake(isHandshakeMatch, handshakeErrorStrBuffer);
    GT_ASSERT_EX(retVal, GPU_STR_REMOTE_HANDSHAKE_FALIURE);

    // Basically, retry should be helpful only if the first connection failed.
    isRetryRelevant = !retVal;

    if (retVal)
    {
        if (isHandshakeMatch)
        {
            // Handle kernel-specific profiling if required.
            gtVector<gtString> specificKernels;

            if (mIsKernelSpecific && !mSpecificKernels.isEmpty())
            {
                // Extract the kernel names.
                gtStringTokenizer tokenizer(mSpecificKernels, L";");
                gtString currKernelName;

                while (tokenizer.getNextToken(currKernelName))
                {
                    // Remove white spaces.
                    QString tmpStr = acGTStringToQString(currKernelName);
                    currKernelName = acQStringToGTString(tmpStr.trimmed());
                    specificKernels.push_back(currKernelName);
                    currKernelName.makeEmpty();
                }
            }

            // Progress notification.
            AsyncProgressUpdater* pProgUpdater = new(std::nothrow)AsyncProgressUpdater();

            pProgUpdater->execute();

            // Launch rcprof on the remote machine, and wait for it to finish.
            // An asynchronous implementation will be available in the future.
            RemoteClientError errorCode = rceUnknown;
            retVal = pDmnClient->LaunchGPUProfiler(mCmdLineArgsStr, mProfileOutputDir, mCounterFileName,
                                                   mTempEnvVarFileName, mApiFilterFileName, mApiRulesFileName, specificKernels, errorCode);

            if (errorCode == rceTargetAppNotFound)
            {
                // Do not retry.
                isRetryRelevant = false;

                // Set the relevant error message.
                strErrorMessageOut = GPU_STR_REMOTE_TARGET_APP_NOT_FOUND;
            }
            else if (errorCode == rcePortUnavailable)
            {
                // Do not retry.
                isRetryRelevant = false;

                // Set the relevant error message.
                strErrorMessageOut = QString::fromWCharArray(GPU_STR_REMOTE_TARGET_APP_PORT_OCCUPIED);
            }
            else if (errorCode == rceTargetAppIsAlreadyRunning)
            {
                // Do not retry.
                isRetryRelevant = false;

                // Set the relevant error message.
                strErrorMessageOut = GPU_STR_REMOTE_TARGET_APP_ALREADY_RUNNING;
            }

            // Stop progress notification.
            pProgUpdater->Stop();
            pProgUpdater->terminate();
            delete pProgUpdater;

        }
        else
        {
            retVal = false;

            if (handshakeErrorStrBuffer.isEmpty())
            {
                handshakeErrorStrBuffer = GPU_STR_REMOTE_HANDSHAKE_UNKNOWN_FAILURE;
            }

            OS_OUTPUT_DEBUG_LOG(handshakeErrorStrBuffer.asCharArray(), OS_DEBUG_LOG_ERROR);
            strErrorMessageOut = acGTStringToQString(handshakeErrorStrBuffer);
        }
    }
    else
    {
        gtString errMsg = GPU_STR_REMOTE_AGENT_VERSION_NOT_RETRIEVED;
        OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
        strErrorMessageOut = acGTStringToQString(errMsg);
    }

    return retVal;
}


bool AsyncRemoteGpuProfilingTask::ExecuteRemoteGpuProfiling(bool isRetryEnabled, QString& strErrorMessageOut)
{
    // Retrieve the daemon's address.
    gtUInt16 dmnPort = afProjectManager::instance().currentProjectSettings().remoteTargetDaemonConnectionPort();
    gtString dmnIp = afProjectManager::instance().currentProjectSettings().remoteTargetName();
    osPortAddress daemonAddr(dmnIp, dmnPort);

    // Initialize the daemon if required.
    bool retVal = CXLDaemonClient::IsInitialized(daemonAddr) ||
                  CXLDaemonClient::Init(daemonAddr, g_REMOTE_CLIENT_TIMEOUT_MS);
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
                retVal = RemoteGpuProfilingHelper(pDmnClient, strErrorMessageOut, isRetryRelevant);

                if (!retVal && isRetryEnabled && isRetryRelevant)
                {
                    // Give it another shot. This is for scenarios where the remote daemon was terminated and then re-launched.
                    strErrorMessageOut.clear();
                    bool isTerminationSuccess = pDmnClient->TerminateWholeSession();
                    GT_ASSERT_EX(isTerminationSuccess, GPU_STR_REMOTE_SESSION_TERMINATION_FAILURE);
                    retVal = CXLDaemonClient::Init(daemonAddr, g_REMOTE_CLIENT_TIMEOUT_MS, true) && ExecuteRemoteGpuProfiling(false, strErrorMessageOut);

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
                        errMsg = acQStringToGTString(strErrorMessageOut);
                    }

                    OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
                    strErrorMessageOut = acGTStringToQString(errMsg);
                }
            }
            else
            {
                gtString errMsg = GPU_STR_REMOTE_CONNECTION_FAILURE;
                OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
                strErrorMessageOut = acGTStringToQString(errMsg);
            }
        }

        if (isHandleProfileResultRequired)
        {
            // Disconnect from the remote agent.
            pDmnClient->TerminateWholeSession();

            // Now handle the profile result.
            apProfileProcessTerminatedEvent eve(GPU_STR_GENERAL_SETTINGS, (retVal ? 0 : -1));
            apEventsHandler::instance().registerPendingDebugEvent(eve);
        }

    }
    else
    {
        gtString errMsg = GPU_STR_REMOTE_AGENT_INIT_FAILURE;
        OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
        strErrorMessageOut = acGTStringToQString(errMsg);
    }

    return retVal;
}

void AsyncRemoteGpuProfilingTask::Register(IAsyncErrorMessageConsumer* pAsyncErrorMessageConsumer)
{
    mErrorObservers.push_back(pAsyncErrorMessageConsumer);
}

