//------------------------------ gpRemotePerfStudioLauncher.h ------------------------------

#ifndef __GPREMOTEGRAPHICSBACKENDLAUNCHER_H
#define __GPREMOTEGRAPHICSBACKENDLAUNCHER_H

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include "AMDTRemoteClient/Include/RemoteClientDataTypes.h"

class CXLDaemonClient;
class osTime;

struct gpRemoteCommand
{
    gpRemoteCommand() = default;
    virtual bool ExecuteCommand(CXLDaemonClient* pDmnClient, RemoteClientError&) const = 0;

};
// ----------------------------------------------------------------------------------
// Class Name:          gpRemoteGraphicsBackendLauncher
// General Description: Handles communication with the remote Graphics beckend server
//
//
// Author:              Naama Zur
// Creation Date:       3/11/15
// ----------------------------------------------------------------------------------
class AMDT_GPU_PROF_API gpRemoteGraphicsBackendHandler
{
public:
    gpRemoteGraphicsBackendHandler();
    virtual ~gpRemoteGraphicsBackendHandler();

    /// Initializes with data needed for remote Graphics beck end server
    /// \param serverPath the server exe path
    /// \param commandArgs the command line arguments
    /// \param workDirectory the full path of the analyzed program
    void Init(const osFilePath& serverPath, const gtString& commandArgs, const osDirectory& workDirectory);

    /// handle the remote application execution
    bool ExecuteRemoteGraphicsBackendServer(bool isRetryEnabled, gtString& strErrorMessageOut);

    ///Returns xml formatted string (capturedFramesXmlStr) with info regarding all projects sessions frames
    ///xml example : <?xml version="1.0" encoding="UTF-8"?><Sessions><Session name="Nov-30-2015_11-47-34@10.20.0.154"><Frame ix="15"><description /><image /><ltr /></Frame><Frame ix="155"><description /><image /><atl /></Frame></Session><Session name="Nov-30-2015_11-47-34@10.20.0.154 - Copy"><Frame ix="156"><description /><image /><atl /></Frame></Session></Sessions>
    bool GetCapturedFrames(const gtString& projectName, gtString& capturedFramesXmlStr, bool& isRetryEnabled,  gtString& strErrorMessageOut);

    ///Returns xml with information about captured frames
    /// Frame information filtered by session name(sessionName) and by minimal frame captured time(capturedMinimalTime),
    /// i.e. framed only with captured time above 'capturedMinimalTime' and with session name == sessionName, would be returned
    bool GetCapturedFrames(const gtString& projectName,
                           const gtString& sessionName,
                           const osTime* pCapturedMinimalTime,
                           gtString& capturedFramesXmlStr,
                           bool& isRetryEnabled,
                           gtString& strErrorMessageOut);

    /// Check if a requested process is running on the remote agent
    /// \param processName the process name
    /// \param isProcessRunning[out] is the process running on the agent
    /// \return true iff the function succeeded
    bool IsProcessRunning(const gtString& processName, bool& isProcessRunning, gtString& strErrorMessageOut);

    /// Kill a running process on the remote agent by it's name
    /// \param processName the process name
    /// \return true iff the function succeeded
    bool KillRunningProcess(const gtString& processName, gtString& strErrorMessageOut);

    /// Delete the requested process if it is running on the remote agent
    /// \param projectName the project of the session
    /// \param sessionName the session name to delete
    /// \param strErrorMessageOut and output error message is the function failed
    /// \return true iff the function succeeded
    bool DeleteRemoteSession(const gtString& projectName, const gtString& sessionName, gtString& strErrorMessageOut);

    ///Returns Specific Frame(per project and session) information such as trace , description and image
    bool GetFrameData(const gtString& projectName, const gtString& sessionName, int frameIndex, FrameInfo& frameData, bool& isRetryEnabled, gtString& strErrorMessageOut);

