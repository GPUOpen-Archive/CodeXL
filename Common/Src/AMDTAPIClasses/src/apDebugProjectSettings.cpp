//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebugProjectSettings.cpp
///
//==================================================================================

//------------------------------ apDebugProjectSettings.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>

// ---------------------------------------------------------------------------
// Name:        apDebugProjectSettings::apDebugProjectSettings
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        29/6/2004
// ---------------------------------------------------------------------------
apDebugProjectSettings::apDebugProjectSettings()
    : m_interceptionMethod(AP_DEFAULT_INTERCEPTION_METHOD), m_frameTerminators(0),
      m_loggedImagesFileType(AP_PNG_FILE), m_maxLoggedOpenGLCallsPerContext(AP_DEFAULT_OPENGL_CONTEXT_CALLS_LOG_MAX_SIZE),
      m_maxLoggedOpenCLCallsPerContext(AP_DEFAULT_OPENCL_CONTEXT_CALLS_LOG_MAX_SIZE), m_maxLoggedOpenCLCommandsPerQueue(AP_DEFAULT_OPENCL_QUEUE_COMMANDS_LOG_MAX_SIZE),
      m_remoteTargetConnectionPort(AP_REMOTE_TARGET_CONNECTION_DEFAULT_CONNECTION_PORT),
      m_remoteTargetEventsPort(AP_REMOTE_TARGET_CONNECTION_DEFAULT_EVENTS_PORT),
      m_remoteConnectionAPIPort(AP_REMOTE_TARGET_CONNECTION_DEFAULT_SPY_API_PORT),
      m_remoteConnectionSpiesEventsPort(AP_REMOTE_TARGET_CONNECTION_DEFAULT_SPY_EVENTS_PORT),
      m_useAutomaticConfiguration(false), m_shouldDebugHSAKernels(false), m_shouldInitializePerformanceCounters(true)
{
}


// ---------------------------------------------------------------------------
// Name:        apDebugProjectSettings::apDebugProjectSettings
// Description: Constructor
// Arguments:
//   executablePath - The full path and name of the executable to be debugged.
//   commandLineArguments - Command line arguments as a command line string.
//   debuggerInstallDirectory - The directory in which CodeXL is installed.
//   workingDirectory - The initial work directory of the debugged process.
//   spiesDirectory - A directory that contains spies - Dlls that imitate other
//                    dlls.
//   frameTerminatorsMask - A mask of apFrameTerminators.
//                          (See ApiClasses/apFrameTerminators.h)
//   apiInitData - The spy API initialization data.
//
// Author:  AMD Developer Tools Team
// Date:        9/11/2003
// ---------------------------------------------------------------------------
apDebugProjectSettings::apDebugProjectSettings(const gtString& projectName,
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
                                               bool useAutomaticConfiguration)
    : apProjectSettings(projectName, executablePath, commandLineArguments, workDirectory),
      m_debuggerInstallDirectory(debuggerInstallDirectory),
      m_spiesDirectory(spiesDirectory),
      m_interceptionMethod(AP_DEFAULT_INTERCEPTION_METHOD),
      m_frameTerminators(frameTerminators),
      m_loggedImagesFileType(loggedImagesFileType),
      m_maxLoggedOpenGLCallsPerContext(maxLoggedOpenGLCallsPerContext),
      m_maxLoggedOpenCLCallsPerContext(maxLoggedOpenCLCalls),
      m_maxLoggedOpenCLCommandsPerQueue(maxLoggedOpenCLCommandsPerQueue),
      m_remoteTargetConnectionPort(AP_REMOTE_TARGET_CONNECTION_DEFAULT_CONNECTION_PORT),
      m_remoteTargetEventsPort(AP_REMOTE_TARGET_CONNECTION_DEFAULT_EVENTS_PORT),
      m_remoteConnectionAPIPort(AP_REMOTE_TARGET_CONNECTION_DEFAULT_SPY_API_PORT),
      m_remoteConnectionSpiesEventsPort(AP_REMOTE_TARGET_CONNECTION_DEFAULT_SPY_EVENTS_PORT),
      m_useAutomaticConfiguration(useAutomaticConfiguration),
      m_shouldDebugHSAKernels(false),
      m_shouldInitializePerformanceCounters(true)
{
    (void)(initializeDirectDrawLibrary); // unused
}


