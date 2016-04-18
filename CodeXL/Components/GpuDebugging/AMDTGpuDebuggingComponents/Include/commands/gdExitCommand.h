//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdExitCommand.h
///
//==================================================================================

//------------------------------ gdExitCommand.h ------------------------------

#ifndef __GDEXITCOMMAND
#define __GDEXITCOMMAND

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommand.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           gdExitCommand : public afCommand
// General Description:
//  Exit command - should be called when the application is terminated.
// Author:               Yaki Tebeka
// Creation Date:        26/3/2008
// ----------------------------------------------------------------------------------
class GD_API gdExitCommand : public afCommand
{
public:
    gdExitCommand() {};
    virtual ~gdExitCommand();

    // Overrides afCommand:
    virtual bool canExecuteSpecificCommand();
    virtual bool executeSpecificCommand();
};


#endif  // __GDEXITCOMMAND
