//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CXLDaemonClient.h
///
//==================================================================================

#ifndef __CXLDaemonClient_h
#define __CXLDaemonClient_h

// Local.
#include "AMDTRemoteClientBuild.h"
#include "RemoteClientDataTypes.h"

// C++.
#include <sstream>
#include <vector>

// For the daemon communication definitions.
#include <AMDTRemoteAgent/Public Include/dmnDefinitions.h>
#include <AMDTOSWrappers/Include/osTCPSocketClient.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

// For file transfer.
#include <AMDTOSWrappers/Include/osRawMemoryBuffer.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

#define CXL_DAEMON_CLIENT CXLDaemonClient::GetInstance()
// This is required in order to forward declare these C structures.
struct AMDTPwrSample;
struct AMDTPwrDevice;
struct AMDTPwrCounterDesc;

struct ApplicationLaunchDetails
{
    // Default CTOR. No launch required.
    ApplicationLaunchDetails() : m_isLaunchRequired(false) {}

    ApplicationLaunchDetails(const gtString& remoteAppFullPath,
                             const gtString& remoteAppWorkingDir, const gtString& remoteAppCmdLineArgs,
                             const gtVector<osEnvironmentVariable>& remoteAppEnvVars) : m_isLaunchRequired(true),
        m_remoteAppFullPath(remoteAppFullPath),
        m_remoteAppWorkingDirectory(remoteAppWorkingDir),
        m_remoteAppCmdLineArgs(remoteAppCmdLineArgs),
        m_remoteAppEnvVars(remoteAppEnvVars) {}

    // True if the remote application needs to be launched.
    // False otherwise.
    bool m_isLaunchRequired;

    // The full path of the target application on the remote machine.
    gtString m_remoteAppFullPath;

    // The working directory of the target application on the remote machine.
    gtString m_remoteAppWorkingDirectory;

    // The command line arguments of the target application..
    gtString m_remoteAppCmdLineArgs;

    // The environment variables of the target application..
    gtVector<osEnvironmentVariable> m_remoteAppEnvVars;
};

enum AppLaunchStatus
{
    rasUnknown,
    rasOk,
    rasApplicationNotFound,
    rasWorkingDirNotFound,
    rasLaunchFailed,
    rasNotSupported
};

class AMDT_REMOTE_CLIENT_API CXLDaemonClient
{
public:

    // Defines a callback which handles the remote file transfer notification.
    typedef void (*FileTransferCompletedCallback)(bool isSuccess,
                                                  const gtString& remoteFileName, const gtString& localTargetFileName, void* params);

    // Initializes the CodeXL daemon client.
    static bool Init(const osPortAddress& daemonAddress, long readTimeout, bool isForcedInitialization = false);

    // Indicates whether the CodeXL daemon client is initialized to
    // interact with the given address.
    static bool IsInitialized(const osPortAddress& dmnAddress);

    // Retrieves an instance of the CodeXL daemon client.
    // If the client does not exist, or the CodeXLDaemonClient was
    // not initialized properly, NULL is returned.
    static CXLDaemonClient* GetInstance();

    // Resets the CXLDaemonClient, and returns it to non-initialized state.
    // Note that it is possible to re-initialize CXLDaemonClient after closing it.
    static void Close();

    // Pings the remote agent by connecting and disconnecting from it.
    // Return value is true iff the connection was established successfully.
    // daemonAddress - the agent's address.
    static bool ValidateConnectivity(const osPortAddress& daemonAddress, bool& isConnectivityValid);

    // Checks if remote application path and working directory are valid
    // Return true if remote query executed successfully
    // daemonAddress - the agent's address
    static bool ValidateAppPaths(const osPortAddress& daemonAddress, const gtString& appFilePath, const gtString& workingFolderPath, bool& isAppValid, bool& isWorkingFolderValid);

    // Connects to CodeXL Daemon.
    // Return value is true iff the connection was established successfully.
    // Upon success, connectionPortBuffer is filled with the ip and port address
    // of the client itself.
    bool ConnectToDaemon(osPortAddress& connectionPortBuffer);