// ---------------------------------------------------------------------------
// Name:        apDebugProjectSettings::apDebugProjectSettings
// Description: Copy constructor
// Author:  AMD Developer Tools Team
// Date:        9/11/2003
// ---------------------------------------------------------------------------
apDebugProjectSettings::apDebugProjectSettings(const apDebugProjectSettings& other)
    : apProjectSettings(other),
      m_debuggerInstallDirectory(other.m_debuggerInstallDirectory),
      m_spiesDirectory(other.m_spiesDirectory),
      m_interceptionMethod(other.m_interceptionMethod),
      m_frameTerminators(other.m_frameTerminators),
      m_loggedImagesFileType(other.m_loggedImagesFileType),
      m_maxLoggedOpenGLCallsPerContext(other.m_maxLoggedOpenGLCallsPerContext),
      m_maxLoggedOpenCLCallsPerContext(other.m_maxLoggedOpenCLCallsPerContext),
      m_maxLoggedOpenCLCommandsPerQueue(other.m_maxLoggedOpenCLCommandsPerQueue),
      m_remoteTargetConnectionPort(other.m_remoteTargetConnectionPort),
      m_remoteTargetEventsPort(other.m_remoteTargetEventsPort),
      m_remoteConnectionAPIPort(other.m_remoteConnectionAPIPort),
      m_remoteConnectionSpiesEventsPort(other.m_remoteConnectionSpiesEventsPort),
      m_spiesEventsPipeName(other.m_spiesEventsPipeName),
      m_useAutomaticConfiguration(other.m_useAutomaticConfiguration),
      m_shouldDebugHSAKernels(other.m_shouldDebugHSAKernels),
      m_shouldInitializePerformanceCounters(other.m_shouldInitializePerformanceCounters)
{
}

// ---------------------------------------------------------------------------
// Name:        apDebugProjectSettings::apDebugProjectSettings
// Description: Construct a debug settings object from a general settings object
//              and another debug settings object
// Arguments:   const apProjectSettings& projectGeneralSettings
//              const apDebugProjectSettings& projectDebugSettings
// Author:  AMD Developer Tools Team
// Date:        10/4/2012
// ---------------------------------------------------------------------------
apDebugProjectSettings::apDebugProjectSettings(const apProjectSettings& projectGeneralSettings, const apDebugProjectSettings& projectDebugSettings)
    : apProjectSettings(projectGeneralSettings),
      m_debuggerInstallDirectory(projectDebugSettings.m_debuggerInstallDirectory),
      m_spiesDirectory(projectDebugSettings.m_spiesDirectory),
      m_interceptionMethod(projectDebugSettings.m_interceptionMethod),
      m_frameTerminators(projectDebugSettings.m_frameTerminators),
      m_loggedImagesFileType(projectDebugSettings.m_loggedImagesFileType),
      m_maxLoggedOpenGLCallsPerContext(projectDebugSettings.m_maxLoggedOpenGLCallsPerContext),
      m_maxLoggedOpenCLCallsPerContext(projectDebugSettings.m_maxLoggedOpenCLCallsPerContext),
      m_maxLoggedOpenCLCommandsPerQueue(projectDebugSettings.m_maxLoggedOpenCLCommandsPerQueue),
      m_remoteTargetConnectionPort(projectDebugSettings.m_remoteTargetConnectionPort),
      m_remoteTargetEventsPort(projectDebugSettings.m_remoteTargetEventsPort),
      m_remoteConnectionAPIPort(projectDebugSettings.m_remoteConnectionAPIPort),
      m_remoteConnectionSpiesEventsPort(projectDebugSettings.m_remoteConnectionSpiesEventsPort),
      m_spiesEventsPipeName(projectDebugSettings.m_spiesEventsPipeName),
      m_useAutomaticConfiguration(projectDebugSettings.m_useAutomaticConfiguration),
      m_shouldDebugHSAKernels(projectDebugSettings.m_shouldDebugHSAKernels),
      m_shouldInitializePerformanceCounters(projectDebugSettings.m_shouldInitializePerformanceCounters)
{
}

