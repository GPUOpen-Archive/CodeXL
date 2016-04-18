#ifndef __AsyncRemoteGpuProfilingTask_h
#define __AsyncRemoteGpuProfilingTask_h
#include <list>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTRemoteClient/Include/CXLDaemonClient.h>

// An interface that allow other objects to consume
// error messages for errors which occur in the async task.
class IAsyncErrorMessageConsumer
{
public:
    virtual ~IAsyncErrorMessageConsumer() {}
    virtual void ConsumeErrorMessage(const QString& errorMsg) = 0;
};

class AsyncRemoteGpuProfilingTask :
    public osThread
{
public:
    AsyncRemoteGpuProfilingTask(bool isCounterFileRequired, const gtString& strCounterFile,
                                bool isEnvVarFileRequired, bool isApiRulesFileRequired, bool isApiFilterFileRequired, const gtString& apiRulesFile,
                                const gtString& strArguments, const gtString& tempEnvVarFileName, const gtString& apiFilterFileName, const gtString& profileOutputDir, bool isKernelSpecific, const gtString& specificKernels);
    ~AsyncRemoteGpuProfilingTask(void);

    // Register to get notifications about errors which occur in the remote
    // profiling process.
    void Register(IAsyncErrorMessageConsumer* pAsyncErrorMessageConsumer);

    virtual int entryPoint();
private:
    // private constructor for enabling compilation
    AsyncRemoteGpuProfilingTask() = delete;
    AsyncRemoteGpuProfilingTask& operator=(AsyncRemoteGpuProfilingTask const&) = delete;

    const gtString mCounterFileName;
    const gtString mApiRulesFileName;
    const gtString mTempEnvVarFileName;
    const gtString mApiFilterFileName;
    const gtString mProfileOutputDir;
    const gtString mCmdLineArgsStr;
    const gtString mSpecificKernels;
    bool mIsCounterFileRequired;
    bool mIsEnvVarFileRequired;
    bool mIsApiRulesFileRequired;
    bool mIsApiFilterFileRequired;
    bool mIsKernelSpecific;


    // Holds all object that would like to consume error messages.
    std::list<IAsyncErrorMessageConsumer*> mErrorObservers;

    bool ExecuteRemoteGpuProfiling(bool isRetryEnabled, QString& strErrorMessageOut);

    bool RemoteGpuProfilingHelper(CXLDaemonClient* pDmnClient, QString& strErrorMessageOut, bool& isRetryRelevant);
};


#endif // __AsyncRemoteGpuProfilingTask_h