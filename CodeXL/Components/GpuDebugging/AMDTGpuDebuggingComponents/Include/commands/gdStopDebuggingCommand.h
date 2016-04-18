//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStopDebuggingCommand.h
///
//==================================================================================

//------------------------------ gdStopDebuggingCommand.h ------------------------------

#ifndef __GDSTOPDEBUGGINGCOMMAND
#define __GDSTOPDEBUGGINGCOMMAND

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommand.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           gdStopDebuggingCommand : public afCommand
// General Description:
//   Terminates the debugged application.
//
// Author:               Yaki Tebeka
// Creation Date:        1/11/2003
// ----------------------------------------------------------------------------------
class GD_API gdStopDebuggingCommand : public afCommand
{
public:
    gdStopDebuggingCommand() {};
    virtual ~gdStopDebuggingCommand();

    // Overrides afCommand:
    virtual bool canExecuteSpecificCommand();
    virtual bool executeSpecificCommand();
};


#endif  // __GDSTOPDEBUGGINGCOMMAND