// ---------------------------------------------------------------------------
// Name:        apDebugProjectSettings::operator=
// Description: Assignment operator - copies other content to me.
// Author:  AMD Developer Tools Team
// Date:        6/9/2005
// ---------------------------------------------------------------------------
apDebugProjectSettings& apDebugProjectSettings::operator=(const apDebugProjectSettings& other)
{
    // Call the base class copy constructor:
    apProjectSettings::operator=(other);

    // Copy other members into me:
    m_debuggerInstallDirectory = other.m_debuggerInstallDirectory;
    m_spiesDirectory = other.m_spiesDirectory;
    m_interceptionMethod = other.m_interceptionMethod;
    m_frameTerminators = other.m_frameTerminators;
    m_loggedImagesFileType = other.m_loggedImagesFileType;
    m_maxLoggedOpenGLCallsPerContext = other.m_maxLoggedOpenGLCallsPerContext;
    m_maxLoggedOpenCLCallsPerContext = other.m_maxLoggedOpenCLCallsPerContext;
    m_maxLoggedOpenCLCommandsPerQueue = other.m_maxLoggedOpenCLCommandsPerQueue;
    m_remoteTargetConnectionPort = other.m_remoteTargetConnectionPort;
    m_remoteTargetEventsPort = other.m_remoteTargetEventsPort;
    m_remoteConnectionAPIPort = other.m_remoteConnectionAPIPort;
    m_remoteConnectionSpiesEventsPort = other.m_remoteConnectionSpiesEventsPort;
    m_spiesEventsPipeName = other.m_spiesEventsPipeName;
    m_useAutomaticConfiguration = other.m_useAutomaticConfiguration;
    m_shouldDebugHSAKernels = other.m_shouldDebugHSAKernels;
    m_shouldInitializePerformanceCounters = other.m_shouldInitializePerformanceCounters;

    // Return a reference to myself:
    return *this;
}

// ---------------------------------------------------------------------------
// Name:        apDebugProjectSettings::copyFrom
// Description: Copy data from general project settings
// Arguments:   const apProjectSettings& projectGeneralSettings
// Author:  AMD Developer Tools Team
// Date:        22/7/2012
// ---------------------------------------------------------------------------
void apDebugProjectSettings::copyFrom(const apProjectSettings& projectGeneralSettings)
{
    // Call the base class copy constructor:
    apProjectSettings::operator=(projectGeneralSettings);
}

// ---------------------------------------------------------------------------
// Name:        apDebugProjectSettings::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apDebugProjectSettings::type() const
{
    return OS_TOBJ_ID_PROCESS_CREATION_DATA;
}


