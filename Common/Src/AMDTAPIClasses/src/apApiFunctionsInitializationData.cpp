//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apApiFunctionsInitializationData.cpp
///
//==================================================================================

//------------------------------ apApiFunctionsInitializationData.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apApiFunctionsInitializationData.h>


// ---------------------------------------------------------------------------
// Name:        apApiFunctionsInitializationData::apApiFunctionsInitializationData
// Description: Default constructor.
// Author:  AMD Developer Tools Team
// Date:        27/6/2004
// ---------------------------------------------------------------------------
apApiFunctionsInitializationData::apApiFunctionsInitializationData()
    : _frameTerminators(0), _loggedTexturesFileType(AP_PNG_FILE), _maxLoggedOpenGLCallsPerContext(AP_DEFAULT_OPENGL_CONTEXT_CALLS_LOG_MAX_SIZE),
      _maxLoggedOpenCLCallsPerContext(AP_DEFAULT_OPENCL_CONTEXT_CALLS_LOG_MAX_SIZE), _maxLoggedOpenCLCommandsPerQueue(AP_DEFAULT_OPENCL_QUEUE_COMMANDS_LOG_MAX_SIZE)
{
}


// ---------------------------------------------------------------------------
// Name:        apApiFunctionsInitializationData::apApiFunctionsInitializationData
// Description: Constructor.
// Arguments:   processCreationData - The debugged process creation data.
// Author:  AMD Developer Tools Team
// Date:        16/5/2004
// ---------------------------------------------------------------------------
apApiFunctionsInitializationData::apApiFunctionsInitializationData(const apDebugProjectSettings& processCreationData)
{
    _frameTerminators = processCreationData.frameTerminatorsMask();
    _debuggerInstallDirectory = processCreationData.debuggerInstallDir();
    _workDirectory = processCreationData.workDirectory();
    _loggedTexturesFileType = processCreationData.loggedImagesFileType();
    _maxLoggedOpenGLCallsPerContext = processCreationData.maxLoggedOpenGLCallsPerContext();
    _maxLoggedOpenCLCallsPerContext = processCreationData.maxLoggedOpenCLCallsPerContext();
    _maxLoggedOpenCLCommandsPerQueue = processCreationData.maxLoggedOpenCLCommandsPerQueue();
    _projectName = processCreationData.projectName();
}


// ---------------------------------------------------------------------------
// Name:        apApiFunctionsInitializationData::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        16/5/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apApiFunctionsInitializationData::type() const
{
    return OS_TOBJ_ID_APIFUNCTIONS_INIT_DATA;
}


// ---------------------------------------------------------------------------
// Name:        apApiFunctionsInitializationData::writeSelfIntoChannel
// Description: Writes my content into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        16/5/2004
// ---------------------------------------------------------------------------
bool apApiFunctionsInitializationData::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write this class data into the channel;
    ipcChannel << (gtUInt32)_frameTerminators;
    ipcChannel << _debuggerInstallDirectory;
    ipcChannel << _workDirectory;
    ipcChannel << _logFilesDirectoryPath;
    ipcChannel << _projectName;
    ipcChannel << (gtInt32)_loggedTexturesFileType;
    ipcChannel << (gtUInt32)_maxLoggedOpenGLCallsPerContext;
    ipcChannel << (gtUInt32)_maxLoggedOpenCLCallsPerContext;
    ipcChannel << (gtUInt32)_maxLoggedOpenCLCommandsPerQueue;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apApiFunctionsInitializationData::readSelfFromChannel
// Description: Read my content from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        16/5/2004
// ---------------------------------------------------------------------------
bool apApiFunctionsInitializationData::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    // Read _frameTerminators from the channel;
    gtUInt32 frameTerminatorsUInt32 = 0;
    ipcChannel >> frameTerminatorsUInt32;
    _frameTerminators = (unsigned int)frameTerminatorsUInt32;

    // Read the debugger install directory:
    gtAutoPtr<osFilePath> aptrDebuggerInstallDir;
    bool rc1 = osReadTransferableObjectFromChannel<osFilePath>(ipcChannel, aptrDebuggerInstallDir);

    if (rc1)
    {
        _debuggerInstallDirectory = *aptrDebuggerInstallDir;
    }

    // Read the debugged process work directory path:
    gtAutoPtr<osDirectory> aptrWorkDirFilePath;
    bool rc2 = osReadTransferableObjectFromChannel<osDirectory>(ipcChannel, aptrWorkDirFilePath);

    if (rc2)
    {
        _workDirectory = *aptrWorkDirFilePath;
    }

    // Read the log files directory path:
    gtAutoPtr<osFilePath> aptrLogFilesDirFilePath;
    bool rc3 = osReadTransferableObjectFromChannel<osFilePath>(ipcChannel, aptrLogFilesDirFilePath);

    if (rc3)
    {
        _logFilesDirectoryPath = *aptrLogFilesDirFilePath;
    }

    // Read the log files directory path:
    ipcChannel  >> _projectName;

    // Read the logged textures file type:
    gtInt32 loggedTexturesFileTypeAsInt32 = 0;
    ipcChannel >> loggedTexturesFileTypeAsInt32;
    _loggedTexturesFileType = (apFileType)loggedTexturesFileTypeAsInt32;

    // Read the logging limits:
    gtUInt32 maxLoggedOpenGLCallsPerContextAsUInt32 = AP_DEFAULT_OPENGL_CONTEXT_CALLS_LOG_MAX_SIZE;
    ipcChannel >> maxLoggedOpenGLCallsPerContextAsUInt32;
    _maxLoggedOpenGLCallsPerContext = (unsigned int)maxLoggedOpenGLCallsPerContextAsUInt32;
    gtUInt32 maxLoggedOpenCLCallsAsUInt32 = AP_DEFAULT_OPENCL_CONTEXT_CALLS_LOG_MAX_SIZE;
    ipcChannel >> maxLoggedOpenCLCallsAsUInt32;
    _maxLoggedOpenCLCallsPerContext = (unsigned int)maxLoggedOpenCLCallsAsUInt32;
    gtUInt32 maxLoggedOpenCLCommandsPerQueueAsUInt32 = AP_DEFAULT_OPENCL_QUEUE_COMMANDS_LOG_MAX_SIZE;
    ipcChannel >> maxLoggedOpenCLCommandsPerQueueAsUInt32;
    _maxLoggedOpenCLCommandsPerQueue = (unsigned int)maxLoggedOpenCLCommandsPerQueueAsUInt32;

    retVal = rc1 && rc2 && rc3;
    return retVal;
}


