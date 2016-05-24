//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnSessionThread.h
///
//==================================================================================

#ifndef __dmnSessionThread_h
#define __dmnSessionThread_h

#include <functional>
#include <memory>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTCPSocketServer.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtSet.h>


// Local:
#include <AMDTRemoteAgent/Public Include/dmnDefinitions.h>
#include <AMDTRemoteAgent/dmnUtils.h>
#include <AMDTRemoteAgent/dmnPowerBackendAdapter.h>

// Represents a thread that is being spawned by
// the remote agent to handle a connection with a single client.
class dmnSessionThread :
    public osThread
{
public:
    dmnSessionThread(osTCPSocketServerConnectionHandler* pConnHandler,
                     const gtString& threadName, bool syncTermination = false);
    ~dmnSessionThread(void);

    // Prints a message to the console about
    // the disconnection of the most recent client.
    void notifyUserAboutDisconnection();

    // Terminates the running session.
    void terminateSession();
    static void KillDependantProcesses();

protected:
    // The threads' entry point.
    virtual int entryPoint() override;

private:
    void LaunchProfiler(bool& isTerminationRequired);
    bool LaunchGraphicsBeckendServer();

    unsigned short GetGraphicServerPortFromArgs(const gtString& fixedCmdLineArgs) const;

    bool GetCapturedFrames();
    bool GetCapturedFramesByTime();
    bool GetCapturedFrameData();
    bool DeleteFrameAnalysisSession();
    bool GetRemoteFile();
    void TerminateWholeSession(bool& isTerminationRequired, int& ret);
    bool LaunchRds();

    /// Write the data for the requested frame after extracting it from the server file system
    /// \param frameIndex the frame index
    /// \param projectName the project name
    /// \param sessionName the session name
    void WriteFrameFilesData(int frameIndex, const gtString& projectName, const gtString& sessionName);

    /// Sends a frame analysis file content to the CodeXL client
    /// \param filePath the local file path
    /// \return true iff the file data was sent successfully
    bool SendFrameAnalysisFileData(const osFilePath& filePath);

    bool ReadFile(const osFilePath& filePath, std::unique_ptr <gtByte[]>& pBuffer, unsigned long& fileSize) const;

    typedef std::function<bool(osFilePath)> FilePathFilter;
    bool CreateCapturedFramesInfoFile(const gtString& projectName, const  FilePathFilter& sessionFilterFunc, const FilePathFilter& frameFilterFunc);
    void BuildFrameCaptureInfoNode(const gtList<osFilePath>& framesFiles, TiXmlElement* frameElement) const;
    bool GetCurrentUserFrameAnalysisFolder(osFilePath& buffer) const;
    void AddFADependantProcessToTerminationList(const osFilePath& serverPath, const gtString& arguments) const;


    bool GetDaemonCXLVersion();
    void GetDaemonPlatform();
    bool TerminateGraphicsBeckendServerSession();
    bool TerminateProfilingSession();
    bool TerminateDebuggingSession();

    /// General utilities

    /// Check if a requested process is running
    bool IsProcessRunning();

    /// Kill a running process by it's name
    bool KillRunningProcess();
    
    /// Returns true if HSA enabled 
    bool IsHSAEnabled();

    bool ValidateAppPaths();

private:
    osTCPSocketServerConnectionHandler* m_pConnHandler;
    osProcessId m_rdsProcId;
    osProcessId m_sProfProcId;
    osProcessId m_sGraphicsProcId;
    dmnPowerBackendAdapter m_powerBackendAdapter;
    bool m_isForcedTerminationRequired;

    // No copy.
    dmnSessionThread(const dmnSessionThread& other);
    const dmnSessionThread& operator=(const dmnSessionThread& other);

    bool CreateProcess(REMOTE_OPERATION_MODE mode, const gtString& cmdLineArgs, std::vector<osEnvironmentVariable>& envVars, const osFilePath& filePath = osFilePath(), const osFilePath& dirPath = osFilePath());

    /// Check if a server process exists, and kill it
    /// \param serverPath the server full path
    static void KillServerExistingProcess(const osFilePath& serverPath);

    bool terminateProcess(REMOTE_OPERATION_MODE mode);
    void releaseResources();
    /// A map containingg the list of supported extensions for frame analysis data files,
    /// with mapping to a boolean - true iff the file extension contain binary data
    static gtMap<gtString, bool> m_sFrameAnalysisFileExtensionToBinaryfileTypeMap;

    static gtSet<gtString> m_ProcessNamesTerminationSet;
};


#endif // __dmnSessionThread_h