// ---------------------------------------------------------------------------
// Name:        apDebugProjectSettings::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebugProjectSettings::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the CodeXL installation directory into the channel:
    retVal = m_debuggerInstallDirectory.writeSelfIntoChannel(ipcChannel);

    // Write the directory of the spies into the channel:
    retVal = retVal && m_spiesDirectory.writeSelfIntoChannel(ipcChannel);

    // Write the interception method to be used into the channel:
    ipcChannel << (gtInt32)m_interceptionMethod;

    // Write the frame terminators into the channels:
    ipcChannel << (gtUInt32) m_frameTerminators;

    // Write the logged textures file type into the channel:
    ipcChannel << (gtInt32) m_loggedImagesFileType;

    // Write the logs maximum sizes to the channel:
    ipcChannel << (gtUInt32)m_maxLoggedOpenGLCallsPerContext;
    ipcChannel << (gtUInt32)m_maxLoggedOpenCLCallsPerContext;
    ipcChannel << (gtUInt32)m_maxLoggedOpenCLCommandsPerQueue;

    // Write the remote target connection parameters:
    ipcChannel << (gtUInt16)m_remoteTargetConnectionPort;
    ipcChannel << (gtUInt16)m_remoteTargetEventsPort;
    ipcChannel << (gtUInt16)m_remoteConnectionAPIPort;
    ipcChannel << (gtUInt16)m_remoteConnectionSpiesEventsPort;

    // Write the spies events pipe name:
    ipcChannel << m_spiesEventsPipeName;

    // Write the automatic configuration flag:
    ipcChannel << m_useAutomaticConfiguration;

    // Write the HSA debugging flag:
    ipcChannel << m_shouldDebugHSAKernels;

    // Write the performance counters flag:
    ipcChannel << m_shouldInitializePerformanceCounters;

    // Write the breakpoint file path:
    bool rcPth = m_breakpointsLastFile.writeSelfIntoChannel(ipcChannel);
    GT_ASSERT(rcPth);

    // Call my parent class's implementation:
    retVal = apProjectSettings::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apDebugProjectSettings::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebugProjectSettings::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the CodeXL installation directory from the channel:
    retVal = m_debuggerInstallDirectory.readSelfFromChannel(ipcChannel);

    // Read the directory of the spies from the channel:
    retVal = retVal && m_spiesDirectory.readSelfFromChannel(ipcChannel);

    // Read the interception method to be used from the channel:
    gtInt32 interceptionMethodAsInt32 = 0;
    ipcChannel >> interceptionMethodAsInt32;
    m_interceptionMethod = (apInterceptionMethod)interceptionMethodAsInt32;

    // Read the frame terminators from the channels:
    gtUInt32 frameTerminatorsAsUInt32 = 0;
    ipcChannel >> frameTerminatorsAsUInt32;
    m_frameTerminators = (unsigned int)frameTerminatorsAsUInt32;

    // Read the logged textures file type from the channel:
    gtInt32 texturesFileTypeAsInt32 = AP_PNG_FILE;
    ipcChannel >> texturesFileTypeAsInt32;
    m_loggedImagesFileType = (apFileType)texturesFileTypeAsInt32;

    // Read the logs maximums from the pipe:
    gtUInt32 maxLoggedOpenGLCallsPerContextAsUInt32 = AP_DEFAULT_OPENGL_CONTEXT_CALLS_LOG_MAX_SIZE;
    ipcChannel >> maxLoggedOpenGLCallsPerContextAsUInt32;
    m_maxLoggedOpenGLCallsPerContext = (unsigned int)maxLoggedOpenGLCallsPerContextAsUInt32;
    gtUInt32 maxLoggedOpenCLCallsAsUInt32 = AP_DEFAULT_OPENCL_CONTEXT_CALLS_LOG_MAX_SIZE;
    ipcChannel >> maxLoggedOpenCLCallsAsUInt32;
    m_maxLoggedOpenCLCallsPerContext = (unsigned int)maxLoggedOpenCLCallsAsUInt32;
    gtUInt32 maxLoggedOpenCLCommandsPerQueueAsUInt32 = AP_DEFAULT_OPENCL_QUEUE_COMMANDS_LOG_MAX_SIZE;
    ipcChannel >> maxLoggedOpenCLCommandsPerQueueAsUInt32;
    m_maxLoggedOpenCLCommandsPerQueue = (unsigned int)maxLoggedOpenCLCommandsPerQueueAsUInt32;

    // Read the remote target connection parameters:
    gtUInt16 remoteTargetConnectionPortAsUInt16 = AP_REMOTE_TARGET_CONNECTION_DEFAULT_CONNECTION_PORT;
    ipcChannel >> remoteTargetConnectionPortAsUInt16;
    m_remoteTargetConnectionPort = (unsigned short)remoteTargetConnectionPortAsUInt16;
    gtUInt16 remoteTargetEventsPortAsUInt16 = AP_REMOTE_TARGET_CONNECTION_DEFAULT_EVENTS_PORT;
    ipcChannel >> remoteTargetEventsPortAsUInt16;
    m_remoteTargetEventsPort = (unsigned short)remoteTargetEventsPortAsUInt16;
    gtUInt16 remoteConnectionAPIPortAsUInt16 = AP_REMOTE_TARGET_CONNECTION_DEFAULT_SPY_API_PORT;
    ipcChannel >> remoteConnectionAPIPortAsUInt16;
    m_remoteConnectionAPIPort = (unsigned short)remoteConnectionAPIPortAsUInt16;
    gtUInt16 remoteConnectionEventsPortAsUInt16 = AP_REMOTE_TARGET_CONNECTION_DEFAULT_SPY_EVENTS_PORT;
    ipcChannel >> remoteConnectionEventsPortAsUInt16;
    m_remoteConnectionSpiesEventsPort = (unsigned short)remoteConnectionEventsPortAsUInt16;

    // Read the spies events pipe name:
    ipcChannel >> m_spiesEventsPipeName;

    // Read the automatic configuration flag:
    ipcChannel >> m_useAutomaticConfiguration;

    // Read the HSA debugging flag:
    ipcChannel >> m_shouldDebugHSAKernels;

    // Read the performance counters flag:
    ipcChannel >> m_shouldInitializePerformanceCounters;

    // Read the breakpoint file path:
    bool rcPth = m_breakpointsLastFile.readSelfFromChannel(ipcChannel);
    GT_ASSERT(rcPth);

    // Call my parent class's implementation:
    retVal = apProjectSettings::readSelfFromChannel(ipcChannel);

    return retVal;
}


