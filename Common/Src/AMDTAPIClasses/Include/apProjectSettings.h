//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apProjectSettings.h
///
//==================================================================================

//------------------------------ apProjectSettings.h ------------------------------

#ifndef __APPROJECTSETTINGS_H
#define __APPROJECTSETTINGS_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apProjectSettings : public osTransferableObject
// General Description: Contain data defining a project
// Author:  AMD Developer Tools Team
// Creation Date:       4/4/2012
// ----------------------------------------------------------------------------------
class AP_API apProjectSettings : public osTransferableObject
{
public:
    apProjectSettings();
    apProjectSettings(const gtString& projectName,
                      const osFilePath& executablePath,
                      const gtString& commandLineArguments,
                      const osFilePath& workDirectory);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    void setProjectName(const gtString& projName) { m_projectName = projName; };
    void setRemoteDebugging(const gtString& hostName, gtUInt16 daemonConnectionPort) { m_isRemoteTarget = true; m_remoteTargetName = hostName; m_remoteTargetDaemonConnectionPort = daemonConnectionPort; };
    void setLocalDebugging() { m_isRemoteTarget = false; m_remoteTargetName.makeEmpty(); m_remoteTargetDaemonConnectionPort = 0; };
    void setShouldDisableVSDebugEngine(bool disableVSDE) { m_doNotUseVSDebugEngine = disableVSDE; };
    void setExecutablePath(const osFilePath& exePath) { m_executablePath = exePath; };
    void setExecutablePathFromString(const gtString& exePath);
    void setWindowsStoreAppUserModelID(const gtString& userModelId) { m_winStoreAppUserModelId = userModelId; };
    void setCommandLineArguments(const gtString& commandLineArgs) { m_commandLineArguments = commandLineArgs; };
    void setLogFilesFolder(const osDirectory& logFilesFolder) { m_logFilesFolder = logFilesFolder; };
    void setLogFilesFolderFromString(const gtString& logFilesFolder) { m_logFilesFolder.setDirectoryFullPathFromString(logFilesFolder); };
    void setWorkDirectory(const osFilePath& workDir) { m_workDirectory = workDir; };
    void setWorkDirectoryFromString(const gtString& exePath);
    void addEnvironmentVariable(const osEnvironmentVariable& envVariable) { m_environmentVariables.push_back(envVariable); };
    void addEnvironmentVariablesString(const gtString& envVariableString, const gtString& delimiters);
    bool getEnvironmentVariable(const gtString& envVarName, gtString& envVarVal) const;
    void setEnvironmentVariable(const gtString& envVarName, const gtString& envVarVal);
    void clearEnvironmentVariables() { m_environmentVariables.clear(); };
    void setLastActiveMode(const gtString& mode) {m_lastActiveMode = mode;};
    void setLastActiveSessionType(const gtString& sessionType) {m_lastActiveSessionType = sessionType;};
    void SetSourceFilesDirectories(const gtString& sourceDirs) {m_sourceFilesDirectories = sourceDirs;};
    void SetSourceCodeRootLocation(gtString sourceCodeRootLocation) {m_sourceCodeRootLocation = sourceCodeRootLocation;};

    // Allows us to remember the host name  and port(even if we are NOT in a remote session).
    // To know whether we are in a remote session or not, you should use the isRemoteTarget() function.
    void SetRemoteTargetHostname(const gtString& remoteHostName);
    void setRemoteTargetDaemonPort(gtUInt16 daemonConnectionPort) {m_remoteTargetDaemonConnectionPort = daemonConnectionPort;};

    const gtString& projectName() const { return m_projectName; };
    bool isRemoteTarget() const { return m_isRemoteTarget; };
    bool shouldDisableVSDebugEngine() const { return m_doNotUseVSDebugEngine; };
    const gtString& remoteTargetName() const {return m_remoteTargetName;};
    gtUInt16 remoteTargetDaemonConnectionPort() const {return m_remoteTargetDaemonConnectionPort;};
    const osFilePath& executablePath() const { return m_executablePath; };
    const gtString& windowsStoreAppUserModelID() const { return m_winStoreAppUserModelId; };
    const gtString& commandLineArguments() const { return m_commandLineArguments; };
    const osDirectory& logFilesFolder() const { return m_logFilesFolder; };
    const osDirectory& workDirectory() const { return m_workDirectory; };
    const gtList<osEnvironmentVariable>& environmentVariables() const { return m_environmentVariables; };
    void environmentVariablesAsString(gtString& envVariablesAsString) const;
    gtString lastActiveMode() const {return m_lastActiveMode;};
    gtString lastActiveSessionType() const { return m_lastActiveSessionType;};

    // Source files:
    gtString SourceFilesDirectories() const { return m_sourceFilesDirectories;};
    const gtString& SourceCodeRootLocation() const { return m_sourceCodeRootLocation; };

    /// Recently used IP addresses:
    void GetRecentlyUsedRemoteIPAddresses(gtVector<gtString>& addressesVector) const ;
    gtString GetRecentlyUsedRemoteIPAddressesAsString() const { return m_recentlyUsedRemoteIPAddressesAsString; };
    void SetRecentlyUsedRemoteIPAddresses(const gtString& recentlyUsedHostsStr) { m_recentlyUsedRemoteIPAddressesAsString = recentlyUsedHostsStr; };

protected:

    // The project name:
    gtString m_projectName;

    // Is the project targeting a remote machine?
    bool m_isRemoteTarget;

    // Contains true iff we want to disallow use of the Visual Studio Native Debug Engine:
    bool m_doNotUseVSDebugEngine;

    // Remote target details:
    gtString m_remoteTargetName;
    gtUInt16 m_remoteTargetDaemonConnectionPort;

    // The path of the project executable:
    osFilePath m_executablePath;

    // The Windows Store App User Model ID
    gtString m_winStoreAppUserModelId;

    // Command line arguments for the debugged executable:
    gtString m_commandLineArguments;

    // The initial work directory of the debugged process:
    osDirectory m_workDirectory;

    // The path for the log files:
    osDirectory m_logFilesFolder;

    // A list of environment variables that will be added to the created process environment:
    gtList<osEnvironmentVariable> m_environmentVariables;

    // Save the last active mode, and the last active session type:
    gtString m_lastActiveMode;
    gtString m_lastActiveSessionType;

    // Source files directories:
    gtString m_sourceFilesDirectories;
    gtString m_sourceCodeRootLocation;

    /// A string describing the IP addresses that were recently used for this project.
    /// The addresses are separated by ";":
    gtString m_recentlyUsedRemoteIPAddressesAsString;

};

#endif //__APPROJECTSETTINGS_H

