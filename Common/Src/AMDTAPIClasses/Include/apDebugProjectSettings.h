//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebugProjectSettings.h
///
//==================================================================================

//------------------------------ apDebugProjectSettings.h ------------------------------

#ifndef __APDEBUGPROJECTSETTINGS_H
#define __APDEBUGPROJECTSETTINGS_H


// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTAPIClasses/Include/apFileType.h>
#include <AMDTAPIClasses/Include/apInterceptionMethod.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTAPIClasses/Include/apProjectSettings.h>

// Default values:
#define AP_DEFAULT_OPENGL_CONTEXT_CALLS_LOG_MAX_SIZE 4000000
#define AP_DEFAULT_OPENCL_CONTEXT_CALLS_LOG_MAX_SIZE 50000
#define AP_DEFAULT_OPENCL_QUEUE_COMMANDS_LOG_MAX_SIZE 2000

// Remote Targets:
#define AP_REMOTE_TARGET_CONNECTION_DEFAULT_CONNECTION_PORT 2023
#define AP_REMOTE_TARGET_CONNECTION_DEFAULT_EVENTS_PORT 2024
#define AP_REMOTE_TARGET_CONNECTION_DEFAULT_SPY_API_PORT 2025
#define AP_REMOTE_TARGET_CONNECTION_DEFAULT_SPY_EVENTS_PORT 2026

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apDebugProjectSettings : public apProjectSettings
// General Description: Contain data required for debugging project
// Author:  AMD Developer Tools Team
// Creation Date:       4/4/2012
// ----------------------------------------------------------------------------------
class AP_API apDebugProjectSettings : public apProjectSettings
{
public:
    apDebugProjectSettings();
    apDebugProjectSettings(const  gtString& projectName,
                           const osFilePath& executablePath,
                           const gtString& commandLineArguments,
                           const osFilePath& debuggerInstallDirectory,
                           const osFilePath& workDirectory,
                           const osFilePath& spiesDirectory,
                           unsigned int frameTerminators,
                           apFileType loggedImagesFileType,
                           unsigned int maxLoggedOpenGLCallsPerContext,
                           unsigned int maxLoggedOpenCLCalls,
                           unsigned int maxLoggedOpenCLCommandsPerQueue,
                           bool initializeDirectDrawLibrary,
                           bool useAutomaticConfiguration);

    apDebugProjectSettings(const apDebugProjectSettings& other);
    apDebugProjectSettings(const apProjectSettings& projectGeneralSettings, const apDebugProjectSettings& projectDebugSettings);
    apDebugProjectSettings& operator=(const apDebugProjectSettings& other);

    void copyFrom(const apProjectSettings& projectGeneralSettings);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    void setDebuggerInstallDir(const osFilePath& installDirectory) { m_debuggerInstallDirectory = installDirectory; };
    void setSpiesDirectory(const osFilePath& spiesDir) { m_spiesDirectory = spiesDir; };
    void setInterceptionMethod(apInterceptionMethod intereptionMethod) { m_interceptionMethod = intereptionMethod; };
    void setFrameTerminators(unsigned int frameTerminators) { m_frameTerminators = frameTerminators; };
    void setLoggedImagesFileType(apFileType fileType) { m_loggedImagesFileType = fileType; };
    void setMaxLoggedOpenGLCallsPerContext(unsigned int callsAmount) {m_maxLoggedOpenGLCallsPerContext = callsAmount;};
    void setMaxLoggedOpenCLCallsPerContext(unsigned int callsAmount) {m_maxLoggedOpenCLCallsPerContext = callsAmount;};
    void setMaxLoggedOpenCLCommandsPerQueue(unsigned int commandsAmount) {m_maxLoggedOpenCLCommandsPerQueue = commandsAmount;};
    void setRemoteTargetConnectionPort(unsigned short remoteTargetConnectionPort) {m_remoteTargetConnectionPort = remoteTargetConnectionPort;};
    void setRemoteTargetEventsPort(unsigned short remoteTargetEventsPort) {m_remoteTargetEventsPort = remoteTargetEventsPort;};
    void setRemoteConnectionAPIPort(unsigned short remoteConnectionAPIPort) {m_remoteConnectionAPIPort = remoteConnectionAPIPort;};
    void setRemoteConnectionSpiesEventsPort(unsigned short remoteConnectionEventsPort) {m_remoteConnectionSpiesEventsPort = remoteConnectionEventsPort;};
    void setSpiesEventsPipeName(const gtString& eventsPipeName) { m_spiesEventsPipeName = eventsPipeName; };
    void setAutomaticConfiguration(bool useAuto) {m_useAutomaticConfiguration = useAuto;};
    void setShouldDebugHSAKernels(bool debugHSA) { m_shouldDebugHSAKernels = debugHSA; };
    void setShouldInitializePerformanceCounters(bool shouldMonitorPerformanceCounters) {m_shouldInitializePerformanceCounters = shouldMonitorPerformanceCounters;};
    void setBreakpointsLastFilePath(const osFilePath& lastBreakpointsFile) {m_breakpointsLastFile = lastBreakpointsFile;}

