//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file rdDebuggerCommandExecutor.h
///
//==================================================================================

//------------------------------ rdDebuggerCommandExecutor.h ------------------------------

#ifndef __RDDEBUGGERCOMMANDEXECUTOR_H
#define __RDDEBUGGERCOMMANDEXECUTOR_H

// Forward declarations:
class osChannel;
class apDebugProjectSettings;
class pdProcessDebugger;

// Local:
#include <AMDTProcessDebugger/Include/pdRemoteProcessDebuggerCommandId.h>

// ----------------------------------------------------------------------------------
// Class Name:          rdDebuggerCommandExecutor
// General Description: A class which reads incoming debugging commands from the channel
//                      connecting to the main app, and passes them to the actual process
//                      debugger.
// Author:              Uri Shomroni
// Creation Date:       10/8/2009
// ----------------------------------------------------------------------------------
class rdDebuggerCommandExecutor
{
public:
    rdDebuggerCommandExecutor(osChannel& processDebuggerConnectionChannel);
    ~rdDebuggerCommandExecutor();

    void listenToDebuggingCommands();

private:
    // Disallow use of default constructor, copy constructor and assignment operator:
    rdDebuggerCommandExecutor() = delete;
    rdDebuggerCommandExecutor(const rdDebuggerCommandExecutor&) = delete;
    rdDebuggerCommandExecutor& operator=(const rdDebuggerCommandExecutor&) = delete;

    bool handleDebuggingCommand(pdRemoteProcessDebuggerCommandId cmdId);
    bool processDebugProjectSettingsForLocalDebugger(apDebugProjectSettings& debugProjectSettings);

private:
    // The channel from which we receive commands for the process debugger:
    osChannel& _processDebuggerConnectionChannel;

    // Should we keep reading?
    bool _continueLoop;

    // In the Windows 32-bit remote debugging server, this "member" variable may change at runtime:
#if !((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) && (AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE))
    // The (real) process debugger, to which we pass the commands (and from which we
    // read outputs):
    pdProcessDebugger& _theProcessDebugger;
#endif
};

#endif //__RDDEBUGGERCOMMANDEXECUTOR_H