    /// handle ending of the application execution
    bool TerminateRemoteGraphicsBeckendServer();
private:
    /// handle the remote application execution
    bool ExecuteRemoteCommand(bool isRetryEnabled,
                              gtString& strErrorMessageOut,
                              gpRemoteCommand* pCommand);
    /// helper function which handles daemon connection
    bool RemoteSessionHelper(CXLDaemonClient* pDmnClient,
                             gtString& strErrorMessageOut,
                             bool& isRetryRelevant,
                             gpRemoteCommand* pCommand);

    /// Was handshake performed already?
    bool m_isHandshakeOnce;

    /// params for remote agent
    gtString m_profileOutputDir;
    gtString m_cmdLineArgsStr;
    osFilePath m_serverPath;
    osDirectory m_workDirectory;

    gtUInt16 m_dmnPort;
    gtString m_dmnIp;

    /// We cannot talk with the remote agent asynchronously, we need a guard before accessing it
    osCriticalSection m_remoteAgentLock;

};

/// Definitions of commands

struct LaunchGBEServerCommand : public gpRemoteCommand
{
    LaunchGBEServerCommand(const gtString& cmdLineArgsStr, const osFilePath& serverPath, const osDirectory& workDirectory);
    virtual bool ExecuteCommand(CXLDaemonClient* pDmnClient, RemoteClientError& errorCode) const override;

    /// params for remote agent
    const gtString m_cmdLineArgsStr;
    const osFilePath m_serverPath;
    const osDirectory m_workDirectory;
};

struct GetCapturedFramesCommand : public gpRemoteCommand
{
    GetCapturedFramesCommand(const gtString& projectName, gtString& capturedFramesXmlStr);
    virtual bool ExecuteCommand(CXLDaemonClient* pDmnClient, RemoteClientError& errorCode) const override;
protected:
    const gtString& m_projectName;
    gtString& m_capturedFramesXmlStr;
};
struct GetCapturedFramesCommandByTime : public GetCapturedFramesCommand
{
    GetCapturedFramesCommandByTime(const gtString& projectName, const gtString& session, const osTime& capturedMinimalTime, gtString& capturedFramesXmlStr);
    virtual bool ExecuteCommand(CXLDaemonClient* pDmnClient, RemoteClientError& errorCode) const override;

protected:
    const gtString& m_session;
    const osTime&   m_capturedMinimalTime;
};
struct GetCapturedFrameDataCommand : public gpRemoteCommand
{
    GetCapturedFrameDataCommand(const gtString& projectName, const gtString& sessionName, const int frameIx, FrameInfo& frameData);
    virtual bool ExecuteCommand(CXLDaemonClient* pDmnClient, RemoteClientError& errorCode) const override;

private:
    const gtString& m_projectName;
    const gtString& m_sessionName;
    const int m_frameIx;
    FrameInfo& m_frameData;
};

struct IsProcessRunningCommand : public gpRemoteCommand
{
    IsProcessRunningCommand(const gtString& processName, bool& isProcessRunning);
    virtual bool ExecuteCommand(CXLDaemonClient* pDmnClient, RemoteClientError& errorCode) const override;
protected:
    const gtString& m_processName;
    bool& m_isProcessRunning;

};

struct KillRunningProcessCommand : public gpRemoteCommand
{
    KillRunningProcessCommand(const gtString& processName);
    virtual bool ExecuteCommand(CXLDaemonClient* pDmnClient, RemoteClientError& errorCode) const override;
protected:
    const gtString& m_processName;

};

struct DeleteRemoteSessionCommand : public gpRemoteCommand
{
    DeleteRemoteSessionCommand(const gtString& projectName, const gtString& sessionName);
    virtual bool ExecuteCommand(CXLDaemonClient* pDmnClient, RemoteClientError& errorCode) const override;
protected:
    const gtString& m_projectName;
    const gtString& m_sessionName;

};


#endif //__GPREMOTEGRAPHICSBACKENDLAUNCHER_H

