//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apApiFunctionsInitializationData.h
///
//==================================================================================

//------------------------------ apApiFunctionsInitializationData.h ------------------------------

#ifndef __APAPIFUNCTIONSINITIALIZATIONDATA
#define __APAPIFUNCTIONSINITIALIZATIONDATA

// Infra:
#include <AMDTOSWrappers/Include/osTransferableObject.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apApiFunctionsInitializationData : public osTransferableObject
// General Description:
//  Contains data required to initialize the OpenGL32.dll spy and its API.
// Author:  AMD Developer Tools Team
// Creation Date:        16/5/2004
// ----------------------------------------------------------------------------------
class AP_API apApiFunctionsInitializationData : public osTransferableObject
{
public:
    apApiFunctionsInitializationData();
    apApiFunctionsInitializationData(const apDebugProjectSettings& processCreationData);

    void setFrameTerminators(unsigned int frameTerminators) { _frameTerminators = frameTerminators; };
    void setDebuggerInstallDir(const osFilePath& installDirectory) { _debuggerInstallDirectory = installDirectory; };
    void setDebuggedProcessWorkDir(const osFilePath& workDirectory) { _workDirectory = workDirectory; };
    void setLogFilesDirectoryPath(const osFilePath& logFilesDirectoryPath) { _logFilesDirectoryPath = logFilesDirectoryPath; };
    void setProjectlogFilesDirectoryPath(const osFilePath& projectlogFilesDirectoryPath) { _projectlogFilesDirectoryPath  = projectlogFilesDirectoryPath; };
    void setLoggedTexturesFileType(apFileType fileType) { _loggedTexturesFileType = fileType; };
    void setLoggingLimits(unsigned int maxOpenGLCallsPerContext, unsigned int maxOpenCLCalls, unsigned int maxOpenCLCommandPerQueue) {_maxLoggedOpenGLCallsPerContext = maxOpenGLCallsPerContext; _maxLoggedOpenCLCallsPerContext = maxOpenCLCalls; _maxLoggedOpenCLCommandsPerQueue = maxOpenCLCommandPerQueue;};

    unsigned int openGLRenderFrameTerminators() const { return _frameTerminators; };
    const osFilePath& debuggerInstallDir() const {  return _debuggerInstallDirectory; };
    const osDirectory& debuggedProcessWorkDir() const { return _workDirectory; };
    const osFilePath& logFilesDirectoryPath() const { return _logFilesDirectoryPath; };
    apFileType loggedTexturesFileType() const { return _loggedTexturesFileType; };
    unsigned int maxLoggedOpenGLCallsPerContext() const {return _maxLoggedOpenGLCallsPerContext;};
    unsigned int maxLoggedOpenCLCallsPerContext() const {return _maxLoggedOpenCLCallsPerContext;};
    unsigned int maxLoggedOpenCLCommandsPerQueue() const {return _maxLoggedOpenCLCommandsPerQueue;};
    gtString projectName() const {return _projectName;};

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
    // OpenGL render frame terminators:
    // (A mask of apFrameTerminators)
    unsigned int _frameTerminators;

    // CodeXL installation directory:
    osFilePath _debuggerInstallDirectory;

    // The debugged application working directory:
    osDirectory _workDirectory;

    // The path of a directory that will contain the log files:
    osFilePath _logFilesDirectoryPath;

    //The path of a directory that will contain the project log files:
    osFilePath _projectlogFilesDirectoryPath;

    // Logged textures file type:
    apFileType _loggedTexturesFileType;

    // Logging limits:
    unsigned int _maxLoggedOpenGLCallsPerContext;
    unsigned int _maxLoggedOpenCLCallsPerContext;
    unsigned int _maxLoggedOpenCLCommandsPerQueue;

    gtString _projectName;
};


#endif  // __APAPIFUNCTIONSINITIALIZATIONDATA
