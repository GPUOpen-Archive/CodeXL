//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStartDebuggingCommand.h
///
//==================================================================================

//------------------------------ gdStartDebuggingCommand.h ------------------------------

#ifndef __GDSTARTDEBUGGINGCOMMAND
#define __GDSTARTDEBUGGINGCOMMAND

// Infra:
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommand.h>

// ----------------------------------------------------------------------------------
// Class Name:           gdStartDebuggingCommand : public afCommand
// General Description:
//   Launches the debugged application.
//
// Author:               Yaki Tebeka
// Creation Date:        1/11/2003
// ----------------------------------------------------------------------------------
class GD_API gdStartDebuggingCommand : public afCommand
{
public:
    gdStartDebuggingCommand(const apDebugProjectSettings& processCreationData);
    virtual ~gdStartDebuggingCommand();

    // Overrides afCommand:
    virtual bool canExecuteSpecificCommand();
    virtual bool executeSpecificCommand();

private:

    bool isHostMachineConfigurationSupported() const;
    bool adjustSpiesDirectoryToBinaryType();
    bool isExecutableTypeSupported(const osFilePath& exePath);
    bool checkRequiredSystemLibraries() const;
    bool setKernelSourceFilePaths();

    /// Check if the debug settings are valid:
    bool AreDebugSettingsValid(bool& exeFileExist, bool& workingFolderExist, bool& logDirExistAndAccessible, bool& arePortsLegal, bool& arePortsDistinct, bool& requiredSystemLibrariesExists);

    /// Handles start debugging in case of invalid settings:
    void HandleInvalidSettings(bool exeFileExist, bool workingFolderExist, bool logDirExistAndAccessible, bool arePortsLegal, bool arePortsDistinct,
                               bool requiredSystemLibrariesExists, bool& failedOnGlobalOption);

    // Start debugging command:
    bool StartDebugging();

private:
    // The debugged process creation data:
    apDebugProjectSettings _processCreationData;
};

#endif  // __GDSTARTDEBUGGINGCOMMAND