    const osFilePath& debuggerInstallDir() const {  return m_debuggerInstallDirectory; };
    const osFilePath& spiesDirectory() const { return m_spiesDirectory; };
    apInterceptionMethod interceptionMethod() const { return m_interceptionMethod; };
    unsigned int frameTerminatorsMask() const { return m_frameTerminators; };
    apFileType loggedImagesFileType() const { return m_loggedImagesFileType; };
    unsigned int maxLoggedOpenGLCallsPerContext() const {return m_maxLoggedOpenGLCallsPerContext;};
    unsigned int maxLoggedOpenCLCallsPerContext() const {return m_maxLoggedOpenCLCallsPerContext;};
    unsigned int maxLoggedOpenCLCommandsPerQueue() const {return m_maxLoggedOpenCLCommandsPerQueue;};
    unsigned short remoteTargetConnectionPort() const {return m_remoteTargetConnectionPort;};
    unsigned short remoteTargetEventsPort() const {return m_remoteTargetEventsPort;};
    unsigned short remoteConnectionAPIPort() const { return m_remoteConnectionAPIPort; };
    unsigned short remoteConnectionSpiesEventsPort() const { return m_remoteConnectionSpiesEventsPort; };
    const gtString& spiesEventsPipeName() const { return m_spiesEventsPipeName; };
    bool useAutomaticConfiguration() const {return m_useAutomaticConfiguration;};
    bool shouldDebugHSAKernels() const { return m_shouldDebugHSAKernels; };
    bool shouldInitializePerformanceCounters() const {return m_shouldInitializePerformanceCounters;};
    const osFilePath& breakpointsLastFilePath() const {return m_breakpointsLastFile;}

protected:

    // CodeXL installation directory:
    osFilePath m_debuggerInstallDirectory;

    // The directory of the spies (Dlls that imitate other dlls):
    osFilePath m_spiesDirectory;

    // The interception method to be used:
    apInterceptionMethod m_interceptionMethod;

    // OpenGL render frame terminators:
    // (A mask of apFrameTerminators)
    unsigned int m_frameTerminators;

    // Logged textures file type:
    apFileType m_loggedImagesFileType;

    // Maximal logged items amounts for loggers:
    unsigned int m_maxLoggedOpenGLCallsPerContext;
    unsigned int m_maxLoggedOpenCLCallsPerContext;
    unsigned int m_maxLoggedOpenCLCommandsPerQueue;

    // Details needed to connect to remote targets:
    unsigned short m_remoteTargetConnectionPort;      // For RDS commands
    unsigned short m_remoteTargetEventsPort;          // For RDS events
    unsigned short m_remoteConnectionAPIPort;         // For Spy commands
    unsigned short m_remoteConnectionSpiesEventsPort; // For Spy events

    // The name of the spies events pipe (used when debugging on the local host)
    gtString m_spiesEventsPipeName;

    // Contains true iff we want to use the automatic configuration option for an iPhone on-device project:
    bool m_useAutomaticConfiguration;

    // Contains true iff we want to enable debugging of HSA kernels (currently only supported on Linux)
    bool m_shouldDebugHSAKernels;

    // Contain true iff we want the spy to monitor performance counters:
    bool m_shouldInitializePerformanceCounters;

    // Last file that was used for breakpoints:
    osFilePath m_breakpointsLastFile;
};



#endif //__APDEBUGPROJECTSETTINGS_H