    /// Check if a requested process is running on the remote agent
    /// \param processName the process name
    /// \param isProcessRunning[out] is the process running on the agent
    /// \return true iff the function succeeded
    bool IsProcessRunning(const gtString& processName, bool& isProcessRunning, RemoteClientError& errorCode);

    /// Kill a running process on the remote agent by it's name
    /// \param processName the process name
    /// \return true iff the function succeeded
    bool KillRunningProcess(const gtString& processName, RemoteClientError& errorCode);

    // Launches RDS on the remote machine.
    // Return value is true iff the process was launched successfully.
    bool LaunchRDS(const gtString& cmdLineArgs,  const std::vector<osEnvironmentVariable>& envVars);

    // Launches CodeXLGpuProfiler on the remote machine, waits for it to finish processing, and takes care
    // to all required file transfers between the machines.
    // Return value is true iff the profiling session completed successfully successfully.
    // Note that the function is currently blocking. An async implementation will be available
    // in the future.
    bool LaunchGPUProfiler(const gtString& cmdLineArgs, const gtString& localCodeXLGpuProfilerBaseDir,
                           const gtString& counterFileName, const gtString& envVarsFileName,
                           const gtString& ApiFilterFileName, const gtString& ApiRulesFileName,
                           const gtVector<gtString>& specificKernels, RemoteClientError& errorCode);

    // Launches PerfStudio on the remote machine.
    // Return value is true iff the process was launched successfully.
    bool LaunchGraphicsBeckendServer(const osFilePath& serverPath, const gtString& cmdLineArgs, const osDirectory& workDirectory, RemoteClientError& errorCode);

    bool GetCapturedFrames(const gtString& projectName, gtString& capturedFramesXmlStr, RemoteClientError& errorCode);
    bool GetCapturedFrames(const gtString& projectName,
                           const gtString& session,
                           const osTime* capturedMinimalTime,
                           gtString& capturedFramesXmlStr,
                           RemoteClientError& errorCode);
    bool GetCapturedFrameData(const gtString& projectName, const gtString& sessionName, const int frameIndex, FrameInfo& frameData, RemoteClientError& errorCode);

    /// Delete the requested frame analysis session on the remote agent
    /// \param projectName the session project name
    /// \param sessionName the session to delete
    /// \param errorCode the remote operation error code
    /// \return true iff the deletion succeeded
    bool DeleteFrameAnalysisSession(const gtString& projectName, const gtString& sessionName, RemoteClientError& errorCode);

    // Terminates the requested process on the remote machine.
    // The requested process (RDS, CodeXLGpuProfiler) is determined according to
    // the REMOTE_OPERATION_MODE received as a parameter.
    // Return value is true iff the process was terminated successfully.
    bool TerminateRemoteProcess(REMOTE_OPERATION_MODE mode);

    // Terminates the current session, and disconnects from the remote agent.
    bool TerminateWholeSession();

    // Checks whether the relevant session is still alive on the remote machine.
    // Return value is true iff the operation completed successfully.
    // The session status is assigned to the buffer in case that the operation had
    // completed successfully.
    bool GetSessionStatus(REMOTE_OPERATION_MODE mode, DaemonSessionStatus& buffer);

    // Transfers a file located on the remote machine and saves the file to the storage
    // in the local machine.
    // remoteFileName is the full path name of the requested file on the remote machine.
    // localTargetFileName is the full path name indicating where to save the remote file locally.
    // If isAsync is false, the function blocks and returns its success/failure indication using
    // its return value. In this case the parameters cb and params have no meaning.
    // If isAsync is true, then true will be returned immediately and the file transfer would
    // occur asynchronously. When the file transfer operation is completed, the cb callback is
    // being called with the correct parameters. The params parameter can be used to pass data
    // to cb (you can pass the this pointer, for example).
    bool GetRemoteFile(const gtString& remoteFileName, const gtString& localTargetFileName,
                       bool isAsync, FileTransferCompletedCallback cb, void* params);

