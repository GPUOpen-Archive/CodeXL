//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdExitCommand.cpp
///
//==================================================================================

//------------------------------ gdExitCommand.cpp ------------------------------


// Local:
#include <AMDTGpuDebuggingComponents/Include/commands/gdExitCommand.h>


// ---------------------------------------------------------------------------
// Name:        gdExitCommand::~gdExitCommand
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        26/3/2008
// ---------------------------------------------------------------------------
gdExitCommand::~gdExitCommand()
{
}


// ---------------------------------------------------------------------------
// Name:        gdExitCommand::canExecuteSpecificCommand
// Description: Always returns true.
// Author:      Yaki Tebeka
// Date:        26/3/2008
// ---------------------------------------------------------------------------
bool gdExitCommand::canExecuteSpecificCommand()
{
    return true;
}


// ---------------------------------------------------------------------------
// Name:        gdExitCommand::executeSpecificCommand
// Description:
//  Performs actions that shold be performed before the application is terminated.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/3/2008
// ---------------------------------------------------------------------------
bool gdExitCommand::executeSpecificCommand()
{
    return true;
}
