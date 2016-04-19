//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afCommand.cpp
///
//==================================================================================

// Local:
#include <AMDTApplicationFramework/Include/afCommand.h>


// ---------------------------------------------------------------------------
// Name:        afCommand::~afCommand
// Description: Destructor.
// Author:      Avi Shapira
// Date:        10/1/2007
// ---------------------------------------------------------------------------
afCommand::~afCommand()
{
}

// ---------------------------------------------------------------------------
// Name:        afCommand::canExecute
// Description: Checks if the command can be executed.
// Return Val:  bool - true - the command can be executed.
//                     false - The command cannot be executed.
// Author:      Yaki Tebeka
// Date:        1/11/2003
// Implementation Notes:
//   Each command sub-class should answer this question by itself.
//   But - we can also do things for all the commands (licenses, etc),
//         therefore we have this method.
// ---------------------------------------------------------------------------
bool afCommand::canExecute()
{
    return canExecuteSpecificCommand();
}


// ---------------------------------------------------------------------------
// Name:        afCommand::execute
// Description: Executes the command.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
bool afCommand::execute()
{
    bool rc = false;

    // If the command can be executed:
    if (canExecute())
    {
        // Execute it:
        rc = executeSpecificCommand();
    }

    return rc;
}