    // Retrieves the CodeXL version with which the daemon works.
    // The returned string should be convertible to osProductVersion structure.
    // Version format is "<PLATFORMNAME>$<VERSIONNUMBER>", e.g. "Windows$1.3.4.5".
    bool GetDaemonCodeXLVersion(gtString& versionBuffer);

    // Verifies that the current application version matches the version of the
    // remote agent. The return value indicates whether the operation succeeded.
    // If the operation succeeds (return value is true), errorStrBuffer indicates
    // the mismatch cause.
    bool PerformHandshake(bool& resultBuffer, gtString& errorStrBuffer);

    // **********************************
    // Real-Time Power Profiling section
    // **********************************
    // Initialize a power profiling session.
    bool InitPowerProfilingSession(gtUInt32& beRetVal);

    // Set a power profiling sampling configuration.
    bool SetPowerProfilingSamplingOption(gtInt32 samplingOption, gtUInt32& beRetVal);

    // Set the sampling interval for the power profiling session.
    bool SetPowerSamplingInterval(gtUInt32 samplingInterval, gtUInt32& beApiRetVal);

    // Get the system topology.
    bool GetSystemTopology(AMDTPwrDevice*& pTreeRoot, gtUInt32& beApiRetVal);

    // Get a device's supported counters.
    bool GetDeviceSupportedCounters(gtUInt32 deviceId, gtUInt32& beApiRetVal, gtUInt32& numOfSupportedCounters, AMDTPwrCounterDesc*& pSupportedCounters);

    // Get the minimum sampling interval.
    bool GetMinSamplingIntervalMs(gtUInt32& minSamplingIntervalMs, gtUInt32& beApiRetVal);

    // Get the current sampling interval.
    bool GetCurrentSamplingIntervalMs(gtUInt32& currentSamplingIntervalMs, gtUInt32& beApiRetVal);

    // Enable counter.
    bool EnableCounter(gtUInt32 counterId, gtUInt32& beApiRetVal);

    // Disable counter.
    bool DisableCounter(gtUInt32 counterId, gtUInt32& beApiRetVal);

    // Is counter enabled.
    bool IsCounterEnabled(gtUInt32 counterId, gtUInt32& beApiRetVal);

    // Start power profiling.
    // remoteAppDetails - details about the target application.
    // beApiRetVal - return type of the API on the remote machine.
    // remoteAppLaunchStatus - PPResult value indicating the status of the remote application's launch.
    bool StartPowerProfiling(const ApplicationLaunchDetails& remoteAppDetails, gtUInt32& beApiRetVal, AppLaunchStatus& remoteAppLaunchStatus);

    // Stop power profiling.
    bool StopPowerProfiling(gtUInt32& beApiRetVal);

    // Pause power profiling.
    bool PausePowerProfiling(gtUInt32& beApiRetVal);

    // Resume power profiling.
    bool ResumePowerProfiling(gtUInt32& beApiRetVal);

    // Close power profiling session.
    bool ClosePowerProfiling(gtUInt32& beApiRetVal);

    // Get the consecutive sample.
    bool ReadAllEnabledCounters(gtUInt32& numOfSamples, AMDTPwrSample*& pSamples, gtUInt32& beApiRetVal);

    // Disconnects from the client, without closing active sessions.
    bool DisconnectWithoutClosing();

    bool GetDaemonAddress(osPortAddress&);

    // retruns true if HSA enabled on remote machine
    bool IsHSAEnabled();

    bool ValidateAppPaths(const gtString& appFilePath, const gtString& workingFolderPath, bool& isAppValid, bool& isWorkingFolderValid);

private:

    // This will have to be a global singleton, to allow for cross system
    // sharing of this object (under the current architecture's limits).
    CXLDaemonClient() = delete;
    CXLDaemonClient(const osPortAddress& daemonAddress, long readTimeout);
    ~CXLDaemonClient(void);

    class Impl;
    Impl* m_pImpl;

    // No copy.
    CXLDaemonClient(const CXLDaemonClient& other);
    CXLDaemonClient& operator=(const CXLDaemonClient& other);
};

#endif